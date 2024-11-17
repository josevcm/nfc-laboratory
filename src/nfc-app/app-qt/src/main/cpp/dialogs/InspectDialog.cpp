/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2024 Jose Vicente Campos Martinez, <josevcm@gmail.com>

  NFC-LABORATORY is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NFC-LABORATORY is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NFC-LABORATORY. If not, see <http://www.gnu.org/licenses/>.

*/

#include <QVBoxLayout>
#include <QTreeView>
#include <QMouseEvent>

#include <QtConfig.h>

#include <lab/data/RawFrame.h>

#include <model/ParserModel.h>

#include <styles/Theme.h>

#include <widgets/ParserDelegate.h>

#include "ui_InspectDialog.h"

#include "InspectDialog.h"

struct InspectDialog::Impl
{
   InspectDialog *dialog = nullptr;

   QSharedPointer<Ui_InspectDialog> ui;

   ParserModel *parserModel;

   explicit Impl(InspectDialog *dialog) : dialog(dialog), parserModel(new ParserModel(dialog)), ui(new Ui_InspectDialog())
   {
      setup();
   }

   void setup()
   {
      ui->setupUi(dialog);

      // setup protocol view model
      ui->infoView->setModel(parserModel);
      ui->infoView->setColumnWidth(ParserModel::Name, 120);
      ui->infoView->setColumnWidth(ParserModel::Flags, 60);
      ui->infoView->setItemDelegate(new ParserDelegate(ui->infoView));

      // update window caption
      dialog->setWindowTitle(NFC_LAB_VENDOR_STRING);

      // connect selection signal from frame model
      QObject::connect(ui->infoView->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &deselected) {
         infoSelectionChanged();
      });
   }

   void infoSelectionChanged()
   {
      QModelIndexList indexList = ui->infoView->selectionModel()->selectedIndexes();

      if (!indexList.isEmpty())
      {
         auto firstIndex = indexList.first();

         if (auto firstEntry = parserModel->entry(firstIndex))
         {
            ui->dataView->setData(toByteArray(firstEntry->frame()));
            ui->dataView->setSelection(firstEntry->rangeStart(), firstEntry->rangeEnd());
         }
      }
   }

   QByteArray toByteArray(const lab::RawFrame &frame)
   {
      QByteArray data;

      for (int i = 0; i < frame.limit(); i++)
      {
         data.append(frame[i]);
      }

      return data;
   }
};

InspectDialog::InspectDialog(QWidget *parent) : QDialog(parent, Qt::WindowCloseButtonHint | Qt::WindowTitleHint), impl(new Impl(this))
{
}

void InspectDialog::clear()
{
   impl->parserModel->resetModel();
}

void InspectDialog::addFrame(const lab::RawFrame &frame)
{
   impl->parserModel->append(frame);

   impl->ui->infoView->expandAll();
}

int InspectDialog::showModal()
{
   return Theme::showModalInDarkMode(this);
}

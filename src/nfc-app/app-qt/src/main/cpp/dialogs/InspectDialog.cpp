/*

  Copyright (c) 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <QVBoxLayout>
#include <QTreeView>

#include <nfc/NfcFrame.h>

#include <model/ParserModel.h>

#include <parser/ParserNfc.h>

#include <styles/ParserStyle.h>

#include <views/ui_InspectDialog.h>

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
      ui->infoView->setColumnWidth(ParserModel::Flags, 32);
      ui->infoView->setItemDelegate(new ParserStyle(ui->infoView));

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
         }
      }
   }

   QByteArray toByteArray(const nfc::NfcFrame &frame)
   {
      QByteArray data;

      for (int i = 0; i < frame.limit(); i++)
      {
         data.append(frame[i]);
      }

      return data;
   }
};

InspectDialog::InspectDialog(QWidget *parent) : QDialog(parent), impl(new Impl(this))
{
}

void InspectDialog::clear()
{
   impl->parserModel->resetModel();
}

void InspectDialog::addFrame(const nfc::NfcFrame &frame)
{
   impl->parserModel->append(frame);

   impl->ui->infoView->expandAll();
}

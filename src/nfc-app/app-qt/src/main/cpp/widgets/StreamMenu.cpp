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

#include <QDebug>
#include <QInputDialog>
#include <QWidgetAction>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QActionEvent>

#include <model/StreamFilter.h>
#include <model/StreamModel.h>

#include <styles/Theme.h>

#include "IconDelegate.h"

#include "StreamHeader.h"
#include "StreamMenu.h"

#include "ui_StreamMenu.h"

class OptionsAction : public QWidgetAction
{
   public:

      OptionsAction(QObject *parent, QWidget *widget) : QWidgetAction(parent)
      {
         setDefaultWidget(widget);
      }
};

struct StreamMenu::Impl
{
   StreamMenu *streamMenu;
   StreamHeader *streamHeader;
   StreamFilter *streamFilter;

   int section;

   int type;

   bool listFilterAction = false;

   QStandardItemModel optionsModel;

   QSharedPointer<Ui_StreamMenu> ui;

   static const QRegularExpression hexString;

   Impl(StreamMenu *streamMenu, StreamFilter *streamFilter, int section, StreamHeader *streamHeader) :
      streamHeader(streamHeader),
      streamMenu(streamMenu),
      streamFilter(streamFilter),
      section(section),
      ui(new Ui_StreamMenu)
   {
      // get column data type (UserRole give this information)
      type = streamFilter->headerData(section, Qt::Horizontal, Qt::UserRole).toInt();

      // setup visual layout
      ui->setupUi(streamMenu);

      // configure menu model for option list
      ui->optionsView->setModel(&optionsModel);
      ui->optionsView->setItemDelegate(new IconDelegate(ui->optionsView));

      // list for current selected options
      QStringList options;

      // set current filter values
      for (const StreamFilter::Filter &filter: streamFilter->filters(section))
      {
         // enable clear filter option
         ui->clearFilterAction->setEnabled(true);

         // show current filter status
         switch (filter.mode)
         {
            case StreamFilter::Greater:
               ui->greaterFilterAction->setText(format(tr("Greater than: %1"), filter));
               break;

            case StreamFilter::Smaller:
               ui->smallerFilterAction->setText(format(tr("Smaller than: %1"), filter));
               break;

            case StreamFilter::RegExp:
               ui->matchFilterAction->setText(format(tr("Match expression: %1"), filter));
               break;

            case StreamFilter::Bytes:
               ui->bytesFilterAction->setText(format(tr("Match bytes: %1"), filter));
               break;

            case StreamFilter::List:
               options = filter.value.toStringList();
               break;
         }
      }

      // enable greater / smaller filter for strings and numbers
      if (type == QMetaType::QString ||
         type == QMetaType::Int ||
         type == QMetaType::UInt ||
         type == QMetaType::Float ||
         type == QVariant::Double)
      {
         ui->greaterFilterAction->setVisible(true);
         ui->smallerFilterAction->setVisible(true);
      }

      // enable regexp filter for strings
      if (type == QMetaType::QString)
      {
         ui->matchFilterAction->setVisible(true);
      }

      // enable regexp filter for bytes
      if (type == QMetaType::QByteArray)
      {
         ui->matchFilterAction->setVisible(true);
         ui->bytesFilterAction->setVisible(true);
      }

      // build section filter options
      if (fillOptions(options))
      {
         ui->optionsWidget->setVisible(true);
         ui->listFilterAction->setDefaultWidget(ui->optionsWidget);
         streamMenu->addAction(ui->listFilterAction);

         // workaround for detect click on list options button
         QObject::connect(ui->confirmButton, &QPushButton::clicked, [=] {
            listFilterAction = true;
         });

         // workaround for detect click on list options button
         QObject::connect(ui->cancelButton, &QPushButton::clicked, [=] {
            listFilterAction = false;
         });
      }
   }

   bool fillOptions(const QStringList &selected)
   {
      // options for string columns
      if (!(type == QMetaType::QString || type == QMetaType::QStringList))
         return false;

      // get source model to give all options
      QAbstractItemModel *model = streamFilter->sourceModel();

      // build options with distinct values
      QMap<QString, QString> options;

      // the list is automatically sorted by QMap
      for (int r = 0; r < model->rowCount(); r++)
      {
         QModelIndex index = model->index(r, section);

         if (type == QMetaType::QString)
         {
            QString value = model->data(index).toString().trimmed();
            options[value] = value.isEmpty() ? "<blank>" : value;
         }
         else
         {
            for (const QString &value: model->data(index).toStringList())
            {
               options[value] = value;
            }
         }
      }

      // clear current model
      optionsModel.clear();

      // set model data
      for (const QString &value: options.values())
      {
         auto *item = new QStandardItem(value);

         item->setCheckable(true);
         item->setCheckState(selected.contains(value != "<blank>" ? value : "") ? Qt::Checked : Qt::Unchecked);

         if (section == StreamModel::Flags)
         {
            if (value == "startup")
               item->setIcon(Theme::startupIcon);
            else if (value == "exchange")
               item->setIcon(Theme::exchangeIcon);
            else if (value == "request")
               item->setIcon(Theme::requestIcon);
            else if (value == "response")
               item->setIcon(Theme::responseIcon);
            else if (value == "carrier-on")
               item->setIcon(Theme::carrierOnIcon);
            else if (value == "carrier-off")
               item->setIcon(Theme::carrierOffIcon);
            else if (value == "encrypted")
               item->setIcon(Theme::encryptedIcon);
            else if (value == "truncated")
               item->setIcon(Theme::truncatedIcon);
            else if (value == "crc-error")
               item->setIcon(Theme::crcErrorIcon);
            else if (value == "parity-error")
               item->setIcon(Theme::parityErrorIcon);
            else if (value == "sync-error")
               item->setIcon(Theme::syncErrorIcon);
            else if (value == "vcc-low")
               item->setIcon(Theme::vccLowIcon);
            else if (value == "vcc-high")
               item->setIcon(Theme::vccHighIcon);
            else if (value == "rst-low")
               item->setIcon(Theme::rstLowIcon);
            else if (value == "rst-high")
               item->setIcon(Theme::rstHighIcon);
         }

         optionsModel.appendRow(item);
      }

      return true;
   }

   void triggerAction(QAction *action)
   {
      if (action == ui->clearFilterAction)
      {
         filterClear();
         return;
      }

      if (action == ui->greaterFilterAction)
      {
         filterValue(StreamFilter::Greater);
         return;
      }

      if (action == ui->smallerFilterAction)
      {
         filterValue(StreamFilter::Smaller);
         return;
      }

      if (action == ui->matchFilterAction)
      {
         filterValue(StreamFilter::RegExp);
         return;
      }

      if (action == ui->bytesFilterAction)
      {
         filterValue(StreamFilter::Bytes);
         return;
      }

      if (listFilterAction)
      {
         filterList();
         return;
      }
   }

   void filterClear() const
   {
      streamFilter->clearFilters(section);
   }

   void filterValue(StreamFilter::Mode mode) const
   {
      QString input;
      QVariant filter;

      do
      {
         QInputDialog inputDialog;
         inputDialog.setWindowTitle(tr("Filter values"));
         inputDialog.setInputMode(QInputDialog::TextInput);
         inputDialog.setTextValue(input);

         if (mode == StreamFilter::Greater)
            inputDialog.setLabelText(tr("Greater than:"));
         else if (mode == StreamFilter::Smaller)
            inputDialog.setLabelText(tr("Smaller than:"));
         else if (mode == StreamFilter::Bytes)
            inputDialog.setLabelText(tr("Match bytes (hex):"));
         else if (mode == StreamFilter::RegExp)
            inputDialog.setLabelText(tr("Match expression:"));

         if (!inputDialog.exec())
            return;

         // take input value
         input = inputDialog.textValue();

         // filter for string values
         if (type == QMetaType::QString)
         {
            qInfo() << "filtering string value:" << input;

            filter = input;
         }

         // filter for bytes
         else if (mode == StreamFilter::Bytes)
         {
            // remove all white spaces
            QString text = input.simplified().replace(" ", "");

            // resulting string must be even
            if (text.length() % 2)
            {
               showError(tr("Invalid hex format, must be even number of chars"));
               continue;
            }

            // hex string must contain only 0-9 A-F chars
            if (!hexString.match(text).hasMatch())
            {
               showError(tr("Invalid hex format, must be string of 0-9 A-F chars"));
               continue;
            }

            qInfo() << "filtering bytes value:" << text;

            // convert hex string to bytes
            filter = QByteArray::fromHex(text.toLatin1());
         }

         // filter for regular expressions
         else if (mode == StreamFilter::RegExp)
         {
            QRegularExpression expression(inputDialog.textValue());

            if (!expression.isValid())
            {
               showError(tr("Invalid expression, %1").arg(expression.errorString()));
               continue;
            }

            qInfo() << "filtering regular expression:" << input;

            filter = expression;
         }

         // filter for double numbers
         else
         {
            bool ok;

            filter = inputDialog.textValue().toDouble(&ok);

            if (!ok)
            {
               showError(tr("Invalid number format"));
               continue;
            }

            qInfo() << "filtering number value:" << input;
         }

         streamFilter->addFilter(section, mode, filter);

         break;

      }
      while (true);
   }

   bool filterList() const
   {
      QStringList selected;

      for (int row = 0; row < optionsModel.rowCount(); row++)
      {
         QStandardItem *item = optionsModel.item(row);

         if (item->checkState() == Qt::Checked)
         {
            selected.append(item->text() != "<blank>" ? item->text() : "");
         }
      }

      if (!selected.isEmpty())
         streamFilter->addFilter(section, StreamFilter::List, selected);
      else
         streamFilter->removeFilter(section, StreamFilter::List);

      return !selected.isEmpty();
   }

   bool showError(const QString &message) const
   {
      Theme::messageDialog(streamHeader, tr("Alert"), message, QMessageBox::Warning, QMessageBox::Ok);
      return false;
   }

   static QString format(const QString &format, const StreamFilter::Filter &filter)
   {
      switch (filter.value.userType())
      {
         case QMetaType::QString:
            return format.arg(filter.value.toString());

         case QMetaType::Float:
            return format.arg(filter.value.toFloat());

         case QMetaType::Double:
            return format.arg(filter.value.toDouble());

         case QMetaType::Int:
            return format.arg(filter.value.toInt());

         case QMetaType::UInt:
            return format.arg(filter.value.toUInt());

         case QMetaType::QRegularExpression:
            return format.arg(filter.value.toRegularExpression().pattern());

         case QMetaType::QByteArray:
         {
            QString hex(filter.value.toByteArray().toHex(' ').toUpper());
            return format.arg(hex.length() > 11 ? hex.left(11) + "..." : hex);
         }

         default:
            return format.arg("<unknown value>");
      }
   }
};

const QRegularExpression StreamMenu::Impl::hexString("^[0-9A-Fa-f]+$");

StreamMenu::StreamMenu(StreamFilter *filter, int section, StreamHeader *streamHeader) : QMenu(streamHeader), impl(new Impl(this, filter, section, streamHeader))
{
}

void StreamMenu::exec(const QPoint &pos)
{
   QAction *action = QMenu::exec(pos);

   impl->triggerAction(action);
}

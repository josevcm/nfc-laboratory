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

#include <comp/QHexView.h>

#include <model/ParserModel.h>

#include <parser/ParserNfc.h>

#include <styles/ParserStyle.h>

#include "InspectDialog.h"

struct InspectDialog::Impl
{
   InspectDialog *parent = nullptr;

   QTreeView *infoView;
   QHexView *dataView;

   ParserModel *parserModel;

   explicit Impl(InspectDialog *parent) : parent(parent), parserModel(new ParserModel(parent))
   {
      setup();
   }

   void setup()
   {
      QFont infoFont = {"Courier", 10, -1, false};

      infoView = new QTreeView();
      dataView = new QHexView();

      // setup protocol view model
      infoView->setFont(infoFont);
      infoView->setLineWidth(0);
      infoView->setFrameShape(QFrame::NoFrame);
      infoView->setRootIsDecorated(true);
      infoView->setSelectionMode(QAbstractItemView::NoSelection);
      infoView->setSelectionBehavior(QAbstractItemView::SelectRows);
      infoView->setRootIsDecorated(true);
//      infoView->setHeaderHidden(true);
      infoView->setColumnWidth(ParserModel::Cmd, 120);
      infoView->setColumnWidth(ParserModel::Flags, 32);
      infoView->setItemDelegate(new ParserStyle(infoView));
      infoView->setModel(parserModel);

      // prepare layout
      auto *layout = new QVBoxLayout(parent);

      layout->setSpacing(0);
      layout->setContentsMargins(0, 0, 0, 0);
      layout->addWidget(infoView);
      layout->addWidget(dataView);
   }
};

InspectDialog::InspectDialog(QWidget *parent) : QDialog(parent), impl(new Impl(this))
{
   setMinimumSize({640, 400});
}

void InspectDialog::clear()
{
   impl->parserModel->resetModel();
}

void InspectDialog::addFrame(const nfc::NfcFrame &frame)
{
   // add frame
   impl->parserModel->append(frame);

   // expand protocol information
   impl->infoView->expandAll();
}

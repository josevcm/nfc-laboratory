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

#include "ParserDelegate.h"
#include "ParserWidget.h"

struct ParserWidget::Impl
{
   ParserWidget *widget = nullptr;

   explicit Impl(ParserWidget *parent) : widget(parent)
   {
   }
};

ParserWidget::ParserWidget(QWidget *parent) : QTreeView(parent), impl(new Impl(this))
{
   setItemDelegate(new ParserDelegate(this));
}

void ParserWidget::scrollTo(const QModelIndex &index, ScrollHint hint)
{
   // avoid horizontal scroll jump to last column when click table
   QTreeView::scrollTo(model()->index(index.row(), 0), hint);
}

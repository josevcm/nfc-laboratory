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

#include <model/StreamModel.h>
#include <model/StreamFilter.h>

#include "StreamDelegate.h"
#include "StreamHeader.h"
#include "StreamWidget.h"

struct StreamWidget::Impl
{
   StreamWidget *widget = nullptr;

   explicit Impl(StreamWidget *parent) : widget(parent)
   {
   }

   void selectAndScroll(double from, double to) const
   {
      QModelIndexList selectionList;

      if (auto streamFilter = dynamic_cast<StreamFilter *>(widget->model()))
      {
         selectionList = streamFilter->modelRange(from, to);
      }

      // clear current selection if is empty of multiple rows selected when sorting by not index column
      if (selectionList.isEmpty() || (selectionList.size() > 1 && widget->horizontalHeader()->sortIndicatorSection() != 0))
      {
         widget->selectionModel()->clearSelection();

         return;
      }

      // select range
      QItemSelection selection(selectionList.first(), selectionList.last());

      widget->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

      QRect selectionRect = widget->visualRect(selectionList.first());

      if (!widget->viewport()->rect().contains(selectionRect, true))
      {
         widget->scrollTo(selectionList.first());
      }
   }

   void clearFilters() const
   {
      if (auto streamFilter = dynamic_cast<StreamFilter *>(widget->model()))
      {
         // iterate over all columns and clear filter
         for (int column = 0; column < streamFilter->columnCount(); column++)
         {
            streamFilter->clearFilters(column);
         }
      }
   }

   bool isRowVisible(const QModelIndex &rowIndex) const
   {
      QRect rect = widget->visualRect(rowIndex);

      return widget->viewport()->rect().contains(rect);
   }

   bool isLastRowVisible() const
   {
      int totalRows = widget->model()->rowCount();

      QModelIndex lastRowIndex = widget->model()->index(totalRows - 1, 0);

      return isRowVisible(lastRowIndex);
   }
};

StreamWidget::StreamWidget(QWidget *parent) : QTableView(parent), impl(new Impl(this))
{
   QTableView::setSortingEnabled(true);
   QTableView::setStyleSheet("QTableView::pane { border: 0; }");
   QTableView::setHorizontalHeader(new StreamHeader(this));
   QTableView::setItemDelegate(new StreamDelegate(this));
   QTableView::setObjectName("StreamWidget");

   // toggle to single selection mode when sort not index column
   connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, [this](int section, Qt::SortOrder order) {

      if (section == 0)
         setSelectionMode(QAbstractItemView::ContiguousSelection);
      else
         setSelectionMode(QAbstractItemView::SingleSelection);
   });
}

void StreamWidget::select(double from, double to)
{
   impl->selectAndScroll(from, to);
}

void StreamWidget::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
   // avoid horizontal scroll jump to last column when click table
   QTableView::scrollTo(model()->index(index.row(), 0), hint);
}

void StreamWidget::clearFilters()
{
   impl->clearFilters();
}

bool StreamWidget::isRowVisible(const QModelIndex &index) const
{
   return impl->isRowVisible(index);
}

bool StreamWidget::isLastRowVisible() const
{
   return impl->isLastRowVisible();
}

void StreamWidget::setModel(QAbstractItemModel *model)
{
   QTableView::setModel(model);
}

void StreamWidget::setColumnType(int column, Type type)
{
   if (auto delegate = dynamic_cast<StreamDelegate *>(itemDelegate()))
   {
      delegate->setColumnType(column, type);
   }
}

void StreamWidget::setSortingEnabled(int column, bool enabled)
{
   if (auto header = dynamic_cast<StreamHeader *>(horizontalHeader()))
   {
      header->setSortingEnabled(column, enabled);
   }
}


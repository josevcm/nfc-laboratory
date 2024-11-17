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
#include <QHash>
#include <QItemSelection>
#include <QRegularExpression>
#include <utility>

#include "StreamModel.h"
#include "StreamFilter.h"

struct StreamFilter::Impl
{
   bool enabled = false;

   QHash<int, QList<Filter>> columnFilters;

   QHash<int, int> rowsAccepted;

   static QHash<QMetaType, int> numericTypes;

   static bool greater(const QVariant &threshold, const QVariant &value)
   {
      return compare(threshold, value) < 0;
   }

   static bool smaller(const QVariant &threshold, const QVariant &value)
   {
      return compare(threshold, value) > 0;
   }

   static bool matches(const QVariant &filter, const QVariant &value)
   {
      if (!value.isValid() || filter.userType() != QMetaType::QRegularExpression)
         return false;

      return filter.toRegularExpression().match(value.toString()).hasMatch();
   }

   static bool contains(const QVariant &filter, const QVariant &value)
   {
      // check for byte array or string
      if (filter.userType() == QMetaType::QByteArray && (value.userType() == QMetaType::QByteArray || value.userType() == QMetaType::QString))
      {
         return value.toByteArray().contains(filter.toByteArray());
      }

      // check for list or string
      if (filter.userType() == QMetaType::QStringList)
      {
         QStringList list = filter.toStringList();
         QStringList values = value.toStringList();

         return std::any_of(list.begin(), list.end(), [&values](const QString &item) {
            return values.contains(item) || (item.length() == 0 && values.isEmpty());
         });
      }

      return false;
   }

   static int compare(const QVariant &v1, const QVariant &v2)
   {
      if (!v1.isValid() && v2.isValid())
         return -1;

      if (v1.isValid() && !v2.isValid())
         return 1;

      // if at least one the values is string, perform compare as string
      if (v1.userType() == QMetaType::QString || v2.userType() == QMetaType::QString)
         return v1.toString().compare(v2.toString());

      // otherwise compare as numbers, converting to double
      if (v1.toDouble() < v2.toDouble())
         return -1;

      if (v1.toDouble() > v2.toDouble())
         return 1;

      return 0;
   }

   void addFilter(int column, const StreamFilter::Filter &filter)
   {
      // add new filter
      columnFilters[column].append(filter);
   }

   void removeFilter(int column, StreamFilter::Mode mode)
   {
      if (!columnFilters.contains(column))
         return;

      QList<Filter> &filters = columnFilters[column];

      auto it = filters.begin();

      while (it != filters.end())
      {
         if (it->mode == mode)
            it = filters.erase(it);
         else
            it++;
      }

      // if all filters are removed, clear column
      if (filters.isEmpty())
         columnFilters.remove(column);
   }

   void clearFilters(int column)
   {
      columnFilters.remove(column);
   }

   void clearAccepted()
   {
      rowsAccepted.clear();
   }
};

StreamFilter::StreamFilter(QObject *parent) : QSortFilterProxyModel(parent), impl(new Impl)
{
}

QVariant StreamFilter::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (role != Qt::UserRole + 1)
      return QSortFilterProxyModel::headerData(section, orientation, role);

   if (impl->rowsAccepted.contains(section))
      return impl->rowsAccepted[section];

   return {};
}

bool StreamFilter::filterAcceptsRow(int row, const QModelIndex &sourceParent) const
{
   // accept all rows if filtering is not enabled
   if (!impl->enabled)
      return true;

   // by default accept all rows
   bool rowAccepted = true;

   // check column filters
   for (int column = 0; column < sourceModel()->columnCount(sourceParent); column++)
   {
      // skip column if no filter set
      if (!impl->columnFilters.contains(column))
         continue;

      // get cell index, skip if not valid
      QModelIndex index = sourceModel()->index(row, column, sourceParent);

      if (!index.isValid())
         continue;

      // get cell value, skip if not set
      QVariant value = index.data(filterRole());

      // by default accept column
      bool columnAccepted = true;

      // check column filters
      for (const Filter &filter: impl->columnFilters[column])
      {
         switch (filter.mode)
         {
            case Greater:
               columnAccepted &= Impl::greater(filter.value, value);
               break;

            case Smaller:
               columnAccepted &= Impl::smaller(filter.value, value);
               break;

            case RegExp:
               columnAccepted &= Impl::matches(filter.value, value);
               break;

            case Bytes:
               columnAccepted &= Impl::contains(filter.value, value);
               break;

            case List:
               columnAccepted &= Impl::contains(filter.value, value);
               break;
         }
      }

      if (columnAccepted)
         impl->rowsAccepted[column]++;

      rowAccepted &= columnAccepted;
   }

   return rowAccepted;
}

bool StreamFilter::isEnabled()
{
   return impl->enabled;
}

void StreamFilter::setEnabled(bool enabled)
{
   impl->enabled = enabled;

   invalidateFilter();
}

bool StreamFilter::hasFilters(int column)
{
   return impl->columnFilters.contains(column);
}

QList<StreamFilter::Filter> StreamFilter::filters(int column)
{
   if (!impl->columnFilters.contains(column))
      return {};

   return impl->columnFilters[column];
}

void StreamFilter::addFilter(int column, const StreamFilter::Filter &filter)
{
   // remove current mode filter if exists
   impl->removeFilter(column, filter.mode);

   // add filter to column
   impl->addFilter(column, filter);

   // clear current accepted rows
   impl->clearAccepted();

   // trigger filter invalidate
   invalidateFilter();
}

void StreamFilter::addFilter(int column, Mode mode, const QVariant &value)
{
   addFilter(column, {.mode=mode, .value=value});
}

void StreamFilter::removeFilter(int column, const StreamFilter::Filter &filter)
{
   removeFilter(column, filter.mode);
}

void StreamFilter::removeFilter(int column, StreamFilter::Mode mode)
{
   // remove column filter
   impl->removeFilter(column, mode);

   // clear current accepted rows
   impl->clearAccepted();

   // trigger filter invalidate
   invalidateFilter();
}

void StreamFilter::clearFilters(int column)
{
   // clear all column filters
   impl->clearFilters(column);

   // clear accepted rows
   impl->clearAccepted();

   // trigger filter invalidate
   invalidateFilter();
}

int StreamFilter::rowsAccepted(int column)
{
   return impl->rowsAccepted.contains(column) && impl->rowsAccepted[column];
}

QModelIndexList StreamFilter::modelRange(double from, double to)
{
   QModelIndexList list;

   if (auto streamModel = dynamic_cast<StreamModel *>(sourceModel()))
   {
      for (QModelIndex sourceIndex: streamModel->modelRange(from, to))
      {
         list.append(mapFromSource(sourceIndex));
      }
   }

   return list;
}

const lab::RawFrame *StreamFilter::frame(const QModelIndex &index) const
{
   if (const auto streamModel = dynamic_cast<StreamModel *>(sourceModel()))
   {
      return streamModel->frame(mapToSource(index));
   }

   return {};
}




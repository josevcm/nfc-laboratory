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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <QDebug>
#include <QItemSelection>
#include <QRegularExpression>

#include "StreamModel.h"
#include "StreamFilter.h"

StreamFilter::StreamFilter(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool StreamFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
   if (!filterRegularExpression().isValid())
      return true;

   QString line;

   for (int sourceColumn = 0; sourceColumn < sourceModel()->columnCount(sourceParent); ++sourceColumn)
   {
      QModelIndex sourceIndex = sourceModel()->index(sourceRow, sourceColumn, sourceParent);

      QVariant value = sourceModel()->data(sourceIndex, filterRole());

      line.append(" " + value.toString());
   }

   return filterRegularExpression().match(line).hasMatch();
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

nfc::NfcFrame *StreamFilter::frame(const QModelIndex &index) const
{
   if (auto streamModel = dynamic_cast<StreamModel *>(sourceModel()))
   {
      return streamModel->frame(mapToSource(index));
   }

   return {};
}

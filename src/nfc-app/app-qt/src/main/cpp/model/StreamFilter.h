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

#ifndef NFC_LAB_STREAMFILTER_H
#define NFC_LAB_STREAMFILTER_H

#include <QObject>
#include <QSortFilterProxyModel>

namespace lab {
class RawFrame;
}

class StreamFilter : public QSortFilterProxyModel
{
   Q_OBJECT

      struct Impl;

   public:

      enum Mode
      {
         Greater, Smaller, RegExp, Bytes, List
      };

      struct Filter
      {
         Mode mode;
         QVariant value;
      };

   public:

      explicit StreamFilter(QObject *parent = nullptr);

      QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

      bool isEnabled();

      void setEnabled(bool enabled);

      bool hasFilters(int column);

      void clearFilters(int column);

      QList<Filter> filters(int column);

      void addFilter(int column, const Filter &filter);

      void addFilter(int column, Mode mode, const QVariant &value);

      void removeFilter(int column, const Filter &filter);

      void removeFilter(int column, Mode mode);

      int rowsAccepted(int column);

   public:

      QModelIndexList modelRange(double from, double to);

      const lab::RawFrame *frame(const QModelIndex &index) const;

   protected:

      bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

   private:

      QSharedPointer<Impl> impl;
};


#endif //NFC_LAB_STREAMFILTER_H

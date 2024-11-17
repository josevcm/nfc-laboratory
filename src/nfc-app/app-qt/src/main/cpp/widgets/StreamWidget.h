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

#ifndef NFC_LAB_STREAMWIDGET_H
#define NFC_LAB_STREAMWIDGET_H

#include <QTableView>

class StreamWidget : public QTableView
{
   Q_OBJECT

      struct Impl;

   public:

   public:

      enum Type
      {
         None = 0, Integer = 1, Elapsed = 2, Seconds = 3, DateTime = 4, Rate = 5, String = 6, Hex = 7
      };

   public:

      explicit StreamWidget(QWidget *parent = nullptr);

      void select(double from, double to);

      void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;

      void clearFilters();

      bool isRowVisible(const QModelIndex &index) const;

      bool isLastRowVisible() const;

      void setModel(QAbstractItemModel *model) override;

      void setColumnType(int column, Type type);

      void setSortingEnabled(int column, bool enabled);

   private:

      QSharedPointer<Impl> impl;
};

#endif // NFC_LAB_STREAMWIDGET_H

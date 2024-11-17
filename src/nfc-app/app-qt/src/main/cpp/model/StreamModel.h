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

#ifndef APP_STREAMMODEL_H
#define APP_STREAMMODEL_H

#include <QObject>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractTableModel>
#include <QList>
#include <QSharedPointer>

#include <QFont>
#include <QLabel>
#include <QPixmap>
#include <QPointer>

namespace lab {
class RawFrame;
}


class StreamModel : public QAbstractTableModel
{
   Q_OBJECT

      struct Impl;

   public:

      enum TimeSource
      {
         Elapsed = 0, DateTime = 1
      };

      enum Columns
      {
         Id = 0, Time = 1, Delta = 2, Rate = 3, Tech = 4, Event = 5, Flags = 6, Data = 7
      };

   public:

      explicit StreamModel(QObject *parent = nullptr);

      int rowCount(const QModelIndex &parent = QModelIndex()) const override;

      int columnCount(const QModelIndex &parent = QModelIndex()) const override;

      QVariant data(const QModelIndex &index, int role) const override;

      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

      QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

      Qt::ItemFlags flags(const QModelIndex &index) const override;

      bool canFetchMore(const QModelIndex &parent = QModelIndex()) const override;

      void fetchMore(const QModelIndex &parent = QModelIndex()) override;

      QModelIndexList modelRange(double from, double to);

      void resetModel();

      void append(const lab::RawFrame &frame);

      int timeSource() const;

      void setTimeSource(TimeSource timeSource);

      const lab::RawFrame *frame(const QModelIndex &index) const;

   signals:

      void modelChanged();

   private:

      QSharedPointer<Impl> impl;
};

#endif

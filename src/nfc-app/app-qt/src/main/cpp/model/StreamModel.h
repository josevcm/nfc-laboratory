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

namespace nfc {
class NfcFrame;
}

class StreamModel : public QAbstractTableModel
{
      struct Impl;

   Q_OBJECT

   public:

      enum Columns
      {
         Id = 0, Time = 1, Delta = 2, Rate = 3, Tech = 4, Cmd = 5, Flags = 6, Data = 7
      };

      enum TimeFormat
      {
         ElapsedTimeFormat = 0, DateTimeFormat = 1
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

      void append(const nfc::NfcFrame &frame);

      nfc::NfcFrame *frame(const QModelIndex &index) const;

      void setTimeFormat(int mode);

   signals:

      void modelChanged();

   private:

      QSharedPointer<Impl> impl;
};

#endif //NFC_LAB_TABLEMODEL_H

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

#ifndef APP_FRAMEMODEL_H
#define APP_FRAMEMODEL_H

#include <QObject>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QList>
#include <QSharedPointer>

#include <QFont>
#include <QLabel>
#include <QPixmap>
#include <QPointer>

#include <protocol/ProtocolFrame.h>

namespace nfc {
class NfcFrame;
}

class ParserModel : public QAbstractItemModel
{
      struct Impl;

   Q_OBJECT

   public:

      enum Columns
      {
         Name = 0, Flags = 1, Data = 2
      };

   public:

      ParserModel(QObject *parent = nullptr);

      QVariant data(const QModelIndex &index, int role) const override;

      Qt::ItemFlags flags(const QModelIndex &index) const override;

      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

      QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;

      QModelIndex parent(const QModelIndex &index) const override;

      bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

      int rowCount(const QModelIndex &parent = QModelIndex()) const override;

      int columnCount(const QModelIndex &parent = QModelIndex()) const override;

      bool insertRows(int position, int rows, const QModelIndex &parent) override;

      void resetModel();

      void append(const nfc::NfcFrame &frame);

      ProtocolFrame *entry(const QModelIndex &index) const;

   signals:

      void modelChanged();

   private:

      QSharedPointer<Impl> impl;
};

#endif /* APP_FRAMEMODEL_H */

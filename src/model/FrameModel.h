/*

  Copyright (c) 2020 Jose Vicente Campos Martinez - <josevcm@gmail.com>

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

#ifndef FRAMEMODEL_H
#define FRAMEMODEL_H

#include <QObject>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QList>

#include <QFont>
#include <QPixmap>
#include <QPointer>

#include <decoder/NfcStream.h>
#include <decoder/NfcIterator.h>

class ProtocolParser;
class ProtocolFrame;

class FrameModel: public QAbstractItemModel
{
      Q_OBJECT

   public:

      enum Columns
      {
         Id = 0, Time = 1, Elapsed = 2, Rate = 3, Type = 4, Data = 5
      };

   private:

      NfcStream *mStream;
      NfcIterator mIterator;

      ProtocolFrame *mRootFrame;
      ProtocolParser *mProtocolParser;

      bool mGroupRepeated;

      QFont mDefaultFont;
      QFont mRequestDefaultFont;
      QFont mRequestParityErrorFont;
      QFont mRequestCrcErrorFont;
      QFont mResponseDefaultFont;
      QFont mResponseParityErrorFont;
      QFont mResponseCrcErrorFont;
      QFont mFieldFont;

      QPixmap mFieldIcon;
      QPixmap mRequestIcon;
      QPixmap mResponseIcon;
      QPixmap mWarningIcon;
      QPixmap mPaddingIcon;

   private:

      ProtocolFrame *getItem(const QModelIndex &index) const;

      QString toString(const QByteArray &value) const;

      QString padding(int count, QVariant data) const;

      bool compare(ProtocolFrame *a, ProtocolFrame *b);

   public:

      FrameModel(NfcStream *stream, QObject *parent = nullptr);

      virtual ~FrameModel();

      QVariant data(const QModelIndex &index, int role) const;

      Qt::ItemFlags flags(const QModelIndex &index) const;

      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

      QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;

      QModelIndex parent(const QModelIndex &index) const;

      bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

      int rowCount(const QModelIndex &parent = QModelIndex()) const;

      int columnCount(const QModelIndex &parent = QModelIndex()) const;

      bool insertRows(int position, int rows, const QModelIndex &parent);

      bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;

      void fetchMore(const QModelIndex &parent = QModelIndex());

      QModelIndexList modelRange(double from, double to);

      void resetModel();

      void setGroupRepeated(bool value);

   signals:

      void modelChanged();
};

#endif /* FRAMEMODEL_H */

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

#include <QtWidgets>

#include <QStringList>

#include <protocol/ProtocolParser.h>
#include <protocol/ProtocolFrame.h>

#include "FrameModel.h"

FrameModel::FrameModel(NfcStream *stream, QObject *parent) :
   QAbstractItemModel(parent),
   mStream(stream),
   mIterator(stream),
   mRootFrame(nullptr),
   mProtocolParser(nullptr),
   mGroupRepeated(false),
   mFieldIcon(QPixmap(":/app_icons/arrow-blue")),
   mRequestIcon(QPixmap(":/app_icons/arrow-green")),
   mResponseIcon(QPixmap(":/app_icons/arrow-red")),
   mWarningIcon(QPixmap(":/app_icons/warning-yellow")),
   mPaddingIcon(QPixmap(":/app_icons/transparent"))
{
   QVector<QVariant> rootData;

   rootData << "#" << "Time" << "Elapsed" << "Rate" << "Type" << "Frame";

   mProtocolParser = new ProtocolParser(this);
   mRootFrame = new ProtocolFrame(rootData, this);

   mRequestDefaultFont.setBold(true);
   mRequestCrcErrorFont.setBold(true);
   mRequestCrcErrorFont.setUnderline(true);
   mRequestParityErrorFont.setBold(true);
   mRequestParityErrorFont.setStrikeOut(true);

   mResponseDefaultFont.setItalic(true);
   mResponseCrcErrorFont.setItalic(true);
   mResponseCrcErrorFont.setUnderline(true);
   mResponseParityErrorFont.setItalic(true);
   mResponseParityErrorFont.setStrikeOut(true);

   mFieldFont.setItalic(true);
}

FrameModel::~FrameModel()
{
}

QVariant FrameModel::data(const QModelIndex &index, int role) const
{
   if (!index.isValid())
      return QVariant();

   int col = index.column();

   ProtocolFrame *item = getItem(index);

   if (role == Qt::UserRole)
   {
      return item->data(col);
   }

   if (role == Qt::DecorationRole)
   {
      switch (col)
      {
         case Columns::Type:
         {
            if (item->isFrameField())
               return mFieldIcon;

            break;
         }

         case Columns::Data:
         {
            if (item->isFieldInfo())
               return  mPaddingIcon;

            if (item->isFrameField())
               return  mPaddingIcon;

            if (item->hasCrcError() || item->hasParityError())
               return mWarningIcon;

            if (item->isRequestFrame())
               return mRequestIcon;

            if (item->isResponseFrame())
               return  mResponseIcon;

            break;
         }
      }
   }

   if (role == Qt::DisplayRole)
   {
      switch (col)
      {
         case Columns::Time:
         {
            bool ok = false;

            double timestamp = item->data(col).toDouble(&ok);

            if (ok)
            {
               return QString("%1").arg(timestamp, 9, 'f', 5);
            }

            return QVariant();
         }

         case Columns::Elapsed:
         {
            bool ok = false;

            double elapsed = item->data(col).toDouble(&ok);

            if (ok)
            {
               if (elapsed < 1E-3)
                  return QString("%1 us").arg(elapsed * 1000000, 3, 'f', 0);

               if (elapsed < 1)
                  return QString("%1 ms").arg(elapsed * 1000, 3, 'f', 0);

               return QString("%1 s").arg(elapsed, 3, 'f', 0);
            }

            return QVariant();
         }

         case Columns::Rate:
         {
            bool ok = false;

            int rate = item->data(col).toInt(&ok);

            if (ok)
            {
               return QString("%1k").arg(double(rate / 1000.0f), 3, 'f', 0);
            }

            return QVariant();
         }

         case Columns::Type:
         {
            QString type = item->data(col).toString();

            if (item->repeated() > 0)
               return QString("%1 [%2]").arg(type).arg(item->repeated() + 1);

            return type;
         }

         case Columns::Data:
         {
            QVariant info = item->data(col);

            if (info.type() == QVariant::ByteArray)
            {
               return padding(item->childDeep(), toString(info.toByteArray()));
            }

            return padding(item->childDeep(), info);
         }
      }

      return item->data(col);
   }

   if (role == Qt::FontRole)
   {
      switch (col)
      {
         case Columns::Type:
         {
            if (item->isFrameField())
               return mFieldFont;

            break;
         }

         case Columns::Data:
         {
            if (item->isRequestFrame())
            {
               if (item->isFieldInfo())
                  return  mFieldFont;

               if (item->hasCrcError())
                  return mRequestCrcErrorFont;

               if (item->hasParityError())
                  return mRequestParityErrorFont;

               return  mRequestDefaultFont;
            }
            else
            {
               if (item->isFieldInfo())
                  return  mFieldFont;

               if (item->hasCrcError())
                  return mResponseCrcErrorFont;

               if (item->hasParityError())
                  return mResponseParityErrorFont;

               return  mResponseDefaultFont;
            }
         }
      }
   }

   if (role == Qt::TextColorRole)
   {
      switch (col)
      {
         case Columns::Data:
         {
            if (item->isResponseFrame())
            {
               return QColor(Qt::darkGray);
            }
         }
      }
   }

   return QVariant();
}

int FrameModel::columnCount(const QModelIndex &parent) const
{
   if (parent.isValid())
      return static_cast<ProtocolFrame*>(parent.internalPointer())->columnCount();

   return mRootFrame->columnCount();
}

Qt::ItemFlags FrameModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
      return Qt::NoItemFlags;

   return QAbstractItemModel::flags(index);
}

QVariant FrameModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return mRootFrame->data(section);

   return QVariant();
}

QModelIndex FrameModel::index(int row, int column, const QModelIndex &parent) const
{
   if (parent.isValid() && parent.column() != 0)
      return QModelIndex();

   ProtocolFrame *parentItem = getItem(parent);
   ProtocolFrame *childItem = parentItem->child(row);

   if (childItem)
      return createIndex(row, column, childItem);

   return QModelIndex();
}

QModelIndex FrameModel::parent(const QModelIndex &index) const
{
   if (!index.isValid())
      return QModelIndex();

   ProtocolFrame *childItem = getItem(index);
   ProtocolFrame *parentItem = childItem->parent();

   if (parentItem == mRootFrame)
      return QModelIndex();

   return createIndex(parentItem->row(), 0, parentItem);
}

bool FrameModel::hasChildren(const QModelIndex &parent) const
{
   ProtocolFrame *parentItem = getItem(parent);

   return parentItem->childCount() > 0;
}

int FrameModel::rowCount(const QModelIndex &parent) const
{
   ProtocolFrame *parentItem = getItem(parent);

   return parentItem->childCount();
}

bool FrameModel::insertRows(int position, int rows, const QModelIndex &parent)
{
   bool success;

   ProtocolFrame *parentItem = getItem(parent);

   beginInsertRows(parent, position, position + rows - 1);
   success = parentItem->insertChilds(position, rows, mRootFrame->columnCount());
   endInsertRows();

   modelChanged();

   return success;
}

bool FrameModel::canFetchMore(const QModelIndex &parent) const
{
   return mIterator.hasNext();
}

void FrameModel::fetchMore(const QModelIndex &parent)
{
   if (mIterator.hasNext())
   {
      beginInsertRows(QModelIndex(), mRootFrame->childCount(), mRootFrame->childCount());

      ProtocolFrame *last = mRootFrame->child(mRootFrame->childCount() - 1);

      while (mIterator.hasNext())
      {
         ProtocolFrame *next = mProtocolParser->parse(mIterator.next());

         if (mGroupRepeated && last && compare(last, next))
         {
            last->addRepeated(1);
         }
         else
         {
            mRootFrame->appendChild(next);

            last = next;
         }
      }

      endInsertRows();
   }
}

void FrameModel::resetModel()
{
   beginResetModel();
   mIterator.reset();
   mRootFrame->clearChilds();
   mProtocolParser->reset();
   endResetModel();
}

QModelIndexList FrameModel::modelRange(double from, double to)
{
   QModelIndexList list;

   for (int i = 0; i < mRootFrame->childCount(); i++)
   {
      ProtocolFrame *frame = mRootFrame->child(i);

      if (frame)
      {
         double time = frame->data(ProtocolFrame::Time).toDouble();

         if (time >= from && time <= to)
         {
            list.append(index(i, 0));
         }
      }
   }

   return list;
}

void FrameModel::setGroupRepeated(bool value)
{
   mGroupRepeated = value;
}

ProtocolFrame *FrameModel::getItem(const QModelIndex &index) const
{
   if (!index.isValid())
      return mRootFrame;

   return static_cast<ProtocolFrame*>(index.internalPointer());
}

QString FrameModel::toString(const QByteArray &value) const
{
   QString text;

   for (QByteArray::ConstIterator it = value.begin(); it != value.end(); it++)
   {
      text.append(QString("%1 ").arg(*it & 0xff, 2, 16, QLatin1Char('0')));
   }

   return text.trimmed();
}

QString FrameModel::padding(int count, QVariant data) const
{
   QString result = "";

   while(count--) result.append("  ");

   return result + data.toString();
}

bool FrameModel::compare(ProtocolFrame *a, ProtocolFrame *b)
{
   QVariant va = a->data(Columns::Data).toByteArray();
   QVariant vb = b->data(Columns::Data).toByteArray();

   return va == vb;
}



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

#include <QFont>
#include <QLabel>
#include <QDebug>

#include <nfc/NfcFrame.h>

#include <protocol/ProtocolParser.h>
#include <protocol/ProtocolFrame.h>

#include "ParserModel.h"

struct ParserModel::Impl
{
   // root node
   ProtocolFrame *root;

   // protocol parser
   ProtocolParser *parser;

   // fonts
   QFont defaultFont;
   QFont requestDefaultFont;
   QFont responseDefaultFont;
   QFont fieldFont;

   Impl() : root(nullptr), parser(new ProtocolParser())
   {
      QVector<QVariant> rootData;

      rootData << "Name" << "" << "Data";

      // root data
      root = new ProtocolFrame(rootData, 0, nullptr);

      // request fonts
      requestDefaultFont.setBold(true);

      // response fonts
      responseDefaultFont.setBold(true);

      // frame fields font
      fieldFont.setItalic(true);
   }

   ~Impl()
   {
      delete parser;
      delete root;
   }

   QString toString(const QByteArray &value) const
   {
      QString text;

      for (char it: value)
      {
         text.append(QString("%1 ").arg(it & 0xff, 2, 16, QLatin1Char('0')));
      }

      return text.trimmed();
   }

   static QString padding(int count, const QVariant &data)
   {
      QString result = "";

      while (--count > 0)
         result.append("   ");

      return result + data.toString();
   }
};

ParserModel::ParserModel(QObject *parent) : QAbstractItemModel(parent), impl(new Impl)
{
}

QVariant ParserModel::data(const QModelIndex &index, int role) const
{
   if (!index.isValid())
      return QVariant();

   int col = index.column();

   auto frame = static_cast<ProtocolFrame *>(index.internalPointer());

   if (role == Qt::UserRole)
   {
      switch (col)
      {
         case Columns::Name:
            return frame->data(ProtocolFrame::Name);

         case Columns::Flags:
            return frame->data(ProtocolFrame::Flags);

         case Columns::Data:
            return frame->data(ProtocolFrame::Data);
      }
   }

   else if (role == Qt::DisplayRole)
   {
      switch (col)
      {
         case Columns::Name:
         {
            return frame->data(ProtocolFrame::Name);
         }

         case Columns::Flags:
         {
            return frame->data(ProtocolFrame::Flags);
         }

         case Columns::Data:
         {
            QVariant info = frame->data(ProtocolFrame::Data);

            if (info.type() == QVariant::ByteArray)
            {
               QString flags;

               if (frame->hasCrcError())
                  flags += " [ECRC]";

               if (frame->hasParityError())
                  flags += " [EPAR]";

               if (frame->hasSyncError())
                  flags += " [ESYNC]";

               return impl->padding(frame->childDeep(), impl->toString(info.toByteArray()) + flags);
            }

            return impl->padding(frame->childDeep(), info);
         }
      }
   }

   else if (role == Qt::FontRole)
   {
      switch (col)
      {
         case Columns::Name:
         {
            if (frame->isFrameField())
               return impl->fieldFont;

            break;
         }

         case Columns::Data:
         {
            if (frame->isRequestFrame())
            {
               if (frame->isFieldInfo())
                  return impl->fieldFont;

               return impl->requestDefaultFont;
            }
            else
            {
               if (frame->isFieldInfo())
                  return impl->fieldFont;

               return impl->responseDefaultFont;
            }
         }
      }
   }

   return QVariant();
}

int ParserModel::columnCount(const QModelIndex &parent) const
{
   if (!parent.isValid())
      return impl->root->columnCount();

   return static_cast<ProtocolFrame *>(parent.internalPointer())->columnCount();
}

Qt::ItemFlags ParserModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
      return Qt::NoItemFlags;

   return {Qt::ItemIsEnabled | Qt::ItemIsSelectable};
}

QVariant ParserModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return impl->root->data(section);

   return QVariant();
}

QModelIndex ParserModel::index(int row, int column, const QModelIndex &parent) const
{
   if (!hasIndex(row, column, parent))
      return QModelIndex();

   auto parentFrame = !parent.isValid() ? impl->root : static_cast<ProtocolFrame *>(parent.internalPointer());

   if (ProtocolFrame *childFrame = parentFrame->child(row))
   {
      return createIndex(row, column, childFrame);
   }

   return QModelIndex();
}

QModelIndex ParserModel::parent(const QModelIndex &index) const
{
   if (!index.isValid())
      return QModelIndex();

   // get current frame for given index
   auto indexFrame = static_cast<ProtocolFrame *>(index.internalPointer());

   // get parent frame, null for root node
   if (ProtocolFrame *parentFrame = indexFrame->parent())
   {
      return createIndex(parentFrame->row(), 0, parentFrame);
   }

   return QModelIndex();
}

bool ParserModel::hasChildren(const QModelIndex &parent) const
{
   if (!parent.isValid())
      return impl->root->childCount() > 0;

   return static_cast<ProtocolFrame *>(parent.internalPointer())->childCount() > 0;
}

int ParserModel::rowCount(const QModelIndex &parent) const
{
   if (!parent.isValid())
      return impl->root->childCount();

   return static_cast<ProtocolFrame *>(parent.internalPointer())->childCount();
}

bool ParserModel::insertRows(int position, int rows, const QModelIndex &parent)
{
   auto parentFrame = !parent.isValid() ? impl->root : static_cast<ProtocolFrame *>(parent.internalPointer());

   beginInsertRows(parent, position, position + rows - 1);
   bool success = parentFrame->insertChilds(position, rows, impl->root->columnCount());
   endInsertRows();

   return success;
}

void ParserModel::resetModel()
{
   beginResetModel();
   impl->root->clearChilds();
   endResetModel();
}

void ParserModel::append(const nfc::NfcFrame &frame)
{
   if (auto child = impl->parser->parse(frame))
   {
      beginInsertRows(QModelIndex(), 0, 0);
      impl->root->appendChild(child);
      endInsertRows();
   }
}

ProtocolFrame *ParserModel::entry(const QModelIndex &index) const
{
   if (!index.isValid())
      return nullptr;

   return static_cast<ProtocolFrame *>(index.internalPointer());
}


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

#include <lab/data/RawFrame.h>

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

      rootData << tr("Name") << "" << tr("Data");

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
      return {};

   int col = index.column();

   auto frame = static_cast<ProtocolFrame *>(index.internalPointer());

   if (role == Qt::UserRole)
   {
      switch (col)
      {
         case Name:
            return frame->data(ProtocolFrame::Name);

         case Flags:
            return frame->data(ProtocolFrame::Flags);

         case Data:
            return frame->data(ProtocolFrame::Data);
      }

      return {};
   }

   if (role == Qt::DisplayRole)
   {
      switch (col)
      {
         case Name:
            return frame->data(ProtocolFrame::Name);

         case Flags:
            return frame->data(ProtocolFrame::Flags);

         case Data:
         {
            QVariant info = frame->data(ProtocolFrame::Data);

            if (info.typeId() == QMetaType::QByteArray)
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

      return {};
   }

   if (role == Qt::FontRole)
   {
      switch (col)
      {
         case Name:
         {
            if (frame->isFrameField())
               return impl->fieldFont;

            break;
         }

         case Data:
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

      return {};
   }

   if (role == Qt::SizeHintRole)
   {
      return QSize(0, 20);
   }

   return {};
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

   return {};
}

QModelIndex ParserModel::index(int row, int column, const QModelIndex &parent) const
{
   if (!hasIndex(row, column, parent))
      return {};

   auto parentFrame = !parent.isValid() ? impl->root : static_cast<ProtocolFrame *>(parent.internalPointer());

   if (ProtocolFrame *childFrame = parentFrame->child(row))
   {
      return createIndex(row, column, childFrame);
   }

   return {};
}

QModelIndex ParserModel::parent(const QModelIndex &index) const
{
   if (!index.isValid())
      return {};

   // get current frame for given index
   auto indexFrame = static_cast<ProtocolFrame *>(index.internalPointer());

   // get parent frame, null for root node
   if (ProtocolFrame *parentFrame = indexFrame->parent())
   {
      return createIndex(parentFrame->row(), 0, parentFrame);
   }

   return {};
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
   bool success = parentFrame->insertChild(position, rows, impl->root->columnCount());
   endInsertRows();

   return success;
}

void ParserModel::resetModel()
{
   beginResetModel();
   impl->root->clearChilds();
   endResetModel();
}

void ParserModel::append(const lab::RawFrame &frame)
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

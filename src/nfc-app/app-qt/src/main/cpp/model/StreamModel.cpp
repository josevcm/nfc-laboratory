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

#include <QFont>
#include <QLabel>
#include <QQueue>
#include <QReadLocker>

#include <nfc/NfcFrame.h>

#include "StreamModel.h"

static QMap<int, QString> NfcACmd = {
      {0x1B, "PWD_AUTH"}, // MIFARE Ultralight EV1
      {0x26, "REQA"}, // ISO/IEC 14443
      {0x30, "READ"}, // MIFARE Ultralight EV1
      {0x39, "READ_CNT"}, // MIFARE Ultralight EV1
      {0x3A, "FAST_READ"}, // MIFARE Ultralight EV1
      {0x3C, "READ_SIG"}, // MIFARE Ultralight EV1
      {0x3E, "TEARING"}, // MIFARE Ultralight EV1
      {0x4B, "VCSL"}, // MIFARE Ultralight EV1
      {0x50, "HLTA"}, // ISO/IEC 14443
      {0x52, "WUPA"}, // ISO/IEC 14443
      {0x60, "AUTH"}, // MIFARE Classic
      {0x61, "AUTH"},// MIFARE Classic EV1
      {0x93, "SEL1"}, // ISO/IEC 14443
      {0x95, "SEL2"}, // ISO/IEC 14443
      {0x97, "SEL3"}, // ISO/IEC 14443
      {0xA0, "COMP_WRITE"}, // MIFARE Ultralight EV1
      {0xA2, "WRITE"}, // MIFARE Ultralight EV1
      {0xA5, "INCR_CNT"}, // MIFARE Ultralight EV1
      {0xE0, "RATS"}
};

static QMap<int, QString> NfcBCmd = {
      {0x05, "REQB"},
      {0x1d, "ATTRIB"},
      {0x50, "HLTB"}
};

struct StreamModel::Impl
{
   // fonts
   QFont defaultFont;
   QFont requestDefaultFont;
   QFont responseDefaultFont;

   // table header
   QVector<QString> headers;

   // frame list
   QList<nfc::NfcFrame *> frames;

   // frame stream
   QQueue<nfc::NfcFrame> stream;

   // stream lock
   QReadWriteLock lock;

   Impl()
   {
      headers << "#" << "Time" << "Delta" << "Rate" << "Type" << "Cmd" << "" << "Frame";

      // request fonts
      requestDefaultFont.setBold(true);

      // response fonts
      responseDefaultFont.setItalic(true);
   }

   ~Impl()
   {
      qDeleteAll(frames);
   }

   inline static QString frameTime(const nfc::NfcFrame *frame)
   {
      return QString("%1").arg(frame->timeStart(), 9, 'f', 5);
   }

   inline static QString frameDelta(const nfc::NfcFrame *frame, const nfc::NfcFrame *prev)
   {
      if (!prev)
         return "";

      double elapsed = frame->timeStart() - prev->timeEnd();

      if (elapsed < 1E-3)
         return QString("%1 us").arg(elapsed * 1000000, 3, 'f', 0);

      if (elapsed < 1)
         return QString("%1 ms").arg(elapsed * 1000, 3, 'f', 0);

      return QString("%1 s").arg(elapsed, 3, 'f', 0);
   }

   inline static QString frameRate(const nfc::NfcFrame *frame)
   {
      return QString("%1k").arg(double(frame->frameRate() / 1000.0f), 3, 'f', 0);
   }

   inline static QString frameTech(const nfc::NfcFrame *frame)
   {
      if (frame->isNfcA())
         return "NfcA";

      if (frame->isNfcB())
         return "NfcB";

      if (frame->isNfcF())
         return "NfcF";

      if (frame->isNfcV())
         return "NfcV";

      return {};
   }

   inline static QString frameCmd(const nfc::NfcFrame *frame)
   {
      if (frame->isPollFrame())
      {
         int command = (*frame)[0];

         // raw protocol commands
         if (!frame->isEncrypted())
         {
            if (frame->isNfcA())
            {
               if (NfcACmd.contains(command))
                  return NfcACmd[command];

               // Protocol Parameter Selection
               if ((command & 0xF0) == 0xD0)
                  return "PPS";
            }
            else if (frame->isNfcB())
            {
               if (NfcBCmd.contains(command))
                  return NfcBCmd[command];
            }

            // ISO-DEP protocol I-Block
            if ((command & 0xE2) == 0x02)
               return "I-Block";

            // ISO-DEP protocol R-Block
            if ((command & 0xE6) == 0xA2)
               return "R-Block";

            // ISO-DEP protocol S-Block
            if ((command & 0xC7) == 0xC2)
               return "S-Block";
         }
      }

      return {};
   }

   inline static int frameFlags(const nfc::NfcFrame *frame)
   {
      return frame->frameFlags() << 8 | frame->frameType();
   }

   inline static QString frameData(const nfc::NfcFrame *frame)
   {
      QString text;

      for (int i = 0; i < frame->available(); i++)
      {
         text.append(QString("%1 ").arg((*frame)[i], 2, 16, QLatin1Char('0')));
      }

      if (!frame->isEncrypted())
      {
         if (frame->hasCrcError())
            text.append("[ECRC]");

         if (frame->hasParityError())
            text.append("[EPAR]");
      }

      return text.trimmed();
   }
};

StreamModel::StreamModel(QObject *parent) : QAbstractTableModel(parent), impl(new Impl)
{
}

int StreamModel::rowCount(const QModelIndex &parent) const
{
   return impl->frames.size();
}

int StreamModel::columnCount(const QModelIndex &parent) const
{
   return impl->headers.size();
}

QVariant StreamModel::data(const QModelIndex &index, int role) const
{
   if (!index.isValid() || index.row() >= impl->frames.size() || index.row() < 0)
      return {};

   nfc::NfcFrame *prev = nullptr;

   auto frame = impl->frames.at(index.row());

   if (index.row() > 0)
      prev = impl->frames.at(index.row() - 1);

   if (role == Qt::DisplayRole || role == Qt::UserRole)
   {
      switch (index.column())
      {
         case Columns::Id:
            return index.row();

         case Columns::Time:
            return impl->frameTime(frame);

         case Columns::Delta:
            return impl->frameDelta(frame, prev);

         case Columns::Rate:
            return impl->frameRate(frame);

         case Columns::Tech:
            return impl->frameTech(frame);

         case Columns::Cmd:
            return impl->frameCmd(frame);

         case Columns::Flags:
            return impl->frameFlags(frame);

         case Columns::Data:
            return impl->frameData(frame);
      }

      return {};
   }

   else if (role == Qt::FontRole)
   {
      switch (index.column())
      {
         case Columns::Data:
         {
            if (frame->isPollFrame())
               return impl->requestDefaultFont;

            if (frame->isListenFrame())
               return impl->responseDefaultFont;
         }
      }
   }

   else if (role == Qt::TextColorRole)
   {
      if (index.column() == Columns::Data)
      {
         if (frame->isListenFrame())
            return QColor(Qt::darkGray);
      }
   }

   else if (role == Qt::TextAlignmentRole)
   {
      switch (index.column())
      {
         case Columns::Id:
         case Columns::Time:
         case Columns::Delta:
            return Qt::AlignRight;

         case Columns::Rate:
            return Qt::AlignCenter;
      }

      return Qt::AlignLeft;
   }

   return {};
}

Qt::ItemFlags StreamModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
      return Qt::NoItemFlags;

   return {Qt::ItemIsEnabled | Qt::ItemIsSelectable};
}

QVariant StreamModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return impl->headers.value(section);

   return {};
}

QModelIndex StreamModel::index(int row, int column, const QModelIndex &parent) const
{
   if (!hasIndex(row, column, parent))
      return {};

   return createIndex(row, column, impl->frames[row]);
}

bool StreamModel::canFetchMore(const QModelIndex &parent) const
{
   QReadLocker locker(&impl->lock);

   return impl->stream.size() > 0;
}

void StreamModel::fetchMore(const QModelIndex &parent)
{
   QReadLocker locker(&impl->lock);

   beginInsertRows(QModelIndex(), impl->frames.size(), impl->frames.size() + impl->stream.size() - 1);

   while (!impl->stream.isEmpty())
   {
      impl->frames.append(new nfc::NfcFrame(impl->stream.dequeue()));
   }

   endInsertRows();
}

void StreamModel::resetModel()
{
   QWriteLocker locker(&impl->lock);

   beginResetModel();
   qDeleteAll(impl->frames);
   impl->frames.clear();
   endResetModel();
}

QModelIndexList StreamModel::modelRange(double from, double to)
{
   QModelIndexList list;

   for (int i = 0; i < impl->frames.size(); i++)
   {
      auto frame = impl->frames.at(i);

      if (frame->timeStart() >= from && frame->timeEnd() <= to)
      {
         list.append(index(i, 0));
      }
   }

   return list;
}

void StreamModel::append(const nfc::NfcFrame &frame)
{
   QWriteLocker locker(&impl->lock);

   impl->stream.enqueue(frame);
}

nfc::NfcFrame *StreamModel::frame(const QModelIndex &index) const
{
   if (!index.isValid())
      return nullptr;

   return static_cast<nfc::NfcFrame *>(index.internalPointer());
}


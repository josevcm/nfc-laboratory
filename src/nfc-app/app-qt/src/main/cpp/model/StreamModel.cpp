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

#include <QDebug>

#include <QFont>
#include <QLabel>
#include <QQueue>
#include <QDateTime>
#include <QReadLocker>

#include <lab/data/RawFrame.h>

#include "StreamModel.h"

static QMap<int, QString> ISO7816APcb = {
   // R-Block commands
   {0x80, "R(ACK)"}, //  R-ACK
   {0x90, "R(NACK)"}, //  R-NACK
   {0x91, "R(NACK)"}, //  R-ERROR
   {0x92, "R(NACK)"}, //  R-ERROR

   // S-Block commands
   {0xC0, "S(RESYNC)"}, //  RESYNC request
   {0xE0, "S(RESYNC)"}, //  RESYNC request
   {0xC1, "S(IFS)"}, //  IFS request
   {0xE1, "S(IFS)"}, //  IFS response
   {0xC2, "S(ABORT)"}, //  IFS request
   {0xE2, "S(ABORT)"}, //  IFS response
   {0xC3, "S(WTX)"}, //  WTX request
   {0xE3, "S(WTX)"}, //  WTX response
};

static QMap<int, QString> NfcACmd = {
   {0x1A, "AUTH"}, // MIFARE Ultralight C authentication
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
   {0x61, "AUTH"}, // MIFARE Classic EV1
   {0x6A, "VASUP-A"}, // ECP protocol
   {0x93, "SEL1"}, // ISO/IEC 14443
   {0x95, "SEL2"}, // ISO/IEC 14443
   {0x97, "SEL3"}, // ISO/IEC 14443
   {0xA0, "COMP_WRITE"}, // MIFARE Ultralight EV1
   {0xA2, "WRITE"}, // MIFARE Ultralight EV1
   {0xA5, "INCR_CNT"}, // MIFARE Ultralight EV1
   {0xE0, "RATS"}
};

static QMap<int, QString> NfcAResp = {
   {0x26, "ATQA"},
   {0x52, "ATQA"}
};

static QMap<int, QString> NfcBCmd = {
   {0x05, "REQB"},
   {0x06, "INIT"},
   {0x08, "READ"},
   {0x09, "WRITE"},
   {0x0B, "GET UID"},
   {0x0E, "SELECT"},
   {0x1d, "ATTRIB"},
   {0x50, "HLTB"}
};

static QMap<int, QString> NfcBResp = {
   {0x05, "ATQB"},
};

static QMap<int, QString> NfcFCmd = {
   {0x00, "REQC"},
};

static QMap<int, QString> NfcFResp = {
   {0x00, "ATQC"},
};

static QMap<int, QString> NfcVCmd = {
   {0x01, "Inventory"},
   {0x02, "StayQuiet"},
   {0x20, "ReadBlock"},
   {0x21, "WriteBlock"},
   {0x22, "LockBlock"},
   {0x23, "ReadBlocks"},
   {0x24, "WriteBlocks"},
   {0x25, "Select"},
   {0x26, "Reset"},
   {0x27, "WriteAFI"},
   {0x28, "LockAFI"},
   {0x29, "WriteDSFID"},
   {0x2a, "LockDSFID"},
   {0x2b, "SysInfo"},
   {0x2c, "GetSecurity"}
};

struct StreamModel::Impl
{
   // time format
   int timeSource = StreamModel::Elapsed;

   // fonts
   QFont defaultFont;
   QFont requestDefaultFont;
   QFont responseDefaultFont;

   // table header
   QVector<QString> headers;

   // column tooltips
   QVector<QString> tooltips;

   // column types
   QVector<QMetaType::Type> types;

   // frame list
   QList<lab::RawFrame> frames;

   // frame stream
   QQueue<lab::RawFrame> stream;

   // stream lock
   QReadWriteLock lock;

   explicit Impl()
   {
      // setup column names
      headers << tr("#");
      headers << tr("Time");
      headers << tr("Delta");
      headers << tr("Rate");
      headers << tr("Type");
      headers << tr("Event");
      headers << tr("Origin");
      headers << tr("Frame");

      // setup column tooltips
      tooltips << tr("Frame sequence number");
      tooltips << tr("Start time of frame");
      tooltips << tr("Time between two consecutive events");
      tooltips << tr("Protocol symbol rate");
      tooltips << tr("Protocol modulation type");
      tooltips << tr("Protocol event name");
      tooltips << tr("Message origin from");
      tooltips << tr("Raw message data");

      // request fonts
      requestDefaultFont.setBold(true);

      // response fonts
      responseDefaultFont.setItalic(true);
   }

   ~Impl()
   {
   }

   int dataType(int section) const
   {
      switch (section)
      {
         case Id:
            return QMetaType::Int;

         case Time:
            return QMetaType::Double;

         case Delta:
            return QMetaType::Double;

         case Rate:
            return QMetaType::Int;

         case Tech:
         case Event:
            return QMetaType::QString;

         case Flags:
            return QMetaType::QStringList;

         case Data:
            return QMetaType::QByteArray;

         default:
            return QMetaType::UnknownType;
      }
   }

   QVariant dataValue(const QModelIndex &index, const lab::RawFrame &frame, const lab::RawFrame &prev) const
   {
      switch (index.column())
      {
         case Id:
            return index.row();

         case Time:
            return timeSource == Elapsed ? frameTime(frame) : frameStart(frame);

         case Delta:
            return frameDelta(frame, prev);

         case Rate:
            return frameRate(frame);

         case Tech:
            return frameTech(frame);

         case Event:
            return frameEvent(frame, prev);

         case Flags:
            return frameFlags(frame);

         case Data:
            return frameData(frame);
      }

      return {};
   }

   QVariant frameStart(const lab::RawFrame &frame) const
   {
      return QVariant::fromValue(frame.dateTime());
   }

   QVariant frameTime(const lab::RawFrame &frame) const
   {
      return QVariant::fromValue(frame.timeStart());
   }

   QVariant frameDelta(const lab::RawFrame &frame, const lab::RawFrame &prev) const
   {
      if (!prev)
         return {};

      return QVariant::fromValue(frame.timeStart() - prev.timeStart());
   }

   QVariant frameRate(const lab::RawFrame &frame) const
   {
      if (frame.frameType() == lab::FrameType::NfcCarrierOn ||
         frame.frameType() == lab::FrameType::NfcCarrierOff ||
         frame.frameType() == lab::FrameType::IsoVccLow ||
         frame.frameType() == lab::FrameType::IsoVccHigh ||
         frame.frameType() == lab::FrameType::IsoRstLow ||
         frame.frameType() == lab::FrameType::IsoRstHigh)
         return {};

      return QVariant::fromValue(frame.frameRate());
   }

   QVariant frameTech(const lab::RawFrame &frame) const
   {
      if (frame.techType() == lab::FrameTech::NfcATech)
         return "NfcA";

      if (frame.techType() == lab::FrameTech::NfcBTech)
         return "NfcB";

      if (frame.techType() == lab::FrameTech::NfcFTech)
         return "NfcF";

      if (frame.techType() == lab::FrameTech::NfcVTech)
         return "NfcV";

      if (frame.techType() == lab::FrameTech::Iso7816Tech)
         return "ISO7816";

      return {};
   }

   QVariant frameEvent(const lab::RawFrame &frame, const lab::RawFrame &prev) const
   {
      switch (frame.frameType())
      {
         case lab::FrameType::NfcCarrierOn:
            return {"RF-On"};

         case lab::FrameType::NfcCarrierOff:
            return {"RF-Off"};

         case lab::FrameType::IsoVccLow:
            return {"VCC-Low"};

         case lab::FrameType::IsoVccHigh:
            return {"VCC-High"};

         case lab::FrameType::IsoRstLow:
            return {"RST-Low"};

         case lab::FrameType::IsoRstHigh:
            return {"RST-High"};

         default:
            break;
      }

      switch (frame.techType())
      {
         case lab::FrameTech::NfcATech:
            return eventNfcA(frame, prev);

         case lab::FrameTech::NfcBTech:
            return eventNfcB(frame, prev);

         case lab::FrameTech::NfcFTech:
            return eventNfcF(frame, prev);

         case lab::FrameTech::NfcVTech:
            return eventNfcV(frame, prev);

         case lab::FrameTech::Iso7816Tech:
            return eventIso7816(frame, prev);

         default:
            break;
      }

      return {};
   }

   QVariant frameFlags(const lab::RawFrame &frame) const
   {
      QStringList flags;

      if (frame.frameType() == lab::FrameType::IsoVccLow)
         flags.append("vcc-low");

      if (frame.frameType() == lab::FrameType::IsoVccHigh)
         flags.append("vcc-high");

      if (frame.frameType() == lab::FrameType::IsoRstLow)
         flags.append("rst-low");

      if (frame.frameType() == lab::FrameType::IsoRstHigh)
         flags.append("rst-high");

      if (frame.frameType() == lab::FrameType::IsoATRFrame)
         flags.append("startup");

      if (frame.frameType() == lab::FrameType::IsoRequestFrame)
         flags.append("request");

      if (frame.frameType() == lab::FrameType::IsoResponseFrame)
         flags.append("response");

      if (frame.frameType() == lab::FrameType::IsoExchangeFrame)
         flags.append("exchange");

      if (frame.frameType() == lab::FrameType::NfcPollFrame)
         flags.append("request");

      if (frame.frameType() == lab::FrameType::NfcListenFrame)
         flags.append("response");

      if (frame.frameType() == lab::FrameType::NfcCarrierOn)
         flags.append("carrier-on");

      if (frame.frameType() == lab::FrameType::NfcCarrierOff)
         flags.append("carrier-off");

      if (frame.hasFrameFlags(lab::FrameFlags::Encrypted))
         flags.append("encrypted");

      if (frame.hasFrameFlags(lab::FrameFlags::Truncated))
         flags.append("truncated");

      if (frame.hasFrameFlags(lab::FrameFlags::CrcError))
         flags.append("crc-error");

      if (frame.hasFrameFlags(lab::FrameFlags::ParityError))
         flags.append("parity-error");

      if (frame.hasFrameFlags(lab::FrameFlags::SyncError))
         flags.append("sync-error");

      return QVariant::fromValue(flags);
   }

   QVariant frameData(const lab::RawFrame &frame) const
   {
      QByteArray data;

      for (int i = 0; i < frame.limit(); i++)
      {
         data.append(frame[i]);
      }

      return data;
   }

   static QString eventNfcA(const lab::RawFrame &frame, const lab::RawFrame &prev)
   {
      QString result;

      // skip encrypted frames
      if (frame.hasFrameFlags(lab::FrameFlags::Encrypted))
         return {};

      if (frame.frameType() == lab::FrameType::NfcPollFrame)
      {
         int command = frame[0];

         // Protocol Parameter Selection
         if (command == 0x50 && frame.limit() == 4)
            return "HALT";

         // Protocol Parameter Selection
         if ((command & 0xF0) == 0xD0 && frame.limit() == 5)
            return "PPS";

         result = eventIsoDep(frame);
         if (!result.isEmpty())
            return result;

         if (NfcACmd.contains(command))
            return NfcACmd[command];
      }
      else if (prev && prev.frameType() == lab::FrameType::NfcPollFrame)
      {
         int command = prev[0];

         if (command == 0x93 || command == 0x95 || command == 0x97)
         {
            if (frame.limit() == 3)
               return "SAK";

            if (frame.limit() == 5)
               return "UID";
         }

         if (command == 0xE0 && frame[0] == (frame.limit() - 2))
            return "ATS";

         result = eventIsoDep(frame);
         if (!result.isEmpty())
            return result;

         if (NfcAResp.contains(command))
            return NfcAResp[command];
      }

      return {};
   }

   static QString eventNfcB(const lab::RawFrame &frame, const lab::RawFrame &prev)
   {
      QString result;

      if (frame.frameType() == lab::FrameType::NfcPollFrame)
      {
         int command = frame[0];

         result = eventIsoDep(frame);
         if (!result.isEmpty())
            return result;

         if (NfcBCmd.contains(command))
            return NfcBCmd[command];
      }
      else if (frame.frameType() == lab::FrameType::NfcListenFrame)
      {
         int command = frame[0];

         result = eventIsoDep(frame);
         if (!result.isEmpty())
            return result;

         if (NfcBResp.contains(command))
            return NfcBResp[command];
      }

      return {};
   }

   static QString eventNfcF(const lab::RawFrame &frame, const lab::RawFrame &prev)
   {
      int command = frame[1];

      if (frame.frameType() == lab::FrameType::NfcPollFrame)
      {
         if (NfcFCmd.contains(command))
            return NfcFCmd[command];

         return QString("CMD %1").arg(command, 2, 16, QChar('0'));
      }

      if (frame.frameType() == lab::FrameType::NfcListenFrame)
      {
         if (NfcFResp.contains(command))
            return NfcFResp[command];
      }

      return {};
   }

   static QString eventNfcV(const lab::RawFrame &frame, const lab::RawFrame &prev)
   {
      if (frame.frameType() == lab::FrameType::NfcPollFrame)
      {
         int command = frame[1];

         if (NfcVCmd.contains(command))
            return NfcVCmd[command];

         return QString("CMD %1").arg(command, 2, 16, QChar('0'));
      }

      return {};
   }

   static QString eventIso7816(const lab::RawFrame &frame, const lab::RawFrame &prev)
   {
      if (frame.frameType() == lab::FrameType::IsoATRFrame)
         return {"ATR"};

      if (frame.frameType() == lab::FrameType::IsoExchangeFrame)
         return "TPDU";

      int nad = frame[0];
      int pcb = frame[1];

      if (nad == 0xff)
         return "PPS";

      // check for commond PCB Values
      if (ISO7816APcb.contains(pcb))
         return ISO7816APcb[pcb];

      // ISO-DEP protocol I-Block
      if ((pcb & 0x80) == 0x00 && frame.limit() >= 4)
         return "I-Block";

      // I-Block protocol
      if ((pcb & 0xC0) == 0x80 && frame.limit() >= 4)
         return "R-Block";

      // ISO-DEP protocol S-Block
      if ((pcb & 0xC0) == 0xC0 && frame.limit() >= 4)
         return "S-Block";

      return {};
   }

   static QString eventIsoDep(const lab::RawFrame &frame)
   {
      int command = frame[0];

      // ISO-DEP protocol S(Deselect)
      if ((command & 0xF7) == 0xC2 && frame.limit() >= 3 && frame.limit() <= 4)
         return "S(Deselect)";

      // ISO-DEP protocol S(WTX)
      if ((command & 0xF7) == 0xF2 && frame.limit() >= 3 && frame.limit() <= 4)
         return "S(WTX)";

      // ISO-DEP protocol R(ACK)
      if ((command & 0xF6) == 0xA2 && frame.limit() == 3)
         return "R(ACK)";

      // ISO-DEP protocol R(NACK)
      if ((command & 0xF6) == 0xB2 && frame.limit() == 3)
         return "R(NACK)";

      // ISO-DEP protocol I-Block
      if ((command & 0xE6) == 0x02 && frame.limit() >= 4)
         return "I-Block";

      // ISO-DEP protocol R-Block
      if ((command & 0xE6) == 0xA2 && frame.limit() == 3)
         return "R-Block";

      // ISO-DEP protocol S-Block
      if ((command & 0xC7) == 0xC2 && frame.limit() >= 3 && frame.limit() <= 4)
         return "S-Block";

      return {};
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

   lab::RawFrame prev;

   auto frame = impl->frames.at(index.row());

   if (index.row() > 0)
      prev = impl->frames.at(index.row() - 1);

   if (role == Qt::DisplayRole)
      return impl->dataValue(index, frame, prev);

   if (role == Qt::FontRole)
   {
      switch (index.column())
      {
         case Data:
         {
            if (frame.frameType() == lab::FrameType::NfcPollFrame || frame.frameType() == lab::FrameType::IsoRequestFrame || frame.frameType() == lab::FrameType::IsoExchangeFrame)
               return impl->requestDefaultFont;

            if (frame.frameType() == lab::FrameType::NfcListenFrame || frame.frameType() == lab::FrameType::IsoResponseFrame)
               return impl->responseDefaultFont;

            break;
         }

         case Event:
         {
            if (frame.frameType() == lab::FrameType::NfcListenFrame)
               return impl->responseDefaultFont;
         }
      }

      return impl->defaultFont;
   }

   if (role == Qt::ForegroundRole)
   {
      switch (index.column())
      {
         case Event:
         case Data:
            if (frame.frameType() == lab::FrameType::NfcListenFrame)
               return QColor(Qt::darkGray);
      }

      return {};
   }

   if (role == Qt::TextAlignmentRole)
   {
      switch (index.column())
      {
         case Time:
         case Delta:
            return int(Qt::AlignRight | Qt::AlignVCenter);

         case Id:
         case Tech:
         case Rate:
         case Event:
            return int(Qt::AlignHCenter | Qt::AlignVCenter);
      }

      return int(Qt::AlignLeft | Qt::AlignVCenter);
   }

   if (role == Qt::SizeHintRole)
   {
      return QSize(0, 20);
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
   switch (role)
   {
      case Qt::DisplayRole:
         return impl->headers.value(section);

      case Qt::ToolTipRole:
         return impl->tooltips.value(section);

      case Qt::UserRole:
         return impl->dataType(section);

      default:
         break;
   }

   return {};
}

QModelIndex StreamModel::index(int row, int column, const QModelIndex &parent) const
{
   if (!hasIndex(row, column, parent))
      return {};

   return createIndex(row, column, &impl->frames[row]);
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
      lab::RawFrame frame = impl->stream.dequeue();

      // find insertion point
      auto it = std::lower_bound(impl->frames.begin(), impl->frames.end(), frame);

      // insert frame sorted by time
      impl->frames.insert(it - impl->frames.begin(), frame);
   }

   endInsertRows();
}

void StreamModel::resetModel()
{
   beginResetModel();
   impl->frames.clear();
   endResetModel();
}

QModelIndexList StreamModel::modelRange(double from, double to)
{
   QModelIndexList list;

   for (int i = 0; i < impl->frames.size(); i++)
   {
      auto frame = impl->frames.at(i);

      if (frame.timeStart() < to && frame.timeEnd() > from)
      {
         list.append(index(i, 0));
      }
   }

   return list;
}

void StreamModel::append(const lab::RawFrame &frame)
{
   QWriteLocker locker(&impl->lock);

   impl->stream.enqueue(frame);
}

const lab::RawFrame *StreamModel::frame(const QModelIndex &index) const
{
   if (!index.isValid())
      return nullptr;

   return static_cast<lab::RawFrame *>(index.internalPointer());
}

int StreamModel::timeSource() const
{
   return impl->timeSource;
}

void StreamModel::setTimeSource(TimeSource timeSource)
{
   impl->timeSource = timeSource;

   modelChanged();
}

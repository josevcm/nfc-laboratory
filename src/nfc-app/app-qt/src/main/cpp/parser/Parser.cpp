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

#include <parser/Parser.h>

ProtocolFrame *Parser::buildRootInfo(const QString &name, const lab::RawFrame &frame, int flags)
{
   QVector<QVariant> values;

   // handle NFC flags
   flags |= frame.frameType() == lab::FrameType::NfcPollFrame ? ProtocolFrame::Flags::RequestFrame : 0;
   flags |= frame.frameType() == lab::FrameType::NfcListenFrame ? ProtocolFrame::Flags::ResponseFrame : 0;

   // handle ISO flags
   flags |= frame.frameType() == lab::FrameType::IsoATRFrame ? ProtocolFrame::Flags::StartupFrame : 0;
   flags |= frame.frameType() == lab::FrameType::IsoExchangeFrame ? ProtocolFrame::Flags::RequestFrame | ProtocolFrame::Flags::ResponseFrame : 0;
   flags |= frame.frameType() == lab::FrameType::IsoRequestFrame ? ProtocolFrame::Flags::RequestFrame : 0;
   flags |= frame.frameType() == lab::FrameType::IsoResponseFrame ? ProtocolFrame::Flags::ResponseFrame : 0;

   // handle error flags
   flags |= frame.hasFrameFlags(lab::FrameFlags::CrcError) ? ProtocolFrame::Flags::CrcError : 0;
   flags |= frame.hasFrameFlags(lab::FrameFlags::ParityError) ? ProtocolFrame::Flags::ParityError : 0;

   // frame values
   values << QVariant::fromValue(name);
   values << QVariant::fromValue(flags);
   values << QVariant::fromValue(toByteArray(frame));

   return new ProtocolFrame(values, flags, frame);
}

ProtocolFrame *Parser::buildChildInfo(const QVariant &info)
{
   return buildChildInfo("", info, ProtocolFrame::FieldInfo, -1, 0);
}

ProtocolFrame *Parser::buildChildInfo(const QString &name, const QVariant &info)
{
   return buildChildInfo(name, info, ProtocolFrame::FieldInfo, -1, 0);
}

ProtocolFrame *Parser::buildChildInfo(const QString &name, const lab::RawFrame &frame, int start, int length)
{
   int from = start < 0 ? frame.limit() + start : start;

   return buildChildInfo(name, toByteArray(frame, from, length), ProtocolFrame::FrameField, from, length);
}

ProtocolFrame *Parser::buildChildInfo(const QString &name, const QVariant &info, int start, int length)
{
   return buildChildInfo(name, info, ProtocolFrame::FrameField, start, length);
}

ProtocolFrame *Parser::buildChildInfo(const QString &name, const QVariant &info, int flags, int start, int length)
{
   QVector<QVariant> values;

   values << QVariant::fromValue(name);
   values << QVariant::fromValue(flags);
   values << info;

   return new ProtocolFrame(values, flags, nullptr, start, start + length - 1);
}

QByteArray Parser::toByteArray(const lab::RawFrame &frame, int from, int length)
{
   QByteArray data;

   if (length > frame.limit())
      length = frame.limit();

   if (from >= 0)
   {
      for (int i = from; i < frame.limit() && length > 0; i++, length--)
      {
         data.append(frame[i]);
      }
   }
   else
   {
      for (int i = frame.limit() + from; i < frame.limit() && length > 0; i++, length--)
      {
         data.append(frame[i]);
      }
   }

   return data;
}

QString Parser::toString(const QByteArray &array)
{
   QString text;

   for (unsigned char value: array)
   {
      if (value >= 0x20 && value <= 0x7f)
         text.append((QChar)value);
      else
         text.append(".");
   }

   return "[" + text + "]";
}

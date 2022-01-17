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

#include <parser/ParserNfc.h>

ProtocolFrame *ParserNfc::parse(const nfc::NfcFrame &frame)
{
   if (frame.isPollFrame())
      return parseRequestUnknown(frame);

   if (frame.isListenFrame())
      return parseResponseUnknown(frame);

   return nullptr;
}

void ParserNfc::reset()
{
   lastCommand = 0;
}

ProtocolFrame *ParserNfc::parseRequestUnknown(const nfc::NfcFrame &frame)
{
   int flags = frame.hasParityError() ? ProtocolFrame::Flags::ParityError : 0;

   return buildFrameInfo("(unk)", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), flags, 0);
}

ProtocolFrame *ParserNfc::parseResponseUnknown(const nfc::NfcFrame &frame)
{
   int flags = frame.hasParityError() ? ProtocolFrame::Flags::ParityError : 0;

   return buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), flags, 0);
}

ProtocolFrame *ParserNfc::parseAPDU(const QString &name, const QByteArray &data)
{
   int lc = (unsigned char) data[4];

   ProtocolFrame *info = buildFieldInfo(name, data);

   info->appendChild(buildFieldInfo("CLA", data.mid(0, 1)));
   info->appendChild(buildFieldInfo("INS", data.mid(1, 1)));
   info->appendChild(buildFieldInfo("P1", data.mid(2, 1)));
   info->appendChild(buildFieldInfo("P2", data.mid(3, 1)));
   info->appendChild(buildFieldInfo("LC", data.mid(4, 1)));

   if (data.length() > lc + 5)
      info->appendChild(buildFieldInfo("LE", data.mid(data.length() - 1, 1)));

   if (lc > 0)
   {
      QByteArray array = data.mid(5, lc);

      ProtocolFrame *payload = buildFieldInfo("DATA", array);

      payload->appendChild(buildFieldInfo(toString(array)));

      info->appendChild(payload);
   }

   return info;
}

ProtocolFrame *ParserNfc::buildFrameInfo(const QString &name, int rate, const QVariant &info, double time, double end, int flags, int type)
{
   return buildInfo(name, rate, info, time, end, flags | ProtocolFrame::RequestFrame, type);
}

ProtocolFrame *ParserNfc::buildFrameInfo(int rate, const QVariant &info, double time, double end, int flags, int type)
{
   return buildInfo(QString(), rate, info, time, end, flags | ProtocolFrame::ResponseFrame, type);
}

ProtocolFrame *ParserNfc::buildFieldInfo(const QString &name, const QVariant &info)
{
   return buildInfo(name, 0, info, 0, 0, ProtocolFrame::FrameField, 0);
}

ProtocolFrame *ParserNfc::buildFieldInfo(const QVariant &info)
{
   return buildInfo(QString(), 0, info, 0, 0, ProtocolFrame::FieldInfo, 0);
}

ProtocolFrame *ParserNfc::buildInfo(const QString &name, int rate, const QVariant &info, double start, double end, int flags, int type)
{
   QVector<QVariant> values;

   // frame number
   values << QVariant::Invalid;

   // time start
   if (start > 0)
      values << QVariant::fromValue(start);
   else
      values << QVariant::Invalid;

   // time end
   if (end > 0)
      values << QVariant::fromValue(end);
   else
      values << QVariant::Invalid;

   // rate
   if (rate > 0)
      values << QVariant::fromValue(rate);
   else
      values << QVariant::Invalid;

   // type
   if (!name.isEmpty())
      values << QVariant::fromValue(name);
   else
      values << QVariant::Invalid;

   // flags
   values << QVariant::fromValue(flags);

   // content
   values << info;

   return new ProtocolFrame(values, flags, type);
}

bool ParserNfc::isApdu(const QByteArray &apdu)
{
   // all APDUs contains at least CLA INS P1 P2 and LE / LC field
   if (apdu.length() < 5)
      return false;

   // get apdu length
   int lc = apdu[4];

   // APDU length must be at least 5 + lc bytes
   if (apdu.length() < lc + 5)
      return false;

   // APDU length must be up to 6 + lc bytes (extra le byte)
   if (apdu.length() > lc + 6)
      return false;

   return true;
}

QByteArray ParserNfc::toByteArray(const nfc::NfcFrame &frame, int from, int length)
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

QString ParserNfc::toString(const QByteArray &array)
{
   QString text;

   for (unsigned char value: array)
   {
      if (value >= 0x20 && value <= 0x7f)
         text.append(value);
      else
         text.append(".");
   }

   return "[" + text + "]";
}

void ParserNfcIsoDep::reset()
{
   ParserNfc::reset();
}

ProtocolFrame *ParserNfcIsoDep::parse(const nfc::NfcFrame &frame)
{
   ProtocolFrame *info = nullptr;

   if (frame.isPollFrame())
   {
      do
      {
         // ISO-DEP protocol I-Block
         if ((info = parseRequestIBlock(frame)))
            break;

         // ISO-DEP protocol R-Block
         if ((info = parseRequestRBlock(frame)))
            break;

         // ISO-DEP protocol S-Block
         if ((info = parseRequestSBlock(frame)))
            break;

         // Unknown frame...
         info = ParserNfc::parse(frame);

      } while (false);
   }
   else if (frame.isListenFrame())
   {
      do
      {
         // ISO-DEP protocol I-Block
         if ((info = parseResponseIBlock(frame)))
            break;

         // ISO-DEP protocol R-Block
         if ((info = parseResponseRBlock(frame)))
            break;

         // ISO-DEP protocol S-Block
         if ((info = parseResponseSBlock(frame)))
            break;

         // Unknown frame...
         info = ParserNfc::parse(frame);

      } while (false);
   }

   return info;
}

ProtocolFrame *ParserNfcIsoDep::parseRequestIBlock(const nfc::NfcFrame &frame)
{
   if ((frame[0] & 0xE2) != 0x02 || frame.limit() < 5)
      return nullptr;

   int pcb = frame[0], offset = 1;
   int flags = 0;

   flags |= frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0;
   flags |= frame.hasParityError() ? ProtocolFrame::Flags::ParityError : 0;

   ProtocolFrame *root = buildFrameInfo("I-Block", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::ApplicationFrame);

   if (pcb & 0x08)
      root->appendChild(buildFieldInfo("CID", frame[offset++] & 0x0F));

   if (pcb & 0x04)
      root->appendChild(buildFieldInfo("NAD", frame[offset++] & 0xFF));

   if (offset < frame.limit() - 2)
   {
      QByteArray data = toByteArray(frame, offset, frame.limit() - offset - 2);

      if (isApdu(data))
      {
         flags |= IS_APDU;

         root->appendChild(parseAPDU("APDU", data));
      }
      else
      {
         if (ProtocolFrame *payload = root->appendChild(buildFieldInfo("DATA", data)))
         {
            payload->appendChild(buildFieldInfo(toString(data)));
         }
      }
   }

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcIsoDep::parseResponseIBlock(const nfc::NfcFrame &frame)
{
   if ((lastCommand & 0xE2) != 0x02)
      return nullptr;

   int pcb = frame[0], offset = 1;
   int flags = 0;

   flags |= frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0;
   flags |= frame.hasParityError() ? ProtocolFrame::Flags::ParityError : 0;

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::ApplicationFrame);

   if (pcb & 0x08)
      root->appendChild(buildFieldInfo("CID", frame[offset++] & 0x0F));

   if (pcb & 0x04)
      root->appendChild(buildFieldInfo("NAD", frame[offset++] & 0xFF));

   if (offset < frame.limit() - 2)
   {
      if (flags & IS_APDU)
      {
         if (offset < frame.limit() - 4)
         {
            QByteArray data = toByteArray(frame, offset, frame.limit() - offset - 4);

            if (ProtocolFrame *payload = root->appendChild(buildFieldInfo("DATA", data)))
            {
               payload->appendChild(buildFieldInfo(toString(data)));
            }
         }

         root->appendChild(buildFieldInfo("SW", toByteArray(frame, -4, 2)));
      }
      else
      {
         QByteArray data = toByteArray(frame, offset, frame.limit() - offset - 2);

         if (ProtocolFrame *payload = root->appendChild(buildFieldInfo("DATA", data)))
         {
            payload->appendChild(buildFieldInfo(toString(data)));
         }
      }
   }

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcIsoDep::parseRequestRBlock(const nfc::NfcFrame &frame)
{
   if ((frame[0] & 0xE6) != 0xA2 || frame.limit() != 3)
      return nullptr;

   int pcb = frame[0], offset = 1;
   int flags = 0;

   ProtocolFrame *root;

   flags |= frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0;
   flags |= frame.hasParityError() ? ProtocolFrame::Flags::ParityError : 0;

   if (pcb & 0x10)
      root = buildFrameInfo("R(NACK)", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::ApplicationFrame);
   else
      root = buildFrameInfo("R(ACK)", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::ApplicationFrame);

   if (pcb & 0x08)
      root->appendChild(buildFieldInfo("CID", frame[offset++] & 0x0F));

   if (offset < frame.limit() - 2)
      root->appendChild(buildFieldInfo("INF", toByteArray(frame, offset, frame.limit() - offset - 2)));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcIsoDep::parseResponseRBlock(const nfc::NfcFrame &frame)
{
   if ((lastCommand & 0xE6) != 0xA2)
      return nullptr;

   int flags = frame.hasParityError() ? ProtocolFrame::Flags::ParityError : 0;

   return buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::ApplicationFrame);
}

ProtocolFrame *ParserNfcIsoDep::parseRequestSBlock(const nfc::NfcFrame &frame)
{
   if ((frame[0] & 0xC7) != 0xC2 || frame.limit() != 4)
      return nullptr;

   int pcb = frame[0], offset = 1;
   int flags = 0;

   flags |= frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0;
   flags |= frame.hasParityError() ? ProtocolFrame::Flags::ParityError : 0;

   ProtocolFrame *root = buildFrameInfo("S-Block", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::ApplicationFrame);

   if (pcb & 0x08)
      root->appendChild(buildFieldInfo("CID", frame[offset++] & 0x0F));

   if (offset < frame.limit() - 2)
      root->appendChild(buildFieldInfo("INF", toByteArray(frame, offset, frame.limit() - offset - 2)));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcIsoDep::parseResponseSBlock(const nfc::NfcFrame &frame)
{
   if ((lastCommand & 0xC7) != 0xC2)
      return nullptr;

   int flags = frame.hasParityError() ? ProtocolFrame::Flags::ParityError : 0;

   return buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::ApplicationFrame);
}


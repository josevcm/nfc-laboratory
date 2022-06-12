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
   return buildRootInfo("(unk)", frame, 0);
}

ProtocolFrame *ParserNfc::parseResponseUnknown(const nfc::NfcFrame &frame)
{
   return buildRootInfo("", frame, 0);
}

ProtocolFrame *ParserNfc::parseAPDU(const QString &name, const nfc::NfcFrame &frame, int start, int length)
{
   int lc = (unsigned char) frame[start + 4];

   ProtocolFrame *info = buildChildInfo(name, frame, start, length);

   info->appendChild(buildChildInfo("CLA", frame, start + 0, 1));
   info->appendChild(buildChildInfo("INS", frame, start + 1, 1));
   info->appendChild(buildChildInfo("P1", frame, start + 2, 1));
   info->appendChild(buildChildInfo("P2", frame, start + 3, 1));
   info->appendChild(buildChildInfo("LC", frame, start + 4, 1));

   if (length > lc + 5)
      info->appendChild(buildChildInfo("LE", frame, start + length - 1, 1));

   if (lc > 0)
      info->appendChild(buildChildInfo("DATA", frame, start + 5, lc));

   return info;
}

ProtocolFrame *ParserNfc::buildRootInfo(const QString &name, const nfc::NfcFrame &frame, int flags)
{
   QVector<QVariant> values;

   flags |= frame.isPollFrame() ? ProtocolFrame::Flags::RequestFrame : 0;
   flags |= frame.isListenFrame() ? ProtocolFrame::Flags::ResponseFrame : 0;
   flags |= frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0;
   flags |= frame.hasParityError() ? ProtocolFrame::Flags::ParityError : 0;

   // frame number
   values << QVariant::fromValue(name);
   values << QVariant::fromValue(flags);
   values << QVariant::fromValue(toByteArray(frame));

   return new ProtocolFrame(values, flags, frame);
}

ProtocolFrame *ParserNfc::buildChildInfo(const QVariant &info)
{
   return buildChildInfo("", info, ProtocolFrame::FieldInfo, -1, 0);
}

ProtocolFrame *ParserNfc::buildChildInfo(const QString &name, const QVariant &info)
{
   return buildChildInfo(name, info, ProtocolFrame::FieldInfo, -1, 0);
}

ProtocolFrame *ParserNfc::buildChildInfo(const QString &name, const nfc::NfcFrame &frame, int start, int length)
{
   int from = start < 0 ? frame.limit() + start : start;

   return buildChildInfo(name, toByteArray(frame, from, length), ProtocolFrame::FrameField, from, length);
}

ProtocolFrame *ParserNfc::buildChildInfo(const QString &name, const QVariant &info, int start, int length)
{
   return buildChildInfo(name, info, ProtocolFrame::FrameField, start, length);
}

ProtocolFrame *ParserNfc::buildChildInfo(const QString &name, const QVariant &info, int flags, int start, int length)
{
   QVector<QVariant> values;

   values << QVariant::fromValue(name);
   values << QVariant::fromValue(flags);
   values << info;

   return new ProtocolFrame(values, flags, nullptr, start, start + length - 1);
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

   if (frame.isPollFrame() || frame.isListenFrame())
   {
      do
      {
         if (!frame.isEncrypted())
         {
            // ISO-DEP protocol I-Block
            if ((info = parseIBlock(frame)))
               break;

            // ISO-DEP protocol R-Block
            if ((info = parseRBlock(frame)))
               break;

            // ISO-DEP protocol S-Block
            if ((info = parseSBlock(frame)))
               break;
         }

         // Unknown frame...
         info = ParserNfc::parse(frame);

      } while (false);
   }

   return info;
}

ProtocolFrame *ParserNfcIsoDep::parseIBlock(const nfc::NfcFrame &frame)
{
   int pcb = frame[0], offset = 1;

   if ((pcb & 0xE2) != 0x02 || frame.limit() < 4)
      return nullptr;

   ProtocolFrame *root = buildRootInfo("I-Block", frame, ProtocolFrame::ApplicationFrame);

   if (ProtocolFrame *pcbf = root->appendChild(buildChildInfo("PCB", frame, 0, 1)))
   {
      pcbf->appendChild(buildChildInfo("[00....1.] I-Block"));

      if ((pcb & 0x10) == 0x00)
         pcbf->appendChild(buildChildInfo("[...0....] NO Chaining"));
      else
         pcbf->appendChild(buildChildInfo("[...1....] Frame chaining"));

      if ((pcb & 0x08) == 0x00)
         pcbf->appendChild(buildChildInfo("[....0...] NO CID following"));
      else
         pcbf->appendChild(buildChildInfo("[....1...] CID following"));

      if ((pcb & 0x04) == 0x00)
         pcbf->appendChild(buildChildInfo("[.....0..] NO NAD following"));
      else
         pcbf->appendChild(buildChildInfo("[.....1..] NAD following"));

      if ((pcb & 0x01) == 0x00)
         pcbf->appendChild(buildChildInfo("[.......0] Block number"));
      else
         pcbf->appendChild(buildChildInfo("[.......1] Block number"));
   }

   if (pcb & 0x08)
   {
      root->appendChild(buildChildInfo("CID", frame[offset] & 0x0F, offset, 1));
      offset++;
   }

   if (pcb & 0x04)
   {
      root->appendChild(buildChildInfo("NAD", frame[offset] & 0xFF, offset, 1));
      offset++;
   }

   if (offset < frame.limit() - 2)
   {
      QByteArray data = toByteArray(frame, offset, frame.limit() - offset - 2);

      if (isApdu(data))
      {
         root->appendChild(parseAPDU("APDU", frame, offset, frame.limit() - offset - 2));
      }
      else
      {
         root->appendChild(buildChildInfo("DATA", data, offset, frame.limit() - offset - 2));
      }
   }

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcIsoDep::parseRBlock(const nfc::NfcFrame &frame)
{
   int pcb = frame[0], offset = 1;

   if ((pcb & 0xE6) != 0xA2 || frame.limit() != 3)
      return nullptr;

   ProtocolFrame *root;

   if (pcb & 0x10)
      root = buildRootInfo("R(NACK)", frame, ProtocolFrame::ApplicationFrame);
   else
      root = buildRootInfo("R(ACK)", frame, ProtocolFrame::ApplicationFrame);

   if (ProtocolFrame *pcbf = root->appendChild(buildChildInfo("PCB", frame, 0, 1)))
   {
      pcbf->appendChild(buildChildInfo("[101..01.] R-Block"));

      if ((pcb & 0x10) == 0x00)
         pcbf->appendChild(buildChildInfo("[...0....] ACK"));
      else
         pcbf->appendChild(buildChildInfo("[...1....] NACK"));

      if ((pcb & 0x08) == 0x00)
         pcbf->appendChild(buildChildInfo("[....0...] NO CID following"));
      else
         pcbf->appendChild(buildChildInfo("[....1...] CID following"));

      if ((pcb & 0x01) == 0x00)
         pcbf->appendChild(buildChildInfo("[.......0] Block number"));
      else
         pcbf->appendChild(buildChildInfo("[.......1] Block number"));
   }

   if (pcb & 0x08)
   {
      root->appendChild(buildChildInfo("CID", frame[offset] & 0x0F, offset, 1));
      offset++;
   }

   if (offset < frame.limit() - 2)
      root->appendChild(buildChildInfo("INF", frame, offset, frame.limit() - offset - 2));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcIsoDep::parseSBlock(const nfc::NfcFrame &frame)
{
   int pcb = frame[0], offset = 1;

   if ((pcb & 0xC7) != 0xC2 || frame.limit() < 3 || frame.limit() > 4)
      return nullptr;

   ProtocolFrame *root = buildRootInfo("S-Block", frame, ProtocolFrame::ApplicationFrame);

   if (ProtocolFrame *pcbf = root->appendChild(buildChildInfo("PCB", frame, 0, 1)))
   {
      pcbf->appendChild(buildChildInfo("[11...010] S-Block"));

      if ((pcb & 0x30) == 0x00)
         pcbf->appendChild(buildChildInfo("[..00....] DESELECT"));
      else if ((pcb & 0x30) == 0x30)
         pcbf->appendChild(buildChildInfo("[..11....] WTX"));

      if ((pcb & 0x08) == 0x00)
         pcbf->appendChild(buildChildInfo("[....0...] NO CID following"));
      else
         pcbf->appendChild(buildChildInfo("[....1...] CID following"));
   }

   if (pcb & 0x08)
   {
      root->appendChild(buildChildInfo("CID", frame[offset] & 0x0F, offset, 1));
      offset++;
   }

   if (offset < frame.limit() - 2)
      root->appendChild(buildChildInfo("INF", frame, offset, frame.limit() - offset - 2));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

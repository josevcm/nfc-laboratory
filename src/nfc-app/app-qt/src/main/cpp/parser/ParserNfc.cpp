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

#include <lab/nfc/Nfc.h>

#include <parser/ParserNfc.h>

ProtocolFrame *ParserNfc::parse(const lab::RawFrame &frame)
{
   if (frame.frameType() == lab::FrameType::NfcPollFrame)
      return parseRequestUnknown(frame);

   if (frame.frameType() == lab::FrameType::NfcListenFrame)
      return parseResponseUnknown(frame);

   return nullptr;
}

void ParserNfc::reset()
{
   lastCommand = 0;
}

ProtocolFrame *ParserNfc::parseRequestUnknown(const lab::RawFrame &frame)
{
   return buildRootInfo("(unk)", frame, 0);
}

ProtocolFrame *ParserNfc::parseResponseUnknown(const lab::RawFrame &frame)
{
   return buildRootInfo("", frame, 0);
}

ProtocolFrame *ParserNfc::parseAPDU(const QString &name, const lab::RawFrame &frame, int start, int length)
{
   int lc = (unsigned char)frame[start + 4];

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

void ParserNfcIsoDep::reset()
{
   ParserNfc::reset();
}

ProtocolFrame *ParserNfcIsoDep::parse(const lab::RawFrame &frame)
{
   ProtocolFrame *info = nullptr;

   if (frame.frameType() == lab::FrameType::NfcPollFrame || frame.frameType() == lab::FrameType::NfcListenFrame)
   {
      do
      {
         if (!frame.hasFrameFlags(lab::FrameFlags::Encrypted))
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

      }
      while (false);
   }

   return info;
}

ProtocolFrame *ParserNfcIsoDep::parseIBlock(const lab::RawFrame &frame)
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

ProtocolFrame *ParserNfcIsoDep::parseRBlock(const lab::RawFrame &frame)
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

      pcbf->appendChild(buildChildInfo(QString("[......%1] Sequence number, %2").arg(pcb & 1, 1, 2, QChar('0')).arg(pcb & 1)));
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

ProtocolFrame *ParserNfcIsoDep::parseSBlock(const lab::RawFrame &frame)
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
         pcbf->appendChild(buildChildInfo("[..11....] WTX (waiting time extension block)"));

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

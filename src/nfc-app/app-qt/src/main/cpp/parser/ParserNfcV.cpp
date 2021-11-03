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

#include <nfc/Nfc.h>

#include <parser/ParserNfcV.h>

void ParserNfcV::reset()
{
   ParserNfc::reset();
}

ProtocolFrame *ParserNfcV::parse(const nfc::NfcFrame &frame)
{
   ProtocolFrame *info = nullptr;

   if (frame.isPollFrame())
   {
      do
      {
         // Inventory Command
         if ((info = parseRequestInventory(frame)))
            break;

//         // Attrib request
//         if ((info = parseRequestATTRIB(frame)))
//            break;
//
//         // Halt request
//         if ((info = parseRequestHLTB(frame)))
//            break;

         // generic NFC-V request frame...
         info = parseRequestGeneric(frame);

      } while (false);

      lastCommand = frame[1];
   }
   else
   {
      do
      {
         // Inventory response
         if ((info = parseResponseInventory(frame)))
            break;

//         // Attrib request
//         if ((info = parseResponseATTRIB(frame)))
//            break;
//
//         // Halt request
//         if ((info = parseResponseHLTB(frame)))
//            break;

         // generic NFC-V response frame...
         info = parseResponseGeneric(frame);

      } while (false);
   }

   return info;
}

ProtocolFrame *ParserNfcV::parseRequestInventory(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x01)
      return nullptr;

   int offset = 0;
   int flags = frame[offset++];

   ProtocolFrame *root = buildFrameInfo("Inventory", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::SenseFrame);

   parseRequestFlags(root, frame);

   // skip command code
   offset++;

   // AFI flag is set
   if ((flags & 0x14) == 0x14)
   {
      int afi = frame[offset++];

      if (ProtocolFrame *afif = root->appendChild(buildFieldInfo("AFI", QString("%1").arg(afi, 2, 16, QChar('0')))))
      {
         if (afi == 0x00)
            afif->appendChild(buildFieldInfo("[00000000] All families and sub-families"));
         else if ((afi & 0x0f) == 0x00)
            afif->appendChild(buildFieldInfo(QString("[%10000] All sub-families of family %2").arg(afi >> 4, 4, 2, QChar('0')).arg(afi >> 4)));
         else if ((afi & 0xf0) == 0x00)
            afif->appendChild(buildFieldInfo(QString("[0000%1] Proprietary sub-family %2 only").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else if ((afi & 0xf0) == 0x10)
            afif->appendChild(buildFieldInfo(QString("[0001%1] Transport sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else if ((afi & 0xf0) == 0x20)
            afif->appendChild(buildFieldInfo(QString("[0010%1] Financial sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else if ((afi & 0xf0) == 0x30)
            afif->appendChild(buildFieldInfo(QString("[0011%1] Identification sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else if ((afi & 0xf0) == 0x40)
            afif->appendChild(buildFieldInfo(QString("[0100%1] Telecommunication sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else if ((afi & 0xf0) == 0x50)
            afif->appendChild(buildFieldInfo(QString("[0101%1] Medical sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else if ((afi & 0xf0) == 0x60)
            afif->appendChild(buildFieldInfo(QString("[0110%1] Multimedia sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else if ((afi & 0xf0) == 0x70)
            afif->appendChild(buildFieldInfo(QString("[0111%1] Gaming sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else if ((afi & 0xf0) == 0x80)
            afif->appendChild(buildFieldInfo(QString("[1000%1] Data Storage sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else if ((afi & 0xf0) == 0x90)
            afif->appendChild(buildFieldInfo(QString("[1001%1] Item management sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else if ((afi & 0xf0) == 0xA0)
            afif->appendChild(buildFieldInfo(QString("[1010%1] Express parcels sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else if ((afi & 0xf0) == 0xB0)
            afif->appendChild(buildFieldInfo(QString("[1011%1] Postal services sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else if ((afi & 0xf0) == 0xC0)
            afif->appendChild(buildFieldInfo(QString("[1100%1] Airline bags sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
         else
            afif->appendChild(buildFieldInfo(QString("[%1] RFU %2").arg(afi, 8, 2, QChar('0')).arg(afi)));
      }
   }

   int mlen = frame[offset++];

   root->appendChild(buildFieldInfo("MLEN", QString("%1").arg(mlen, 2, 16, QChar('0'))));

   if (mlen > 0)
   {
      root->appendChild(buildFieldInfo("MASK", toByteArray(frame, offset, (mlen >> 3) + (mlen & 0x7) ? 1 : 0)));
   }

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseInventory(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x01)
      return nullptr;

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::SenseFrame);

   parseResponseFlags(root, frame);

   root->appendChild(buildFieldInfo("DSFID", QString("%1").arg(frame[1], 2, 16, QChar('0'))));
   root->appendChild(buildFieldInfo("UID", toByteArray(frame, 2, 8)));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseRequestGeneric(const nfc::NfcFrame &frame)
{
   int cmd = frame[1]; // frame command

   ProtocolFrame *root = buildFrameInfo(QString("CMD %1").arg(cmd, 2, 16, QChar('0')), frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   parseResponseFlags(root, frame);

   root->appendChild(buildFieldInfo("CODE", QString("%1 [%2]").arg(cmd, 2, 16, QChar('0')).arg(cmd, 8, 2, QChar('0'))));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseGeneric(const nfc::NfcFrame &frame)
{
   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   parseResponseFlags(root, frame);

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseRequestFlags(ProtocolFrame *root, const nfc::NfcFrame &frame)
{
   int rqf = frame[0]; // request frame flags

   if (ProtocolFrame *afrf = root->appendChild(buildFieldInfo("FLAGS", QString("%1").arg(rqf, 2, 16, QChar('0')))))
   {
      if (rqf & 0x01)
         afrf->appendChild(buildFieldInfo("[.......1] Two sub-carriers shall be used by the VICC"));
      else
         afrf->appendChild(buildFieldInfo("[.......0] A single sub-carrier frequency shall be used by the VICC"));

      if (rqf & 0x02)
         afrf->appendChild(buildFieldInfo("[......1.] High data rate shall be used"));
      else
         afrf->appendChild(buildFieldInfo("[......0.] Low data rate shall be used"));

      if (rqf & 0x08)
         afrf->appendChild(buildFieldInfo("[....1...] Protocol format is extended"));
      else
         afrf->appendChild(buildFieldInfo("[....0...] No protocol format extension"));

      if (rqf & 0x04)
      {
         if (rqf & 0x10)
            afrf->appendChild(buildFieldInfo("[...1.1..] AFI field is present"));
         else
            afrf->appendChild(buildFieldInfo("[...0.1..] AFI field is not present"));

         if (rqf & 0x20)
            afrf->appendChild(buildFieldInfo("[..1..1..] 1 slot"));
         else
            afrf->appendChild(buildFieldInfo("[..0..1..] 16 slots"));
      }
      else
      {
         if (rqf & 0x10)
            afrf->appendChild(buildFieldInfo("[...1.0..] Request shall be executed only by VICC in selected state"));
         else
            afrf->appendChild(buildFieldInfo("[...0.0..] Request shall be executed by any VICC according to the setting of Address flag"));

         if (rqf & 0x20)
            afrf->appendChild(buildFieldInfo("[..1..0..] Request is addressed. UID field is present. It shall be executed only by the VICC whose UID matches"));
         else
            afrf->appendChild(buildFieldInfo("[..0..0..] Request is not addressed. UID field is not present. It shall be executed by any VICC"));
      }

      afrf->appendChild(buildFieldInfo(QString("[.%1...1..] Custom flag").arg((rqf & 0x40), 1, 2, QChar('0'))));
      afrf->appendChild(buildFieldInfo(QString("[%1....1..] Reserved for future use").arg((rqf & 0x80), 1, 2, QChar('0'))));
   }

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseFlags(ProtocolFrame *root, const nfc::NfcFrame &frame)
{
   int rsf = frame[0]; // response frame flags

   if (ProtocolFrame *afrf = root->appendChild(buildFieldInfo("FLAGS", QString("%1").arg(rsf, 2, 16, QChar('0')))))
   {
      if (rsf & 0x01)
         afrf->appendChild(buildFieldInfo("[.......1] Error detected. Error code is in the error field"));
      else
         afrf->appendChild(buildFieldInfo("[.......0] No error"));

      afrf->appendChild(buildFieldInfo(QString("[.....%1.] Reserved for future use").arg((rsf >> 1) & 0x03, 2, 2, QChar('0'))));

      if (rsf & 0x08)
         afrf->appendChild(buildFieldInfo("[....1...] Protocol format is extended"));
      else
         afrf->appendChild(buildFieldInfo("[....0...] No protocol format extension"));

      afrf->appendChild(buildFieldInfo(QString("[%1....] Reserved for future use").arg((rsf >> 4) & 0x0f, 4, 2, QChar('0'))));
   }

   if (rsf & 0x01)
   {
      int err = frame[1]; // response error code

      if (ProtocolFrame *aerr = root->appendChild(buildFieldInfo("ERROR", QString("%1").arg(err, 2, 16, QChar('0')))))
      {
         if (err == 0x01)
            aerr->appendChild(buildFieldInfo(QString("[%1] The command is not supported").arg(err, 8, 2, QChar('0'))));
         else if (err == 0x02)
            aerr->appendChild(buildFieldInfo(QString("[%1] The command is not recognized").arg(err, 8, 2, QChar('0'))));
         else if (err == 0x0f)
            aerr->appendChild(buildFieldInfo(QString("[%1] Unknown error").arg(err, 8, 2, QChar('0'))));
         else if (err == 0x10)
            aerr->appendChild(buildFieldInfo(QString("[%1] The specified block is not available").arg(err, 8, 2, QChar('0'))));
         else if (err == 0x11)
            aerr->appendChild(buildFieldInfo(QString("[%1] The specified block is already locked").arg(err, 8, 2, QChar('0'))));
         else if (err == 0x12)
            aerr->appendChild(buildFieldInfo(QString("[%1] The specified block is locked and its content cannot be changed").arg(err, 8, 2, QChar('0'))));
         else if (err == 0x13)
            aerr->appendChild(buildFieldInfo(QString("[%1] The specified block was not successfully programmed").arg(err, 8, 2, QChar('0'))));
         else if (err == 0x14)
            aerr->appendChild(buildFieldInfo(QString("[%1] The specified block was not successfully locked").arg(err, 8, 2, QChar('0'))));
         else
            aerr->appendChild(buildFieldInfo(QString("[%1] Custom command error code").arg(err, 8, 2, QChar('0'))));
      }
   }

   return root;
}

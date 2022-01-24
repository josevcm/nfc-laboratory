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

#include <parser/ParserNfcB.h>

void ParserNfcB::reset()
{
   ParserNfcIsoDep::reset();
}

ProtocolFrame *ParserNfcB::parse(const nfc::NfcFrame &frame)
{
   ProtocolFrame *info = nullptr;

   if (frame.isPollFrame())
   {
      do
      {
         // Request Command
         if ((info = parseRequestREQB(frame)))
            break;

         // Attrib request
         if ((info = parseRequestATTRIB(frame)))
            break;

         // Halt request
         if ((info = parseRequestHLTB(frame)))
            break;

         // Unknown frame...
         info = ParserNfcIsoDep::parse(frame);

      } while (false);
   }
   else
   {
      do
      {
         // Request Command
         if ((info = parseResponseREQB(frame)))
            break;

         // Attrib request
         if ((info = parseResponseATTRIB(frame)))
            break;

         // Halt request
         if ((info = parseResponseHLTB(frame)))
            break;

         // Unknown frame...
         info = ParserNfcIsoDep::parse(frame);

      } while (false);

      lastCommand = 0;
   }

   return info;
}

ProtocolFrame *ParserNfcB::parseRequestREQB(const nfc::NfcFrame &frame)
{
   if (frame[0] != 0x05)
      return nullptr;

   lastCommand = frame[0];

   int apf = frame[0];
   int afi = frame[1];
   int param = frame[2];
   int nslot = param & 0x07;

   ProtocolFrame *root = buildRootInfo(param & 0x8 ? "WUPB" : "REQB", frame, ProtocolFrame::SenseFrame);

   if (ProtocolFrame *afif = root->appendChild(buildChildInfo("AFI", frame, 1, 1)))
   {
      if (afi == 0x00)
         afif->appendChild(buildChildInfo("[00000000] All families and sub-families"));
      else if ((afi & 0x0f) == 0x00)
         afif->appendChild(buildChildInfo(QString("[%10000] All sub-families of family %2").arg(afi >> 4, 4, 2, QChar('0')).arg(afi >> 4)));
      else if ((afi & 0xf0) == 0x00)
         afif->appendChild(buildChildInfo(QString("[0000%1] Proprietary sub-family %2 only").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
      else if ((afi & 0xf0) == 0x10)
         afif->appendChild(buildChildInfo(QString("[0001%1] Transport sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
      else if ((afi & 0xf0) == 0x20)
         afif->appendChild(buildChildInfo(QString("[0010%1] Financial sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
      else if ((afi & 0xf0) == 0x30)
         afif->appendChild(buildChildInfo(QString("[0011%1] Identification sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
      else if ((afi & 0xf0) == 0x40)
         afif->appendChild(buildChildInfo(QString("[0100%1] Telecommunication sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
      else if ((afi & 0xf0) == 0x50)
         afif->appendChild(buildChildInfo(QString("[0101%1] Medical sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
      else if ((afi & 0xf0) == 0x60)
         afif->appendChild(buildChildInfo(QString("[0110%1] Multimedia sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
      else if ((afi & 0xf0) == 0x70)
         afif->appendChild(buildChildInfo(QString("[0111%1] Gaming sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
      else if ((afi & 0xf0) == 0x80)
         afif->appendChild(buildChildInfo(QString("[1000%1] Data Storage sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
      else
         afif->appendChild(buildChildInfo(QString("[%1] RFU %2").arg(afi, 8, 2, QChar('0')).arg(afi)));
   }

   if (ProtocolFrame *paramf = root->appendChild(buildChildInfo("PARAM", frame, 2, 1)))
   {
      if (param & 0x8)
         paramf->appendChild(buildChildInfo("[....1...] WUPB command"));
      else
         paramf->appendChild(buildChildInfo("[....0...] REQB command"));

      paramf->appendChild(buildChildInfo(QString("[.....%1] number of slots: %2").arg(nslot, 3, 2, QChar('0')).arg(nfc::NFCB_SLOT_TABLE[nslot])));
   }

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcB::parseResponseREQB(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x05)
      return nullptr;

   int rate = frame[9];
   int fdsi = (frame[10] >> 4) & 0x0f;
   int type = frame[10] & 0x0f;
   int fwi = (frame[11] >> 4) & 0x0f;
   int adc = (frame[11] >> 2) & 0x03;
   int fo = frame[11] & 0x3;
   int fds = nfc::NFC_FDS_TABLE[fdsi];
   float fwt = nfc::NFC_FWT_TABLE[fwi] / nfc::NFC_FC;

   ProtocolFrame *root = buildRootInfo("", frame, ProtocolFrame::SenseFrame);

   root->appendChild(buildChildInfo("PUPI", frame, 1, 4));
   root->appendChild(buildChildInfo("APP", frame, 5, 4));

   if (ProtocolFrame *inff = root->appendChild(buildChildInfo("PROTO", frame, 9, 3)))
   {
      // protocol rate
      if (ProtocolFrame *ratef = inff->appendChild(buildChildInfo("RATE", frame, 9, 1)))
      {
         if (rate & 0x80)
            ratef->appendChild(buildChildInfo(QString("[1.......] only support same rate for both directions")));
         else
            ratef->appendChild(buildChildInfo(QString("[0.......] supported different rates for each direction")));

         if (rate & 0x40)
            ratef->appendChild(buildChildInfo(QString("[.1......] supported 848 kbps PICC to PCD")));

         if (rate & 0x20)
            ratef->appendChild(buildChildInfo(QString("[..1.....] supported 424 kbps PICC to PCD")));

         if (rate & 0x10)
            ratef->appendChild(buildChildInfo(QString("[...1....] supported 212 kbps PICC to PCD")));

         if (rate & 0x04)
            ratef->appendChild(buildChildInfo(QString("[.....1..] supported 848 kbps PCD to PICC")));

         if (rate & 0x02)
            ratef->appendChild(buildChildInfo(QString("[......1.] supported 424 kbps PCD to PICC")));

         if (rate & 0x01)
            ratef->appendChild(buildChildInfo(QString("[.......1] supported 212 kbps PCD to PICC")));

         if ((rate & 0x7f) == 0x00)
            ratef->appendChild(buildChildInfo(QString("[.0000000] only 106 kbps supported")));
      }

      // frame size
      if (ProtocolFrame *protof = inff->appendChild(buildChildInfo("FRAME", frame, 10, 1)))
      {
         protof->appendChild(buildChildInfo(QString("[%1....] maximum frame size, %2 bytes").arg(fdsi, 4, 2, QChar('0')).arg(fds)));

         if (type == 0)
            protof->appendChild(buildChildInfo("[....0000] PICC not compliant with ISO/IEC 14443-4"));
         else if (type == 1)
            protof->appendChild(buildChildInfo("[....0001] PICC compliant with ISO/IEC 14443-4"));
         else
            protof->appendChild(buildChildInfo(QString("[....%1] protocol type %2").arg(type, 4, 2, QChar('0')).arg(type)));
      }

      // other parameters
      if (ProtocolFrame *otherf = inff->appendChild(buildChildInfo("OTHER", frame, 11, 1)))
      {
         otherf->appendChild(buildChildInfo(QString("[%1....] frame waiting time FWT = %2 ms").arg(fwi, 4, 2, QChar('0')).arg(1E3 * fwt, 0, 'f', 2)));

         if (adc == 0)
            otherf->appendChild(buildChildInfo("[....00..] application is proprietary"));
         else if (adc == 1)
            otherf->appendChild(buildChildInfo("[....01..] application is coded in APP field"));
         else
            otherf->appendChild(buildChildInfo(QString("[....%1..] RFU").arg(adc, 2, 2, QChar('0'))));

         if (fo & 0x2)
            otherf->appendChild(buildChildInfo("[......1.] NAD supported by the PICC"));

         if (fo & 0x1)
            otherf->appendChild(buildChildInfo("[.......1] CID supported by the PICC"));
      }
   }

   // frame CRC
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcB::parseRequestATTRIB(const nfc::NfcFrame &frame)
{
   if (frame[0] != 0x1d)
      return nullptr;

   lastCommand = frame[0];

   int flags = 0;
   int param1 = frame[5];
   int param2 = frame[6];
   int param3 = frame[7];
   int param4 = frame[8];

   ProtocolFrame *root = buildRootInfo("ATTRIB", frame, ProtocolFrame::SenseFrame);

   root->appendChild(buildChildInfo("ID", frame, 1, 4));

   if (ProtocolFrame *param1f = root->appendChild(buildChildInfo("PARAM1", frame, 5, 1)))
   {
      int tr0min = (param1 >> 6) & 0x3;
      int tr1min = (param1 >> 4) & 0x3;

      if (tr0min)
         param1f->appendChild(buildChildInfo(QString("[%1.....] minimum TR0, %2 us").arg(tr0min, 2, 2, QChar('0')).arg(1E3 * nfc::NFCB_TR0_MIN_TABLE[tr0min] / nfc::NFC_FC, 0, 'f', 2)));
      else
         param1f->appendChild(buildChildInfo(QString("[%1.....] minimum TR0, DEFAULT").arg(tr0min, 2, 2, QChar('0'))));

      if (tr1min)
         param1f->appendChild(buildChildInfo(QString("[%1.....] minimum TR1, %2 us").arg(tr1min, 2, 2, QChar('0')).arg(1E3 * nfc::NFCB_TR1_MIN_TABLE[tr1min] / nfc::NFC_FC, 0, 'f', 2)));
      else
         param1f->appendChild(buildChildInfo(QString("[%1.....] minimum TR1, DEFAULT").arg(tr1min, 2, 2, QChar('0'))));

      if (param1 & 0x08)
         param1f->appendChild(buildChildInfo(QString("[....1..] suppression of the EOF: Yes")));
      else
         param1f->appendChild(buildChildInfo(QString("[....0..] suppression of the EOF: No")));

      if (param1 & 0x04)
         param1f->appendChild(buildChildInfo(QString("[....1..] suppression of the SOF: Yes")));
      else
         param1f->appendChild(buildChildInfo(QString("[....0..] suppression of the SOF: No")));
   }

   if (ProtocolFrame *param2f = root->appendChild(buildChildInfo("PARAM2", frame, 6, 1)))
   {
      int fdsi = param2 & 0x0f;
      int fds = nfc::NFC_FDS_TABLE[fdsi] / nfc::NFC_FC;

      if ((param2 & 0xC0) == 0x00)
         param2f->appendChild(buildChildInfo("[00......] selected 106 kbps PICC to PCD rate"));
      else if ((param2 & 0xC0) == 0x40)
         param2f->appendChild(buildChildInfo("[01......] selected 212 kbps PICC to PCD rate"));
      else if ((param2 & 0xC0) == 0x80)
         param2f->appendChild(buildChildInfo("[10......] selected 424 kbps PICC to PCD rate"));
      else if ((param2 & 0xC0) == 0xC0)
         param2f->appendChild(buildChildInfo("[11......] selected 848 kbps PICC to PCD rate"));

      if ((param2 & 0x30) == 0x00)
         param2f->appendChild(buildChildInfo("[..00....] selected 106 kbps PCD to PICC rate"));
      else if ((param2 & 0x30) == 0x10)
         param2f->appendChild(buildChildInfo("[..01....] selected 212 kbps PCD to PICC rate"));
      else if ((param2 & 0x30) == 0x20)
         param2f->appendChild(buildChildInfo("[..10....] selected 424 kbps PCD to PICC rate"));
      else if ((param2 & 0x30) == 0x30)
         param2f->appendChild(buildChildInfo("[..11....] selected 848 kbps PCD to PICC rate"));

      param2f->appendChild(buildChildInfo(QString("[....%1] maximum frame size, %2 bytes").arg(fdsi, 4, 2, QChar('0')).arg(fds)));
   }

   if (ProtocolFrame *param3f = root->appendChild(buildChildInfo("PARAM3", frame, 7, 1)))
   {
      if (param3 & 1)
         param3f->appendChild(buildChildInfo("[.......1] PICC compliant with ISO/IEC 14443-4"));
      else
         param3f->appendChild(buildChildInfo("[.......0] PICC not compliant with ISO/IEC 14443-4"));
   }

   if (ProtocolFrame *param4f = root->appendChild(buildChildInfo("PARAM4", frame, 8, 1)))
   {
      int cid = param4 & 0x0f;

      param4f->appendChild(buildChildInfo(QString("[....%1] card identifier (CID) = %2").arg(cid, 4, 2, QChar('0')).arg(cid)));
   }

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcB::parseResponseATTRIB(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x1d)
      return nullptr;

   ProtocolFrame *root = buildRootInfo("", frame, ProtocolFrame::SenseFrame);

   int mbli = (frame[0] >> 4) & 0x0f;
   int cid = frame[0] & 0x0f;

   root->appendChild(buildChildInfo("MBLI", mbli));
   root->appendChild(buildChildInfo("CID", cid));

   if (frame.limit() > 3)
   {
      root->appendChild(buildChildInfo("INF", frame, 1, frame.limit() - 3));
   }

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcB::parseRequestHLTB(const nfc::NfcFrame &frame)
{
   if (frame[0] != 0x50)
      return nullptr;

   lastCommand = frame[0];

   ProtocolFrame *root = buildRootInfo("HLTB", frame, ProtocolFrame::SenseFrame);

   root->appendChild(buildChildInfo("PUPI", frame, 1, 4));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcB::parseResponseHLTB(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x50)
      return nullptr;

   ProtocolFrame *root = buildRootInfo("", frame, ProtocolFrame::SenseFrame);

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

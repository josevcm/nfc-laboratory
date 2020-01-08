/*

  Copyright (c) 2020 Jose Vicente Campos Martinez - <josevcm@gmail.com>

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

#include <QDebug>
#include <QVector>

#include "ProtocolParser.h"
#include "ProtocolFrame.h"

// FSDI to FSD conversion (frame size)
static const int   TABLE_FDS[]   = { 16, 24, 32, 40, 48, 64, 96, 128, 256, 0, 0, 0, 0, 0, 0, 0 };

// Frame waiting time (FWT = (256 x 16 / fc) * 2 ^ FWI)
static const float TABLE_FWT[]  = { 0.000302065, 0.00060413, 0.00120826, 0.002416519, 0.004833038, 0.009666077, 0.019332153, 0.038664307, 0.077328614, 0.154657227, 0.309314454, 0.618628909, 1.237257817, 2.474515634, 4.949031268, 9.898062537 };

// Start-up Frame Guard Time (SFGT = (256 x 16 / fc) * 2 ^ SFGI)
static const float TABLE_SFGT[] = { 0.000302065, 0.00060413, 0.00120826, 0.002416519, 0.004833038, 0.009666077, 0.019332153, 0.038664307, 0.077328614, 0.154657227, 0.309314454, 0.618628909, 1.237257817, 2.474515634, 4.949031268, 9.898062537 };

ProtocolParser::ProtocolParser(QObject *parent) :
   QObject(parent), mCount(1), mChaining(0), mRequest(0)
{
}

ProtocolParser::~ProtocolParser()
{
}

void ProtocolParser::reset()
{
   mCount = 1;
   mChaining = 0;
   mRequest = 0;
}

ProtocolFrame *ProtocolParser::parse(const NfcFrame &frame)
{
   ProtocolFrame *info = nullptr;

   if (frame.isRequestFrame())
   {
      if (mChaining == 0)
      {
         int command = frame[0];

         // Request Command, Type A
         if (command == 0x26)
            info = parseRequestREQA(frame);

         // HALT Command, Type A
         else if (command == 0x50)
            info = parseRequestHLTA(frame);

         // Wake-UP Command, Type A
         else if (command == 0x52)
            info = parseRequestWUPA(frame);

         // Mifare AUTH
         else if (command == 0x60 || command == 0x61)
            info = parseRequestAUTH(frame);

         // Select Command, Type A
         else if (command == 0x93 || command == 0x95 || command == 0x97)
            info = parseRequestSELn(frame);

         // Request for Answer to Select
         else if (command == 0xE0)
            info = parseRequestRATS(frame);

         // Protocol Parameter Selection
         else if ((command & 0xF0) == 0xD0)
            info = parseRequestPPSr(frame);

         // ISO-DEP protocol I-Block
         else if ((command & 0xE2) == 0x02)
            info = parseRequestIBlock(frame);

         // ISO-DEP protocol R-Block
         else if ((command & 0xE6) == 0xA2)
            info = parseRequestRBlock(frame);

         // ISO-DEP protocol S-Block
         else if ((command & 0xC7) == 0xC2)
            info = parseRequestSBlock(frame);

         // Unknow frame...
         else
            info = parseRequestUnknow(frame);

         mRequest = command;
      }

      // Mifare AUTH, two pass
      else if (mChaining == 0x60 || mChaining == 0x61)
      {
         info = parseRequestAUTH(frame);
      }
   }
   else
   {
      int command = mRequest;

      // Request Command, Type A
      if (command == 0x26)
         info = parseResponseREQA(frame);

      // HALT Command, Type A
      else if (command == 0x50)
         info = parseResponseHLTA(frame);

      // Wake-UP Command, Type A
      else if (command == 0x52)
         info = parseResponseWUPA(frame);

      // Mifare AUTH
      else if (command == 0x60 || command == 0x61)
         info = parseResponseAUTH(frame);

      // Select Command, Type A
      else if (command == 0x93 || command == 0x95 || command == 0x97)
         info = parseResponseSELn(frame);

      // Request for Answer to Select
      else if (command == 0xE0)
         info = parseResponseRATS(frame);

      // Protocol Parameter Selection
      else if ((command & 0xF0) == 0xD0)
         info = parseResponsePPSr(frame);

      // ISO-DEP protocol I-Block
      else if ((command & 0xE2) == 0x02)
         info = parseResponseIBlock(frame);

      // ISO-DEP protocol R-Block
      else if ((command & 0xE6) == 0xA2)
         info = parseResponseRBlock(frame);

      // ISO-DEP protocol S-Block
      else if ((command & 0xC7) == 0xC2)
         info = parseResponseSBlock(frame);

      // Unknow frame...
      else
         info = parseResponseUnknow(frame);
   }

   // Inter-frame timming
   if (info && mLast && frame.timeStart() > mLast.timeStart())
      info->set(ProtocolFrame::Elapsed, frame.timeStart() - mLast.timeStart());

   mLast = frame;

   return info;
}

ProtocolFrame *ProtocolParser::parseRequestREQA(const NfcFrame &frame)
{
   int flags = frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;

   return buildFrameInfo("REQA", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SenseFrame);
}

ProtocolFrame *ProtocolParser::parseRequestWUPA(const NfcFrame &frame)
{
   int flags = frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;

   return buildFrameInfo("WUPA", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SenseFrame);
}

ProtocolFrame *ProtocolParser::parseRequestSELn(const NfcFrame &frame)
{
   ProtocolFrame *root = nullptr;

   int cmd = frame[0];
   int nvb = frame[1] >> 4;
   int flags = 0;

   flags |= frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;
   flags |= verifyCrc(frame.toByteArray()) ? ProtocolFrame::Flags::CrcError : 0;

   if (cmd == 0x93)
      root = buildFrameInfo("SEL1", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SelectionFrame);
   else if (cmd == 0x95)
      root = buildFrameInfo("SEL2", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SelectionFrame);
   else if (cmd == 0x97)
      root = buildFrameInfo("SEL3", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SelectionFrame);
   else
      root = buildFrameInfo("SEL?", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SelectionFrame);

   // command detailed info
   root->appendChild(buildFieldInfo("NVB", nvb));

   if (nvb == 7)
   {
      if (frame[2] == 0x88) // cascade tag
      {
         root->appendChild(buildFieldInfo("CT", frame.toByteArray(2, 1)));
         root->appendChild(buildFieldInfo("UID", frame.toByteArray(3, 3)));
      }
      else
      {
         root->appendChild(buildFieldInfo("UID", frame.toByteArray(2, 4)));
      }

      root->appendChild(buildFieldInfo("BCC", frame.toByteArray(6, 1)));
      root->appendChild(buildFieldInfo("CRC", frame.toByteArray(-2)));
   }

   return root;
}

ProtocolFrame *ProtocolParser::parseRequestRATS(const NfcFrame &frame)
{
   int cdi = (frame[1] & 0x0F);
   int fsdi = (frame[1] >> 4 )& 0x0F;
   int flags = 0;

   flags |= frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;
   flags |= verifyCrc(frame.toByteArray()) ? ProtocolFrame::Flags::CrcError : 0;

   ProtocolFrame *root = buildFrameInfo("RATS", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SelectionFrame);

   ProtocolFrame *param = buildFieldInfo("PARAM", frame.toByteArray(1, 1));

   param->appendChild(buildFieldInfo(QString("[....%1] CDI logical channel %2").arg(cdi, 4, 2, QChar('0')).arg(cdi)));
   param->appendChild(buildFieldInfo(QString("[%1....] FSD max frame size %2").arg(fsdi, 4, 2, QChar('0')).arg(TABLE_FDS[fsdi])));

   root->appendChild(param);
   root->appendChild(buildFieldInfo("CRC", frame.toByteArray(-2)));

   return root;
}

ProtocolFrame *ProtocolParser::parseRequestHLTA(const NfcFrame &frame)
{
   int flags = 0;

   flags |= frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;
   flags |= verifyCrc(frame.toByteArray()) ? ProtocolFrame::Flags::CrcError : 0;

   ProtocolFrame *root = buildFrameInfo("HLTA", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SenseFrame);

   root->appendChild(buildFieldInfo("CRC", frame.toByteArray(-2)));

   return root;
}

ProtocolFrame *ProtocolParser::parseRequestPPSr(const NfcFrame &frame)
{
   int pps = frame[0];
   int flags = 0;

   flags |= frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;
   flags |= verifyCrc(frame.toByteArray()) ? ProtocolFrame::Flags::CrcError : 0;

   ProtocolFrame *root = buildFrameInfo("PPS", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SelectionFrame);

   root->appendChild(buildFieldInfo("CID", pps & 0x0F));
   root->appendChild(buildFieldInfo("PPS0", frame.toByteArray(1, 1)));

   int pps0 = frame[1];

   if (pps0 & 0x10)
   {
      ProtocolFrame *pps1f = buildFieldInfo("PPS1", frame.toByteArray(2, 1));

      int pps1 = frame[2];

      if ((pps1 & 0x0C) == 0x00)
         pps1f->appendChild(buildFieldInfo("[....00..] selected 106 kbps PICC to PCD rate"));
      else if ((pps1 & 0x0C) == 0x04)
         pps1f->appendChild(buildFieldInfo("[....01..] selected 212 kbps PICC to PCD rate"));
      else if ((pps1 & 0x0C) == 0x08)
         pps1f->appendChild(buildFieldInfo("[....10..] selected 424 kbps PICC to PCD rate"));
      else if ((pps1 & 0x0C) == 0x0C)
         pps1f->appendChild(buildFieldInfo("[....11..] selected 848 kbps PICC to PCD rate"));

      if ((pps1 & 0x03) == 0x00)
         pps1f->appendChild(buildFieldInfo("[......00] selected 106 kbps PCD to PICC rate"));
      else if ((pps1 & 0x03) == 0x01)
         pps1f->appendChild(buildFieldInfo("[......01] selected 212 kbps PCD to PICC rate"));
      else if ((pps1 & 0x03) == 0x02)
         pps1f->appendChild(buildFieldInfo("[......10] selected 424 kbps PCD to PICC rate"));
      else if ((pps1 & 0x03) == 0x03)
         pps1f->appendChild(buildFieldInfo("[......11] selected 848 kbps PCD to PICC rate"));

      root->appendChild(pps1f);
   }

   root->appendChild(buildFieldInfo("CRC", frame.toByteArray(-2)));

   return root;
}

ProtocolFrame *ProtocolParser::parseRequestAUTH(const NfcFrame &frame)
{
   int flags = frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;

   if (mChaining == 0)
   {
      int cmd = frame[0];
      int block = frame[1];

      flags |= verifyCrc(frame.toByteArray()) ? ProtocolFrame::Flags::CrcError : 0;

      ProtocolFrame *root = buildFrameInfo(cmd == 0x60 ? "AUTH(A)" : "AUTH(B)", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::AuthFrame);

      root->appendChild(buildFieldInfo("BLOCK", block));
      root->appendChild(buildFieldInfo("CRC", frame.toByteArray(-2)));

      mChaining = cmd;

      return root;
   }

   ProtocolFrame *root = buildFrameInfo(mChaining == 0x60 ? "AUTH(A)" : "AUTH(B)", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::AuthFrame);

   root->appendChild(buildFieldInfo("TOKEN", frame.toByteArray()));

   mChaining = 0;

   return root;
}

ProtocolFrame *ProtocolParser::parseRequestIBlock(const NfcFrame &frame)
{
   int pcb = frame[0], offset = 1;
   int flags = 0;

   flags |= frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;
   flags |= verifyCrc(frame.toByteArray()) ? ProtocolFrame::Flags::CrcError : 0;

   ProtocolFrame *root = buildFrameInfo("I-Block", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::InformationFrame);

   if (pcb & 0x08)
      root->appendChild(buildFieldInfo("CID", frame[offset++] & 0x0F));

   if (pcb & 0x04)
      root->appendChild(buildFieldInfo("NAD", frame[offset++] & 0xFF));

   if (offset < frame.length() - 2)
   {
      //      QByteArray data = frame.toByteArray(offset, frame.length() - offset - 2);

      //		if (isApdu(data))
      //			root->appendChild(parseAPDU("APDU", data));
      //		else
      root->appendChild(buildFieldInfo("DATA", frame.toByteArray(offset, frame.length() - offset - 2)));
   }

   root->appendChild(buildFieldInfo("CRC", frame.toByteArray(-2)));

   return root;
}

ProtocolFrame *ProtocolParser::parseRequestRBlock(const NfcFrame &frame)
{
   ProtocolFrame *root;

   int pcb = frame[0], offset = 1;
   int flags = 0;

   flags |= frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;
   flags |= verifyCrc(frame.toByteArray()) ? ProtocolFrame::Flags::CrcError : 0;

   if (pcb & 0x10)
      root = buildFrameInfo("R(NACK)", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::InformationFrame);
   else
      root = buildFrameInfo("R(ACK)", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::InformationFrame);

   if (pcb & 0x08)
      root->appendChild(buildFieldInfo("CID", frame[offset++] & 0x0F));

   if (offset < frame.length() - 2)
      root->appendChild(buildFieldInfo("INF", frame.toByteArray(offset, frame.length() - offset - 2)));

   root->appendChild(buildFieldInfo("CRC", frame.toByteArray(-2)));

   return root;
}

ProtocolFrame *ProtocolParser::parseRequestSBlock(const NfcFrame &frame)
{
   int pcb = frame[0], offset = 1;
   int flags = 0;

   flags |= frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;
   flags |= verifyCrc(frame.toByteArray()) ? ProtocolFrame::Flags::CrcError : 0;

   ProtocolFrame *root = buildFrameInfo("S-Block", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::InformationFrame);

   if (pcb & 0x08)
      root->appendChild(buildFieldInfo("CID", frame[offset++] & 0x0F));

   if (offset < frame.length() - 2)
      root->appendChild(buildFieldInfo("INF", frame.toByteArray(offset, frame.length() - offset - 2)));

   root->appendChild(buildFieldInfo("CRC", frame.toByteArray(-2)));

   return root;
}

ProtocolFrame *ProtocolParser::parseRequestUnknow(const NfcFrame &frame)
{
   return buildFrameInfo("(unk)", frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), frame.isParityError(), 0);
}

ProtocolFrame *ProtocolParser::parseResponseREQA(const NfcFrame &frame)
{
   int offset = 0;
   int uids = frame[offset++];
   int type = frame[offset++];

   int flags = frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;

   ProtocolFrame *atqa = buildFieldInfo("ATQA", frame.toByteArray(0, 2));

   // check UID size
   if ((uids & 0xC0) == 0x00)
      atqa->appendChild(buildFieldInfo("[00......] single size UID"));
   else if ((uids & 0xC0) == 0x40)
      atqa->appendChild(buildFieldInfo("[01......] double size UID"));
   else if ((uids & 0xC0) == 0x80)
      atqa->appendChild(buildFieldInfo("[10......] triple size UID"));
   else if ((uids & 0xC0) == 0xC0)
      atqa->appendChild(buildFieldInfo("[11......] unknow UID size (reserved)"));

   // check SSD bit
   if ((uids & 0x1F) == 0x00)
      atqa->appendChild(buildFieldInfo("[...00000] bit frame anticollision (Type 1 Tag)"));
   else if ((uids & 0x1F) == 0x01)
      atqa->appendChild(buildFieldInfo("[...00001] bit frame anticollision"));
   else if ((uids & 0x1F) == 0x02)
      atqa->appendChild(buildFieldInfo("[...00010] bit frame anticollision"));
   else if ((uids & 0x1F) == 0x04)
      atqa->appendChild(buildFieldInfo("[...00100] bit frame anticollision"));
   else if ((uids & 0x1F) == 0x08)
      atqa->appendChild(buildFieldInfo("[...01000] bit frame anticollision"));
   else if ((uids & 0x1F) == 0x10)
      atqa->appendChild(buildFieldInfo("[...10000] bit frame anticollision"));

   // check propietary TYPE
   atqa->appendChild(buildFieldInfo(QString("[....%1] type %2").arg(type & 0x0F, 4, 2, QChar('0')).arg(type & 0x0F)));

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SenseFrame);

   root->appendChild(atqa);

   return root;
}

ProtocolFrame *ProtocolParser::parseResponseWUPA(const NfcFrame &frame)
{
   return parseResponseREQA(frame);
}

ProtocolFrame *ProtocolParser::parseResponseSELn(const NfcFrame &frame)
{
   int flags = frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SelectionFrame);

   if (frame.length() == 5)
   {
      if (frame[0] == 0x88) // cascade tag
      {
         root->appendChild(buildFieldInfo("CT", frame.toByteArray(0, 1)));
         root->appendChild(buildFieldInfo("UID", frame.toByteArray(1, 3)));
      }
      else
      {
         root->appendChild(buildFieldInfo("UID", frame.toByteArray(0, 4)));
      }

      root->appendChild(buildFieldInfo("BCC", frame.toByteArray(4, 1)));
   }
   else if (frame.length() == 3)
   {
      ProtocolFrame *sak = buildFieldInfo("SAK", frame.toByteArray(0, 1));

      if (frame[0] & 0x20)
         sak->appendChild(buildFieldInfo("[..1.....] ISO/IEC 14443-4 compliant"));
      else
         sak->appendChild(buildFieldInfo("[..0.....] not compliant with ISO/IEC 14443-4"));

      if (frame[0] & 0x04)
         sak->appendChild(buildFieldInfo("[.....1..] UID not complete"));
      else
         sak->appendChild(buildFieldInfo("[.....0..] UID complete"));

      root->appendChild(sak);
      root->appendChild(buildFieldInfo("CRC", frame.toByteArray(1, 2)));
   }

   return root;
}

ProtocolFrame *ProtocolParser::parseResponseRATS(const NfcFrame &frame)
{
   int offset = 0;
   int tl = frame[offset++];
   int flags = 0;

   flags |= frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;
   flags |= verifyCrc(frame.toByteArray()) ? ProtocolFrame::Flags::CrcError : 0;

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SelectionFrame);

   ProtocolFrame *ats = buildFieldInfo("ATS", frame.toByteArray(1, frame.length() - 3));

   if (tl > 0)
   {
      ProtocolFrame *t0f = buildFieldInfo("T0", frame.toByteArray(offset, 1));

      int t0 = frame[offset++];
      int fsci = t0 & 0x0f;

      t0f->appendChild(buildFieldInfo(QString("[....%1] max frame size %2").arg(fsci, 4, 2, QChar('0')).arg(TABLE_FDS[fsci])));

      ats->appendChild(t0f);

      // TA is transmitted, if bit 4 is set to 1
      if (t0 & 0x10)
      {
         t0f->prependChild(buildFieldInfo("[...1....] TA transmitted"));

         ProtocolFrame *taf = buildFieldInfo("TA", frame.toByteArray(offset, 1));

         int ta = frame[offset++];

         if (ta & 0x80)
            taf->appendChild(buildFieldInfo(QString("[1.......] only support same rate for both directions")));
         else
            taf->appendChild(buildFieldInfo(QString("[0.......] supported different rates for each direction")));

         if (ta & 0x40)
            taf->appendChild(buildFieldInfo(QString("[.1......] supported 848 kbps PICC to PCD")));

         if (ta & 0x20)
            taf->appendChild(buildFieldInfo(QString("[..1.....] supported 424 kbps PICC to PCD")));

         if (ta & 0x10)
            taf->appendChild(buildFieldInfo(QString("[...1....] supported 212 kbps PICC to PCD")));

         if (ta & 0x04)
            taf->appendChild(buildFieldInfo(QString("[.....1..] supported 848 kbps PCD to PICC")));

         if (ta & 0x02)
            taf->appendChild(buildFieldInfo(QString("[......1.] supported 424 kbps PCD to PICC")));

         if (ta & 0x01)
            taf->appendChild(buildFieldInfo(QString("[.......1] supported 212 kbps PCD to PICC")));

         if ((ta & 0x7f) == 0x00)
            taf->appendChild(buildFieldInfo(QString("[.0000000] only 106 kbps supported")));

         ats->appendChild(taf);
      }

      // TB is transmitted, if bit 5 is set to 1
      if (t0 & 0x20)
      {
         t0f->prependChild(buildFieldInfo("[..1.....] TB transmitted"));

         ProtocolFrame *tbf = buildFieldInfo("TB", frame.toByteArray(offset, 1));

         int tb = frame[offset++];

         int sfgi = (tb & 0x0f);
         int fwi = (tb >> 4) & 0x0f;

         float sfgt = TABLE_SFGT[sfgi] * 1000;
         float fwt = TABLE_SFGT[fwi] * 1000;

         tbf->appendChild(buildFieldInfo(QString("[%1....] frame waiting time FWT = %2 ms").arg(fwi, 4, 2, QChar('0')).arg(fwt, 0, 'f', 2)));
         tbf->appendChild(buildFieldInfo(QString("[....%1] start-up frame guard time SFGT = %2 ms").arg(sfgi, 4, 2, QChar('0')).arg(sfgt, 0, 'f', 2)));

         ats->appendChild(tbf);
      }

      // TC is transmitted, if bit 5 is set to 1
      if (t0 & 0x40)
      {
         t0f->prependChild(buildFieldInfo("[.1......] TC transmitted"));

         ProtocolFrame *tcf = buildFieldInfo("TC", frame.toByteArray(offset, 1));

         int tc = frame[offset++];

         if (tc & 0x01)
            tcf->appendChild(buildFieldInfo("[.......1] NAD supported"));

         if (tc & 0x02)
            tcf->appendChild(buildFieldInfo("[......1.] CID supported"));

         ats->appendChild(tcf);
      }

      if (offset < tl)
      {
         ProtocolFrame *hbf = buildFieldInfo("HIST", frame.toByteArray(offset, tl - offset));

         ats->appendChild(hbf);
      }
   }

   root->appendChild(buildFieldInfo("TL", tl));
   root->appendChild(ats);
   root->appendChild(buildFieldInfo("CRC", frame.toByteArray(-2)));

   return root;
}

ProtocolFrame *ProtocolParser::parseResponseHLTA(const NfcFrame &frame)
{
   int flags = frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;

   return buildFrameInfo(frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SenseFrame);
}

ProtocolFrame *ProtocolParser::parseResponsePPSr(const NfcFrame &frame)
{
   int flags = frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;

   return buildFrameInfo(frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::SelectionFrame);
}

ProtocolFrame *ProtocolParser::parseResponseAUTH(const NfcFrame &frame)
{
   int flags = frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;

   return buildFrameInfo(frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::AuthFrame);
}

ProtocolFrame *ProtocolParser::parseResponseIBlock(const NfcFrame &frame)
{
   int pcb = frame[0], offset = 1;
   int flags = 0;

   flags |= frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;
   flags |= verifyCrc(frame.toByteArray()) ? ProtocolFrame::Flags::CrcError : 0;

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::InformationFrame);

   if (pcb & 0x08)
      root->appendChild(buildFieldInfo("CID", frame[offset++] & 0x0F));

   if (pcb & 0x04)
      root->appendChild(buildFieldInfo("NAD", frame[offset++] & 0xFF));

   if (offset < frame.length() - 2)
   {
      root->appendChild(buildFieldInfo("DATA", frame.toByteArray(offset, frame.length() - offset - 2)));
   }

   root->appendChild(buildFieldInfo("CRC", frame.toByteArray(-2)));

   return root;
}

ProtocolFrame *ProtocolParser::parseResponseRBlock(const NfcFrame &frame)
{
   int flags = frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;

   return buildFrameInfo(frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::InformationFrame);
}

ProtocolFrame *ProtocolParser::parseResponseSBlock(const NfcFrame &frame)
{
   int flags = frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;

   return buildFrameInfo(frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, ProtocolFrame::InformationFrame);
}

ProtocolFrame *ProtocolParser::parseResponseUnknow(const NfcFrame &frame)
{
   int flags = frame.isParityError() ? ProtocolFrame::Flags::ParityError : 0;

   return buildFrameInfo(frame.frameRate(), frame.toByteArray(), frame.timeStart(), frame.timeEnd(), flags, 0);
}

bool ProtocolParser::isApdu(const QByteArray &apdu)
{
   if (apdu.length() < 5)
      return false;

   if (apdu.length() - 5 != apdu[4])
      return false;

   return true;
}

ProtocolFrame *ProtocolParser::parseAPDU(const QString &name, const QByteArray &data)
{
   int lc = data[4];

   ProtocolFrame *info = buildFieldInfo(name, data);

   info->appendChild(buildFieldInfo("CLA", data.mid(0, 1)));
   info->appendChild(buildFieldInfo("INS", data.mid(1, 1)));
   info->appendChild(buildFieldInfo("P1", data.mid(2, 1)));
   info->appendChild(buildFieldInfo("P2", data.mid(3, 1)));
   info->appendChild(buildFieldInfo("LC", data.mid(4, 1)));

   if (lc > 0)
      info->appendChild(buildFieldInfo("DATA", data.mid(5, lc)));

   return info;
}

ProtocolFrame *ProtocolParser::buildFrameInfo(const QString &name, int rate, QVariant info, double time, double end, int flags, int type)
{
   return buildInfo(name, rate, info, time, end, 0, flags | ProtocolFrame::RequestFrame, type);
}

ProtocolFrame *ProtocolParser::buildFrameInfo(int rate, QVariant info, double time, double end, int flags, int type)
{
   return buildInfo(QString(), rate, info, time, end, 0, flags | ProtocolFrame::ResponseFrame, type);
}

ProtocolFrame *ProtocolParser::buildFieldInfo(const QString &name, QVariant info)
{
   return buildInfo(name, 0, info, 0, 0, 0, ProtocolFrame::FrameField, 0);
}

ProtocolFrame *ProtocolParser::buildFieldInfo(QVariant info)
{
   return buildInfo(QString(), 0, info, 0, 0, 0, ProtocolFrame::FieldInfo, 0);
}

ProtocolFrame *ProtocolParser::buildInfo(const QString &name, int rate, QVariant info, double time, double end, float diff, int flags, int type)
{
   QVector<QVariant> values;

   // number
   if (flags & ProtocolFrame::RequestFrame || flags & ProtocolFrame::ResponseFrame)
      values << QVariant::fromValue(mCount++);
   else
      values << QVariant::Invalid;

   // time start
   if (time > 0)
      values << QVariant::fromValue(time);
   else
      values << QVariant::Invalid;

   // delta
   if (diff > 0)
      values << QVariant::fromValue(diff);
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

   // content
   values << info;

   // time end
   if (end > 0)
      values << QVariant::fromValue(end);
   else
      values << QVariant::Invalid;

   return new ProtocolFrame(values, flags, type);
}

bool ProtocolParser::verifyCrc(const QByteArray &data, int type)
{
   unsigned short crc = 0, res = 0;

   int length = data.length();

   if (length <= 2)
      return false;

   if (type == CrcType::A)
      crc = 0x6363; // NFC-A ITU-V.41
   else if (type == CrcType::B)
      crc = 0xFFFF; // NFC-B ISO/IEC 13239

   for (int i = 0; i < length - 2; i++)
   {
      unsigned char d = (unsigned char)data[i];

      d = (d ^ (unsigned int)(crc & 0xff));
      d = (d ^ (d << 4));

      crc = (crc >> 8) ^ ((unsigned short)(d << 8)) ^ ((unsigned short)(d << 3)) ^ ((unsigned short)(d >> 4));
   }

   if (type == CrcType::B)
      crc = ~crc;

   res |= ((unsigned int)data[length - 2] & 0xff);
   res |= ((unsigned int)data[length - 1] & 0xff) << 8;

   return res != crc;
}


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

#include <parser/ParserNfcA.h>

void ParserNfcA::reset()
{
   ParserNfcIsoDep::reset();

   frameChain = 0;
}

ProtocolFrame *ParserNfcA::parse(const nfc::NfcFrame &frame)
{
   ProtocolFrame *info = nullptr;

   if (frame.isPollFrame())
   {
      if (frameChain == 0)
      {
         do
         {
            if (!frame.isEncrypted())
            {
               // Request Command, Type A
               if ((info = parseRequestREQA(frame)))
                  break;

               // Wake-UP Command, Type A
               if ((info = parseRequestWUPA(frame)))
                  break;

               // HALT Command, Type A
               if ((info = parseRequestHLTA(frame)))
                  break;

               // Select Command, Type A
               if ((info = parseRequestSELn(frame)))
                  break;

               // Request for Answer to Select
               if ((info = parseRequestRATS(frame)))
                  break;

               // Protocol Parameter Selection
               if ((info = parseRequestPPSr(frame)))
                  break;

               // Mifare AUTH
               if ((info = parseRequestAUTH(frame)))
                  break;
            }

            // Unknown frame... try isoDep
            info = ParserNfcIsoDep::parse(frame);

         } while (false);
      }
         // Mifare AUTH, two pass
      else if (frameChain == 0x60 || frameChain == 0x61)
      {
         info = parseRequestAUTH(frame);
      }
   }
   else
   {
      do
      {
         if (!frame.isEncrypted())
         {
            // Request Command, Type A
            if ((info = parseResponseREQA(frame)))
               break;

            // Wake-UP Command, Type A
            if ((info = parseResponseWUPA(frame)))
               break;

            // HALT Command, Type A
            if ((info = parseResponseHLTA(frame)))
               break;

            // Select Command, Type A
            if ((info = parseResponseSELn(frame)))
               break;

            // Request for Answer to Select
            if ((info = parseResponseRATS(frame)))
               break;

            // Protocol Parameter Selection
            if ((info = parseResponsePPSr(frame)))
               break;

            // Mifare AUTH
            if ((info = parseResponseAUTH(frame)))
               break;
         }

         // Unknown frame... try isoDep
         info = ParserNfcIsoDep::parse(frame);

      } while (false);

      lastCommand = 0;
   }

   return info;
}

ProtocolFrame *ParserNfcA::parseRequestREQA(const nfc::NfcFrame &frame)
{
   if (frame[0] != 0x26 || frame.limit() != 1)
      return nullptr;

   lastCommand = frame[0];

   return buildRootInfo("REQA", frame, ProtocolFrame::SenseFrame);
}

ProtocolFrame *ParserNfcA::parseResponseREQA(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x26 && lastCommand != 0x52)
      return nullptr;

   int atqv = frame[1] << 8 | frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, ProtocolFrame::SenseFrame);

   if (ProtocolFrame *atqa = root->appendChild(buildChildInfo("ATQA", QString("%1 [%2]").arg(atqv, 4, 16, QChar('0')).arg(atqv, 16, 2, QChar('0')), 0, 2)))
   {
      // propietary TYPE
      atqa->appendChild(buildChildInfo(QString("  [....%1........] propietary type %2").arg((atqv >> 8) & 0x0F, 4, 2, QChar('0')).arg((atqv >> 8) & 0x0F, 1, 16, QChar('0'))));

      // check UID size
      if ((atqv & 0xC0) == 0x00)
         atqa->appendChild(buildChildInfo("  [........00......] single size UID"));
      else if ((atqv & 0xC0) == 0x40)
         atqa->appendChild(buildChildInfo("  [........01......] double size UID"));
      else if ((atqv & 0xC0) == 0x80)
         atqa->appendChild(buildChildInfo("  [........10......] triple size UID"));
      else if ((atqv & 0xC0) == 0xC0)
         atqa->appendChild(buildChildInfo("  [........11......] unknow UID size (reserved)"));

      // check SSD bit
      if ((atqv & 0x1F) == 0x00)
         atqa->appendChild(buildChildInfo("  [...........00000] bit frame anticollision (Type 1 Tag)"));
      else if ((atqv & 0x1F) == 0x01)
         atqa->appendChild(buildChildInfo("  [...........00001] bit frame anticollision"));
      else if ((atqv & 0x1F) == 0x02)
         atqa->appendChild(buildChildInfo("  [...........00010] bit frame anticollision"));
      else if ((atqv & 0x1F) == 0x04)
         atqa->appendChild(buildChildInfo("  [...........00100] bit frame anticollision"));
      else if ((atqv & 0x1F) == 0x08)
         atqa->appendChild(buildChildInfo("  [...........01000] bit frame anticollision"));
      else if ((atqv & 0x1F) == 0x10)
         atqa->appendChild(buildChildInfo("  [...........10000] bit frame anticollision"));
   }

   return root;
}

ProtocolFrame *ParserNfcA::parseRequestWUPA(const nfc::NfcFrame &frame)
{
   if (frame[0] != 0x52 || frame.limit() != 1)
      return nullptr;

   lastCommand = frame[0];

   return buildRootInfo("WUPA", frame, ProtocolFrame::SenseFrame);
}

ProtocolFrame *ParserNfcA::parseResponseWUPA(const nfc::NfcFrame &frame)
{
   return parseResponseREQA(frame);
}

ProtocolFrame *ParserNfcA::parseRequestSELn(const nfc::NfcFrame &frame)
{
   int cmd = frame[0];

   if (cmd != 0x93 && cmd != 0x95 && cmd != 0x97)
      return nullptr;

   lastCommand = frame[0];

   ProtocolFrame *root;

   int nvb = frame[1] >> 4;

   switch (cmd)
   {
      case 0x93:
         root = buildRootInfo("SEL1", frame, ProtocolFrame::SelectionFrame);
         break;
      case 0x95:
         root = buildRootInfo("SEL2", frame, ProtocolFrame::SelectionFrame);
         break;
      case 0x97:
         root = buildRootInfo("SEL3", frame, ProtocolFrame::SelectionFrame);
         break;
   }

   // command detailed info
   root->appendChild(buildChildInfo("NVB", nvb, 1, 1));

   if (nvb == 7)
   {
      if (frame[2] == 0x88) // cascade tag
      {
         root->appendChild(buildChildInfo("CT", frame, 2, 1));
         root->appendChild(buildChildInfo("UID", frame, 3, 3));
      }
      else
      {
         root->appendChild(buildChildInfo("UID", frame, 2, 4));
      }

      root->appendChild(buildChildInfo("BCC", frame, 6, 1));
      root->appendChild(buildChildInfo("CRC", frame, -2, 2));
   }

   return root;
}

ProtocolFrame *ParserNfcA::parseResponseSELn(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x93 && lastCommand != 0x95 && lastCommand != 0x97)
      return nullptr;

   ProtocolFrame *root = buildRootInfo("", frame, ProtocolFrame::SelectionFrame);

   if (frame.limit() == 5)
   {
      if (frame[0] == 0x88) // cascade tag
      {
         root->appendChild(buildChildInfo("CT", frame, 0, 1));
         root->appendChild(buildChildInfo("UID", frame, 1, 3));
      }
      else
      {
         root->appendChild(buildChildInfo("UID", frame, 0, 4));
      }

      root->appendChild(buildChildInfo("BCC", frame, 4, 1));
   }
   else if (frame.limit() == 3)
   {
      int sa = frame[0];

      if (ProtocolFrame *sak = root->appendChild(buildChildInfo("SAK", QString("%1 [%2]").arg(sa, 2, 16, QChar('0')).arg(sa, 8, 2, QChar('0')), 0, 1)))
      {
         if (sa & 0x40)
            sak->appendChild(buildChildInfo("[.1......] ISO/IEC 18092 (NFC) compliant"));
         else
            sak->appendChild(buildChildInfo("[.0......] not compliant with 18092 (NFC)"));

         if (sa & 0x20)
            sak->appendChild(buildChildInfo("[..1.....] ISO/IEC 14443-4 compliant"));
         else
            sak->appendChild(buildChildInfo("[..0.....] not compliant with ISO/IEC 14443-4"));

         if (sa & 0x04)
            sak->appendChild(buildChildInfo("[.....1..] UID not complete"));
         else
            sak->appendChild(buildChildInfo("[.....0..] UID complete"));
      }

      root->appendChild(buildChildInfo("CRC", frame, 1, 2));
   }

   return root;
}

ProtocolFrame *ParserNfcA::parseRequestRATS(const nfc::NfcFrame &frame)
{
   if (frame[0] != 0xE0 || frame.limit() != 4)
      return nullptr;

   lastCommand = frame[0];

   int par = frame[1];
   int cdi = (par & 0x0F);
   int fsdi = (par >> 4) & 0x0F;

   ProtocolFrame *root = buildRootInfo("RATS", frame, ProtocolFrame::SelectionFrame);

   if (ProtocolFrame *param = root->appendChild(buildChildInfo("PARAM", QString("%1 [%2]").arg(par, 2, 16, QChar('0')).arg(par, 8, 2, QChar('0')), 0, 1)))
   {
      param->appendChild(buildChildInfo(QString("[%1....] FSD max frame size %2").arg(fsdi, 4, 2, QChar('0')).arg(nfc::NFC_FDS_TABLE[fsdi])));
      param->appendChild(buildChildInfo(QString("[....%1] CDI logical channel %2").arg(cdi, 4, 2, QChar('0')).arg(cdi)));
   }

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcA::parseResponseRATS(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0xE0)
      return nullptr;

   int tl = frame[0];
   int offset = 1;

   ProtocolFrame *root = buildRootInfo("", frame, ProtocolFrame::SelectionFrame);

   root->appendChild(buildChildInfo("TL", tl, 0, 1));

   if (ProtocolFrame *ats = root->appendChild(buildChildInfo("ATS", frame, 1, frame.limit() - 3)))
   {
      if (tl > 0)
      {
         int t0 = frame[offset++];
         int fsci = t0 & 0x0f;

         if (ProtocolFrame *t0f = ats->appendChild(buildChildInfo("T0", QString("%1 [%2]").arg(t0, 2, 16, QChar('0')).arg(t0, 8, 2, QChar('0')), offset - 1, 1)))
         {
            t0f->appendChild(buildChildInfo(QString("[....%1] max frame size %2").arg(fsci, 4, 2, QChar('0')).arg(nfc::NFC_FDS_TABLE[fsci])));

            // TA is transmitted, if bit 4 is set to 1
            if (t0 & 0x10)
            {
               t0f->prependChild(buildChildInfo("[...1....] TA transmitted"));

               int ta = frame[offset++];

               if (ProtocolFrame *taf = ats->appendChild(buildChildInfo("TA", QString("%1 [%2]").arg(ta, 2, 16, QChar('0')).arg(ta, 8, 2, QChar('0')), offset - 1, 1)))
               {
                  if (ta & 0x80)
                     taf->appendChild(buildChildInfo(QString("[1.......] only support same rate for both directions")));
                  else
                     taf->appendChild(buildChildInfo(QString("[0.......] supported different rates for each direction")));

                  if (ta & 0x40)
                     taf->appendChild(buildChildInfo(QString("[.1......] supported 848 kbps PICC to PCD")));

                  if (ta & 0x20)
                     taf->appendChild(buildChildInfo(QString("[..1.....] supported 424 kbps PICC to PCD")));

                  if (ta & 0x10)
                     taf->appendChild(buildChildInfo(QString("[...1....] supported 212 kbps PICC to PCD")));

                  if (ta & 0x04)
                     taf->appendChild(buildChildInfo(QString("[.....1..] supported 848 kbps PCD to PICC")));

                  if (ta & 0x02)
                     taf->appendChild(buildChildInfo(QString("[......1.] supported 424 kbps PCD to PICC")));

                  if (ta & 0x01)
                     taf->appendChild(buildChildInfo(QString("[.......1] supported 212 kbps PCD to PICC")));

                  if ((ta & 0x7f) == 0x00)
                     taf->appendChild(buildChildInfo(QString("[.0000000] only 106 kbps supported")));
               }
            }

            // TB is transmitted, if bit 5 is set to 1
            if (t0 & 0x20)
            {
               t0f->prependChild(buildChildInfo("[..1.....] TB transmitted"));

               int tb = frame[offset++];

               if (ProtocolFrame *tbf = ats->appendChild(buildChildInfo("TB", QString("%1 [%2]").arg(tb, 2, 16, QChar('0')).arg(tb, 8, 2, QChar('0')), offset - 1, 1)))
               {
                  int fwi = (tb >> 4) & 0x0f;
                  int sfgi = (tb & 0x0f);

                  float fwt = nfc::NFC_FWT_TABLE[fwi] / nfc::NFC_FC;
                  float sfgt = nfc::NFC_SFGT_TABLE[sfgi] / nfc::NFC_FC;

                  tbf->appendChild(buildChildInfo(QString("[%1....] frame waiting time FWT = %2 ms").arg(fwi, 4, 2, QChar('0')).arg(1E3 * fwt, 0, 'f', 2)));
                  tbf->appendChild(buildChildInfo(QString("[....%1] start-up frame guard time SFGT = %2 ms").arg(sfgi, 4, 2, QChar('0')).arg(1E3 * sfgt, 0, 'f', 2)));
               }
            }

            // TC is transmitted, if bit 5 is set to 1
            if (t0 & 0x40)
            {
               t0f->prependChild(buildChildInfo("[.1......] TC transmitted"));

               int tc = frame[offset++];

               if (ProtocolFrame *tcf = ats->appendChild(buildChildInfo("TC", QString("%1 [%2]").arg(tc, 2, 16, QChar('0')).arg(tc, 8, 2, QChar('0')), offset - 1, 1)))
               {
                  if (tc & 0x01)
                     tcf->appendChild(buildChildInfo("[.......1] NAD supported"));

                  if (tc & 0x02)
                     tcf->appendChild(buildChildInfo("[......1.] CID supported"));
               }
            }
         }

         if (offset < tl)
         {
            ats->appendChild(buildChildInfo("HIST", frame, offset, tl - offset));
         }
      }
   }

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcA::parseRequestHLTA(const nfc::NfcFrame &frame)
{
   if (frame[0] != 0x50 || frame.limit() != 4)
      return nullptr;

   lastCommand = frame[0];

   ProtocolFrame *root = buildRootInfo("HLTA", frame, ProtocolFrame::SenseFrame);

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcA::parseResponseHLTA(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x50)
      return nullptr;

   return buildRootInfo("", frame, ProtocolFrame::SenseFrame);
}

ProtocolFrame *ParserNfcA::parseRequestPPSr(const nfc::NfcFrame &frame)
{
   int pps = frame[0];

   if ((pps & 0xF0) != 0xD0 || frame.limit() != 5)
      return nullptr;

   lastCommand = frame[0];

   ProtocolFrame *root = buildRootInfo("PPS", frame, ProtocolFrame::SelectionFrame);

   root->appendChild(buildChildInfo("CID", pps & 0x0F, 0, 1));
   root->appendChild(buildChildInfo("PPS0", frame, 1, 1));

   int pps0 = frame[1];

   if (pps0 & 0x10)
   {
      int pps1 = frame[2];

      if (ProtocolFrame *pps1f = root->appendChild(buildChildInfo("PPS1", QString("%1 [%2]").arg(pps1, 2, 16, QChar('0')).arg(pps1, 8, 2, QChar('0')), 2, 1)))
      {
         if ((pps1 & 0x0C) == 0x00)
            pps1f->appendChild(buildChildInfo("[....00..] selected 106 kbps PICC to PCD rate"));
         else if ((pps1 & 0x0C) == 0x04)
            pps1f->appendChild(buildChildInfo("[....01..] selected 212 kbps PICC to PCD rate"));
         else if ((pps1 & 0x0C) == 0x08)
            pps1f->appendChild(buildChildInfo("[....10..] selected 424 kbps PICC to PCD rate"));
         else if ((pps1 & 0x0C) == 0x0C)
            pps1f->appendChild(buildChildInfo("[....11..] selected 848 kbps PICC to PCD rate"));

         if ((pps1 & 0x03) == 0x00)
            pps1f->appendChild(buildChildInfo("[......00] selected 106 kbps PCD to PICC rate"));
         else if ((pps1 & 0x03) == 0x01)
            pps1f->appendChild(buildChildInfo("[......01] selected 212 kbps PCD to PICC rate"));
         else if ((pps1 & 0x03) == 0x02)
            pps1f->appendChild(buildChildInfo("[......10] selected 424 kbps PCD to PICC rate"));
         else if ((pps1 & 0x03) == 0x03)
            pps1f->appendChild(buildChildInfo("[......11] selected 848 kbps PCD to PICC rate"));
      }
   }

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcA::parseResponsePPSr(const nfc::NfcFrame &frame)
{
   if ((lastCommand & 0xF0) != 0xD0)
      return nullptr;

   return buildRootInfo("", frame, ProtocolFrame::SelectionFrame);
}

ProtocolFrame *ParserNfcA::parseRequestAUTH(const nfc::NfcFrame &frame)
{
   if ((frame[0] != 0x60 && frame[0] != 0x61) || frame.limit() != 4)
      return nullptr;

   lastCommand = frame[0];

   if (frameChain == 0)
   {
      int cmd = frame[0];
      int block = frame[1];

      ProtocolFrame *root = buildRootInfo(cmd == 0x60 ? "AUTH(A)" : "AUTH(B)", frame, ProtocolFrame::AuthFrame);

      root->appendChild(buildChildInfo("BLOCK", block));
      root->appendChild(buildChildInfo("CRC", frame, -2, 2));

      frameChain = cmd;

      return root;
   }

   ProtocolFrame *root = buildRootInfo(frameChain == 0x60 ? "AUTH(A)" : "AUTH(B)", frame, ProtocolFrame::AuthFrame);

   root->appendChild(buildChildInfo("TOKEN", frame, 0, frame.limit()));

   frameChain = 0;

   return root;
}

ProtocolFrame *ParserNfcA::parseResponseAUTH(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x60 && lastCommand != 0x61)
      return nullptr;

   return buildRootInfo("", frame, ProtocolFrame::AuthFrame);
}


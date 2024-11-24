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

#include <lab/iso/Iso.h>

#include <parser/ParserISO7816.h>

#define ATR_TA_MASK 0x10
#define ATR_TB_MASK 0x20
#define ATR_TC_MASK 0x40
#define ATR_TD_MASK 0x80

#define PPS_PPS1_MASK 0x10
#define PPS_PPS2_MASK 0x20
#define PPS_PPS3_MASK 0x40
#define PPS_PPS4_MASK 0x80

#define PPS_MIN_LEN 3
#define PPS_MAX_LEN 6
#define PPS_CMD 0xFF

#define TPDU_MIN_LEN 5
#define TPDU_MAX_LEN 255
#define TPDU_HEADER_LEN 5

#define TPDU_CLA_OFFSET 0
#define TPDU_INS_OFFSET 1
#define TPDU_P1_OFFSET 2
#define TPDU_P2_OFFSET 3
#define TPDU_P3_OFFSET 4
#define TPDU_PROC_OFFSET 5

void ParserISO7816::reset()
{
}

ProtocolFrame *ParserISO7816::parse(const lab::RawFrame &frame)
{
   if (frame.frameType() == lab::FrameType::IsoATRFrame)
      return parseATR(frame);

   switch (frame.frameType())
   {
      case lab::FrameType::IsoVccLow:
      case lab::FrameType::IsoVccHigh:
         return parseVCC(frame);

      case lab::FrameType::IsoRstLow:
      case lab::FrameType::IsoRstHigh:
         return parseRST(frame);

      case lab::FrameType::IsoATRFrame:
         return parseATR(frame);
   }

   ProtocolFrame *info = nullptr;

   do
   {
      // Protocol parameters selection
      if ((info = parsePPS(frame)))
         break;

      // T0 TPDU
      if ((info = parseTPDU(frame)))
         break;

      // T1 I-Block
      if ((info = parseIBlock(frame)))
         break;

      // T1 R-Block
      if ((info = parseRBlock(frame)))
         break;

      // T1 S-Block
      if ((info = parseSBlock(frame)))
         break;

   }
   while (false);

   return info;
}

ProtocolFrame *ParserISO7816::parseVCC(const lab::RawFrame &frame)
{
   if (frame.frameType() != lab::FrameType::IsoVccLow || frame.frameType() != lab::FrameType::IsoVccHigh)
      return nullptr;

   ProtocolFrame *root = buildRootInfo("VCC", frame, 0);

   return root;
}

ProtocolFrame *ParserISO7816::parseRST(const lab::RawFrame &frame)
{
   if (frame.frameType() != lab::FrameType::IsoRstLow || frame.frameType() != lab::FrameType::IsoRstHigh)
      return nullptr;

   ProtocolFrame *root = buildRootInfo("RST", frame, 0);

   return root;
}

ProtocolFrame *ParserISO7816::parseATR(const lab::RawFrame &frame)
{
   if (frame.frameType() != lab::FrameType::IsoATRFrame)
      return nullptr;

   ProtocolFrame *root = buildRootInfo("ATR", frame, 0);

   unsigned int offset = 0;
   unsigned int ts = frame[offset++];
   unsigned int tk = 0, hb = 0, k = 0;

   ProtocolFrame *tsf = root->appendChild(buildChildInfo("TS", QString("%1 [%2]").arg(ts, 2, 16, QChar('0')).arg(ts, 8, 2, QChar('0')), offset - 1, 1));

   if (ts == 0x3B)
      tsf->appendChild(buildChildInfo(QString("[00111011] Direct convention")));
   else if (ts == 0x3F)
      tsf->appendChild(buildChildInfo(QString("[00111111] Inverse convention")));
   else
      tsf->appendChild(buildChildInfo(QString("[%1] Unknown convention pattern").arg(ts, 8, 2, QChar('0'))));

   do
   {
      tk = frame[offset++];

      ProtocolFrame *txf = root->appendChild(buildChildInfo(QString("T%1%2").arg(k > 0 ? "D" : "").arg(k), QString("%1 [%2]").arg(tk, 2, 16, QChar('0')).arg(tk, 8, 2, QChar('0')), offset - 1, 1));

      if (tk & ATR_TD_MASK)
         txf->appendChild(buildChildInfo(QString("[1.......] TD%1 transmitted").arg(k + 1)));

      if (tk & ATR_TC_MASK)
         txf->appendChild(buildChildInfo(QString("[.1......] TC%1 transmitted").arg(k + 1)));

      if (tk & ATR_TB_MASK)
         txf->appendChild(buildChildInfo(QString("[..1.....] TB%1 transmitted").arg(k + 1)));

      if (tk & ATR_TA_MASK)
         txf->appendChild(buildChildInfo(QString("[...1....] TA%1 transmitted").arg(k + 1)));

      switch (k)
      {
         case 0:
         {
            hb = tk & 0x0F;
            txf->appendChild(buildChildInfo(QString("[....%1] %2 historical bytes").arg(tk & 0x0f, 4, 2, QChar('0')).arg(tk & 0x0f)));
            break;
         }
         case 1:
         case 2:
         {
            switch (tk & 0x0f)
            {
               case 0x00:
                  txf->appendChild(buildChildInfo("[....0000] T=0 half-duplex transmission of characters"));
                  break;
               case 0x01:
                  txf->appendChild(buildChildInfo("[....0001] T=1 half-duplex transmission of blocks"));
                  break;
               case 0x02:
                  txf->appendChild(buildChildInfo("[....0010] T=2 reserved for future full-duplex operations"));
                  break;
               case 0x03:
                  txf->appendChild(buildChildInfo("[....0011] T=3 reserved for future full-duplex operations"));
                  break;
               case 0x04:
                  txf->appendChild(buildChildInfo("[....0100] T=4 reserved for an enhanced half-duplex transmission of characters"));
                  break;
               case 0x0E:
                  txf->appendChild(buildChildInfo("[....1110] T=14 refers to transmission protocols not standardized"));
                  break;
               case 0x0F:
                  txf->appendChild(buildChildInfo("[....1111] T=15 qualifies global interface bytes"));
                  break;
               default:
                  txf->appendChild(buildChildInfo(QString("[....%1] T=%2 reserved for future use").arg(tk & 0x0f, 4, 2, QChar('0')).arg(tk & 0x0f)));
            }

            break;
         }
      }

      // check presence of TAk+1
      if (tk & ATR_TA_MASK)
      {
         unsigned int ta = frame[offset++];

         ProtocolFrame *taf = root->appendChild(buildChildInfo(QString("TA%1").arg(k + 1), QString("%1 [%2]").arg(ta, 2, 16, QChar('0')).arg(ta, 8, 2, QChar('0')), offset - 1, 1));

         switch (k + 1)
         {
            case 1:
            {
               // TA1 encodes the value of the clock rate conversion integer (Fi), the baud rate adjustment integer(Di)
               // and the maximum value of the frequency supported by the card
               unsigned int fi = ta >> 4;
               unsigned int di = ta & 0x0f;
               unsigned int dn = lab::ISO_DI_TABLE[di];
               unsigned int fn = lab::ISO_FM_TABLE[fi];

               taf->appendChild(buildChildInfo(QString("[%1....] Maximum frequency supported, Fi = %2 (%3 MHz)").arg(fi, 4, 2, QChar('0')).arg(fi).arg(fn / 1E6, 0, 'f', 2)));
               taf->appendChild(buildChildInfo(QString("[....%1] Baud rate divisor, Di = %2 (1/%3)").arg(di, 4, 2, QChar('0')).arg(di).arg(dn)));

               break;
            }
            case 3:
            {
               taf->appendChild(buildChildInfo(QString("[%1] Information field size for the card, IFSC = %2").arg(ta, 8, 2, QChar('0')).arg(ta)));
               break;
            }
         }
      }

      // check presence of TBk+1
      if (tk & ATR_TB_MASK)
      {
         unsigned int tb = frame[offset++];

         ProtocolFrame *tbf = root->appendChild(buildChildInfo(QString("TB%1").arg(
                                                                  k + 1), QString("%1 [%2]").arg(tb, 2, 16, QChar('0')).arg(tb, 8, 2, QChar('0')), offset - 1, 1));

         switch (k + 1)
         {
            case 1:
            {
               tbf->appendChild(buildChildInfo(QString("[%1] Global, deprecated programming current and voltage").arg(tb, 8, 2, QChar('0'))));
               break;
            }
            case 3:
            {
               unsigned int bwi = tb >> 4;
               unsigned int cwi = tb & 0x0f;
               unsigned int bwt = 11 + lab::ISO_BWT_TABLE[bwi];
               unsigned int cwt = 11 + lab::ISO_CWT_TABLE[cwi];

               tbf->appendChild(buildChildInfo(QString("[%1....] Block waiting time, BWT = %2 (%3 ETUs)").arg(bwi, 4, 2, QChar('0')).arg(bwi).arg(bwt)));
               tbf->appendChild(buildChildInfo(QString("[....%1] Character waiting time, CWI = %2 (%3 ETUs)").arg(cwi, 4, 2, QChar('0')).arg(cwi).arg(cwt)));
               break;
            }
         }

      }

      // check presence of TCk+1
      if (tk & ATR_TC_MASK)
      {
         unsigned int tc = frame[offset++];

         ProtocolFrame *tcf = root->appendChild(buildChildInfo(QString("TC%1").arg(k + 1), QString("%1 [%2]").arg(tc, 2, 16, QChar('0')).arg(tc, 8, 2, QChar('0')), offset - 1, 1));

         switch (k + 1)
         {
            case 1:
            {
               tcf->appendChild(buildChildInfo(QString("[%1] Extra guard time %2 ETU").arg(tc, 8, 2, QChar('0')).arg(tc)));
               break;
            }
            case 2:
            {
               tcf->appendChild(buildChildInfo(QString("[%1] Waiting time %2 ETU").arg(tc, 8, 2, QChar('0')).arg(tc * 960)));
               break;
            }
            case 3:
            {
               tcf->appendChild(buildChildInfo(QString("[%1] Error detection code to be used: %2").arg(tc, 8, 2, QChar('0')).arg(tc & 0x01 ? "CRC" : "LRC")));
               break;
            }
         }
      }

      // check presence of TDk+1
      if (tk & ATR_TD_MASK)
      {
         // next structural byte
         k++;
      }
   }
   while (tk & ATR_TD_MASK && offset < frame.size());

   // parse historical bytes
   if (hb)
      root->appendChild(buildChildInfo("HB", frame, offset, hb));

   if (frame.size() > offset + hb)
      root->appendChild(buildChildInfo("TCK", frame, offset + hb, 1));

   return root;
}

ProtocolFrame *ParserISO7816::parsePPS(const lab::RawFrame &frame)
{
   if (frame.frameType() != lab::FrameType::IsoRequestFrame && frame.frameType() != lab::FrameType::IsoResponseFrame)
      return nullptr;

   if (frame.size() < PPS_MIN_LEN || frame.size() > PPS_MAX_LEN || frame[0] != PPS_CMD)
      return nullptr;

   int offset = 1;

   ProtocolFrame *root = buildRootInfo("PPS", frame, 0);

   unsigned int pps0 = frame[offset++];
   unsigned int tn = pps0 & 0x0f;

   ProtocolFrame *pps0f = root->appendChild(buildChildInfo("PPS0", QString("%1 [%2]").arg(pps0, 2, 16, QChar('0')).arg(pps0, 8, 2, QChar('0')), offset - 1, 1));

   if (pps0 & PPS_PPS4_MASK)
      pps0f->appendChild(buildChildInfo(QString("[1.......] PPS4 transmitted (reserved for future use)")));

   if (pps0 & PPS_PPS3_MASK)
      pps0f->appendChild(buildChildInfo(QString("[.1......] PPS3 transmitted")));

   if (pps0 & PPS_PPS2_MASK)
      pps0f->appendChild(buildChildInfo(QString("[..1.....] PPS2 transmitted")));

   if (pps0 & PPS_PPS1_MASK)
      pps0f->appendChild(buildChildInfo(QString("[...1....] PPS1 transmitted")));

   pps0f->appendChild(buildChildInfo(QString("[....%1] T=%2 protocol selection").arg(tn & 0x0f, 4, 2, QChar('0')).arg(tn & 0x0f)));

   if (pps0 & PPS_PPS1_MASK)
   {
      unsigned pps1 = frame[offset++];
      unsigned int fi = pps1 >> 4;
      unsigned int di = pps1 & 0x0f;
      unsigned int dn = lab::ISO_DI_TABLE[di];
      unsigned int fn = lab::ISO_FI_TABLE[fi];

      ProtocolFrame *pps1f = root->appendChild(buildChildInfo("PPS1", QString("%1 [%2]").arg(pps1, 2, 16, QChar('0')).arg(pps1, 8, 2, QChar('0')), offset - 1, 1));

      pps1f->appendChild(buildChildInfo(QString("[%1....] Frequency adjustment, Fi = %2 (%3)").arg(fi, 4, 2, QChar('0')).arg(fi).arg(fn)));
      pps1f->appendChild(buildChildInfo(QString("[....%1] Baud rate divisor, Di = %2, (1/%3)").arg(di, 4, 2, QChar('0')).arg(di).arg(dn)));
   }

   if (pps0 & PPS_PPS2_MASK)
   {
      unsigned pps2 = frame[offset++];
      ProtocolFrame *pps2f = root->appendChild(buildChildInfo("PPS1", QString("%1 [%2]").arg(pps2, 2, 16, QChar('0')).arg(pps2, 8, 2, QChar('0')), offset - 1, 1));
   }

   if (pps0 & PPS_PPS3_MASK)
   {
      unsigned pps3 = frame[offset++];
      ProtocolFrame *pps3f = root->appendChild(buildChildInfo("PPS1", QString("%1 [%2]").arg(pps3, 2, 16, QChar('0')).arg(pps3, 8, 2, QChar('0')), offset - 1, 1));
   }

   root->appendChild(buildChildInfo("PCK", frame, offset, 1));

   return root;
}

ProtocolFrame *ParserISO7816::parseTPDU(const lab::RawFrame &frame)
{
   if (frame.frameType() != lab::FrameType::IsoExchangeFrame)
      return nullptr;

   if (frame.size() < TPDU_MIN_LEN || frame.size() > TPDU_MAX_LEN || frame[0] == PPS_CMD)
      return nullptr;

   ProtocolFrame *root = buildRootInfo("TPDU", frame, 0);

   ProtocolFrame *header = root->appendChild(buildChildInfo("HEADER", frame, 0, TPDU_HEADER_LEN));

   header->appendChild(buildChildInfo("CLA", frame, TPDU_CLA_OFFSET, 1));
   header->appendChild(buildChildInfo("INS", frame, TPDU_INS_OFFSET, 1));
   header->appendChild(buildChildInfo("P1", frame, TPDU_P1_OFFSET, 1));
   header->appendChild(buildChildInfo("P2", frame, TPDU_P2_OFFSET, 1));
   header->appendChild(buildChildInfo("P3", frame, TPDU_P3_OFFSET, 1));

   for (unsigned int offset = TPDU_PROC_OFFSET; offset < frame.size(); offset++)
   {
      // check if procedure byte is NULL (INS), then device must transmit all remaining data
      if (frame[offset] == 0x60)
      {
         root->appendChild(buildChildInfo("NULL", frame, offset, 1));
         continue;
      }

      // check if procedure byte is SW1
      if ((frame[offset] & 0xF0) == 0x60 || (frame[offset] & 0xF0) == 0x90)
      {
         root->appendChild(buildChildInfo("SW", frame, offset, 2));
         break;
      }

      // check if procedure byte is ACK (INS), then device must transmit all remaining data
      if (frame[offset] == frame[TPDU_INS_OFFSET])
      {
         unsigned int p3 = frame[TPDU_P3_OFFSET];
         root->appendChild(buildChildInfo("ACK", frame, offset, 1));
         root->appendChild(buildChildInfo("DATA", frame, offset + 1, p3));
         offset += p3;
      }

      // check if procedure byte is ACK (INS^0xFF), then device must transmit ONE byte
      else if (frame[offset] == (frame[TPDU_INS_OFFSET] ^ 0xFF))
      {
         root->appendChild(buildChildInfo("ACK", frame, offset, 1));
         root->appendChild(buildChildInfo("DATA", frame, offset + 1, 1));
         offset += 1;
      }
   }

   return root;
}

ProtocolFrame *ParserISO7816::parseIBlock(const lab::RawFrame &frame)
{
   // check if frame is an I-Block, bit 8 must be and length must be at least 4 bytes
   if (frame.limit() < 4 || (frame[1] & 0x80))
      return nullptr;

   int offset = 0;
   int nad = frame[offset++];
   int pcb = frame[offset++];
   int len = frame[offset++];

   ProtocolFrame *root = buildRootInfo("I-Block", frame, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildChildInfo("NAD", frame, 0, 1));

   if (ProtocolFrame *pcbf = root->appendChild(buildChildInfo("PCB", QString("%1 [%2]").arg(pcb, 2, 16, QChar('0')).arg(pcb, 8, 2, QChar('0')), 1, 1)))
   {
      pcbf->appendChild(buildChildInfo("[0.......] I-Block"));
      pcbf->appendChild(buildChildInfo(QString("[.%1......] Sequence number, %2").arg((pcb >> 6) & 1, 1, 2, QChar('0')).arg((pcb >> 6) & 1)));

      if (pcb & 0x20)
         pcbf->appendChild(buildChildInfo("[..1.....] More data (chaining)"));
      else
         pcbf->appendChild(buildChildInfo("[..0.....] No more data (no chaining)"));
   }

   root->appendChild(buildChildInfo("LEN", frame, 2, 1));

   if (len > 0)
      root->appendChild(buildChildInfo("INF", frame, 3, len));

   if (offset + len == frame.size() - 1)
      root->appendChild(buildChildInfo("LRC", frame, -1, 1));
   else
      root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserISO7816::parseRBlock(const lab::RawFrame &frame)
{
   // check if frame is an R-Block, bit 7 and 8 must be 1,0 and length must be at least 4 bytes
   if (frame.limit() < 4 || (frame[1] & 0xC0) != 0x80)
      return nullptr;

   int offset = 0;
   int nad = frame[offset++];
   int pcb = frame[offset++];
   int len = frame[offset++];

   ProtocolFrame *root = buildRootInfo("R-Block", frame, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildChildInfo("NAD", frame, 0, 1));

   if (ProtocolFrame *pcbf = root->appendChild(buildChildInfo("PCB", QString("%1 [%2]").arg(pcb, 2, 16, QChar('0')).arg(pcb, 8, 2, QChar('0')), 1, 1)))
   {
      pcbf->appendChild(buildChildInfo("[10......] R-Block"));

      if (pcb & 0x10)
         pcbf->appendChild(buildChildInfo("[..1.....] NACK (error)"));
      else
         pcbf->appendChild(buildChildInfo("[..0.....] ACK (no error)"));

      if (pcb & 0x0F == 0x00)
         pcbf->appendChild(buildChildInfo("[....0000] Error-free acknowledgement"));
      else if (pcb & 0x0F == 0x01)
         pcbf->appendChild(buildChildInfo("[....0001] Redundancy code error or a character parity error"));
      else if (pcb & 0x0F == 0x02)
         pcbf->appendChild(buildChildInfo("[....0010] Other errors"));
   }

   return root;
}

ProtocolFrame *ParserISO7816::parseSBlock(const lab::RawFrame &frame)
{
   // check if frame is an S-Block, bit 7 and 8 must be 1 and length must be at least 4 bytes
   if (frame.limit() < 4 || (frame[1] & 0xC0) != 0xC0)
      return nullptr;

   int offset = 0;
   int nad = frame[offset++];
   int pcb = frame[offset++];
   int len = frame[offset++];

   QString name = "S-Block";

   if ((pcb & 0x1F) == 0x00)
      name = "S(RESYNCH)";
   else if ((pcb & 0x1F) == 0x01)
      name = "S(IFS)";
   else if ((pcb & 0x1F) == 0x02)
      name = "S(ABORT)";
   else if ((pcb & 0x1F) == 0x03)
      name = "S(WTX)";

   ProtocolFrame *root = buildRootInfo(name, frame, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildChildInfo("NAD", frame, 0, 1));

   if (ProtocolFrame *pcbf = root->appendChild(buildChildInfo("PCB", QString("%1 [%2]").arg(pcb, 2, 16, QChar('0')).arg(pcb, 8, 2, QChar('0')), 1, 1)))
   {
      pcbf->appendChild(buildChildInfo("[11......] S-Block"));

      if (pcb & 0x20)
         pcbf->appendChild(buildChildInfo("[..1.....] Response block"));
      else
         pcbf->appendChild(buildChildInfo("[..0.....] Request block"));

      if ((pcb & 0x1F) == 0x00)
         pcbf->appendChild(buildChildInfo("[...00000] RESYNCH (resynchronization block)"));
      else if ((pcb & 0x1F) == 0x01)
         pcbf->appendChild(buildChildInfo("[...00001] IFS (information field size block)"));
      else if ((pcb & 0x1F) == 0x02)
         pcbf->appendChild(buildChildInfo("[...00010] ABORT (operation abort block)"));
      else if ((pcb & 0x1F) == 0x03)
         pcbf->appendChild(buildChildInfo("[...00011] WTX (waiting time extension block)"));
   }

   root->appendChild(buildChildInfo("LEN", frame, 2, 1));

   // add IFS parameter
   if ((pcb & 0x1F) == 0x01)
   {
      unsigned int ifs = frame[offset++];

      ProtocolFrame *ifsf = root->appendChild(buildChildInfo("IFS", QString("%1 [%2]").arg(ifs, 2, 16, QChar('0')).arg(ifs, 8, 2, QChar('0')), offset - 1, 1));

      ifsf->appendChild(buildChildInfo(QString("[%1] Information field size, %2 bytes").arg(ifs, 8, 2, QChar('0')).arg(ifs)));
   }

   if (offset == frame.size() - 1)
      root->appendChild(buildChildInfo("LRC", frame, -1, 1));
   else
      root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

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

#include <parser/ParserNfcV.h>

void ParserNfcV::reset()
{
   ParserNfc::reset();
}

ProtocolFrame *ParserNfcV::parse(const lab::RawFrame &frame)
{
   ProtocolFrame *info = nullptr;

   if (frame.frameType() == lab::FrameType::NfcPollFrame)
   {
      do
      {
         // Inventory Command
         if ((info = parseRequestInventory(frame)))
            break;

         // Stay quiet command
         if ((info = parseRequestStayQuiet(frame)))
            break;

         // Read single block
         if ((info = parseRequestReadSingle(frame)))
            break;

         // Write single block
         if ((info = parseRequestWriteSingle(frame)))
            break;

         // Lock block
         if ((info = parseRequestLockBlock(frame)))
            break;

         // Read multiple blocks
         if ((info = parseRequestReadMultiple(frame)))
            break;

         // Write multiple blocks
         if ((info = parseRequestWriteMultiple(frame)))
            break;

         // Select
         if ((info = parseRequestSelect(frame)))
            break;

         // Reset to Ready
         if ((info = parseRequestResetReady(frame)))
            break;

         // Write AFI
         if ((info = parseRequestWriteAFI(frame)))
            break;

         // Lock AFI
         if ((info = parseRequestLockAFI(frame)))
            break;

         // Write DSFID
         if ((info = parseRequestWriteDSFID(frame)))
            break;

         // Lock DSFID
         if ((info = parseRequestLockDSFID(frame)))
            break;

         // System Information
         if ((info = parseRequestSysInfo(frame)))
            break;

         // Get multiple block security status
         if ((info = parseRequestGetSecurity(frame)))
            break;

         // generic NFC-V request frame...
         info = parseRequestGeneric(frame);

      } while (false);
   }
   else
   {
      do
      {
         // Inventory response
         if ((info = parseResponseInventory(frame)))
            break;

         // Read single block
         if ((info = parseResponseReadSingle(frame)))
            break;

         // Write single block
         if ((info = parseResponseWriteSingle(frame)))
            break;

         // Lock block
         if ((info = parseResponseLockBlock(frame)))
            break;

         // Read multiple blocks
         if ((info = parseResponseReadMultiple(frame)))
            break;

         // Write multiple blocks
         if ((info = parseResponseWriteMultiple(frame)))
            break;

         // Select
         if ((info = parseResponseSelect(frame)))
            break;

         // Reset to Ready
         if ((info = parseResponseResetReady(frame)))
            break;

         // Write AFI
         if ((info = parseResponseWriteAFI(frame)))
            break;

         // Lock AFI
         if ((info = parseResponseLockAFI(frame)))
            break;

         // Write DSFID
         if ((info = parseResponseWriteDSFID(frame)))
            break;

         // Lock DSFID
         if ((info = parseResponseLockDSFID(frame)))
            break;

         // System Information
         if ((info = parseResponseSysInfo(frame)))
            break;

         // Get multiple block security status
         if ((info = parseResponseGetSecurity(frame)))
            break;

         // generic NFC-V response frame...
         info = parseResponseGeneric(frame);

      } while (false);

      lastCommand = 0;
   }

   return info;
}

/*
 * Inventory
 * Command code = 0x01
 * When receiving the Inventory request, the VICC shall perform the anticollision sequence
 */
ProtocolFrame *ParserNfcV::parseRequestInventory(const lab::RawFrame &frame)
{
   if (frame[1] != 0x01)
      return nullptr;

   lastCommand = frame[1];

   int offset = 2;

   ProtocolFrame *root = buildRootInfo("Inventory", frame, ProtocolFrame::SelectionFrame);

   // parse request flags
   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));

   // if AFI flag is set parse family
   if ((frame[0] & 0x14) == 0x14)
      root->appendChild(buildApplicationFamily(frame, offset++));

   root->appendChild(buildChildInfo("MLEN", frame, offset, 1));

   int mlen = frame[offset++];

   if (mlen > 0)
      root->appendChild(buildChildInfo("MASK", frame, offset, (mlen & 0x7) ? 1 + (mlen >> 3) : (mlen >> 3)));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Inventory response
 */
ProtocolFrame *ParserNfcV::parseResponseInventory(const lab::RawFrame &frame)
{
   if (lastCommand != 0x01)
      return nullptr;

   ProtocolFrame *root = buildRootInfo("", frame, ProtocolFrame::SenseFrame);

   root->appendChild(buildResponseFlags(frame, 0));
   root->appendChild(buildChildInfo("DSFID", frame, 1, 1));
   root->appendChild(buildChildInfo("UID", frame, 2, 8));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Stay quiet command
 * When receiving the VICC shall enter the quiet state and shall NOT send back a response. There is NO response to the Stay quiet command
 */
ProtocolFrame *ParserNfcV::parseRequestStayQuiet(const lab::RawFrame &frame)
{
   if (frame[1] != 0x02)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildRootInfo("StayQuiet", frame, ProtocolFrame::SelectionFrame);

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));
   root->appendChild(buildChildInfo("UID", frame, 2, 8));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * There is NO response to the Stay quiet command.
 */
ProtocolFrame *ParserNfcV::parseResponseStayQuiet(const lab::RawFrame &frame)
{
   return nullptr;
}

/*
 * Read single block
 * Command code = 0x20
 * When receiving the Read single block command, the VICC shall read the requested block and send back its value in the response.
 */
ProtocolFrame *ParserNfcV::parseRequestReadSingle(const lab::RawFrame &frame)
{
   if (frame[1] != 0x20)
      return nullptr;

   lastCommand = frame[1];

   int offset = 2;

   ProtocolFrame *root = buildRootInfo("ReadBlock", frame, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildChildInfo("UID", frame, offset, 8));
      offset += 8;
   }

   root->appendChild(buildChildInfo("BLOCK", frame, offset, 1));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseReadSingle(const lab::RawFrame &frame)
{
   if (lastCommand != 0x20)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame, 1));
   else
      root->appendChild(buildChildInfo("DATA", frame, 1, frame.limit() - 3));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Write single block
 * Command code = 0x21
 * When receiving the Write single block command, the VICC shall write the requested block with the data contained in the request and report the success of the operation in the response.
 */
ProtocolFrame *ParserNfcV::parseRequestWriteSingle(const lab::RawFrame &frame)
{
   if (frame[1] != 0x21)
      return nullptr;

   lastCommand = frame[1];

   int offset = 2;

   ProtocolFrame *root = buildRootInfo("WriteBlock", frame, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildChildInfo("UID", frame, offset, 8));
      offset += 8;
   }

   root->appendChild(buildChildInfo("BLOCK", frame, offset++, 1));
   root->appendChild(buildChildInfo("DATA", frame, offset, frame.limit() - offset - 2));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseWriteSingle(const lab::RawFrame &frame)
{
   if (lastCommand != 0x21)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame, 1));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Command code = 0x22
 * When receiving the Lock block command, the VICC shall lock permanently the requested block.
 */
ProtocolFrame *ParserNfcV::parseRequestLockBlock(const lab::RawFrame &frame)
{
   if (frame[1] != 0x22)
      return nullptr;

   lastCommand = frame[1];

   int offset = 2;

   ProtocolFrame *root = buildRootInfo("LockBlock", frame, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildChildInfo("UID", frame, offset, 8));
      offset += 8;
   }

   root->appendChild(buildChildInfo("BLOCK", frame, offset++, 1));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;

}

ProtocolFrame *ParserNfcV::parseResponseLockBlock(const lab::RawFrame &frame)
{
   if (lastCommand != 0x22)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame, 1));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Read multiple blocks
 * Command code = 0x23
 * When receiving the Read multiple block command, the VICC shall read the requested block(s) and send back their value in the response.
 */
ProtocolFrame *ParserNfcV::parseRequestReadMultiple(const lab::RawFrame &frame)
{
   if (frame[1] != 0x23)
      return nullptr;

   lastCommand = frame[1];

   int offset = 2;

   ProtocolFrame *root = buildRootInfo("ReadBlocks", frame, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildChildInfo("UID", frame, offset, 8));
      offset += 8;
   }

   root->appendChild(buildChildInfo("FIRST", frame, offset++, 1));
   root->appendChild(buildChildInfo("COUNT", frame, offset++, 1));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseReadMultiple(const lab::RawFrame &frame)
{
   if (lastCommand != 0x23)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame, 1));
   else
      root->appendChild(buildChildInfo("DATA", frame, 1, frame.limit() - 3));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Write multiple blocks
 * Command code = 0x24
 * When receiving the Write multiple single block command, the VICC shall write the requested block(s) with the data contained in the request and report the success of the operation in the response
 */
ProtocolFrame *ParserNfcV::parseRequestWriteMultiple(const lab::RawFrame &frame)
{
   if (frame[1] != 0x24)
      return nullptr;

   lastCommand = frame[1];

   int offset = 2;

   ProtocolFrame *root = buildRootInfo("WriteBlocks", frame, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildChildInfo("UID", frame, offset, 8));
      offset += 8;
   }

   root->appendChild(buildChildInfo("FIRST", frame, offset++, 1));
   root->appendChild(buildChildInfo("COUNT", frame, offset++, 1));
   root->appendChild(buildChildInfo("DATA", frame, offset, frame.limit() - offset - 3));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseWriteMultiple(const lab::RawFrame &frame)
{
   if (lastCommand != 0x24)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame, 1));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Select
 * Command code = 0x25
 * When receiving the Select command:
 *   - if the UID is equal to its own UID, the VICC shall enter the selected state and shall send a response.
 *   - if it is different, the VICC shall return to the Ready state and shall not send a response.
 */
ProtocolFrame *ParserNfcV::parseRequestSelect(const lab::RawFrame &frame)
{
   if (frame[1] != 0x25)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildRootInfo("Select", frame, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));
   root->appendChild(buildChildInfo("UID", frame, 2, 8));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseSelect(const lab::RawFrame &frame)
{
   if (lastCommand != 0x25)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame, 1));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::parseRequestResetReady(const lab::RawFrame &frame)
{
   if (frame[1] != 0x26)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildRootInfo("Reset", frame, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));
   root->appendChild(buildChildInfo("UID", frame, 2, 8));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseResetReady(const lab::RawFrame &frame)
{
   if (lastCommand != 0x26)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame, 1));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Write AFI
 * Command code = 0x27
 * When receiving the Write AFI request, the VICC shall write the AFI value into its memory.
 */
ProtocolFrame *ParserNfcV::parseRequestWriteAFI(const lab::RawFrame &frame)
{
   if (frame[1] != 0x27)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildRootInfo("WriteAFI", frame, ProtocolFrame::ApplicationFrame);

   int offset = 2;

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildChildInfo("UID", frame, offset, 8));
      offset += 8;
   }

   root->appendChild(buildApplicationFamily(frame, offset++));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;

}

ProtocolFrame *ParserNfcV::parseResponseWriteAFI(const lab::RawFrame &frame)
{
   if (lastCommand != 0x27)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame, 1));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Lock AFI
 * Command code = 0x28
 * When receiving the Lock AFI request, the VICC shall lock the AFI value permanently into its memory.
 */
ProtocolFrame *ParserNfcV::parseRequestLockAFI(const lab::RawFrame &frame)
{
   if (frame[1] != 0x28)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildRootInfo("LockAFI", frame, ProtocolFrame::ApplicationFrame);

   int offset = 2;

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildChildInfo("UID", frame, offset, 8));
      offset += 8;
   }

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;

}

ProtocolFrame *ParserNfcV::parseResponseLockAFI(const lab::RawFrame &frame)
{
   if (lastCommand != 0x28)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame, 1));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Write DSFID command
 * Command code = 0x29
 * When receiving the Write DSFID request, the VICC shall write the DSFID value into its memory
 */
ProtocolFrame *ParserNfcV::parseRequestWriteDSFID(const lab::RawFrame &frame)
{
   if (frame[1] != 0x29)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildRootInfo("WriteDSFID", frame, ProtocolFrame::ApplicationFrame);

   int offset = 2;

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildChildInfo("UID", frame, offset, 8));
      offset += 8;
   }

   root->appendChild(buildChildInfo("DSFID", frame, -2, 2));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;

}

ProtocolFrame *ParserNfcV::parseResponseWriteDSFID(const lab::RawFrame &frame)
{
   if (lastCommand != 0x29)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame, 1));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Lock AFI
 * Command code = 0x28
 * When receiving the Lock AFI request, the VICC shall lock the AFI value permanently into its memory.
 */
ProtocolFrame *ParserNfcV::parseRequestLockDSFID(const lab::RawFrame &frame)
{
   if (frame[1] != 0x2A)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildRootInfo("LockDSFID", frame, ProtocolFrame::ApplicationFrame);

   int offset = 2;

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildChildInfo("UID", frame, offset, 8));
      offset += 8;
   }

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseLockDSFID(const lab::RawFrame &frame)
{
   if (lastCommand != 0x2A)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame, 1));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Get system information
 * Command code = 0x2B
 * This command allows for retrieving the system information value from the VICC.
 */
ProtocolFrame *ParserNfcV::parseRequestSysInfo(const lab::RawFrame &frame)
{
   if (frame[1] != 0x2B)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildRootInfo("SysInfo", frame, ProtocolFrame::ApplicationFrame);

   int offset = 2;

   root->appendChild(buildRequestFlags(frame, 0));
   root->appendChild(buildChildInfo("CMD", frame, 1, 1));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildChildInfo("UID", frame, offset, 8));
      offset += 8;
   }

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseSysInfo(const lab::RawFrame &frame)
{
   if (lastCommand != 0x2B)
      return nullptr;

   int flags = frame[0];
   int info = frame[1];
   int offset = 10;

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if ((flags & 1) == 0)
   {
      ProtocolFrame *ainfo = root->appendChild(buildChildInfo("INFO", frame, 1, 1));

      if (info & 0x01)
         ainfo->appendChild(buildChildInfo("[.......1] DSFID is supported. DSFID field is present"));
      else
         ainfo->appendChild(buildChildInfo("[.......0] DSFID is not supported. DSFID field is not present"));

      if (info & 0x02)
         ainfo->appendChild(buildChildInfo("[......1.] AFI is supported. AFI field is present"));
      else
         ainfo->appendChild(buildChildInfo("[......0.] AFI is not supported. AFI field is not present"));

      if (info & 0x04)
         ainfo->appendChild(buildChildInfo("[.....1..] Information on VICC memory size is supported. Memory size field is present"));
      else
         ainfo->appendChild(buildChildInfo("[.....0..] Information on VICC memory size is not supported. Memory size field is not present"));

      if (info & 0x08)
         ainfo->appendChild(buildChildInfo("[....1...] Information on IC reference is supported. IC reference field is present"));
      else
         ainfo->appendChild(buildChildInfo("[....0...] Information on IC reference is not supported. IC reference field is not present"));

      ainfo->appendChild(buildChildInfo(QString("[%1....] Reserved for future use").arg((info >> 4) & 0x0f, 4, 2, QChar('0'))));

      // tag UID
      root->appendChild(buildChildInfo("UID", frame, 2, 8));

      // DSFID field is present
      if (info & 0x01)
         root->appendChild(buildChildInfo("DSFID", frame, offset++, 1));

      // AFI field is present
      if (info & 0x02)
         root->appendChild(buildApplicationFamily(frame, offset++));

      // Memory size field is present
      if (info & 0x04)
      {
         ProtocolFrame *amem = root->appendChild(buildChildInfo("MEMORY", frame, offset, 2));

         int count = frame[offset++];
         int size = frame[offset++] & 0x1f;

         amem->appendChild(buildChildInfo(QString("[%1] Number of blocks %2").arg(count, 8, 2, QChar('0')).arg(count)));
         amem->appendChild(buildChildInfo(QString("[...%1] Block size %2 bytes").arg(size, 5, 2, QChar('0')).arg(size)));
      }

      // IC reference field is not present
      if (info & 0x08)
         root->appendChild(buildChildInfo("IC", frame, offset, 1));
   }
   else
   {
      root->appendChild(buildResponseError(frame, 1));
   }

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

/*
 * Get multiple block security status
 * Command code = 0x2C
 * When receiving the Get multiple block security status command, the VICC shall send back the block security status.
 */
ProtocolFrame *ParserNfcV::parseRequestGetSecurity(const lab::RawFrame &frame)
{
   if (frame[1] != 0x2C)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildRootInfo("GetSecurity", frame, ProtocolFrame::ApplicationFrame);

   int offset = 2;

   root->appendChild(buildRequestFlags(frame, 0));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildChildInfo("UID", frame, offset, 8));
      offset += 8;
   }

   root->appendChild(buildChildInfo("FIRST", frame, offset++, 1));
   root->appendChild(buildChildInfo("COUNT", frame, offset++, 1));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseGetSecurity(const lab::RawFrame &frame)
{
   if (lastCommand != 0x2C)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));
   root->appendChild(buildChildInfo("DATA", frame, 1, frame.limit() - 3));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::parseRequestGeneric(const lab::RawFrame &frame)
{
   int cmd = frame[1]; // frame command

   QString name = QString("CMD %1").arg(cmd, 2, 16, QChar('0'));

   ProtocolFrame *root = buildRootInfo(name, frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));
   root->appendChild(buildChildInfo("CODE", frame, 1, 1));
   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseGeneric(const lab::RawFrame &frame)
{
   int flags = frame[0];

   ProtocolFrame *root = buildRootInfo("", frame, 0);

   root->appendChild(buildResponseFlags(frame, 0));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame, 1));
   else
      root->appendChild(buildChildInfo("PARAMS", frame, 1, frame.limit() - 3));

   root->appendChild(buildChildInfo("CRC", frame, -2, 2));

   return root;
}

ProtocolFrame *ParserNfcV::buildRequestFlags(const lab::RawFrame &frame, int offset)
{
   int flags = frame[offset];

   ProtocolFrame *afrf = buildChildInfo("FLAGS", frame, offset, 1);

   if (flags & 0x01)
      afrf->appendChild(buildChildInfo("[.......1] Two sub-carriers shall be used by the VICC"));
   else
      afrf->appendChild(buildChildInfo("[.......0] A single sub-carrier frequency shall be used by the VICC"));

   if (flags & 0x02)
      afrf->appendChild(buildChildInfo("[......1.] High data rate shall be used"));
   else
      afrf->appendChild(buildChildInfo("[......0.] Low data rate shall be used"));

   if (flags & 0x08)
      afrf->appendChild(buildChildInfo("[....1...] Protocol format is extended"));
   else
      afrf->appendChild(buildChildInfo("[....0...] No protocol format extension"));

   if (flags & 0x04)
   {
      if (flags & 0x10)
         afrf->appendChild(buildChildInfo("[...1.1..] AFI field is present"));
      else
         afrf->appendChild(buildChildInfo("[...0.1..] AFI field is not present"));

      if (flags & 0x20)
         afrf->appendChild(buildChildInfo("[..1..1..] 1 slot"));
      else
         afrf->appendChild(buildChildInfo("[..0..1..] 16 slots"));

      afrf->appendChild(buildChildInfo(QString("[.%1...1..] Custom flag. Meaning is defined by the Custom command").arg((flags >> 6) & 1, 1, 2, QChar('0'))));
      afrf->appendChild(buildChildInfo(QString("[%1....1..] Reserved for future use").arg((flags >> 7) & 1, 1, 2, QChar('0'))));
   }
   else
   {
      if (flags & 0x10)
         afrf->appendChild(buildChildInfo("[...1.0..] Request shall be executed only by VICC in selected state"));
      else
         afrf->appendChild(buildChildInfo("[...0.0..] Request shall be executed by any VICC according to the setting of Address flag"));

      if (flags & 0x20)
         afrf->appendChild(buildChildInfo("[..1..0..] Request is addressed. UID field is present. It shall be executed only by the VICC whose UID matches"));
      else
         afrf->appendChild(buildChildInfo("[..0..0..] Request is not addressed. UID field is not present. It shall be executed by any VICC"));

      afrf->appendChild(buildChildInfo(QString("[.%1...0..] Custom flag. Meaning is defined by the Custom command").arg((flags >> 6) & 1, 1, 2, QChar('0'))));
      afrf->appendChild(buildChildInfo(QString("[%1....0..] Reserved for future use").arg((flags >> 7) & 1, 1, 2, QChar('0'))));
   }

   return afrf;
}

ProtocolFrame *ParserNfcV::buildResponseFlags(const lab::RawFrame &frame, int offset)
{
   int flags = frame[offset];

   ProtocolFrame *afrf = buildChildInfo("FLAGS", frame, offset, 1);

   if (flags & 0x01)
      afrf->appendChild(buildChildInfo("[.......1] Error detected. Error code is in the error field"));
   else
      afrf->appendChild(buildChildInfo("[.......0] No error"));

   afrf->appendChild(buildChildInfo(QString("[.....%1.] Reserved for future use").arg((flags >> 1) & 0x03, 2, 2, QChar('0'))));

   if (flags & 0x08)
      afrf->appendChild(buildChildInfo("[....1...] Protocol format is extended"));
   else
      afrf->appendChild(buildChildInfo("[....0...] No protocol format extension"));

   afrf->appendChild(buildChildInfo(QString("[%1....] Reserved for future use").arg((flags >> 4) & 0x0f, 4, 2, QChar('0'))));

   return afrf;
}

ProtocolFrame *ParserNfcV::buildResponseError(const lab::RawFrame &frame, int offset)
{
   int error = frame[offset];

   ProtocolFrame *aerr = buildChildInfo("ERROR", frame, offset, 1);

   if (error == 0x01)
      aerr->appendChild(buildChildInfo(QString("[%1] The command is not supported").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x02)
      aerr->appendChild(buildChildInfo(QString("[%1] The command is not recognized").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x0f)
      aerr->appendChild(buildChildInfo(QString("[%1] Unknown error").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x10)
      aerr->appendChild(buildChildInfo(QString("[%1] The specified block is not available").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x11)
      aerr->appendChild(buildChildInfo(QString("[%1] The specified block is already locked").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x12)
      aerr->appendChild(buildChildInfo(QString("[%1] The specified block is locked and its content cannot be changed").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x13)
      aerr->appendChild(buildChildInfo(QString("[%1] The specified block was not successfully programmed").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x14)
      aerr->appendChild(buildChildInfo(QString("[%1] The specified block was not successfully locked").arg(error, 8, 2, QChar('0'))));
   else
      aerr->appendChild(buildChildInfo(QString("[%1] Custom command error code").arg(error, 8, 2, QChar('0'))));

   return aerr;
}

ProtocolFrame *ParserNfcV::buildApplicationFamily(const lab::RawFrame &frame, int offset)
{
   int afi = frame[offset];

   ProtocolFrame *afif = buildChildInfo("AFI", frame, offset, 1);

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
   else if ((afi & 0xf0) == 0x90)
      afif->appendChild(buildChildInfo(QString("[1001%1] Item management sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
   else if ((afi & 0xf0) == 0xA0)
      afif->appendChild(buildChildInfo(QString("[1010%1] Express parcels sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
   else if ((afi & 0xf0) == 0xB0)
      afif->appendChild(buildChildInfo(QString("[1011%1] Postal services sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
   else if ((afi & 0xf0) == 0xC0)
      afif->appendChild(buildChildInfo(QString("[1100%1] Airline bags sub-family %2").arg((afi & 0xf), 4, 2, QChar('0')).arg(afi & 0xf)));
   else
      afif->appendChild(buildChildInfo(QString("[%1] RFU %2").arg(afi, 8, 2, QChar('0')).arg(afi)));

   return afif;
}

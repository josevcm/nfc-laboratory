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
ProtocolFrame *ParserNfcV::parseRequestInventory(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x01)
      return nullptr;

   lastCommand = frame[1];

   int offset = 2;

   ProtocolFrame *root = buildFrameInfo("Inventory", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::SelectionFrame);

   // parse request flags
   root->appendChild(buildRequestFlags(frame[0]));

   // if AFI flag is set parse family
   if ((frame[0] & 0x14) == 0x14)
      root->appendChild(buildApplicationFamily(frame[offset++]));

   int mlen = frame[offset++];

   root->appendChild(buildFieldInfo("MLEN", QString("%1").arg(mlen, 2, 16, QChar('0'))));

   if (mlen > 0)
      root->appendChild(buildFieldInfo("MASK", toByteArray(frame, offset, (mlen & 0x7) ? 1 + (mlen >> 3) : (mlen >> 3))));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Inventory response
 */
ProtocolFrame *ParserNfcV::parseResponseInventory(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x01)
      return nullptr;

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::SenseFrame);

   root->appendChild(buildResponseFlags(frame[0]));
   root->appendChild(buildFieldInfo("DSFID", QString("%1").arg(frame[1], 2, 16, QChar('0'))));
   root->appendChild(buildFieldInfo("UID", toByteArray(frame, 2, 8)));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Stay quiet command
 * When receiving the VICC shall enter the quiet state and shall NOT send back a response. There is NO response to the Stay quiet command
 */
ProtocolFrame *ParserNfcV::parseRequestStayQuiet(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x02)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("StayQuiet", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::SelectionFrame);

   root->appendChild(buildRequestFlags(frame[0]));
   root->appendChild(buildFieldInfo("UID", toByteArray(frame, 2, 8)));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * There is NO response to the Stay quiet command.
 */
ProtocolFrame *ParserNfcV::parseResponseStayQuiet(const nfc::NfcFrame &frame)
{
   return nullptr;
}

/*
 * Read single block
 * Command code = 0x20
 * When receiving the Read single block command, the VICC shall read the requested block and send back its value in the response.
 */
ProtocolFrame *ParserNfcV::parseRequestReadSingle(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x20)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("ReadBlock", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame[0]));

   int offset = 2;

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildFieldInfo("UID", toByteArray(frame, offset, 8)));
      offset += 8;
   }

   root->appendChild(buildFieldInfo("BLOCK", QString("%1").arg(frame[offset], 2, 16, QChar('0'))));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseReadSingle(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x20)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame[1]));
   else
      root->appendChild(buildFieldInfo("DATA", toByteArray(frame, 1, frame.limit() - 3)));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Write single block
 * Command code = 0x21
 * When receiving the Write single block command, the VICC shall write the requested block with the data contained in the request and report the success of the operation in the response.
 */
ProtocolFrame *ParserNfcV::parseRequestWriteSingle(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x21)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("WriteBlock", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame[0]));

   int offset = 2;

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildFieldInfo("UID", toByteArray(frame, offset, 8)));
      offset += 8;
   }

   root->appendChild(buildFieldInfo("BLOCK", QString("%1").arg(frame[offset++], 2, 16, QChar('0'))));
   root->appendChild(buildFieldInfo("DATA", toByteArray(frame, offset, frame.limit() - offset - 2)));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseWriteSingle(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x21)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame[1]));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Command code = 0x22
 * When receiving the Lock block command, the VICC shall lock permanently the requested block.
 */
ProtocolFrame *ParserNfcV::parseRequestLockBlock(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x22)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("LockBlock", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame[0]));

   int offset = 2;

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildFieldInfo("UID", toByteArray(frame, offset, 8)));
      offset += 8;
   }

   root->appendChild(buildFieldInfo("BLOCK", QString("%1").arg(frame[offset++], 2, 16, QChar('0'))));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;

}

ProtocolFrame *ParserNfcV::parseResponseLockBlock(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x22)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame[1]));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Read multiple blocks
 * Command code = 0x23
 * When receiving the Read multiple block command, the VICC shall read the requested block(s) and send back their value in the response.
 */
ProtocolFrame *ParserNfcV::parseRequestReadMultiple(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x23)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("ReadBlocks", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame[0]));

   int offset = 2;

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildFieldInfo("UID", toByteArray(frame, offset, 8)));
      offset += 8;
   }

   root->appendChild(buildFieldInfo("FIRST", QString("%1").arg(frame[offset++], 2, 16, QChar('0'))));
   root->appendChild(buildFieldInfo("COUNT", QString("%1").arg(frame[offset++], 2, 16, QChar('0'))));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseReadMultiple(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x23)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame[1]));
   else
      root->appendChild(buildFieldInfo("DATA", toByteArray(frame, 1, frame.limit() - 3)));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Write multiple blocks
 * Command code = 0x24
 * When receiving the Write multiple single block command, the VICC shall write the requested block(s) with the data contained in the request and report the success of the operation in the response
 */
ProtocolFrame *ParserNfcV::parseRequestWriteMultiple(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x24)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("WriteBlocks", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame[0]));

   int offset = 2;

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildFieldInfo("UID", toByteArray(frame, offset, 8)));
      offset += 8;
   }

   root->appendChild(buildFieldInfo("FIRST", QString("%1").arg(frame[offset++], 2, 16, QChar('0'))));
   root->appendChild(buildFieldInfo("COUNT", QString("%1").arg(frame[offset++], 2, 16, QChar('0'))));
   root->appendChild(buildFieldInfo("DATA", toByteArray(frame, offset, frame.limit() - offset - 3)));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseWriteMultiple(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x24)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame[1]));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Select
 * Command code = 0x25
 * When receiving the Select command:
 *   - if the UID is equal to its own UID, the VICC shall enter the selected state and shall send a response.
 *   - if it is different, the VICC shall return to the Ready state and shall not send a response.
 */
ProtocolFrame *ParserNfcV::parseRequestSelect(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x25)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("Select", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame[0]));
   root->appendChild(buildFieldInfo("UID", toByteArray(frame, 2, 8)));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseSelect(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x25)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame[1]));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseRequestResetReady(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x26)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("Reset", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   root->appendChild(buildRequestFlags(frame[0]));
   root->appendChild(buildFieldInfo("UID", toByteArray(frame, 2, 8)));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseResetReady(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x26)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame[1]));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Write AFI
 * Command code = 0x27
 * When receiving the Write AFI request, the VICC shall write the AFI value into its memory.
 */
ProtocolFrame *ParserNfcV::parseRequestWriteAFI(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x27)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("WriteAFI", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   int offset = 2;

   root->appendChild(buildRequestFlags(frame[0]));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildFieldInfo("UID", toByteArray(frame, offset, 8)));
      offset += 8;
   }

   root->appendChild(buildApplicationFamily(frame[offset++]));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;

}

ProtocolFrame *ParserNfcV::parseResponseWriteAFI(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x27)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame[1]));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Lock AFI
 * Command code = 0x28
 * When receiving the Lock AFI request, the VICC shall lock the AFI value permanently into its memory.
 */
ProtocolFrame *ParserNfcV::parseRequestLockAFI(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x28)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("LockAFI", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   int offset = 2;

   root->appendChild(buildRequestFlags(frame[0]));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildFieldInfo("UID", toByteArray(frame, offset, 8)));
      offset += 8;
   }

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;

}

ProtocolFrame *ParserNfcV::parseResponseLockAFI(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x28)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame[1]));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Write DSFID command
 * Command code = 0x29
 * When receiving the Write DSFID request, the VICC shall write the DSFID value into its memory
 */
ProtocolFrame *ParserNfcV::parseRequestWriteDSFID(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x29)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("WriteDSFID", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   int offset = 2;

   root->appendChild(buildRequestFlags(frame[0]));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildFieldInfo("UID", toByteArray(frame, offset, 8)));
      offset += 8;
   }

   root->appendChild(buildFieldInfo("DSFID", toByteArray(frame, -2)));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;

}

ProtocolFrame *ParserNfcV::parseResponseWriteDSFID(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x29)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame[1]));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Lock AFI
 * Command code = 0x28
 * When receiving the Lock AFI request, the VICC shall lock the AFI value permanently into its memory.
 */
ProtocolFrame *ParserNfcV::parseRequestLockDSFID(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x2A)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("LockDSFID", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   int offset = 2;

   root->appendChild(buildRequestFlags(frame[0]));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildFieldInfo("UID", toByteArray(frame, offset, 8)));
      offset += 8;
   }

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseLockDSFID(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x2A)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame[1]));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Get system information
 * Command code = 0x2B
 * This command allows for retrieving the system information value from the VICC.
 */
ProtocolFrame *ParserNfcV::parseRequestSysInfo(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x2B)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("SysInfo", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   int offset = 2;

   root->appendChild(buildRequestFlags(frame[0]));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildFieldInfo("UID", toByteArray(frame, offset, 8)));
      offset += 8;
   }

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseSysInfo(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x2B)
      return nullptr;

   int flags = frame[0];
   int info = frame[1];
   int offset = 10;

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if ((flags & 1) == 0)
   {
      ProtocolFrame *ainfo = root->appendChild(buildFieldInfo("INFO", QString("%1").arg(info, 2, 16, QChar('0'))));

      if (info & 0x01)
         ainfo->appendChild(buildFieldInfo("[.......1] DSFID is supported. DSFID field is present"));
      else
         ainfo->appendChild(buildFieldInfo("[.......0] DSFID is not supported. DSFID field is not present"));

      if (info & 0x02)
         ainfo->appendChild(buildFieldInfo("[......1.] AFI is supported. AFI field is present"));
      else
         ainfo->appendChild(buildFieldInfo("[......0.] AFI is not supported. AFI field is not present"));

      if (info & 0x04)
         ainfo->appendChild(buildFieldInfo("[.....1..] Information on VICC memory size is supported. Memory size field is present"));
      else
         ainfo->appendChild(buildFieldInfo("[.....0..] Information on VICC memory size is not supported. Memory size field is not present"));

      if (info & 0x08)
         ainfo->appendChild(buildFieldInfo("[....1...] Information on IC reference is supported. IC reference field is present"));
      else
         ainfo->appendChild(buildFieldInfo("[....0...] Information on IC reference is not supported. IC reference field is not present"));

      ainfo->appendChild(buildFieldInfo(QString("[%1....] Reserved for future use").arg((info >> 4) & 0x0f, 4, 2, QChar('0'))));

      // tag UID
      root->appendChild(buildFieldInfo("UID", toByteArray(frame, 2, 8)));

      // DSFID field is present
      if (info & 0x01)
         root->appendChild(buildFieldInfo("DSFID", QString("%1").arg(frame[offset++], 2, 16, QChar('0'))));

      // AFI field is present
      if (info & 0x02)
         root->appendChild(buildApplicationFamily(frame[offset++]));

      // Memory size field is present
      if (info & 0x04)
      {
         ProtocolFrame *amem = root->appendChild(buildFieldInfo("MEMORY", toByteArray(frame, offset, 2)));

         int count = frame[offset++];
         int size = frame[offset++] & 0x1f;

         amem->appendChild(buildFieldInfo(QString("[%1] Number of blocks %2").arg(count, 8, 2, QChar('0')).arg(count)));
         amem->appendChild(buildFieldInfo(QString("[...%1] Block size %2 bytes").arg(size, 5, 2, QChar('0')).arg(size)));
      }

      // IC reference field is not present
      if (info & 0x08)
         root->appendChild(buildFieldInfo("IC", toByteArray(frame, offset, 1)));
   }
   else
   {
      root->appendChild(buildResponseError(frame[1]));
   }

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

/*
 * Get multiple block security status
 * Command code = 0x2C
 * When receiving the Get multiple block security status command, the VICC shall send back the block security status.
 */
ProtocolFrame *ParserNfcV::parseRequestGetSecurity(const nfc::NfcFrame &frame)
{
   if (frame[1] != 0x2C)
      return nullptr;

   lastCommand = frame[1];

   ProtocolFrame *root = buildFrameInfo("GetSecurity", frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, ProtocolFrame::ApplicationFrame);

   int offset = 2;

   root->appendChild(buildRequestFlags(frame[0]));

   // if UID flag is set parse address
   if ((frame[0] & 0x24) == 0x20)
   {
      root->appendChild(buildFieldInfo("UID", toByteArray(frame, offset, 8)));
      offset += 8;
   }

   root->appendChild(buildFieldInfo("FIRST", QString("%1").arg(frame[offset++], 2, 16, QChar('0'))));
   root->appendChild(buildFieldInfo("COUNT", QString("%1").arg(frame[offset++], 2, 16, QChar('0'))));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseGetSecurity(const nfc::NfcFrame &frame)
{
   if (lastCommand != 0x2C)
      return nullptr;

   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));
   root->appendChild(buildFieldInfo("DATA", toByteArray(frame, 1, frame.limit() - 3)));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseRequestGeneric(const nfc::NfcFrame &frame)
{
   int cmd = frame[1]; // frame command

   ProtocolFrame *root = buildFrameInfo(QString("CMD %1").arg(cmd, 2, 16, QChar('0')), frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(frame[0]));
   root->appendChild(buildFieldInfo("CODE", QString("%1 [%2]").arg(cmd, 2, 16, QChar('0')).arg(cmd, 8, 2, QChar('0'))));
   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::parseResponseGeneric(const nfc::NfcFrame &frame)
{
   int flags = frame[0];

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0, 0);

   root->appendChild(buildResponseFlags(flags));

   if (flags & 0x1)
      root->appendChild(buildResponseError(frame[1]));
   else
      root->appendChild(buildFieldInfo("PARAMS", toByteArray(frame, 1, frame.limit() - 3)));

   root->appendChild(buildFieldInfo("CRC", toByteArray(frame, -2)));

   return root;
}

ProtocolFrame *ParserNfcV::buildRequestFlags(int flags)
{
   ProtocolFrame *afrf = buildFieldInfo("FLAGS", QString("%1").arg(flags, 2, 16, QChar('0')));

   if (flags & 0x01)
      afrf->appendChild(buildFieldInfo("[.......1] Two sub-carriers shall be used by the VICC"));
   else
      afrf->appendChild(buildFieldInfo("[.......0] A single sub-carrier frequency shall be used by the VICC"));

   if (flags & 0x02)
      afrf->appendChild(buildFieldInfo("[......1.] High data rate shall be used"));
   else
      afrf->appendChild(buildFieldInfo("[......0.] Low data rate shall be used"));

   if (flags & 0x08)
      afrf->appendChild(buildFieldInfo("[....1...] Protocol format is extended"));
   else
      afrf->appendChild(buildFieldInfo("[....0...] No protocol format extension"));

   if (flags & 0x04)
   {
      if (flags & 0x10)
         afrf->appendChild(buildFieldInfo("[...1.1..] AFI field is present"));
      else
         afrf->appendChild(buildFieldInfo("[...0.1..] AFI field is not present"));

      if (flags & 0x20)
         afrf->appendChild(buildFieldInfo("[..1..1..] 1 slot"));
      else
         afrf->appendChild(buildFieldInfo("[..0..1..] 16 slots"));

      afrf->appendChild(buildFieldInfo(QString("[.%1...1..] Custom flag. Meaning is defined by the Custom command").arg((flags >> 6) & 1, 1, 2, QChar('0'))));
      afrf->appendChild(buildFieldInfo(QString("[%1....1..] Reserved for future use").arg((flags >> 7) & 1, 1, 2, QChar('0'))));
   }
   else
   {
      if (flags & 0x10)
         afrf->appendChild(buildFieldInfo("[...1.0..] Request shall be executed only by VICC in selected state"));
      else
         afrf->appendChild(buildFieldInfo("[...0.0..] Request shall be executed by any VICC according to the setting of Address flag"));

      if (flags & 0x20)
         afrf->appendChild(buildFieldInfo("[..1..0..] Request is addressed. UID field is present. It shall be executed only by the VICC whose UID matches"));
      else
         afrf->appendChild(buildFieldInfo("[..0..0..] Request is not addressed. UID field is not present. It shall be executed by any VICC"));

      afrf->appendChild(buildFieldInfo(QString("[.%1...0..] Custom flag. Meaning is defined by the Custom command").arg((flags >> 6) & 1, 1, 2, QChar('0'))));
      afrf->appendChild(buildFieldInfo(QString("[%1....0..] Reserved for future use").arg((flags >> 7) & 1, 1, 2, QChar('0'))));
   }

   return afrf;
}

ProtocolFrame *ParserNfcV::buildResponseFlags(int flags)
{
   ProtocolFrame *afrf = buildFieldInfo("FLAGS", QString("%1").arg(flags, 2, 16, QChar('0')));

   if (flags & 0x01)
      afrf->appendChild(buildFieldInfo("[.......1] Error detected. Error code is in the error field"));
   else
      afrf->appendChild(buildFieldInfo("[.......0] No error"));

   afrf->appendChild(buildFieldInfo(QString("[.....%1.] Reserved for future use").arg((flags >> 1) & 0x03, 2, 2, QChar('0'))));

   if (flags & 0x08)
      afrf->appendChild(buildFieldInfo("[....1...] Protocol format is extended"));
   else
      afrf->appendChild(buildFieldInfo("[....0...] No protocol format extension"));

   afrf->appendChild(buildFieldInfo(QString("[%1....] Reserved for future use").arg((flags >> 4) & 0x0f, 4, 2, QChar('0'))));

   return afrf;
}

ProtocolFrame *ParserNfcV::buildResponseError(int error)
{
   ProtocolFrame *aerr = buildFieldInfo("ERROR", QString("%1").arg(error, 2, 16, QChar('0')));

   if (error == 0x01)
      aerr->appendChild(buildFieldInfo(QString("[%1] The command is not supported").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x02)
      aerr->appendChild(buildFieldInfo(QString("[%1] The command is not recognized").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x0f)
      aerr->appendChild(buildFieldInfo(QString("[%1] Unknown error").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x10)
      aerr->appendChild(buildFieldInfo(QString("[%1] The specified block is not available").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x11)
      aerr->appendChild(buildFieldInfo(QString("[%1] The specified block is already locked").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x12)
      aerr->appendChild(buildFieldInfo(QString("[%1] The specified block is locked and its content cannot be changed").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x13)
      aerr->appendChild(buildFieldInfo(QString("[%1] The specified block was not successfully programmed").arg(error, 8, 2, QChar('0'))));
   else if (error == 0x14)
      aerr->appendChild(buildFieldInfo(QString("[%1] The specified block was not successfully locked").arg(error, 8, 2, QChar('0'))));
   else
      aerr->appendChild(buildFieldInfo(QString("[%1] Custom command error code").arg(error, 8, 2, QChar('0'))));

   return aerr;
}

ProtocolFrame *ParserNfcV::buildApplicationFamily(int afi)
{
   ProtocolFrame *afif = buildFieldInfo("AFI", QString("%1").arg(afi, 2, 16, QChar('0')));

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

   return afif;
}

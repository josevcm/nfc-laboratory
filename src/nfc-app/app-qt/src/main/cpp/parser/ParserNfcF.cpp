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

#include <parser/ParserNfcF.h>

void ParserNfcF::reset()
{
   ParserNfc::reset();
}

ProtocolFrame *ParserNfcF::parse(const nfc::NfcFrame &frame)
{
   ProtocolFrame *info = nullptr;

   if (frame.isPollFrame())
   {
      do
      {
         // Request Command
         if ((info = parseRequestREQC(frame)))
            break;

         // generic NFC-F request frame...
         info = parseRequestGeneric(frame);

      } while (false);
   }
   else
   {
      do
      {
         // Request Command
         if ((info = parseResponseREQC(frame)))
            break;

         // generic NFC-F request frame...
         info = parseResponseGeneric(frame);

      } while (false);

      lastCommand = 0;
   }

   return info;
}

ProtocolFrame *ParserNfcF::parseRequestREQC(const nfc::NfcFrame &frame)
{
   return nullptr;
}

ProtocolFrame *ParserNfcF::parseResponseREQC(const nfc::NfcFrame &frame)
{
   return nullptr;
}

ProtocolFrame *ParserNfcF::parseRequestGeneric(const nfc::NfcFrame &frame)
{
   int cmd = frame[1]; // frame command
   int flags = 0;

   flags |= frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0;
   flags |= frame.hasSyncError() ? ProtocolFrame::Flags::SyncError : 0;

   ProtocolFrame *root = buildFrameInfo(QString("CMD %1").arg(cmd, 2, 16, QChar('0')), frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), flags, 0);

   return root;
}

ProtocolFrame *ParserNfcF::parseResponseGeneric(const nfc::NfcFrame &frame)
{
   int flags = 0;

   flags |= frame.hasCrcError() ? ProtocolFrame::Flags::CrcError : 0;
   flags |= frame.hasSyncError() ? ProtocolFrame::Flags::SyncError : 0;

   ProtocolFrame *root = buildFrameInfo(frame.frameRate(), toByteArray(frame), frame.timeStart(), frame.timeEnd(), flags, 0);

   return root;
}

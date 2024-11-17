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

#include <parser/ParserNfcF.h>

void ParserNfcF::reset()
{
   ParserNfc::reset();
}

ProtocolFrame *ParserNfcF::parse(const lab::RawFrame &frame)
{
   ProtocolFrame *info = nullptr;

   if (frame.frameType() == lab::FrameType::NfcPollFrame)
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

ProtocolFrame *ParserNfcF::parseRequestREQC(const lab::RawFrame &frame)
{
   return nullptr;
}

ProtocolFrame *ParserNfcF::parseResponseREQC(const lab::RawFrame &frame)
{
   return nullptr;
}

ProtocolFrame *ParserNfcF::parseRequestGeneric(const lab::RawFrame &frame)
{
   int cmd = frame[1]; // frame command

   QString name = QString("CMD %1").arg(cmd, 2, 16, QChar('0'));

   ProtocolFrame *root = buildRootInfo(name, frame, 0);

   return root;
}

ProtocolFrame *ParserNfcF::parseResponseGeneric(const lab::RawFrame &frame)
{
   ProtocolFrame *root = buildRootInfo("", frame, 0);

   return root;
}

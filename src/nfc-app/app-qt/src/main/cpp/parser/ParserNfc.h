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

#ifndef NFC_LAB_PARSERNFC_H
#define NFC_LAB_PARSERNFC_H

#include <lab/data/RawFrame.h>

#include <protocol/ProtocolFrame.h>

#include <parser/Parser.h>

struct ParserNfc : Parser
{
   enum Caps
   {
      IS_APDU = 1
   };

   unsigned int lastCommand = 0;

   virtual void reset();

   virtual ProtocolFrame *parse(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestUnknown(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseUnknown(const lab::RawFrame &frame);

   ProtocolFrame *parseAPDU(const QString &name, const lab::RawFrame &frame, int start, int length);

   bool isApdu(const QByteArray &apdu);
};

struct ParserNfcIsoDep : ParserNfc
{
   void reset() override;

   ProtocolFrame *parse(const lab::RawFrame &frame) override;

   ProtocolFrame *parseIBlock(const lab::RawFrame &frame);

   ProtocolFrame *parseRBlock(const lab::RawFrame &frame);

   ProtocolFrame *parseSBlock(const lab::RawFrame &frame);
};

#endif //NFC_LAB_PARSERNFC_H

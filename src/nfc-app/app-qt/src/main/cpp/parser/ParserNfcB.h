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

#ifndef NFC_LAB_PARSERNFCB_H
#define NFC_LAB_PARSERNFCB_H

#include <parser/ParserNfc.h>

struct ParserNfcB : ParserNfcIsoDep
{
   void reset() override;

   ProtocolFrame *parse(const lab::RawFrame &frame) override;

   ProtocolFrame *parseRequestREQB(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseREQB(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestINIT(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseINIT(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestREAD(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseREAD(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestWRITE(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseWRITE(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestGETUID(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseGETUID(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestSELECT(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseSELECT(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestATTRIB(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseATTRIB(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestHLTB(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseHLTB(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestVASUP(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseVASUP(const lab::RawFrame &frame);
};


#endif //NFC_LAB_PARSERNFCB_H

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

#ifndef NFC_LAB_PARSERNFCA_H
#define NFC_LAB_PARSERNFCA_H

#include <parser/ParserNfc.h>

struct ParserNfcA : ParserNfcIsoDep
{
   unsigned int frameChain = 0;

   void reset() override;

   ProtocolFrame *parse(const lab::RawFrame &frame) override;

   ProtocolFrame *parseRequestREQA(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseREQA(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestWUPA(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseWUPA(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestSELn(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseSELn(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestRATS(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseRATS(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestHLTA(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseHLTA(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestPPSr(const lab::RawFrame &frame);

   ProtocolFrame *parseResponsePPSr(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestAUTH(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseAUTH(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestVASUP(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseVASUP(const lab::RawFrame &frame);
};


#endif //NFC_LAB_PARSERNFCA_H

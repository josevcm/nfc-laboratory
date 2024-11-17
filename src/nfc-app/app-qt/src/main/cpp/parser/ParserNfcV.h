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

#ifndef NFC_LAB_PARSERNFCV_H
#define NFC_LAB_PARSERNFCV_H

#include <parser/ParserNfc.h>

struct ParserNfcV : ParserNfc
{
   void reset() override;

   ProtocolFrame *parse(const lab::RawFrame &frame) override;

   ProtocolFrame *parseRequestInventory(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseInventory(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestStayQuiet(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseStayQuiet(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestReadSingle(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseReadSingle(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestWriteSingle(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseWriteSingle(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestLockBlock(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseLockBlock(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestReadMultiple(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseReadMultiple(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestWriteMultiple(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseWriteMultiple(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestSelect(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseSelect(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestResetReady(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseResetReady(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestWriteAFI(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseWriteAFI(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestLockAFI(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseLockAFI(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestWriteDSFID(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseWriteDSFID(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestLockDSFID(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseLockDSFID(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestSysInfo(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseSysInfo(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestGetSecurity(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseGetSecurity(const lab::RawFrame &frame);

   ProtocolFrame *parseRequestGeneric(const lab::RawFrame &frame);

   ProtocolFrame *parseResponseGeneric(const lab::RawFrame &frame);

   ProtocolFrame *buildRequestFlags(const lab::RawFrame &frame, int offset);

   ProtocolFrame *buildResponseFlags(const lab::RawFrame &frame, int offset);

   ProtocolFrame *buildResponseError(const lab::RawFrame &frame, int offset);

   ProtocolFrame *buildApplicationFamily(const lab::RawFrame &frame, int offset);
};


#endif //NFC_LAB_PARSERNFCV_H

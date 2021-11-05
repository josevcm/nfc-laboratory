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

#ifndef NFC_LAB_PARSERNFCV_H
#define NFC_LAB_PARSERNFCV_H

#include <parser/ParserNfc.h>

struct ParserNfcV : ParserNfc
{
   void reset() override;

   ProtocolFrame *parse(const nfc::NfcFrame &frame) override;

   ProtocolFrame *parseRequestInventory(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseInventory(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestStayQuiet(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseStayQuiet(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestReadSingle(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseReadSingle(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestWriteSingle(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseWriteSingle(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestLockBlock(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseLockBlock(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestReadMultiple(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseReadMultiple(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestWriteMultiple(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseWriteMultiple(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestSelect(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseSelect(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestResetReady(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseResetReady(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestWriteAFI(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseWriteAFI(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestLockAFI(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseLockAFI(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestWriteDSFID(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseWriteDSFID(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestLockDSFID(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseLockDSFID(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestSysInfo(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseSysInfo(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestGetSecurity(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseGetSecurity(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestGeneric(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseGeneric(const nfc::NfcFrame &frame);

   ProtocolFrame *buildRequestFlags(int flags);

   ProtocolFrame *buildResponseFlags(int flags);

   ProtocolFrame *buildResponseError(int error);

   ProtocolFrame *buildApplicationFamily(int afi);
};


#endif //NFC_LAB_PARSERNFCV_H

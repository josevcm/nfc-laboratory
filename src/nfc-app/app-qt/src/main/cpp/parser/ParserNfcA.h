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

#ifndef NFC_LAB_PARSERNFCA_H
#define NFC_LAB_PARSERNFCA_H

#include <parser/ParserNfc.h>

struct ParserNfcA : ParserNfcIsoDep
{
   unsigned int frameChain = 0;

   void reset() override;

   ProtocolFrame *parse(const nfc::NfcFrame &frame) override;

   ProtocolFrame *parseRequestREQA(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseREQA(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestWUPA(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseWUPA(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestSELn(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseSELn(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestRATS(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseRATS(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestHLTA(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseHLTA(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestPPSr(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponsePPSr(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestAUTH(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseAUTH(const nfc::NfcFrame &frame);
};


#endif //NFC_LAB_PARSERNFCA_H

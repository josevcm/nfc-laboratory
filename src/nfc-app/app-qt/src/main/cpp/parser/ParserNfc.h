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

#ifndef NFC_LAB_PARSERNFC_H
#define NFC_LAB_PARSERNFC_H

#include <nfc/NfcFrame.h>

#include <protocol/ProtocolFrame.h>

struct ParserNfc
{
   enum Caps
   {
      IS_APDU = 1
   };

   unsigned int lastCommand = 0;

   virtual void reset();

   virtual ProtocolFrame *parse(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRequestUnknown(const nfc::NfcFrame &frame);

   ProtocolFrame *parseResponseUnknown(const nfc::NfcFrame &frame);

   ProtocolFrame *parseAPDU(const QString &name, const nfc::NfcFrame &frame, int start, int length);

   ProtocolFrame *buildRootInfo(const QString &name, const nfc::NfcFrame &frame, int flags);

   ProtocolFrame *buildChildInfo(const QVariant &info);

   ProtocolFrame *buildChildInfo(const QString &name, const QVariant &info);

   ProtocolFrame *buildChildInfo(const QString &name, const QVariant &info, int start, int length);

   ProtocolFrame *buildChildInfo(const QString &name, const nfc::NfcFrame &frame, int start, int length);

   ProtocolFrame *buildChildInfo(const QString &name, const QVariant &info, int flags, int start, int length);

   bool isApdu(const QByteArray &apdu);

   QByteArray toByteArray(const nfc::NfcFrame &frame, int from = 0, int length = INT32_MAX);

   QString toString(const QByteArray &array);
};

struct ParserNfcIsoDep : ParserNfc
{
   void reset() override;

   ProtocolFrame *parse(const nfc::NfcFrame &frame) override;

   ProtocolFrame *parseIBlock(const nfc::NfcFrame &frame);

   ProtocolFrame *parseRBlock(const nfc::NfcFrame &frame);

   ProtocolFrame *parseSBlock(const nfc::NfcFrame &frame);
};

#endif //NFC_LAB_PARSERNFC_H

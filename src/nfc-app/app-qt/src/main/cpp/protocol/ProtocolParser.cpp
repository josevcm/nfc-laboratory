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

#include <QDebug>

#include <parser/ParserNfc.h>
#include <parser/ParserNfcA.h>
#include <parser/ParserNfcB.h>
#include <parser/ParserNfcF.h>
#include <parser/ParserNfcV.h>

#include "ProtocolParser.h"

struct ProtocolParser::Impl
{
   ParserNfcA nfca;

   ParserNfcB nfcb;

   ParserNfcF nfcf;

   ParserNfcV nfcv;

   unsigned int frameCount;

   nfc::NfcFrame lastFrame;

   Impl() : frameCount(1)
   {
   }

   void reset()
   {
      frameCount = 1;

      nfca.reset();
      nfcb.reset();
      nfcf.reset();
      nfcv.reset();
   }

   ProtocolFrame *parse(const nfc::NfcFrame &frame)
   {
      ProtocolFrame *info = nullptr;

      if (frame.isNfcA())
      {
         info = nfca.parse(frame);
      }
      else if (frame.isNfcB())
      {
         info = nfcb.parse(frame);
      }
      else if (frame.isNfcF())
      {
         info = nfcf.parse(frame);
      }
      else if (frame.isNfcV())
      {
         info = nfcv.parse(frame);
      }

      return info;
   }
};

ProtocolParser::ProtocolParser(QObject *parent) : QObject(parent), impl(new Impl)
{
}

ProtocolParser::~ProtocolParser() = default;

void ProtocolParser::reset()
{
   impl->reset();
}

ProtocolFrame *ProtocolParser::parse(const nfc::NfcFrame &frame)
{
   return impl->parse(frame);
}


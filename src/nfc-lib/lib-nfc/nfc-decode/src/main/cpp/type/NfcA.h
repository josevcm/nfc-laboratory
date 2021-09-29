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

#ifndef NFC_NFCA_H
#define NFC_NFCA_H

#include <list>

#include <rt/Logger.h>

#include <sdr/SignalBuffer.h>

#include <nfc/Nfc.h>
#include <nfc/NfcFrame.h>

#include "NfcStatus.h"

namespace nfc {

struct NfcA
{
   struct Impl;

   Impl *self;

   enum PatternType
   {
      Invalid = 0,
      NoPattern = 1,
      PatternX = 2,
      PatternY = 3,
      PatternZ = 4,
      PatternD = 5,
      PatternE = 6,
      PatternF = 7,
      PatternM = 8,
      PatternN = 9,
      PatternO = 10
   };

   enum CommandType
   {
      NFCA_REQA = 0x26,
      NFCA_HLTA = 0x50,
      NFCA_WUPA = 0x52,
      NFCA_AUTH1 = 0x60,
      NFCA_AUTH2 = 0x61,
      NFCA_SEL1 = 0x93,
      NFCA_SEL2 = 0x95,
      NFCA_SEL3 = 0x97,
      NFCA_RATS = 0xE0,
      NFCA_PPS = 0xD0,
      NFCA_IBLOCK = 0x02,
      NFCA_RBLOCK = 0xA2,
      NFCA_SBLOCK = 0xC2
   };

   explicit NfcA(DecoderStatus *decoder);

   ~NfcA();

   void configure(long sampleRate);

   bool detectModulation();

   void decodeFrame(sdr::SignalBuffer &samples, std::list<NfcFrame> &frames);

   bool decodePollFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames);

   bool decodeListenFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames);

   int decodePollFrameSymbolAsk(sdr::SignalBuffer &buffer);

   int decodeListenFrameSymbolAsk(sdr::SignalBuffer &buffer);

   int decodeListenFrameSymbolBpsk(sdr::SignalBuffer &buffer);

   void resetFrameSearch();

   void resetModulation();

   void process(NfcFrame frame);

   bool processREQA(NfcFrame &frame);

   bool processHLTA(NfcFrame &frame);

   bool processSELn(NfcFrame &frame);

   bool processRATS(NfcFrame &frame);

   bool processPPSr(NfcFrame &frame);

   bool processAUTH(NfcFrame &frame);

   bool processIBlock(NfcFrame &frame);

   bool processRBlock(NfcFrame &frame);

   bool processSBlock(NfcFrame &frame);

   void processOther(NfcFrame &frame);

   static bool checkCrc(NfcFrame &frame);

   static bool checkParity(unsigned int value, unsigned int parity);
};

}

#endif //NFC_NFCA_H

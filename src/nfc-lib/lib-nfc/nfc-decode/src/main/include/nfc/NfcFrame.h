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

#ifndef NFC_NFCFRAME_H
#define NFC_NFCFRAME_H

#include <rt/ByteBuffer.h>

namespace nfc {

class NfcFrame : public rt::ByteBuffer
{
      struct Impl;

   public:

      static const NfcFrame Nil;

   public:

      NfcFrame();

      NfcFrame(int techType, int frameType);

      NfcFrame(int techType, int frameType, double timeStart, double timeEnd);

      NfcFrame(const NfcFrame &other);

      NfcFrame &operator=(const NfcFrame &other);

      operator bool() const;

      bool isNfcA() const;

      bool isNfcB() const;

      bool isNfcF() const;

      bool isNfcV() const;

      bool isNoCarrier() const;

      bool isEmptyFrame() const;

      bool isPollFrame() const;

      bool isListenFrame() const;

      bool isShortFrame() const;

      bool isEncrypted() const;

      bool isTruncated() const;

      bool hasParityError() const;

      bool hasCrcError() const;

      unsigned int techType() const;

      void setTechType(unsigned int tech);

      unsigned int frameType() const;

      void setFrameType(unsigned int type);

      unsigned int framePhase() const;

      void setFramePhase(unsigned int phase);

      unsigned int frameFlags() const;

      void setFrameFlags(unsigned int flags);

      void clearFrameFlags(unsigned int flags);

      bool hasFrameFlags(unsigned int flags);

      unsigned int frameRate() const;

      void setFrameRate(unsigned int rate);

      double timeStart() const;

      void setTimeStart(double start);

      double timeEnd() const;

      void setTimeEnd(double end);

      unsigned long sampleStart() const;

      void setSampleStart(unsigned long start);

      unsigned long sampleEnd() const;

      void setSampleEnd(unsigned long end);

   private:

      std::shared_ptr<Impl> impl;
};

}

#endif

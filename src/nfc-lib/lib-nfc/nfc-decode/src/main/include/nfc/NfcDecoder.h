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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#ifndef NFC_NFCDECODER_H
#define NFC_NFCDECODER_H

#include <list>

#include <rt/FloatBuffer.h>

#include <sdr/SignalBuffer.h>

#include <nfc/NfcFrame.h>

namespace nfc {

class NfcDecoder
{
      struct Impl;

   public:

      NfcDecoder();

      void initialize();

      void cleanup();

      std::list<NfcFrame> nextFrames(sdr::SignalBuffer samples);

      bool isNfcAEnabled() const;

      void setEnableNfcA(bool enabled);

      bool isNfcBEnabled() const;

      void setEnableNfcB(bool enabled);

      bool isNfcFEnabled() const;

      void setEnableNfcF(bool enabled);

      bool isNfcVEnabled() const;

      void setEnableNfcV(bool enabled);

      long sampleRate() const;

      void setSampleRate(long sampleRate);

      long streamTime() const;

      void setStreamTime(long referenceTime);

      float powerLevelThreshold() const;

      void setPowerLevelThreshold(float value);

      void setModulationThresholdNfcA(float min, float max);

      void setModulationThresholdNfcB(float min, float max);

      void setModulationThresholdNfcF(float min, float max);

      void setModulationThresholdNfcV(float min, float max);

   private:

      std::shared_ptr<Impl> impl;
};

}

#endif //NFC_LAB_NFCDECODER_H

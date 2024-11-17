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

#ifndef NFC_NFCDECODER_H
#define NFC_NFCDECODER_H

#include <list>

#include <hw/SignalBuffer.h>

#include <lab/data/RawFrame.h>

namespace lab {

class NfcDecoder
{
      struct Impl;

   public:

      NfcDecoder();

      void initialize();

      void cleanup();

      std::list<RawFrame> nextFrames(hw::SignalBuffer samples);

      bool isDebugEnabled() const;

      void setEnableDebug(bool enabled);

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

      float modulationThresholdNfcAMin() const;

      float modulationThresholdNfcAMax() const;

      void setModulationThresholdNfcA(float min, float max);

      float modulationThresholdNfcBMin() const;

      float modulationThresholdNfcBMax() const;

      void setModulationThresholdNfcB(float min, float max);

      float modulationThresholdNfcFMin() const;

      float modulationThresholdNfcFMax() const;

      void setModulationThresholdNfcF(float min, float max);

      float modulationThresholdNfcVMin() const;

      float modulationThresholdNfcVMax() const;

      void setModulationThresholdNfcV(float min, float max);

      float correlationThresholdNfcA() const;

      void setCorrelationThresholdNfcA(float value);

      float correlationThresholdNfcB() const;

      void setCorrelationThresholdNfcB(float value);

      float correlationThresholdNfcF() const;

      void setCorrelationThresholdNfcF(float value);

      float correlationThresholdNfcV() const;

      void setCorrelationThresholdNfcV(float value);

   private:

      std::shared_ptr<Impl> impl;
};

}

#endif

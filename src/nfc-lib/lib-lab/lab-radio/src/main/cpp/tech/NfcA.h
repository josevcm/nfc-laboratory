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

#ifndef LAB_NFCA_H
#define LAB_NFCA_H

#include <list>

#include <hw/SignalBuffer.h>

#include <lab/data/RawFrame.h>

#include <NfcTech.h>

namespace lab {

struct NfcA
{
   struct Impl;

   Impl *self;

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
      NFCA_SBLOCK = 0xC0
   };

   explicit NfcA(NfcDecoderStatus *decoder);

   ~NfcA();

   float modulationThresholdMin() const;

   float modulationThresholdMax() const;

   void setModulationThreshold(float min, float max);

   float correlationThreshold() const;

   void setCorrelationThreshold(float value);

   void initialize(unsigned int sampleRate);

   bool detect();

   void decode(hw::SignalBuffer &samples, std::list<RawFrame> &frames);
};

}

#endif

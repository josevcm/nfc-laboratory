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

#include <rt/Logger.h>

#include "IsoTech.h"

namespace lab {

// rt::Logger *log = rt::Logger::getLogger("decoder:IsoDecoderStatus");

// process next sample from signal buffer
bool IsoDecoderStatus::nextSample(hw::SignalBuffer &buffer)
{
   if (buffer.remaining() == 0 || buffer.type() != hw::SignalType::SIGNAL_TYPE_LOGIC_SAMPLES)
      return false;

   // number of channels
   const unsigned int ch = buffer.stride();

   // get next samples from buffer
   buffer.get(sampleData, ch);

   // initialize last samples
   if (signalClock < 0)
   {
#pragma omp simd
      for (int i = 0; i < ch; i++)
      {
         sampleLast[i] = sampleData[i];
      }
   }

   // calculate data edges from previous samples
#pragma omp simd
   for (int i = 0; i < ch; i++)
   {
      sampleEdge[i] = sampleData[i] - sampleLast[i];
      sampleLast[i] = sampleData[i];
   }

   // update signal clock and pulse filter
   ++signalClock;

   // store debug data if enabled
   if (debug)
   {
      debug->block(signalClock);

      for (int i = 0; i < ch; i++)
      {
         debug->set(DEBUG_SIGNAL_DATA_CHANNEL + i, sampleData[i]);
         debug->set(DEBUG_SIGNAL_EDGE_CHANNEL + i, sampleEdge[i]);
      }
   }

   return true;
}

}

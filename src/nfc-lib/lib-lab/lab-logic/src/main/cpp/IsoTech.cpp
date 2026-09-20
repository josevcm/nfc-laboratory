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

// rt::Logger *log = rt::Logger::getLogger("decoder.IsoDecoderStatus");

// process next sample from signal buffer
bool IsoDecoderStatus::nextSample(hw::SignalBuffer &buffer)
{
   if (buffer.remaining() == 0 || buffer.type() != hw::SignalType::SIGNAL_TYPE_LOGIC_SAMPLES)
      return false;

   // number of channels carried by the buffer
   const unsigned int stride = buffer.stride();

   // a sample holds one value per probe, drain anything that cannot form a complete one instead of reading past it
   if (stride == 0 || buffer.remaining() < stride)
   {
      while (buffer.remaining() > 0)
         buffer.get();

      return false;
   }

   // number of channels tracked, probes beyond the ones we follow are read but discarded
   const unsigned int ch = std::min(stride, ISO_CHANNEL_COUNT);

   // get next samples from buffer
   buffer.get(sampleData, ch);

   // drop remaining probes of this sample, the buffer must always advance a full stride to stay aligned
   for (unsigned int i = ch; i < stride; i++)
      buffer.get();

   // initialize last samples, master clock still holds its reset value until incremented below
   if (signalClock == static_cast<unsigned int>(-1))
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

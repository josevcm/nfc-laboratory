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
   // integrate new buffer interleaving with previous cache
   if (buffer)
   {
      // add new buffer to cache (interleave with previous content)
      if (signalCache && signalCache.offset() == buffer.offset())
      {
         hw::SignalBuffer interleaved(signalCache.size() + buffer.size(), signalCache.stride() + buffer.stride(), 1, signalCache.sampleRate(), signalCache.offset(), signalCache.decimation(), signalCache.type());

         for (int i = 0; i < signalCache.elements(); i++)
         {
            // copy current buffer keep space to interleave new buffer
            signalCache.get(interleaved.pull(signalCache.stride()), signalCache.stride());

            // add incoming buffer
            buffer.get(interleaved.pull(buffer.stride()), buffer.stride());
         }

         // flip buffer pointers
         interleaved.flip();

         // use interleaved buffer as new cache
         signalCache = interleaved;
      }

      // initialize cache on first buffer
      else if (!signalCache)
      {
         // reject non-raw logic signal buffers
         if (buffer.type() != hw::SignalType::SIGNAL_TYPE_RAW_LOGIC)
            return false;

         // initialize cache with new buffer
         signalCache = buffer;
      }
   }

   if (signalCache.offset() != buffer.offset())
   {
      // if there is no more samples in cache, start with new buffer
      if (signalCache.available() < signalCache.stride())
      {
         signalCache = buffer;
         return false;
      }

      // get next samples from buffer
      signalCache.get(sampleData, signalCache.stride());

      // initialize last samples
      if (signalClock == 0)
      {
         for (int i = 0; i < signalCache.stride(); i++)
         {
            sampleLast[i] = sampleData[i];
         }
      }

      // calculate data edges from previous samples
      for (int i = 0; i < signalCache.stride(); i++)
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

         for (int i = 0; i < signalCache.stride(); i++)
         {
            debug->set(DEBUG_SIGNAL_DATA_CHANNEL + i, sampleData[i]);
            debug->set(DEBUG_SIGNAL_EDGE_CHANNEL + i, sampleEdge[i]);
         }
      }

      return true;
   }

   return false;
}

bool IsoDecoderStatus::hasSamples(const hw::SignalBuffer &buffer) const
{
   return signalCache.offset() != buffer.offset();
}

}

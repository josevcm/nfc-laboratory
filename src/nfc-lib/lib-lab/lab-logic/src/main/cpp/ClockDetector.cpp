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

#include <cmath>

#include <lab/logic/ClockDetector.h>

namespace lab {

// logic level above which a probe reads high, matching the rest of the logic pipeline
#define LOGIC_THRESHOLD 0.5f

// divisor applied to the sample rate to size the default measurement window, one millisecond
#define DEFAULT_WINDOW_DIVISOR 1000

// default relative frequency change required to emit a new state
#define DEFAULT_TOLERANCE 0.02f

struct ClockDetector::Impl
{
   unsigned int sampleRate = 0;

   // measurement window requested by the caller, zero while the default is in use
   unsigned int windowOverride = 0;

   // measurement window actually applied
   unsigned int window = 0;

   float tolerance = DEFAULT_TOLERANCE;

   // previous sample level, needed to spot edges that fall across two calls
   bool lastLevel = false;

   // cleared until the first sample of the stream has been read, since no edge can be derived from it
   bool primed = false;

   // rising edges counted so far within the window, and how many samples of it have been consumed
   unsigned int windowEdges = 0;
   unsigned int windowCount = 0;

   // stream sample index the current window starts at
   unsigned long long windowStart = 0;

   // last frequency reported, kept as the reference the tolerance is measured against so that drift smaller than the
   // tolerance never accumulates into a new state
   float lastFrequency = 0;

   // cleared until a first state has been emitted, which always happens on the first complete window
   bool established = false;

   void applyWindow()
   {
      window = windowOverride ? windowOverride : sampleRate / DEFAULT_WINDOW_DIVISOR;

      reset();
   }

   void reset()
   {
      lastLevel = false;
      primed = false;
      windowEdges = 0;
      windowCount = 0;
      windowStart = 0;
      lastFrequency = 0;
      established = false;
   }

   /*
    * Decide whether the frequency just measured differs enough from the one in force to be worth reporting
    */
   bool changed(float frequency) const
   {
      if (!established)
         return true;

      // starting and stopping are always worth reporting, however small the tolerance
      if ((frequency > 0) != (lastFrequency > 0))
         return true;

      if (lastFrequency == 0)
         return false;

      return std::fabs(frequency - lastFrequency) > tolerance * lastFrequency;
   }

   std::vector<ClockState> process(const float *samples, unsigned int count, unsigned int stride, unsigned long long offset)
   {
      std::vector<ClockState> states;

      if (!window || !stride || !samples)
         return states;

      for (unsigned int i = 0; i < count; i++)
      {
         const bool level = samples[static_cast<size_t>(i) * stride] > LOGIC_THRESHOLD;

         // the first sample of the stream only establishes a reference, and opens the first window
         if (!primed)
         {
            primed = true;
            windowStart = offset + i;
         }
         else if (level && !lastLevel)
         {
            windowEdges++;
         }

         lastLevel = level;

         if (++windowCount < window)
            continue;

         // one rising edge per period, so the edge count over a window of known duration is the frequency
         const auto frequency = static_cast<float>(static_cast<double>(windowEdges) * sampleRate / window);

         if (changed(frequency))
         {
            states.push_back({windowStart, frequency});

            lastFrequency = frequency;
            established = true;
         }

         windowEdges = 0;
         windowCount = 0;
         windowStart = offset + i + 1;
      }

      return states;
   }
};

ClockDetector::ClockDetector(unsigned int sampleRate) : impl(std::make_shared<Impl>())
{
   setSampleRate(sampleRate);
}

void ClockDetector::setSampleRate(unsigned int sampleRate)
{
   impl->sampleRate = sampleRate;
   impl->applyWindow();
}

unsigned int ClockDetector::sampleRate() const
{
   return impl->sampleRate;
}

void ClockDetector::setWindow(unsigned int samples)
{
   impl->windowOverride = samples;
   impl->applyWindow();
}

unsigned int ClockDetector::window() const
{
   return impl->window;
}

void ClockDetector::setTolerance(float tolerance)
{
   impl->tolerance = tolerance;
}

float ClockDetector::tolerance() const
{
   return impl->tolerance;
}

void ClockDetector::reset()
{
   impl->reset();
}

std::vector<ClockState> ClockDetector::process(const float *samples, unsigned int count, unsigned int stride, unsigned long long offset)
{
   return impl->process(samples, count, stride, offset);
}

std::vector<ClockState> clockStatesInRange(const std::vector<ClockState> &states, unsigned long long start, unsigned long long end)
{
   std::vector<ClockState> result;

   if (end <= start)
      return result;

   // state in force when the range opens, kept aside until it is known whether anything precedes it inside the range
   bool carried = false;
   float carriedFrequency = 0;

   for (const ClockState &state: states)
   {
      if (state.offset < start)
      {
         carried = true;
         carriedFrequency = state.frequency;
         continue;
      }

      if (state.offset > end)
         break;

      if (carried)
      {
         result.push_back({start, carriedFrequency});
         carried = false;
      }

      result.push_back(state);
   }

   // a range falling entirely inside one state carries no change of its own, only the state covering it
   if (carried)
      result.push_back({start, carriedFrequency});

   return result;
}

}

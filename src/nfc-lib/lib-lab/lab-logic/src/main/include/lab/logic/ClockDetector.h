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

#ifndef LAB_CLOCKDETECTOR_H
#define LAB_CLOCKDETECTOR_H

#include <memory>
#include <vector>

namespace lab {

/*
 * Frequency measured over one stretch of a clock probe.
 *
 * A clock line toggles far too fast to be drawn edge by edge, so instead of the waveform we keep the frequency it runs
 * at, and emit a new state only where that frequency actually changes. A state holds until the next one starts.
 */
struct ClockState
{
   // stream sample index where this state begins
   unsigned long long offset;

   // measured frequency in Hz, zero while the clock is stopped
   float frequency;
};

/*
 * Measures the frequency of a clock probe by counting rising edges over fixed windows of samples.
 *
 * Detection is incremental: samples arrive in whatever blocks the device produces, and the result must not depend on
 * where those blocks happen to be cut, so both the running edge count and the last sample carry across calls.
 */
class ClockDetector
{
      struct Impl;

   public:

      explicit ClockDetector(unsigned int sampleRate = 0);

      /*
       * Sample rate of the probe, in Hz. Changing it resizes the default measurement window and resets detection.
       */
      void setSampleRate(unsigned int sampleRate);

      unsigned int sampleRate() const;

      /*
       * Samples measured per window. Passing zero restores the default of one millisecond worth of samples, which sets
       * the frequency resolution to sampleRate / window. Changing it resets detection.
       */
      void setWindow(unsigned int samples);

      unsigned int window() const;

      /*
       * Relative frequency change required to emit a new state, e.g. 0.02 for 2%. Starting and stopping always emit,
       * whatever the tolerance.
       */
      void setTolerance(float tolerance);

      float tolerance() const;

      /*
       * Discard all accumulated state, so the next call starts a fresh stream.
       */
      void reset();

      /*
       * Measure `count` samples of one probe, read every `stride` floats starting at `samples`, where `offset` is the
       * stream sample index of the first of them. Returns the states detected within this call, in stream order, which
       * is usually none: a steady clock emits a single state for the whole capture.
       */
      std::vector<ClockState> process(const float *samples, unsigned int count, unsigned int stride, unsigned long long offset);

   private:

      std::shared_ptr<Impl> impl;
};

/*
 * Select the states that apply to the sample range [start, end].
 *
 * A state holds until the next one starts, so the last state before the range is the one in force when it opens.
 * Plain filtering would drop it and leave the range headless, so it is carried forward and re-anchored at `start`.
 * Returns an empty list for an empty range.
 */
std::vector<ClockState> clockStatesInRange(const std::vector<ClockState> &states, unsigned long long start, unsigned long long end);

}

#endif

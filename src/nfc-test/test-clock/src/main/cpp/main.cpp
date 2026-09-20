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

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <rt/Logger.h>

#include <lab/logic/ClockDetector.h>

using namespace rt;
using namespace lab;

Logger *logger = Logger::getLogger("main");

// sample rate used by every synthetic capture, matching the Sipeed logic analyzer default
constexpr unsigned int SAMPLE_RATE = 20000000;

// nominal ISO-7816 clock frequency
constexpr double CLOCK_FREQ = 3571200.0;

// number of channels the synthetic buffers interleave, so the detector is exercised with a realistic stride
constexpr unsigned int STRIDE = 4;

// index of the clock probe inside the stride
constexpr unsigned int CLOCK_CHANNEL = 1;

// relative deviation accepted between the measured and the nominal frequency
constexpr double FREQ_EPSILON = 0.001;

/*
 * Synthetic interleaved capture, with the clock written into CLOCK_CHANNEL and the remaining probes left idle
 */
struct Capture
{
   std::vector<float> samples;

   unsigned int count() const
   {
      return static_cast<unsigned int>(samples.size() / STRIDE);
   }

   /*
    * Append `duration` seconds of a square wave of `frequency` Hz, or of a flat idle line when frequency is zero
    */
   void append(double frequency, double duration)
   {
      const auto total = static_cast<unsigned int>(duration * SAMPLE_RATE);
      const unsigned int base = count();

      samples.resize(samples.size() + static_cast<size_t>(total) * STRIDE, 0.0f);

      if (frequency <= 0)
         return;

      for (unsigned int i = 0; i < total; i++)
      {
         // phase of the square wave at this sample, continuous within the appended stretch
         const double phase = std::fmod(static_cast<double>(i) * frequency / SAMPLE_RATE, 1.0);

         samples[static_cast<size_t>(base + i) * STRIDE + CLOCK_CHANNEL] = phase < 0.5 ? 1.0f : 0.0f;
      }
   }
};

/*
 * Feed a capture to the detector in blocks of `block` samples, collecting every state it reports
 */
std::vector<ClockState> detect(const Capture &capture, unsigned int block)
{
   ClockDetector detector(SAMPLE_RATE);

   std::vector<ClockState> states;

   for (unsigned int pos = 0; pos < capture.count(); pos += block)
   {
      const unsigned int size = std::min(block, capture.count() - pos);

      for (const ClockState &state: detector.process(capture.samples.data() + static_cast<size_t>(pos) * STRIDE + CLOCK_CHANNEL, size, STRIDE, pos))
         states.push_back(state);
   }

   return states;
}

void report(const std::vector<ClockState> &states)
{
   for (const ClockState &state: states)
   {
      std::cout << "      offset " << std::setw(10) << state.offset << "  frequency " << std::fixed << std::setprecision(1) << state.frequency << " Hz" << std::endl;
   }
}

bool check(const std::string &name, bool condition)
{
   std::cout << "  " << (condition ? "PASS" : "FAIL") << "  " << name << std::endl;

   return condition;
}

bool closeTo(double measured, double expected)
{
   if (expected == 0)
      return measured == 0;

   return std::abs(measured - expected) / expected < FREQ_EPSILON;
}

/*
 * A clock running steadily for the whole capture must collapse into a single state
 */
int testSteadyClock()
{
   std::cout << "test: steady clock" << std::endl;

   Capture capture;
   capture.append(CLOCK_FREQ, 0.010);

   const std::vector<ClockState> states = detect(capture, 4096);

   report(states);

   int failed = 0;

   failed += !check("emits exactly one state", states.size() == 1);

   if (states.size() == 1)
   {
      failed += !check("state starts at sample zero", states[0].offset == 0);
      failed += !check("measures the nominal frequency", closeTo(states[0].frequency, CLOCK_FREQ));
   }

   return failed;
}

/*
 * A probe that never toggles must report a stopped clock rather than nothing at all
 */
int testStoppedClock()
{
   std::cout << "test: stopped clock" << std::endl;

   Capture capture;
   capture.append(0, 0.010);

   const std::vector<ClockState> states = detect(capture, 4096);

   report(states);

   int failed = 0;

   failed += !check("emits exactly one state", states.size() == 1);

   if (states.size() == 1)
      failed += !check("reports zero frequency", states[0].frequency == 0);

   return failed;
}

/*
 * Starting and stopping must be reported whatever the tolerance, and at the right sample
 */
int testClockGap()
{
   std::cout << "test: clock stops and restarts" << std::endl;

   Capture capture;
   capture.append(CLOCK_FREQ, 0.005);
   capture.append(0, 0.005);
   capture.append(CLOCK_FREQ, 0.005);

   const std::vector<ClockState> states = detect(capture, 4096);

   report(states);

   int failed = 0;

   failed += !check("emits three states", states.size() == 3);

   if (states.size() == 3)
   {
      failed += !check("runs from the start", closeTo(states[0].frequency, CLOCK_FREQ));
      failed += !check("stops at 5 ms", states[1].frequency == 0 && states[1].offset == SAMPLE_RATE / 200);
      failed += !check("restarts at 10 ms", closeTo(states[2].frequency, CLOCK_FREQ) && states[2].offset == SAMPLE_RATE / 100);
   }

   return failed;
}

/*
 * A change of frequency larger than the tolerance must split the capture in two
 */
int testFrequencyChange()
{
   std::cout << "test: frequency change" << std::endl;

   Capture capture;
   capture.append(4000000.0, 0.005);
   capture.append(1000000.0, 0.005);

   const std::vector<ClockState> states = detect(capture, 4096);

   report(states);

   int failed = 0;

   failed += !check("emits two states", states.size() == 2);

   if (states.size() == 2)
   {
      failed += !check("measures 4 MHz first", closeTo(states[0].frequency, 4000000.0));
      failed += !check("measures 1 MHz after 5 ms", closeTo(states[1].frequency, 1000000.0) && states[1].offset == SAMPLE_RATE / 200);
   }

   return failed;
}

/*
 * The result must depend on the signal alone, never on how the device happened to cut it into blocks. Odd block sizes
 * land window boundaries and signal edges in the middle of a call, which is where carry-over bugs surface.
 */
int testBlockIndependence()
{
   std::cout << "test: independence from block size" << std::endl;

   Capture capture;
   capture.append(CLOCK_FREQ, 0.004);
   capture.append(0, 0.003);
   capture.append(2000000.0, 0.004);

   const std::vector<ClockState> reference = detect(capture, capture.count());

   report(reference);

   int failed = 0;

   for (unsigned int block: {1u, 7u, 999u, 4096u, 65537u})
   {
      const std::vector<ClockState> states = detect(capture, block);

      bool equal = states.size() == reference.size();

      for (size_t i = 0; equal && i < states.size(); i++)
         equal = states[i].offset == reference[i].offset && states[i].frequency == reference[i].frequency;

      failed += !check("matches the single block result with block size " + std::to_string(block), equal);
   }

   return failed;
}

/*
 * Counting whole edges per window quantizes the measurement, and that quantization must not leak out as a stream of
 * spurious states while the clock is in fact steady
 */
int testNoSpuriousStates()
{
   std::cout << "test: steady clock does not drift into extra states" << std::endl;

   Capture capture;

   // a frequency that does not divide the window evenly, so the edge count alternates between two values
   capture.append(3333333.0, 0.050);

   const std::vector<ClockState> states = detect(capture, 4096);

   report(states);

   return !check("emits exactly one state over 50 ms", states.size() == 1);
}

/*
 * Saving a selection filters the states down to the saved range. A state holds until the next one starts, so the one
 * in force when the range opens has to survive that filtering, or the band reloads blank until the next change.
 */
int testRangeSelection()
{
   std::cout << "test: selecting a sample range" << std::endl;

   const std::vector<ClockState> states = {
      {0, 0},
      {1000, 3571200.0f},
      {5000, 0},
      {9000, 2000000.0f}
   };

   int failed = 0;

   {
      const std::vector<ClockState> saved = clockStatesInRange(states, 0, 10000);

      failed += !check("a full range keeps every state", saved.size() == 4);
   }

   {
      // opens midway through the 3571200 Hz state and closes midway through the stopped one
      const std::vector<ClockState> saved = clockStatesInRange(states, 2000, 7000);

      report(saved);

      failed += !check("a partial range keeps the state in force plus the changes inside", saved.size() == 2);

      if (saved.size() == 2)
      {
         failed += !check("the carried state is re-anchored at the range start", saved[0].offset == 2000 && saved[0].frequency == 3571200.0f);
         failed += !check("the change inside the range keeps its offset", saved[1].offset == 5000 && saved[1].frequency == 0);
      }
   }

   {
      // falls entirely inside the 3571200 Hz state, so nothing changes within it
      const std::vector<ClockState> saved = clockStatesInRange(states, 2000, 4000);

      report(saved);

      failed += !check("a range inside a single state keeps that state alone", saved.size() == 1);

      if (saved.size() == 1)
         failed += !check("and re-anchors it at the range start", saved[0].offset == 2000 && saved[0].frequency == 3571200.0f);
   }

   {
      const std::vector<ClockState> saved = clockStatesInRange(states, 5000, 5000);

      failed += !check("an empty range keeps nothing", saved.empty());
   }

   {
      const std::vector<ClockState> saved = clockStatesInRange({}, 0, 10000);

      failed += !check("an empty capture keeps nothing", saved.empty());
   }

   return failed;
}

int main(int argc, char *argv[])
{
   logger->info("***********************************************************************");
   logger->info("NFC laboratory, 2024 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   logger->info("***********************************************************************");

   int failed = 0;

   failed += testSteadyClock();
   failed += testStoppedClock();
   failed += testClockGap();
   failed += testFrequencyChange();
   failed += testBlockIndependence();
   failed += testNoSpuriousStates();
   failed += testRangeSelection();

   std::cout << std::endl;

   if (failed)
      std::cout << failed << " check(s) FAILED" << std::endl;
   else
      std::cout << "all checks passed" << std::endl;

   return failed ? 1 : 0;
}

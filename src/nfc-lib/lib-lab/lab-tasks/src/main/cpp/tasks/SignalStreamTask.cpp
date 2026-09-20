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

#include <mutex>

#include <rt/BlockingQueue.h>
#include <rt/Throughput.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <lab/logic/ClockDetector.h>
#include <lab/tasks/SignalStreamTask.h>

#include "AbstractTask.h"

#define WINDOW 51
#define THRESHOLD 0.005

// forced keep-alive interval (in raw samples) used during flat/idle signal, so the live plot and the
// storage stream still get a periodic refresh point. APCM v3 stores offset deltas as varints, so this
// is no longer bound to 1 byte (255) as in the old fixed-width storage format: raising it lets long
// idle stretches collapse to a handful of points instead of one every 255 samples.
#define LOGIC_INTERVAL 1000000
#define RADIO_INTERVAL 1000000

// probe carrying the smart card clock, too fast to be drawn edge by edge, so it is summarized as frequency instead
#define CLOCK_CHANNEL 1

namespace lab {

struct SignalStreamTask::Impl : SignalStreamTask, AbstractTask
{
   // signal buffer frame stream subject
   rt::Subject<hw::SignalBuffer> *logicSignalStream = nullptr;

   // signal buffer frame stream subject
   rt::Subject<hw::SignalBuffer> *radioSignalStream = nullptr;

   // signal buffer frame stream subject
   rt::Subject<hw::SignalBuffer> *adaptiveSignalStream = nullptr;

   // signal stream subscription
   rt::Subject<hw::SignalBuffer>::Subscription logicSignalSubscription;

   // signal stream subscription
   rt::Subject<hw::SignalBuffer>::Subscription radioSignalSubscription;

   // signal stream queue buffer
   rt::BlockingQueue<hw::SignalBuffer> signalQueue;

   // throughput meter
   rt::Throughput taskThroughput;

   // stream lock
   std::mutex signalMutex;

   // frequency tracker for the clock probe
   ClockDetector clockDetector;

   // last status sent
   std::chrono::time_point<std::chrono::steady_clock> lastStatus;

   explicit Impl() : AbstractTask("worker.SignalResampling", "adaptive")
   {
      // access to radio signal subject stream
      logicSignalStream = rt::Subject<hw::SignalBuffer>::name("logic.signal.raw");

      // access to logic signal subject stream
      radioSignalStream = rt::Subject<hw::SignalBuffer>::name("radio.signal.raw");

      // access to signal subject stream
      adaptiveSignalStream = rt::Subject<hw::SignalBuffer>::name("adaptive.signal");

      // subscribe to logic signal events
      logicSignalSubscription = logicSignalStream->subscribe([=](const hw::SignalBuffer &buffer) {
         signalQueue.add(buffer);
      });

      // subscribe to radio signal events
      radioSignalSubscription = radioSignalStream->subscribe([=](const hw::SignalBuffer &buffer) {
         signalQueue.add(buffer);
      });
   }

   ~Impl() override = default;

   void start() override
   {
      taskThroughput.begin();
   }

   void stop() override
   {
      taskThroughput.end();
   }

   bool loop() override
   {
      /*
       * first process pending commands
       */
      if (auto command = commandQueue.get())
      {
         log->debug("adaptive command [{}]", {command->code});
      }

      /*
       * process signal queue
       */
      if (const auto buffer = signalQueue.get(10))
      {
         if (buffer.has_value())
         {
            process(buffer.value());
         }
      }

      // trace task throughput
      if (std::chrono::steady_clock::now() - lastStatus > std::chrono::milliseconds(1000))
      {
         if (taskThroughput.average() > 0)
            log->info("average throughput {.2} Msps", {taskThroughput.average() / 1E6});

         // store last search time
         lastStatus = std::chrono::steady_clock::now();
      }

      return true;
   }

   void process(const hw::SignalBuffer &buffer)
   {
      // propagate EOF
      if (!buffer.isValid())
      {
         // the next capture starts a new stream, nothing measured so far applies to it
         clockDetector.reset();

         adaptiveSignalStream->next({});
         return;
      }

      switch (buffer.type())
      {
         // adaptive resample for raw real signal
         case hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES:
         {
            processRadioSignal(buffer);
            break;
         }

         // adaptive resample for stream logic signal
         case hw::SignalType::SIGNAL_TYPE_LOGIC_SAMPLES:
         {
            processClockSignal(buffer);
            processLogicSignal(buffer);
            break;
         }

         default:
            break;
      }
   }

   void processRadioSignal(const hw::SignalBuffer &buffer)
   {
      hw::SignalBuffer resampled((buffer.elements() / RADIO_INTERVAL) * 2 + buffer.elements() * 2, 2, 1, buffer.sampleRate(), buffer.offset(), 0, hw::SignalType::SIGNAL_TYPE_RADIO_SIGNAL, buffer.id());

      float avrg = 0;
      float last = buffer[0];
      float filter = THRESHOLD;

      // initialize average
      for (int i = 0; i < (WINDOW / 2); i++)
         avrg += buffer[i];

      // always store first sample
      resampled.put(buffer[0]).put(0.0);

      // index of current point and last control point
      int i = 0, c = 0, p = -1;

      // adaptive resample based on maximum average deviation
      for (int r = i - (WINDOW / 2) - 1, a = i + (WINDOW / 2); i < buffer.limit(); ++i, ++p, ++a, ++r)
      {
         float value = buffer[i];

         // add new sample
         if (a < buffer.limit())
            avrg += buffer[a];

         // remove old sample
         if (r >= 0)
            avrg -= buffer[r];

         // detect deviation from average
         float stdev = std::abs(value - (avrg / static_cast<float>(WINDOW)));

         // store new sample if different from last or every RADIO_INTERVAL samples
         if (stdev > filter || (i - c) >= RADIO_INTERVAL)
         {
            // append control point
            if (stdev > filter && c < p)
               resampled.put(last).put(static_cast<float>(p));

            // append new value
            resampled.put(value).put(static_cast<float>(i));

            // update control point index
            c = i;
         }

         // store last value
         last = value;
      }

      // store last sample
      if (c < p)
         resampled.put(last).put(static_cast<float>(p));

      resampled.flip();

      adaptiveSignalStream->next(resampled);

      taskThroughput.update(buffer.elements());
   }

   /*
    * Summarize the clock probe as the frequency it runs at.
    *
    * A smart card clock toggles millions of times per second, so one plot point per edge exhausts memory within
    * seconds of capture and makes every replot walk the whole capture. What an analyst reads off that trace is
    * whether the clock runs and how fast, and both survive being reduced to one point per frequency change.
    */
   void processClockSignal(const hw::SignalBuffer &buffer)
   {
      // captures that do not carry the clock probe leave it out of the stride entirely
      if (buffer.stride() <= CLOCK_CHANNEL)
         return;

      if (clockDetector.sampleRate() != buffer.sampleRate())
         clockDetector.setSampleRate(buffer.sampleRate());

      const std::vector<ClockState> states = clockDetector.process(buffer.data() + CLOCK_CHANNEL, buffer.elements(), buffer.stride(), buffer.offset());

      if (states.empty())
         return;

      // a measurement window can straddle two buffers, so the first state may predate this one: anchor the buffer on
      // that state instead of on the buffer offset, keeping every stored offset relative and positive
      const unsigned long long base = states.front().offset;

      hw::SignalBuffer clock(static_cast<unsigned int>(states.size()) * 2, 2, 1, buffer.sampleRate(), base, 0, hw::SignalType::SIGNAL_TYPE_CLK_SIGNAL, CLOCK_CHANNEL);

      for (const ClockState &state: states)
      {
         clock.put(state.frequency).put(static_cast<float>(state.offset - base));
      }

      clock.flip();

      adaptiveSignalStream->next(clock);
   }

   void processLogicSignal(const hw::SignalBuffer &buffer)
   {
      unsigned int ch = buffer.stride();

#pragma omp parallel for default(none) shared(buffer, ch, adaptiveSignalStream, taskThroughput) schedule(static)
      for (unsigned int n = 0; n < ch; ++n)
      {
         // the clock is summarized by processClockSignal instead, drawing it edge by edge would flood the plot
         if (n == CLOCK_CHANNEL)
            continue;

         hw::SignalBuffer resampled((buffer.elements() / LOGIC_INTERVAL) * 2 + buffer.elements() * 2, 2, 1, buffer.sampleRate(), buffer.offset(), 0, hw::SignalType::SIGNAL_TYPE_LOGIC_SIGNAL, n);

         // get value of the first sample of a channel
         float last = buffer[n];

         // and store in resampled buffer
         resampled.put(last).put(0.0);

         // adaptive resample based values changes (logic)
         for (unsigned int s = 1, i = ch + n, c = 0; i < buffer.limit(); ++s, i += ch)
         {
            float value = buffer[i];

            // store new sample if different from last or every LOGIC_INTERVAL samples
            if (value != last || (s - c) >= LOGIC_INTERVAL)
            {
               resampled.put(value).put(static_cast<float>(s));

               // update last value
               last = value;

               // update control point index
               c = s;
            }
         }

         resampled.flip();

         adaptiveSignalStream->next(resampled);
      }

      taskThroughput.update(buffer.elements());
   }
};

SignalStreamTask::SignalStreamTask() : Worker("SignalResampling")
{
}

rt::Worker *SignalStreamTask::construct()
{
   return new Impl;
}

}

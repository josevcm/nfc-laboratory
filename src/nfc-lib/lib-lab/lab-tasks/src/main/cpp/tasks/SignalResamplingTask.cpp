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

#include <lab/tasks/SignalResamplingTask.h>

#include "AbstractTask.h"

#define WINDOW 51
#define THRESHOLD 0.005
#define LOGIC_INTERVAL 255 // max 2^8-1 (1 byte)
#define RADIO_INTERVAL 255 // max 2^8-1 (1 byte)

namespace lab {

struct SignalResamplingTask::Impl : SignalResamplingTask, AbstractTask
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
      if (const auto buffer = signalQueue.get(25))
      {
         process(buffer.value());
      }

      // trace task throughput
      if ((std::chrono::steady_clock::now() - lastStatus) > std::chrono::milliseconds(1000))
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
         adaptiveSignalStream->next({});
         return;
      }

      switch (buffer.type())
      {
         // adaptive resample for real signal
         case hw::SignalType::SIGNAL_TYPE_STM_LOGIC:
         {
            hw::SignalBuffer resampled(buffer.elements() * 2, 2, 1, buffer.sampleRate(), buffer.offset(), 0, hw::SignalType::SIGNAL_TYPE_ADV_LOGIC, buffer.id());

            // get value of the first sample
            float last = buffer[0];

            // and store in resampled buffer
            resampled.put(last).put(0.0);

            // adaptive resample based values changes (logic)
            for (int i = 1, c = 0; i < buffer.limit(); i++)
            {
               float value = buffer[i];

               // store new sample if different from last or every LOGIC_INTERVAL samples
               if (value != last || (i - c) >= LOGIC_INTERVAL)
               {
                  resampled.put(value).put(static_cast<float>(i));

                  // update last value
                  last = value;

                  // update control point index
                  c = i;
               }
            }

            resampled.flip();

            adaptiveSignalStream->next(resampled);

            taskThroughput.update(buffer.elements());

            break;
         }

         case hw::SignalType::SIGNAL_TYPE_RAW_REAL:
         {
            hw::SignalBuffer resampled(buffer.elements() * 2, 2, 1, buffer.sampleRate(), buffer.offset(), 0, hw::SignalType::SIGNAL_TYPE_ADV_REAL, buffer.id());

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
            for (int r = i - (WINDOW / 2) - 1, a = i + (WINDOW / 2); i < buffer.limit(); i++, p++, a++, r++)
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
               resampled.put(last).put(float(p));

            resampled.flip();

            adaptiveSignalStream->next(resampled);

            taskThroughput.update(buffer.elements());

            break;
         }

         default:
            break;
      }
   }
};

SignalResamplingTask::SignalResamplingTask() : Worker("AdaptiveSamplingTask")
{
}

rt::Worker *SignalResamplingTask::construct()
{
   return new Impl;
}

}
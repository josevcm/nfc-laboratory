/*

  Copyright (c) 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <mutex>

#include <rt/Logger.h>
#include <rt/BlockingQueue.h>

#include <nfc/AdaptiveSamplingTask.h>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>

#include "AbstractTask.h"

#define WINDOW 101
#define THRESHOLD 0.005

namespace nfc {

struct AdaptiveSamplingTask::Impl : AdaptiveSamplingTask, AbstractTask
{
   // signal buffer frame stream subject
   rt::Subject<sdr::SignalBuffer> *samplingStream = nullptr;

   // signal buffer frame stream subject
   rt::Subject<sdr::SignalBuffer> *adaptiveStream = nullptr;

   // signal stream subscription
   rt::Subject<sdr::SignalBuffer>::Subscription samplingSubscription;

   // signal stream queue buffer
   rt::BlockingQueue<sdr::SignalBuffer> signalQueue;

   // stream lock
   std::mutex signalMutex;

   explicit Impl() : AbstractTask("AdaptiveSamplingTask", "adaptive")
   {
      // access to signal subject stream
      samplingStream = rt::Subject<sdr::SignalBuffer>::name("signal.real");

      // access to signal subject stream
      adaptiveStream = rt::Subject<sdr::SignalBuffer>::name("signal.adaptive");

      // subscribe to signal events
      samplingSubscription = samplingStream->subscribe([=](const sdr::SignalBuffer &buffer) {
         signalQueue.add(buffer);
      });
   }

   ~Impl() override = default;

   void start() override
   {
   }

   void stop() override
   {
   }

   bool loop() override
   {
      /*
       * first process pending commands
       */
      if (auto command = commandQueue.get())
      {
         log.info("adaptive command [{}]", {command->code});
      }

      if (auto buffer = signalQueue.get(50))
      {
         if (buffer->isValid())
         {
            process(buffer.value());
         }
      }

      return true;
   }

   void process(const sdr::SignalBuffer &buffer) const
   {
      sdr::SignalBuffer resampled(buffer.elements() * 2, 2, buffer.sampleRate(), buffer.offset(), 0, sdr::SignalType::ADAPTIVE_REAL);

      float avrg = 0;
      float last = buffer[0];
      float step = 1.0f / float(buffer.sampleRate());
      float start = float(buffer.offset()) / float(buffer.sampleRate());
      float filter = THRESHOLD;

      // initialize average
      for (int i = 0; i < (WINDOW / 2); i++)
         avrg += buffer[i];

      // always store first sample
      resampled.put(start).put(buffer[0]);

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
         float stdev = abs(value - (avrg / float(WINDOW)));

         // filter values
         if (stdev > filter || (i - c) > 100)
         {
            float rp = fmaf(step, p, start); // ri = step * p + start
            float ri = fmaf(step, i, start); // ri = step * i + start

            // append control point
            if (stdev > filter && c < p)
               resampled.put(rp).put(last);

            // append new value
            resampled.put(ri).put(value);

            // update control point index
            c = i;
         }

         // store last value
         last = value;
      }

      // store last sample
      if (c < p)
         resampled.put(fmaf(step, p, start)).put(last);

      resampled.flip();

      adaptiveStream->next(resampled);
   }
};

AdaptiveSamplingTask::AdaptiveSamplingTask() : rt::Worker("AdaptiveSamplingTask")
{
}

rt::Worker *AdaptiveSamplingTask::construct()
{
   return new AdaptiveSamplingTask::Impl;
}

}
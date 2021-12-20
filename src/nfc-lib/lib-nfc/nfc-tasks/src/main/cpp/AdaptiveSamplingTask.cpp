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

#define WINDOW 51
#define THRESHOLD 0.005

namespace nfc {

struct AdaptiveSamplingTask::Impl : AdaptiveSamplingTask, AbstractTask
{
   // signal buffer frame stream subject
   rt::Subject<sdr::SignalBuffer> *signalRawStream = nullptr;

   // signal buffer frame stream subject
   rt::Subject<sdr::SignalBuffer> *signalAdpStream = nullptr;

   // signal stream subscription
   rt::Subject<sdr::SignalBuffer>::Subscription signalSubscription;

   // signal stream queue buffer
   rt::BlockingQueue<sdr::SignalBuffer> signalQueue;

   // stream lock
   std::mutex signalMutex;

   explicit Impl() : AbstractTask("AdaptiveSamplingTask", "adaptive")
   {
      // access to signal subject stream
      signalRawStream = rt::Subject<sdr::SignalBuffer>::name("signal.raw");

      // access to signal subject stream
      signalAdpStream = rt::Subject<sdr::SignalBuffer>::name("signal.adp");

      // subscribe to signal events
      signalSubscription = signalRawStream->subscribe([=](const sdr::SignalBuffer &buffer) {
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
         log.debug("adaptive command [{}]", {command->code});
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
         float stdev = abs(value - (avrg / float(WINDOW)));

         // filter values
         if (stdev > filter || (i - c) > 100)
         {
            // append control point
            if (stdev > filter && c < p)
               resampled.put(last).put(float(p));

            // append new value
            resampled.put(value).put(float(i));

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

      signalAdpStream->next(resampled);
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
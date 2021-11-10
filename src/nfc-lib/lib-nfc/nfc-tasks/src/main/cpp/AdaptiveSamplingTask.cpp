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

   // last signal average
   float signalAverage;

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

   void process(const sdr::SignalBuffer &buffer)
   {
      sdr::SignalBuffer resampled(buffer.elements() * 2, 2, buffer.sampleRate(), buffer.offset(), 0, sdr::SignalType::ADAPTIVE_REAL);

      float last = buffer[0];
      float step = 1.0f / float(buffer.sampleRate());
      float start = float(buffer.offset()) / float(buffer.sampleRate());

      // initialize average on first buffer
      if (buffer.offset() == 0)
         signalAverage = buffer[0];

      // store first sample
      resampled.put(start).put(buffer[0]);

      // index of previous inserted point and last control point
      int p = 0, c = 0;

      // adaptive resample
      for (int i = 1; i < buffer.limit(); p = i, i++)
      {
         float value = buffer[i];

         // update average
         signalAverage = value * 0.01f + signalAverage * (1 - 0.01f);

         // detect deviation from average
         bool insert = abs(value - signalAverage) > 0.005;

         // filter values
         if (insert || (i - c) > 100)
         {
            // append control point only if deviation is over threshold
            if (insert && c < p)
               resampled.put(start + step * p).put(last);

            // append new value
            resampled.put(start + step * i).put(value);

            // update control point index
            c = i;
         }

         last = value;
      }

      // store last sample
      if (c < p)
         resampled.put(start + step * p).put(last);

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
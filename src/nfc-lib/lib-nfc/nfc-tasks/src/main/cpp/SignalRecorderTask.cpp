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

#if defined(__SSE2__) && defined(USE_SSE2)

#include <x86intrin.h>

#endif

#include <rt/Logger.h>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>
#include <sdr/RecordDevice.h>

#include <nfc/SignalRecorderTask.h>

#include "AbstractTask.h"

namespace nfc {

struct SignalRecorderTask::Impl : SignalRecorderTask, AbstractTask
{
   // decoder status
   int status;

   // signal buffer frame stream subject
   rt::Subject<sdr::SignalBuffer> *signalIqStream = nullptr;
   rt::Subject<sdr::SignalBuffer> *signalRvStream = nullptr;

   // signal stream subscription
   rt::Subject<sdr::SignalBuffer>::Subscription signalIqSubscription;
   rt::Subject<sdr::SignalBuffer>::Subscription signalRvSubscription;

   // signal stream queue buffer
   rt::BlockingQueue<sdr::SignalBuffer> signalQueue;

   // last status sent
   std::chrono::time_point<std::chrono::steady_clock> lastStatus;

   // record device
   std::shared_ptr<sdr::RecordDevice> device;

   Impl() : AbstractTask("SignalRecorderTask", "recorder"), status(SignalRecorderTask::Idle)
   {
      // access to signal subject stream
      signalIqStream = rt::Subject<sdr::SignalBuffer>::name("signal.iq");
      signalRvStream = rt::Subject<sdr::SignalBuffer>::name("signal.raw");

      // subscribe to signal events
//      signalIqSubscription = signalIqStream->subscribe([this](const sdr::SignalBuffer &buffer) {
//         if (status == SignalRecorderTask::Writing || status == SignalRecorderTask::Capture)
//            signalQueue.add(buffer);
//      });

      signalRvSubscription = signalRvStream->subscribe([this](const sdr::SignalBuffer &buffer) {
         if (status == SignalRecorderTask::Writing || status == SignalRecorderTask::Capture)
            signalQueue.add(buffer);
      });
   }

   void start() override
   {
   }

   void stop() override
   {
      close();
   }

   bool loop() override
   {
      /*
       * first process pending commands
       */
      if (auto command = commandQueue.get())
      {
         log.debug("recorder command [{}]", {command->code});

         if (command->code == SignalRecorderTask::Read)
         {
            readFile(command.value());
         }
         else if (command->code == SignalRecorderTask::Write)
         {
            writeFile(command.value());
         }
         else if (command->code == SignalRecorderTask::Stop)
         {
            closeFile(command.value());
         }
         else if (command->code == SignalRecorderTask::Capture)
         {
            startCapture(command.value());
         }
         else if (command->code == SignalRecorderTask::Replay)
         {
            startReplay(command.value());
         }
      }

      /*
       * now process device reading
       */
      if (status == SignalRecorderTask::Reading)
      {
         signalRead();
      }
      else if (status == SignalRecorderTask::Writing)
      {
         signalWrite();
      }
      else if (status == SignalRecorderTask::Capture)
      {
         signalCapture();
      }
      else if (status == SignalRecorderTask::Replaying)
      {
         signalReplay();
      }
      else
      {
         wait(50);
      }

      /*
      * update recorder status
      */
//      if ((std::chrono::steady_clock::now() - lastStatus) > std::chrono::milliseconds(500))
//      {
//         updateRecorderStatus();
//      }

      return true;
   }

   void readFile(const rt::Event &command)
   {
      if (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log.info("read file command: {}", {config.dump()});

         if (config.contains("fileName"))
         {
            device = std::make_shared<sdr::RecordDevice>(config["fileName"]);

            signalQueue.clear();

            if (device->open(sdr::SignalDevice::Read))
            {
               if (device->channelCount() <= 2)
               {
                  log.info("streaming started for file [{}]", {device->name()});

                  command.resolve();

                  updateRecorderStatus(SignalRecorderTask::Reading);

                  return;
               }
               else
               {
                  log.warn("too many channels in file [{}]", {device->name()});

                  device.reset();
               }
            }
            else
            {
               log.warn("unable to open file [{}]", {device->name()});

               device.reset();
            }
         }
         else
         {
            log.info("recording failed, no file name");
         }
      }
      else
      {
         log.info("recording failed, invalid command data");
      }

      command.reject();

      updateRecorderStatus(SignalRecorderTask::Idle);
   }

   void writeFile(const rt::Event &command)
   {
      if (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log.info("write file command: {}", {config.dump()});

         if (config.contains("fileName"))
         {
            device = std::make_shared<sdr::RecordDevice>(config["fileName"]);

            if (config.contains("sampleRate"))
               device->setSampleRate(config["sampleRate"]);
            else
               device->setSampleRate(10E6);

            if (config.contains("channelCount"))
               device->setChannelCount(config["channelCount"]);
            else
               device->setChannelCount(1);

            signalQueue.clear();

            if (device->open(sdr::SignalDevice::Write))
            {
               log.info("enable recording {}", {device->name()});

               command.resolve();

               updateRecorderStatus(SignalRecorderTask::Writing);

               return;
            }
            else
            {
               log.info("enable recording {} failed!", {device->name()});

               device.reset();
            }
         }
         else
         {
            log.info("recording failed, no file name");
         }
      }
      else
      {
         log.info("recording failed, invalid command data");
      }

      command.reject();

      updateRecorderStatus(SignalRecorderTask::Idle);
   }

   void closeFile(const rt::Event &command)
   {
      close();

      command.resolve();

      updateRecorderStatus(SignalRecorderTask::Idle);
   }

   void startCapture(const rt::Event &command)
   {
      command.resolve();

      updateRecorderStatus(SignalRecorderTask::Buffering);
   }

   void startReplay(const rt::Event &command)
   {
      command.resolve();

      updateRecorderStatus(SignalRecorderTask::Replaying);
   }

   void signalRead()
   {
      if (device && device->isOpen())
      {
         int sampleRate = device->sampleRate();
         int channelCount = device->channelCount();
         int sampleOffset = device->sampleOffset();

         switch (channelCount)
         {
            case 1:
            {
               sdr::SignalBuffer buffer(65536 * channelCount, 1, sampleRate, sampleOffset, 0, sdr::SignalType::SAMPLE_REAL);

               if (device->read(buffer) > 0)
               {
                  signalRvStream->next(buffer);
               }

               break;
            }
            case 2:
            {
               sdr::SignalBuffer buffer(65536 * channelCount, 2, sampleRate, sampleOffset >> 1, 0, sdr::SignalType::SAMPLE_IQ);

               if (device->read(buffer) > 0)
               {
                  sdr::SignalBuffer result(buffer.elements(), 1, buffer.sampleRate(), buffer.offset(), 0, sdr::SignalType::SAMPLE_REAL);

                  float *src = buffer.data();
                  float *dst = result.pull(buffer.elements());

                  // compute real signal value
#if defined(__SSE2__) && defined(USE_SSE2)
                  for (int j = 0, n = 0; j < buffer.elements(); j += 8, n += 16)
                  {
                     // load 8 I/Q vectors
                     __m128 a1 = _mm_load_ps(src + n + 0);  // I0, Q0, I1, Q1
                     __m128 a2 = _mm_load_ps(src + n + 4);  // I2, Q2, I3, Q3
                     __m128 a3 = _mm_load_ps(src + n + 8);  // I4, Q4, I5, Q5
                     __m128 a4 = _mm_load_ps(src + n + 12); // I6, Q6, I7, Q7

                     // square all components
                     __m128 p1 = _mm_mul_ps(a1, a1); // I0^2, Q0^2, I1^2, Q1^2
                     __m128 p2 = _mm_mul_ps(a2, a2); // I2^2, Q2^2, I3^2, Q3^2
                     __m128 p3 = _mm_mul_ps(a3, a3); // I4^2, Q4^2, I5^2, Q5^2
                     __m128 p4 = _mm_mul_ps(a4, a4); // I6^2, Q6^2, I6^2, Q7^2

                     // permute components
                     __m128 i1 = _mm_shuffle_ps(p1, p2, _MM_SHUFFLE(2, 0, 2, 0)); // I0^2, I1^2, I2^2, I3^2
                     __m128 i2 = _mm_shuffle_ps(p3, p4, _MM_SHUFFLE(2, 0, 2, 0)); // I4^2, I5^2, I6^2, I7^2
                     __m128 q1 = _mm_shuffle_ps(p1, p2, _MM_SHUFFLE(3, 1, 3, 1)); // Q0^2, Q1^2, Q2^2, Q3^2
                     __m128 q2 = _mm_shuffle_ps(p3, p4, _MM_SHUFFLE(3, 1, 3, 1)); // Q4^2, Q5^2, Q6^2, Q7^2

                     // add vector components
                     __m128 r1 = _mm_add_ps(i1, q1); // I0^2+Q0^2, I1^2+Q1^2, I2^2+Q2^2, I3^2+Q3^2
                     __m128 r2 = _mm_add_ps(i2, q2); // I4^2+Q4^2, I5^2+Q5^2, I6^2+Q6^2, I7^2+Q7^2

                     // square-root vectors
                     __m128 m1 = _mm_sqrt_ps(r1); // sqrt(I0^2+Q0^2), sqrt(I1^2+Q1^2), sqrt(I2^2+Q2^2), sqrt(I3^2+Q3^2)
                     __m128 m2 = _mm_sqrt_ps(r2); // sqrt(I4^2+Q4^2), sqrt(I5^2+Q5^2), sqrt(I6^2+Q6^2), sqrt(I7^2+Q7^2)

                     // store results
                     _mm_store_ps(dst + j + 0, m1);
                     _mm_store_ps(dst + j + 4, m2);
                  }
#else
#pragma GCC ivdep
                  for (int j = 0, n = 0; j < buffer.elements(); j += 4, n += 8)
                  {
                     dst[j + 0] = sqrtf(src[n + 0] * src[n + 0] + src[n + 1] * src[n + 1]);
                     dst[j + 1] = sqrtf(src[n + 2] * src[n + 2] + src[n + 3] * src[n + 3]);
                     dst[j + 2] = sqrtf(src[n + 4] * src[n + 4] + src[n + 5] * src[n + 5]);
                     dst[j + 3] = sqrtf(src[n + 6] * src[n + 6] + src[n + 7] * src[n + 7]);
                  }
#endif
                  // flip buffer pointers
                  result.flip();

                  // send IQ value buffer
                  signalIqStream->next(buffer);

                  // send Real value buffer
                  signalRvStream->next(result);
               }

               break;
            }

            default:
            {
               device->close();
            }
         }

         if (device->isEof() || !device->isOpen())
         {
            log.info("streaming finished for file [{}]", {device->name()});

            // send null buffer for EOF
            signalIqStream->next({});
            signalRvStream->next({});

            // close file
            device.reset();

            // update status
            updateRecorderStatus(SignalRecorderTask::Idle);
         }
      }
   }

   void signalWrite()
   {
      if (device && device->isOpen())
      {
         if (auto buffer = signalQueue.get())
         {
            if (!buffer->isEmpty())
            {
               device->write(buffer.value());
            }
         }
      }
   }

   void signalCapture()
   {
   }

   void signalReplay()
   {
   }

   void close()
   {
      device.reset();
   }

   void updateRecorderStatus(int value)
   {
      status = value;

      json data;

      // data status
      switch (status)
      {
         case Idle:
            data["status"] = "idle";
            break;
         case Reading:
            data["status"] = "reading";
            break;
         case Writing:
            data["status"] = "writing";
            break;
         case Buffering:
            data["status"] = "buffering";
            break;
         case Replaying:
            data["status"] = "replaying";
            break;
      }

      if (device)
      {
         data["file"] = device->name();
         data["channelCount"] = device->channelCount();
         data["sampleCount"] = device->sampleCount();
         data["sampleOffset"] = device->sampleOffset();
         data["sampleRate"] = device->sampleRate();
         data["sampleSize"] = device->sampleSize();
         data["sampleType"] = device->sampleType();
         data["streamTime"] = device->streamTime();
      }

      log.info("updated recorder status: {}", {data.dump()});

      updateStatus(status, data);

      lastStatus = std::chrono::steady_clock::now();
   }
};

SignalRecorderTask::SignalRecorderTask() : rt::Worker("SignalRecorderTask")
{
}

rt::Worker *SignalRecorderTask::construct()
{
   return new SignalRecorderTask::Impl;
}

}

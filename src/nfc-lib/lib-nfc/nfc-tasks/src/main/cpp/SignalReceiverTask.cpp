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

#ifdef __SSE2__

#include <x86intrin.h>

#endif

#include <rt/Logger.h>
#include <rt/Format.h>
#include <rt/BlockingQueue.h>
#include <rt/Throughput.h>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>
#include <sdr/AirspyDevice.h>

#include <nfc/SignalReceiverTask.h>

#include "AbstractTask.h"

namespace nfc {

#define LOWER_GAIN_THRESHOLD 0.05
#define UPPER_GAIN_THRESHOLD 0.25

struct SignalReceiverTask::Impl : SignalReceiverTask, AbstractTask
{
   // radio device
   std::shared_ptr<sdr::RadioDevice> receiver;

   // signal stream subject for raw data
   rt::Subject<sdr::SignalBuffer> *signalRvStream = nullptr;

   // signal stream subject for IQ data
   rt::Subject<sdr::SignalBuffer> *signalIqStream = nullptr;

   // signal stream queue buffer
   rt::BlockingQueue<sdr::SignalBuffer> signalQueue;

   // throughput meter
   rt::Throughput taskThroughput;

   // last detection attempt
   std::chrono::time_point<std::chrono::steady_clock> lastSearch;

   // current receiver gain mode
   int receiverGainMode = 0;

   // current receiver gain value
   int receiverGainValue = 0;

   // last control offset
   unsigned int receiverGainChange = 0;

   Impl() : AbstractTask("SignalReceiverTask", "receiver")
   {
      signalRvStream = rt::Subject<sdr::SignalBuffer>::name("signal.raw");
      signalIqStream = rt::Subject<sdr::SignalBuffer>::name("signal.iq");
   }

   void start() override
   {
      refresh();
   }

   void stop() override
   {
      if (receiver)
      {
         log.info("shutdown device {}", {receiver->name()});
         receiver.reset();
      }
   }

   bool loop() override
   {
      /*
       * process pending commands
       */
      if (auto command = commandQueue.get())
      {
         log.debug("receiver command [{}]", {command->code});

         if (command->code == SignalReceiverTask::Start)
         {
            startReceiver(command.value());
         }
         else if (command->code == SignalReceiverTask::Stop)
         {
            stopReceiver(command.value());
         }
         else if (command->code == SignalReceiverTask::Query)
         {
            queryReceiver(command.value());
         }
         else if (command->code == SignalReceiverTask::Configure)
         {
            configReceiver(command.value());
         }
      }

      /*
      * process device refresh
      */
      if ((std::chrono::steady_clock::now() - lastSearch) > std::chrono::milliseconds(1000))
      {
         refresh();

         if (receiver && receiver->isStreaming())
         {
            log.info("average throughput {.2} Msps", {taskThroughput.average() / 1E6});
         }
      }

      processQueue(50);

      return true;
   }

   void refresh()
   {
      int mode = SignalReceiverTask::Statistics;

      if (!receiver)
      {
         // open first available receiver
         for (const auto &name: sdr::AirspyDevice::listDevices())
         {
            // create device instance
            receiver.reset(new sdr::AirspyDevice(name));

            // default parameters
            receiver->setCenterFreq(13.56E6);
            receiver->setSampleRate(10E6);
            receiver->setGainMode(0);
            receiver->setGainValue(0);
            receiver->setMixerAgc(0);
            receiver->setTunerAgc(0);

            // try to open...
            if (receiver->open(sdr::SignalDevice::Read))
            {
               log.info("device {} connected!", {name});

               mode = SignalReceiverTask::Attach;

               break;
            }

            receiver.reset();

            log.warn("device {} open failed", {name});
         }
      }
      else if (!receiver->isReady())
      {
         log.warn("device {} disconnected", {receiver->name()});

         // send null buffer for EOF
         signalIqStream->next({});

         // send null buffer for EOF
         signalRvStream->next({});

         // close device
         receiver.reset();
      }

      // update receiver status
      updateReceiverStatus(mode);

      // store last search time
      lastSearch = std::chrono::steady_clock::now();
   }

   void startReceiver(const rt::Event &command)
   {
      if (receiver)
      {
         log.info("start streaming for device {}", {receiver->name()});

         // read current gain mode and value
         receiverGainChange = 0;

         log.info("gain mode {} gain value {}", {receiverGainMode, receiverGainValue});

         // start receiving
         receiver->start([this](sdr::SignalBuffer &buffer) {
            signalQueue.add(buffer);
         });

         command.resolve();

         updateReceiverStatus(SignalReceiverTask::Streaming);
      }
   }

   void stopReceiver(const rt::Event &command)
   {
      if (receiver)
      {
         log.info("stop streaming for device {}", {receiver->name()});

         receiver->stop();

         command.resolve();

         updateReceiverStatus(SignalReceiverTask::Halt);
      }
   }

   void queryReceiver(const rt::Event &command)
   {
      if (receiver)
      {
         log.info("query status for device {}", {receiver->name()});

         command.resolve();

         updateReceiverStatus(SignalReceiverTask::Query);
      }
   }

   void configReceiver(const rt::Event &command)
   {
      if (receiver)
      {
         if (auto data = command.get<std::string>("data"))
         {
            auto config = json::parse(data.value());

            log.info("change receiver config {}: {}", {receiver->name(), config.dump()});

            if (config.contains("centerFreq"))
               receiver->setCenterFreq(config["centerFreq"]);

            if (config.contains("sampleRate"))
               receiver->setSampleRate(config["sampleRate"]);

            if (config.contains("tunerAgc"))
               receiver->setTunerAgc(config["tunerAgc"]);

            if (config.contains("mixerAgc"))
               receiver->setMixerAgc(config["mixerAgc"]);

            if (config.contains("gainMode"))
            {
               receiverGainMode = config["gainMode"];

               if (receiverGainMode > 0)
               {
                  receiver->setGainMode(receiverGainMode);
               }
               else
               {
                  receiverGainValue = 0;
                  receiver->setGainMode(sdr::AirspyDevice::Linearity);
                  receiver->setGainValue(receiverGainValue);
               }
            }

            if (config.contains("gainValue"))
            {
               if (receiverGainMode > 0)
               {
                  receiverGainValue = config["gainValue"];
                  receiver->setGainValue(config["gainValue"]);
               }
            }
         }
      }

      command.resolve();

      updateReceiverStatus(SignalReceiverTask::Config);
   }

   void updateReceiverStatus(int event)
   {
      json data;

      if (receiver)
      {
         // data name and status
         data["name"] = receiver->name();
         data["version"] = receiver->version();
         data["status"] = receiver->isStreaming() ? "streaming" : "idle";

         // data parameters
         data["centerFreq"] = receiver->centerFreq();
         data["sampleRate"] = receiver->sampleRate();
         data["gainMode"] = receiver->gainMode();
         data["gainValue"] = receiver->gainValue();
         data["mixerAgc"] = receiver->mixerAgc();
         data["tunerAgc"] = receiver->tunerAgc();

         // data statistics
         data["samplesReceived"] = receiver->samplesReceived();
         data["samplesDropped"] = receiver->samplesDropped();

         // send capabilities on data attach
         if (event == SignalReceiverTask::Attach)
         {
            data["gainModes"].push_back({
                                              {"value", 0},
                                              {"name",  "Auto"}
                                        });

            for (const auto &entry: receiver->supportedGainModes())
            {
               if (entry.first > 0)
               {
                  data["gainModes"].push_back({
                                                    {"value", entry.first},
                                                    {"name",  entry.second}
                                              });
               }
            }

            for (const auto &entry: receiver->supportedGainValues())
            {
               data["gainValues"].push_back({
                                                  {"value", entry.first},
                                                  {"name",  entry.second}
                                            });
            }

            for (const auto &entry: receiver->supportedSampleRates())
            {
               data["sampleRates"].push_back({
                                                   {"value", entry.first},
                                                   {"name",  entry.second}
                                             });
            }
         }
      }
      else
      {
         data["status"] = "absent";
      }

      updateStatus(event, data);
   }

   void processQueue(int timeout)
   {
      if (auto entry = signalQueue.get(timeout))
      {
         sdr::SignalBuffer buffer = entry.value();
         sdr::SignalBuffer result(buffer.elements(), 1, buffer.sampleRate(), buffer.offset(), 0, sdr::SignalType::SAMPLE_REAL);

         float *src = buffer.data();
         float *dst = result.pull(buffer.elements());
         float avrg = 0;

         taskThroughput.begin();

         // compute real signal value and average value
#ifdef __SSE2__
         for (int j = 0, n = 0; j < buffer.elements(); j += 16, n += 32)
         {
            // load 16 I/Q vectors
            __m128 a0 = _mm_load_ps(src + n + 0);  // I0, Q0, I1, Q1
            __m128 a1 = _mm_load_ps(src + n + 4);  // I2, Q2, I3, Q3
            __m128 a2 = _mm_load_ps(src + n + 8);  // I4, Q4, I5, Q5
            __m128 a3 = _mm_load_ps(src + n + 12); // I6, Q6, I7, Q7
            __m128 a4 = _mm_load_ps(src + n + 16);  // I8, Q8, I9, Q9
            __m128 a5 = _mm_load_ps(src + n + 20); // I10, Q10, I11, Q11
            __m128 a6 = _mm_load_ps(src + n + 24);  // I12, Q12, I13, Q13
            __m128 a7 = _mm_load_ps(src + n + 28); // I14, Q14, I15, Q15

            // square all components
            __m128 p0 = _mm_mul_ps(a0, a0); // I0^2, Q0^2, I1^2, Q1^2
            __m128 p1 = _mm_mul_ps(a1, a1); // I2^2, Q2^2, I3^2, Q3^2
            __m128 p2 = _mm_mul_ps(a2, a2); // I4^2, Q4^2, I5^2, Q5^2
            __m128 p3 = _mm_mul_ps(a3, a3); // I6^2, Q6^2, I7^2, Q7^2
            __m128 p4 = _mm_mul_ps(a4, a4); // I8^2, Q8^2, I9^2, Q9^2
            __m128 p5 = _mm_mul_ps(a5, a5); // I10^2, Q10^2, I11^2, Q11^2
            __m128 p6 = _mm_mul_ps(a6, a6); // I12^2, Q12^2, I13^2, Q13^2
            __m128 p7 = _mm_mul_ps(a7, a7); // I14^2, Q14^2, I15^2, Q15^2

            // permute components
            __m128 i0 = _mm_shuffle_ps(p0, p1, _MM_SHUFFLE(2, 0, 2, 0)); // I0^2, I1^2, I2^2, I3^2
            __m128 i1 = _mm_shuffle_ps(p2, p3, _MM_SHUFFLE(2, 0, 2, 0)); // I4^2, I5^2, I6^2, I7^2
            __m128 i2 = _mm_shuffle_ps(p4, p5, _MM_SHUFFLE(2, 0, 2, 0)); // I8^2, I9^2, I10^2, I11^2
            __m128 i3 = _mm_shuffle_ps(p6, p7, _MM_SHUFFLE(2, 0, 2, 0)); // I12^2, I13^2, I14^2, I15^2
            __m128 q0 = _mm_shuffle_ps(p0, p1, _MM_SHUFFLE(3, 1, 3, 1)); // Q0^2, Q1^2, Q2^2, Q3^2
            __m128 q1 = _mm_shuffle_ps(p2, p3, _MM_SHUFFLE(3, 1, 3, 1)); // Q4^2, Q5^2, Q6^2, Q7^2
            __m128 q2 = _mm_shuffle_ps(p4, p5, _MM_SHUFFLE(3, 1, 3, 1)); // Q8^2, Q9^2, Q10^2, Q11^2
            __m128 q3 = _mm_shuffle_ps(p6, p7, _MM_SHUFFLE(3, 1, 3, 1)); // Q12^2, Q13^2, Q14^2, Q15^2

            // add vector components
            __m128 r0 = _mm_add_ps(i0, q0); // I0^2+Q0^2, I1^2+Q1^2, I2^2+Q2^2, I3^2+Q3^2
            __m128 r1 = _mm_add_ps(i1, q1); // I4^2+Q4^2, I5^2+Q5^2, I6^2+Q6^2, I7^2+Q7^2
            __m128 r2 = _mm_add_ps(i2, q2); // I8^2+Q8^2, I9^2+Q1^2, I10^2+Q10^2, I11^2+Q11^2
            __m128 r3 = _mm_add_ps(i3, q3); // I12^2+Q12^2, I13^2+Q13^2, I14^2+Q14^2, I15^2+Q15^2

            // square-root vectors
            __m128 m0 = _mm_sqrt_ps(r0); // sqrt(I0^2+Q0^2), sqrt(I1^2+Q1^2), sqrt(I2^2+Q2^2), sqrt(I3^2+Q3^2)
            __m128 m1 = _mm_sqrt_ps(r1); // sqrt(I4^2+Q4^2), sqrt(I5^2+Q5^2), sqrt(I6^2+Q6^2), sqrt(I7^2+Q7^2)
            __m128 m2 = _mm_sqrt_ps(r2); // sqrt(I8^2+Q8^2), sqrt(I9^2+Q9^2), sqrt(I10^2+Q10^2), sqrt(I11^2+Q11^2)
            __m128 m3 = _mm_sqrt_ps(r3); // sqrt(I12^2+Q12^2), sqrt(I13^2+Q13^2), sqrt(I14^2+Q14^2), sqrt(I15^2+Q15^2)

            // store results
            _mm_store_ps(dst + j + 0, m0);
            _mm_store_ps(dst + j + 4, m1);
            _mm_store_ps(dst + j + 8, m2);
            _mm_store_ps(dst + j + 12, m3);

            // compute exponential average
            avrg = avrg * (1 - 0.001f) + dst[j + 0] * 0.001f;
            avrg = avrg * (1 - 0.001f) + dst[j + 4] * 0.001f;
            avrg = avrg * (1 - 0.001f) + dst[j + 8] * 0.001f;
            avrg = avrg * (1 - 0.001f) + dst[j + 12] * 0.001f;
         }
#else
#pragma GCC ivdep
         for (int j = 0, n = 0; j < buffer.elements(); j += 4, n += 8)
         {
            dst[j + 0] = sqrtf(src[n + 0] * src[n + 0] + src[n + 1] * src[n + 1]);
            dst[j + 1] = sqrtf(src[n + 2] * src[n + 2] + src[n + 3] * src[n + 3]);
            dst[j + 2] = sqrtf(src[n + 4] * src[n + 4] + src[n + 5] * src[n + 5]);
            dst[j + 3] = sqrtf(src[n + 6] * src[n + 6] + src[n + 7] * src[n + 7]);

            avrg = avrg * (1 - 0.001f) + dst[j + 0] * 0.001f;
         }
#endif

         taskThroughput.update(buffer.elements());

         // flip buffer pointers
         result.flip();

         // send IQ value buffer
         signalIqStream->next(buffer);

         // send Real value buffer
         signalRvStream->next(result);

         // if automatic gain control is engaged adjust gain dynamically
         if (receiverGainMode == 0 && buffer.offset() > receiverGainChange)
         {
            // for weak signals, increase receiver gain
            if (avrg < LOWER_GAIN_THRESHOLD && receiverGainValue < 6)
            {
               receiverGainChange = buffer.offset() + buffer.elements();
               receiver->setGainValue(++receiverGainValue);
               log.info("increase gain {}", {receiverGainValue});
            }

            // for strong signals, decrease receiver gain
            if (avrg > UPPER_GAIN_THRESHOLD && receiverGainValue > 0)
            {
               receiverGainChange = buffer.offset() + buffer.elements();
               receiver->setGainValue(--receiverGainValue);
               log.info("decrease gain {}", {receiverGainValue});
            }
         }
      }
   }
};

SignalReceiverTask::SignalReceiverTask() : rt::Worker("SignalReceiverTask")
{
}

rt::Worker *SignalReceiverTask::construct()
{
   return new SignalReceiverTask::Impl;
}

}
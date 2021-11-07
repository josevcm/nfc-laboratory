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

#include <rt/Logger.h>
#include <rt/Format.h>
#include <rt/BlockingQueue.h>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>
#include <sdr/AirspyDevice.h>

#include <nfc/SignalReceiverTask.h>

#include "AbstractTask.h"

namespace nfc {

struct SignalReceiverTask::Impl : SignalReceiverTask, AbstractTask
{
   // radio device
   std::shared_ptr<sdr::RadioDevice> receiver;

   // signal stream subject for IQ data
   rt::Subject<sdr::SignalBuffer> *signalIQStream = nullptr;

   // signal stream subject for real data
   rt::Subject<sdr::SignalBuffer> *signalRealStream = nullptr;

   // signal stream queue buffer
   rt::BlockingQueue<sdr::SignalBuffer> signalQueue;

   // last detection attempt
   std::chrono::time_point<std::chrono::steady_clock> lastSearch;

   Impl() : AbstractTask("SignalReceiverTask", "receiver")
   {
      signalIQStream = rt::Subject<sdr::SignalBuffer>::name("signal.iq");
      signalRealStream = rt::Subject<sdr::SignalBuffer>::name("signal.real");
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
         log.info("receiver command [{}]", {command->code});

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
      if ((std::chrono::steady_clock::now() - lastSearch) > std::chrono::milliseconds(500))
      {
         refresh();
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
         signalIQStream->next({});

         // send null buffer for EOF
         signalRealStream->next({});

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
               receiver->setGainMode(config["gainMode"]);

            if (config.contains("gainValue"))
               receiver->setGainValue(config["gainValue"]);
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
            for (const auto &entry: receiver->supportedGainModes())
            {
               data["gainModes"].push_back({
                                                 {"value", entry.first},
                                                 {"name",  entry.second}
                                           });
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
         sdr::SignalBuffer result(buffer.elements(), 1, buffer.sampleRate(), buffer.offset(), 0, sdr::SignalType::REAL_VALUE);

         float *src = buffer.data();
         float *dst = result.pull(buffer.elements());

#pragma GCC ivdep
         for (int i = 0, n = 0; i < buffer.elements(); i++, n += 2)
         {
            dst[i] = sqrtf(src[n + 0] * src[n + 0] + src[n + 1] * src[n + 1]);
         }

         result.flip();

         // send IQ value buffer
         signalIQStream->next(buffer);

         // send Real value buffer
         signalRealStream->next(result);
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
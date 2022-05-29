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
#include <rt/BlockingQueue.h>
#include <rt/Throughput.h>

#include <nfc/NfcDecoder.h>
#include <nfc/FrameDecoderTask.h>

#include "AbstractTask.h"

namespace nfc {

struct FrameDecoderTask::Impl : FrameDecoderTask, AbstractTask
{
   // decoder status
   int status;

   // signal buffer frame stream subject
   rt::Subject<sdr::SignalBuffer> *signalStream = nullptr;

   // frame stream subject
   rt::Subject<nfc::NfcFrame> *frameStream = nullptr;

   // signal stream subscription
   rt::Subject<sdr::SignalBuffer>::Subscription signalSubscription;

   // signal stream queue buffer
   rt::BlockingQueue<sdr::SignalBuffer> signalQueue;

   // throughput meter
   rt::Throughput taskThroughput;

   // decoder
   std::shared_ptr<nfc::NfcDecoder> decoder;

   // last status sent
   std::chrono::time_point<std::chrono::steady_clock> lastStatus;

   // last Throughput statistics
   std::chrono::time_point<std::chrono::steady_clock> lastThroughput;

   Impl() : AbstractTask("FrameDecoderTask", "decoder"), status(FrameDecoderTask::Halt), decoder(new nfc::NfcDecoder())
   {
      // access to signal subject stream
      signalStream = rt::Subject<sdr::SignalBuffer>::name("signal.raw");

      // create frame stream subject
      frameStream = rt::Subject<nfc::NfcFrame>::name("decoder.frame");

      // subscribe to signal events
      signalSubscription = signalStream->subscribe([this](const sdr::SignalBuffer &buffer) {
         if (status == FrameDecoderTask::Listen)
            signalQueue.add(buffer);
      });
   }

   void start() override
   {
   }

   void stop() override
   {
   }

   bool loop() override
   {
      /*
       * process pending commands
       */
      if (auto command = commandQueue.get())
      {
         log.debug("decoder command [{}]", {command->code});

         if (command->code == FrameDecoderTask::Start)
         {
            startDecoder(command.value());
         }
         else if (command->code == FrameDecoderTask::Stop)
         {
            stopDecoder(command.value());
         }
         else if (command->code == FrameDecoderTask::Configure)
         {
            configDecoder(command.value());
         }
      }

      /*
       * process signal decode
       */
      if (status == FrameDecoderTask::Listen)
      {
         signalDecode();
      }
      else
      {
         wait(50);
      }

      return true;
   }

   void startDecoder(rt::Event &command)
   {
//      long epoch = (long) std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

      log.info("start frame decoding with {} pending buffers!", {signalQueue.size()});

      signalQueue.clear();

      decoder->initialize();

      command.resolve();

      updateDecoderStatus(FrameDecoderTask::Listen);
   }

   void stopDecoder(rt::Event &command)
   {
      log.info("stop frame decoding with {} pending buffers!", {signalQueue.size()});

      signalQueue.clear();

      for (const auto &frame: decoder->nextFrames({}))
      {
         frameStream->next(frame);
      }

      command.resolve();

      updateDecoderStatus(FrameDecoderTask::Halt);
   }

   void configDecoder(rt::Event &command)
   {
      if (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log.info("change decoder config: {}", {config.dump()});

         // NFC-A parameters
         if (config.contains("nfca"))
         {
            auto nfca = config["nfca"];

            float min = NAN;
            float max = NAN;

            if (nfca.contains("enabled"))
               decoder->setEnableNfcA(nfca["enabled"]);

            if (nfca.contains("minimumModulationDeep"))
               min = nfca["minimumModulationDeep"];

            if (nfca.contains("maximumModulationDeep"))
               max = nfca["maximumModulationDeep"];

            decoder->setModulationThresholdNfcA(min, max);
         }

         // NFC-B parameters
         if (config.contains("nfcb"))
         {
            auto nfcb = config["nfcb"];

            float min = NAN;
            float max = NAN;

            if (nfcb.contains("enabled"))
               decoder->setEnableNfcB(nfcb["enabled"]);

            if (nfcb.contains("minimumModulationDeep"))
               min = nfcb["minimumModulationDeep"];

            if (nfcb.contains("maximumModulationDeep"))
               max = nfcb["maximumModulationDeep"];

            decoder->setModulationThresholdNfcB(min, max);
         }

         // NFC-F parameters
         if (config.contains("nfcf"))
         {
            auto nfcf = config["nfcf"];

            float min = NAN;
            float max = NAN;

            if (nfcf.contains("enabled"))
               decoder->setEnableNfcF(nfcf["enabled"]);

            if (nfcf.contains("minimumModulationDeep"))
               min = nfcf["minimumModulationDeep"];

            if (nfcf.contains("maximumModulationDeep"))
               max = nfcf["maximumModulationDeep"];

            decoder->setModulationThresholdNfcF(min, max);
         }

         // NFC-V parameters
         if (config.contains("nfcv"))
         {
            auto nfcv = config["nfcv"];

            float min = NAN;
            float max = NAN;

            if (nfcv.contains("enabled"))
               decoder->setEnableNfcV(nfcv["enabled"]);

            if (nfcv.contains("minimumModulationDeep"))
               min = nfcv["minimumModulationDeep"];

            if (nfcv.contains("maximumModulationDeep"))
               max = nfcv["maximumModulationDeep"];

            decoder->setModulationThresholdNfcV(min, max);
         }

         // stream reference time
         if (config.contains("streamTime"))
            decoder->setStreamTime(config["streamTime"]);

         // global power level threshold
         if (config.contains("powerLevelThreshold"))
            decoder->setPowerLevelThreshold(config["powerLevelThreshold"]);

         // sample rate must be last value set
         if (config.contains("sampleRate"))
            decoder->setSampleRate(config["sampleRate"]);

         command.resolve();

         updateDecoderStatus(status, true);
      }
      else
      {
         command.reject();
      }
   }

   void signalDecode()
   {
      if (auto buffer = signalQueue.get())
      {
         taskThroughput.begin();

         for (const auto &frame: decoder->nextFrames(buffer.value()))
         {
            frameStream->next(frame);
         }

         taskThroughput.update(buffer->elements());

         if (!buffer->isValid())
         {
            log.info("decoder EOF buffer received, finish!");

            decoder->cleanup();

            updateDecoderStatus(FrameDecoderTask::Halt);
         }

         if ((std::chrono::steady_clock::now() - lastThroughput) > std::chrono::milliseconds(1000))
         {
            log.info("average throughput {.2} Msps", {taskThroughput.average() / 1E6});

            lastThroughput = std::chrono::steady_clock::now();
         }
      }
   }

   void updateDecoderStatus(int value, bool config = false)
   {
      status = value;

      json data({
                      {"status",     status == Listen ? "decoding" : "idle"},
                      {"queueSize",  signalQueue.size()},
                      {"sampleRate", decoder->sampleRate()},
                      {"streamTime", decoder->streamTime()}
                });

      if (config)
      {
         data["nfca"] = {
               {"enabled", decoder->isNfcAEnabled()}
         };

         data["nfcb"] = {
               {"enabled", decoder->isNfcBEnabled()}
         };

         data["nfcf"] = {
               {"enabled", decoder->isNfcFEnabled()}
         };

         data["nfcv"] = {
               {"enabled", decoder->isNfcVEnabled()}
         };
      }

      log.info("updated decoder status: {}", {data.dump()});

      updateStatus(status, data);

      lastStatus = std::chrono::steady_clock::now();
   }
};

FrameDecoderTask::FrameDecoderTask() : rt::Worker("FrameDecoderTask")
{
}

rt::Worker *FrameDecoderTask::construct()
{
   return new FrameDecoderTask::Impl;
}

}

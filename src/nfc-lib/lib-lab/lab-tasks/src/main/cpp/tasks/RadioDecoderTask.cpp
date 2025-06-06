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

#include <memory>

#include <rt/BlockingQueue.h>
#include <rt/Throughput.h>

#include <lab/nfc/NfcDecoder.h>

#include <lab/tasks/RadioDecoderTask.h>

#include "AbstractTask.h"

namespace lab {

struct RadioDecoderTask::Impl : RadioDecoderTask, AbstractTask
{
   // signal buffer frame stream subject
   rt::Subject<hw::SignalBuffer> *radioSignalStream = nullptr;

   // signal stream subscription
   rt::Subject<hw::SignalBuffer>::Subscription radioSignalSubscription;

   // frame stream subject
   rt::Subject<RawFrame> *decoderFrameStream = nullptr;

   // radio signal stream queue buffer
   rt::BlockingQueue<hw::SignalBuffer> radioSignalQueue;

   // byte throughput meter
   rt::Throughput taskThroughput;

   // decoder
   std::shared_ptr<NfcDecoder> decoder;

   // last Throughput statistics
   std::chrono::time_point<std::chrono::steady_clock> lastStatus;

   // control flags
   bool radioDecoderEnabled = false;

   // decoder status
   int radioDecoderStatus = Idle;

   // current decoder configuration
   json currentConfig;

   Impl() : AbstractTask("worker.RadioDecoder", "radio.decoder"), decoder(new NfcDecoder())
   {
      // access to signal subject stream
      radioSignalStream = rt::Subject<hw::SignalBuffer>::name("radio.signal.raw");

      // create frame stream subject
      decoderFrameStream = rt::Subject<RawFrame>::name("radio.decoder.frame");

      // subscribe to radio signal events
      radioSignalSubscription = radioSignalStream->subscribe([this](const hw::SignalBuffer &buffer) {
         if (radioDecoderStatus == Streaming)
            radioSignalQueue.add(buffer);
      });
   }

   void start() override
   {
      updateDecoderStatus(Idle);
   }

   void stop() override
   {
      updateDecoderStatus(Idle);
   }

   bool loop() override
   {
      /*
       * process pending commands
       */
      if (auto command = commandQueue.get())
      {
         log->debug("command [{}]", {command->code});

         if (command->code == Start)
         {
            startDecoder(command.value());
         }
         else if (command->code == Stop)
         {
            stopDecoder(command.value());
         }
         else if (command->code == Query)
         {
            queryDecoder(command.value());
         }
         else if (command->code == Configure)
         {
            configDecoder(command.value());
         }
         else if (command->code == Clear)
         {
            clearDecoder(command.value());
         }
      }

      /*
       * process signal decode
       */
      if (radioDecoderEnabled && radioDecoderStatus == Streaming)
      {
         signalDecode();

         if (std::chrono::steady_clock::now() - lastStatus > std::chrono::milliseconds(1000))
         {
            if (taskThroughput.average() > 0)
               log->info("average throughput {.2} Msps, {} pending buffers", {taskThroughput.average() / 1E6, radioSignalQueue.size()});

            lastStatus = std::chrono::steady_clock::now();
         }
      }
      else
      {
         wait(100);
      }

      return true;
   }

   void startDecoder(const rt::Event &command)
   {
      if (!radioDecoderEnabled)
      {
         log->warn("decoder is disabled");
         command.reject(TaskDisabled);
         return;
      }

      log->info("start frame decoding with {} pending buffers!", {radioSignalQueue.size()});

      taskThroughput.begin();

      radioSignalQueue.clear();

      decoder->initialize();

      command.resolve();

      updateDecoderStatus(Streaming);
   }

   void stopDecoder(const rt::Event &command)
   {
      if (!radioDecoderEnabled)
      {
         log->warn("decoder is disabled");
         command.reject(TaskDisabled);
         return;
      }

      log->info("stop frame decoding with {} pending buffers!", {radioSignalQueue.size()});

      radioSignalQueue.clear();

      for (const auto &frame: decoder->nextFrames({}))
      {
         decoderFrameStream->next(frame);
      }

      command.resolve();

      updateDecoderStatus(Idle);
   }

   void queryDecoder(const rt::Event &command)
   {
      log->debug("query status");

      command.resolve();

      updateDecoderStatus(radioDecoderStatus, true);
   }

   void configDecoder(const rt::Event &command)
   {
      if (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log->info("change config: {}", {config.dump()});

         // update current configuration
         currentConfig.merge_patch(config);

         if (config.contains("enabled"))
            radioDecoderEnabled = config["enabled"];

         // stream reference time
         if (config.contains("streamTime"))
            decoder->setStreamTime(config["streamTime"]);

         // Debug parameters
         if (config.contains("debugEnabled"))
            decoder->setEnableDebug(config["debugEnabled"]);

         // global power level threshold
         if (config.contains("powerLevelThreshold"))
            decoder->setPowerLevelThreshold(config["powerLevelThreshold"]);

         // sample rate must be last value set
         if (config.contains("sampleRate"))
            decoder->setSampleRate(config["sampleRate"]);

         if (config.contains("protocol"))
         {
            auto protocol = config["protocol"];

            // NFC-A parameters
            if (protocol.contains("nfca"))
            {
               auto nfca = protocol["nfca"];

               float min = NAN;
               float max = NAN;

               if (nfca.contains("enabled"))
                  decoder->setEnableNfcA(nfca["enabled"]);

               if (nfca.contains("correlationThreshold"))
                  decoder->setCorrelationThresholdNfcA(nfca["correlationThreshold"]);

               if (nfca.contains("minimumModulationDeep"))
                  min = nfca["minimumModulationDeep"];

               if (nfca.contains("maximumModulationDeep"))
                  max = nfca["maximumModulationDeep"];

               if (!std::isnan(min) && !std::isnan(max))
                  decoder->setModulationThresholdNfcA(min, max);
            }

            // NFC-B parameters
            if (protocol.contains("nfcb"))
            {
               auto nfcb = protocol["nfcb"];

               float min = NAN;
               float max = NAN;

               if (nfcb.contains("enabled"))
                  decoder->setEnableNfcB(nfcb["enabled"]);

               if (nfcb.contains("correlationThreshold"))
                  decoder->setCorrelationThresholdNfcB(nfcb["correlationThreshold"]);

               if (nfcb.contains("minimumModulationDeep"))
                  min = nfcb["minimumModulationDeep"];

               if (nfcb.contains("maximumModulationDeep"))
                  max = nfcb["maximumModulationDeep"];

               if (!std::isnan(min) && !std::isnan(max))
                  decoder->setModulationThresholdNfcB(min, max);
            }

            // NFC-F parameters
            if (protocol.contains("nfcf"))
            {
               auto nfcf = protocol["nfcf"];

               float min = NAN;
               float max = NAN;

               if (nfcf.contains("enabled"))
                  decoder->setEnableNfcF(nfcf["enabled"]);

               if (nfcf.contains("correlationThreshold"))
                  decoder->setCorrelationThresholdNfcF(nfcf["correlationThreshold"]);

               if (nfcf.contains("minimumModulationDeep"))
                  min = nfcf["minimumModulationDeep"];

               if (nfcf.contains("maximumModulationDeep"))
                  max = nfcf["maximumModulationDeep"];

               if (!std::isnan(min) && !std::isnan(max))
                  decoder->setModulationThresholdNfcF(min, max);
            }

            // NFC-V parameters
            if (protocol.contains("nfcv"))
            {
               auto nfcv = protocol["nfcv"];

               float min = NAN;
               float max = NAN;

               if (nfcv.contains("enabled"))
                  decoder->setEnableNfcV(nfcv["enabled"]);

               if (nfcv.contains("correlationThreshold"))
                  decoder->setCorrelationThresholdNfcV(nfcv["correlationThreshold"]);

               if (nfcv.contains("minimumModulationDeep"))
                  min = nfcv["minimumModulationDeep"];

               if (nfcv.contains("maximumModulationDeep"))
                  max = nfcv["maximumModulationDeep"];

               if (!std::isnan(min) && !std::isnan(max))
                  decoder->setModulationThresholdNfcV(min, max);
            }
         }

         if (!radioDecoderEnabled && radioDecoderStatus == Streaming)
         {
            radioSignalQueue.clear();

            for (const auto &frame: decoder->nextFrames({}))
            {
               decoderFrameStream->next(frame);
            }

            radioDecoderStatus = Idle;
         }

         command.resolve();

         updateDecoderStatus(radioDecoderStatus, true);
      }
      else
      {
         log->warn("invalid config data");

         command.reject(InvalidConfig);
      }
   }

   void clearDecoder(const rt::Event &command)
   {
      log->info("clear decoder queue with {} pending buffers", {radioSignalQueue.size()});

      radioSignalQueue.clear();

      command.resolve();
   }

   void signalDecode()
   {
      if (const auto buffer = radioSignalQueue.get())
      {
         int frames = 0;

         log->trace("decode new buffer {} offset {} with {} samples", {buffer->id(), buffer->offset(), buffer->elements()});

         for (const auto &frame: decoder->nextFrames(buffer.value()))
         {
            decoderFrameStream->next(frame);
            frames++;
         }

         taskThroughput.update(buffer->elements());

         if (!buffer->isValid())
         {
            log->info("decoder EOF buffer received, finish!");

            decoder->cleanup();

            decoderFrameStream->next({});

            updateDecoderStatus(Idle);
         }
      }
   }

   void updateDecoderStatus(int value, bool full = false)
   {
      radioDecoderStatus = value;

      json data({
         {"status", radioDecoderEnabled ? (radioDecoderStatus == Streaming ? "decoding" : "idle") : "disabled"},
         {"queueSize", radioSignalQueue.size()},
         {"sampleRate", decoder->sampleRate()},
         {"streamTime", decoder->streamTime()},
         {"debugEnabled", decoder->isDebugEnabled()},
         {"powerLevelThreshold", decoder->powerLevelThreshold()},
         {"sampleThroughput", taskThroughput.average()}
      });

      if (full)
      {
         json protocol({
            {
               "nfca", {
                  {"enabled", decoder->isNfcAEnabled()},
                  {"correlationThreshold", decoder->correlationThresholdNfcA()},
                  {"minimumModulationDeep", decoder->modulationThresholdNfcAMin()},
                  {"maximumModulationDeep", decoder->modulationThresholdNfcAMax()}
               }
            },
            {
               "nfcb", {
                  {"enabled", decoder->isNfcBEnabled()},
                  {"correlationThreshold", decoder->correlationThresholdNfcB()},
                  {"minimumModulationDeep", decoder->modulationThresholdNfcBMin()},
                  {"maximumModulationDeep", decoder->modulationThresholdNfcBMax()}
               }
            },
            {
               "nfcf", {
                  {"enabled", decoder->isNfcFEnabled()},
                  {"correlationThreshold", decoder->correlationThresholdNfcF()},
                  {"minimumModulationDeep", decoder->modulationThresholdNfcFMin()},
                  {"maximumModulationDeep", decoder->modulationThresholdNfcFMax()}
               }
            },
            {
               "nfcv", {
                  {"enabled", decoder->isNfcVEnabled()},
                  {"correlationThreshold", decoder->correlationThresholdNfcV()},
                  {"minimumModulationDeep", decoder->modulationThresholdNfcVMin()},
                  {"maximumModulationDeep", decoder->modulationThresholdNfcVMax()}
               }
            }
         });

         data["protocol"] = protocol;
      }

      updateStatus(radioDecoderStatus, data);
   }
};

RadioDecoderTask::RadioDecoderTask() : Worker("FrameDecoderTask")
{
}

rt::Worker *RadioDecoderTask::construct()
{
   return new Impl;
}
}

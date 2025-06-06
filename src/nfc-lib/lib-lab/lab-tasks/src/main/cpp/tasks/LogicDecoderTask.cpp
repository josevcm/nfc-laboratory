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

#include <rt/BlockingQueue.h>
#include <rt/Throughput.h>

#include <lab/iso/IsoDecoder.h>

#include <lab/tasks/LogicDecoderTask.h>

#include "AbstractTask.h"

namespace lab {

struct LogicDecoderTask::Impl : LogicDecoderTask, AbstractTask
{
   // signal buffer frame stream subject
   rt::Subject<hw::SignalBuffer> *logicSignalStream = nullptr;

   // signal stream subscription
   rt::Subject<hw::SignalBuffer>::Subscription logicSignalSubscription;

   // frame stream subject
   rt::Subject<RawFrame> *decoderFrameStream = nullptr;

   // radio signal stream queue buffer
   rt::BlockingQueue<hw::SignalBuffer> logicSignalQueue;

   // byte throughput meter
   rt::Throughput taskThroughput;

   // decoder
   std::shared_ptr<IsoDecoder> decoder;

   // last Throughput statistics
   std::chrono::time_point<std::chrono::steady_clock> lastStatus;

   // control flags
   bool logicDecoderEnabled = false;

   // decoder status
   int logicDecoderStatus = Idle;

   // current decoder configuration
   json currentConfig;

   Impl() : AbstractTask("worker.LogicDecoder", "logic.decoder"), decoder(new IsoDecoder())
   {
      // access to signal subject stream
      logicSignalStream = rt::Subject<hw::SignalBuffer>::name("logic.signal.raw");

      // create frame stream subject
      decoderFrameStream = rt::Subject<RawFrame>::name("logic.decoder.frame");

      // subscribe to radio signal events
      logicSignalSubscription = logicSignalStream->subscribe([this](const hw::SignalBuffer &buffer) {
         if (logicDecoderStatus == Streaming)
            logicSignalQueue.add(buffer);
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
      if (logicDecoderEnabled && logicDecoderStatus == Streaming)
      {
         signalDecode();

         if (std::chrono::steady_clock::now() - lastStatus > std::chrono::milliseconds(1000))
         {
            if (taskThroughput.average() > 0)
               log->info("average throughput {.2} Msps, {} pending buffers", {taskThroughput.average() / 1E6, logicSignalQueue.size()});

            lastStatus = std::chrono::steady_clock::now();
         }
      }
      else
      {
         wait(50);
      }

      return true;
   }

   void startDecoder(const rt::Event &command)
   {
      if (!logicDecoderEnabled)
      {
         log->warn("decoder is disabled");
         command.reject(TaskDisabled);
         return;
      }

      log->info("start frame decoding with {} pending buffers!", {logicSignalQueue.size()});

      taskThroughput.begin();

      logicSignalQueue.clear();

      decoder->initialize();

      command.resolve();

      updateDecoderStatus(Streaming);
   }

   void stopDecoder(const rt::Event &command)
   {
      if (!logicDecoderEnabled)
      {
         log->warn("decoder is disabled");
         command.reject(TaskDisabled);
         return;
      }

      log->info("stop frame decoding with {} pending buffers!", {logicSignalQueue.size()});

      logicSignalQueue.clear();

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

      updateDecoderStatus(logicDecoderStatus, true);
   }

   void configDecoder(rt::Event &command)
   {
      if (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log->info("change config: {}", {config.dump()});

         // update current configuration
         currentConfig.merge_patch(config);

         if (config.contains("enabled"))
            logicDecoderEnabled = config["enabled"];

         if (config.contains("protocol"))
         {
            auto protocol = config["protocol"];
         }

         // stream reference time
         if (config.contains("streamTime"))
            decoder->setStreamTime(config["streamTime"]);

         // Debug parameters
         if (config.contains("debugEnabled"))
            decoder->setEnableDebug(config["debugEnabled"]);

         // sample rate must be last value set
         if (config.contains("sampleRate"))
            decoder->setSampleRate(config["sampleRate"]);

         if (config.contains("protocol"))
         {
            auto protocol = config["protocol"];

            // NFC-A parameters
            if (protocol.contains("iso7816"))
            {
               auto iso7816 = protocol["iso7816"];

               if (iso7816.contains("enabled"))
                  decoder->setEnableISO7816(iso7816["enabled"]);

            }
         }

         if (!logicDecoderEnabled && logicDecoderStatus == Streaming)
         {
            logicSignalQueue.clear();

            for (const auto &frame: decoder->nextFrames({}))
            {
               decoderFrameStream->next(frame);
            }

            logicDecoderStatus = Idle;
         }

         command.resolve();

         updateDecoderStatus(logicDecoderStatus, true);
      }
      else
      {
         log->warn("invalid config data");

         command.reject(InvalidConfig);
      }
   }

   void clearDecoder(const rt::Event &command)
   {
      log->info("clear decoder queue with {} pending buffers", {logicSignalQueue.size()});

      logicSignalQueue.clear();

      command.resolve();
   }

   void signalDecode()
   {
      if (const auto buffer = logicSignalQueue.get())
      {
         int frames = 0;

         log->trace("decode new buffer {} offset {} with {} samples", {buffer->id(), buffer->offset(), buffer->elements()});

         taskThroughput.update(buffer->elements());

         for (const auto &frame: decoder->nextFrames(buffer.value()))
         {
            decoderFrameStream->next(frame);
            frames++;
         }

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
      logicDecoderStatus = value;

      json data({
         {"status", logicDecoderEnabled ? (logicDecoderStatus == Streaming ? "decoding" : "idle") : "disabled"},
         {"queueSize", logicSignalQueue.size()},
         {"sampleRate", decoder->sampleRate()},
         {"streamTime", decoder->streamTime()},
         {"debugEnabled", decoder->isDebugEnabled()},
         {"sampleThroughput", taskThroughput.average()}
      });

      if (full)
      {
         json protocol({
            {
               "iso7816", {
                  {"enabled", decoder->isISO7816Enabled()},
               }
            }
         });

         data["protocol"] = protocol;
      }

      updateStatus(logicDecoderStatus, data);
   }
};

LogicDecoderTask::LogicDecoderTask() : Worker("LogicDecoderTask")
{
}

rt::Worker *LogicDecoderTask::construct()
{
   return new Impl;
}

}

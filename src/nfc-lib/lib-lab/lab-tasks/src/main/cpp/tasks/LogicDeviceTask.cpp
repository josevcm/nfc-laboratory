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

#include <hw/DeviceFactory.h>

#include <hw/logic/LogicDevice.h>
#include <hw/logic/DSLogicDevice.h>

#include <lab/tasks/LogicDeviceTask.h>

#include "AbstractTask.h"

namespace lab {

struct LogicDeviceTask::Impl : LogicDeviceTask, AbstractTask
{
   // logic device
   std::shared_ptr<hw::LogicDevice> device;

   // signal stream subject for raw data
   rt::Subject<hw::SignalBuffer> *signalStream = nullptr;

   // signal stream queue buffer
   rt::BlockingQueue<hw::SignalBuffer> signalQueue;

   // throughput meter
   rt::Throughput taskThroughput;

   // last detection attempt
   std::chrono::time_point<std::chrono::steady_clock> lastSearch;

   // current task status
   bool logicReceiverEnabled = false;

   // current receiver status
   int logicReceiverStatus = Idle;

   // current device configuration
   json currentConfig;

   Impl() : AbstractTask("worker.LogicDevice", "logic.receiver")
   {
      signalStream = rt::Subject<hw::SignalBuffer>::name("logic.signal.raw");
   }

   void start() override
   {
      log->info("registering logic devices");

      hw::DeviceFactory::registerDevice("logic.dslogic", []() -> std::vector<std::string> { return hw::DSLogicDevice::enumerate(); }, [](const std::string &name) -> hw::DSLogicDevice *{ return new hw::DSLogicDevice(name); });
   }

   void stop() override
   {
      if (device)
      {
         log->info("shutdown device {}", {std::get<std::string>(device->get(hw::LogicDevice::PARAM_DEVICE_NAME))});
         device.reset();
      }

      updateDeviceStatus(Idle);
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
            startDevice(command.value());
         }
         else if (command->code == Stop)
         {
            stopDevice(command.value());
         }
         else if (command->code == Query)
         {
            queryDevice(command.value());
         }
         else if (command->code == Configure)
         {
            configDevice(command.value());
         }
         else if (command->code == Clear)
         {
            clearDevice(command.value());
         }
      }

      /*
       * process device refresh
       */
      if (logicReceiverEnabled)
      {
         if (std::chrono::steady_clock::now() - lastSearch > std::chrono::milliseconds(1000))
         {
            if (!device || !device->isStreaming())
            {
               refresh();
            }
            else if (taskThroughput.average() > 0)
            {
               log->info("average throughput {.2} Msps", {taskThroughput.average() / 1E6});

               // reset throughput meter
               taskThroughput.begin();
            }

            // store last search time
            lastSearch = std::chrono::steady_clock::now();
         }

         processQueue(50);
      }
      else
      {
         wait(100);
      }

      return true;
   }

   void refresh()
   {
      if (!device)
      {
         // open first available receiver
         for (const auto &name: hw::DeviceFactory::enumerate("logic"))
         {
            log->info("detected device {}", {name});

            // create device instance
            device.reset(hw::DeviceFactory::newInstance<hw::LogicDevice>(name));

            if (!device)
               continue;

            // setup firmware location before open
            if (currentConfig.contains("firmwarePath"))
               device->set(hw::LogicDevice::PARAM_FIRMWARE_PATH, static_cast<std::string>(currentConfig["firmwarePath"]));

            // try to open...
            if (device->open(hw::LogicDevice::Mode::Read))
            {
               log->info("device {} connected!", {name});

               // set device parameters from current configuration
               setup(currentConfig);

               // update receiver status
               updateDeviceStatus(Idle, true);

               return;
            }

            device.reset();

            log->warn("device {} open failed", {name});
         }
      }
      else if (!device->isReady())
      {
         log->warn("device {} disconnected", {std::get<std::string>(device->get(hw::LogicDevice::PARAM_DEVICE_NAME))});

         // send null buffer for EOF
         signalStream->next({});

         // close device
         device.reset();

         // update receiver status
         updateDeviceStatus(Absent);

         return;
      }

      // update receiver status
      updateDeviceStatus(logicReceiverStatus);
   }

   void setup(const json &config)
   {
      log->info("applying configuration: {}", {config.dump()});

      std::vector<int> channels;

      if (config.contains("channels") && config["channels"].is_array())
      {
         for (auto &elem: config["channels"])
         {
            channels.push_back(elem.is_string() ? std::stoi(elem.get<std::string>()) : elem.get<int>());
         }
      }

      // set default channel if not defined
      if (channels.empty())
         channels = {0};

      // default parameters for DSLogic
      device->set(hw::LogicDevice::PARAM_OPERATION_MODE, hw::DSLogicDevice::OP_STREAM);
      device->set(hw::LogicDevice::PARAM_LIMIT_SAMPLES, static_cast<unsigned long long>(-1));

      // setup sample rate
      if (config.contains("sampleRate"))
         device->set(hw::LogicDevice::PARAM_SAMPLE_RATE, static_cast<unsigned int>(config["sampleRate"]));

      // setup threshold level
      if (config.contains("vThreshold"))
         device->set(hw::LogicDevice::PARAM_VOLTAGE_THRESHOLD, static_cast<float>(config["vThreshold"]));

      // setup channels
      for (int c = 0; c < std::get<unsigned int>(device->get(hw::LogicDevice::PARAM_CHANNEL_TOTAL)); c++)
      {
         device->set(hw::LogicDevice::PARAM_PROBE_ENABLE, std::find(channels.begin(), channels.end(), c) != channels.end(), c); // DATA
      }
   }

   void startDevice(const rt::Event &command)
   {
      if (!logicReceiverEnabled)
      {
         log->warn("device is disabled");
         command.reject(TaskDisabled);
         return;
      }

      if (device)
      {
         log->info("start streaming for device {}", {std::get<std::string>(device->get(hw::LogicDevice::PARAM_DEVICE_NAME))});

         // reset throughput meter
         taskThroughput.begin();

         // start receiving
         device->start([this](hw::SignalBuffer &buffer) {
            signalQueue.add(buffer);
            return true;
         });

         // resolve command
         command.resolve();

         updateDeviceStatus(Streaming);
      }
   }

   void stopDevice(const rt::Event &command)
   {
      if (!logicReceiverEnabled)
      {
         log->warn("device is disabled");
         command.reject(TaskDisabled);
         return;
      }

      if (device)
      {
         log->info("stop streaming for device {}", {std::get<std::string>(device->get(hw::LogicDevice::PARAM_DEVICE_NAME))});

         // stop underline receiver
         device->stop();

         // resolve command
         command.resolve();

         // set receiver in flush buffers status
         updateDeviceStatus(Flush);
      }
   }

   void queryDevice(const rt::Event &command)
   {
      log->debug("query status");

      command.resolve();

      updateDeviceStatus(logicReceiverStatus, true);
   }

   void configDevice(const rt::Event &command)
   {
      if (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log->info("change config: {}", {config.dump()});

         // update current configuration
         currentConfig.merge_patch(config);

         // check if receiver is enabled
         if (config.contains("enabled"))
            logicReceiverEnabled = config["enabled"];

         if (device)
         {
            // update device configuration
            setup(currentConfig);

            // stop streaming if device is streaming when disabled is requested
            if (!logicReceiverEnabled && device->isStreaming())
            {
               log->info("stop streaming");

               // stop underline receiver
               device->stop();

               // flush pending buffers before disable
               logicReceiverStatus = Flush;
            }
         }

         command.resolve();

         updateDeviceStatus(logicReceiverStatus, config.contains("enabled") && logicReceiverEnabled);
      }
      else
      {
         log->warn("invalid config data");

         command.reject(InvalidConfig);
      }
   }

   void clearDevice(const rt::Event &command)
   {
      log->info("clear signal queue with {} pending buffers", {signalQueue.size()});

      signalQueue.clear();

      command.resolve();
   }

   void updateDeviceStatus(int status, bool full = false)
   {
      json data;

      logicReceiverStatus = status;

      if (device)
      {
         // device name and status
         data["name"] = std::get<std::string>(device->get(hw::LogicDevice::PARAM_DEVICE_NAME));
         data["vendor"] = std::get<std::string>(device->get(hw::LogicDevice::PARAM_DEVICE_VENDOR));
         data["model"] = std::get<std::string>(device->get(hw::LogicDevice::PARAM_DEVICE_MODEL));
         data["version"] = std::get<std::string>(device->get(hw::LogicDevice::PARAM_DEVICE_VERSION));
         data["serial"] = std::get<std::string>(device->get(hw::LogicDevice::PARAM_DEVICE_SERIAL));
         data["status"] = logicReceiverEnabled ? (device->isStreaming() ? "streaming" : "idle") : "disabled";

         // device parameters
         data["sampleRate"] = std::get<unsigned int>(device->get(hw::LogicDevice::PARAM_SAMPLE_RATE));
         data["streamTime"] = std::get<unsigned int>(device->get(hw::LogicDevice::PARAM_STREAM_TIME));

         // device statistics
         data["samplesRead"] = std::get<unsigned long long>(device->get(hw::LogicDevice::PARAM_SAMPLES_READ));
         data["samplesLost"] = std::get<unsigned long long>(device->get(hw::LogicDevice::PARAM_SAMPLES_LOST));
      }
      else
      {
         data["status"] = logicReceiverEnabled ? "absent" : "disabled";
      }

      updateStatus(status, data);
   }

   void processQueue(int timeout)
   {
      if (const auto entry = signalQueue.get(timeout))
      {
         const hw::SignalBuffer &buffer = entry.value();

         // send value buffer
         signalStream->next(buffer);

         // update receiver throughput
         taskThroughput.update(buffer.elements());
      }
      else if (logicReceiverStatus == Flush)
      {
         // send null buffer for EOF
         signalStream->next({});

         // flush finished
         updateDeviceStatus(Idle);
      }
   }
};

LogicDeviceTask::LogicDeviceTask() : Worker("LogicDeviceTask")
{
}

rt::Worker *LogicDeviceTask::construct()
{
   return new Impl;
}

}

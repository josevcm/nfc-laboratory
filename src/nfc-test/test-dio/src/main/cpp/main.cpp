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

#include <iostream>
#include <fstream>
#include <iomanip>

#include <rt/Logger.h>

#include <hw/RecordDevice.h>

#include <hw/logic/DSLogicDevice.h>

std::string fileName(const std::string &type)
{
   std::ostringstream oss;
   std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
   const std::tm *tm = std::localtime(&time);

   oss << type << "-" << std::put_time(tm, "%Y%m%dT%H%M%S") << ".wav";

   return oss.str();
}

int main(int argc, char *argv[])
{
   rt::Logger::init(std::cout, false);

   rt::Logger *log = rt::Logger::getLogger("app.main", rt::Logger::INFO_LEVEL);

   log->info("***********************************************************************");
   log->info("NFC laboratory, 2024 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   log->info("***********************************************************************");

   for (std::string name: hw::logic::DSLogicDevice::enumerate())
   {
      log->info("found device: {}", {name});

      hw::logic::DSLogicDevice device {name};

      if (device.open(hw::logic::LogicDevice::Read))
      {
         log->info("start receiving");

         // device.set(hw::logic::LogicDevice::PARAM_OPERATION_MODE, LogicDevice::OP_INTEST);
         device.set(hw::logic::LogicDevice::PARAM_OPERATION_MODE, hw::logic::LogicDevice::OP_STREAM);
         device.set(hw::logic::LogicDevice::PARAM_LIMIT_SAMPLES, static_cast<unsigned long long>(-1));
         //         device.set(hw::logic::LogicDevice::PARAM_RLE_COMPRESS, false);
         //         device.set(hw::logic::LogicDevice::PARAM_FILTER_MODE, FILTER_NONE);
         device.set(hw::logic::LogicDevice::PARAM_CHANNEL_MODE, hw::logic::DSLogicDevice::DSL_STREAM50x6);
         device.set(hw::logic::LogicDevice::PARAM_SAMPLE_RATE, static_cast<unsigned int>(25000000));
         device.set(hw::logic::LogicDevice::PARAM_VOLTAGE_THRESHOLD, static_cast<float>(1.0));

         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, true, 0);
         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 1);
         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 2);
         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 3);
         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 4);
         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 5);
         //         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 6);
         //         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 7);
         //         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 8);
         //         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 9);
         //         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 10);
         //         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 11);
         //         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 12);
         //         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 13);
         //         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 14);
         //         device.set(hw::logic::LogicDevice::PARAM_PROBE_ENABLE, false, 15);

         //         device.set(LogicDevice::PARAM_PROBE_COUPLING, GND_COUPLING, 0);

         std::string fileName = ::fileName("logic");
         unsigned int sampleSize = 8;
         unsigned int sampleRate = std::get<unsigned int>(device.get(hw::logic::DSLogicDevice::PARAM_SAMPLE_RATE));
         unsigned int validChannels = std::get<unsigned int>(device.get(hw::logic::DSLogicDevice::PARAM_CHANNEL_VALID));

         hw::RecordDevice storage(fileName);

         log->info("creating storage file {}, sampleRate {} sampleSize {} channels {}", {fileName, sampleRate, sampleSize, validChannels});

         storage.set(hw::SignalDevice::PARAM_SAMPLE_RATE, sampleRate);
         storage.set(hw::SignalDevice::PARAM_SAMPLE_SIZE, sampleSize);
         storage.set(hw::SignalDevice::PARAM_CHANNEL_COUNT, validChannels);

         int count = 0;

         if (storage.open(hw::RecordDevice::Mode::Write))
         {
            log->info("successfully opened storage file: {}", {fileName});

            device.start([&](const hw::SignalBuffer &buffer) {

               log->info("{} [{} - {.3}]: {} samples", {buffer.offset(), count++, buffer.offset() / (sampleRate / 1000.0), buffer.elements()});

               storage.write(buffer);

               return true;
            });
         }

         std::this_thread::sleep_for(std::chrono::seconds(1));

         log->info("stop receiving");

         device.close();

         storage.close();
      }
   }

   rt::Logger::flush();

   return 0;
}

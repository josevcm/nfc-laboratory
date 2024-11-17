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

#include <hw/usb/Usb.h>

#include <hw/logic/DSLogicDevice.h>

using namespace rt;
using namespace hw;

int main(int argc, char *argv[])
{
   Logger::init(std::cout, false);

   rt::Logger *log = rt::Logger::getLogger("app.main", rt::Logger::INFO_LEVEL);

   log->info("***********************************************************************");
   log->info("NFC laboratory, 2024 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   log->info("***********************************************************************");

   for (std::string name: DSLogicDevice::enumerate())
   {
      log->info("found device: {}", {name});

      DSLogicDevice device {name};

      if (device.open(LogicDevice::Read))
      {
         log->info("start receiving");

         device.set(LogicDevice::PARAM_OPERATION_MODE, LogicDevice::OP_INTEST);
//         device.set(LogicDevice::PARAM_OPERATION_MODE, LogicDevice::OP_STREAM);
//         device.set(LogicDevice::PARAM_RLE_COMPRESS, false);
//         device.set(LogicDevice::PARAM_FILTER_MODE, FILTER_NONE);
         device.set(LogicDevice::PARAM_SAMPLE_RATE, 1000000);
         device.set(LogicDevice::PARAM_CHANNEL_MODE, DSLogicDevice::DSL_STREAM50x6);
         device.set(LogicDevice::PARAM_VOLTAGE_THRESHOLD, 0.5);

         device.set(LogicDevice::PARAM_PROBE_ENABLE, true, 0);
         device.set(LogicDevice::PARAM_PROBE_ENABLE, true, 1);
         device.set(LogicDevice::PARAM_PROBE_ENABLE, true, 2);
         device.set(LogicDevice::PARAM_PROBE_ENABLE, true, 3);
//         device.set(LogicDevice::PARAM_PROBE_ENABLE, false, 4);
//         device.set(LogicDevice::PARAM_PROBE_ENABLE, false, 5);
//         device.set(LogicDevice::PARAM_PROBE_ENABLE, false, 6);
//         device.set(LogicDevice::PARAM_PROBE_ENABLE, false, 7);
//         device.set(LogicDevice::PARAM_PROBE_ENABLE, false, 8);
//         device.set(LogicDevice::PARAM_PROBE_ENABLE, false, 9);
//         device.set(LogicDevice::PARAM_PROBE_ENABLE, false, 10);
//         device.set(LogicDevice::PARAM_PROBE_ENABLE, false, 11);
//         device.set(LogicDevice::PARAM_PROBE_ENABLE, false, 12);
//         device.set(LogicDevice::PARAM_PROBE_ENABLE, false, 13);
//         device.set(LogicDevice::PARAM_PROBE_ENABLE, false, 14);
//         device.set(LogicDevice::PARAM_PROBE_ENABLE, false, 15);

//         device.set(LogicDevice::PARAM_PROBE_COUPLING, GND_COUPLING, 0);

         device.start([=](SignalBuffer &buffer) {
            log->info("received {} samples", {buffer.elements()});
//            log->info("\n{}", {buffer.asByteBuffer()});
            return true;
         });

         std::this_thread::sleep_for(std::chrono::seconds(1));

         log->info("stop receiving");

         device.close();
      }

//      std::this_thread::sleep_for(std::chrono::seconds(10));
   }

   return 0;
}

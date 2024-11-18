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

#include <rt/Logger.h>

#include <hw/DeviceFactory.h>

namespace hw {

rt::Logger *DeviceFactory::log = rt::Logger::getLogger("hw.DeviceFactory");

std::mutex DeviceFactory::mutex;

std::map<std::string, DeviceFactory::Enumerator> DeviceFactory::enumerators;
std::map<std::string, DeviceFactory::Constructor> DeviceFactory::constructors;

void DeviceFactory::registerDevice(const std::string &type, Enumerator enumerator, Constructor constructor)
{
   std::lock_guard lock(mutex);

   log->info("registering device type {}", {type.c_str()});

   enumerators[type] = std::move(enumerator);
   constructors[type] = std::move(constructor);
}

std::vector<std::string> DeviceFactory::enumerate(const std::string &filter)
{
   std::lock_guard lock(mutex);

   std::vector<std::string> devices;

   for (const auto &entry: enumerators)
   {
      // execute enumerator and collect available devices
      for (const auto &name: entry.second())
      {
         if (filter.empty() || name.find(filter) != std::string::npos)
            devices.push_back(name);
      }
   }

   return devices;
}

}
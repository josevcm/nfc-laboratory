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

#ifndef DEV_DEVICEFACTORY_H
#define DEV_DEVICEFACTORY_H

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <mutex>

#include <rt/Logger.h>

#include <hw/Device.h>

namespace hw {

class DeviceFactory
{
   private:

      DeviceFactory() = default;

   public:

      typedef std::function<std::vector<std::string> ()> Enumerator;

      typedef std::function<void *(const std::string &name)> Constructor;

      static void registerDevice(const std::string &type, Enumerator enumerator, Constructor constructor);

      static std::vector<std::string> enumerate(const std::string &filter);

      template<class T>
      static T *newInstance(const std::string &name)
      {
         std::lock_guard lock(mutex);

         std::string type = name.find("://") != std::string::npos ? name.substr(0, name.find("://")) : "";

         auto it = constructors.find(type);

         if (it != constructors.end())
            return static_cast<T *>(it->second(name));

         return nullptr;
      }

   private:

      static rt::Logger *log;

      static std::mutex mutex;

      static std::map<std::string, Enumerator> enumerators;

      static std::map<std::string, Constructor> constructors;
};

}
#endif

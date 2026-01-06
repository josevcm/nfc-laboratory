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

#ifndef DEV_DEVICE_H
#define DEV_DEVICE_H

#include <string>

#include <rt/Variant.h>

namespace hw {

template <typename BufferType>
class Device
{
   public:

      enum Mode
      {
         Read = 1, Write = 2, Duplex = 3
      };

      enum Params
      {
         // device parameters
         PARAM_DEVICE_TYPE = 0,
         PARAM_DEVICE_INDEX = 1,
         PARAM_DEVICE_NAME = 2,
         PARAM_DEVICE_SERIAL = 3,
         PARAM_DEVICE_VENDOR = 4,
         PARAM_DEVICE_MODEL = 5,
         PARAM_DEVICE_VERSION = 6,
         PARAM_DEVICE_LAST = 99,
      };

   public:

      virtual ~Device() = default;

      virtual bool open(Mode mode) = 0;

      virtual void close() = 0;

      template <typename V>
      V get(int id)
      {
         return std::get<V>(this->get(id, -1));
      }

      template <typename V>
      V get(int id, int channel)
      {
         return std::get<V>(this->get(id, channel));
      }

      virtual bool set(int id, const rt::Variant &value)
      {
         return this->set(id, value, -1);
      }

      virtual rt::Variant get(int id, int channel) const = 0;

      virtual bool set(int id, const rt::Variant &value, int channel) = 0;

      virtual bool isOpen() const = 0;

      virtual bool isEof() const = 0;

      virtual bool isReady() const = 0;

      virtual long read(BufferType &buffer) = 0;

      virtual long write(const BufferType &buffer) = 0;
};

}
#endif

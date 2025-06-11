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

#ifndef DEV_RECORDDEVICE_H
#define DEV_RECORDDEVICE_H

#include <hw/SignalDevice.h>

namespace hw {

class RecordDevice : public SignalDevice
{
   struct Impl;

   public:

      explicit RecordDevice(const std::string &name);

      bool open(Mode mode) override;

      void close() override;

      rt::Variant get(int id, int channel = -1) const;

      bool set(int id, const rt::Variant &value, int channel = -1);

      bool isOpen() const override;

      bool isEof() const override;

      bool isReady() const override;

      bool isStreaming() const override;

      int read(SignalBuffer &buffer) override;

      int write(const SignalBuffer &buffer) override;

   private:

      std::shared_ptr<Impl> impl;
};

}

#endif

/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2025 Jose Vicente Campos Martinez, <josevcm@gmail.com>

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

#ifndef RADIO_HYDRADEVICE_H
#define RADIO_HYDRADEVICE_H

#include <vector>
#include <string>
#include <memory>

#include <hw/radio/RadioDevice.h>

namespace hw::radio {

class HydraDevice : public RadioDevice
{
   public:

      struct Impl;

      enum GainMode
      {
         Auto = 0, Linearity = 1, Sensitivity = 2
      };

   public:

      explicit HydraDevice(int fd);

      explicit HydraDevice(const std::string &name);

      bool open(Mode mode) override;

      void close() override;

      int start(StreamHandler handler) override;

      int stop() override;

      int pause() override;

      int resume() override;

      using Device::get;

      rt::Variant get(int id, int channel) const override;

      using Device::set;

      bool set(int id, const rt::Variant &value, int channel) override;

      bool isOpen() const override;

      bool isEof() const override;

      bool isReady() const override;

      bool isPaused() const override;

      bool isStreaming() const override;

      long read(SignalBuffer &buffer) override;

      long write(const SignalBuffer &buffer) override;

      static std::vector<std::string> enumerate();

   private:

      std::shared_ptr<Impl> impl;
};

}
#endif

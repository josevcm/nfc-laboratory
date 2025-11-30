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

#ifndef LOGIC_SIPEEDLOGICDEVICE_H
#define LOGIC_SIPEEDLOGICDEVICE_H

#include <vector>
#include <string>
#include <memory>

#include <hw/logic/LogicDevice.h>

namespace hw {

class SipeedLogicDevice : public LogicDevice
{
   struct Impl;

   public:

      enum ChannelMode
      {
      };

      /** Device threshold level. */
      enum ThresholdLevel
      {
         /** 1.8/2.5/3.3 level */
         TH_3V3 = 0,
         /** 5.0 level */
         TH_5V0 = 1,
      };

   public:

      explicit SipeedLogicDevice(const std::string &name);

      ~SipeedLogicDevice() override;

      bool open(Device::Mode mode) override;

      void close() override;

      int start(StreamHandler handler) override;

      int stop() override;

      int pause() override;

      int resume() override;

      rt::Variant get(int id, int channel = -1) const;

      bool set(int id, const rt::Variant &value, int channel = -1);

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

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

#ifndef LOGIC_DSLOGICDEVICE_H
#define LOGIC_DSLOGICDEVICE_H

#include <vector>
#include <string>
#include <memory>

#include <hw/logic/LogicDevice.h>

namespace hw {

class DSLogicDevice : public LogicDevice
{
   struct Impl;

   public:

      enum ChannelMode
      {
         DSL_STREAM20x16 = 0,
         DSL_STREAM25x12 = 1,
         DSL_STREAM50x6 = 2,
         DSL_STREAM100x3 = 3,

         DSL_STREAM20x16_3DN2 = 4,
         DSL_STREAM25x12_3DN2 = 5,
         DSL_STREAM50x6_3DN2 = 6,
         DSL_STREAM100x3_3DN2 = 7,

         DSL_STREAM10x32_32_3DN2 = 8,
         DSL_STREAM20x16_32_3DN2 = 9,
         DSL_STREAM25x12_32_3DN2 = 10,
         DSL_STREAM50x6_32_3DN2 = 11,
         DSL_STREAM100x3_32_3DN2 = 12,

         DSL_STREAM50x32 = 13,
         DSL_STREAM100x30 = 14,
         DSL_STREAM250x12 = 15,
         DSL_STREAM125x16_16 = 16,
         DSL_STREAM250x12_16 = 17,
         DSL_STREAM500x6 = 18,
         DSL_STREAM1000x3 = 19,

         DSL_BUFFER100x16 = 20,
         DSL_BUFFER200x8 = 21,
         DSL_BUFFER400x4 = 22,

         DSL_BUFFER250x32 = 23,
         DSL_BUFFER500x16 = 24,
         DSL_BUFFER1000x8 = 25
      };

      /** Device threshold level. */
      enum ThresholdLevel
      {
         /** 1.8/2.5/3.3 level */
         TH_3V3 = 0,
         /** 5.0 level */
         TH_5V0 = 1,
      };

      enum TestMode
      {
         /** No test mode */
         TEST_NONE,
         /** Internal pattern test mode */
         TEST_INTERNAL,
         /** External pattern test mode */
         TEST_EXTERNAL,
         /** SDRAM loopback test mode */
         TEST_LOOPBACK,
      };

      /** Coupling. */
      enum CouplingMode
      {
         /** DC */
         DC_COUPLING = 0,

         /** Ground */
         GND_COUPLING = 2,
      };

      enum TriggerSource
      {
         TRIGGER_AUTO = 0,
         TRIGGER_CH0 = 1,
         TRIGGER_CH1 = 2,
         TRIGGER_CH0A1 = 3,
         TRIGGER_CH0O1 = 4,
      };

      enum TriggerSlope
      {
         TRIGGER_RISING = 0,
         TRIGGER_FALLING = 1,
      };

      enum TriggerMode
      {
         SIMPLE_TRIGGER = 0,
         ADV_TRIGGER = 1,
         SERIAL_TRIGGER = 2,
      };

      enum FilterMode
      {
         /** None */
         FILTER_NONE = 0,

         /** One clock cycle */
         FILTER_1T = 1,
      };

   public:

      explicit DSLogicDevice(const std::string &name);

      ~DSLogicDevice() override;

      bool open(Device::Mode mode) override;

      void close() override;

      int start(StreamHandler handler) override;

      int stop() override;

      rt::Variant get(int id, int channel = -1) const;

      bool set(int id, const rt::Variant &value, int channel = -1);

      bool isOpen() const override;

      bool isEof() const override;

      bool isReady() const override;

      bool isStreaming() const override;

      long read(SignalBuffer &buffer) override;

      long write(const SignalBuffer &buffer) override;

      static std::vector<std::string> enumerate();

   private:

      std::shared_ptr<Impl> impl;
};

}
#endif

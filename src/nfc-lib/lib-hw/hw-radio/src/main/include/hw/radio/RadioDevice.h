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

#ifndef RADIO_RADIODEVICE_H
#define RADIO_RADIODEVICE_H

#include <hw/SignalDevice.h>

namespace hw {

class RadioDevice : public SignalDevice
{
   public:

      enum Params
      {
         // radio parameters
         PARAM_TUNE_FREQUENCY = 1001,
         PARAM_FREQUENCY_OFFSET = 1002,
         PARAM_GAIN_MODE = 1003,
         PARAM_GAIN_VALUE = 1004,
         PARAM_TUNER_AGC = 1005,
         PARAM_MIXER_AGC = 1006,
         PARAM_BIAS_TEE = 1007,
         PARAM_DIRECT_SAMPLING = 1008,
         PARAM_DECIMATION = 1009,
         PARAM_LIMIT_SAMPLES = 1010,

         // operation parameters
         PARAM_TEST_MODE = 1010,

         // capabilities
         PARAM_SUPPORTED_GAIN_MODES = 1101,
         PARAM_SUPPORTED_GAIN_VALUES = 1102
      };

   public:

      typedef std::function<void(SignalBuffer &)> StreamHandler;

   public:

      virtual bool isPaused() const = 0;

      virtual bool isStreaming() const = 0;

      virtual int start(StreamHandler handler) = 0;

      virtual int stop() = 0;

      virtual int pause()
      {
         return -1;
      }

      virtual int resume()
      {
         return -1;
      }
};

}
#endif

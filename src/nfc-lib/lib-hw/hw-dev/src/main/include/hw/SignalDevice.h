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

#ifndef DEV_SIGNALDEVICE_H
#define DEV_SIGNALDEVICE_H

#include <hw/Device.h>
#include <hw/SignalBuffer.h>

namespace hw {

class SignalDevice : public Device<SignalBuffer>
{
   public:

      enum Params
      {
         // sampling parameters
         PARAM_SAMPLE_RATE = 100,
         PARAM_SAMPLE_SIZE = 101,
         PARAM_SAMPLE_TYPE = 102,
         PARAM_SAMPLE_OFFSET = 103,

         // stream parameters
         PARAM_STREAM_TIME = 104,
         PARAM_SAMPLES_READ = 105,
         PARAM_SAMPLES_WRITE = 106,
         PARAM_SAMPLES_LOST = 107,

         // channel parameters
         PARAM_CHANNEL_COUNT = 108,
         PARAM_CHANNEL_KEYS = 109,

         // capabilities
         PARAM_SUPPORTED_SAMPLE_RATES = 121,
         PARAM_SUPPORTED_SAMPLE_SIZES = 122,
         PARAM_SUPPORTED_SAMPLE_TYPES = 123,
      };
};

}
#endif

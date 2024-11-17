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

#ifndef LOGIC_LOGICDEVICE_H
#define LOGIC_LOGICDEVICE_H

#include <string>

#include <rt/Variant.h>

#include <hw/SignalDevice.h>

namespace hw {

class LogicDevice : public SignalDevice
{
   public:

      enum Params
      {
         PARAM_CLOCK_TYPE = 1001,
         PARAM_CLOCK_EDGE = 1002,
         PARAM_RLE_COMPRESS = 1003,
         PARAM_RLE_SUPPORT = 1004,
         PARAM_LIMIT_SAMPLES = 1005,
         PARAM_PROBE_VDIV = 1006,
         PARAM_PROBE_FACTOR = 1007,
         PARAM_PROBE_COUPLING = 1008,
         PARAM_PROBE_ENABLE = 1009,
         PARAM_TIMEBASE = 1010,
         PARAM_OPERATION_MODE = 1011,
         PARAM_CHANNEL_MODE = 1012,
         PARAM_CHANNEL_TOTAL = 1013,
         PARAM_CHANNEL_VALID = 1014,
         PARAM_VOLTAGE_THRESHOLD = 1015,
         PARAM_FILTER_MODE = 1016,
         PARAM_THRESHOLD_LEVEL = 1017,
         PARAM_STREAM = 1018,
         PARAM_TEST = 1019,

         /** configure trigger */
         PARAM_TRIGGER_SOURCE = 1101,
         PARAM_TRIGGER_CHANNEL = 1102,
         PARAM_TRIGGER_SLOPE = 1103,
         PARAM_TRIGGER_VALUE = 1104,
         PARAM_TRIGGER_HORIZPOS = 1105,
         PARAM_TRIGGER_HOLDOFF = 1106,
         PARAM_TRIGGER_MARGIN = 1107,

         /** Other parameters */
         PARAM_FIRMWARE_PATH = 1201
      };

      enum OperationMode
      {
         /** Buffer mode */
         OP_BUFFER = 0,
         /** Stream mode */
         OP_STREAM = 1,
         /** Internal pattern test mode */
         OP_INTEST = 2,
         /** External pattern test mode */
         OP_EXTEST = 3,
         /** SDRAM loopback test mode */
         OP_LPTEST = 4,
      };

   public:

      typedef std::function<bool(SignalBuffer &)> StreamHandler;

   public:

      virtual int start(StreamHandler handler) = 0;

      virtual int stop() = 0;
};

}

#endif

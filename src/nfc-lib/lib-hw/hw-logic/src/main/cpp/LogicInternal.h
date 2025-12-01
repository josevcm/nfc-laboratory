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

#ifndef LOGIC_LOGICINTERNAL_H
#define LOGIC_LOGICINTERNAL_H

#include <cstdint>

namespace hw {

enum DeviceMode
{
   LOGIC = 0,
   DSO = 1,
   ANALOG = 2,
   UNKNOWN = 99,
};

enum DeviceStatus
{
   STATUS_ERROR = -1,
   STATUS_READY = 0,
   STATUS_INIT = 1,
   STATUS_START = 2,
   STATUS_TRIGGERED = 3,
   STATUS_DATA = 4,
   STATUS_STOP = 5,
   STATUS_PAUSE = 6,
   STATUS_FINISH = 7,
   STATUS_ABORT = 8,
};

enum ChannelType
{
   CHANNEL_LOGIC,
   CHANNEL_DSO,
   CHANNEL_ANALOG,
};

}

#endif

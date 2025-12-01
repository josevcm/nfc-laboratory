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

#ifndef LOGIC_SIPEEDLOGICINTERNAL_H
#define LOGIC_SIPEEDLOGICINTERNAL_H

#include <libusb.h>

#include <LogicInternal.h>

namespace hw {

struct sipeed_caps
{
};

struct sipeed_profile
{
   int vid;
   int pid;
   libusb_speed usb_speed;

   const char *vendor;
   const char *model; //product name

   sipeed_caps dev_caps;
};

// supported devices
static const sipeed_profile sipeed_profiles[] = {
   {
      .vid = 0x359F,
      .pid = 0x0300,
      .usb_speed = LIBUSB_SPEED_HIGH,
      .vendor = "Sipeed",
      .model = "SLogic Combo8",
      .dev_caps {
      }
   }
};

}
#endif //LOGIC_SIPEEDLOGICINTERNAL_H

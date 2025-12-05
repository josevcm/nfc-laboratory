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

#include "LogicInternal.h"

/* Protocol commands */
#define CMD_START	0xb1
#define CMD_STOP	0xb3

#define SAMPLES_ALIGN 1023LL

// #define ATOMIC_BITS 8
// #define ATOMIC_SAMPLES (1 << ATOMIC_BITS)
// #define ATOMIC_SIZE (1 << (ATOMIC_BITS - 3))
// #define ATOMIC_MASK (0xFFFF << ATOMIC_BITS)

namespace hw::logic {

struct sipeed_caps
{
   int total_ch_num;
   long channels;
   const unsigned long long *samplerates;
   int default_channelid;
   long default_samplerate;
   long default_samplelimit;
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

#pragma pack(push, 1)
struct cmd_start_acquisition
{
   union
   {
      struct
      {
         uint8_t sample_rate_l;
         uint8_t sample_rate_h;
      };

      uint16_t sample_rate;
   };

   uint8_t sample_channel;
   uint8_t unknow_value;
};
#pragma pack(pop)

static const uint64_t samplerates[] = {
   /* 160M = 2*2*2*2*2*5M */
   DEV_MHZ(1),
   DEV_MHZ(2),
   DEV_MHZ(4),
   DEV_MHZ(5),
   DEV_MHZ(8),
   DEV_MHZ(10),
   DEV_MHZ(16),
   DEV_MHZ(20),
   DEV_MHZ(32),
   DEV_MHZ(36),
   DEV_MHZ(40),
   /* x 4ch */
   DEV_MHZ(64),
   DEV_MHZ(80),
   /* x 2ch */
   DEV_MHZ(120),
   DEV_MHZ(128),
   DEV_MHZ(144),
   DEV_MHZ(160),
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
         .total_ch_num = 8, // total_ch_num
         .channels = DEV_CH(SipeedLogicDevice::SLD_STREAM120x2) | DEV_CH(SipeedLogicDevice::SLD_STREAM40x4) | DEV_CH(SipeedLogicDevice::SLD_STREAM20x8),
         .samplerates = samplerates, // samplerates
         .default_channelid = SipeedLogicDevice::SLD_STREAM20x8, // default_channelid
         .default_samplerate = DEV_MHZ(1), // default_samplerate
         .default_samplelimit = DEV_Mn(1), // default_samplelimit
      }
   }
};

}
#endif //LOGIC_SIPEEDLOGICINTERNAL_H

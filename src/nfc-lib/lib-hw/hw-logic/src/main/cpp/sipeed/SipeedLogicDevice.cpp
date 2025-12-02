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

// https://github.com/sipeed/sigrok_slogic/blob/hardware-sipeed-slogic-analyzer-support/src/hardware/sipeed-slogic-analyzer/protocol.h#L165
// https://github.com/sipeed/sigrok_slogic/blob/hardware-sipeed-slogic-analyzer-support/src/hardware/dreamsourcelab-dslogic/protocol.c#L538

#include <unistd.h>

#include <cmath>
#include <list>
#include <fstream>
#include <algorithm>

#include <rt/Logger.h>

#include <hw/usb/Usb.h>

#include <hw/SignalType.h>

#include <hw/logic/SipeedLogicDevice.h>

#include "SipeedLogicInternal.h"

#define ENDPOINT_IN 1
#define USB_INTERFACE 0
#define SIZE_MAX_EP_HS 512
#define NUM_MAX_TRANSFERS 64

#define DEVICE_TYPE_PREFIX "logic.sipeed"
#define CHANNEL_BUFFER_SIZE (1 << 16) // must be multiple of 64
#define CHANNEL_BUFFER_SAMPLES 16384 // number of samples per buffer

namespace hw::logic {

struct SipeedLogicDevice::Impl
{
   rt::Logger *log = rt::Logger::getLogger("hw.SipeedLogicDevice");

   // device parameters
   std::string deviceName;
   std::string deviceVendor;
   std::string deviceModel;
   std::string deviceVersion;
   std::string deviceSerial;
   std::string firmwarePath;

   // Underlying USB device.
   Usb usb;

   // Device profile.
   const sipeed_profile *profile = nullptr;

   // Device configuration
   unsigned int timebase = 0;
   unsigned int samplerate = 0;
   unsigned int streamTime = 0;
   unsigned long long limitSamples = 0;
   unsigned long long captureSamples = 0;
   unsigned long long captureBytes = 0;
   unsigned long long currentSamples = 0;
   unsigned long long currentBytes = 0;
   unsigned long long droppedSamples = 0;
   unsigned long long droppedBytes = 0;

   // Operational settings
   int deviceStatus = STATUS_ERROR;
   int operationMode;
   int channelMode;
   unsigned int totalChannels;
   unsigned int validChannels;

   explicit Impl(const std::string &name) : deviceName(name)
   {
      log->debug("created SipeedLogicDevice [{}]", {deviceName});
   }

   ~Impl()
   {
      close();
      log->debug("destroy SipeedLogicDevice [{}]", {deviceName});
   }

   bool open(Mode mode)
   {
      if (usb.isOpen())
      {
         log->error("device already open!, close first");
         return false;
      }

      if (mode != Read)
      {
         log->warn("invalid device mode [{}]", {mode});
         return false;
      }

      if (deviceName.find(DEVICE_TYPE_PREFIX) != 0)
      {
         log->warn("invalid device name [{}]", {deviceName});
         return false;
      }

      for (auto &descriptor: Usb::list())
      {
         // search for Sipeed device profile
         for (auto &p: sipeed_profiles)
         {
            if (!(descriptor.vid == p.vid && descriptor.pid == p.pid))
               continue;

            if (buildName(&p) != deviceName)
               continue;

            // set USB accessor
            usb = Usb(descriptor);

            break;
         }

         if (usb.isValid())
            break;
      }

      if (!usb.isValid())
      {
         log->warn("unknown device name [{}]", {deviceName});
         return false;
      }

      log->info("opening SipeedLogic on bus {03} device {03}", {usb.descriptor().bus, usb.descriptor().address});

      if (!usb.open())
      {
         log->error("failed to open USB device");
         return false;
      }

      while (true)
      {
         profile = nullptr;

         if (!(usb.isHighSpeed() || usb.isSuperSpeed()))
         {
            log->error("failed to open, usb speed is too low, speed type: {}", {usb.speed()});
            break;
         }

         if (!usb.claimInterface(USB_INTERFACE))
         {
            log->error("failed to claim USB interface {}", {USB_INTERFACE});
            break;
         }

         /* check profile. */
         for (auto &p: sipeed_profiles)
         {
            // find device and initialize for selected profile
            if (usb.descriptor().vid == p.vid && usb.descriptor().pid == p.pid && usb.speed() == p.usb_speed)
            {
               profile = &p;

               // initialize device defaults
               initDevice();

               // initialize channel defaults
               initChannels();

               // device selected, break
               break;
            }
         }

         if (!profile)
         {
            log->error("no profile found for device {0x4}.{04x}", {usb.descriptor().vid, usb.descriptor().pid});
            break;
         }

         // fill device info
         deviceVendor = profile->vendor;
         deviceModel = profile->model;
         deviceSerial = "sipeed";
         deviceStatus = STATUS_READY;

         // finish initialization
         const Usb::Descriptor &desc = usb.descriptor();

         log->info("opened {} on bus {03} device {03}", {std::string(profile->model), desc.bus, desc.address});

         return true;
      }

      usb.close();

      profile = nullptr;

      return false;
   }

   void close()
   {
      if (usb.isOpen())
      {
         // stop acquisition
         stop();

         // release USB interface
         usb.releaseInterface(USB_INTERFACE);

         // close underlying USB device
         usb.close();
      }
   }

   int start(const StreamHandler &handler)
   {
      log->debug("starting acquisition for device {}", {deviceName});

      purgeEndpoint(ENDPOINT_IN);

      log->debug("acquisition started for device {}", {deviceName});

      return 0;
   }

   int stop()
   {
      log->debug("stopping acquisition for device {}", {deviceName});

      log->debug("capture finished for device {}", {deviceName});

      return 0;
   }

   int pause()
   {
      log->debug("pause acquisition for device {}", {deviceName});

      return 0;
   }

   int resume()
   {
      log->debug("resume acquisition for device {}", {deviceName});

      return 0;
   }

   rt::Variant get(int id, int channel) const
   {
      switch (id)
      {
         case PARAM_DEVICE_NAME:
            return deviceName;

         case PARAM_DEVICE_VENDOR:
            return deviceVendor;

         case PARAM_DEVICE_MODEL:
            return deviceModel;

         case PARAM_DEVICE_SERIAL:
            return deviceSerial;

         case PARAM_DEVICE_VERSION:
            return deviceVersion;

         case PARAM_CHANNEL_MODE:
            return channelMode;

         case PARAM_CHANNEL_TOTAL:
            return totalChannels;

         case PARAM_CHANNEL_VALID:
            return validChannels;

         case PARAM_STREAM_TIME:
            return streamTime;

         case PARAM_SAMPLE_RATE:
            return samplerate;

         case PARAM_SAMPLES_READ:
            return currentSamples;

         case PARAM_SAMPLES_LOST:
            return droppedSamples;

         default:
            log->error("invalid configuration id {}", {id});
            return false;
      }
   }

   bool set(int id, const rt::Variant &value, int channel)
   {
      // auto ch = std::find_if(channels.begin(), channels.end(), [channel](const dsl_channel &c) { return c.index == channel; });
      //
      // if (channel >= 0 && ch == channels.end())
      // {
      //    log->error("invalid channel {}", {channel});
      //    return false;
      // }

      switch (id)
      {
         case PARAM_SAMPLE_RATE:
         {
            if (auto v = std::get_if<unsigned int>(&value))
            {
               samplerate = *v;
               log->info("setting samplerate to {}", {samplerate});
               return true;
            }

            log->error("invalid value type for PARAM_SAMPLE_RATE");
            return false;
         }
         case PARAM_OPERATION_MODE:
         {
            if (auto v = std::get_if<int>(&value))
            {
               // update operation mode
               operationMode = *v;
               log->info("setting operation mode to {}", {operationMode});
               return true;
            }

            log->error("invalid value type for PARAM_OPERATION_MODE");
            return false;
         }
         case PARAM_LIMIT_SAMPLES:
         {
            if (auto v = std::get_if<unsigned long long>(&value))
            {
               limitSamples = *v;
               log->info("setting limit samples to {}", {limitSamples});
               return true;
            }

            log->error("invalid value type for PARAM_LIMIT_SAMPLES");
            return false;
         }
      }

      return false;
   }

   void initDevice()
   {
      // device flags
      operationMode = OP_STREAM;

      // device settings
      samplerate = profile->dev_caps.default_samplerate;
      limitSamples = profile->dev_caps.default_samplelimit;
      channelMode = profile->dev_caps.default_channelid;
      timebase = 10000;

      // channels
      totalChannels = 0;
      validChannels = 0;
   }

   int initChannels()
   {
      // channels.clear();
      //
      // for (int i = 0; i < channel_modes[channelMode].vld_num; i++)
      // {
      //    dsl_channel channel {
      //       .index = i,
      //       .type = channel_modes[channelMode].type,
      //       .enabled = true,
      //       .name = probe_names[i],
      //       .bits = channel_modes[channelMode].unit_bits,
      //       .vdiv = 1000,
      //       .vfactor = 1,
      //       .offset = (1 << (channel.bits - 1)),
      //       .vpos_trans = profile->dev_caps.default_pwmtrans,
      //       .coupling = DC_COUPLING,
      //       .trig_value = (1 << (channel.bits - 1)),
      //       .comb_comp = profile->dev_caps.default_comb_comp,
      //       .digi_fgain = 0,
      //       .cali_fgain0 = 1,
      //       .cali_fgain1 = 1,
      //       .cali_fgain2 = 1,
      //       .cali_fgain3 = 1,
      //       .cali_comb_fgain0 = 1,
      //       .cali_comb_fgain1 = 1,
      //       .cali_comb_fgain2 = 1,
      //       .cali_comb_fgain3 = 1,
      //       .map_default = true,
      //       .map_unit = probe_units[0],
      //       .map_min = -(static_cast<double>(channel.vdiv) * static_cast<double>(channel.vfactor) * DS_CONF_DSO_VDIVS / 2000.0),
      //       .map_max = static_cast<double>(channel.vdiv) * static_cast<double>(channel.vfactor) * DS_CONF_DSO_VDIVS / 2000.0,
      //    };
      //
      //    if (profile->dev_caps.vdivs)
      //    {
      //       for (int j = 0; profile->dev_caps.vdivs[j]; j++)
      //       {
      //          dsl_vga vga {
      //             .id = profile->dev_caps.vga_id,
      //             .key = profile->dev_caps.vdivs[j],
      //             .vgain = 0,
      //             .preoff = 0,
      //             .preoff_comp = 0,
      //          };
      //
      //          for (const auto &vga_default: vga_defaults)
      //          {
      //             if (vga_default.id == profile->dev_caps.vga_id && vga_default.key == profile->dev_caps.vdivs[j])
      //             {
      //                vga.vgain = vga_defaults[j].vgain;
      //                vga.preoff = vga_defaults[j].preoff;
      //                vga.preoff_comp = 0;
      //             }
      //          }
      //
      //          channel.vga_list.push_back(vga);
      //       }
      //    }
      //
      //    channels.push_back(channel);
      // }
      //
      // totalChannels = channel_modes[channelMode].vld_num;
      // validChannels = channel_modes[channelMode].vld_num;

      totalChannels = 8;
      validChannels = 8;

      return true;
   }

   bool isReady() const
   {
      return true; //return usbRead(rd_cmd_fw_version);
   }

   void purgeEndpoint(int endpoint)
   {
      log->debug("clearing device endpoint: {}", {endpoint});

      unsigned char tmp[512];
      unsigned int purged = 0;
      unsigned int received = 0;

      while ((received = usb.syncTransfer(Usb::In, endpoint, tmp, sizeof(tmp), 100)) > 0)
      {
         purged += received;
      }

      log->debug("endpoint {}, cleared, purged {} bytes", {endpoint, purged});
   }

   static std::string buildName(const sipeed_profile *profile)
   {
      char buffer[1024];

      snprintf(buffer, sizeof(buffer), "%s://%04x:%04x@%s %s", DEVICE_TYPE_PREFIX, profile->vid, profile->pid, profile->vendor, profile->model);

      return {buffer};
   }
};

SipeedLogicDevice::SipeedLogicDevice(const std::string &name) : impl(new Impl(name))
{
}

SipeedLogicDevice::~SipeedLogicDevice()
{
}

bool SipeedLogicDevice::open(Mode mode)
{
   return impl->open(mode);
}

void SipeedLogicDevice::close()
{
   impl->close();
}

int SipeedLogicDevice::start(StreamHandler handler)
{
   return impl->start(handler);
}

int SipeedLogicDevice::stop()
{
   return impl->stop();
}

int SipeedLogicDevice::pause()
{
   return impl->pause();
}

int SipeedLogicDevice::resume()
{
   return impl->resume();
}

rt::Variant SipeedLogicDevice::get(int id, int channel) const
{
   return impl->get(id, channel);
}

bool SipeedLogicDevice::set(int id, const rt::Variant &value, int channel)
{
   return impl->set(id, value, channel);
}

bool SipeedLogicDevice::isOpen() const
{
   return impl->usb.isOpen();
}

bool SipeedLogicDevice::isEof() const
{
   return false; //return impl->deviceStatus != STATUS_READY && impl->deviceStatus != STATUS_START && impl->deviceStatus != STATUS_DATA;
}

bool SipeedLogicDevice::isReady() const
{
   return impl->deviceStatus >= STATUS_READY && impl->isReady();
}

bool SipeedLogicDevice::isPaused() const
{
   return false; //return impl->deviceStatus == STATUS_PAUSE;
}

bool SipeedLogicDevice::isStreaming() const
{
   return false; //return impl->deviceStatus == STATUS_START || impl->deviceStatus == STATUS_DATA;
}

long SipeedLogicDevice::read(SignalBuffer &buffer)
{
   return -1;
}

long SipeedLogicDevice::write(const SignalBuffer &buffer)
{
   return -1;
}

std::vector<std::string> SipeedLogicDevice::enumerate()
{
   std::vector<std::string> devices;

   for (const auto &descriptor: Usb::list())
   {
      // search for Sipeed Logic device
      for (auto &profile: sipeed_profiles)
      {
         if (!(descriptor.vid == profile.vid && descriptor.pid == profile.pid))
            continue;

         devices.push_back(Impl::buildName(&profile));

         break;
      }
   }

   return devices;
}

}

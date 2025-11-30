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

#define USB_INTERFACE        0

#define DEVICE_TYPE_PREFIX "logic.sipeedlogic"
#define CHANNEL_BUFFER_SIZE (1 << 16) // must be multiple of 64
#define CHANNEL_BUFFER_SAMPLES 16384 // number of samples per buffer

namespace hw {

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
               // initDevice();

               // initialize channel defaults
               // initChannels();

               // device selected, break
               break;
            }
         }

         if (!profile)
         {
            log->error("no profile found for device {0x4}.{04x}", {usb.descriptor().vid, usb.descriptor().pid});
            break;
         }

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

      return false;
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
   return false; //return impl->deviceStatus >= STATUS_READY && impl->isReady();
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

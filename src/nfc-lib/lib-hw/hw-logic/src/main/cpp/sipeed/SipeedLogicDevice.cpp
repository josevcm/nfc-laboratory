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
#define CHANNEL_BUFFER_SAMPLES 32768 // number of samples per buffer

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

   /*
    * Transfer buffers
    */
   std::list<Usb::Transfer *> transfers;

   /*
    * Received buffers
    */
   SignalBuffer buffer;

   /*
    * Receive handler
    */
   StreamHandler streamHandler;

   // sample conversion map
   const static float sri_samples[256][8];

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

      deviceStatus = STATUS_INIT;

      currentSamples = 0;
      currentBytes = 0;

      droppedSamples = 0;
      droppedBytes = 0;

      buffer.reset();

      // purge pending data
      purgeTransfers();

      // setup usb transfers
      beginTransfers(handler);

      // send start command
      if (startAcquisition() != 0)
      {
         deviceStatus = STATUS_ERROR;
         return -1;
      }

      streamHandler = handler;
      deviceStatus = STATUS_START;

      log->debug("acquisition started for device {}", {deviceName});

      return 0;
   }

   int stop()
   {
      log->debug("stopping acquisition for device {}", {deviceName});

      // if device is not started, just return
      if (deviceStatus == STATUS_PAUSE)
         return 0;

      // send stop command
      // stopAcquisition();

      // cancel current transfers
      for (const auto transfer: transfers)
         usb.cancelTransfer(transfer);

      deviceStatus = STATUS_STOP;
      streamHandler = nullptr;

      log->debug("acquisition finished for device {}", {deviceName});

      return 0;
   }

   int pause()
   {
      log->debug("pause acquisition for device {}", {deviceName});

      if (deviceStatus != STATUS_DATA)
      {
         log->error("failed to pause acquisition, deice is not streaming");
         return -1;
      }

      // send stop command
      stopAcquisition();

      // cancel current transfers
      for (const auto transfer: transfers)
         usb.cancelTransfer(transfer);

      deviceStatus = STATUS_PAUSE;

      return 0;
   }

   int resume()
   {
      log->debug("resume acquisition for device {}", {deviceName});

      if (deviceStatus != STATUS_PAUSE)
      {
         log->error("failed to resume acquisition, deice is not paused");
         return -1;
      }

      buffer.reset();

      // purge pending data
      purgeTransfers();

      // setup usb transfers
      beginTransfers(streamHandler);

      // start acquisition
      if (startAcquisition() != 0)
         deviceStatus = STATUS_ERROR;
      else
         deviceStatus = STATUS_START;

      return deviceStatus == STATUS_START ? 0 : -1;
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
      totalChannels = 8;
      validChannels = 4;

      return true;
   }

   int startAcquisition() const
   {
      cmd_start_acquisition start {};

      start.sample_rate = samplerate / DEV_MHZ(1);
      start.sample_channel = totalChannels;
      start.unknow_value = 0;

      // send start command
      if (!usb.ctrlTransfer(CMD_START, &start, sizeof(start), 0, nullptr, 0))
      {
         log->error("usb transfer CMD_START failed: {}", {usb.lastError()});
         return -1;
      }

      return 0;
   }

   int stopAcquisition() const
   {
      // send start command
      if (!usb.ctrlTransfer(CMD_STOP, nullptr, 0, 0, nullptr, 0))
      {
         log->error("usb transfer CMD_STOP failed: {}", {usb.lastError()});
         return -1;
      }

      return 0;
   }

   void purgeTransfers() const
   {
      constexpr int endpoint = ENDPOINT_IN;

      log->debug("clearing device endpoint: {}", {endpoint});

      unsigned char tmp[512];

      int purged = 0;
      int received = 0;

      while ((received = usb.syncTransfer(Usb::In, endpoint, tmp, sizeof(tmp), 100)) > 0)
      {
         purged += received;
      }

      log->debug("endpoint {}, cleared, purged {} bytes", {endpoint, purged});
   }

   bool beginTransfers(const StreamHandler &handler)
   {
      // create header buffer
      log->debug("usb transfer buffer size {}", {bufferSize()});

      // initialize target buffer for received data
      buffer = SignalBuffer(CHANNEL_BUFFER_SAMPLES * validChannels, validChannels, 1, samplerate, 0, 0, SIGNAL_TYPE_LOGIC_SAMPLES);

      // submit transfer of data buffers
      for (int i = 0; i < totalTransfers(); i++)
      {
         auto transfer = new Usb::Transfer();
         transfer->data = new unsigned char[bufferSize()];
         transfer->available = bufferSize();
         transfer->timeout = 5000;
         transfer->callback = [=](Usb::Transfer *t) -> Usb::Transfer * { return usbProcessData(t, handler); };

         // clean buffer
         memset(transfer->data, 0, transfer->available);

         // add transfer to device list
         transfers.push_back(transfer);

         // submit transfer of data buffer
         if (!usb.asyncTransfer(Usb::In, ENDPOINT_IN, transfer))
         {
            log->error("failed to setup async transfer: {}", {usb.lastError()});

            for (Usb::Transfer *t: transfers)
               usb.cancelTransfer(t);

            break;
         }
      }

      return true;
   }

   Usb::Transfer *usbProcessData(Usb::Transfer *transfer, const StreamHandler &handler)
   {
      if (deviceStatus == STATUS_START)
         deviceStatus = STATUS_DATA;

      switch (transfer->status)
      {
         case Usb::Completed:
         case Usb::TimeOut:
            break;
         case Usb::Cancelled:
            log->debug("data transfer cancelled with USB status {}", {transfer->status});
            break;
         default:
            log->error("data transfer failed with USB status {}", {transfer->status});
            deviceStatus = STATUS_ERROR;
            break;
      }

      // trigger next transfer
      if (deviceStatus == STATUS_DATA && transfer->actual != 0)
      {
         // interleave received data in single buffer with N channels stride
         std::vector<SignalBuffer> buffers = interleave(transfer);

         // call user handler for each channel
         for (auto &b: buffers)
         {
            if (!handler(b))
            {
               log->warn("data transfer stopped by handler, aborting!");
               deviceStatus = STATUS_ABORT;
               break;
            }
         }

         if (deviceStatus == STATUS_DATA)
         {
            // reset transfer buffer received size
            transfer->actual = 0;

            // clean buffer
            memset(transfer->data, 0, transfer->available);

            // resend new transfer
            return transfer;
         }
      }

      // remove transfer from list
      transfers.remove(transfer);

      // free header buffer
      delete transfer->data;

      // free transfer
      delete transfer;

      log->debug("finish data transfer, remain {} transfers", {transfers.size()});

      // no resend transfer
      return nullptr;
   }

   std::vector<SignalBuffer> interleave(const Usb::Transfer *transfer)
   {
      std::vector<SignalBuffer> result;

      /*
       * Interleave data from transfer buffer
       */
      for (unsigned int i = 0; i < transfer->actual; ++i)
      {
         const unsigned char value = transfer->data[i];

         buffer.put(sri_samples[value], validChannels);

         // rotate buffer and create new once
         if (buffer.isFull())
         {
            buffer.flip();

            result.push_back(buffer);

            buffer = SignalBuffer(CHANNEL_BUFFER_SAMPLES * validChannels, validChannels, 1, samplerate, currentSamples + i, 0, SIGNAL_TYPE_LOGIC_SAMPLES);
         }
      }

      // update current bytes and samples
      currentBytes += transfer->actual;
      currentSamples += transfer->actual;

      return result;
   }

   unsigned int totalTransfers() const
   {
      return 64;
   }

   unsigned int bufferSize() const
   {
      return 256 * SIZE_MAX_EP_HS;
   }

   bool isReady() const
   {
      return true; //return usbRead(rd_cmd_fw_version);
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

#define L 0
#define H 1

// bitmap values from 0 to 255 to float
const float SipeedLogicDevice::Impl::sri_samples[256][8] {
   {L, L, L, L, L, L, L, L},
   {H, L, L, L, L, L, L, L},
   {L, H, L, L, L, L, L, L},
   {H, H, L, L, L, L, L, L},
   {L, L, H, L, L, L, L, L},
   {H, L, H, L, L, L, L, L},
   {L, H, H, L, L, L, L, L},
   {H, H, H, L, L, L, L, L},
   {L, L, L, H, L, L, L, L},
   {H, L, L, H, L, L, L, L},
   {L, H, L, H, L, L, L, L},
   {H, H, L, H, L, L, L, L},
   {L, L, H, H, L, L, L, L},
   {H, L, H, H, L, L, L, L},
   {L, H, H, H, L, L, L, L},
   {H, H, H, H, L, L, L, L},
   {L, L, L, L, H, L, L, L},
   {H, L, L, L, H, L, L, L},
   {L, H, L, L, H, L, L, L},
   {H, H, L, L, H, L, L, L},
   {L, L, H, L, H, L, L, L},
   {H, L, H, L, H, L, L, L},
   {L, H, H, L, H, L, L, L},
   {H, H, H, L, H, L, L, L},
   {L, L, L, H, H, L, L, L},
   {H, L, L, H, H, L, L, L},
   {L, H, L, H, H, L, L, L},
   {H, H, L, H, H, L, L, L},
   {L, L, H, H, H, L, L, L},
   {H, L, H, H, H, L, L, L},
   {L, H, H, H, H, L, L, L},
   {H, H, H, H, H, L, L, L},
   {L, L, L, L, L, H, L, L},
   {H, L, L, L, L, H, L, L},
   {L, H, L, L, L, H, L, L},
   {H, H, L, L, L, H, L, L},
   {L, L, H, L, L, H, L, L},
   {H, L, H, L, L, H, L, L},
   {L, H, H, L, L, H, L, L},
   {H, H, H, L, L, H, L, L},
   {L, L, L, H, L, H, L, L},
   {H, L, L, H, L, H, L, L},
   {L, H, L, H, L, H, L, L},
   {H, H, L, H, L, H, L, L},
   {L, L, H, H, L, H, L, L},
   {H, L, H, H, L, H, L, L},
   {L, H, H, H, L, H, L, L},
   {H, H, H, H, L, H, L, L},
   {L, L, L, L, H, H, L, L},
   {H, L, L, L, H, H, L, L},
   {L, H, L, L, H, H, L, L},
   {H, H, L, L, H, H, L, L},
   {L, L, H, L, H, H, L, L},
   {H, L, H, L, H, H, L, L},
   {L, H, H, L, H, H, L, L},
   {H, H, H, L, H, H, L, L},
   {L, L, L, H, H, H, L, L},
   {H, L, L, H, H, H, L, L},
   {L, H, L, H, H, H, L, L},
   {H, H, L, H, H, H, L, L},
   {L, L, H, H, H, H, L, L},
   {H, L, H, H, H, H, L, L},
   {L, H, H, H, H, H, L, L},
   {H, H, H, H, H, H, L, L},
   {L, L, L, L, L, L, H, L},
   {H, L, L, L, L, L, H, L},
   {L, H, L, L, L, L, H, L},
   {H, H, L, L, L, L, H, L},
   {L, L, H, L, L, L, H, L},
   {H, L, H, L, L, L, H, L},
   {L, H, H, L, L, L, H, L},
   {H, H, H, L, L, L, H, L},
   {L, L, L, H, L, L, H, L},
   {H, L, L, H, L, L, H, L},
   {L, H, L, H, L, L, H, L},
   {H, H, L, H, L, L, H, L},
   {L, L, H, H, L, L, H, L},
   {H, L, H, H, L, L, H, L},
   {L, H, H, H, L, L, H, L},
   {H, H, H, H, L, L, H, L},
   {L, L, L, L, H, L, H, L},
   {H, L, L, L, H, L, H, L},
   {L, H, L, L, H, L, H, L},
   {H, H, L, L, H, L, H, L},
   {L, L, H, L, H, L, H, L},
   {H, L, H, L, H, L, H, L},
   {L, H, H, L, H, L, H, L},
   {H, H, H, L, H, L, H, L},
   {L, L, L, H, H, L, H, L},
   {H, L, L, H, H, L, H, L},
   {L, H, L, H, H, L, H, L},
   {H, H, L, H, H, L, H, L},
   {L, L, H, H, H, L, H, L},
   {H, L, H, H, H, L, H, L},
   {L, H, H, H, H, L, H, L},
   {H, H, H, H, H, L, H, L},
   {L, L, L, L, L, H, H, L},
   {H, L, L, L, L, H, H, L},
   {L, H, L, L, L, H, H, L},
   {H, H, L, L, L, H, H, L},
   {L, L, H, L, L, H, H, L},
   {H, L, H, L, L, H, H, L},
   {L, H, H, L, L, H, H, L},
   {H, H, H, L, L, H, H, L},
   {L, L, L, H, L, H, H, L},
   {H, L, L, H, L, H, H, L},
   {L, H, L, H, L, H, H, L},
   {H, H, L, H, L, H, H, L},
   {L, L, H, H, L, H, H, L},
   {H, L, H, H, L, H, H, L},
   {L, H, H, H, L, H, H, L},
   {H, H, H, H, L, H, H, L},
   {L, L, L, L, H, H, H, L},
   {H, L, L, L, H, H, H, L},
   {L, H, L, L, H, H, H, L},
   {H, H, L, L, H, H, H, L},
   {L, L, H, L, H, H, H, L},
   {H, L, H, L, H, H, H, L},
   {L, H, H, L, H, H, H, L},
   {H, H, H, L, H, H, H, L},
   {L, L, L, H, H, H, H, L},
   {H, L, L, H, H, H, H, L},
   {L, H, L, H, H, H, H, L},
   {H, H, L, H, H, H, H, L},
   {L, L, H, H, H, H, H, L},
   {H, L, H, H, H, H, H, L},
   {L, H, H, H, H, H, H, L},
   {H, H, H, H, H, H, H, L},
   {L, L, L, L, L, L, L, H},
   {H, L, L, L, L, L, L, H},
   {L, H, L, L, L, L, L, H},
   {H, H, L, L, L, L, L, H},
   {L, L, H, L, L, L, L, H},
   {H, L, H, L, L, L, L, H},
   {L, H, H, L, L, L, L, H},
   {H, H, H, L, L, L, L, H},
   {L, L, L, H, L, L, L, H},
   {H, L, L, H, L, L, L, H},
   {L, H, L, H, L, L, L, H},
   {H, H, L, H, L, L, L, H},
   {L, L, H, H, L, L, L, H},
   {H, L, H, H, L, L, L, H},
   {L, H, H, H, L, L, L, H},
   {H, H, H, H, L, L, L, H},
   {L, L, L, L, H, L, L, H},
   {H, L, L, L, H, L, L, H},
   {L, H, L, L, H, L, L, H},
   {H, H, L, L, H, L, L, H},
   {L, L, H, L, H, L, L, H},
   {H, L, H, L, H, L, L, H},
   {L, H, H, L, H, L, L, H},
   {H, H, H, L, H, L, L, H},
   {L, L, L, H, H, L, L, H},
   {H, L, L, H, H, L, L, H},
   {L, H, L, H, H, L, L, H},
   {H, H, L, H, H, L, L, H},
   {L, L, H, H, H, L, L, H},
   {H, L, H, H, H, L, L, H},
   {L, H, H, H, H, L, L, H},
   {H, H, H, H, H, L, L, H},
   {L, L, L, L, L, H, L, H},
   {H, L, L, L, L, H, L, H},
   {L, H, L, L, L, H, L, H},
   {H, H, L, L, L, H, L, H},
   {L, L, H, L, L, H, L, H},
   {H, L, H, L, L, H, L, H},
   {L, H, H, L, L, H, L, H},
   {H, H, H, L, L, H, L, H},
   {L, L, L, H, L, H, L, H},
   {H, L, L, H, L, H, L, H},
   {L, H, L, H, L, H, L, H},
   {H, H, L, H, L, H, L, H},
   {L, L, H, H, L, H, L, H},
   {H, L, H, H, L, H, L, H},
   {L, H, H, H, L, H, L, H},
   {H, H, H, H, L, H, L, H},
   {L, L, L, L, H, H, L, H},
   {H, L, L, L, H, H, L, H},
   {L, H, L, L, H, H, L, H},
   {H, H, L, L, H, H, L, H},
   {L, L, H, L, H, H, L, H},
   {H, L, H, L, H, H, L, H},
   {L, H, H, L, H, H, L, H},
   {H, H, H, L, H, H, L, H},
   {L, L, L, H, H, H, L, H},
   {H, L, L, H, H, H, L, H},
   {L, H, L, H, H, H, L, H},
   {H, H, L, H, H, H, L, H},
   {L, L, H, H, H, H, L, H},
   {H, L, H, H, H, H, L, H},
   {L, H, H, H, H, H, L, H},
   {H, H, H, H, H, H, L, H},
   {L, L, L, L, L, L, H, H},
   {H, L, L, L, L, L, H, H},
   {L, H, L, L, L, L, H, H},
   {H, H, L, L, L, L, H, H},
   {L, L, H, L, L, L, H, H},
   {H, L, H, L, L, L, H, H},
   {L, H, H, L, L, L, H, H},
   {H, H, H, L, L, L, H, H},
   {L, L, L, H, L, L, H, H},
   {H, L, L, H, L, L, H, H},
   {L, H, L, H, L, L, H, H},
   {H, H, L, H, L, L, H, H},
   {L, L, H, H, L, L, H, H},
   {H, L, H, H, L, L, H, H},
   {L, H, H, H, L, L, H, H},
   {H, H, H, H, L, L, H, H},
   {L, L, L, L, H, L, H, H},
   {H, L, L, L, H, L, H, H},
   {L, H, L, L, H, L, H, H},
   {H, H, L, L, H, L, H, H},
   {L, L, H, L, H, L, H, H},
   {H, L, H, L, H, L, H, H},
   {L, H, H, L, H, L, H, H},
   {H, H, H, L, H, L, H, H},
   {L, L, L, H, H, L, H, H},
   {H, L, L, H, H, L, H, H},
   {L, H, L, H, H, L, H, H},
   {H, H, L, H, H, L, H, H},
   {L, L, H, H, H, L, H, H},
   {H, L, H, H, H, L, H, H},
   {L, H, H, H, H, L, H, H},
   {H, H, H, H, H, L, H, H},
   {L, L, L, L, L, H, H, H},
   {H, L, L, L, L, H, H, H},
   {L, H, L, L, L, H, H, H},
   {H, H, L, L, L, H, H, H},
   {L, L, H, L, L, H, H, H},
   {H, L, H, L, L, H, H, H},
   {L, H, H, L, L, H, H, H},
   {H, H, H, L, L, H, H, H},
   {L, L, L, H, L, H, H, H},
   {H, L, L, H, L, H, H, H},
   {L, H, L, H, L, H, H, H},
   {H, H, L, H, L, H, H, H},
   {L, L, H, H, L, H, H, H},
   {H, L, H, H, L, H, H, H},
   {L, H, H, H, L, H, H, H},
   {H, H, H, H, L, H, H, H},
   {L, L, L, L, H, H, H, H},
   {H, L, L, L, H, H, H, H},
   {L, H, L, L, H, H, H, H},
   {H, H, L, L, H, H, H, H},
   {L, L, H, L, H, H, H, H},
   {H, L, H, L, H, H, H, H},
   {L, H, H, L, H, H, H, H},
   {H, H, H, L, H, H, H, H},
   {L, L, L, H, H, H, H, H},
   {H, L, L, H, H, H, H, H},
   {L, H, L, H, H, H, H, H},
   {H, H, L, H, H, H, H, H},
   {L, L, H, H, H, H, H, H},
   {H, L, H, H, H, H, H, H},
   {L, H, H, H, H, H, H, H},
   {H, H, H, H, H, H, H, H},
};

}

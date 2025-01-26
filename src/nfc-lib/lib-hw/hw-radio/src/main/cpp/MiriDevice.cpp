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

#include <queue>
#include <mutex>
#include <chrono>

#include <mirisdr.h>

#include <rt/Logger.h>

#include <hw/DeviceFactory.h>
#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <hw/radio/MiriDevice.h>

namespace hw {

#define MIRI_SUCCESS 0

#define MAX_QUEUE_SIZE 4

#define ASYNC_BUF_NUMBER 32
#define ASYNC_BUF_LENGTH (16 * 16384)

#define DEVICE_TYPE_PREFIX "radio.miri"

int process_transfer(unsigned char *buf, uint32_t len, void *ctx);

struct MiriDevice::Impl
{
   rt::Logger *log = rt::Logger::getLogger("hw.MiriDevice");

   std::string deviceName;
   std::string deviceSerial;
   std::string deviceVendor;
   std::string deviceModel;
   std::string deviceVersion;
   int fileDesc = 0;
   unsigned int centerFreq = 13.56E6;
   unsigned int sampleRate = 10E6;
   unsigned int sampleSize = 16;
   unsigned int sampleType = SAMPLE_TYPE_FLOAT;
   unsigned int gainMode = 0;
   unsigned int gainValue = 0;
   unsigned int tunerAgc = 0;
   unsigned int mixerAgc = 0;
   unsigned int decimation = 0;
   unsigned int streamTime = 0;

   mirisdr_dev_t *deviceHandle = nullptr;

   std::mutex streamMutex;
   std::queue<SignalBuffer> streamQueue;
   StreamHandler streamCallback;

   std::thread asyncThread;

   long long samplesReceived = 0;
   long long samplesDropped = 0;

   explicit Impl(std::string name) : deviceName(std::move(name))
   {
      log->debug("created MiriDevice for name [{}]", {this->deviceName});
   }

   explicit Impl(int fileDesc) : fileDesc(fileDesc)
   {
      log->debug("created MiriDevice for file descriptor [{}]", {fileDesc});
   }

   ~Impl()
   {
      log->debug("destroy MiriDevice [{}]", {deviceName});

      close();
   }

   static std::vector<std::string> enumerate()
   {
      std::vector<std::string> result;

      unsigned int count = mirisdr_get_device_count();

      for (int i = 0; i < count; i++)
      {
         char buffer[256];

         const char *name = mirisdr_get_device_name(i);

         snprintf(buffer, sizeof(buffer), "%s://%s", DEVICE_TYPE_PREFIX, name);

         result.emplace_back(buffer);
      }

      return result;
   }

   bool open(Mode mode)
   {
      mirisdr_dev_t *handle = nullptr;

      if (mode != RadioDevice::Read)
      {
         log->warn("invalid device mode [{}]", {mode});
         return false;
      }

      if (deviceName.find(DEVICE_TYPE_PREFIX) != 0)
      {
         log->warn("invalid device name [{}]", {deviceName});
         return false;
      }

      close();

      int index = 0;

      if (mirisdr_open(&handle, index) == MIRI_SUCCESS)
      {
         deviceHandle = handle;

         char vendor[512], product[512], serial[512];

         // read board serial
         if (mirisdr_get_device_usb_strings(index, vendor, product, serial) != MIRI_SUCCESS)
            log->warn("failed mirisdr_get_device_usb_strings!");

         // set HW flavour
         if (mirisdr_set_hw_flavour(handle, MIRISDR_HW_DEFAULT) != MIRI_SUCCESS)
            log->warn("failed mirisdr_set_hw_flavour!");

         // set bandwidth
         if (mirisdr_set_bandwidth(handle, 8000000) != MIRI_SUCCESS)
            log->warn("failed mirisdr_set_bandwidth!");

         // set sample format, 10+2 bit
         if (mirisdr_set_sample_format(handle, (char *) "384_S16") != MIRI_SUCCESS)
            log->warn("failed mirisdr_set_sample_format!");

         // set USB transfer type
         if (mirisdr_set_transfer(handle, (char *) "BULK") != MIRI_SUCCESS)
            log->warn("failed mirisdr_set_transfer!");

         // set IF mode
         if (mirisdr_set_if_freq(handle, 0) != MIRI_SUCCESS)
            log->warn("failed mirisdr_set_if_freq!");

         // fill device info
         deviceVendor = "Generic";
         deviceModel = "MSI2500-MSI001";
         deviceSerial = serial;

         // configure frequency
         setCenterFreq(centerFreq);

         // configure samplerate
         setSampleRate(sampleRate);

         // configure gain mode
         setGainMode(gainMode);

         // configure gain value
         setGainValue(gainValue);

         log->info("opened miri device {}, vendor {} product {} serial {}", {deviceName, std::string(vendor), std::string(product), std::string(serial)});

         return true;
      }

      log->warn("failed mirisdr_open!");

      return false;
   }

   void close()
   {
      if (deviceHandle)
      {
         // stop streaming if active...
         stop();

         log->info("close device {}", {deviceName});

         // close device
         if (mirisdr_close(deviceHandle) != MIRI_SUCCESS)
            log->warn("failed mirisdr_close!");

         deviceName = "";
         deviceSerial = "";
         deviceVersion = "";
         deviceHandle = nullptr;
      }
   }

   int start(RadioDevice::StreamHandler handler)
   {
      if (deviceHandle)
      {
         log->info("start streaming for device {}", {deviceName});

         // clear counters
         samplesDropped = 0;
         samplesReceived = 0;

         // reset stream status
         streamCallback = std::move(handler);
         streamQueue = std::queue<SignalBuffer>();

         // sets stream start time
         streamTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

         // start async transfer thread
         asyncThread = std::thread([=]() {
            if (mirisdr_read_async(deviceHandle, reinterpret_cast<mirisdr_read_async_cb_t>(process_transfer), this, 0, 0) != MIRI_SUCCESS)
               log->warn("failed mirisdr_read_async!");
         });

         return 0;
      }

      return -1;
   }

   int stop()
   {
      if (deviceHandle && streamCallback)
      {
         log->info("stop streaming for device {}", {deviceName});

         // stop reception
         if (mirisdr_cancel_async(deviceHandle) != MIRI_SUCCESS)
            log->warn("failed mirisdr_cancel_async!");

         // wait for thread joint
         asyncThread.join();

         // disable stream callback and queue
         streamCallback = nullptr;
         streamQueue = std::queue<SignalBuffer>();
         streamTime = 0;

         return 0;
      }

      return -1;
   }

   bool isOpen() const
   {
      return deviceHandle;
   }

   bool isEof() const
   {
      return !deviceHandle || !streamCallback;
   }

   bool isReady() const
   {
      return deviceHandle;
   }

   bool isStreaming() const
   {
      return deviceHandle && streamCallback;
   }

   int setCenterFreq(unsigned int value)
   {
      centerFreq = value;

      if (deviceHandle)
      {
         if (mirisdr_set_center_freq(deviceHandle, centerFreq) != MIRI_SUCCESS)
            log->warn("failed mirisdr_set_center_freq!");

         return 0;
      }

      return -1;
   }

   int setSampleRate(unsigned int value)
   {
      sampleRate = value;

      if (deviceHandle)
      {
         if (mirisdr_set_sample_rate(deviceHandle, sampleRate) != MIRI_SUCCESS)
            log->warn("failed mirisdr_set_sample_rate!");

         return 0;
      }

      return -1;
   }

   int setGainMode(unsigned int mode)
   {
      gainMode = mode;

      if (deviceHandle)
      {
         if (mirisdr_set_tuner_gain_mode(deviceHandle, mode) != MIRI_SUCCESS)
            log->warn("failed mirisdr_set_tuner_gain_mode!");

         if (gainMode == MiriDevice::Manual)
         {
            return setGainValue(gainValue);
         }
      }

      return -1;
   }

   int setGainValue(unsigned int value)
   {
      gainValue = value;

      if (deviceHandle)
      {
         if (mirisdr_set_tuner_gain(deviceHandle, gainValue) != MIRI_SUCCESS)
            log->warn("failed mirisdr_set_tuner_gain!");

         return 0;
      }

      return -1;
   }

   int setTunerAgc(unsigned int value)
   {
      tunerAgc = value;

      return -1;
   }

   int setMixerAgc(unsigned int value)
   {
      mixerAgc = value;

      return 0;
   }

   int setDecimation(unsigned int value)
   {
      decimation = value;

      return 0;
   }

   rt::Catalog supportedSampleRates() const
   {
      rt::Catalog result;

      result[5000000] = "5000000"; // 5 MSPS
      result[10000000] = "10000000"; // 10 MSPS

      return result;
   }

   rt::Catalog supportedGainModes() const
   {
      rt::Catalog result;

      result[MiriDevice::Auto] = "Auto";
      result[MiriDevice::Manual] = "Manual";

      return result;
   }

   rt::Catalog supportedGainValues() const
   {
      int gains[512];

      rt::Catalog result;

      int count = mirisdr_get_tuner_gains(deviceHandle, gains);

      for (int i = 0; i < count; i++)
      {
         int value = gains[i];

         char buffer[64];

         snprintf(buffer, sizeof(buffer), "%d db", value);

         result[value] = buffer;
      }

      return result;
   }

   int read(SignalBuffer &buffer)
   {
      // lock buffer access
      std::lock_guard<std::mutex> lock(streamMutex);

      if (!streamQueue.empty())
      {
         buffer = streamQueue.front();

         streamQueue.pop();

         return buffer.limit();
      }

      return -1;
   }

   int write(SignalBuffer &buffer)
   {
      log->warn("write not supported on this device!");

      return -1;
   }
};

MiriDevice::MiriDevice(const std::string &name) : impl(std::make_shared<Impl>(name))
{
}

MiriDevice::MiriDevice(int fd) : impl(std::make_shared<Impl>(fd))
{
}

std::vector<std::string> MiriDevice::enumerate()
{
   return Impl::enumerate();
}

bool MiriDevice::open(Device::Mode mode)
{
   return impl->open(mode);
}

void MiriDevice::close()
{
   impl->close();
}

int MiriDevice::start(RadioDevice::StreamHandler handler)
{
   return impl->start(handler);
}

int MiriDevice::stop()
{
   return impl->stop();
}

rt::Variant MiriDevice::get(int id, int channel) const
{
   switch (id)
   {
      case PARAM_DEVICE_NAME:
         return impl->deviceName;

      case PARAM_DEVICE_SERIAL:
         return impl->deviceSerial;

      case PARAM_DEVICE_VENDOR:
         return impl->deviceVendor;

      case PARAM_DEVICE_MODEL:
         return impl->deviceModel;

      case PARAM_DEVICE_VERSION:
         return impl->deviceVersion;

      case PARAM_SAMPLE_RATE:
         return impl->sampleRate;

      case PARAM_SAMPLE_SIZE:
         return impl->sampleSize;

      case PARAM_SAMPLE_TYPE:
         return impl->sampleType;

      case PARAM_TUNE_FREQUENCY:
         return impl->centerFreq;

      case PARAM_TUNER_AGC:
         return impl->tunerAgc;

      case PARAM_MIXER_AGC:
         return impl->mixerAgc;

      case PARAM_GAIN_MODE:
         return impl->gainMode;

      case PARAM_GAIN_VALUE:
         return impl->gainValue;

      case PARAM_BIAS_TEE:
         return (unsigned int) 0;

      case PARAM_DIRECT_SAMPLING:
         return (int) 0;

      case PARAM_DECIMATION:
         return impl->decimation;

      case PARAM_STREAM_TIME:
         return impl->streamTime;

      case PARAM_SAMPLES_READ:
         return impl->samplesReceived;

      case PARAM_SAMPLES_LOST:
         return impl->samplesDropped;

      case PARAM_SUPPORTED_SAMPLE_RATES:
         return impl->supportedSampleRates();

      case PARAM_SUPPORTED_GAIN_MODES:
         return impl->supportedGainModes();

      case PARAM_SUPPORTED_GAIN_VALUES:
         return impl->supportedGainValues();

      default:
         return {};
   }
}

bool MiriDevice::set(int id, const rt::Variant &value, int channel)
{
   switch (id)
   {
      case PARAM_SAMPLE_RATE:
      {
         if (auto v = std::get_if<int>(&value))
            return impl->setSampleRate(*v);

         impl->log->error("invalid value type for PARAM_SAMPLE_RATE");
         return false;
      }
      case PARAM_TUNE_FREQUENCY:
      {
         if (auto v = std::get_if<int>(&value))
            return impl->setCenterFreq(*v);

         impl->log->error("invalid value type for PARAM_TUNE_FREQUENCY");
         return false;
      }
      case PARAM_TUNER_AGC:
      {
         if (auto v = std::get_if<int>(&value))
            return impl->setTunerAgc(*v);

         impl->log->error("invalid value type for PARAM_TUNER_AGC");
         return false;
      }
      case PARAM_MIXER_AGC:
      {
         if (auto v = std::get_if<int>(&value))
            return impl->setMixerAgc(*v);

         impl->log->error("invalid value type for PARAM_MIXER_AGC");
         return false;
      }
      case PARAM_GAIN_MODE:
      {
         if (auto v = std::get_if<int>(&value))
            return impl->setGainMode(*v);

         impl->log->error("invalid value type for PARAM_GAIN_MODE");
         return false;
      }
      case PARAM_GAIN_VALUE:
      {
         if (auto v = std::get_if<int>(&value))
            return impl->setGainValue(*v);

         impl->log->error("invalid value type for PARAM_GAIN_VALUE");
         return false;
      }
      case PARAM_DECIMATION:
      {
         if (auto v = std::get_if<int>(&value))
            return impl->setDecimation(*v);

         impl->log->error("invalid value type for PARAM_DECIMATION");
         return false;
      }
      default:
         impl->log->warn("unknown or unsupported configuration id {}", {id});
         return false;
   }
}

bool MiriDevice::isOpen() const
{
   return impl->isOpen();
}

bool MiriDevice::isEof() const
{
   return impl->isEof();
}

bool MiriDevice::isReady() const
{
   return impl->isReady();
}

bool MiriDevice::isStreaming() const
{
   return impl->isStreaming();
}

int MiriDevice::read(SignalBuffer &buffer)
{
   return impl->read(buffer);
}

int MiriDevice::write(SignalBuffer &buffer)
{
   return impl->write(buffer);
}

int process_transfer(unsigned char *buf, uint32_t len, void *ctx)
{
   printf("process_transfer");
   fflush(stdout);

   // check device validity
   if (auto *device = static_cast<MiriDevice::Impl *>(ctx))
   {
      SignalBuffer buffer;

//      SignalBuffer buffer = SignalBuffer((float *) transfer->samples, transfer->sample_count * 2, 2, device->sampleRate, device->samplesReceived, 0, SignalType::SIGNAL_TYPE_RAW_IQ);

      // update counters
      device->samplesReceived += len;

      // stream to buffer callback
      if (device->streamCallback)
      {
         device->streamCallback(buffer);
      }

         // or store buffer in receive queue
      else
      {
         // lock buffer access
         std::lock_guard<std::mutex> lock(device->streamMutex);

         // discard oldest buffers
         if (device->streamQueue.size() >= MAX_QUEUE_SIZE)
         {
            device->samplesDropped += device->streamQueue.front().elements();
            device->streamQueue.pop();
         }

         // queue new sample buffer
         device->streamQueue.push(buffer);
      }

      // continue streaming
      return 0;
   }

   return -1;
}

}
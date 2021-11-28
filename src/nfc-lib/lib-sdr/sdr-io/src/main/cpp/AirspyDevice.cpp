/*

  Copyright (c) 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <queue>
#include <mutex>

#include <airspy.h>

#include <rt/Logger.h>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>
#include <sdr/AirspyDevice.h>

#define MAX_QUEUE_SIZE 4

namespace sdr {

int process_transfer(airspy_transfer *transfer);

struct AirspyDevice::Impl
{
   rt::Logger log {"AirspyDevice"};

   std::string deviceName;
   std::string deviceVersion;
   int fileDesc = 0;
   int centerFreq = 0;
   int sampleRate = 0;
   int sampleSize = 16;
   int sampleType = RadioDevice::Float;
   int gainMode = 0;
   int gainValue = 0;
   int tunerAgc = 0;
   int mixerAgc = 0;
   int decimation = 0;

   int airspyResult = 0;
   airspy_device *airspyHandle = nullptr;
   airspy_read_partid_serialno_t airspySerial {};
   airspy_sample_type airspySample = AIRSPY_SAMPLE_FLOAT32_IQ;

   std::mutex streamMutex;
   std::queue<SignalBuffer> streamQueue;
   RadioDevice::StreamHandler streamCallback;

   long samplesReceived = 0;
   long samplesDropped = 0;
   long samplesStreamed = 0;

   explicit Impl(std::string name) : deviceName(std::move(name))
   {
      log.debug("created AirspyDevice for name [{}]", {this->deviceName});
   }

   explicit Impl(int fileDesc) : fileDesc(fileDesc)
   {
      log.debug("created AirspyDevice for file descriptor [{}]", {fileDesc});
   }

   ~Impl()
   {
      log.debug("destroy AirspyDevice");

      close();
   }

   static std::vector<std::string> listDevices()
   {
      std::vector<std::string> result;

      uint64_t devices[8];

      int count = airspy_list_devices(devices, sizeof(devices) / sizeof(uint64_t));

      for (int i = 0; i < count; i++)
      {
         char buffer[256];

         snprintf(buffer, sizeof(buffer), "airspy://%0llx", devices[i]);

         result.emplace_back(buffer);
      }

      return result;
   }

   bool open(SignalDevice::OpenMode mode)
   {
      airspy_device *handle;

      if (deviceName.find("://") != -1 && deviceName.find("airspy://") == -1)
      {
         log.warn("invalid device name [{}]", {deviceName});
         return false;
      }

      close();

#ifdef ANDROID
      // special open mode based on /sys/ node and file descriptor (primary for android devices)
   if (name.startsWith("airspy://sys/"))
   {
      // extract full /sys/... path to device node
      QByteArray node = device->name.mid(8).toLocal8Bit();

      // open Airspy device
      result = airspy_open_fd(&device->handle, node.data(), device->fileDesc);
   }
   else
#endif

      // standard open mode based on serial number
      {
         // extract serial number
         uint64_t sn = std::stoull(deviceName.substr(9), nullptr, 16);

         // open Airspy device
         airspyResult = airspy_open_sn(&handle, sn);
      }

      if (airspyResult == AIRSPY_SUCCESS)
      {
         airspyHandle = handle;

         char tmp[128];

         // get version string
         if ((airspyResult = airspy_version_string_read(handle, tmp, sizeof(tmp))) != AIRSPY_SUCCESS)
            log.warn("failed airspy_version_string_read: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

         // disable bias tee
         if ((airspyResult = airspy_set_rf_bias(handle, 0)) != AIRSPY_SUCCESS)
            log.warn("failed airspy_set_rf_bias: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

         // read board serial
         if ((airspyResult = airspy_board_partid_serialno_read(handle, &airspySerial)) != AIRSPY_SUCCESS)
            log.warn("failed airspy_board_partid_serialno_read: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

         // set sample type
         if ((airspyResult = airspy_set_sample_type(handle, airspySample)) != AIRSPY_SUCCESS)
            log.warn("failed airspy_set_sample_type: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

         // set version string
         deviceVersion = std::string(tmp);

         // configure frequency
         setCenterFreq(centerFreq);

         // configure samplerate
         setSampleRate(sampleRate);

         // configure gain mode
         setGainMode(gainMode);

         // configure gain value
         setGainValue(gainValue);

         log.info("openned airspy device {}, firmware {}", {deviceName, deviceVersion});

         return true;
      }

      log.warn("failed airspy_open_sn: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

      return false;
   }

   void close()
   {
      if (airspyHandle)
      {
         // stop streaming if active...
         stop();

         log.info("close device {}", {deviceName});

         // close device
         if ((airspyResult = airspy_close(airspyHandle)) != AIRSPY_SUCCESS)
            log.warn("failed airspy_close: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

         deviceName = "";
         deviceVersion = "";
         airspyHandle = nullptr;
      }
   }

   int start(RadioDevice::StreamHandler handler)
   {
      if (airspyHandle)
      {
         log.info("start streaming for device {}", {deviceName});

         // clear counters
         samplesDropped = 0;
         samplesReceived = 0;
         samplesStreamed = 0;
         streamCallback = std::move(handler);
         streamQueue = std::queue<SignalBuffer>();

         // start reception
         if ((airspyResult = airspy_start_rx(airspyHandle, reinterpret_cast<airspy_sample_block_cb_fn>(process_transfer), this)) != AIRSPY_SUCCESS)
            log.warn("failed airspy_start_rx: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

         // clear callback to disable receiver
         if (airspyResult != AIRSPY_SUCCESS)
            streamCallback = nullptr;

         return airspyResult;
      }

      return -1;
   }

   int stop()
   {
      if (airspyHandle && streamCallback)
      {
         log.info("stop streaming for device {}", {deviceName});

         // stop reception
         if ((airspyResult = airspy_stop_rx(airspyHandle)) != AIRSPY_SUCCESS)
            log.warn("failed airspy_stop_rx: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

         // disable stream callback and queue
         streamCallback = nullptr;
         streamQueue = std::queue<SignalBuffer>();

         return airspyResult;
      }

      return -1;
   }

   bool isOpen() const
   {
      return airspyHandle;
   }

   bool isEof() const
   {
      return !airspyHandle || !airspy_is_streaming(airspyHandle);
   }

   bool isReady() const
   {
      char version[1];

      return airspyHandle && airspy_version_string_read(airspyHandle, version, sizeof(version)) == AIRSPY_SUCCESS;
   }

   bool isStreaming() const
   {
      return airspyHandle && airspy_is_streaming(airspyHandle);
   }

   int setCenterFreq(long value)
   {
      centerFreq = value;

      if (airspyHandle)
      {
         if ((airspyResult = airspy_set_freq(airspyHandle, centerFreq)) != AIRSPY_SUCCESS)
            log.warn("failed airspy_set_freq: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

         return airspyResult;
      }

      return 0;
   }

   int setSampleRate(long value)
   {
      sampleRate = value;

      if (airspyHandle)
      {
         if ((airspyResult = airspy_set_samplerate(airspyHandle, sampleRate)) != AIRSPY_SUCCESS)
            log.warn("failed airspy_set_samplerate: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

         return airspyResult;
      }

      return 0;
   }

   int setGainMode(int mode)
   {
      gainMode = mode;

      if (airspyHandle)
      {
         if (gainMode == AirspyDevice::Auto)
         {
            if ((airspyResult = airspy_set_lna_agc(airspyHandle, tunerAgc)) != AIRSPY_SUCCESS)
               log.warn("failed airspy_set_lna_agc: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

            if ((airspyResult = airspy_set_mixer_agc(airspyHandle, mixerAgc)) != AIRSPY_SUCCESS)
               log.warn("failed airspy_set_mixer_agc: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

            return airspyResult;
         }
         else
         {
            return setGainValue(gainValue);
         }
      }

      return 0;
   }

   int setGainValue(int value)
   {
      gainValue = value;

      if (airspyHandle)
      {
         if (gainMode == AirspyDevice::Linearity)
         {
            if ((airspyResult = airspy_set_linearity_gain(airspyHandle, gainValue)) != AIRSPY_SUCCESS)
               log.warn("failed airspy_set_linearity_gain: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
         }
         else if (gainMode == AirspyDevice::Sensitivity)
         {
            if ((airspyResult = airspy_set_sensitivity_gain(airspyHandle, gainValue)) != AIRSPY_SUCCESS)
               log.warn("failed airspy_set_linearity_gain: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
         }

         return airspyResult;
      }

      return 0;
   }

   int setTunerAgc(int value)
   {
      tunerAgc = value;

      if (tunerAgc)
         gainMode = AirspyDevice::Auto;

      if (airspyHandle)
      {
         if ((airspyResult = airspy_set_lna_agc(airspyHandle, tunerAgc)) != AIRSPY_SUCCESS)
            log.warn("failed airspy_set_lna_agc: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

         return airspyResult;
      }

      return 0;
   }

   int setMixerAgc(int value)
   {
      mixerAgc = value;

      if (mixerAgc)
         gainMode = AirspyDevice::Auto;

      if (airspyHandle)
      {
         if ((airspyResult = airspy_set_mixer_agc(airspyHandle, mixerAgc)) != AIRSPY_SUCCESS)
            log.warn("failed airspy_set_mixer_agc: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

         return airspyResult;
      }

      return 0;
   }

   int setDecimation(int value)
   {
      decimation = value;

      return 0;
   }


   std::map<int, std::string> supportedSampleRates() const
   {
      std::map<int, std::string> result;

      uint32_t count, rates[256];

      if (airspyHandle)
      {
         // get number of supported sample rates
         airspy_get_samplerates(airspyHandle, &count, 0);

         // get list of supported sample rates
         airspy_get_samplerates(airspyHandle, rates, count);

         for (int i = 0; i < (int) count; i++)
         {
            char buffer[64];
            int samplerate = rates[i];

            snprintf(buffer, sizeof(buffer), "%d", samplerate);

            result[samplerate] = buffer;
         }
      }

      return result;
   }

   std::map<int, std::string> supportedGainModes() const
   {
      std::map<int, std::string> result;

      result[AirspyDevice::Auto] = "Auto";
      result[AirspyDevice::Linearity] = "Linearity";
      result[AirspyDevice::Sensitivity] = "Sensitivity";

      return result;
   }

   std::map<int, std::string> supportedGainValues() const
   {
      std::map<int, std::string> result;

      for (int i = 0; i < 22; i++)
      {
         char buffer[64];

         snprintf(buffer, sizeof(buffer), "%d db", i);

         result[i] = buffer;
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
      log.warn("write not supported on this device!");

      return -1;
   }

};

AirspyDevice::AirspyDevice(const std::string &name) : impl(std::make_shared<Impl>(name))
{
}

AirspyDevice::AirspyDevice(int fd) : impl(std::make_shared<Impl>(fd))
{
}

std::vector<std::string> AirspyDevice::listDevices()
{
   return Impl::listDevices();
}

const std::string &AirspyDevice::name()
{
   return impl->deviceName;
}

const std::string &AirspyDevice::version()
{
   return impl->deviceVersion;
}

bool AirspyDevice::open(OpenMode mode)
{
   return impl->open(mode);
}

void AirspyDevice::close()
{
   impl->close();
}

int AirspyDevice::start(StreamHandler handler)
{
   return impl->start(handler);
}

int AirspyDevice::stop()
{
   return impl->stop();
}

bool AirspyDevice::isOpen() const
{
   return impl->isOpen();
}

bool AirspyDevice::isEof() const
{
   return impl->isEof();
}

bool AirspyDevice::isReady() const
{
   return impl->isReady();
}

bool AirspyDevice::isStreaming() const
{
   return impl->isStreaming();
}

int AirspyDevice::sampleSize() const
{
   return impl->sampleSize;
}

int AirspyDevice::setSampleSize(int value)
{
   impl->log.warn("setSampleSize has no effect!");

   return -1;
}

long AirspyDevice::sampleRate() const
{
   return impl->sampleRate;
}

int AirspyDevice::setSampleRate(long value)
{
   return impl->setSampleRate(value);
}

int AirspyDevice::sampleType() const
{
   return Float;
}

int AirspyDevice::setSampleType(int value)
{
   impl->log.warn("setSampleType has no effect!");

   return -1;
}

long AirspyDevice::centerFreq() const
{
   return impl->centerFreq;
}

int AirspyDevice::setCenterFreq(long value)
{
   return impl->setCenterFreq(value);
}

int AirspyDevice::tunerAgc() const
{
   return impl->tunerAgc;
}

int AirspyDevice::setTunerAgc(int value)
{
   return impl->setTunerAgc(value);
}

int AirspyDevice::mixerAgc() const
{
   return impl->mixerAgc;
}

int AirspyDevice::setMixerAgc(int value)
{
   return impl->setMixerAgc(value);
}

int AirspyDevice::gainMode() const
{
   return impl->gainMode;
}

int AirspyDevice::setGainMode(int value)
{
   return impl->setGainMode(value);
}

int AirspyDevice::gainValue() const
{
   return impl->gainValue;
}

int AirspyDevice::setGainValue(int value)
{
   return impl->setGainValue(value);
}

int AirspyDevice::decimation() const
{
   return impl->decimation;
}

int AirspyDevice::setDecimation(int value)
{
   return impl->setDecimation(value);
}

long AirspyDevice::samplesReceived()
{
   return impl->samplesReceived;
}

long AirspyDevice::samplesDropped()
{
   return impl->samplesDropped;
}

long AirspyDevice::samplesStreamed()
{
   return impl->samplesStreamed;
}

std::map<int, std::string> AirspyDevice::supportedSampleRates() const
{
   return impl->supportedSampleRates();
}

std::map<int, std::string> AirspyDevice::supportedGainModes() const
{
   return impl->supportedGainModes();
}

std::map<int, std::string> AirspyDevice::supportedGainValues() const
{
   return impl->supportedGainValues();
}

int AirspyDevice::read(SignalBuffer &buffer)
{
   return impl->read(buffer);
}

int AirspyDevice::write(SignalBuffer &buffer)
{
   return impl->write(buffer);
}

int process_transfer(airspy_transfer *transfer)
{
   // check device validity
   if (auto *device = static_cast<AirspyDevice::Impl *>(transfer->ctx))
   {
      SignalBuffer buffer;

      switch (transfer->sample_type)
      {
         case AIRSPY_SAMPLE_FLOAT32_REAL:
            buffer = SignalBuffer((float *) transfer->samples, transfer->sample_count, 1, device->sampleRate, device->samplesReceived, 0, SignalType::SAMPLE_REAL);
            break;

         case AIRSPY_SAMPLE_FLOAT32_IQ:
            buffer = SignalBuffer((float *) transfer->samples, transfer->sample_count * 2, 2, device->sampleRate, device->samplesReceived, 0, SignalType::SAMPLE_IQ);
            break;
      }

      // update counters
      device->samplesReceived += transfer->sample_count;
      device->samplesDropped += transfer->dropped_samples;

      // stream to buffer callback
      if (device->streamCallback)
      {
         device->samplesStreamed += transfer->sample_count;
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

      // trace dropped samples
      if (transfer->dropped_samples > 0)
         device->log.warn("dropped samples {}", {device->samplesDropped});

      // continue streaming
      return 0;
   }

   return -1;
}

}
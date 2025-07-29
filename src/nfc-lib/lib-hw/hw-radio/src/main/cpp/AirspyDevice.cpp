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
#include <cinttypes>

#include <airspy.h>

#include <rt/Logger.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <hw/radio/AirspyDevice.h>

#define MAX_QUEUE_SIZE 4

#define DEVICE_TYPE_PREFIX "radio.airspy"

namespace hw {

int process_transfer(airspy_transfer *transfer);

struct AirspyDevice::Impl
{
   rt::Logger *log = rt::Logger::getLogger("hw.AirspyDevice");

   std::string deviceName;
   std::string deviceSerial;
   std::string deviceVendor;
   std::string deviceModel;
   std::string deviceVersion;

   int fileDesc = 0;

   unsigned int centerFreq = 40.68E6;
   unsigned int sampleRate = 10E6;
   unsigned int sampleSize = 16;
   unsigned int sampleType = SAMPLE_TYPE_FLOAT;
   unsigned int gainMode = 0;
   unsigned int gainValue = 0;
   unsigned int tunerAgc = 0;
   unsigned int mixerAgc = 0;
   unsigned int biasTee = 0;
   unsigned int decimation = 0;
   unsigned int streamTime = 0;

   int airspyResult = 0;
   airspy_device *airspyHandle = nullptr;
   airspy_read_partid_serialno_t airspySerial {};
   airspy_sample_type airspySample = AIRSPY_SAMPLE_FLOAT32_IQ;

   std::mutex streamMutex;
   std::queue<SignalBuffer> streamQueue;
   StreamHandler streamCallback;
   bool streamPaused = false;

   long long samplesReceived = 0;
   long long samplesDropped = 0;

   explicit Impl(std::string name) : deviceName(std::move(name))
   {
      log->debug("created AirspyDevice for name [{}]", {this->deviceName});
   }

   explicit Impl(int fileDesc) : fileDesc(fileDesc)
   {
      log->debug("created AirspyDevice for file descriptor [{}]", {fileDesc});
   }

   ~Impl()
   {
      log->debug("destroy AirspyDevice [{}]", {deviceName});

      close();
   }

   static std::vector<std::string> enumerate()
   {
      std::vector<std::string> result;

      uint64_t devices[8];

      int count = airspy_list_devices(devices, sizeof(devices) / sizeof(uint64_t));

      for (int i = 0; i < count; i++)
      {
         char buffer[256];

         snprintf(buffer, sizeof(buffer), "%s://%" PRIu64, DEVICE_TYPE_PREFIX, devices[i]);

         result.emplace_back(buffer);
      }

      return result;
   }

   bool open(Mode mode)
   {
      airspy_device *handle = nullptr;

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

      close();

#ifdef ANDROID
      // special open mode based on /sys/ node and file descriptor (primary for android devices)
   if (name.startsWith(DEVICE_TYPE_PREFIX "://sys/"))
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
         deviceSerial = deviceName.substr(strlen(DEVICE_TYPE_PREFIX) + 3);

         // extract serial number
         uint64_t sn = std::stoull(deviceSerial, nullptr, 10);

         // open Airspy device
         airspyResult = airspy_open_sn(&handle, sn);
      }

      if (airspyResult == AIRSPY_SUCCESS)
      {
         airspyHandle = handle;

         char tmp[128];

         // get version string
         if ((airspyResult = airspy_version_string_read(handle, tmp, sizeof(tmp))) != AIRSPY_SUCCESS)
            log->warn("failed airspy_version_string_read: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

         // read board serial
         if ((airspyResult = airspy_board_partid_serialno_read(handle, &airspySerial)) != AIRSPY_SUCCESS)
            log->warn("failed airspy_board_partid_serialno_read: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

         // set sample type
         if ((airspyResult = airspy_set_sample_type(handle, airspySample)) != AIRSPY_SUCCESS)
            log->warn("failed airspy_set_sample_type: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

         // fill device info
         std::string vtmp = std::string(tmp); // format "AirSpy MINI v1.0.0-rc10-6-g4008185 2020-05-08"

         deviceVendor = "AirSpy";
         deviceModel = vtmp.substr(0, vtmp.find(" v"));
         deviceVersion = vtmp.substr(vtmp.find('v'), vtmp.find(' ', vtmp.find('v')) - vtmp.find('v'));

         // configure frequency
         setCenterFreq(centerFreq);

         // configure samplerate
         setSampleRate(sampleRate);

         // configure gain mode
         setGainMode(gainMode);

         // configure gain value
         setGainValue(gainValue);

         // configure bias tee (LNA or SpyVerter)
         setBiasTee(biasTee);

         log->info("opened device {}, model {} firmware {}", {deviceName, deviceModel, deviceVersion});

         return true;
      }

      log->warn("failed airspy_open_sn: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

      return false;
   }

   void close()
   {
      if (!airspyHandle)
         return;

      // stop streaming if active...
      stop();

      // disable bias tee
      if ((airspyResult = airspy_set_rf_bias(airspyHandle, 0)) != AIRSPY_SUCCESS)
         log->warn("failed airspy_set_rf_bias: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

      log->info("close device {}", {deviceName});

      // close device
      if ((airspyResult = airspy_close(airspyHandle)) != AIRSPY_SUCCESS)
         log->warn("failed airspy_close: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

      deviceName = "";
      deviceVersion = "";
      airspyHandle = nullptr;
   }

   int start(StreamHandler handler)
   {
      if (!airspyHandle)
         return -1;

      log->info("start streaming for device {}", {deviceName});

      // clear counters
      samplesDropped = 0;
      samplesReceived = 0;

      // reset stream status
      streamPaused = false;
      streamCallback = std::move(handler);
      streamQueue = std::queue<SignalBuffer>();

      // start reception
      if ((airspyResult = airspy_start_rx(airspyHandle, process_transfer, this)) != AIRSPY_SUCCESS)
         log->warn("failed airspy_start_rx: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

      // clear callback to disable receiver
      if (airspyResult != AIRSPY_SUCCESS)
         streamCallback = nullptr;

      // sets stream start time
      streamTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

      return airspyResult;
   }

   int stop()
   {
      if (!airspyHandle || !streamCallback)
         return 1;

      log->info("stop streaming for device {}", {deviceName});

      // stop reception
      if ((airspyResult = airspy_stop_rx(airspyHandle)) != AIRSPY_SUCCESS)
         log->warn("failed airspy_stop_rx: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

      // disable stream callback and queue
      streamPaused = false;
      streamCallback = nullptr;
      streamQueue = std::queue<SignalBuffer>();
      streamTime = 0;

      return airspyResult;
   }

   int pause()
   {
      if (!airspyHandle || !streamCallback)
         return 1;

      log->info("pause streaming for device {}", {deviceName});

      // stop reception
      if ((airspyResult = airspy_stop_rx(airspyHandle)) != AIRSPY_SUCCESS)
         log->warn("failed airspy_stop_rx: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

      // set paused state
      streamPaused = airspyResult == AIRSPY_SUCCESS;

      return airspyResult;
   }

   int resume()
   {
      if (!airspyHandle || !streamCallback || !streamPaused)
         return -1;

      log->info("resume streaming for device {}", {deviceName});

      // start reception
      if ((airspyResult = airspy_start_rx(airspyHandle, process_transfer, this)) != AIRSPY_SUCCESS)
         log->warn("failed airspy_start_rx: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

      // clear callback to disable receiver
      if (airspyResult != AIRSPY_SUCCESS)
         streamCallback = nullptr;

      streamPaused = false;

      return airspyResult;
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

   bool isPaused() const
   {
      return airspyHandle && streamPaused;
   }

   bool isStreaming() const
   {
      return airspyHandle && airspy_is_streaming(airspyHandle);
   }

   int setCenterFreq(unsigned int value)
   {
      centerFreq = value;

      if (airspyHandle)
      {
         if ((airspyResult = airspy_set_freq(airspyHandle, centerFreq)) != AIRSPY_SUCCESS)
            log->warn("failed airspy_set_freq: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

         return airspyResult;
      }

      return 0;
   }

   int setSampleRate(unsigned int value)
   {
      sampleRate = value;

      if (airspyHandle)
      {
         if ((airspyResult = airspy_set_samplerate(airspyHandle, sampleRate)) != AIRSPY_SUCCESS)
            log->warn("failed airspy_set_samplerate: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

         return airspyResult;
      }

      return 0;
   }

   int setGainMode(unsigned int mode)
   {
      gainMode = mode;

      if (airspyHandle)
      {
         if (gainMode == AirspyDevice::Auto)
         {
            if ((airspyResult = airspy_set_lna_agc(airspyHandle, tunerAgc)) != AIRSPY_SUCCESS)
               log->warn("failed airspy_set_lna_agc: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

            if ((airspyResult = airspy_set_mixer_agc(airspyHandle, mixerAgc)) != AIRSPY_SUCCESS)
               log->warn("failed airspy_set_mixer_agc: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

            return airspyResult;
         }

         return setGainValue(gainValue);
      }

      return 0;
   }

   int setGainValue(unsigned int value)
   {
      gainValue = value;

      if (airspyHandle)
      {
         if (gainMode == AirspyDevice::Linearity)
         {
            if ((airspyResult = airspy_set_linearity_gain(airspyHandle, gainValue)) != AIRSPY_SUCCESS)
               log->warn("failed airspy_set_linearity_gain: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});
         }
         else if (gainMode == AirspyDevice::Sensitivity)
         {
            if ((airspyResult = airspy_set_sensitivity_gain(airspyHandle, gainValue)) != AIRSPY_SUCCESS)
               log->warn("failed airspy_set_linearity_gain: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});
         }

         return airspyResult;
      }

      return 0;
   }

   int setTunerAgc(unsigned int value)
   {
      tunerAgc = value;

      if (tunerAgc)
         gainMode = AirspyDevice::Auto;

      if (airspyHandle)
      {
         if ((airspyResult = airspy_set_lna_agc(airspyHandle, tunerAgc)) != AIRSPY_SUCCESS)
            log->warn("failed airspy_set_lna_agc: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

         return airspyResult;
      }

      return 0;
   }

   int setMixerAgc(unsigned int value)
   {
      mixerAgc = value;

      if (mixerAgc)
         gainMode = AirspyDevice::Auto;

      if (airspyHandle)
      {
         if ((airspyResult = airspy_set_mixer_agc(airspyHandle, mixerAgc)) != AIRSPY_SUCCESS)
            log->warn("failed airspy_set_mixer_agc: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

         return airspyResult;
      }

      return 0;
   }

   int setBiasTee(unsigned int value)
   {
      biasTee = value;

      if (airspyHandle)
      {
         if ((airspyResult = airspy_set_rf_bias(airspyHandle, biasTee)) != AIRSPY_SUCCESS)
            log->warn("failed airspy_set_rf_bias: [{}] {}", {airspyResult, airspy_error_name(static_cast<airspy_error>(airspyResult))});

         return airspyResult;
      }

      return 0;
   }

   int setDecimation(unsigned int value)
   {
      decimation = value;

      return 0;
   }

   int setTestMode(unsigned int value)
   {
      log->warn("test mode not supported on this device!");

      return -1;
   }

   rt::Catalog supportedSampleRates() const
   {
      rt::Catalog result;

      uint32_t count, rates[256];

      if (airspyHandle)
      {
         // get number of supported sample rates
         airspy_get_samplerates(airspyHandle, &count, 0);

         // get list of supported sample rates
         airspy_get_samplerates(airspyHandle, rates, count);

         for (int i = 0; i < (int)count; i++)
         {
            char buffer[64];
            int samplerate = rates[i];

            snprintf(buffer, sizeof(buffer), "%d", samplerate);

            result[samplerate] = buffer;
         }
      }

      return result;
   }

   rt::Catalog supportedGainModes() const
   {
      rt::Catalog result;

      result[AirspyDevice::Auto] = "Auto";
      result[AirspyDevice::Linearity] = "Linearity";
      result[AirspyDevice::Sensitivity] = "Sensitivity";

      return result;
   }

   rt::Catalog supportedGainValues() const
   {
      rt::Catalog result;

      for (int i = 0; i < 22; i++)
      {
         char buffer[64];

         snprintf(buffer, sizeof(buffer), "%d db", i);

         result[i] = buffer;
      }

      return result;
   }

   long read(SignalBuffer &buffer)
   {
      // lock buffer access
      std::lock_guard lock(streamMutex);

      if (!streamQueue.empty())
      {
         buffer = streamQueue.front();

         streamQueue.pop();

         return buffer.limit();
      }

      return -1;
   }

   long write(const SignalBuffer &buffer)
   {
      log->warn("write not supported on this device!");

      return -1;
   }

};

AirspyDevice::AirspyDevice(const std::string &name) : impl(std::make_shared<Impl>(name))
{
}

AirspyDevice::AirspyDevice(int fd) : impl(std::make_shared<Impl>(fd))
{
}

std::vector<std::string> AirspyDevice::enumerate()
{
   return Impl::enumerate();
}

bool AirspyDevice::open(Mode mode)
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

int AirspyDevice::pause()
{
   return impl->pause();
}

int AirspyDevice::resume()
{
   return impl->resume();
}

rt::Variant AirspyDevice::get(int id, int channel) const
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

      case PARAM_TEST_MODE:
         return (unsigned int)0;

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
         return impl->biasTee;

      case PARAM_DIRECT_SAMPLING:
         return (unsigned int)0;

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

bool AirspyDevice::set(int id, const rt::Variant &value, int channel)
{
   switch (id)
   {
      case PARAM_SAMPLE_RATE:
      {
         if (auto v = std::get_if<unsigned int>(&value))
            return impl->setSampleRate(*v);

         impl->log->error("invalid value type for PARAM_SAMPLE_RATE");
         return false;
      }
      case PARAM_TUNE_FREQUENCY:
      {
         if (auto v = std::get_if<unsigned int>(&value))
            return impl->setCenterFreq(*v);

         impl->log->error("invalid value type for PARAM_TUNE_FREQUENCY");
         return false;
      }
      case PARAM_TUNER_AGC:
      {
         if (auto v = std::get_if<unsigned int>(&value))
            return impl->setTunerAgc(*v);

         impl->log->error("invalid value type for PARAM_TUNER_AGC");
         return false;
      }
      case PARAM_MIXER_AGC:
      {
         if (auto v = std::get_if<unsigned int>(&value))
            return impl->setMixerAgc(*v);

         impl->log->error("invalid value type for PARAM_MIXER_AGC");
         return false;
      }
      case PARAM_GAIN_MODE:
      {
         if (auto v = std::get_if<unsigned int>(&value))
            return impl->setGainMode(*v);

         impl->log->error("invalid value type for PARAM_GAIN_MODE");
         return false;
      }
      case PARAM_GAIN_VALUE:
      {
         if (auto v = std::get_if<unsigned int>(&value))
            return impl->setGainValue(*v);

         impl->log->error("invalid value type for PARAM_GAIN_VALUE");
         return false;
      }
      case PARAM_BIAS_TEE:
      {
         if (auto v = std::get_if<unsigned int>(&value))
            return impl->setBiasTee(*v);

         impl->log->error("invalid value type for PARAM_BIAS_TEE");
         return false;
      }
      case PARAM_DECIMATION:
      {
         if (auto v = std::get_if<unsigned int>(&value))
            return impl->setDecimation(*v);

         impl->log->error("invalid value type for PARAM_DECIMATION");
         return false;
      }
      default:
         impl->log->warn("unknown or unsupported configuration id {}", {id});
         return false;
   }
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

bool AirspyDevice::isPaused() const
{
   return impl->isPaused();
}

bool AirspyDevice::isStreaming() const
{
   return impl->isStreaming();
}

long AirspyDevice::read(SignalBuffer &buffer)
{
   return impl->read(buffer);
}

long AirspyDevice::write(const SignalBuffer &buffer)
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
         case AIRSPY_SAMPLE_FLOAT32_IQ:
            buffer = SignalBuffer(static_cast<float *>(transfer->samples), transfer->sample_count * 2, 2, 1, device->sampleRate, device->samplesReceived, 0, SignalType::SIGNAL_TYPE_RADIO_IQ);
            break;

         case AIRSPY_SAMPLE_FLOAT32_REAL:
            buffer = SignalBuffer(static_cast<float *>(transfer->samples), transfer->sample_count, 1, 1, device->sampleRate, device->samplesReceived, 0, SignalType::SIGNAL_TYPE_RADIO_SAMPLES);
            break;

         default:
            return -1;
      }

      // update counters
      device->samplesReceived += transfer->sample_count;
      device->samplesDropped += transfer->dropped_samples;

      // stream to buffer callback
      if (device->streamCallback)
      {
         device->streamCallback(buffer);
      }

      // or store buffer in receive queue
      else
      {
         // lock buffer access
         std::lock_guard lock(device->streamMutex);

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
         device->log->warn("dropped samples {}", {device->samplesDropped});

      // continue streaming
      return 0;
   }

   return -1;
}

}

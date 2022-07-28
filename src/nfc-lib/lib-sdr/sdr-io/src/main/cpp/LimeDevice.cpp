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

// https://gist.github.com/csete/ece39ec9f66d751889821fa7619360e4

#include <queue>
#include <mutex>
#include <chrono>

#include <LimeSuite.h>

#include <rt/Logger.h>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>
#include <sdr/LimeDevice.h>

namespace sdr {

struct LimeDevice::Impl
{
   rt::Logger log {"LimeDevice"};

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
   int streamTime = 0;

   int limeResult = 0;
   lms_device_t *limeHandle = 0;

   std::mutex streamMutex;
   std::queue<SignalBuffer> streamQueue;
   RadioDevice::StreamHandler streamCallback;

   long samplesReceived = 0;
   long samplesDropped = 0;

   explicit Impl(std::string name) : deviceName(std::move(name))
   {
      log.debug("created LimeDevice for name [{}]", {this->deviceName});
   }

   ~Impl()
   {
      log.debug("destroy LimeDevice");

      close();
   }

   static std::vector<std::string> listDevices()
   {
      std::vector<std::string> result;

      lms_info_str_t devices[8];

      int count = LMS_GetDeviceList(devices);

      for (int i = 0; i < count; i++)
      {
         char buffer[256];

         snprintf(buffer, sizeof(buffer), "lime://%s", devices[i]);

         result.emplace_back(buffer);
      }

      return result;
   }

   bool open(SignalDevice::OpenMode mode)
   {
      lms_device_t *handle;

      if (deviceName.find("://") != -1 && deviceName.find("lime://") == -1)
      {
         log.warn("invalid device name [{}]", {deviceName});
         return false;
      }

      close();

      // extract device name
      std::string device = deviceName.substr(7);

      // open lime device
      if (LMS_Open(&handle, device.c_str(), NULL) == LMS_SUCCESS)
      {
         limeHandle = handle;
      }
//
//      if (airspyResult == AIRSPY_SUCCESS)
//      {
//         airspyHandle = handle;
//
//         char tmp[128];
//
//         // get version string
//         if ((airspyResult = airspy_version_string_read(handle, tmp, sizeof(tmp))) != AIRSPY_SUCCESS)
//            log.warn("failed airspy_version_string_read: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//
//         // disable bias tee
//         if ((airspyResult = airspy_set_rf_bias(handle, 0)) != AIRSPY_SUCCESS)
//            log.warn("failed airspy_set_rf_bias: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//
//         // read board serial
//         if ((airspyResult = airspy_board_partid_serialno_read(handle, &airspySerial)) != AIRSPY_SUCCESS)
//            log.warn("failed airspy_board_partid_serialno_read: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//
//         // set sample type
//         if ((airspyResult = airspy_set_sample_type(handle, airspySample)) != AIRSPY_SUCCESS)
//            log.warn("failed airspy_set_sample_type: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//
//         // set version string
//         deviceVersion = std::string(tmp);
//
//         // configure frequency
//         setCenterFreq(centerFreq);
//
//         // configure samplerate
//         setSampleRate(sampleRate);
//
//         // configure gain mode
//         setGainMode(gainMode);
//
//         // configure gain value
//         setGainValue(gainValue);
//
//         log.info("opened airspy device {}, firmware {}", {deviceName, deviceVersion});
//
//         return true;
//      }
//
//      log.warn("failed airspy_open_sn: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});

      return false;
   }

   void close()
   {
      if (limeHandle)
      {
         // stop streaming if active...
         stop();

         log.info("close device {}", {deviceName});

         // close device
         if ((limeResult = LMS_Close(limeHandle)) != LMS_SUCCESS)
            log.warn("failed LMS_Close: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

         deviceName = "";
         deviceVersion = "";
         limeHandle = nullptr;
      }
   }

   int start(RadioDevice::StreamHandler handler)
   {
//      if (airspyHandle)
//      {
//         log.info("start streaming for device {}", {deviceName});
//
//         // clear counters
//         samplesDropped = 0;
//         samplesReceived = 0;
//
//         // reset stream status
//         streamCallback = std::move(handler);
//         streamQueue = std::queue<SignalBuffer>();
//
//         // start reception
//         if ((airspyResult = airspy_start_rx(airspyHandle, reinterpret_cast<airspy_sample_block_cb_fn>(process_transfer), this)) != AIRSPY_SUCCESS)
//            log.warn("failed airspy_start_rx: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//
//         // clear callback to disable receiver
//         if (airspyResult != AIRSPY_SUCCESS)
//            streamCallback = nullptr;
//
//         // sets stream start time
//         streamTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//
//         return airspyResult;
//      }

      return -1;
   }

   int stop()
   {
//      if (airspyHandle && streamCallback)
//      {
//         log.info("stop streaming for device {}", {deviceName});
//
//         // stop reception
//         if ((airspyResult = airspy_stop_rx(airspyHandle)) != AIRSPY_SUCCESS)
//            log.warn("failed airspy_stop_rx: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//
//         // disable stream callback and queue
//         streamCallback = nullptr;
//         streamQueue = std::queue<SignalBuffer>();
//         streamTime = 0;
//
//         return airspyResult;
//      }

      return -1;
   }

   bool isOpen() const
   {
//      return airspyHandle;
      return false;
   }

   bool isEof() const
   {
//      return !airspyHandle || !airspy_is_streaming(airspyHandle);
      return true;
   }

   bool isReady() const
   {
//      char version[1];
//
//      return airspyHandle && airspy_version_string_read(airspyHandle, version, sizeof(version)) == AIRSPY_SUCCESS;

      return false;
   }

   bool isStreaming() const
   {
//      return airspyHandle && airspy_is_streaming(airspyHandle);

      return false;
   }

   int setCenterFreq(long value)
   {
//      centerFreq = value;
//
//      if (airspyHandle)
//      {
//         if ((airspyResult = airspy_set_freq(airspyHandle, centerFreq)) != AIRSPY_SUCCESS)
//            log.warn("failed airspy_set_freq: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//
//         return airspyResult;
//      }

      return 0;
   }

   int setSampleRate(long value)
   {
//      sampleRate = value;
//
//      if (airspyHandle)
//      {
//         if ((airspyResult = airspy_set_samplerate(airspyHandle, sampleRate)) != AIRSPY_SUCCESS)
//            log.warn("failed airspy_set_samplerate: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//
//         return airspyResult;
//      }

      return 0;
   }

   int setGainMode(int mode)
   {
//      gainMode = mode;
//
//      if (airspyHandle)
//      {
//         if (gainMode == LimeDevice::Auto)
//         {
//            if ((airspyResult = airspy_set_lna_agc(airspyHandle, tunerAgc)) != AIRSPY_SUCCESS)
//               log.warn("failed airspy_set_lna_agc: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//
//            if ((airspyResult = airspy_set_mixer_agc(airspyHandle, mixerAgc)) != AIRSPY_SUCCESS)
//               log.warn("failed airspy_set_mixer_agc: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//
//            return airspyResult;
//         }
//         else
//         {
//            return setGainValue(gainValue);
//         }
//      }

      return 0;
   }

   int setGainValue(int value)
   {
//      gainValue = value;
//
//      if (airspyHandle)
//      {
//         if (gainMode == LimeDevice::Linearity)
//         {
//            if ((airspyResult = airspy_set_linearity_gain(airspyHandle, gainValue)) != AIRSPY_SUCCESS)
//               log.warn("failed airspy_set_linearity_gain: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//         }
//         else if (gainMode == LimeDevice::Sensitivity)
//         {
//            if ((airspyResult = airspy_set_sensitivity_gain(airspyHandle, gainValue)) != AIRSPY_SUCCESS)
//               log.warn("failed airspy_set_linearity_gain: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//         }
//
//         return airspyResult;
//      }

      return 0;
   }

   int setTunerAgc(int value)
   {
//      tunerAgc = value;
//
//      if (tunerAgc)
//         gainMode = LimeDevice::Auto;
//
//      if (airspyHandle)
//      {
//         if ((airspyResult = airspy_set_lna_agc(airspyHandle, tunerAgc)) != AIRSPY_SUCCESS)
//            log.warn("failed airspy_set_lna_agc: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//
//         return airspyResult;
//      }

      return 0;
   }

   int setMixerAgc(int value)
   {
//      mixerAgc = value;
//
//      if (mixerAgc)
//         gainMode = LimeDevice::Auto;
//
//      if (airspyHandle)
//      {
//         if ((airspyResult = airspy_set_mixer_agc(airspyHandle, mixerAgc)) != AIRSPY_SUCCESS)
//            log.warn("failed airspy_set_mixer_agc: [{}] {}", {airspyResult, airspy_error_name((enum airspy_error) airspyResult)});
//
//         return airspyResult;
//      }

      return 0;
   }

   int setDecimation(int value)
   {
      decimation = value;

      return 0;
   }

   int setTestMode(int value)
   {
      log.warn("test mode not supported on this device!");

      return -1;
   }

   std::map<int, std::string> supportedSampleRates() const
   {
      std::map<int, std::string> result;

//      uint32_t count, rates[256];
//
//      if (airspyHandle)
//      {
//         // get number of supported sample rates
//         airspy_get_samplerates(airspyHandle, &count, 0);
//
//         // get list of supported sample rates
//         airspy_get_samplerates(airspyHandle, rates, count);
//
//         for (int i = 0; i < (int) count; i++)
//         {
//            char buffer[64];
//            int samplerate = rates[i];
//
//            snprintf(buffer, sizeof(buffer), "%d", samplerate);
//
//            result[samplerate] = buffer;
//         }
//      }

      return result;
   }

   std::map<int, std::string> supportedGainModes() const
   {
      std::map<int, std::string> result;

//      result[LimeDevice::Auto] = "Auto";
//      result[LimeDevice::Linearity] = "Linearity";
//      result[LimeDevice::Sensitivity] = "Sensitivity";

      return result;
   }

   std::map<int, std::string> supportedGainValues() const
   {
      std::map<int, std::string> result;

//      for (int i = 0; i < 22; i++)
//      {
//         char buffer[64];
//
//         snprintf(buffer, sizeof(buffer), "%d db", i);
//
//         result[i] = buffer;
//      }

      return result;
   }

   int read(SignalBuffer &buffer)
   {
//      // lock buffer access
//      std::lock_guard<std::mutex> lock(streamMutex);
//
//      if (!streamQueue.empty())
//      {
//         buffer = streamQueue.front();
//
//         streamQueue.pop();
//
//         return buffer.limit();
//      }

      return -1;
   }

   int write(SignalBuffer &buffer)
   {
      log.warn("write not supported on this device!");

      return -1;
   }

};

LimeDevice::LimeDevice(const std::string &name) : impl(std::make_shared<Impl>(name))
{
}

std::vector<std::string> LimeDevice::listDevices()
{
   return Impl::listDevices();
}

const std::string &LimeDevice::name()
{
   return impl->deviceName;
}

const std::string &LimeDevice::version()
{
   return impl->deviceVersion;
}

bool LimeDevice::open(OpenMode mode)
{
   return impl->open(mode);
}

void LimeDevice::close()
{
   impl->close();
}

int LimeDevice::start(StreamHandler handler)
{
   return impl->start(handler);
}

int LimeDevice::stop()
{
   return impl->stop();
}

bool LimeDevice::isOpen() const
{
   return impl->isOpen();
}

bool LimeDevice::isEof() const
{
   return impl->isEof();
}

bool LimeDevice::isReady() const
{
   return impl->isReady();
}

bool LimeDevice::isStreaming() const
{
   return impl->isStreaming();
}

int LimeDevice::sampleSize() const
{
   return impl->sampleSize;
}

int LimeDevice::setSampleSize(int value)
{
   impl->log.warn("setSampleSize has no effect!");

   return -1;
}

long LimeDevice::sampleRate() const
{
   return impl->sampleRate;
}

int LimeDevice::setSampleRate(long value)
{
   return impl->setSampleRate(value);
}

int LimeDevice::sampleType() const
{
   return Float;
}

int LimeDevice::setSampleType(int value)
{
   impl->log.warn("setSampleType has no effect!");

   return -1;
}

long LimeDevice::streamTime() const
{
   return impl->streamTime;
}

int LimeDevice::setStreamTime(long value)
{
   return 0;
}

long LimeDevice::centerFreq() const
{
   return impl->centerFreq;
}

int LimeDevice::setCenterFreq(long value)
{
   return impl->setCenterFreq(value);
}

int LimeDevice::tunerAgc() const
{
   return impl->tunerAgc;
}

int LimeDevice::setTunerAgc(int value)
{
   return impl->setTunerAgc(value);
}

int LimeDevice::mixerAgc() const
{
   return impl->mixerAgc;
}

int LimeDevice::setMixerAgc(int value)
{
   return impl->setMixerAgc(value);
}

int LimeDevice::gainMode() const
{
   return impl->gainMode;
}

int LimeDevice::setGainMode(int value)
{
   return impl->setGainMode(value);
}

int LimeDevice::gainValue() const
{
   return impl->gainValue;
}

int LimeDevice::setGainValue(int value)
{
   return impl->setGainValue(value);
}

int LimeDevice::decimation() const
{
   return impl->decimation;
}

int LimeDevice::setDecimation(int value)
{
   return impl->setDecimation(value);
}

int LimeDevice::testMode() const
{
   return 0;
}

int LimeDevice::setTestMode(int value)
{
   return impl->setTestMode(value);
}

long LimeDevice::samplesReceived()
{
   return impl->samplesReceived;
}

long LimeDevice::samplesDropped()
{
   return impl->samplesDropped;
}

std::map<int, std::string> LimeDevice::supportedSampleRates() const
{
   return impl->supportedSampleRates();
}

std::map<int, std::string> LimeDevice::supportedGainModes() const
{
   return impl->supportedGainModes();
}

std::map<int, std::string> LimeDevice::supportedGainValues() const
{
   return impl->supportedGainValues();
}

int LimeDevice::read(SignalBuffer &buffer)
{
   return impl->read(buffer);
}

int LimeDevice::write(SignalBuffer &buffer)
{
   return impl->write(buffer);
}

}
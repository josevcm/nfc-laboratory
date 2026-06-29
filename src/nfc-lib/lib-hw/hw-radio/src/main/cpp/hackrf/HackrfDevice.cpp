/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2026 Benjamin DELPY, <benjamin@gentilkiwi.com>
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
#include <cstdint>
#include <algorithm>

#include <hackrf.h>

#include <rt/Logger.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <hw/radio/HackrfDevice.h>

#define MAX_QUEUE_SIZE 4

#define LNA_GAIN_MAX 40
#define LNA_GAIN_STEP 8
#define LNA_GAIN_STEPS 6

#define VGA_GAIN_MAX 62
#define VGA_GAIN_STEP 2
#define VGA_GAIN_STEPS 32

#define DEVICE_TYPE_PREFIX "radio.hackrf"

namespace hw::radio {

int process_transfer(hackrf_transfer *transfer);

static bool hackrfInit()
{
   static int result = hackrf_init();

   return result == HACKRF_SUCCESS;
}

struct HackrfDevice::Impl
{
   rt::Logger *log = rt::Logger::getLogger("hw.HackrfDevice");

   std::string deviceName;
   std::string deviceSerial;
   std::string deviceVendor;
   std::string deviceModel;
   std::string deviceVersion;

   int fileDesc = 0;

   unsigned int centerFreq = 11.56E6; // offset tuning: NFC carrier (13.56) lands at +2 MHz IF, away from the zero-IF DC artifacts
   unsigned int sampleRate = 10E6;
   unsigned int sampleSize = 8;
   unsigned int sampleType = SAMPLE_TYPE_FLOAT;
   unsigned int gainMode = 1; // LNA/IF gain selector: step index (0..LNA_GAIN_STEPS-1) + 1
   unsigned int gainValue = 0; // VGA/baseband gain step index (dB = index * VGA_GAIN_STEP)
   unsigned int tunerAgc = 0; // mapped to the front-end RF amplifier (+14 dB)
   unsigned int mixerAgc = 0; // not supported by HackRF
   unsigned int biasTee = 0; // antenna port power (3.3V)
   unsigned int decimation = 0;
   unsigned int streamTime = 0;

   int hackrfResult = 0;
   hackrf_device *hackrfHandle = nullptr;

   std::mutex streamMutex;
   std::queue<SignalBuffer> streamQueue;
   StreamHandler streamCallback;
   bool streamPaused = false;

   unsigned long long samplesReceived = 0;
   unsigned long long samplesDropped = 0;

   explicit Impl(std::string name) : deviceName(std::move(name))
   {
      log->debug("created HackrfDevice for name [{}]", {this->deviceName});
   }

   explicit Impl(int fileDesc) : fileDesc(fileDesc)
   {
      log->debug("created HackrfDevice for file descriptor [{}]", {fileDesc});
   }

   ~Impl()
   {
      log->debug("destroy HackrfDevice [{}]", {deviceName});

      close();
   }

   static std::vector<std::string> enumerate()
   {
      std::vector<std::string> result;

      if (!hackrfInit())
         return result;

      hackrf_device_list_t *list = hackrf_device_list();

      if (list == nullptr)
         return result;

      for (int i = 0; i < list->devicecount; i++)
      {
         if (list->serial_numbers[i] == nullptr)
            continue;

         char buffer[256];

         snprintf(buffer, sizeof(buffer), "%s://%s", DEVICE_TYPE_PREFIX, list->serial_numbers[i]);

         result.emplace_back(buffer);
      }

      hackrf_device_list_free(list);

      return result;
   }

   bool open(Mode mode)
   {
      hackrf_device *handle = nullptr;

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

      if (!hackrfInit())
      {
         log->warn("failed hackrf_init");
         return false;
      }

      close();

      deviceSerial = deviceName.substr(strlen(DEVICE_TYPE_PREFIX) + 3);

      if ((hackrfResult = hackrf_open_by_serial(deviceSerial.c_str(), &handle)) == HACKRF_SUCCESS)
      {
         hackrfHandle = handle;

         char tmp[128];

         if ((hackrfResult = hackrf_version_string_read(handle, tmp, sizeof(tmp))) != HACKRF_SUCCESS)
            log->warn("failed hackrf_version_string_read: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});
         else
            deviceVersion = std::string(tmp);

         uint8_t boardId = BOARD_ID_UNDETECTED;

         if ((hackrfResult = hackrf_board_id_read(handle, &boardId)) != HACKRF_SUCCESS)
            log->warn("failed hackrf_board_id_read: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

         deviceVendor = "Great Scott Gadgets";
         deviceModel = hackrf_board_id_name(static_cast<hackrf_board_id>(boardId));

         setSampleRate(sampleRate);
         setCenterFreq(centerFreq);
         setTunerAgc(tunerAgc);
         setGainValue(gainValue);
         setBiasTee(biasTee);

         log->info("opened device {}, model {} firmware {}", {deviceName, deviceModel, deviceVersion});

         return true;
      }

      log->warn("failed hackrf_open_by_serial: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

      return false;
   }

   void close()
   {
      if (!hackrfHandle)
         return;

      stop();

      if ((hackrfResult = hackrf_set_antenna_enable(hackrfHandle, 0)) != HACKRF_SUCCESS)
         log->warn("failed hackrf_set_antenna_enable: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

      log->info("close device {}", {deviceName});

      if ((hackrfResult = hackrf_close(hackrfHandle)) != HACKRF_SUCCESS)
         log->warn("failed hackrf_close: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

      deviceName = "";
      deviceVersion = "";
      hackrfHandle = nullptr;
   }

   int start(StreamHandler handler)
   {
      if (!hackrfHandle)
         return -1;

      log->info("start streaming for device {}", {deviceName});

      samplesDropped = 0;
      samplesReceived = 0;

      streamPaused = false;
      streamCallback = std::move(handler);
      streamQueue = std::queue<SignalBuffer>();

      if ((hackrfResult = hackrf_start_rx(hackrfHandle, process_transfer, this)) != HACKRF_SUCCESS)
         log->warn("failed hackrf_start_rx: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

      if (hackrfResult != HACKRF_SUCCESS)
         streamCallback = nullptr;

      streamTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

      return hackrfResult;
   }

   int stop()
   {
      if (!hackrfHandle || !streamCallback)
         return 1;

      log->info("stop streaming for device {}", {deviceName});

      if ((hackrfResult = hackrf_stop_rx(hackrfHandle)) != HACKRF_SUCCESS)
         log->warn("failed hackrf_stop_rx: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

      streamPaused = false;
      streamCallback = nullptr;
      streamQueue = std::queue<SignalBuffer>();
      streamTime = 0;

      return hackrfResult;
   }

   int pause()
   {
      if (!hackrfHandle || !streamCallback)
         return 1;

      log->info("pause streaming for device {}", {deviceName});

      if ((hackrfResult = hackrf_stop_rx(hackrfHandle)) != HACKRF_SUCCESS)
         log->warn("failed hackrf_stop_rx: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

      streamPaused = hackrfResult == HACKRF_SUCCESS;

      return hackrfResult;
   }

   int resume()
   {
      if (!hackrfHandle || !streamCallback || !streamPaused)
         return -1;

      log->info("resume streaming for device {}", {deviceName});

      if ((hackrfResult = hackrf_start_rx(hackrfHandle, process_transfer, this)) != HACKRF_SUCCESS)
         log->warn("failed hackrf_start_rx: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

      if (hackrfResult != HACKRF_SUCCESS)
         streamCallback = nullptr;

      streamPaused = false;

      return hackrfResult;
   }

   bool isOpen() const
   {
      return hackrfHandle;
   }

   bool isEof() const
   {
      return !hackrfHandle || hackrf_is_streaming(hackrfHandle) != HACKRF_TRUE;
   }

   bool isReady() const
   {
      return hackrfHandle != nullptr;
   }

   bool isPaused() const
   {
      return hackrfHandle && streamPaused;
   }

   bool isStreaming() const
   {
      return hackrfHandle && hackrf_is_streaming(hackrfHandle) == HACKRF_TRUE;
   }

   int setCenterFreq(unsigned int value)
   {
      centerFreq = value;

      if (hackrfHandle)
      {
         if ((hackrfResult = hackrf_set_freq(hackrfHandle, centerFreq)) != HACKRF_SUCCESS)
            log->warn("failed hackrf_set_freq: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

         return hackrfResult;
      }

      return 0;
   }

   int setSampleRate(unsigned int value)
   {
      sampleRate = value;

      if (hackrfHandle)
      {
         if ((hackrfResult = hackrf_set_sample_rate(hackrfHandle, static_cast<double>(sampleRate))) != HACKRF_SUCCESS)
            log->warn("failed hackrf_set_sample_rate: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

         return hackrfResult;
      }

      return 0;
   }

   int applyGains()
   {
      if (!hackrfHandle)
         return 0;

      uint32_t lnaStep = gainMode > 0 ? gainMode - 1 : 0;
      uint32_t lnaGain = std::min<uint32_t>(lnaStep * LNA_GAIN_STEP, LNA_GAIN_MAX);
      uint32_t vgaGain = std::min<uint32_t>(gainValue * VGA_GAIN_STEP, VGA_GAIN_MAX);

      if ((hackrfResult = hackrf_set_lna_gain(hackrfHandle, lnaGain)) != HACKRF_SUCCESS)
         log->warn("failed hackrf_set_lna_gain: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

      if ((hackrfResult = hackrf_set_vga_gain(hackrfHandle, vgaGain)) != HACKRF_SUCCESS)
         log->warn("failed hackrf_set_vga_gain: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

      log->debug("gains applied: amp {}, lna {} db, vga {} db", {tunerAgc, lnaGain, vgaGain});

      return hackrfResult;
   }

   int setGainMode(unsigned int mode)
   {
      gainMode = mode;

      return applyGains();
   }

   int setGainValue(unsigned int value)
   {
      gainValue = value;

      return applyGains();
   }

   int setTunerAgc(unsigned int value)
   {
      tunerAgc = value;

      if (hackrfHandle)
      {
         if ((hackrfResult = hackrf_set_amp_enable(hackrfHandle, tunerAgc ? 1 : 0)) != HACKRF_SUCCESS)
            log->warn("failed hackrf_set_amp_enable: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

         return hackrfResult;
      }

      return 0;
   }

   int setMixerAgc(unsigned int value)
   {
      mixerAgc = value;

      return 0;
   }

   int setBiasTee(unsigned int value)
   {
      biasTee = value;

      if (hackrfHandle)
      {
         if ((hackrfResult = hackrf_set_antenna_enable(hackrfHandle, biasTee ? 1 : 0)) != HACKRF_SUCCESS)
            log->warn("failed hackrf_set_antenna_enable: [{}] {}", {hackrfResult, hackrf_error_name(static_cast<hackrf_error>(hackrfResult))});

         return hackrfResult;
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

      result[2000000] = "2000000";
      result[4000000] = "4000000";
      result[5000000] = "5000000";
      result[8000000] = "8000000";
      result[10000000] = "10000000";
      result[12500000] = "12500000";
      result[16000000] = "16000000";
      result[20000000] = "20000000";

      return result;
   }

   rt::Catalog supportedGainModes() const
   {
      rt::Catalog result;

      for (int i = 0; i < LNA_GAIN_STEPS; i++)
      {
         char buffer[64];

         snprintf(buffer, sizeof(buffer), "LNA %d db", i * LNA_GAIN_STEP);

         result[i + 1] = buffer;
      }

      return result;
   }

   rt::Catalog supportedGainValues() const
   {
      rt::Catalog result;

      for (int i = 0; i < VGA_GAIN_STEPS; i++)
      {
         char buffer[64];

         snprintf(buffer, sizeof(buffer), "VGA %d db", i * VGA_GAIN_STEP);

         result[i] = buffer;
      }

      return result;
   }

   long read(SignalBuffer &buffer)
   {
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

HackrfDevice::HackrfDevice(const std::string &name) : impl(std::make_shared<Impl>(name))
{
}

HackrfDevice::HackrfDevice(int fd) : impl(std::make_shared<Impl>(fd))
{
}

std::vector<std::string> HackrfDevice::enumerate()
{
   return Impl::enumerate();
}

bool HackrfDevice::open(Mode mode)
{
   return impl->open(mode);
}

void HackrfDevice::close()
{
   impl->close();
}

int HackrfDevice::start(StreamHandler handler)
{
   return impl->start(handler);
}

int HackrfDevice::stop()
{
   return impl->stop();
}

int HackrfDevice::pause()
{
   return impl->pause();
}

int HackrfDevice::resume()
{
   return impl->resume();
}

rt::Variant HackrfDevice::get(int id, int channel) const
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

bool HackrfDevice::set(int id, const rt::Variant &value, int channel)
{
   switch (id)
   {
      case PARAM_SAMPLE_RATE:
      {
         if (const auto v = std::get_if<unsigned int>(&value))
            return impl->setSampleRate(*v);

         impl->log->error("invalid value type for PARAM_SAMPLE_RATE");
         return false;
      }
      case PARAM_TUNE_FREQUENCY:
      {
         if (const auto v = std::get_if<unsigned int>(&value))
            return impl->setCenterFreq(*v);

         impl->log->error("invalid value type for PARAM_TUNE_FREQUENCY");
         return false;
      }
      case PARAM_TUNER_AGC:
      {
         if (const auto v = std::get_if<unsigned int>(&value))
            return impl->setTunerAgc(*v);

         impl->log->error("invalid value type for PARAM_TUNER_AGC");
         return false;
      }
      case PARAM_MIXER_AGC:
      {
         if (const auto v = std::get_if<unsigned int>(&value))
            return impl->setMixerAgc(*v);

         impl->log->error("invalid value type for PARAM_MIXER_AGC");
         return false;
      }
      case PARAM_GAIN_MODE:
      {
         if (const auto v = std::get_if<unsigned int>(&value))
            return impl->setGainMode(*v);

         impl->log->error("invalid value type for PARAM_GAIN_MODE");
         return false;
      }
      case PARAM_GAIN_VALUE:
      {
         if (const auto v = std::get_if<unsigned int>(&value))
            return impl->setGainValue(*v);

         impl->log->error("invalid value type for PARAM_GAIN_VALUE");
         return false;
      }
      case PARAM_BIAS_TEE:
      {
         if (const auto v = std::get_if<unsigned int>(&value))
            return impl->setBiasTee(*v);

         impl->log->error("invalid value type for PARAM_BIAS_TEE");
         return false;
      }
      case PARAM_DECIMATION:
      {
         if (const auto v = std::get_if<unsigned int>(&value))
            return impl->setDecimation(*v);

         impl->log->error("invalid value type for PARAM_DECIMATION");
         return false;
      }
      case PARAM_DIRECT_SAMPLING:
      {
         return true; // avoid warning
      }
      default:
         impl->log->warn("unknown or unsupported configuration id {}", {id});
         return false;
   }
}

bool HackrfDevice::isOpen() const
{
   return impl->isOpen();
}

bool HackrfDevice::isEof() const
{
   return impl->isEof();
}

bool HackrfDevice::isReady() const
{
   return impl->isReady();
}

bool HackrfDevice::isPaused() const
{
   return impl->isPaused();
}

bool HackrfDevice::isStreaming() const
{
   return impl->isStreaming();
}

long HackrfDevice::read(SignalBuffer &buffer)
{
   return impl->read(buffer);
}

long HackrfDevice::write(const SignalBuffer &buffer)
{
   return impl->write(buffer);
}

int process_transfer(hackrf_transfer *transfer)
{
   if (auto *device = static_cast<HackrfDevice::Impl *>(transfer->rx_ctx))
   {
      const int count = transfer->valid_length;

      SignalBuffer buffer(count, 2, 1, device->sampleRate, device->samplesReceived, 0, SignalType::SIGNAL_TYPE_RADIO_IQ);

      const auto *src = reinterpret_cast<const int8_t *>(transfer->buffer);
      float *dst = buffer.push(count);

#pragma GCC ivdep
      for (int i = 0; i < count; i++)
         dst[i] = static_cast<float>(src[i]) / 128.0f;

      buffer.flip();

      device->samplesReceived += count / 2;

      if (device->streamCallback)
      {
         device->streamCallback(buffer);
      }
      else
      {
         std::lock_guard lock(device->streamMutex);

         if (device->streamQueue.size() >= MAX_QUEUE_SIZE)
         {
            device->samplesDropped += device->streamQueue.front().elements();
            device->streamQueue.pop();
         }

         device->streamQueue.push(buffer);
      }

      return 0;
   }

   return -1;
}

}

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

#ifdef _WIN32

#include <windows.h>

#undef ERROR
#endif

#include <atomic>
#include <queue>
#include <mutex>
#include <thread>

#include <rtl-sdr.h>

#include <rt/Logger.h>

#include <hw/DeviceFactory.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <hw/radio/RealtekDevice.h>

#define READER_SAMPLES 2048
#define BUFFER_SAMPLES 65536

#define MAX_QUEUE_SIZE 4

#define DEVICE_TYPE_PREFIX "radio.rtlsdr"

typedef struct rtlsdr_dev *rtldev;

namespace hw::radio {

struct RealtekDevice::Impl
{
   rt::Logger *log = rt::Logger::getLogger("hw.RealtekDevice");

   std::string deviceName;
   std::string deviceVendor;
   std::string deviceModel;
   std::string deviceSerial;
   std::string deviceVersion;
   int fileDesc = 0;
   unsigned int centerFreq = 27.12E6;
   unsigned int sampleRate = 3.2E6;
   unsigned int sampleSize = 16;
   unsigned int sampleType = SAMPLE_TYPE_FLOAT;
   unsigned int gainMode = 0;
   unsigned int gainValue = 0;
   unsigned int tunerAgc = 0;
   unsigned int mixerAgc = 0;
   unsigned int biasTee = 0;
   unsigned int decimation = 0;
   unsigned int testMode = 0;
   unsigned int streamTime = 0;
   unsigned int directSampling = 0;

   int rtlsdrResult = 0;
   rtlsdr_dev *rtlsdrHandle = nullptr;
   rtlsdr_tuner rtlsdrTuner;

   std::mutex workerMutex;
   std::atomic_bool workerPaused {false};
   std::atomic_bool workerRunning {false};
   std::thread workerThread;

   std::mutex streamMutex;
   std::queue<SignalBuffer> streamQueue;
   StreamHandler streamCallback;

   long long samplesReceived = 0;
   long long samplesDropped = 0;

   explicit Impl(std::string name) : deviceName(std::move(name))
   {
      log->debug("created RealtekDevice for name [{}]", {this->deviceName});
   }

   explicit Impl(int fileDesc) : fileDesc(fileDesc)
   {
      log->debug("created RealtekDevice for file descriptor [{}]", {fileDesc});
   }

   ~Impl()
   {
      log->debug("destroy RealtekDevice [{}]", {deviceName});

      // close underline device
      close();

   }

   static std::vector<std::string> enumerate()
   {
      std::vector<std::string> result;

      const unsigned int count = rtlsdr_get_device_count();

      for (int i = 0; i < count; i++)
      {
         char manufact[1024];
         char product[1024];
         char serial[1024];
         char buffer[2048];

         if (rtlsdr_get_device_usb_strings(i, manufact, product, serial) == 0)
         {
            snprintf(buffer, sizeof(buffer), "%s://%s", DEVICE_TYPE_PREFIX, serial);

            result.emplace_back(buffer);
         }
      }

      return result;
   }

   bool open(Mode mode)
   {
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

      int index;

      // extract serial number from device name
      auto serial = deviceName.substr(strlen(DEVICE_TYPE_PREFIX) + 3);

      // find device index
      if ((index = rtlsdr_get_index_by_serial(serial.c_str())) < 0)
         log->error("failed rtlsdr_get_index_by_serial: [{}]", {rtlsdrResult});

      if (index >= 0)
      {
         rtlsdr_dev *device = nullptr;

         if ((rtlsdrResult = rtlsdr_open(&device, static_cast<unsigned int>(index))) == 0)
         {
            rtlsdrHandle = device;

            // read tuner type
            rtlsdrTuner = rtlsdr_get_tuner_type(rtlsdrHandle);

            // set default tuner bandwidth
            if ((rtlsdrResult = rtlsdr_set_tuner_bandwidth(rtlsdrHandle, 0)) < 0)
               log->warn("failed rtlsdr_set_tuner_bandwidth: [{}]", {rtlsdrResult});

            while (true)
            {
               // disable test mode
               if (setTestMode(testMode) != 0)
                  break;

               // configure direct sampling
               if (setDirectSampling(directSampling) != 0)
                  break;

               // configure frequency
               if (setCenterFreq(centerFreq) != 0)
                  break;

               // configure samplerate
               if (setSampleRate(sampleRate) != 0)
                  break;

               // configure mixer automatic gain control
               if (setMixerAgc(mixerAgc) != 0)
                  break;

               // configure tuner automatic gain control
               if (setTunerAgc(tunerAgc) != 0)
                  break;

               // configure gain mode
               if (setGainMode(gainMode) != 0)
                  break;

               // configure gain value
               if (setGainValue(gainValue) != 0)
                  break;

               // fill device info
               deviceVendor = "Generic";
               deviceModel = "RTLSDR";
               deviceSerial = std::string(serial);

               log->info("opened rtlsdr device {} width tuner type {}", {deviceName, rtlsdrTuner});

               return true;
            }

            // close underline device
            if ((rtlsdrResult = rtlsdr_close(rtlsdrHandle)) < 0)
               log->warn("failed rtlsdr_close: [{}]", {rtlsdrResult});

            deviceName = "";
            deviceVersion = "";
            rtlsdrHandle = nullptr;
         }
      }

      return false;
   }

   void close()
   {
      if (!rtlsdrHandle)
         return;

      // stop streaming if active...
      stop();

      log->info("close device {}", {deviceName});

      // close underline device
      if ((rtlsdrResult = rtlsdr_close(rtlsdrHandle)) < 0)
         log->warn("failed rtlsdr_close: [{}]", {rtlsdrResult});

      deviceName = "";
      deviceVersion = "";
      rtlsdrHandle = nullptr;
   }

   int start(StreamHandler handler)
   {
      if (!rtlsdrHandle)
         return -1;

      // delay worker start until method is finished
      std::lock_guard lock(workerMutex);

      // clear counters
      samplesDropped = 0;
      samplesReceived = 0;

      // reset stream status
      streamCallback = std::move(handler);
      streamQueue = std::queue<SignalBuffer>();

      // reset buffer to start streaming
      if ((rtlsdrResult = rtlsdr_reset_buffer(rtlsdrHandle)) < 0)
         log->warn("failed rtlsdr_reset_buffer: [{}] {}", {rtlsdrResult});

      // enable worker thread on success
      if (rtlsdrResult == 0)
      {
         log->info("start streaming for device {}", {deviceName});

         // set streaming flag to enable worker
         workerRunning = true;
         workerPaused = false;

         // run worker thread
         workerThread = std::thread([this] { streamWorker(); });
      }

      // sets stream start time
      streamTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

      return rtlsdrResult;
   }

   int stop()
   {
      if (!rtlsdrHandle || !workerRunning)
         return -1;

      log->info("stop streaming for device {}", {deviceName});

      // signal finish to running thread
      workerRunning = false;
      workerPaused = false;

      // wait until worker is finished
      std::lock_guard lock(workerMutex);

      // wait for thread joint
      workerThread.join();

      // disable stream callback and queue
      streamCallback = nullptr;
      streamQueue = std::queue<SignalBuffer>();
      streamTime = 0;

      return 0;
   }

   int pause()
   {
      if (!rtlsdrHandle || !workerRunning)
         return 1;

      log->info("pause streaming for device {}", {deviceName});

      // set paused state
      workerPaused = true;

      return 0;
   }

   int resume()
   {
      if (!rtlsdrHandle || !workerRunning || !workerPaused)
         return -1;

      log->info("resume streaming for device {}", {deviceName});

      // reset buffer to start streaming
      if ((rtlsdrResult = rtlsdr_reset_buffer(rtlsdrHandle)) < 0)
         log->warn("failed rtlsdr_reset_buffer: [{}] {}", {rtlsdrResult});

      workerPaused = false;

      return 0;
   }

   bool isOpen() const
   {
      return rtlsdrHandle;
   }

   bool isEof() const
   {
      return !rtlsdrHandle || !workerRunning;
   }

   bool isReady() const
   {
      return rtlsdrHandle;
   }

   bool isPaused() const
   {
      return rtlsdrHandle && workerPaused;
   }

   bool isStreaming() const
   {
      return rtlsdrHandle && workerRunning && !workerPaused;
   }

   int setCenterFreq(unsigned int value)
   {
      centerFreq = value;

      if (!rtlsdrHandle)
         return 0;

      log->debug("rtlsdr_set_center_freq({})", {centerFreq});

      if ((rtlsdrResult = rtlsdr_set_center_freq(rtlsdrHandle, centerFreq)) < 0)
         log->warn("failed rtlsdr_set_center_freq: [{}]", {rtlsdrResult});

      return rtlsdrResult;
   }

   int setSampleRate(unsigned int value)
   {
      sampleRate = value;

      if (!rtlsdrHandle)
         return 0;

      log->debug("rtlsdr_set_sample_rate({})", {sampleRate});

      if ((rtlsdrResult = rtlsdr_set_sample_rate(rtlsdrHandle, sampleRate)) < 0)
         log->warn("failed rtlsdr_set_sample_rate: [{}]", {rtlsdrResult});

      return rtlsdrResult;
   }

   int setGainMode(unsigned int mode)
   {
      gainMode = mode;

      if (!rtlsdrHandle)
         return 0;

      if (gainMode == Auto)
      {
         log->debug("rtlsdr_set_tuner_gain_mode({})", {0});

         // set automatic gain
         if ((rtlsdrResult = rtlsdr_set_tuner_gain_mode(rtlsdrHandle, 0)) < 0)
            log->warn("failed rtlsdr_set_tuner_gain_mode: [{}]", {rtlsdrResult});

         return rtlsdrResult;
      }

      log->debug("rtlsdr_set_tuner_gain_mode({})", {1});

      // set manual gain
      if ((rtlsdrResult = rtlsdr_set_tuner_gain_mode(rtlsdrHandle, 1)) < 0)
         log->warn("failed rtlsdr_set_tuner_gain: [{}]", {rtlsdrResult});

      return setGainValue(gainValue);
   }

   int setGainValue(unsigned int value)
   {
      gainValue = value;

      if (!rtlsdrHandle)
         return 0;

      if (gainMode == RealtekDevice::Manual)
      {
         log->debug("rtlsdr_set_tuner_gain({})", {gainValue});

         if ((rtlsdrResult = rtlsdr_set_tuner_gain(rtlsdrHandle, gainValue)) < 0)
            log->warn("failed rtlsdr_set_tuner_gain: [{}]", {rtlsdrResult});
      }

      return rtlsdrResult;
   }

   int setTunerAgc(unsigned int value)
   {
      tunerAgc = value;

      if (tunerAgc)
         gainMode = RealtekDevice::Auto;

      if (!rtlsdrHandle)
         return 0;

      log->debug("rtlsdr_set_tuner_gain_mode({})", {tunerAgc ? 0 : 1});

      if ((rtlsdrResult = rtlsdr_set_tuner_gain_mode(rtlsdrHandle, tunerAgc ? 0 : 1)) < 0)
         log->warn("failed rtlsdr_set_tuner_gain_mode: [{}]", {rtlsdrResult});

      return rtlsdrResult;
   }

   int setMixerAgc(unsigned int value)
   {
      mixerAgc = value;

      if (mixerAgc)
         gainMode = Auto;

      if (!rtlsdrHandle)
         return 0;

      log->debug("rtlsdr_set_agc_mode({})", {mixerAgc});

      if ((rtlsdrResult = rtlsdr_set_agc_mode(rtlsdrHandle, mixerAgc)) < 0)
         log->warn("failed rtlsdr_set_agc_mode: [{}]", {rtlsdrResult});

      return rtlsdrResult;
   }

   int setDecimation(unsigned int value)
   {
      decimation = value;

      //      if (rtlsdrHandle)
      //      {
      //         log->debug("rtlsdr_set_agc_mode({})", {mixerAgc});
      //
      //         if ((rtlsdrResult = rtlsdr_set_agc_mode(rtlsdrHandle, mixerAgc)) < 0)
      //            log->warn("failed rtlsdr_set_agc_mode: [{}]", {rtlsdrResult});
      //
      //         return rtlsdrResult;
      //      }

      return -1;
   }

   int setTestMode(unsigned int value)
   {
      testMode = value;

      if (!rtlsdrHandle)
         return 0;

      log->debug("rtlsdr_set_testmode({})", {testMode});

      if ((rtlsdrResult = rtlsdr_set_testmode(rtlsdrHandle, testMode)) < 0)
         log->warn("failed rtlsdr_set_testmode: [{}]", {rtlsdrResult});

      return rtlsdrResult;
   }

   int setDirectSampling(unsigned int value)
   {
      directSampling = value;

      if (!rtlsdrHandle)
         return 0;

      log->debug("rtlsdr_set_direct_sampling({})", {directSampling});

      if ((rtlsdrResult = rtlsdr_set_direct_sampling(rtlsdrHandle, directSampling)) < 0)
         log->warn("failed rtlsdr_set_direct_sampling: [{}]", {rtlsdrResult});

      return rtlsdrResult;
   }

   rt::Catalog supportedSampleRates() const
   {
      rt::Catalog result;

      result[225000] = "225000"; // 0.25 MSPS
      result[900000] = "900000"; // 0.90 MSPS
      result[1024000] = "1024000"; // 1.024 MSPS
      result[1400000] = "1400000"; // 1.4 MSPS
      result[1800000] = "1800000"; // 1.8 MSPS
      result[1920000] = "1920000"; // 1.92 MSPS
      result[2048000] = "2048000"; // 2.048 MSPS
      result[2400000] = "2400000"; // 2.4 MSPS
      result[2560000] = "2560000"; // 2.56 MSPS
      result[2800000] = "2800000"; // 2.8 MSPS
      result[3200000] = "3200000"; // 3.2 MSPS

      return result;
   }

   rt::Catalog supportedGainModes() const
   {
      rt::Catalog result;

      result[RealtekDevice::Auto] = "Auto";
      result[RealtekDevice::Manual] = "Manual";

      return result;
   }

   rt::Catalog supportedGainValues() const
   {
      if (!rtlsdrHandle)
         return {};

      rt::Catalog result;

      int count = rtlsdr_get_tuner_gains(rtlsdrHandle, nullptr);

      if (count > 0)
      {
         int values[1024];

         rtlsdr_get_tuner_gains(rtlsdrHandle, values);

         for (int i = 0; i < count; i++)
         {
            char buffer[64];

            snprintf(buffer, sizeof(buffer), "%.2f db", static_cast<float>(values[i]) / 10.0f);

            result[values[i]] = buffer;
         }
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

   void streamWorker()
   {
      float scaled[READER_SAMPLES * 2];
      unsigned char data[READER_SAMPLES * 2];

#ifdef _WIN32
      SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#else
      sched_param param {20};
      pthread_setschedparam(pthread_self(), SCHED_RR, &param);
#endif

      std::lock_guard workerLock(workerMutex);

      log->info("stream worker started for device {}", {deviceName});

      while (workerRunning)
      {
         if (workerPaused)
         {
            // wait until worker is resumed
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
         }

         int length;

         SignalBuffer buffer = SignalBuffer(BUFFER_SAMPLES * 2, 2, 1, sampleRate, samplesReceived, 0, SIGNAL_TYPE_RADIO_IQ);

         while (buffer.remaining() > READER_SAMPLES && (rtlsdr_read_sync(rtlsdrHandle, data, sizeof(data), &length) == 0))
         {
            int dropped = sizeof(data) - length;

#pragma GCC ivdep
            for (int i = 0; i < length; i += 4)
            {
               scaled[i + 0] = static_cast<float>((data[i + 0] - 128) / 256.0) + 0.0025f;
               scaled[i + 1] = static_cast<float>((data[i + 1] - 128) / 256.0) + 0.0025f;
               scaled[i + 2] = static_cast<float>((data[i + 2] - 128) / 256.0) + 0.0025f;
               scaled[i + 3] = static_cast<float>((data[i + 3] - 128) / 256.0) + 0.0025f;
            }

            // add data to buffer
            buffer.put(scaled, length);

            // update counters
            samplesReceived += length >> 1;
            samplesDropped += dropped >> 1;

            // trace dropped samples
            if (dropped > 0)
               log->warn("dropped samples {}", {samplesDropped});
         }

         // flip buffer contents
         buffer.flip();

         // stream to buffer callback
         if (streamCallback)
         {
            streamCallback(buffer);
         }

         // or store buffer in receive queue
         else
         {
            // lock buffer access
            std::lock_guard streamLock(streamMutex);

            // discard oldest buffers
            if (streamQueue.size() >= MAX_QUEUE_SIZE)
            {
               samplesDropped += streamQueue.front().elements();
               streamQueue.pop();
            }

            // queue new sample buffer
            streamQueue.push(buffer);
         }
      }

      log->info("stream worker finished for device {}", {deviceName});
   }
};

RealtekDevice::RealtekDevice(const std::string &name) : impl(std::make_shared<Impl>(name))
{
}

RealtekDevice::RealtekDevice(int fd) : impl(std::make_shared<Impl>(fd))
{
}

std::vector<std::string> RealtekDevice::enumerate()
{
   return Impl::enumerate();
}

bool RealtekDevice::open(Mode mode)
{
   return impl->open(mode);
}

void RealtekDevice::close()
{
   impl->close();
}

int RealtekDevice::start(StreamHandler handler)
{
   return impl->start(handler);
}

int RealtekDevice::stop()
{
   return impl->stop();
}

int RealtekDevice::pause()
{
   return impl->pause();
}

int RealtekDevice::resume()
{
   return impl->resume();
}

rt::Variant RealtekDevice::get(int id, int channel) const
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
         return impl->testMode;

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
         return impl->directSampling;

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

bool RealtekDevice::set(int id, const rt::Variant &value, int channel)
{
   switch (id)
   {
      case PARAM_TEST_MODE:
      {
         if (const auto v = std::get_if<unsigned int>(&value))
            return impl->setTestMode(*v);

         impl->log->error("invalid value type for PARAM_TEST_MODE");
         return false;
      }
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
      case PARAM_DECIMATION:
      {
         if (const auto v = std::get_if<unsigned int>(&value))
            return impl->setDecimation(*v);

         impl->log->error("invalid value type for PARAM_DECIMATION");
         return false;
      }
      default:
         impl->log->warn("unknown or unsupported configuration id {}", {id});
         return false;
   }
}

bool RealtekDevice::isOpen() const
{
   return impl->isOpen();
}

bool RealtekDevice::isEof() const
{
   return impl->isEof();
}

bool RealtekDevice::isReady() const
{
   return impl->isReady();
}

bool RealtekDevice::isPaused() const
{
   return impl->isPaused();
}

bool RealtekDevice::isStreaming() const
{
   return impl->isStreaming();
}

long RealtekDevice::read(SignalBuffer &buffer)
{
   return impl->read(buffer);
}

long RealtekDevice::write(const SignalBuffer &buffer)
{
   return impl->write(buffer);
}
}

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

#ifdef _WIN32

#include <windows.h>

#undef ERROR
#endif

#include <queue>
#include <mutex>
#include <chrono>

#include <LimeSuite.h>

#include <rt/Logger.h>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>
#include <sdr/LimeDevice.h>

#define STREAM_SAMPLES 1024*1024
#define BUFFER_SAMPLES 65536
#define MAX_QUEUE_SIZE 4

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
   int testMode = 0;

   int limeResult = 0;
   lms_device_t *limeHandle = 0;
   lms_stream_t limeStream = {};

   std::mutex workerMutex;
   std::atomic_bool workerStreaming {false};
   std::thread workerThread;

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
      log.info("open device {}", {deviceName});

      lms_device_t *handle;
      const lms_dev_info_t *info;

      if (deviceName.find("://") != -1 && deviceName.find("lime://") == -1)
      {
         log.warn("invalid device name [{}]", {deviceName});
         return false;
      }

      close();

      // extract device name
      std::string device = deviceName.substr(7);

      // open lime device
      if ((limeResult = LMS_Open(&handle, device.c_str(), nullptr)) == LMS_SUCCESS)
      {
         limeHandle = handle;

         if ((info = LMS_GetDeviceInfo(limeHandle)) != nullptr)
         {
            log.info("firmware version {}", {std::string(info->firmwareVersion)});
            log.info("hardware version {}", {std::string(info->hardwareVersion)});
            log.info("protocol version {}", {std::string(info->protocolVersion)});
            log.info("gateware version {}", {std::string(info->gatewareVersion)});
         }

//         // Reset device configuration
//         if ((limeResult = LMS_Reset(limeHandle)) != LMS_SUCCESS)
//            log.warn("failed LMS_Reset: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

         // Initialize device with default configuration, Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI
         if ((limeResult = LMS_Init(limeHandle)) != LMS_SUCCESS)
            log.warn("failed LMS_Init: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

         // enable RX channel
         if ((limeResult = LMS_EnableChannel(limeHandle, LMS_CH_RX, 0, true)) != LMS_SUCCESS)
            log.warn("failed LMS_EnableChannel: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

         // configure frequency
         setCenterFreq(centerFreq);

         // configure samplerate
         setSampleRate(sampleRate);

         // configure gain mode
         setGainMode(gainMode);

         // configure gain value
         setGainValue(gainValue);

         return true;
      }

      log.warn("failed LMS_Open: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

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
      if (limeHandle)
      {
         log.info("start streaming for device {}", {deviceName});

         // reset result
         limeResult = LMS_SUCCESS;

         // clear counters
         samplesDropped = 0;
         samplesReceived = 0;

         // reset stream status
         streamCallback = std::move(handler);
         streamQueue = std::queue<SignalBuffer>();

         // setup stream
         limeStream.channel = 0;
         limeStream.isTx = false;
         limeStream.fifoSize = STREAM_SAMPLES;
         limeStream.throughputVsLatency = 1.0;
         limeStream.dataFmt = lms_stream_t::LMS_FMT_F32;

         if (!limeResult && (limeResult = LMS_SetupStream(limeHandle, &limeStream)) != LMS_SUCCESS)
            log.warn("failed LMS_SetupStream: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

         // start reception
         if (!limeResult && (limeResult = LMS_StartStream(&limeStream)) != LMS_SUCCESS)
            log.warn("failed LMS_StartStream: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

         // enable worker thread on success
         if (!limeResult)
         {
            // set streaming flag to enable worker
            workerStreaming = true;

            // run worker thread
            workerThread = std::thread([this] { streamWorker(); });
         }

         // sets stream start time
         streamTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

         return limeResult;
      }

      return -1;
   }

   int stop()
   {
      if (limeHandle && streamCallback)
      {
         // reset result
         limeResult = LMS_SUCCESS;

         log.info("stop streaming for device {}", {deviceName});

         // signal finish to running thread
         workerStreaming = false;

         // wait until worker is finished
         std::lock_guard<std::mutex> lock(workerMutex);

         // wait for thread joint
         workerThread.join();

         // stop reception
         if (!limeResult && (limeResult = LMS_StopStream(&limeStream)) != LMS_SUCCESS)
            log.warn("failed LMS_StopStream: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

         // release stream
         if (!limeResult && (limeResult = LMS_DestroyStream(limeHandle, &limeStream)) != LMS_SUCCESS)
            log.warn("failed LMS_DestroyStream: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

         // disable stream callback and queue
         streamCallback = nullptr;
         streamQueue = std::queue<SignalBuffer>();
         streamTime = 0;

         return limeResult;
      }

      return -1;
   }

   bool isOpen() const
   {
      return limeHandle;
   }

   bool isEof() const
   {
      return !limeHandle || !workerStreaming;
   }

   bool isReady() const
   {
      return limeHandle;
   }

   bool isStreaming() const
   {
      return workerStreaming;
   }

   int setCenterFreq(long value)
   {
      centerFreq = value;

      if (limeHandle)
      {
         if ((limeResult = LMS_SetLOFrequency(limeHandle, LMS_CH_RX, 0, centerFreq)) != LMS_SUCCESS)
            log.warn("failed LMS_SetLOFrequency: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

         return limeResult;
      }

      return 0;
   }

   int setSampleRate(long value)
   {
      sampleRate = value;

      if (limeHandle)
      {
         if ((limeResult = LMS_SetSampleRate(limeHandle, sampleRate, 0)) != LMS_SUCCESS)
            log.warn("failed LMS_SetSampleRate: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

         return limeResult;
      }

      return 0;
   }

   int setGainMode(int mode)
   {
      return 0;
   }

   int setGainValue(int value)
   {
      gainValue = value;

      if (limeHandle)
      {
         if ((limeResult = LMS_SetGaindB(limeHandle, LMS_CH_RX, 0, gainValue)) != LMS_SUCCESS)
            log.warn("failed LMS_SetGaindB: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

         return limeResult;
      }

      return 0;
   }

   int setTunerAgc(int value)
   {
      return 0;
   }

   int setMixerAgc(int value)
   {
      return 0;
   }

   int setDecimation(int value)
   {
      decimation = value;

      return 0;
   }

   int setTestMode(int value)
   {
      testMode = value;

      if (limeHandle)
      {
         if ((limeResult = LMS_SetTestSignal(limeHandle, LMS_CH_RX, 0, testMode ? LMS_TESTSIG_NCODIV8 : LMS_TESTSIG_NONE, 0, 0)) != LMS_SUCCESS)
            log.warn("failed LMS_SetTestSignal: [{}] {}", {limeResult, LMS_GetLastErrorMessage()});

         return limeResult;
      }

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

   void streamWorker()
   {
      int received;

#ifdef _WIN32
      SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#elif
      sched_param param {20};

      if (pthread_setschedparam(workerThread.native_handle(), SCHED_MAX, &param))
         log.warn("error setting logger thread priority: {}", {std::strerror(errno)});
#endif

      std::lock_guard<std::mutex> lock(workerMutex);

      log.info("stream worker started for device {}", {deviceName});

      int readTimeout = int(1E4 * float(BUFFER_SAMPLES) / sampleRate); // 10 * sample buffer in ms

      while (workerStreaming)
      {
         SignalBuffer buffer = SignalBuffer(BUFFER_SAMPLES * 2, 2, sampleRate, samplesReceived, 0, SignalType::SAMPLE_IQ);

         if ((received = LMS_RecvStream(&limeStream, buffer.data(), BUFFER_SAMPLES, nullptr, readTimeout)) > 0)
         {
            buffer.pull(received * 2);

            // flip buffer contents
            buffer.flip();

            // update counters
            samplesReceived += received;

            // stream to buffer callback
            if (streamCallback)
            {
               streamCallback(buffer);
            }

               // or store buffer in receive queue
            else
            {
               // lock buffer access
               std::lock_guard<std::mutex> lock(streamMutex);

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
         else
         {
            log.warn("read timeout");
         }
      }

      log.info("stream worker finished for device {}", {deviceName});
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
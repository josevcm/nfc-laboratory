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

#ifdef _WIN32

#include <windows.h>

#undef ERROR
#endif

#include <queue>
#include <mutex>
#include <thread>

#include <rtl-sdr.h>

#include <rt/Logger.h>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>
#include <sdr/RealtekDevice.h>

#define READER_SAMPLES 2048
#define BUFFER_SAMPLES 65536

#define MAX_QUEUE_SIZE 4

typedef struct rtlsdr_dev *rtldev;

namespace sdr {

struct RealtekDevice::Impl
{
   rt::Logger log {"RealtekDevice"};

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
   int testMode = 0;
   int streamTime = 0;

   int rtlsdrResult = 0;
   rtlsdr_dev *rtlsdrHandle {};
   rtlsdr_tuner rtlsdrTuner;

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
      log.debug("created RealtekDevice for name [{}]", {this->deviceName});
   }

   explicit Impl(int fileDesc) : fileDesc(fileDesc)
   {
      log.debug("created RealtekDevice for file descriptor [{}]", {fileDesc});
   }

   ~Impl()
   {
      log.debug("destroy RealtekDevice");

      // close underline device
      close();

   }

   static std::vector<std::string> listDevices()
   {
      std::vector<std::string> result;

      int count = rtlsdr_get_device_count();

      for (int i = 0; i < count; i++)
      {
         char manufact[1024];
         char product[1024];
         char serial[1024];
         char buffer[1024];

         if (rtlsdr_get_device_usb_strings(i, manufact, product, serial) == 0)
         {
            snprintf(buffer, sizeof(buffer), "rtlsdr://%s", serial);

            result.emplace_back(buffer);
         }
      }

      return result;
   }

   bool open(SignalDevice::OpenMode mode)
   {
      log.info("open device device {}", {deviceName});

      if (deviceName.find("://") != -1 && deviceName.find("rtlsdr://") == -1)
      {
         log.warn("invalid device name [{}]", {deviceName});
         return false;
      }

      close();

      int index;

      // extract serial number from device name
      auto serial = deviceName.substr(9);

      // find device index
      if ((index = rtlsdr_get_index_by_serial(serial.c_str())) < 0)
         log.error("failed rtlsdr_get_index_by_serial: [{}]", {rtlsdrResult});

      if (index >= 0)
      {
         rtlsdr_dev *device;

         if ((rtlsdrResult = rtlsdr_open((rtldev *) &device, (unsigned int) index)) == 0)
         {
            rtlsdrHandle = device;

            // read tuner type
            rtlsdrTuner = rtlsdr_get_tuner_type(rtldev(rtlsdrHandle));

            // set default tuner bandwidth
            if ((rtlsdrResult = rtlsdr_set_tuner_bandwidth(rtldev(rtlsdrHandle), 0)) < 0)
               log.warn("failed rtlsdr_set_tuner_bandwidth: [{}]", {rtlsdrResult});

            // disable test mode
            setTestMode(testMode);

            // configure frequency
            setCenterFreq(centerFreq);

            // configure samplerate
            setSampleRate(sampleRate);

            // configure gain mode
            setGainMode(gainMode);

            // configure gain value
            setGainValue(gainValue);

            log.info("tuner type {}", {rtlsdrTuner});

            return true;
         }
      }

      return false;
   }

   void close()
   {
      if (rtlsdrHandle)
      {
         // stop streaming if active...
         stop();

         log.info("close device {}", {deviceName});

         // close underline device
         if ((rtlsdrResult = rtlsdr_close(rtldev(rtlsdrHandle))) < 0)
            log.warn("failed rtlsdr_close: [{}]", {rtlsdrResult});

         deviceName = "";
         deviceVersion = "";
         rtlsdrHandle = nullptr;
      }
   }

   int start(RadioDevice::StreamHandler handler)
   {
      if (rtlsdrHandle)
      {
         log.info("start streaming for device {}", {deviceName});

         // delay worker start until method is finished
         std::lock_guard<std::mutex> lock(workerMutex);

         // clear counters
         samplesDropped = 0;
         samplesReceived = 0;

         // reset stream status
         streamCallback = std::move(handler);
         streamQueue = std::queue<SignalBuffer>();

         // reset buffer to start streaming
         if ((rtlsdrResult = rtlsdr_reset_buffer(rtldev(rtlsdrHandle))) < 0)
            log.warn("failed rtlsdr_reset_buffer: [{}] {}", {rtlsdrResult});

         // enable worker thread on success
         if (rtlsdrResult == 0)
         {
            // set streaming flag to enable worker
            workerStreaming = true;

            // run worker thread
            workerThread = std::thread([this] { streamWorker(); });
         }

         // sets stream start time
         streamTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

         return rtlsdrResult;
      }

      return -1;
   }

   int stop()
   {
      if (rtlsdrHandle && workerStreaming)
      {
         log.info("stop streaming for device {}", {deviceName});

         // signal finish to running thread
         workerStreaming = false;

         // wait until worker is finished
         std::lock_guard<std::mutex> lock(workerMutex);

         // wait for thread joint
         workerThread.join();

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
      return rtlsdrHandle;
   }

   bool isEof() const
   {
      return !rtlsdrHandle || !workerStreaming;
   }

   bool isReady() const
   {
      return rtlsdrHandle;
   }

   bool isStreaming() const
   {
      return workerStreaming;
   }

   int setCenterFreq(long value)
   {
      centerFreq = value;

      if (rtlsdrHandle)
      {
         log.debug("rtlsdr_set_center_freq({})", {centerFreq});

         if ((rtlsdrResult = rtlsdr_set_center_freq(rtldev(rtlsdrHandle), centerFreq)) < 0)
            log.warn("failed rtlsdr_set_center_freq: [{}]", {rtlsdrResult});

         return rtlsdrResult;
      }

      return 0;
   }

   int setSampleRate(long value)
   {
      sampleRate = value;

      if (rtlsdrHandle)
      {
         log.debug("rtlsdr_set_sample_rate({})", {sampleRate});

         if ((rtlsdrResult = rtlsdr_set_sample_rate(rtldev(rtlsdrHandle), sampleRate)) < 0)
            log.warn("failed rtlsdr_set_sample_rate: [{}]", {rtlsdrResult});

         return rtlsdrResult;
      }

      return 0;
   }

   int setGainMode(int mode)
   {
      gainMode = mode;

      if (rtlsdrHandle)
      {
         if (gainMode == RealtekDevice::Auto)
         {
            log.debug("rtlsdr_set_tuner_gain_mode({})", {0});

            // set automatic gain
            if ((rtlsdrResult = rtlsdr_set_tuner_gain_mode(rtldev(rtlsdrHandle), 0)) < 0)
               log.warn("failed rtlsdr_set_tuner_gain_mode: [{}]", {rtlsdrResult});

            return rtlsdrResult;
         }
         else
         {
            log.debug("rtlsdr_set_tuner_gain_mode({})", {1});

            // set manual gain
            if ((rtlsdrResult = rtlsdr_set_tuner_gain_mode(rtldev(rtlsdrHandle), 1)) < 0)
               log.warn("failed rtlsdr_set_tuner_gain: [{}]", {rtlsdrResult});

            return setGainValue(gainValue);
         }
      }

      return 0;
   }

   int setGainValue(int value)
   {
      gainValue = value;

      if (rtlsdrHandle)
      {
         if (gainMode == RealtekDevice::Manual)
         {
            log.debug("rtlsdr_set_tuner_gain({})", {gainValue});

            if ((rtlsdrResult = rtlsdr_set_tuner_gain(rtldev(rtlsdrHandle), gainValue)) < 0)
               log.warn("failed rtlsdr_set_tuner_gain: [{}]", {rtlsdrResult});
         }

         return rtlsdrResult;
      }

      return 0;
   }

   int setTunerAgc(int value)
   {
      tunerAgc = value;

      if (tunerAgc)
         gainMode = RealtekDevice::Auto;

      if (rtlsdrHandle)
      {
         log.debug("rtlsdr_set_tuner_gain_mode({})", {!tunerAgc});

         if ((rtlsdrResult = rtlsdr_set_tuner_gain_mode(rtldev(rtlsdrHandle), !tunerAgc)) < 0)
            log.warn("failed rtlsdr_set_tuner_gain_mode: [{}]", {rtlsdrResult});

         return rtlsdrResult;
      }

      return 0;
   }

   int setMixerAgc(int value)
   {
      mixerAgc = value;

      if (mixerAgc)
         gainMode = RealtekDevice::Auto;

      if (rtlsdrHandle)
      {
         log.debug("rtlsdr_set_tuner_gain_mode({})", {mixerAgc});

         if ((rtlsdrResult = rtlsdr_set_agc_mode(rtldev(rtlsdrHandle), mixerAgc)) < 0)
            log.warn("failed rtlsdr_set_agc_mode: [{}]", {rtlsdrResult});

         return rtlsdrResult;
      }

      return 0;
   }

   int setDecimation(int value)
   {
      decimation = value;

//      if (rtlsdrHandle)
//      {
//         log.debug("rtlsdr_set_agc_mode({})", {mixerAgc});
//
//         if ((rtlsdrResult = rtlsdr_set_agc_mode(rtldev(rtlsdrHandle), mixerAgc)) < 0)
//            log.warn("failed rtlsdr_set_agc_mode: [{}]", {rtlsdrResult});
//
//         return rtlsdrResult;
//      }

      return -1;
   }

   int setTestMode(int value)
   {
      testMode = value;

      if (rtlsdrHandle)
      {
         log.debug("rtlsdr_set_testmode({})", {testMode});

         if ((rtlsdrResult = rtlsdr_set_testmode(rtldev(rtlsdrHandle), testMode)) < 0)
            log.warn("failed rtlsdr_set_testmode: [{}]", {rtlsdrResult});

         return rtlsdrResult;
      }

      return 0;
   }

   std::map<int, std::string> supportedSampleRates() const
   {
      std::map<int, std::string> result;

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

   std::map<int, std::string> supportedGainModes() const
   {
      std::map<int, std::string> result;

      result[RealtekDevice::Auto] = "Auto";
      result[RealtekDevice::Manual] = "Manual";

      return result;
   }

   std::map<int, std::string> supportedGainValues() const
   {
      std::map<int, std::string> result;

      if (rtlsdrHandle)
      {
         int count = rtlsdr_get_tuner_gains(rtldev(rtlsdrHandle), nullptr);

         if (count > 0)
         {
            int values[1024];

            rtlsdr_get_tuner_gains(rtldev(rtlsdrHandle), values);

            for (int i = 0; i < count; i++)
            {
               char buffer[64];

               snprintf(buffer, sizeof(buffer), "%.2f db", float(values[i]) / 10.0f);

               result[values[i]] = buffer;
            }
         }
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

   void streamWorker()
   {
      float scaled[READER_SAMPLES * 2];
      unsigned char data[READER_SAMPLES * 2];

#ifdef _WIN32
      SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#elif
      sched_param param {20};

      if (pthread_setschedparam(workerThread.native_handle(), SCHED_MAX, &param))
         log.warn("error setting logger thread priority: {}", {std::strerror(errno)});
#endif

      std::lock_guard<std::mutex> lock(workerMutex);

      log.info("stream worker started for device {}", {deviceName});

      while (workerStreaming)
      {
         int length;

         SignalBuffer buffer = SignalBuffer(BUFFER_SAMPLES * 2, 2, sampleRate, samplesReceived, 0, SignalType::SAMPLE_IQ);

         while (buffer.available() > READER_SAMPLES && (rtlsdr_read_sync(rtldev(rtlsdrHandle), data, sizeof(data), &length) == 0))
         {
            int dropped = sizeof(data) - length;

#pragma GCC ivdep
            for (int i = 0; i < length; i += 4)
            {
               scaled[i + 0] = float((data[i + 0] - 128) / 256.0) + 0.0025f;
               scaled[i + 1] = float((data[i + 1] - 128) / 256.0) + 0.0025f;
               scaled[i + 2] = float((data[i + 2] - 128) / 256.0) + 0.0025f;
               scaled[i + 3] = float((data[i + 3] - 128) / 256.0) + 0.0025f;
            }

            // add data to buffer
            buffer.put(scaled, length);

            // update counters
            samplesReceived += length >> 1;
            samplesDropped += dropped >> 1;

            // trace dropped samples
            if (dropped > 0)
               log.warn("dropped samples {}", {samplesDropped});
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

      log.info("stream worker finished for device {}", {deviceName});
   }
};

RealtekDevice::RealtekDevice(const std::string &name) : impl(std::make_shared<Impl>(name))
{
}

RealtekDevice::RealtekDevice(int fd) : impl(std::make_shared<Impl>(fd))
{
}

std::vector<std::string> RealtekDevice::listDevices()
{
   return Impl::listDevices();
}

const std::string &RealtekDevice::name()
{
   return impl->deviceName;
}

const std::string &RealtekDevice::version()
{
   return impl->deviceVersion;
}

bool RealtekDevice::open(OpenMode mode)
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

bool RealtekDevice::isStreaming() const
{
   return impl->isStreaming();
}

int RealtekDevice::sampleSize() const
{
   return impl->sampleSize;
}

int RealtekDevice::setSampleSize(int value)
{
   impl->log.warn("setSampleSize has no effect!");

   return -1;
}

long RealtekDevice::sampleRate() const
{
   return impl->sampleRate;
}

int RealtekDevice::setSampleRate(long value)
{
   return impl->setSampleRate(value);
}

int RealtekDevice::sampleType() const
{
   return Float;
}

int RealtekDevice::setSampleType(int value)
{
   impl->log.warn("setSampleType has no effect!");

   return -1;
}

long RealtekDevice::streamTime() const
{
   return 0;
}

int RealtekDevice::setStreamTime(long value)
{
   return 0;
}

long RealtekDevice::centerFreq() const
{
   return impl->centerFreq;
}

int RealtekDevice::setCenterFreq(long value)
{
   return impl->setCenterFreq(value);
}

int RealtekDevice::tunerAgc() const
{
   return impl->tunerAgc;
}

int RealtekDevice::setTunerAgc(int value)
{
   return impl->setTunerAgc(value);
}

int RealtekDevice::mixerAgc() const
{
   return impl->mixerAgc;
}

int RealtekDevice::setMixerAgc(int value)
{
   return impl->setMixerAgc(value);
}

int RealtekDevice::gainMode() const
{
   return impl->gainMode;
}

int RealtekDevice::setGainMode(int value)
{
   return impl->setGainMode(value);
}

int RealtekDevice::gainValue() const
{
   return impl->gainValue;
}

int RealtekDevice::setGainValue(int value)
{
   return impl->setGainValue(value);
}

int RealtekDevice::decimation() const
{
   return impl->decimation;
}

int RealtekDevice::setDecimation(int value)
{
   return impl->setDecimation(value);
}

int RealtekDevice::testMode() const
{
   return impl->testMode;
}

int RealtekDevice::setTestMode(int value)
{
   return impl->setTestMode(value);
}

long RealtekDevice::samplesReceived()
{
   return impl->samplesReceived;
}

long RealtekDevice::samplesDropped()
{
   return impl->samplesDropped;
}

std::map<int, std::string> RealtekDevice::supportedSampleRates() const
{
   return impl->supportedSampleRates();
}

std::map<int, std::string> RealtekDevice::supportedGainModes() const
{
   return impl->supportedGainModes();
}

std::map<int, std::string> RealtekDevice::supportedGainValues() const
{
   return impl->supportedGainValues();
}

int RealtekDevice::read(SignalBuffer &buffer)
{
   return impl->read(buffer);
}

int RealtekDevice::write(SignalBuffer &buffer)
{
   return impl->write(buffer);
}
}
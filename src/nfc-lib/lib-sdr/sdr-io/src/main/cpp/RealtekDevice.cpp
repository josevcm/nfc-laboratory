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

#include <rtl-sdr.h>

#include <rt/Logger.h>

#include <sdr/SignalBuffer.h>
#include <sdr/RealtekDevice.h>

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

   int rtlsdrResult = 0;
   rtlsdr_dev *rtlsdrHandle {};
   bool rtlsdrStreaming = false;

   std::mutex streamMutex;
   std::queue<SignalBuffer> streamQueue;
   RadioDevice::StreamHandler streamCallback;

   long samplesReceived = 0;
   long samplesDropped = 0;
   long samplesStreamed = 0;

   explicit Impl(const std::string &name) : deviceName(name)
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
      if (deviceName.find("://") != -1 && deviceName.find("rtlsdr://") == -1)
      {
         log.warn("invalid device name [{}]", {deviceName});
         return false;
      }

      close();

      int index = -1;

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

            // disable test mode
            if ((rtlsdrResult = rtlsdr_set_testmode(rtldev(rtlsdrHandle), 0)) < 0)
               log.warn("failed rtlsdr_set_testmode: [{}]", {rtlsdrResult});

            // configure frequency
            setCenterFreq(centerFreq);

            // configure samplerate
            setSampleRate(sampleRate);

            // configure gain mode
            setGainMode(gainMode);

            // configure gain value
            setGainValue(gainValue);

            log.info("openned rtlsdr device {}", {deviceName});

            return true;
         }
      }

//   int index = listDevices().indexOf(name);
//
//   // open RTL device
//   if ((result = rtlsdr_open((rtldev *) &device, (unsigned int) index)) == 0)
//   {
//      // start prefetch worker
//      QThreadPool::globalInstance()->start(new TaskRunner<RealtekDevice>(this, &RealtekDevice::prefetch), QThread::HighPriority);
//   }
//   else
//   {
//      qWarning() << "failed rtlsdr_open:" << result;
//      return false;
//   }
//
//   return QIODevice::open(mode);
//
//   return true;

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

         // clear counters
         samplesDropped = 0;
         samplesReceived = 0;
         samplesStreamed = 0;
         streamCallback = std::move(handler);
         streamQueue = std::queue<SignalBuffer>();

         // reset buffer to start streaming
         if ((rtlsdrResult = rtlsdr_reset_buffer(rtldev(rtlsdrHandle))) < 0)
            log.warn("failed rtlsdr_reset_buffer: [{}] {}", {rtlsdrResult});

         return rtlsdrResult;
      }

      return -1;
   }

   int stop()
   {
      return 0;
   }

   bool isOpen() const
   {
      return rtlsdrHandle;
   }

   bool isEof() const
   {
      return !rtlsdrHandle || !rtlsdrStreaming;
   }

   bool isReady() const
   {
      return rtlsdrHandle;
   }

   bool isStreaming() const
   {
      return rtlsdrStreaming;
   }

   int setCenterFreq(long value)
   {
      centerFreq = value;

      if (rtlsdrHandle)
      {
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
            // set automatic gain
            if ((rtlsdrResult = rtlsdr_set_tuner_gain_mode(rtldev(rtlsdrHandle), 0)) < 0)
               log.warn("failed rtlsdr_set_tuner_gain_mode: [{}]", {rtlsdrResult});

            return rtlsdrResult;
         }
         else
         {
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
         if ((rtlsdrResult = rtlsdr_set_agc_mode(rtldev(rtlsdrHandle), mixerAgc)) < 0)
            log.warn("failed rtlsdr_set_agc_mode: [{}]", {rtlsdrResult});

         return rtlsdrResult;
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

               result[i] = buffer;
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

long RealtekDevice::samplesReceived()
{
   return impl->samplesReceived;
}

long RealtekDevice::samplesDropped()
{
   return impl->samplesDropped;
}

long RealtekDevice::samplesStreamed()
{
   return impl->samplesStreamed;
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

//int RealtekDevice::start(StreamHandler handler)
//{
//   int result = -1;
//
//   if (device->handle)
//   {
//      device->log.info("start streaming for device %s", device->name.c_str());
//
//      // setup stream callback
//      device->streamCallback = std::move(handler);
//
//      // start reception
//      if ((result = airspy_start_rx(device->handle, reinterpret_cast<airspy_sample_block_cb_fn>(process_transfer), device)) != AIRSPY_SUCCESS)
//         device->log.warn("failed airspy_start_rx: [%d] %s", result, airspy_error_name((enum airspy_error) result));
//   }
//
//   return result;
//}

//int RealtekDevice::stop()
//{
//   int result = -1;
//
//   if (device->handle)
//   {
//      device->log.info("stop stream %s", device->name.c_str());
//
//      // stop reception
//      if ((result = airspy_stop_rx(device->handle)) != AIRSPY_SUCCESS)
//         device->log.warn("failed airspy_stop_rx: [%d] %s", result, airspy_error_name((enum airspy_error) result));
//
//      // disable stream callback
//      device->streamCallback = nullptr;
//   }
//
//   return result;
//}

//int RealtekDevice::setSampleRate(long value)
//{
//   int result;
//
//   sampleRate = sampleRate;
//
//   if (mDevice)
//   {
//      if ((result = rtlsdr_set_sample_rate(rtldev(mDevice), sampleRate)) < 0)
//         qWarning() << "failed rtlsdr_set_sample_rate(" << sampleRate << ")" << result;
//   }
//
//   return -1;
//}


//int RealtekDevice::setTunerAgc(int value)
//{
//   int result;
//
//   mGainMode = gainMode;
//
//   if (mDevice)
//   {
//      if (gainMode & TUNER_AUTO)
//      {
//         qInfo() << "enable tuner AGC";
//
//         // set automatic gain
//         if ((result = rtlsdr_set_tuner_gain_mode(rtldev(mDevice), 0)) < 0)
//            qWarning() << "failed rtlsdr_set_tuner_gain_mode(" << 0 << ")" << result;
//      }
//      else if (mTunerGain != -1)
//      {
//         qInfo() << "set tuner gain to" << mTunerGain << "db";
//
//         // set manual gain
//         if ((result = rtlsdr_set_tuner_gain_mode(rtldev(mDevice), 1)) < 0)
//            qWarning() << "failed rtlsdr_set_tuner_gain_mode(" << 1 << ")" << result;
//
//         if ((result = rtlsdr_set_tuner_gain(rtldev(mDevice), mTunerGain)) < 0)
//            qWarning() << "failed rtlsdr_set_tuner_gain(" << mTunerGain << ")" << result;
//      }
//
//      if (gainMode & DIGITAL_AUTO)
//      {
//         qInfo() << "enable digital AGC";
//
//         // enable digital AGC
//         if ((result = rtlsdr_set_agc_mode(rtldev(mDevice), 1)) < 0)
//            qWarning() << "failed rtlsdr_set_agc_mode(" << 1 << ")" << result;
//      }
//      else
//      {
//         qInfo() << "disable digital AGC";
//
//         // disable digital AGC
//         if ((result = rtlsdr_set_agc_mode(rtldev(mDevice), 0)) < 0)
//            qWarning() << "failed rtlsdr_set_agc_mode(" << 0 << ")" << result;
//      }
//   }
//
//   return 0;
//}

//
//int RealtekDevice::setMixerAgc(int value)
//{
//   mTunerGain = tunerGain;
//
//   if (mDevice)
//   {
//      int gain = tunerGain * 10;
//
//      if (rtlsdr_set_tuner_gain(rtldev(mDevice), gain) < 0)
//      {
//         qWarning() << "rtlsdr_set_tuner_gain failed!";
//      }
//   }
//
//   return 0;
//}

//void RealtekDevice::prefetch()
//{
//   static int prefetchCount1 = 0;
//   static int prefetchCount2 = 0;
//
//   int stored;
//   int readed;
//   char data[4096];
//
//   QMutexLocker lock(&mWorker);
//
//   qWarning() << "starting realtek prefetch thread";
//
//   while (isOpen())
//   {
//      if (rtlsdr_read_sync(rtldev(mDevice), data, sizeof(data), &readed) < 0)
//         qWarning() << "failed rtlsdr_read_sync!";
//      else if (readed != sizeof(data))
//         qWarning() << "short read, samples lost!";
//
//      prefetchCount1++;
//
//      if (prefetchCount1 - prefetchCount2 == 100)
//      {
//         qInfo().noquote() << "prefetch," << prefetchCount1 << "blocks," << mReceivedSamples << "samples," << "buffer load" << QString("%1 %").arg(100 * double(mBufferLoad) / double(BUFFER_SIZE), 1, 'f', 2);
//         prefetchCount2 = prefetchCount1;
//      }
//
//      if ((stored = mBufferLoad) < BUFFER_SIZE && readed > 0)
//      {
//         unsigned int blocks = 0;
//         unsigned int length = readed >> 1;
//
//         unsigned char *src = (unsigned char *) data;
//         float *dst = mBufferHead;
//         int i, q;
//
//         while (blocks < length && stored < BUFFER_SIZE)
//         {
//            i = *src++;
//            q = *src++;
//
//            *dst++ = float((i - 128) / 128.0);
//            *dst++ = float((q - 128) / 128.0);
//
//            if (dst >= mBufferLimit)
//               dst = mBufferData;
//
//            blocks++;
//            stored++;
//         }
//
//         // update sample counter
//         mReceivedSamples += blocks;
//
//         // update buffer status
//         mBufferHead = dst;
//         mBufferLoad.fetchAndAddOrdered(blocks);
//      }
//      else
//      {
//         qWarning() << "buffer full, samples lost!";
//      }
//   }
//
//   qWarning() << "terminate realtek prefetch";
//}
}
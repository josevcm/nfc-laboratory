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

   std::string name {};
   int fileDesc {};
   long frequency {};
   long sampleRate {};
   int gainMode {};
   int gainValue {};
   int tunerAgc {};
   int mixerAgc {};

   rtlsdr_dev *handle {};

   std::mutex streamMutex;
   std::queue<SignalBuffer> streamQueue;
   RadioDevice::StreamHandler streamCallback;

   long samplesReceived {};
   long samplesDropped {};
   long samplesStreamed {};

   explicit Impl(const std::string &name) : name(name)
   {
      log.debug("created RealtekDevice for name [{}]", {name});
   }

   explicit Impl(int fileDesc) : fileDesc(fileDesc)
   {
      log.debug("created RealtekDevice for file descriptor [{}]", {fileDesc});
   }

   ~Impl()
   {
      log.debug("destroy RealtekDevice");
   }

   static std::vector<std::string> listDevices()
   {
      std::vector<std::string> result;

      int count = rtlsdr_get_device_count();

      for (int i = 0; i < count; i++)
      {
         char buffer[256];

         snprintf(buffer, sizeof(buffer), "rtlsdr://%s", rtlsdr_get_device_name(i));

         result.emplace_back(buffer);
      }

      return result;
   }

   bool isOpen() const
   {
      return handle;
   }

   bool isEof() const
   {
      return !handle;
   }

   bool isStreaming() const
   {
      return false;
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
   return impl->name;
}

bool RealtekDevice::open(SignalDevice::OpenMode mode)
{
//   int result;
//   void *device;
//
//   close();
//
//   // check device name
//   if (!name.startsWith("rtlsdr://"))
//   {
//      qWarning() << "invalid device name" << name;
//      return false;
//   }
//
//   int index = listDevices().indexOf(name);
//
//   // open RTL device
//   if ((result = rtlsdr_open((rtldev *) &device, (unsigned int) index)) == 0)
//   {
//      // configure frequency
//      if (mCenterFrequency != -1)
//      {
//         qInfo() << "set frequency to" << mCenterFrequency << "Hz";
//
//         if ((result = rtlsdr_set_center_freq(rtldev(device), mCenterFrequency)) < 0)
//            qWarning() << "failed rtlsdr_set_center_freq:" << result;
//      }
//
//      // configure sample rate
//      if (sampleRate != -1)
//      {
//         qInfo() << "set samplerate to" << sampleRate;
//
//         if ((result = rtlsdr_set_sample_rate(rtldev(device), sampleRate)) < 0)
//            qWarning() << "failed rtlsdr_set_sample_rate:" << result;
//      }
//
//      // configure tuner gain
//      if (mGainMode & TUNER_AUTO)
//      {
//         qInfo() << "set tuner gain to AUTO";
//
//         // set automatic gain
//         if ((result = rtlsdr_set_tuner_gain_mode(rtldev(device), 0)) < 0)
//            qWarning() << "failed rtlsdr_set_tuner_gain_mode:" << result;
//      }
//      else if (mTunerGain != -1)
//      {
//         qInfo() << "set tuner gain to" << mTunerGain << "db";
//
//         // set manual gain
//         if ((result = rtlsdr_set_tuner_gain_mode(rtldev(device), 1)) < 0)
//            qWarning() << "failed rtlsdr_set_tuner_gain_mode:" << result;
//
//         if ((result = rtlsdr_set_tuner_gain(rtldev(device), mTunerGain)) < 0)
//            qWarning() << "failed rtlsdr_set_tuner_gain:" << result;
//      }
//
//      // set RTL AGC mode
//      if (mGainMode & DIGITAL_AUTO)
//      {
//         qInfo() << "enable digital AGC";
//
//         if ((result = rtlsdr_set_agc_mode(rtldev(device), 1)) < 0)
//            qWarning() << "failed rtlsdr_set_agc_mode:" << result;
//      }
//      else
//      {
//         qInfo() << "disable digital AGC";
//
//         if ((result = rtlsdr_set_agc_mode(rtldev(device), 0)) < 0)
//            qWarning() << "failed rtlsdr_set_agc_mode:" << result;
//      }
//
//      // enable test mode
////      if ((result = rtlsdr_set_testmode(rtldev(device), 1)) < 0)
////         qWarning() << "failed rtlsdr_set_testmode:" << result;
//
//      // reset buffer to start streaming
//      if ((result = rtlsdr_reset_buffer(rtldev(device))) < 0)
//         qWarning() << "failed rtlsdr_reset_buffer:" << result;
//
//      // setup device data
//      mName = name;
//      mDevice = device;
//
//      // reset buffer status
//      mBufferLoad = 0;
//      mBufferTail = mBufferData;
//      mBufferHead = mBufferData;
//      mBufferLimit = mBufferData + BUFFER_SIZE;
//
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

   return true;
}

void RealtekDevice::close()
{
//   QIODevice::close();
//
//   if (mDevice)
//   {
//      QMutexLocker lock(&mWorker);
//
//      rtlsdr_close(rtldev(mDevice));
//
//      mDevice = 0;
//      mName = "";
//   }
}

int RealtekDevice::start(StreamHandler handler)
{
   int result = -1;

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

   return result;
}

int RealtekDevice::stop()
{
   int result = -1;

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

   return result;
}

bool RealtekDevice::isOpen() const
{
   return impl->isOpen();
}

bool RealtekDevice::isEof() const
{
   return impl->isEof();
}

bool RealtekDevice::isStreaming() const
{
   return impl->isStreaming();
}

long RealtekDevice::centerFreq() const
{
   return impl->frequency;
}

int RealtekDevice::setCenterFreq(long value)
{
   //   int result;
   //
   //   mCenterFrequency = frequency;
   //
   //   if (mDevice)
   //   {
   //      if ((result = rtlsdr_set_center_freq(rtldev(mDevice), frequency)) < 0)
   //         qWarning() << "failed rtlsdr_set_center_freq(" << frequency << ")" << result;
   //   }

   return -1;
}

int RealtekDevice::sampleSize() const
{
   return -1;
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
//   int result;
//
//   sampleRate = sampleRate;
//
//   if (mDevice)
//   {
//      if ((result = rtlsdr_set_sample_rate(rtldev(mDevice), sampleRate)) < 0)
//         qWarning() << "failed rtlsdr_set_sample_rate(" << sampleRate << ")" << result;
//   }

   return -1;
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

int RealtekDevice::tunerAgc() const
{
   return impl->tunerAgc;
}

int RealtekDevice::setTunerAgc(int value)
{
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

   return 0;
}

int RealtekDevice::mixerAgc() const
{
   return impl->mixerAgc;
}

int RealtekDevice::setMixerAgc(int value)
{
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

   return 0;
}

int RealtekDevice::gainMode() const
{
   return impl->gainMode;
}

int RealtekDevice::setGainMode(int mode)
{
   int result = 0;

   return result;
}

int RealtekDevice::gainValue() const
{
   return impl->gainValue;
}

int RealtekDevice::setGainValue(int value)
{
   int result = 0;


   return result;
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

std::map<int, std::string> RealtekDevice::supportedGainValues() const
{
   std::map<int, std::string> result;

//   if (mDevice)
//   {
//      int count = rtlsdr_get_tuner_gains(rtldev(mDevice), NULL);
//
//      if (count > 0)
//      {
//         int buffer[1024];
//
//         rtlsdr_get_tuner_gains(rtldev(mDevice), buffer);
//
//         for (int i = 0; i < count; i++)
//         {
//            result.append(buffer[i] / 10.0f);
//         }
//      }
//   }

   return result;
}

std::map<int, std::string> RealtekDevice::supportedGainModes() const
{
   std::map<int, std::string> result;

//   if (mDevice)
//   {
//      int count = rtlsdr_get_tuner_gains(rtldev(mDevice), NULL);
//
//      if (count > 0)
//      {
//         int buffer[1024];
//
//         rtlsdr_get_tuner_gains(rtldev(mDevice), buffer);
//
//         for (int i = 0; i < count; i++)
//         {
//            result.append(buffer[i] / 10.0f);
//         }
//      }
//   }

   return result;
}

int RealtekDevice::read(SignalBuffer &buffer)
{
//   if (!isOpen())
//      return -1;
//
//   while (signal.available() > 0)
//   {
//      int blocks = 0;
//      int length = mBufferLoad;
//      float v[2], *src = mBufferTail;
//
//      while (blocks < length && signal.available() > 0)
//      {
//         v[0] = *src++;
//         v[1] = *src++;
//
//         if (src >= mBufferLimit)
//            src = mBufferData;
//
//         signal.put(v);
//
//         blocks++;
//      }
//
//      mBufferTail = src;
//      mBufferLoad.fetchAndAddOrdered(-blocks);
//   }
//
//   signal.flip();
//
//   return signal.limit();

   return 0;
}

int RealtekDevice::write(SignalBuffer &buffer)
{
   impl->log.warn("write not supported on this device!");

   return -1;
}

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
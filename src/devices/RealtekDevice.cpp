/*

  Copyright (c) 2020 Jose Vicente Campos Martinez - <josevcm@gmail.com>

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

#include <QDebug>
#include <QThread>
#include <QThreadPool>
#include <QElapsedTimer>

#include <support/rtlsdr/rtl-sdr.h>

#include "RealtekDevice.h"

#define BUFFER_SIZE (1024 * 1024)

typedef struct rtlsdr_dev* rtldev;

RealtekDevice::RealtekDevice(QObject *parent) :
   RealtekDevice("", parent)
{
}

RealtekDevice::RealtekDevice(const QString &name, QObject *parent) :
   RadioDevice(parent), mIndex(0), mDevice(nullptr), mName(name), mSampleRate(-1), mCenterFrequency(-1), mTunerGain(-1), mGainMode(0), mReceivedSamples(0)
{
   mBufferData = new float[BUFFER_SIZE];
   mBufferLimit = mBufferData + BUFFER_SIZE;
   mBufferTail = mBufferData;
   mBufferHead = mBufferData;

   qDebug() << "created RealtekDevice" << mName;
}

RealtekDevice::~RealtekDevice()
{
   qDebug() << "destroy RealtekDevice" << mName;

   close();

   delete mBufferData;
}

QStringList RealtekDevice::listDevices()
{
   QStringList result;

   int count = rtlsdr_get_device_count();

   for (int i = 0; i < count; i++)
   {
      result.append(QString("rtlsdr://%1").arg(rtlsdr_get_device_name(i)));
   }

   return result;
}

const QString &RealtekDevice::name()
{
   return mName;
}

bool RealtekDevice::open(OpenMode mode)
{
   return open(mName, mode);
}

bool RealtekDevice::open(const QString &name, OpenMode mode)
{
   int result;
   void *device;

   close();

   // check device name
   if (!name.startsWith("rtlsdr://"))
   {
      qWarning() << "invalid device name" << name;
      return false;
   }

   int index = listDevices().indexOf(name);

   // open RTL device
   if ((result = rtlsdr_open((rtldev*) &device, (unsigned int) index)) == 0)
   {
      // configure frequency
      if (mCenterFrequency != -1)
      {
         qInfo() << "set frequency to" << mCenterFrequency << "Hz";

         if ((result = rtlsdr_set_center_freq(rtldev(device), mCenterFrequency)) < 0)
            qWarning() << "failed rtlsdr_set_center_freq:" << result;
      }

      // configure sample rate
      if (mSampleRate != -1)
      {
         qInfo() << "set samplerate to" << mSampleRate;

         if ((result = rtlsdr_set_sample_rate(rtldev(device), mSampleRate)) < 0)
            qWarning() << "failed rtlsdr_set_sample_rate:" << result;
      }

      // configure tuner gain
      if (mGainMode & TUNER_AUTO)
      {
         qInfo() << "set tuner gain to AUTO";

         // set automatic gain
         if ((result = rtlsdr_set_tuner_gain_mode(rtldev(device), 0)) < 0)
            qWarning() << "failed rtlsdr_set_tuner_gain_mode:" << result;
      }
      else if (mTunerGain != -1)
      {
         qInfo() << "set tuner gain to" << mTunerGain << "db";

         // set manual gain
         if ((result = rtlsdr_set_tuner_gain_mode(rtldev(device), 1)) < 0)
            qWarning() << "failed rtlsdr_set_tuner_gain_mode:" << result;

         if ((result = rtlsdr_set_tuner_gain(rtldev(device), mTunerGain)) < 0)
            qWarning() << "failed rtlsdr_set_tuner_gain:" << result;
      }

      // set RTL AGC mode
      if (mGainMode & DIGITAL_AUTO)
      {
         qInfo() << "enable digital AGC";

         if ((result = rtlsdr_set_agc_mode(rtldev(device), 1)) < 0)
            qWarning() << "failed rtlsdr_set_agc_mode:" << result;
      }
      else
      {
         qInfo() << "disable digital AGC";

         if ((result = rtlsdr_set_agc_mode(rtldev(device), 0)) < 0)
            qWarning() << "failed rtlsdr_set_agc_mode:" << result;
      }

      // enable test mode
//      if ((result = rtlsdr_set_testmode(rtldev(device), 1)) < 0)
//         qWarning() << "failed rtlsdr_set_testmode:" << result;

      // reset buffer to start streaming
      if ((result = rtlsdr_reset_buffer(rtldev(device))) < 0)
         qWarning() << "failed rtlsdr_reset_buffer:" << result;

      // setup device data
      mName = name;
      mDevice = device;

      // reset buffer status
      mBufferLoad = 0;
      mBufferTail = mBufferData;
      mBufferHead = mBufferData;
      mBufferLimit = mBufferData + BUFFER_SIZE;

      // start prefetch worker
      QThreadPool::globalInstance()->start(new TaskRunner<RealtekDevice>(this, &RealtekDevice::prefetch), QThread::HighPriority);
   }
   else
   {
      qWarning() << "failed rtlsdr_open:" << result;
      return false;
   }

   return QIODevice::open(mode);
}

void RealtekDevice::close()
{
   QIODevice::close();

   if (mDevice)
   {
      QMutexLocker lock(&mWorker);

      rtlsdr_close(rtldev(mDevice));

      mDevice = 0;
      mName = "";
   }
}

int RealtekDevice::sampleSize() const
{
   return RealtekDevice::SampleSize;
}

void RealtekDevice::setSampleSize(int sampleSize)
{
   qWarning() << "setSampleBits has no effect!";
}

long RealtekDevice::sampleRate() const
{
   return mSampleRate;
}

void RealtekDevice::setSampleRate(long sampleRate)
{
   int result;

   mSampleRate = sampleRate;

   if (mDevice)
   {
      if ((result = rtlsdr_set_sample_rate(rtldev(mDevice), sampleRate)) < 0)
         qWarning() << "failed rtlsdr_set_sample_rate(" << sampleRate << ")" << result;
   }
}

int RealtekDevice::sampleType() const
{
   return INTEGER;
}

void RealtekDevice::setSampleType(int sampleType)
{
   qWarning() << "setSampleType has no effect!";
}

long RealtekDevice::centerFrequency() const
{
   return mCenterFrequency;
}

void RealtekDevice::setCenterFrequency(long frequency)
{
   int result;

   mCenterFrequency = frequency;

   if (mDevice)
   {
      if ((result = rtlsdr_set_center_freq(rtldev(mDevice), frequency)) < 0)
         qWarning() << "failed rtlsdr_set_center_freq(" << frequency << ")" << result;
   }
}

int RealtekDevice::agcMode() const
{
   return mGainMode;

}

void RealtekDevice::setAgcMode(int gainMode)
{
   int result;

   mGainMode = gainMode;

   if (mDevice)
   {
      if (gainMode & TUNER_AUTO)
      {
         qInfo() << "enable tuner AGC";

         // set automatic gain
         if ((result = rtlsdr_set_tuner_gain_mode(rtldev(mDevice), 0)) < 0)
            qWarning() << "failed rtlsdr_set_tuner_gain_mode(" << 0 << ")" << result;
      }
      else if (mTunerGain != -1)
      {
         qInfo() << "set tuner gain to" << mTunerGain << "db";

         // set manual gain
         if ((result = rtlsdr_set_tuner_gain_mode(rtldev(mDevice), 1)) < 0)
            qWarning() << "failed rtlsdr_set_tuner_gain_mode(" << 1 << ")" << result;

         if ((result = rtlsdr_set_tuner_gain(rtldev(mDevice), mTunerGain)) < 0)
            qWarning() << "failed rtlsdr_set_tuner_gain(" << mTunerGain << ")" << result;
      }

      if (gainMode & DIGITAL_AUTO)
      {
         qInfo() << "enable digital AGC";

         // enable digital AGC
         if ((result = rtlsdr_set_agc_mode(rtldev(mDevice), 1)) < 0)
            qWarning() << "failed rtlsdr_set_agc_mode(" << 1 << ")" << result;
      }
      else
      {
         qInfo() << "disable digital AGC";

         // disable digital AGC
         if ((result = rtlsdr_set_agc_mode(rtldev(mDevice), 0)) < 0)
            qWarning() << "failed rtlsdr_set_agc_mode(" << 0 << ")" << result;
      }
   }
}

float RealtekDevice::receiverGain() const
{
   return mTunerGain;
}

void RealtekDevice::setReceiverGain(float tunerGain)
{
   mTunerGain = tunerGain;

   if (mDevice)
   {
      int gain = tunerGain * 10;

      if (rtlsdr_set_tuner_gain(rtldev(mDevice), gain) < 0)
      {
         qWarning() << "rtlsdr_set_tuner_gain failed!";
      }
   }
}

QVector<int> RealtekDevice::supportedSampleRates() const
{
   QVector<int> result;

   result.append(225000); // 0.25 MSPS
   result.append(900000); // 0.90 MSPS
   result.append(1024000); // 1.024 MSPS
   result.append(1400000); // 1.4 MSPS
   result.append(1800000); // 1.8 MSPS
   result.append(1920000); // 1.92 MSPS
   result.append(2048000); // 2.048 MSPS
   result.append(2400000); // 2.4 MSPS
   result.append(2560000); // 2.56 MSPS
   result.append(2800000); // 2.8 MSPS
   result.append(3200000); // 3.2 MSPS

   return result;
}

QVector<float> RealtekDevice::supportedReceiverGains() const
{
   QVector<float> result;

   if (mDevice)
   {
      int count = rtlsdr_get_tuner_gains(rtldev(mDevice), NULL);

      if (count > 0)
      {
         int buffer[1024];

         rtlsdr_get_tuner_gains(rtldev(mDevice), buffer);

         for (int i = 0; i < count; i++)
         {
            result.append(buffer[i] / 10.0f);
         }
      }
   }

   return result;
}

bool RealtekDevice::waitForReadyRead(int msecs)
{
   if (!isOpen())
      return false;

   QElapsedTimer timer;

   timer.start();

   while (!mBufferLoad && timer.elapsed() < msecs)
   {
      QThread::msleep(1);
   }

   return mBufferLoad;
}

int RealtekDevice::read(SampleBuffer<float> signal)
{
   if (!isOpen())
      return -1;

   while (signal.available() > 0)
   {
      int blocks = 0;
      int length = mBufferLoad;
      float v[2], *src = mBufferTail;

      while (blocks < length && signal.available() > 0)
      {
         v[0] = *src++;
         v[1] = *src++;

         if (src >= mBufferLimit)
            src = mBufferData;

         signal.put(v);

         blocks++;
      }

      mBufferTail = src;
      mBufferLoad.fetchAndAddOrdered(-blocks);
   }

   signal.flip();

   return signal.limit();
}

int RealtekDevice::write(SampleBuffer<float> signal)
{
   qWarning() << "write not supported on this device!";

   return -1;
}

qint64 RealtekDevice::readData(char* data, qint64 maxSize)
{
   if (maxSize & 0x7)
   {
      qWarning() << "read buffer must be multiple of 8 bytes";
      return -1;
   }

   int blocks = 0;
   int length = mBufferLoad;
   float *src = mBufferTail;
   float *dst = (float*) data;

   while (blocks < length && blocks < (maxSize >> 3))
   {
      *dst++ = *src++; // I
      *dst++ = *src++; // Q

      if (src >= mBufferLimit)
         src = mBufferData;

      blocks++;
   }

   mBufferTail = src;
   mBufferLoad.fetchAndAddOrdered(-blocks);

   return blocks * sizeof(float) * 2;
}

qint64 RealtekDevice::writeData(const char* data, qint64 maxSize)
{
   qWarning() << "writeData not supported on this device!";

   return -1;
}

void RealtekDevice::prefetch()
{
   static int prefetchCount1 = 0;
   static int prefetchCount2 = 0;

   int stored;
   int readed;
   char data[4096];

   QMutexLocker lock(&mWorker);

   qWarning() << "starting realtek prefetch thread";

   while (isOpen())
   {
      if (rtlsdr_read_sync(rtldev(mDevice), data, sizeof(data), &readed) < 0)
         qWarning() << "failed rtlsdr_read_sync!";
      else if (readed != sizeof(data))
         qWarning() << "short read, samples lost!";

      prefetchCount1++;

      if (prefetchCount1 - prefetchCount2 == 100)
      {
         qInfo().noquote() << "prefetch," << prefetchCount1 << "blocks," << mReceivedSamples << "samples," << "buffer load" << QString("%1 %").arg(100 * double(mBufferLoad) / double(BUFFER_SIZE), 1, 'f', 2);
         prefetchCount2 = prefetchCount1;
      }

      if ((stored = mBufferLoad) < BUFFER_SIZE && readed > 0)
      {
         unsigned int blocks = 0;
         unsigned int length = readed >> 1;

         unsigned char *src = (unsigned char *) data;
         float *dst = mBufferHead;
         int i, q;

         while (blocks < length && stored < BUFFER_SIZE)
         {
            i = *src++;
            q = *src++;

            *dst++ = float((i - 128) / 128.0);
            *dst++ = float((q - 128) / 128.0);

            if (dst >= mBufferLimit)
               dst = mBufferData;

            blocks++;
            stored++;
         }

         // update sample counter
         mReceivedSamples += blocks;

         // update buffer status
         mBufferHead = dst;
         mBufferLoad.fetchAndAddOrdered(blocks);
      }
      else
      {
         qWarning() << "buffer full, samples lost!";
      }
   }

   qWarning() << "terminate realtek prefetch";
}



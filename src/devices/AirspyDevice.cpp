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
#include <QElapsedTimer>

#include <airspy.h>

#include "AirspyDevice.h"

#define BUFFER_SIZE (1024 * 1024)

typedef struct airspy_device* airdev;

int transferCallback(void *info);

AirspyDevice::AirspyDevice(QObject *parent) :
   AirspyDevice("", parent)
{
}

AirspyDevice::AirspyDevice(const QString &name, QObject *parent) :
   RadioDevice(parent), mDevice(nullptr), mName(name), mFileDesc(-1), mSampleRate(-1), mCenterFrequency(-1), mTunerGain(-1), mGainMode(0), mBufferData(nullptr), mBufferLimit(nullptr), mReceivedSamples(0)
{
   mBufferData = new float[BUFFER_SIZE];
   mBufferLimit = mBufferData + BUFFER_SIZE;
   mBufferTail = mBufferData;
   mBufferHead = mBufferData;

   qDebug() << "created AirspyDevice" << mName;
}

AirspyDevice::AirspyDevice(const QString &name, int fd, QObject *parent) :
   RadioDevice(parent), mDevice(nullptr), mName(name), mFileDesc(fd), mSampleRate(-1), mCenterFrequency(-1), mTunerGain(-1), mGainMode(0), mBufferData(nullptr), mBufferLimit(nullptr), mReceivedSamples(0)
{
   mBufferData = new float[BUFFER_SIZE];
   mBufferLimit = mBufferData + BUFFER_SIZE;
   mBufferTail = mBufferData;
   mBufferHead = mBufferData;

   qDebug() << "created AirspyDevice" << mName << "wrap file descriptor" << mFileDesc;
}

AirspyDevice::~AirspyDevice()
{
   qDebug() << "destroy AirspyDevice" << mName;

   close();

   delete mBufferData;
}

QStringList AirspyDevice::listDevices()
{
   QStringList result;

   uint64_t devices[8];

   int count = airspy_list_devices(devices, sizeof(devices) / sizeof(uint64_t));

   for (int i = 0; i < count; i++)
   {
      result.append(QString("airspy://%1").arg(devices[i], 16, 16, QChar('0')));
   }

   return result;
}

const QString &AirspyDevice::name()
{
   return mName;
}

bool AirspyDevice::open(OpenMode mode)
{
   return open(mName, mode);
}

bool AirspyDevice::open(const QString &name, OpenMode mode)
{
   int result;
   void *device;

   close();

   // check device name
   if (!name.startsWith("airspy://"))
   {
      qWarning() << "invalid device name" << name;
      return false;
   }

#ifdef ANDROID
   // special open mode based on /sys/ node and file descriptor (primary for android devices)
   if (name.startsWith("airspy://sys/"))
   {
      // extract full /sys/... path to device node
      QByteArray node = mName.mid(8).toLocal8Bit();

      // open Airspy device
      result = airspy_open_fd((airdev*)&device, node.data(), mFileDesc);
   }
   else
#endif

   // standard open mode based on serial number
   {
      // extract serial number
      uint64_t sn = name.right(16).toULongLong(nullptr, 16);

      // open Airspy device
      result = airspy_open_sn((airdev*)&device, sn);
   }

   if (result == AIRSPY_SUCCESS)
   {
      qInfo() << "open device" << name;

      airspy_read_partid_serialno_t partno;

      // disable bias tee
      if ((result = airspy_set_rf_bias(airdev(device), 0)) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_set_rf_bias:" << airspy_error_name((enum airspy_error) result);

      // read board serial
      if ((result = airspy_board_partid_serialno_read(airdev(device), &partno)) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_board_partid_serialno_read:" << airspy_error_name((enum airspy_error) result);

      qInfo() << "set sample type to" << AIRSPY_SAMPLE_FLOAT32_IQ;

      if ((result = airspy_set_sample_type(airdev(device), AIRSPY_SAMPLE_FLOAT32_IQ)) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_set_sample_type:" << airspy_error_name((enum airspy_error) result);

      // configure frequency
      if (mCenterFrequency > 0)
      {
         qInfo() << "set frequency to" << mCenterFrequency << "Hz";

         if ((result = airspy_set_freq(airdev(device), mCenterFrequency)) != AIRSPY_SUCCESS)
            qWarning() << "failed airspy_set_freq:" << airspy_error_name((enum airspy_error) result);
      }

      // configure sample rate
      if (mSampleRate > 0)
      {
         qInfo() << "set samplerate to" << mSampleRate;

         if ((result = airspy_set_samplerate(airdev(device), mSampleRate)) != AIRSPY_SUCCESS)
            qWarning() << "failed airspy_set_samplerate:" << airspy_error_name((enum airspy_error) result);
      }

      // configure tuner AGC
      qInfo() << "set LNA AGC to" << (mGainMode & RadioDevice::TUNER_AUTO ? "ON" : "OFF");

      if ((result = airspy_set_lna_agc(airdev(device), (mGainMode & RadioDevice::TUNER_AUTO) != 0)) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_set_lna_agc:" << airspy_error_name((enum airspy_error) result);

      // configure mixer AGC
      qInfo() << "set mixer AGC to" << (mGainMode & RadioDevice::MIXER_AUTO ? "ON" : "OFF");

      if ((result = airspy_set_mixer_agc(airdev(device), mGainMode & RadioDevice::MIXER_AUTO) != 0) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_set_mixer_agc:" << airspy_error_name((enum airspy_error) result);

      // configure linearity gain
      if (mTunerGain >= 0)
      {
         qInfo() << "set linearity gain to" << mTunerGain << "db";

         // set linearity gain
         if ((result = airspy_set_linearity_gain(airdev(device), mTunerGain)) != AIRSPY_SUCCESS)
            qWarning() << "failed airspy_set_linearity_gain:" << airspy_error_name((enum airspy_error) result);
      }

      // start streaming
      if ((result = airspy_start_rx(airdev(device), (airspy_sample_block_cb_fn) transferCallback, this)) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_start_rx:" << airspy_error_name((enum airspy_error) result);

      // set device name and handle
      mName = name;
      mDevice = device;

      // reset buffer status
      mBufferLoad = 0;
      mBufferTail = mBufferData;
      mBufferHead = mBufferData;
      mBufferLimit = mBufferData + BUFFER_SIZE;
   }
   else
   {
      qWarning() << "failed airspy_open_sn:" << airspy_error_name((enum airspy_error) result);
      return false;
   }

   return QIODevice::open(mode);
}

void AirspyDevice::close()
{
   if (mDevice)
   {
      int result;

      qInfo() << "close device" << mName;

      // stop streaming
      if ((result = airspy_stop_rx(airdev(mDevice))) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_stop_rx:" << airspy_error_name((enum airspy_error) result);

      // close device
      if ((result = airspy_close(airdev(mDevice))) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_close:" << airspy_error_name((enum airspy_error) result);

      mDevice = nullptr;
      mName = "";
   }

   QIODevice::close();
}

int AirspyDevice::sampleSize() const
{
   return AirspyDevice::SampleSize;
}

void AirspyDevice::setSampleSize(int sampleSize)
{
   qWarning() << "setSampleSize has no effect!";
}

long AirspyDevice::sampleRate() const
{
   return mSampleRate;
}

void AirspyDevice::setSampleRate(long sampleRate)
{
   int result;

   mSampleRate = sampleRate;

   if (mDevice)
   {
      if ((result = airspy_set_samplerate(airdev(mDevice), mSampleRate)) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_set_samplerate:" << airspy_error_name((enum airspy_error) result);
   }
}

int AirspyDevice::sampleType() const
{
   return INTEGER;
}

void AirspyDevice::setSampleType(int sampleType)
{
   qWarning() << "setSampleType has no effect!";
}

long AirspyDevice::centerFrequency() const
{
   return mCenterFrequency;
}

void AirspyDevice::setCenterFrequency(long frequency)
{
   int result;

   mCenterFrequency = frequency;

   if (mDevice)
   {
      if ((result = airspy_set_freq(airdev(mDevice), mCenterFrequency)) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_set_freq:" << airspy_error_name((enum airspy_error) result);
   }
}

int AirspyDevice::agcMode() const
{
   return mGainMode;
}

void AirspyDevice::setAgcMode(int gainMode)
{
   int result;

   mGainMode = gainMode;

   if (mDevice)
   {
      if ((result = airspy_set_mixer_agc(airdev(mDevice), ((mGainMode & RadioDevice::MIXER_AUTO) != 0))) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_set_mixer_agc:" << airspy_error_name((enum airspy_error) result);

      if ((result = airspy_set_lna_agc(airdev(mDevice), ((mGainMode & RadioDevice::TUNER_AUTO) != 0))) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_set_lna_agc:" << airspy_error_name((enum airspy_error) result);
   }
}

float AirspyDevice::receiverGain() const
{
   return mTunerGain;
}

void AirspyDevice::setReceiverGain(float tunerGain)
{
   int result;

   mTunerGain = tunerGain;

   if (mDevice)
   {
      if ((result = airspy_set_linearity_gain(airdev(mDevice), mTunerGain)) != AIRSPY_SUCCESS)
         qWarning() << "failed airspy_set_linearity_gain:" << airspy_error_name((enum airspy_error) result);
   }
}

QVector<int> AirspyDevice::supportedSampleRates() const
{
   QVector<int> result;

   uint32_t count, rates[256];

   if (mDevice)
   {
      // get number of supported sample rates
      airspy_get_samplerates(airdev(mDevice), &count, 0);

      // get list of supported sample rates
      airspy_get_samplerates(airdev(mDevice), rates, count);

      for (int i = 0; i < (int) count; i++)
      {
         result.append(rates[i]);
      }
   }

   return result;
}

QVector<float> AirspyDevice::supportedReceiverGains() const
{
   QVector<float> result;

   for (int i = 0; i < 22; i++)
   {
      result.append(i);
   }

   return result;
}

bool AirspyDevice::waitForReadyRead(int msecs)
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

int AirspyDevice::read(SampleBuffer<float> signal)
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

int AirspyDevice::write(SampleBuffer<float> signal)
{
   qWarning() << "write not supported on this device!";

   return -1;
}

qint64 AirspyDevice::readData(char* data, qint64 maxSize)
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

qint64 AirspyDevice::writeData(const char* data, qint64 maxSize)
{
   qWarning() << "writeData not supported on this device!";

   return -1;
}

int AirspyDevice::prefetch(int type, void *data, int samples)
{
   int stored;

   static int prefetchCount1 = 0;
   static int prefetchCount2 = 0;

   prefetchCount1++;

   if (prefetchCount1 - prefetchCount2 == 100)
   {
      qInfo().noquote() << "prefetch," << prefetchCount1 << "blocks," << mReceivedSamples << "samples," << "buffer load" << QString("%1 %").arg(100 * double(mBufferLoad) / double(BUFFER_SIZE), 1, 'f', 2);
      prefetchCount2 = prefetchCount1;
   }

   if ((stored = mBufferLoad) < BUFFER_SIZE && samples > 0)
   {
      unsigned int blocks = 0;

      float *src = (float*) data;
      float *dst = mBufferHead;

      while (samples > 0 && stored < BUFFER_SIZE)
      {
         *dst++ = *src++;
         *dst++ = *src++;

         if (dst >= mBufferLimit)
            dst = mBufferData;

         blocks++;
         stored++;
         samples--;
      }

      // update sample counter
      mReceivedSamples += blocks;

      // update buffer status
      mBufferHead = dst;
      mBufferLoad.fetchAndAddOrdered(blocks);
   }

   return 0;
}

int transferCallback(void *info)
{
   airspy_transfer_t *transfer = (airspy_transfer_t*) info;

   AirspyDevice *receiver = (AirspyDevice*) transfer->ctx;

   return receiver->prefetch(transfer->sample_type, transfer->samples, transfer->sample_count);
}


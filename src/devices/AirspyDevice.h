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

#ifndef AIRSPYDEVICE_H
#define AIRSPYDEVICE_H

#include <QMutex>
#include <QIODevice>
#include <QAtomicInt>
#include <QAtomicPointer>

#include "RadioDevice.h"

class AirspyDevice: public RadioDevice
{
   public:

      static const int SampleSize = 16;

   public:

      AirspyDevice(QObject *parent = nullptr);
      AirspyDevice(const QString &name, QObject *parent = nullptr);
      AirspyDevice(const QString &name, int fd, QObject *parent = nullptr);

      virtual ~AirspyDevice();

      static QStringList listDevices();

   public:
      // from QIODevice

      void close();

   public:
      // from RadioDevice

      const QString &name();

      bool open(OpenMode mode);

      bool open(const QString &name, OpenMode mode);

      int sampleSize() const;

      void setSampleSize(int sampleSize);

      long sampleRate() const;

      void setSampleRate(long sampleRate);

      int sampleType() const;

      void setSampleType(int sampleType);

      long centerFrequency() const;

      void setCenterFrequency(long frequency);

      int agcMode() const;

      void setAgcMode(int gainMode);

      float receiverGain() const;

      void setReceiverGain(float tunerGain);

      QVector<int> supportedSampleRates() const;

      QVector<float> supportedReceiverGains() const;

      bool waitForReadyRead(int msecs);

      int read(SampleBuffer<float> signal);

      int write(SampleBuffer<float> signal);

      using SignalDevice::read;

      using SignalDevice::write;

   protected:
      // from QIODevice

      qint64 readData(char* data, qint64 maxSize);

      qint64 writeData(const char* data, qint64 maxSize);

   private:

      int prefetch(int type, void *data, int samples);

   private:

      // device control
      void* mDevice;
      QString mName;
      int mFileDesc;
      long mSampleRate;
      long mCenterFrequency;
      float mTunerGain;
      int mGainMode;
      QMutex mMutex;

      // buffer control
      float *mBufferData;
      float *mBufferLimit;

      QAtomicPointer<float> mBufferHead;
      QAtomicPointer<float> mBufferTail;
      QAtomicInteger<int> mBufferLoad;

      long mReceivedSamples;

      friend int transferCallback(void *info);
};

#endif /* AIRSPYDEVICE_H_ */

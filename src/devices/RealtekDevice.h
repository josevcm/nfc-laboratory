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


#ifndef REALTEKDEVICE_H
#define REALTEKDEVICE_H

#include <QFuture>
#include <QAtomicInt>

#include <support/TaskRunner.h>

#include "RadioDevice.h"

class RealtekDevice: public RadioDevice
{
   public:

      static const int SampleSize = 16;

   public:

      RealtekDevice(QObject *parent = nullptr);
      RealtekDevice(const QString &name, QObject *parent = nullptr);

      virtual ~RealtekDevice();

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

      // buffer prefetch
      void prefetch();

   private:

      int mIndex;
      void *mDevice;
      QString mName;
      long mSampleRate;
      long mCenterFrequency;
      float mTunerGain;
      int mGainMode;

      // worker syncronization
      QMutex mWorker;

      // buffer control
      float *mBufferData;
      float *mBufferLimit;

      QAtomicPointer<float> mBufferHead;
      QAtomicPointer<float> mBufferTail;
      QAtomicInteger<int> mBufferLoad;

      long mReceivedSamples;

      friend class TaskRunner<RealtekDevice>;
};

#endif /* REALTEKDEVICE_H */

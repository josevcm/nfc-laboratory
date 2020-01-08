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

#ifndef RECORDDEVICE_H
#define RECORDDEVICE_H

#include <QFile>
#include <QMutex>

#include <support/TaskRunner.h>

#include "SignalDevice.h"

class RecordDevice: public SignalDevice
{
   private:

      enum TaskStatus
      {
         Offline = 0, Stop = 1, Run = 2
      };

   public:

      RecordDevice(QObject *parent = nullptr);
      RecordDevice(const QString &name, QObject *parent = nullptr);

      virtual ~RecordDevice();

      const QString &name();

      bool open(OpenMode mode);

      bool open(const QString &name, OpenMode mode);

      void close();

      bool atEnd() const;

      // from SignalDevice

      int sampleSize() const;

      void setSampleSize(int sampleSize);

      long sampleRate() const;

      void setSampleRate(long sampleRate);

      int sampleType() const;

      void setSampleType(int sampleType);

      long centerFrequency() const;

      void setCenterFrequency(long frequency);

      int channelCount() const;

      void setChannelCount(int channelCount);

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

      bool readHeader();

      bool writeHeader();

      void readerTask();

      void writerTask();

   private:

      QString mName;
      QFile mFile;

      int mSampleSize;
      int mSampleRate;
      int mSampleType;
      int mChannelCount;

      // read buffer control
      char *mBufferData;
      char *mBufferLimit;

      QAtomicPointer<char> mBufferHead;
      QAtomicPointer<char> mBufferTail;
      QAtomicInteger<int> mBufferLoad;

      QAtomicInteger<int> mTaskStatus;
      mutable QMutex mTaskMutex;

      friend class TaskRunner<RecordDevice>;
};

#endif /* RECORDDEVICE_H */

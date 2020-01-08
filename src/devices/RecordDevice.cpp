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
#include <QtEndian>
#include <QElapsedTimer>

#include "RecordDevice.h"

#define TRANSF_SIZE (1024*1024)
#define BUFFER_SIZE (16*1024*1024)

struct chunk
{
      char id[4];
      quint32 size;
};

struct RIFFHeader
{
      chunk desc;
      char type[4];
};

struct WAVEHeader
{
      chunk desc;
      quint16 audioFormat;
      quint16 numChannels;
      quint32 sampleRate;
      quint32 byteRate;
      quint16 blockAlign;
      quint16 bitsPerSample;
};

struct DATAHeader
{
      chunk desc;
};

struct FILEHeader
{
      RIFFHeader riff;
      WAVEHeader wave;
      DATAHeader data;
};

RecordDevice::RecordDevice(QObject *parent) :
   RecordDevice("", parent)
{
}

RecordDevice::RecordDevice(const QString &name, QObject *parent) :
   SignalDevice(parent), mName(name), mSampleSize(16), mSampleRate(44100), mSampleType(1), mChannelCount(1), mBufferData(nullptr), mBufferLimit(nullptr), mTaskStatus(Offline)
{
   mBufferData = new char[BUFFER_SIZE];
   mBufferLimit = mBufferData + BUFFER_SIZE;
   mBufferTail = mBufferData;
   mBufferHead = mBufferData;
}

RecordDevice::~RecordDevice()
{
   close();

   delete mBufferData;
}

const QString &RecordDevice::name()
{
   return mName;
}

bool RecordDevice::open(OpenMode mode)
{
   return open(mName, mode);
}

bool RecordDevice::open(const QString &name, OpenMode mode)
{
   if (!name.startsWith("record://"))
   {
      qWarning() << "invalid device name" << name;
      return false;
   }

   close();

   mFile.setFileName(name.right(name.length() - 9));

   if (mFile.open(mode))
   {
      if (mode == QIODevice::ReadOnly)
      {
         if (!readHeader())
         {
            mFile.close();
         }

         if (mFile.isOpen())
         {
            mTaskStatus = TaskStatus::Run;

            QThreadPool::globalInstance()->start(new TaskRunner<RecordDevice>(this, &RecordDevice::readerTask));
         }
      }
      else if (mode == QIODevice::WriteOnly)
      {
         if (!writeHeader())
         {
            mFile.close();
         }

         if (mFile.isOpen())
         {
            mTaskStatus = TaskStatus::Run;

            QThreadPool::globalInstance()->start(new TaskRunner<RecordDevice>(this, &RecordDevice::writerTask));
         }
      }
   }

   mName = name;
   mBufferTail = mBufferData;
   mBufferHead = mBufferData;

   return mFile.isOpen() && QIODevice::open(mode);
}

void RecordDevice::close()
{
   if (isOpen())
   {
      if (mTaskStatus > TaskStatus::Offline)
      {
         mTaskStatus = TaskStatus::Stop;

         while(mTaskStatus > TaskStatus::Offline)
         {
            if (mTaskMutex.tryLock(50))
            {
               mTaskMutex.unlock();

               QThread::currentThread()->msleep(50);
            }
         }
      }

      if (openMode() == QIODevice::WriteOnly)
      {
         writeHeader();
      }

      mFile.flush();
      mFile.close();

      QIODevice::close();
   }
}

bool RecordDevice::atEnd() const
{
   bool atEnd;

   if ((atEnd = mTaskMutex.tryLock()))
   {
      mTaskMutex.unlock();
   }

   return atEnd && openMode() == ReadOnly && mBufferLoad == 0;
}

int RecordDevice::sampleSize() const
{
   return mSampleSize;
}

void RecordDevice::setSampleSize(int sampleSize)
{
   mSampleSize = sampleSize;
}

long RecordDevice::sampleRate() const
{
   return mSampleRate;
}

void RecordDevice::setSampleRate(long sampleRate)
{
   mSampleRate = sampleRate;
}

int RecordDevice::sampleType() const
{
   return mSampleType;
}

void RecordDevice::setSampleType(int sampleType)
{
   mSampleType = sampleType;
}

long RecordDevice::centerFrequency() const
{
   return 0;
}

void RecordDevice::setCenterFrequency(long frequency)
{
   qWarning() << "setCenterFrequency has no effect!";
}

int RecordDevice::channelCount() const
{
   return mChannelCount;
}

void RecordDevice::setChannelCount(int channelCount)
{
   mChannelCount = channelCount;
}

bool RecordDevice::waitForReadyRead(int msecs)
{
   if (!isOpen())
      return false;

   QElapsedTimer timer;

   timer.start();

   while (!mBufferLoad && timer.elapsed() < msecs)
   {
      QThread::msleep(1);
   }

   return mBufferLoad;}

int RecordDevice::read(SampleBuffer<float> signal)
{
   int readed;
   float vector[256];
   char buffer[64 * 1024];

   int frame = mChannelCount * mSampleSize / 8;

   while (signal.available() > 0)
   {
      if ((readed = readData(buffer, frame)) <= 0)
         break;

      for (int c = 0; c < signal.stride(); c++)
      {
         if (c < mChannelCount)
         {
            void *p = buffer + c * mSampleSize / 8;

            if (mSampleSize == 8)
               vector[c] = qFromLittleEndian<qint8>(p) / float(1 << 7);
            else if (mSampleSize == 16)
               vector[c] = qFromLittleEndian<qint16>(p) / float(1 << 15);
            else if (mSampleSize == 32)
               vector[c] = qFromLittleEndian<qint32>(p) / float(1 << 31);
         }
         else
         {
            vector[c] = 0;
         }
      }

      signal.put(vector);
   }

   signal.flip();

   return signal.limit();
}

int RecordDevice::write(SampleBuffer<float> signal)
{
   float vector[256], value;

   char buffer[64 * 1024];

   int frame = mChannelCount * mSampleSize / 8;

   while (signal.available() > 0)
   {
      signal.get(vector);

      for (int c = 0; c < mChannelCount; c++)
      {
         void *p = buffer + c * mSampleSize / 8;

         value = c < signal.stride() ? vector[c] : 0;

         if (mSampleSize == 8)
            *((qint8*) (p)) = qToLittleEndian<quint8>(value * (1 << 7));
         else if (mSampleSize == 16)
            *((qint16*) (p)) = qToLittleEndian<quint16>(value * (1 << 15));
         else if (mSampleSize == 32)
            *((qint32*) (p)) = qToLittleEndian<quint32>(value * (1 << 31));
      }

      if (writeData(buffer, frame) < 0)
         break;
   }

   return signal.position();
}

qint64 RecordDevice::readData(char* data, qint64 maxSize)
{
   int stored, blocks = 0;

   if (maxSize > 0)
   {
      while ((stored = mBufferLoad) == 0)
      {
         QThread::msleep(50);

         if (atEnd())
         {
            qWarning() << "stream end reached!";
            return -1;
         }

         if (!isOpen())
         {
            qWarning() << "stream closed!";
            return -1;
         }
      }

      char *dst = data;
      char *src = mBufferTail;

      while (blocks < stored && blocks < maxSize)
      {
         *dst++ = *src++;

         if (src >= mBufferLimit)
            src = mBufferData;

         blocks++;
      }

      mBufferTail = src;
      mBufferLoad.fetchAndAddOrdered(-blocks);
   }

   return blocks;
}

qint64 RecordDevice::writeData(const char* data, qint64 maxSize)
{
   int stored, blocks = 0;

   if (maxSize > 0)
   {
      while ((stored = mBufferLoad) == BUFFER_SIZE)
      {
         QThread::msleep(50);

         if (!isOpen())
         {
            qWarning() << "stream closed!";
            return -1;
         }
      }

      char *dst = mBufferHead;
      char *src = (char*) data;

      while ((stored + blocks) < BUFFER_SIZE && blocks < maxSize)
      {
         *dst++ = *src++;

         if (dst >= mBufferLimit)
            dst = mBufferData;

         blocks++;
      }

      mBufferHead = dst;
      mBufferLoad.fetchAndAddOrdered(blocks);
   }

   return blocks;
}

bool RecordDevice::readHeader()
{
   FILEHeader header;

   mFile.seek(0);

   if (mFile.read(reinterpret_cast<char *>(&header), sizeof(FILEHeader)) != sizeof(FILEHeader))
      return false;

   if (memcmp(&header.riff.desc.id, "RIFF", 4) != 0)
      return false;

   if (memcmp(&header.riff.type, "WAVE", 4) != 0)
      return false;

   if (memcmp(&header.wave.desc.id, "fmt ", 4) != 0)
      return false;

   if (header.wave.audioFormat != 1)
      return false;

   // Establish format
   mChannelCount = qFromLittleEndian<quint16>(header.wave.numChannels);
   mSampleRate = qFromLittleEndian<quint32>(header.wave.sampleRate);
   mSampleSize = qFromLittleEndian<quint16>(header.wave.bitsPerSample);
   mSampleType = INTEGER;

   return true;
}

bool RecordDevice::writeHeader()
{
   FILEHeader header = {
      // RIFFHeader
      { { { 'R', 'I', 'F', 'F' }, 0 }, { 'W', 'A', 'V', 'E' } },
      // WAVEHeader
      { { { 'f', 'm', 't', ' ' }, 16 }, 0, 0, 0, 0, 0, 0 },
      // DATAHeader
      { { { 'd', 'a', 't', 'a' }, 0 } }
   };

   mFile.seek(0);

   int length = mFile.size();

   header.wave.audioFormat = qToLittleEndian<quint16>(1);
   header.wave.numChannels = qToLittleEndian<quint16>(mChannelCount);
   header.wave.sampleRate = qToLittleEndian<quint32>(mSampleRate);
   header.wave.byteRate = qToLittleEndian<quint32>(mChannelCount * mSampleRate * mSampleSize / 8);
   header.wave.blockAlign = qToLittleEndian<quint16>(mChannelCount * mSampleSize / 8);
   header.wave.bitsPerSample = qToLittleEndian<quint16>(mSampleSize);

   header.data.desc.size = qToLittleEndian<quint32>(length > 36 ? length - sizeof(FILEHeader) : 0);
   header.riff.desc.size = qToLittleEndian<quint32>(length > 40 ? length - sizeof(RIFFHeader) + 4 : 40);

   if (mFile.write(reinterpret_cast<char *>(&header), sizeof(FILEHeader)) != sizeof(FILEHeader))
      return false;

   return true;
}

void RecordDevice::readerTask()
{
   qDebug() << "starting stream reader task";

   QMutexLocker lock(&mTaskMutex);

   while (mTaskStatus == TaskStatus::Run && !mFile.atEnd())
   {
      int stored;

      // wait if buffer is full
      while ((stored = mBufferLoad) == BUFFER_SIZE)
      {
         QThread::msleep(100);

         if (mTaskStatus == TaskStatus::Stop || mFile.atEnd())
            break;
      }

      while (stored < BUFFER_SIZE && !mFile.atEnd())
      {
         int blocks = 0;
         int length = (BUFFER_SIZE - stored) > TRANSF_SIZE ? TRANSF_SIZE : (BUFFER_SIZE - stored);

         char *buffer = mBufferHead;

         if (buffer + length > mBufferLimit)
            length = mBufferLimit - buffer;

         if ((blocks = mFile.read(buffer, length)) > 0)
         {
            buffer += blocks;
            stored += blocks;

            if (buffer >= mBufferLimit)
               buffer = mBufferData;

            mBufferHead = buffer;
            mBufferLoad.fetchAndAddOrdered(blocks);
         }
         else if (blocks < 0)
         {
            qWarning() << "record stream read error!";

            return;
         }
      }
   }

   mTaskStatus = TaskStatus::Offline;

   qDebug() << "terminate stream reader task";
}

void RecordDevice::writerTask()
{
   qDebug() << "starting stream writer task";

   QMutexLocker lock(&mTaskMutex);

   while (mTaskStatus == TaskStatus::Run || mBufferLoad)
   {
      int stored;

      // wait if buffer is empty
      while ((stored = mBufferLoad) == 0)
      {
         QThread::msleep(100);

         if (mTaskStatus == TaskStatus::Stop)
            break;
      }

      while (stored > 0)
      {
         int blocks = 0;
         int length = stored > TRANSF_SIZE ? TRANSF_SIZE : stored;

         char *buffer = mBufferTail;

         if (buffer + length > mBufferLimit)
            length = mBufferLimit - buffer;

         if ((blocks = mFile.write(buffer, length)) > 0)
         {
            buffer += blocks;
            stored -= blocks;

            if (buffer >= mBufferLimit)
               buffer = mBufferData;

            mBufferTail = buffer;
            mBufferLoad.fetchAndAddOrdered(-blocks);
         }
         else if (blocks < 0)
         {
            qWarning() << "stream write error!";
         }
      }
   }

   mTaskStatus = TaskStatus::Offline;

   qDebug() << "terminate stream writer task";
}


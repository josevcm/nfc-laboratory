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

#include <decoder/NfcStream.h>

#include "StorageReader.h"

StorageReader::StorageReader(QIODevice *device, QObject *parent) : QObject(parent), mDevice(device), mFrequency(0), mSampling(0)
{
}

StorageReader::~StorageReader()
{
   close();
}

bool StorageReader::open()
{
   if (mDevice->open(QIODevice::ReadOnly))
   {
      mReader.setDevice(mDevice);

      if(mReader.readNextStartElement())
      {
         if (mReader.name() == "stream")
         {
            if (mReader.attributes().hasAttribute("frequency"))
               mFrequency = mReader.attributes().value("frequency").toLong();

            if (mReader.attributes().hasAttribute("sampling"))
               mSampling = mReader.attributes().value("sampling").toLong();

            return true;
         }
      }

      mDevice->close();
   }

   return false;
}

void StorageReader::close()
{
   if (mDevice && mDevice->isOpen())
   {
      mDevice->close();
   }
}

long StorageReader::frequency() const
{
   return mFrequency;
}

long StorageReader::sampling() const
{
   return mSampling;
}

bool StorageReader::readStream(NfcStream *stream)
{
   while(mReader.readNextStartElement())
   {
      if (mReader.name() == "frame")
      {
         QXmlStreamAttributes attr = mReader.attributes();

         if (attr.hasAttribute("tech") && attr.hasAttribute("type") && attr.hasAttribute("flags"))
         {
            int tech = attr.value("tech").toInt();
            int type = attr.value("type").toInt();
            int flags = attr.value("flags").toInt();

            NfcFrame frame(tech, type, flags);

            if (attr.hasAttribute("start"))
               frame.setTimeStart(mReader.attributes().value("start").toDouble());

            if (attr.hasAttribute("end"))
               frame.setTimeEnd(mReader.attributes().value("end").toDouble());

            if (attr.hasAttribute("stage"))
               frame.setFramePhase(mReader.attributes().value("stage").toInt());

            if (attr.hasAttribute("rate"))
               frame.setFrameRate(mReader.attributes().value("rate").toInt());

            if (mSampling > 0)
            {
               frame.setSampleStart(long(frame.timeStart() * mSampling));
               frame.setSampleEnd(long(frame.timeEnd() * mSampling));
            }

            frame.put(QByteArray::fromHex(mReader.readElementText().toUtf8()));

            stream->append(frame);
         }
      }
   }

   return true;
}

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
#include <decoder/NfcIterator.h>

#include "StorageWriter.h"

StorageWriter::StorageWriter(QIODevice *device, QObject *parent) : QObject(parent), mDevice(device)
{
}

StorageWriter::~StorageWriter()
{
   close();
}

bool StorageWriter::open()
{
   if (mDevice->open(QIODevice::WriteOnly | QIODevice::Truncate))
   {
      mWriter.setDevice(mDevice);
      mWriter.setAutoFormatting(true);
      mWriter.writeStartDocument();
      mWriter.writeStartElement("stream");
      mWriter.writeAttribute("frequency", QString("%1").arg(mFrequency));
      mWriter.writeAttribute("sampling", QString("%1").arg(mSampling));

      return true;
   }

   return false;
}

void StorageWriter::close()
{
   if (mDevice && mDevice->isOpen())
   {
      mWriter.writeEndDocument();

      mDevice->close();
   }
}

void StorageWriter::setFrequency(long frequency)
{
   mFrequency = frequency;
}

void StorageWriter::setSampling(long sampling)
{
   mSampling = sampling;
}

bool StorageWriter::writeStream(const NfcStream *stream)
{
   if (mDevice)
   {
      NfcIterator it(stream);

      while(it.hasNext())
      {
         NfcFrame frame = it.next();

         mWriter.writeStartElement("frame");
         mWriter.writeAttribute("start", QString("%1").arg(frame.timeStart(), 0, 'f', 6));
         mWriter.writeAttribute("end", QString("%1").arg(frame.timeEnd(), 0, 'f', 6));
         mWriter.writeAttribute("tech", QString("%1").arg(frame.techType()));
         mWriter.writeAttribute("type", QString("%1").arg(frame.frameType()));
         mWriter.writeAttribute("flags", QString("%1").arg(frame.frameFlags()));
         mWriter.writeAttribute("rate", QString("%1").arg(frame.frameRate()));
         mWriter.writeAttribute("stage", QString("%1").arg(frame.framePhase()));
         mWriter.writeCharacters(frame.toByteArray().toHex());
         mWriter.writeEndElement();
      }

      return true;
   }

   return false;
}

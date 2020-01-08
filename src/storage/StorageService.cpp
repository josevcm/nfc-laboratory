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
#include <QDateTime>
#include <QFileInfo>

#include <Dispatcher.h>

#include <decoder/NfcStream.h>
#include <decoder/NfcIterator.h>

#include <events/StorageControlEvent.h>
#include <events/DecoderControlEvent.h>
#include <events/StreamStatusEvent.h>
#include <events/StreamFrameEvent.h>

#include <storage/StorageReader.h>
#include <storage/StorageWriter.h>

#include "StorageService.h"

StorageService::StorageService(QSettings &settings, NfcStream *stream) : mSettings(settings), mStream(stream)
{
}

StorageService::~StorageService()
{
}

void StorageService::customEvent(QEvent * event)
{
   if (event->type() == StorageControlEvent::Type)
      storageControlEvent(static_cast<StorageControlEvent*>(event));
   else if (event->type() == StreamStatusEvent::Type)
      streamStatusEvent(static_cast<StreamStatusEvent*>(event));
}

void StorageService::storageControlEvent(StorageControlEvent *event)
{
   switch(event->command())
   {
      case StorageControlEvent::Read:
      {
         storageReadCommand(event);
         break;
      }

      case StorageControlEvent::Write:
      {
         storageWriteCommand(event);
         break;
      }
   }
}

void StorageService::streamStatusEvent(StreamStatusEvent *event)
{
   if (event->hasSource())
      mCaptureSource = event->source();

   if (event->hasFrequency())
      mCaptureFrequency = event->frequency();

   if (event->hasSampleRate())
      mCaptureSampleRate = event->sampleRate();
}

void StorageService::storageReadCommand(StorageControlEvent *event)
{
   QString fileName = event->getString("file");

   qInfo() << "storage read" << fileName;

   if (fileName.endsWith(".xml"))
   {
      QFile file(fileName);

      StorageReader reader(&file);

      if (reader.open())
      {
         QFileInfo fileInfo(file);

         mStream->clear();

         Dispatcher::post(StreamStatusEvent::create(StreamStatusEvent::Streaming)->setSource(fileInfo.fileName())->setFrequency(reader.frequency())->setSampleRate(reader.sampling()));

         if (reader.readStream(mStream))
         {
            NfcIterator it(mStream);

            while (it.hasNext())
            {
               Dispatcher::post(new StreamFrameEvent(it.next()));
            }
         }

         Dispatcher::post(StreamStatusEvent::create(StreamStatusEvent::Stopped));

         qInfo() << "storage read completed successful";
      }
   }
   else if (fileName.endsWith(".wav"))
   {
      Dispatcher::post(new DecoderControlEvent(DecoderControlEvent::Start, "source", "record://" + fileName));
   }
}

void StorageService::storageWriteCommand(StorageControlEvent *event)
{
   QString fileName = event->getString("file");

   qInfo() << "storage write" << fileName;

   if (fileName.endsWith(".xml"))
   {
      QFile file(fileName);

      StorageWriter writer(&file);

      writer.setFrequency(mCaptureFrequency);
      writer.setSampling(mCaptureSampleRate);

      if (writer.open())
      {
         if (writer.writeStream(mStream))
         {
            qInfo() << "storage write completed successful";
         }
      }
   }
}

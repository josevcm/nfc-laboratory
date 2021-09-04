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

#include <QJsonArray>
#include <utility>

#include "StorageStatusEvent.h"

const int StorageStatusEvent::Type = QEvent::registerEventType();

const QString StorageStatusEvent::Idle = "idle";
const QString StorageStatusEvent::Reading = "reading";
const QString StorageStatusEvent::Writing = "writing";

StorageStatusEvent::StorageStatusEvent() :
      QEvent(QEvent::Type(Type))
{
}

StorageStatusEvent::StorageStatusEvent(int status) :
      QEvent(QEvent::Type(Type))
{
}

StorageStatusEvent::StorageStatusEvent(QJsonObject data) : QEvent(QEvent::Type(Type)), data(std::move(data))
{
}

bool StorageStatusEvent::isIdle() const
{
   return hasStatus() && status() == Idle;
}

bool StorageStatusEvent::isReading() const
{
   return hasStatus() && status() == Reading;
}

bool StorageStatusEvent::isWriting() const
{
   return hasStatus() && status() == Writing;
}

bool StorageStatusEvent::hasStatus() const
{
   return data.contains("status");
}

QString StorageStatusEvent::status() const
{
   return data["status"].toString();
}

bool StorageStatusEvent::hasFileName() const
{
   return data.contains("file");
}

QString StorageStatusEvent::fileName() const
{
   return data["file"].toString();
}

bool StorageStatusEvent::hasSampleRate() const
{
   return data.contains("sampleRate");
}

int StorageStatusEvent::sampleRate() const
{
   return data["sampleRate"].toInt();
}

bool StorageStatusEvent::hasSampleCount() const
{
   return data.contains("sampleCount");
}

long StorageStatusEvent::sampleCount() const
{
   return data["sampleCount"].toInt();
}

StorageStatusEvent *StorageStatusEvent::create()
{
   return new StorageStatusEvent();
}

StorageStatusEvent *StorageStatusEvent::create(const QJsonObject &data)
{
   return new StorageStatusEvent(data);
}




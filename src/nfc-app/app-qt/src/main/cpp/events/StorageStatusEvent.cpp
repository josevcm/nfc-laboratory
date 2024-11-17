/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2024 Jose Vicente Campos Martinez, <josevcm@gmail.com>

  NFC-LABORATORY is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NFC-LABORATORY is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NFC-LABORATORY. If not, see <http://www.gnu.org/licenses/>.

*/

#include <QJsonArray>
#include <utility>

#include "StorageStatusEvent.h"

const int StorageStatusEvent::Type = registerEventType();

const QString StorageStatusEvent::Reading = "reading";
const QString StorageStatusEvent::Writing = "writing";
const QString StorageStatusEvent::Progress = "progress";
const QString StorageStatusEvent::Complete = "complete";
const QString StorageStatusEvent::Error = "error";

StorageStatusEvent::StorageStatusEvent() : QEvent(static_cast<QEvent::Type>(Type))
{
}

StorageStatusEvent::StorageStatusEvent(int status) : QEvent(static_cast<QEvent::Type>(Type))
{
}

StorageStatusEvent::StorageStatusEvent(QJsonObject data) : QEvent(static_cast<QEvent::Type>(Type)), data(std::move(data))
{
}

bool StorageStatusEvent::isReading() const
{
   return hasStatus() && status() == Reading;
}

bool StorageStatusEvent::isWriting() const
{
   return hasStatus() && status() == Writing;
}

bool StorageStatusEvent::isProgress() const
{
   return hasStatus() && status() == Progress;
}

bool StorageStatusEvent::isComplete() const
{
   return hasStatus() && status() == Complete;
}

bool StorageStatusEvent::isError() const
{
   return hasStatus() && status() == Error;
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

bool StorageStatusEvent::hasStreamTime() const
{
   return data.contains("streamTime");
}

long StorageStatusEvent::streamTime() const
{
   return data["streamTime"].toInt();
}

bool StorageStatusEvent::hasMessage() const
{
   return data.contains("message");
}

QString StorageStatusEvent::message() const
{
   return data["message"].toString();
}

StorageStatusEvent *StorageStatusEvent::create()
{
   return new StorageStatusEvent();
}

StorageStatusEvent *StorageStatusEvent::create(const QJsonObject &data)
{
   return new StorageStatusEvent(data);
}

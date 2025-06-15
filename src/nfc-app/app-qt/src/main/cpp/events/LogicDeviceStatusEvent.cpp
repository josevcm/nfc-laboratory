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

#include "LogicDeviceStatusEvent.h"

const int LogicDeviceStatusEvent::Type = registerEventType();

const QString LogicDeviceStatusEvent::Absent = "absent";
const QString LogicDeviceStatusEvent::Idle = "idle";
const QString LogicDeviceStatusEvent::Streaming = "streaming";
const QString LogicDeviceStatusEvent::Paused = "paused";
const QString LogicDeviceStatusEvent::Disabled = "disabled";

LogicDeviceStatusEvent::LogicDeviceStatusEvent() : QEvent(static_cast<QEvent::Type>(Type))
{
}

LogicDeviceStatusEvent::LogicDeviceStatusEvent(int status) : QEvent(static_cast<QEvent::Type>(Type))
{
}

LogicDeviceStatusEvent::LogicDeviceStatusEvent(QJsonObject data) : QEvent(static_cast<QEvent::Type>(Type)), data(std::move(data))
{
}

const QJsonObject &LogicDeviceStatusEvent::content() const
{
   return data;
}

bool LogicDeviceStatusEvent::isAbsent() const
{
   return hasStatus() && status() == Absent;
}

bool LogicDeviceStatusEvent::isIdle() const
{
   return hasStatus() && status() == Idle;
}

bool LogicDeviceStatusEvent::isPaused() const
{
   return hasStatus() && status() == Paused;
}

bool LogicDeviceStatusEvent::isStreaming() const
{
   return hasStatus() && status() == Streaming;
}

bool LogicDeviceStatusEvent::isDisabled() const
{
   return hasStatus() && status() == Disabled;
}

bool LogicDeviceStatusEvent::hasStatus() const
{
   return data.contains("status");
}

QString LogicDeviceStatusEvent::status() const
{
   return data["status"].toString();
}

bool LogicDeviceStatusEvent::hasName() const
{
   return data.contains("name");
}

QString LogicDeviceStatusEvent::name() const
{
   return data["name"].toString();
}

bool LogicDeviceStatusEvent::hasVendor() const
{
   return data.contains("vendor");
}

QString LogicDeviceStatusEvent::vendor() const
{
   return data["vendor"].toString();
}

bool LogicDeviceStatusEvent::hasModel() const
{
   return data.contains("model");
}

QString LogicDeviceStatusEvent::model() const
{
   return data["model"].toString();
}

bool LogicDeviceStatusEvent::hasSerial() const
{
   return data.contains("serial");
}

QString LogicDeviceStatusEvent::serial() const
{
   return data["serial"].toString();
}

bool LogicDeviceStatusEvent::hasSampleRate() const
{
   return data.contains("sampleRate");
}

int LogicDeviceStatusEvent::sampleRate() const
{
   return data["sampleRate"].toInt();
}

bool LogicDeviceStatusEvent::hasSampleCount() const
{
   return data.contains("samplesRead");
}

long long LogicDeviceStatusEvent::sampleCount() const
{
   return data["samplesRead"].toInteger();
}

bool LogicDeviceStatusEvent::hasStreamTime() const
{
   return data.contains("streamTime");
}

long LogicDeviceStatusEvent::streamTime() const
{
   return data["streamTime"].toInt();
}

bool LogicDeviceStatusEvent::hasStreamProgress() const
{
   return false; //mInfo & StreamProgress;
}

float LogicDeviceStatusEvent::streamProgress() const
{
   return 0;
}

bool LogicDeviceStatusEvent::hasDeviceList() const
{
   return false; //mInfo & DeviceList;
}

QStringList LogicDeviceStatusEvent::deviceList() const
{
   QStringList result;

   return result;
}

bool LogicDeviceStatusEvent::hasSampleRateList() const
{
   return data.contains("sampleRates");
}

QMap<int, QString> LogicDeviceStatusEvent::sampleRateList() const
{
   QMap<int, QString> result;

   for (auto entry: data["sampleRates"].toArray())
   {
      auto item = entry.toObject();

      result[item["value"].toInt()] = item["name"].toString();
   }

   return result;
}

LogicDeviceStatusEvent *LogicDeviceStatusEvent::create()
{
   return new LogicDeviceStatusEvent();
}

LogicDeviceStatusEvent *LogicDeviceStatusEvent::create(const QJsonObject &data)
{
   return new LogicDeviceStatusEvent(data);
}




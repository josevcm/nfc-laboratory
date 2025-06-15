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

#include "RadioDeviceStatusEvent.h"

const int RadioDeviceStatusEvent::Type = registerEventType();

const QString RadioDeviceStatusEvent::Absent = "absent";
const QString RadioDeviceStatusEvent::Idle = "idle";
const QString RadioDeviceStatusEvent::Paused = "paused";
const QString RadioDeviceStatusEvent::Streaming = "streaming";
const QString RadioDeviceStatusEvent::Disabled = "disabled";

RadioDeviceStatusEvent::RadioDeviceStatusEvent() : QEvent(static_cast<QEvent::Type>(Type))
{
}

RadioDeviceStatusEvent::RadioDeviceStatusEvent(int status) : QEvent(static_cast<QEvent::Type>(Type))
{
}

RadioDeviceStatusEvent::RadioDeviceStatusEvent(QJsonObject data) : QEvent(static_cast<QEvent::Type>(Type)), data(std::move(data))
{
}

const QJsonObject &RadioDeviceStatusEvent::content() const
{
   return data;
}

bool RadioDeviceStatusEvent::isAbsent() const
{
   return hasStatus() && status() == Absent;
}

bool RadioDeviceStatusEvent::isIdle() const
{
   return hasStatus() && status() == Idle;
}

bool RadioDeviceStatusEvent::isPaused() const
{
   return hasStatus() && status() == Paused;
}

bool RadioDeviceStatusEvent::isStreaming() const
{
   return hasStatus() && status() == Streaming;
}

bool RadioDeviceStatusEvent::isDisabled() const
{
   return hasStatus() && status() == Disabled;
}

bool RadioDeviceStatusEvent::hasStatus() const
{
   return data.contains("status");
}

QString RadioDeviceStatusEvent::status() const
{
   return data["status"].toString();
}

bool RadioDeviceStatusEvent::hasName() const
{
   return data.contains("name");
}

QString RadioDeviceStatusEvent::name() const
{
   return data["name"].toString();
}

bool RadioDeviceStatusEvent::hasVendor() const
{
   return data.contains("vendor");
}

QString RadioDeviceStatusEvent::vendor() const
{
   return data["vendor"].toString();
}

bool RadioDeviceStatusEvent::hasModel() const
{
   return data.contains("model");
}

QString RadioDeviceStatusEvent::model() const
{
   return data["model"].toString();
}

bool RadioDeviceStatusEvent::hasSerial() const
{
   return data.contains("serial");
}

QString RadioDeviceStatusEvent::serial() const
{
   return data["serial"].toString();
}

bool RadioDeviceStatusEvent::hasCenterFreq() const
{
   return data.contains("centerFreq");
}

int RadioDeviceStatusEvent::centerFreq() const
{
   return data["centerFreq"].toInt();
}

bool RadioDeviceStatusEvent::hasSampleRate() const
{
   return data.contains("sampleRate");
}

int RadioDeviceStatusEvent::sampleRate() const
{
   return data["sampleRate"].toInt();
}

bool RadioDeviceStatusEvent::hasSampleCount() const
{
   return data.contains("samplesRead");
}

long long RadioDeviceStatusEvent::sampleCount() const
{
   return data["samplesRead"].toInteger();
}

bool RadioDeviceStatusEvent::hasStreamTime() const
{
   return data.contains("streamTime");
}

long RadioDeviceStatusEvent::streamTime() const
{
   return data["streamTime"].toInt();
}

bool RadioDeviceStatusEvent::hasGainMode() const
{
   return data.contains("gainMode");
}

int RadioDeviceStatusEvent::gainMode() const
{
   return data["gainMode"].toInt();
}

bool RadioDeviceStatusEvent::hasGainValue() const
{
   return data.contains("gainValue");
}

int RadioDeviceStatusEvent::gainValue() const
{
   return data["gainValue"].toInt();
}

bool RadioDeviceStatusEvent::hasTunerAgc() const
{
   return data.contains("tunerAgc");
}

int RadioDeviceStatusEvent::tunerAgc() const
{
   return data["tunerAgc"].toInt();
}

bool RadioDeviceStatusEvent::hasMixerAgc() const
{
   return data.contains("mixerAgc");
}

int RadioDeviceStatusEvent::mixerAgc() const
{
   return data["mixerAgc"].toInt();
}

bool RadioDeviceStatusEvent::hasBiasTee() const
{
   return data.contains("biasTee");
}

int RadioDeviceStatusEvent::biasTee() const
{
   return data["biasTee"].toInt();
}

bool RadioDeviceStatusEvent::hasDirectSampling() const
{
   return data.contains("directSampling");
}

int RadioDeviceStatusEvent::directSampling() const
{
   return data["directSampling"].toInt();
}

bool RadioDeviceStatusEvent::hasSignalPower() const
{
   return false; //mInfo & SignalPower;
}

float RadioDeviceStatusEvent::signalPower() const
{
   return 0;
}

bool RadioDeviceStatusEvent::hasStreamProgress() const
{
   return false; //mInfo & StreamProgress;
}

float RadioDeviceStatusEvent::streamProgress() const
{
   return 0;
}

bool RadioDeviceStatusEvent::hasDeviceList() const
{
   return false; //mInfo & DeviceList;
}

QStringList RadioDeviceStatusEvent::deviceList() const
{
   QStringList result;

   return result;
}

bool RadioDeviceStatusEvent::hasSampleRateList() const
{
   return data.contains("sampleRates");
}

QMap<int, QString> RadioDeviceStatusEvent::sampleRateList() const
{
   QMap<int, QString> result;

   for (auto entry: data["sampleRates"].toArray())
   {
      auto item = entry.toObject();

      result[item["value"].toInt()] = item["name"].toString();
   }

   return result;
}

bool RadioDeviceStatusEvent::hasGainModeList() const
{
   return data.contains("gainModes");
}

QMap<int, QString> RadioDeviceStatusEvent::gainModeList() const
{
   QMap<int, QString> result;

   for (auto entry: data["gainModes"].toArray())
   {
      auto item = entry.toObject();

      result[item["value"].toInt()] = item["name"].toString();
   }

   return result;
}

bool RadioDeviceStatusEvent::hasGainValueList() const
{
   return data.contains("gainValues");
}

QMap<int, QString> RadioDeviceStatusEvent::gainValueList() const
{
   QMap<int, QString> result;

   for (auto entry: data["gainValues"].toArray())
   {
      auto item = entry.toObject();

      result[item["value"].toInt()] = item["name"].toString();
   }

   return result;
}

RadioDeviceStatusEvent *RadioDeviceStatusEvent::create()
{
   return new RadioDeviceStatusEvent();
}

RadioDeviceStatusEvent *RadioDeviceStatusEvent::create(const QJsonObject &data)
{
   return new RadioDeviceStatusEvent(data);
}




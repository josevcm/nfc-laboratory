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

#include "ReceiverStatusEvent.h"

const int ReceiverStatusEvent::Type = QEvent::registerEventType();

const QString ReceiverStatusEvent::NoDevice = "absent";
const QString ReceiverStatusEvent::Idle = "idle";
const QString ReceiverStatusEvent::Streaming = "streaming";

ReceiverStatusEvent::ReceiverStatusEvent() : QEvent(QEvent::Type(Type))
{
}

ReceiverStatusEvent::ReceiverStatusEvent(int status) : QEvent(QEvent::Type(Type))
{
}

ReceiverStatusEvent::ReceiverStatusEvent(QJsonObject data) : QEvent(QEvent::Type(Type)), data(std::move(data))
{
}

bool ReceiverStatusEvent::hasReceiverStatus() const
{
   return data.contains("status");
}

QString ReceiverStatusEvent::status() const
{
   return data["status"].toString();
}

bool ReceiverStatusEvent::hasReceiverName() const
{
   return data.contains("name");
}

QString ReceiverStatusEvent::source() const
{
   return data["name"].toString();
}

bool ReceiverStatusEvent::hasCenterFreq() const
{
   return data.contains("centerFreq");
}

int ReceiverStatusEvent::centerFreq() const
{
   return data["centerFreq"].toInt();
}

bool ReceiverStatusEvent::hasSampleRate() const
{
   return data.contains("sampleRate");
}

int ReceiverStatusEvent::sampleRate() const
{
   return data["sampleRate"].toInt();
}

bool ReceiverStatusEvent::hasSampleCount() const
{
   return data.contains("samplesReceived");
}

long ReceiverStatusEvent::sampleCount() const
{
   return data["samplesReceived"].toInt();
}

bool ReceiverStatusEvent::hasGainMode() const
{
   return false; //mInfo & GainMode;
}

int ReceiverStatusEvent::gainMode() const
{
   return 0;
}

bool ReceiverStatusEvent::hasGainValue() const
{
   return data.contains("gainValue");
}

int ReceiverStatusEvent::gainValue() const
{
   return data["gainValue"].toInt();
}

bool ReceiverStatusEvent::hasTunerAgc() const
{
   return data.contains("tunerAgc");
}

int ReceiverStatusEvent::tunerAgc() const
{
   return data["tunerAgc"].toInt();
}

bool ReceiverStatusEvent::hasMixerAgc() const
{
   return data.contains("mixerAgc");
}

int ReceiverStatusEvent::mixerAgc() const
{
   return data["mixerAgc"].toInt();
}

bool ReceiverStatusEvent::hasSignalPower() const
{
   return false; //mInfo & SignalPower;
}

float ReceiverStatusEvent::signalPower() const
{
   return 0;
}

bool ReceiverStatusEvent::hasStreamProgress() const
{
   return false; //mInfo & StreamProgress;
}

float ReceiverStatusEvent::streamProgress() const
{
   return 0;
}

bool ReceiverStatusEvent::hasDeviceList() const
{
   return false; //mInfo & DeviceList;
}

QStringList ReceiverStatusEvent::deviceList() const
{
   QStringList result;

   return result;
}

bool ReceiverStatusEvent::hasSampeRateList() const
{
   return data.contains("sampleRates");
}

QMap<int, QString> ReceiverStatusEvent::sampleRateList() const
{
   QMap<int, QString> result;

   for (auto entry: data["sampleRates"].toArray())
   {
      auto item = entry.toObject();

      result[item["value"].toInt()] = item["name"].toString();
   }

   return result;
}

bool ReceiverStatusEvent::hasGainModeList() const
{
   return data.contains("gainModes");
}

QMap<int, QString> ReceiverStatusEvent::gainModeList() const
{
   QMap<int, QString> result;

   for (auto entry: data["gainModes"].toArray())
   {
      auto item = entry.toObject();

      result[item["value"].toInt()] = item["name"].toString();
   }

   return result;
}

bool ReceiverStatusEvent::hasGainValueList() const
{
   return data.contains("gainValues");
}

QMap<int, QString> ReceiverStatusEvent::gainValueList() const
{
   QMap<int, QString> result;

   for (auto entry: data["gainValues"].toArray())
   {
      auto item = entry.toObject();

      result[item["value"].toInt()] = item["name"].toString();
   }

   return result;
}

ReceiverStatusEvent *ReceiverStatusEvent::create()
{
   return new ReceiverStatusEvent();
}

ReceiverStatusEvent *ReceiverStatusEvent::create(const QJsonObject &data)
{
   return new ReceiverStatusEvent(data);
}




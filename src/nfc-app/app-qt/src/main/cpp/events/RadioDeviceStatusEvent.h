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

#ifndef APP_RADIODEVICESTATUSEVENT_H
#define APP_RADIODEVICESTATUSEVENT_H

#include <QEvent>
#include <QMap>
#include <QVector>
#include <QJsonObject>

class RadioDeviceStatusEvent : public QEvent
{
   public:

      static const int Type;

      static const QString Absent;
      static const QString Idle;
      static const QString Streaming;
      static const QString Disabled;

   public:

      RadioDeviceStatusEvent();

      explicit RadioDeviceStatusEvent(int status);

      explicit RadioDeviceStatusEvent(QJsonObject data);

      const QJsonObject &content() const;

      bool isAbsent() const;

      bool isIdle() const;

      bool isStreaming() const;

      bool isDisabled() const;

      bool hasStatus() const;

      QString status() const;

      bool hasName() const;

      QString name() const;

      bool hasVendor() const;

      QString vendor() const;

      bool hasModel() const;

      QString model() const;

      bool hasSerial() const;

      QString serial() const;

      bool hasCenterFreq() const;

      int centerFreq() const;

      bool hasSampleRate() const;

      int sampleRate() const;

      bool hasSampleCount() const;

      long long sampleCount() const;

      bool hasStreamTime() const;

      long streamTime() const;

      bool hasGainMode() const;

      int gainMode() const;

      bool hasGainValue() const;

      int gainValue() const;

      bool hasTunerAgc() const;

      int tunerAgc() const;

      bool hasMixerAgc() const;

      int mixerAgc() const;

      bool hasBiasTee() const;

      int biasTee() const;

      bool hasDirectSampling() const;

      int directSampling() const;

      bool hasSignalPower() const;

      float signalPower() const;

      bool hasStreamProgress() const;

      float streamProgress() const;

      bool hasDeviceList() const;

      QStringList deviceList() const;

      bool hasSampleRateList() const;

      QMap<int, QString> sampleRateList() const;

      bool hasGainModeList() const;

      QMap<int, QString> gainModeList() const;

      bool hasGainValueList() const;

      QMap<int, QString> gainValueList() const;

      static RadioDeviceStatusEvent *create();

      static RadioDeviceStatusEvent *create(const QJsonObject &data);

   private:

      QJsonObject data;
};

#endif

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

#ifndef APP_LOGICDEVICESTATUSEVENT_H
#define APP_LOGICDEVICESTATUSEVENT_H

#include <QEvent>
#include <QMap>
#include <QVector>
#include <QJsonObject>

class LogicDeviceStatusEvent : public QEvent
{
   public:

      static const int Type;

      static const QString Absent;
      static const QString Idle;
      static const QString Paused;
      static const QString Streaming;
      static const QString Disabled;

   public:

      LogicDeviceStatusEvent();

      explicit LogicDeviceStatusEvent(int status);

      explicit LogicDeviceStatusEvent(QJsonObject data);

      const QJsonObject &content() const;

      bool isAbsent() const;

      bool isIdle() const;

      bool isPaused() const;

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

      bool hasSampleRate() const;

      int sampleRate() const;

      bool hasSampleCount() const;

      long long sampleCount() const;

      bool hasStreamTime() const;

      long streamTime() const;

      bool hasStreamProgress() const;

      float streamProgress() const;

      bool hasDeviceList() const;

      QStringList deviceList() const;

      bool hasSampleRateList() const;

      QMap<int, QString> sampleRateList() const;

      static LogicDeviceStatusEvent *create();

      static LogicDeviceStatusEvent *create(const QJsonObject &data);

   private:

      QJsonObject data;
};

#endif

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

#ifndef APP_FOURIERSTATUSEVENT_H
#define APP_FOURIERSTATUSEVENT_H

#include <QEvent>
#include <QMap>
#include <QVector>
#include <QStringList>
#include <QJsonObject>

class FourierStatusEvent : public QEvent
{
   public:

      static const int Type;

      static const QString Idle;
      static const QString Streaming;
      static const QString Disabled;

   public:

      FourierStatusEvent();

      explicit FourierStatusEvent(int status);

      explicit FourierStatusEvent(QJsonObject data);

      bool isAbsent() const;

      bool isIdle() const;

      bool isStreaming() const;

      bool isDisabled() const;

      bool hasStatus() const;

      QString status() const;

      static FourierStatusEvent *create();

      static FourierStatusEvent *create(const QJsonObject &data);

   private:

      QJsonObject data;
};

#endif

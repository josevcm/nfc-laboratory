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

#ifndef NFC_LAB_LOGICDECODERSTATUSEVENT_H
#define NFC_LAB_LOGICDECODERSTATUSEVENT_H

#include <QEvent>
#include <QJsonObject>

class LogicDecoderStatusEvent : public QEvent
{
   public:

      static const int Type;

      static const QString Idle;
      static const QString Decoding;
      static const QString Disabled;

   public:

      LogicDecoderStatusEvent();

      explicit LogicDecoderStatusEvent(int status);

      explicit LogicDecoderStatusEvent(QJsonObject data);

      const QJsonObject &content() const;

      bool hasStatus() const;

      QString status() const;

      bool hasIso7816() const;

      QJsonObject iso7816() const;

      static LogicDecoderStatusEvent *create();

      static LogicDecoderStatusEvent *create(const QJsonObject &data);

   private:

      QJsonObject data;
};

#endif

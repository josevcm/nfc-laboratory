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

#include "LogicDecoderStatusEvent.h"

const int LogicDecoderStatusEvent::Type = registerEventType();

const QString LogicDecoderStatusEvent::Idle = "idle";
const QString LogicDecoderStatusEvent::Decoding = "decoding";
const QString LogicDecoderStatusEvent::Disabled = "disabled";

LogicDecoderStatusEvent::LogicDecoderStatusEvent() : QEvent(static_cast<QEvent::Type>(Type))
{
}

LogicDecoderStatusEvent::LogicDecoderStatusEvent(int status) : QEvent(static_cast<QEvent::Type>(Type))
{
}

LogicDecoderStatusEvent::LogicDecoderStatusEvent(QJsonObject data) : QEvent(static_cast<QEvent::Type>(Type)), data(std::move(data))
{
}

const QJsonObject &LogicDecoderStatusEvent::content() const
{
   return data;
}

bool LogicDecoderStatusEvent::hasStatus() const
{
   return data.contains("status");
}

QString LogicDecoderStatusEvent::status() const
{
   return data["status"].toString();
}

bool LogicDecoderStatusEvent::hasIso7816() const
{
   return !data["protocol"]["iso7816"].isUndefined();
}

QJsonObject LogicDecoderStatusEvent::iso7816() const
{
   return data["protocol"]["iso7816"].toObject();
}

LogicDecoderStatusEvent *LogicDecoderStatusEvent::create()
{
   return new LogicDecoderStatusEvent();
}

LogicDecoderStatusEvent *LogicDecoderStatusEvent::create(const QJsonObject &data)
{
   return new LogicDecoderStatusEvent(data);
}

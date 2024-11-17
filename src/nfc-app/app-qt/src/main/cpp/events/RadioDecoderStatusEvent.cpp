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

#include "RadioDecoderStatusEvent.h"

const int RadioDecoderStatusEvent::Type = registerEventType();

const QString RadioDecoderStatusEvent::Idle = "idle";
const QString RadioDecoderStatusEvent::Decoding = "decoding";
const QString RadioDecoderStatusEvent::Disabled = "disabled";

RadioDecoderStatusEvent::RadioDecoderStatusEvent() : QEvent(static_cast<QEvent::Type>(Type))
{
}

RadioDecoderStatusEvent::RadioDecoderStatusEvent(int status) : QEvent(static_cast<QEvent::Type>(Type))
{
}

RadioDecoderStatusEvent::RadioDecoderStatusEvent(QJsonObject data) : QEvent(static_cast<QEvent::Type>(Type)), data(std::move(data))
{
}

const QJsonObject &RadioDecoderStatusEvent::content() const
{
   return data;
}

bool RadioDecoderStatusEvent::hasStatus() const
{
   return data.contains("status");
}

QString RadioDecoderStatusEvent::status() const
{
   return data["status"].toString();
}

bool RadioDecoderStatusEvent::hasNfcA() const
{
   return !data["protocol"]["nfca"].isUndefined();
}

QJsonObject RadioDecoderStatusEvent::nfca() const
{
   return data["protocol"]["nfca"].toObject();
}

bool RadioDecoderStatusEvent::hasNfcB() const
{
   return !data["protocol"]["nfcb"].isUndefined();
}

QJsonObject RadioDecoderStatusEvent::nfcb() const
{
   return data["protocol"]["nfcb"].toObject();
}

bool RadioDecoderStatusEvent::hasNfcF() const
{
   return !data["protocol"]["nfcf"].isUndefined();
}

QJsonObject RadioDecoderStatusEvent::nfcf() const
{
   return data["protocol"]["nfcf"].toObject();
}

bool RadioDecoderStatusEvent::hasNfcV() const
{
   return !data["protocol"]["nfcv"].isUndefined();
}

QJsonObject RadioDecoderStatusEvent::nfcv() const
{
   return data["protocol"]["nfcv"].toObject();
}

RadioDecoderStatusEvent *RadioDecoderStatusEvent::create()
{
   return new RadioDecoderStatusEvent();
}

RadioDecoderStatusEvent *RadioDecoderStatusEvent::create(const QJsonObject &data)
{
   return new RadioDecoderStatusEvent(data);
}

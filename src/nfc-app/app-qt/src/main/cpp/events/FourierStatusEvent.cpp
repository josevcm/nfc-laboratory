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

#include "FourierStatusEvent.h"

const int FourierStatusEvent::Type = registerEventType();

const QString FourierStatusEvent::Idle = "idle";
const QString FourierStatusEvent::Streaming = "streaming";
const QString FourierStatusEvent::Disabled = "disabled";

FourierStatusEvent::FourierStatusEvent() : QEvent(static_cast<QEvent::Type>(Type))
{
}

FourierStatusEvent::FourierStatusEvent(int status) : QEvent(static_cast<QEvent::Type>(Type))
{
}

FourierStatusEvent::FourierStatusEvent(QJsonObject data) : QEvent(static_cast<QEvent::Type>(Type)), data(std::move(data))
{
}

bool FourierStatusEvent::isIdle() const
{
   return hasStatus() && status() == Idle;
}

bool FourierStatusEvent::isStreaming() const
{
   return hasStatus() && status() == Streaming;
}

bool FourierStatusEvent::isDisabled() const
{
   return hasStatus() && status() == Disabled;
}

bool FourierStatusEvent::hasStatus() const
{
   return data.contains("status");
}

QString FourierStatusEvent::status() const
{
   return data["status"].toString();
}

FourierStatusEvent *FourierStatusEvent::create()
{
   return new FourierStatusEvent();
}

FourierStatusEvent *FourierStatusEvent::create(const QJsonObject &data)
{
   return new FourierStatusEvent(data);
}




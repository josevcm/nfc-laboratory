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

#include "DecoderControlEvent.h"

#include <utility>

int DecoderControlEvent::Type = registerEventType();

DecoderControlEvent::DecoderControlEvent(int command) : QEvent(static_cast<QEvent::Type>(Type)), mCommand(command)
{
}

DecoderControlEvent::DecoderControlEvent(int command, QMap<QString, QVariant> parameters) : QEvent(static_cast<QEvent::Type>(Type)), mCommand(command), mParameters(std::move(parameters))
{
}

DecoderControlEvent::DecoderControlEvent(int command, const QString &name, int value) : QEvent(static_cast<QEvent::Type>(Type)), mCommand(command)
{
   mParameters[name] = value;
}

DecoderControlEvent::DecoderControlEvent(int command, const QString &name, float value) : QEvent(static_cast<QEvent::Type>(Type)), mCommand(command)
{
   mParameters[name] = value;
}

DecoderControlEvent::DecoderControlEvent(int command, const QString &name, bool value) : QEvent(static_cast<QEvent::Type>(Type)), mCommand(command)
{
   mParameters[name] = value;
}

DecoderControlEvent::DecoderControlEvent(int command, const QString &name, const QString &value) : QEvent(static_cast<QEvent::Type>(Type)), mCommand(command)
{
   mParameters[name] = value;
}

int DecoderControlEvent::command() const
{
   return mCommand;
}

bool DecoderControlEvent::contains(const QString &name) const
{
   return mParameters.contains(name);
}

const QMap<QString, QVariant> &DecoderControlEvent::parameters() const
{
   return mParameters;
}

DecoderControlEvent *DecoderControlEvent::setInteger(const QString &name, int value)
{
   mParameters[name] = value;

   return this;
}

int DecoderControlEvent::getInteger(const QString &name, int defVal) const
{
   if (!mParameters.contains(name))
      return defVal;

   return mParameters[name].toInt();
}

DecoderControlEvent *DecoderControlEvent::setFloat(const QString &name, float value)
{
   mParameters[name] = value;

   return this;
}

float DecoderControlEvent::getFloat(const QString &name, float defVal) const
{
   if (!mParameters.contains(name))
      return defVal;

   return mParameters[name].toFloat();
}

DecoderControlEvent *DecoderControlEvent::setDouble(const QString &name, double value)
{
   mParameters[name] = value;

   return this;
}

double DecoderControlEvent::getDouble(const QString &name, double defVal) const
{
   if (!mParameters.contains(name))
      return defVal;

   return mParameters[name].toDouble();
}

DecoderControlEvent *DecoderControlEvent::setBoolean(const QString &name, bool value)
{
   mParameters[name] = value;

   return this;
}

bool DecoderControlEvent::getBoolean(const QString &name, bool defVal) const
{
   if (!mParameters.contains(name))
      return defVal;

   return mParameters[name].toBool();
}

DecoderControlEvent *DecoderControlEvent::setString(const QString &name, const QString &value)
{
   mParameters[name] = value;

   return this;
}

QString DecoderControlEvent::getString(const QString &name, const QString &defVal) const
{
   if (!mParameters.contains(name))
      return defVal;

   return mParameters[name].toString();
}


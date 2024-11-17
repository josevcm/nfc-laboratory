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

#include "StorageControlEvent.h"

int StorageControlEvent::Type = registerEventType();

StorageControlEvent::StorageControlEvent(int command) : QEvent(static_cast<QEvent::Type>(Type)), mCommand(command)
{
}

StorageControlEvent::StorageControlEvent(int command, QMap<QString, QVariant> parameters) : QEvent(static_cast<QEvent::Type>(Type)), mCommand(command), mParameters(parameters)
{
}

StorageControlEvent::StorageControlEvent(int command, const QString &name, int value) : QEvent(static_cast<QEvent::Type>(Type)), mCommand(command)
{
   mParameters[name] = value;
}

StorageControlEvent::StorageControlEvent(int command, const QString &name, float value) : QEvent(static_cast<QEvent::Type>(Type)), mCommand(command)
{
   mParameters[name] = value;
}

StorageControlEvent::StorageControlEvent(int command, const QString &name, bool value) : QEvent(static_cast<QEvent::Type>(Type)), mCommand(command)
{
   mParameters[name] = value;
}

StorageControlEvent::StorageControlEvent(int command, const QString &name, const QString &value) : QEvent(static_cast<QEvent::Type>(Type)), mCommand(command)
{
   mParameters[name] = value;
}

int StorageControlEvent::command()
{
   return mCommand;
}

bool StorageControlEvent::isReadCommand()
{
   return mCommand == Command::Read;
}

bool StorageControlEvent::isWriteCommand()
{
   return mCommand == Command::Write;
}

StorageControlEvent *StorageControlEvent::setInteger(const QString &name, int value)
{
   mParameters[name] = value;

   return this;
}

int StorageControlEvent::getInteger(const QString &name)
{
   return mParameters[name].toInt();
}

StorageControlEvent *StorageControlEvent::setFloat(const QString &name, float value)
{
   mParameters[name] = value;

   return this;
}

float StorageControlEvent::getFloat(const QString &name)
{
   return mParameters[name].toFloat();
}

StorageControlEvent *StorageControlEvent::setBoolean(const QString &name, bool value)
{
   mParameters[name] = value;

   return this;
}

bool StorageControlEvent::getBoolean(const QString &name)
{
   return mParameters[name].toBool();
}

StorageControlEvent *StorageControlEvent::setString(const QString &name, const QString &value)
{
   mParameters[name] = value;

   return this;
}

QString StorageControlEvent::getString(const QString &name)
{
   return mParameters[name].toString();
}

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

#ifndef APP_STORAGECONTROLEVENT_H
#define APP_STORAGECONTROLEVENT_H

#include <QEvent>
#include <QMap>
#include <QVariant>

class StorageControlEvent : public QEvent
{
   public:

      static int Type;

      enum Command
      {
         Read, Write
      };

   private:

      int mCommand;

      QMap<QString, QVariant> mParameters;


   public:

      explicit StorageControlEvent(int command);

      StorageControlEvent(int command, QMap<QString, QVariant> parameters);

      StorageControlEvent(int command, const QString &name, int value);

      StorageControlEvent(int command, const QString &name, float value);

      StorageControlEvent(int command, const QString &name, bool value);

      StorageControlEvent(int command, const QString &name, const QString &value);

      int command();

      bool isReadCommand();

      bool isWriteCommand();

      StorageControlEvent *setInteger(const QString &name, int value);

      int getInteger(const QString &name);

      StorageControlEvent *setFloat(const QString &name, float value);

      float getFloat(const QString &name);

      StorageControlEvent *setBoolean(const QString &name, bool value);

      bool getBoolean(const QString &name);

      StorageControlEvent *setString(const QString &name, const QString &value);

      QString getString(const QString &name);

};

#endif // APP_STORAGECONTROLEVENT_H

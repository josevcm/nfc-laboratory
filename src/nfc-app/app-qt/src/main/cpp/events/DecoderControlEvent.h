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

#ifndef APP_DECODECONTROLEVENT_H
#define APP_DECODECONTROLEVENT_H

#include <QEvent>
#include <QMap>
#include <QVariant>

class DecoderControlEvent : public QEvent
{
   public:

      static int Type;

      enum Command
      {
         Start,
         Stop,
         Pause,
         Resume,
         Clear,
         Change,
         ReadFile,
         WriteFile,
         QueryStream,
         LogicDeviceConfig,
         LogicDecoderConfig,
         RadioDeviceConfig,
         RadioDecoderConfig,
         FourierConfig,
      };

   public:

      explicit DecoderControlEvent(int command);

      explicit DecoderControlEvent(int command, QMap<QString, QVariant> parameters);

      explicit DecoderControlEvent(int command, const QString &name, int value);

      explicit DecoderControlEvent(int command, const QString &name, float value);

      explicit DecoderControlEvent(int command, const QString &name, bool value);

      explicit DecoderControlEvent(int command, const QString &name, const QString &value);

      int command() const;

      bool contains(const QString &name) const;

      const QMap<QString, QVariant> &parameters() const;

      DecoderControlEvent *setInteger(const QString &name, int value);

      int getInteger(const QString &name, int defVal = 0) const;

      DecoderControlEvent *setFloat(const QString &name, float value);

      float getFloat(const QString &name, float defVal = 0.0) const;

      DecoderControlEvent *setDouble(const QString &name, double value);

      double getDouble(const QString &name, double defVal = 0.0) const;

      DecoderControlEvent *setBoolean(const QString &name, bool value);

      bool getBoolean(const QString &name, bool defVal = false) const;

      DecoderControlEvent *setString(const QString &name, const QString &value);

      QString getString(const QString &name, const QString &defVal = {}) const;

   private:

      int mCommand;

      QMap<QString, QVariant> mParameters;
};

#endif // DECODECONTROLEVENT_H

/*

  Copyright (c) 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

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
         ReceiverDecode,
         ReceiverRecord,
         StopDecode,
         ReadFile,
         WriteFile,
         DecoderConfig,
         ReceiverConfig,
         Start,
         Stop,
         Record,
         Store,
         Clear,
         Change
      };

   public:

      explicit DecoderControlEvent(int command);

      explicit DecoderControlEvent(int command, QMap<QString, QVariant> parameters);

      explicit DecoderControlEvent(int command, const QString &name, int value);

      explicit DecoderControlEvent(int command, const QString &name, float value);

      explicit DecoderControlEvent(int command, const QString &name, bool value);

      explicit DecoderControlEvent(int command, const QString &name, const QString &value);

      int command() const;

      bool isStartCommand() const;

      bool isStopCommand() const;

      bool isRecordCommand() const;

      bool isChangeCommand() const;

      bool isClearCommand() const;

      bool contains(const QString &name) const;

      DecoderControlEvent *setInteger(const QString &name, int value);

      int getInteger(const QString &name) const;

      DecoderControlEvent *setFloat(const QString &name, float value);

      float getFloat(const QString &name) const;

      DecoderControlEvent *setBoolean(const QString &name, bool value);

      bool getBoolean(const QString &name) const;

      DecoderControlEvent *setString(const QString &name, const QString &value);

      QString getString(const QString &name) const;

   private:

      int mCommand;

      QMap<QString, QVariant> mParameters;
};

#endif // DECODECONTROLEVENT_H

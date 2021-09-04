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

#include "DecoderControlEvent.h"

#include <utility>

int DecoderControlEvent::Type = QEvent::registerEventType();

DecoderControlEvent::DecoderControlEvent(int command) :
      QEvent(QEvent::Type(Type)), mCommand(command)
{
}

DecoderControlEvent::DecoderControlEvent(int command, QMap<QString, QVariant> parameters) :
      QEvent(QEvent::Type(Type)), mCommand(command), mParameters(std::move(parameters))
{
}

DecoderControlEvent::DecoderControlEvent(int command, const QString &name, int value) :
      QEvent(QEvent::Type(Type)), mCommand(command)
{
   mParameters[name] = value;
}

DecoderControlEvent::DecoderControlEvent(int command, const QString &name, float value) :
      QEvent(QEvent::Type(Type)), mCommand(command)
{
   mParameters[name] = value;
}

DecoderControlEvent::DecoderControlEvent(int command, const QString &name, bool value) :
      QEvent(QEvent::Type(Type)), mCommand(command)
{
   mParameters[name] = value;
}

DecoderControlEvent::DecoderControlEvent(int command, const QString &name, const QString &value) :
      QEvent(QEvent::Type(Type)), mCommand(command)
{
   mParameters[name] = value;
}

int DecoderControlEvent::command() const
{
   return mCommand;
}

bool DecoderControlEvent::isStartCommand() const
{
   return mCommand == Command::Start;
}

bool DecoderControlEvent::isStopCommand() const
{
   return mCommand == Command::Stop;
}

bool DecoderControlEvent::isRecordCommand() const
{
   return mCommand == Command::Record;
}

bool DecoderControlEvent::isChangeCommand() const
{
   return mCommand == Command::Change;
}

bool DecoderControlEvent::isClearCommand() const
{
   return mCommand == Command::Clear;
}

bool DecoderControlEvent::contains(const QString &name) const
{
   return mParameters.contains(name);
}

DecoderControlEvent *DecoderControlEvent::setInteger(const QString &name, int value)
{
   mParameters[name] = value;

   return this;
}

int DecoderControlEvent::getInteger(const QString &name) const
{
   return mParameters[name].toInt();
}

DecoderControlEvent *DecoderControlEvent::setFloat(const QString &name, float value)
{
   mParameters[name] = value;

   return this;
}

float DecoderControlEvent::getFloat(const QString &name) const
{
   return mParameters[name].toFloat();
}

DecoderControlEvent *DecoderControlEvent::setBoolean(const QString &name, bool value)
{
   mParameters[name] = value;

   return this;
}

bool DecoderControlEvent::getBoolean(const QString &name) const
{
   return mParameters[name].toBool();
}

DecoderControlEvent *DecoderControlEvent::setString(const QString &name, const QString &value)
{
   mParameters[name] = value;

   return this;
}

QString DecoderControlEvent::getString(const QString &name) const
{
   return mParameters[name].toString();
}


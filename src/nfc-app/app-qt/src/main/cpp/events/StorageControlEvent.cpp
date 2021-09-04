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

#include "StorageControlEvent.h"

int StorageControlEvent::Type = QEvent::registerEventType();

StorageControlEvent::StorageControlEvent(int command) :
      QEvent(QEvent::Type(Type)), mCommand(command)
{
}

StorageControlEvent::StorageControlEvent(int command, QMap<QString, QVariant> parameters) :
      QEvent(QEvent::Type(Type)), mCommand(command), mParameters(parameters)
{
}

StorageControlEvent::StorageControlEvent(int command, const QString &name, int value) :
      QEvent(QEvent::Type(Type)), mCommand(command)
{
   mParameters[name] = value;
}

StorageControlEvent::StorageControlEvent(int command, const QString &name, float value) :
      QEvent(QEvent::Type(Type)), mCommand(command)
{
   mParameters[name] = value;
}

StorageControlEvent::StorageControlEvent(int command, const QString &name, bool value) :
      QEvent(QEvent::Type(Type)), mCommand(command)
{
   mParameters[name] = value;
}

StorageControlEvent::StorageControlEvent(int command, const QString &name, const QString &value) :
      QEvent(QEvent::Type(Type)), mCommand(command)
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


/*

  Copyright (c) 2020 Jose Vicente Campos Martinez - <josevcm@gmail.com>

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

#ifndef STORAGECONTROLEVENT_H
#define STORAGECONTROLEVENT_H

#include <QEvent>
#include <QMap>
#include <QVariant>

class StorageControlEvent: public QEvent
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

      StorageControlEvent(int command);
      StorageControlEvent(int command, QMap<QString, QVariant> parameters);
      StorageControlEvent(int command, const QString &name, int value);
      StorageControlEvent(int command, const QString &name, float value);
      StorageControlEvent(int command, const QString &name, bool value);
      StorageControlEvent(int command, const QString &name, const QString &value);
      virtual ~StorageControlEvent();

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

#endif // STORAGECONTROLEVENT_H

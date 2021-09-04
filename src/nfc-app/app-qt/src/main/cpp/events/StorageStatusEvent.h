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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#ifndef APP_STORAGESTATUSEVENT_H
#define APP_STORAGESTATUSEVENT_H

#include <QEvent>
#include <QMap>
#include <QVector>
#include <QStringList>
#include <QJsonObject>

class StorageStatusEvent : public QEvent
{
   public:

      static const int Type;

      static const QString Idle;
      static const QString Reading;
      static const QString Writing;

   public:

      StorageStatusEvent();

      explicit StorageStatusEvent(int status);

      explicit StorageStatusEvent(QJsonObject data);

      bool isIdle() const;

      bool isReading() const;

      bool isWriting() const;

      bool hasStatus() const;

      QString status() const;

      bool hasFileName() const;

      QString fileName() const;

      bool hasSampleRate() const;

      int sampleRate() const;

      bool hasSampleCount() const;

      long sampleCount() const;

      static StorageStatusEvent *create();

      static StorageStatusEvent *create(const QJsonObject &data);

   private:

      QJsonObject data;
};


#endif //NFC_LAB_STORAGESTATUSEVENT_H

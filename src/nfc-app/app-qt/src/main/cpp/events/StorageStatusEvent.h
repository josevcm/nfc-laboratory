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

      static const QString Reading;
      static const QString Writing;
      static const QString Progress;
      static const QString Complete;
      static const QString Error;

   public:

      StorageStatusEvent();

      explicit StorageStatusEvent(int status);

      explicit StorageStatusEvent(QJsonObject data);

      bool isReading() const;

      bool isWriting() const;

      bool isProgress() const;

      bool isComplete() const;

      bool isError() const;

      bool hasStatus() const;

      QString status() const;

      bool hasFileName() const;

      QString fileName() const;

      bool hasSampleRate() const;

      int sampleRate() const;

      bool hasSampleCount() const;

      long sampleCount() const;

      bool hasStreamTime() const;

      long streamTime() const;

      bool hasMessage() const;

      QString message() const;

      static StorageStatusEvent *create();

      static StorageStatusEvent *create(const QJsonObject &data);

   private:

      QJsonObject data;
};


#endif //NFC_LAB_STORAGESTATUSEVENT_H

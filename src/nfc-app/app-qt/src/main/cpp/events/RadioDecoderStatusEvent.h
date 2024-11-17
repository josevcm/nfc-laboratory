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

#ifndef NFC_LAB_RADIODECODERSTATUSEVENT_H
#define NFC_LAB_RADIODECODERSTATUSEVENT_H

#include <QEvent>
#include <QMap>
#include <QVector>
#include <QStringList>
#include <QJsonObject>

class RadioDecoderStatusEvent : public QEvent
{
   public:

      static const int Type;

      static const QString Idle;
      static const QString Decoding;
      static const QString Disabled;

   public:

      RadioDecoderStatusEvent();

      explicit RadioDecoderStatusEvent(int status);

      explicit RadioDecoderStatusEvent(QJsonObject data);

      const QJsonObject &content() const;

      bool hasStatus() const;

      QString status() const;

      bool hasNfcA() const;

      QJsonObject nfca() const;

      bool hasNfcB() const;

      QJsonObject nfcb() const;

      bool hasNfcF() const;

      QJsonObject nfcf() const;

      bool hasNfcV() const;

      QJsonObject nfcv() const;

      static RadioDecoderStatusEvent *create();

      static RadioDecoderStatusEvent *create(const QJsonObject &data);

   private:

      QJsonObject data;
};

#endif //NFC_LAB_DECODERSTATUSEVENT_H

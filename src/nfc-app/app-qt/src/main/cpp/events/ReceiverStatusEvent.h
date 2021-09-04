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

#ifndef APP_RECEIVERSTATUSEVENT_H
#define APP_RECEIVERSTATUSEVENT_H

#include <QEvent>
#include <QMap>
#include <QVector>
#include <QStringList>
#include <QJsonObject>

class ReceiverStatusEvent : public QEvent
{
   public:

      static const int Type;

      static const QString NoDevice;
      static const QString Idle;
      static const QString Streaming;

   public:

      ReceiverStatusEvent();

      explicit ReceiverStatusEvent(int status);

      explicit ReceiverStatusEvent(QJsonObject data);

      bool hasReceiverStatus() const;

      QString status() const;

      bool hasReceiverName() const;

      QString source() const;

      bool hasCenterFreq() const;

      int centerFreq() const;

      bool hasSampleRate() const;

      int sampleRate() const;

      bool hasSampleCount() const;

      long sampleCount() const;

      bool hasGainMode() const;

      int gainMode() const;

      bool hasGainValue() const;

      int gainValue() const;

      bool hasTunerAgc() const;

      int tunerAgc() const;

      bool hasMixerAgc() const;

      int mixerAgc() const;

      bool hasSignalPower() const;

      float signalPower() const;

      bool hasStreamProgress() const;

      float streamProgress() const;

      bool hasDeviceList() const;

      QStringList deviceList() const;

      bool hasSampeRateList() const;

      QMap<int, QString> sampleRateList() const;

      bool hasGainModeList() const;

      QMap<int, QString> gainModeList() const;

      bool hasGainValueList() const;

      QMap<int, QString> gainValueList() const;

      static ReceiverStatusEvent *create();

      static ReceiverStatusEvent *create(const QJsonObject &data);

   private:

      QJsonObject data;
};

#endif /* STREAMSTATUSEVENT_H_ */

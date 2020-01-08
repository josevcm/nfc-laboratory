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

#ifndef RADIODEVICE_H
#define RADIODEVICE_H

#include <QVector>

#include "SignalDevice.h"

class RadioDevice: public SignalDevice
{
   public:

      enum GainMode
      {
         TUNER_AUTO = 1, MIXER_AUTO = 2, DIGITAL_AUTO = 4
      };

   public:

      RadioDevice(QObject *parent = nullptr);

      virtual ~RadioDevice();

      virtual int agcMode() const = 0;

      virtual void setAgcMode(int gainMode) = 0;

      virtual float receiverGain() const = 0;

      virtual void setReceiverGain(float tunerGain) = 0;

      virtual QVector<int> supportedSampleRates() const = 0;

      virtual QVector<float> supportedReceiverGains() const = 0;

      static QStringList listDevices();

      static void registerDevice();

};

#endif /* RADIODEVICE_H */

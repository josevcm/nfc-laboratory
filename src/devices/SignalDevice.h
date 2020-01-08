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

#ifndef SIGNALDEVICE_H
#define SIGNALDEVICE_H

#include <QIODevice>

#include "SampleBuffer.h"

class SignalDevice: public QIODevice
{
   public:

      typedef struct
      {
         float i;
         float q;
      } Complex;

      enum SampleType
      {
         INTEGER = 1, FLOAT = 2
      };

   public:

      SignalDevice(QObject *parent = nullptr);

      virtual ~SignalDevice();

      virtual const QString &name() = 0;

      virtual bool open(OpenMode mode) = 0;

      virtual bool open(const QString &name, OpenMode mode) = 0;

      virtual int sampleSize() const = 0;

      virtual void setSampleSize(int sampleSize) = 0;

      virtual long sampleRate() const = 0;

      virtual void setSampleRate(long sampleRate) = 0;

      virtual int sampleType() const = 0;

      virtual void setSampleType(int sampleType) = 0;

      virtual long centerFrequency() const = 0;

      virtual void setCenterFrequency(long frequency) = 0;

      virtual int read(SampleBuffer<float> signal) = 0;

      virtual int write(SampleBuffer<float> signal) = 0;

      virtual int read(char *data, int len);

      virtual int write(char *data, int len);

      virtual bool isSequential() const;

      static SignalDevice *newInstance(const QString &name, QObject *parent = nullptr);
};

#endif /* SIGNALDEVICE_H */

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

#ifndef NFCCAPTURE_H
#define NFCCAPTURE_H

#include <QObject>
#include <QPointer>

#include <devices/SampleBuffer.h>

class SignalDevice;

class NfcCapture: public QObject
{
   public:

      // buffer for signal receiver
      constexpr static const int SampleBufferLength = 4096;

      // buffer for signal integration, must be power of 2^n
      constexpr static const int SignalBufferLength = 256;

   public:

      NfcCapture(SignalDevice *source, SignalDevice *target, QObject *parent = nullptr);
     virtual ~NfcCapture();

      long process(long timeout);
      long sampleCount() const;
      float signalStrength() const;

   private:

      // total processed samples
      long mSampleCount;

      // raw signal source
      QSharedPointer<SignalDevice> mSourceDevice;

      // raw signal recorder
      QSharedPointer<SignalDevice> mTargetDevice;

      // sample data buffer
      SampleBuffer<float> mSourceBuffer;

      // sample data buffer
      SampleBuffer<float> mTargetBuffer;

};

#endif // NFCCAPTURE_H

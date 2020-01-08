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

#include <math.h>

#include <QtCore>

#include <devices/SignalDevice.h>
#include <devices/RecordDevice.h>

#include "NfcCapture.h"

NfcCapture::NfcCapture(SignalDevice *source, SignalDevice *target, QObject *parent) : QObject(parent),
   mSampleCount(0),
   mSourceDevice(source),
   mTargetDevice(target),
   mSourceBuffer(SampleBuffer<int>::IQ, SampleBufferLength, 2),
   mTargetBuffer(SampleBuffer<int>::Real, SampleBufferLength, 1)
{
}

NfcCapture::~NfcCapture()
{
   qInfo() << "finish frame capture";
}

long NfcCapture::process(long timeout)
{
   long readedSamples = 0;
   float sampleData[2];
   short magnitude;

   QElapsedTimer timer;

   // start timeout
   timer.start();

   while (!timer.hasExpired(timeout))
   {
      // fetch next block
      if (mSourceBuffer.isEmpty())
      {
         // wait 50 ms to receive data
         if (mSourceDevice->waitForReadyRead(50))
         {
            mSourceDevice->read(mSourceBuffer.reset());
         }
      }

      // if no more samples, exit
      if (mSourceBuffer.isEmpty())
         break;

      // read next sample data
      mSourceBuffer.get(sampleData);

      // normalize sample between 0 and 1 (16 bit)
      double i = double(sampleData[0]);
      double q = double(sampleData[1]);

      // compute magnitude from IQ channels
      magnitude = short(sqrt(i * i + q * q) * (1 << 15));

      // write magnitude
      mTargetDevice->write((char*) &magnitude, sizeof(magnitude));

      readedSamples++;
   }

//   // reset buffer status
//   mSourceBuffer.reset();

//   while (timer.elapsed() < timeout && mSourceBuffer.available() > 0)
//   {
//      // wait 50 ms to receive data
//      if (mSourceDevice->waitForReadyRead(50))
//      {
//         // reset target buffer
//         mTargetBuffer.reset();

//         // if data available, read buffer
//         mSourceDevice->read(mSourceBuffer);

//         // total samples received
//         readedSamples += mSourceBuffer.available();

//         // calculate I / Q magnitude and store in output buffer
//         while(mSourceBuffer.available() > 0)
//         {
//            mSourceBuffer.get(sampleData);

//            magnitude = sqrt(double(sampleData[0]) * double(sampleData[0]) + double(sampleData[1]) * double(sampleData[1]));

//            mTargetBuffer.put(&magnitude);
//         }

//         mTargetDevice->write(mTargetBuffer.flip());
//      }
//   }

   mSampleCount += readedSamples;

   return readedSamples;
}

long NfcCapture::sampleCount() const
{
   return mSampleCount;
}

float NfcCapture::signalStrength() const
{
   return 0;
}

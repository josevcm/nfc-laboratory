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

#include <fft.h>
#include <mutex>

#include <rt/Logger.h>
#include <rt/BlockingQueue.h>

#include <nfc/NfcDecoder.h>
#include <nfc/FourierProcessTask.h>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>

#include "AbstractTask.h"

namespace nfc {

struct FourierProcessTask::Impl : FourierProcessTask, AbstractTask
{
   // decoder status
   int status;

   // FFT length and decimation
   int length;
   int decimation = 16;

   // FFT buffers
   float *fftIn = nullptr;
   float *fftOut = nullptr;
   float *fftMag = nullptr;
   float *fftWin = nullptr;

   // complex to complex FFT plan
   mufft_plan_1d *fftC2C = nullptr;

   // signal buffer frame stream subject
   rt::Subject<sdr::SignalBuffer> *signalStream = nullptr;

   // signal buffer frame stream subject
   rt::Subject<sdr::SignalBuffer> *frequencyStream = nullptr;

   // signal stream subscription
   rt::Subject<sdr::SignalBuffer>::Subscription signalSubscription;

   // last status sent
   std::chrono::time_point<std::chrono::steady_clock> lastStatus;

   // last signal buffer
   sdr::SignalBuffer signalBuffer;

   // stream lock
   std::mutex signalMutex;

   explicit Impl(int length = 1024) : AbstractTask("FourierProcessTask", "fourier"), status(FourierProcessTask::Idle), length(length)
   {
      // create fft buffers
      fftIn = static_cast<float *>(mufft_alloc(length * sizeof(float) * 2));
      fftOut = static_cast<float *>(mufft_alloc(length * sizeof(float) * 2));
      fftMag = static_cast<float *>(mufft_alloc(length * sizeof(float)));
      fftWin = static_cast<float *>(mufft_alloc(length * sizeof(float)));

      // create FFT plans
      fftC2C = mufft_create_plan_1d_c2c(length, MUFFT_FORWARD, MUFFT_FLAG_CPU_NO_AVX);

      // access to signal subject stream
      signalStream = rt::Subject<sdr::SignalBuffer>::name("signal.iq");

      // access to signal subject stream
      frequencyStream = rt::Subject<sdr::SignalBuffer>::name("signal.fft");

      // subscribe to signal events
      signalSubscription = signalStream->subscribe([=](const sdr::SignalBuffer &buffer) {
         if (signalMutex.try_lock())
         {
            signalBuffer = buffer;
            signalMutex.unlock();
         }
      });
   }

   ~Impl() override
   {
      mufft_free(fftIn);
      mufft_free(fftOut);
      mufft_free(fftMag);
      mufft_free(fftWin);
      mufft_free_plan_1d(fftC2C);
   }

   void start() override
   {
      // initialize Hamming Window
      for (int i = 0; i < length; i++)
      {
         fftWin[i] = pow((float) sin(float(M_PI * i / length)), 2);
      }
   }

   void stop() override
   {
   }

   bool loop() override
   {
      // process FFT at 20 fps (50ms)
      wait(50);

      // compute fast fourier transform
      process();

      // update recorder status
      if ((std::chrono::steady_clock::now() - lastStatus) > std::chrono::milliseconds(500))
      {
         updateFourierStatus();
      }

      return true;
   }

   void process()
   {
      std::lock_guard<std::mutex> lock(signalMutex);

      // IQ complex signal to real FFT transform
      if (signalBuffer.isValid() && signalBuffer.type() == sdr::SignalType::COMPLEX_IQ)
      {
         float *data = signalBuffer.data();

         // apply signal windowing and decimation
#pragma GCC ivdep
         for (int i = 0, w = 0; w < length; i += 2, w++)
         {
            fftIn[i + 0] = data[decimation * i + 0] * fftWin[w];
            fftIn[i + 1] = data[decimation * i + 1] * fftWin[w];
         }

         // execute FFT
         mufft_execute_plan_1d(fftC2C, fftOut, fftIn);

         // apply FFT complex to real
#pragma GCC ivdep
         for (int i = 0, w = 0; w < length; i += 2, w++)
         {
            fftMag[w] = std::sqrt(fftOut[i] * fftOut[i] + fftOut[i + 1] * fftOut[i + 1]);
         }

         // create output buffer
         sdr::SignalBuffer result(length, 1, signalBuffer.sampleRate(), 0, decimation, sdr::SignalType::COMPLEX_FREQ);

         // add data width negative / positive frequency shift
         result.put(fftMag + (length >> 1), (length >> 1)).put(fftMag, (length >> 1)).flip();

         // publish to observers
         frequencyStream->next(result);
      }
   }

   void updateFourierStatus()
   {
   }
};

FourierProcessTask::FourierProcessTask() : rt::Worker("FourierProcessTask")
{
}

rt::Worker *FourierProcessTask::construct()
{
   return new FourierProcessTask::Impl;
}

}
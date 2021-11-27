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

#ifdef __SSE2__
#include <x86intrin.h>
#endif

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
   rt::Subject<sdr::SignalBuffer> *signalIqStream = nullptr;

   // signal buffer frame stream subject
   rt::Subject<sdr::SignalBuffer> *frequencyStream = nullptr;

   // signal stream subscription
   rt::Subject<sdr::SignalBuffer>::Subscription signalIqSubscription;

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
      signalIqStream = rt::Subject<sdr::SignalBuffer>::name("signal.iq");

      // access to signal subject stream
      frequencyStream = rt::Subject<sdr::SignalBuffer>::name("signal.fft");

      // subscribe to signal events
      signalIqSubscription = signalIqStream->subscribe([=](const sdr::SignalBuffer &buffer) {
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
      // process FFT at 50 fps (20ms / frame)
      wait(20);

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
      if (signalBuffer.isValid() && signalBuffer.type() == sdr::SignalType::SAMPLE_IQ)
      {
         float *data = signalBuffer.data();

         // apply signal windowing and decimation
#ifdef __SSE2__
         for (int i = 0, w = 0; w < length; i += 8, w += 4)
         {
            __m128 w1 = _mm_set_ps(fftWin[w + 1], fftWin[w + 1], fftWin[w + 0], fftWin[w + 0]);
            __m128 w2 = _mm_set_ps(fftWin[w + 3], fftWin[w + 3], fftWin[w + 2], fftWin[w + 2]);

            __m128 a1 = _mm_loadu_ps(data + i + 0);
            __m128 a2 = _mm_loadu_ps(data + i + 4);

            __m128 r1 = _mm_mul_ps(a1, w1);
            __m128 r2 = _mm_mul_ps(a2, w2);

            _mm_storeu_ps(fftIn + i + 0, r1);
            _mm_storeu_ps(fftIn + i + 4, r2);
         }
#else
#pragma GCC ivdep
         for (int i = 0, w = 0; w < length; i += 4, w += 2)
         {
            fftIn[i + 0] = data[decimation * i + 0] * fftWin[w + 0];
            fftIn[i + 1] = data[decimation * i + 1] * fftWin[w + 0];
            fftIn[i + 2] = data[decimation * i + 2] * fftWin[w + 1];
            fftIn[i + 3] = data[decimation * i + 3] * fftWin[w + 1];
         }
#endif

         // execute FFT
         mufft_execute_plan_1d(fftC2C, fftOut, fftIn);

         // transform complex FFT to real
#ifdef __SSE2__
         for (int i = 0, w = 0; w < length; i += 16, w += 8)
         {
            // load 8 I/Q vectors
            __m128 a1 = _mm_loadu_ps(fftOut + i + 0);  // r0, i0, r1, i1
            __m128 a2 = _mm_loadu_ps(fftOut + i + 4);  // r2, i2, r3, i3
            __m128 a3 = _mm_loadu_ps(fftOut + i + 8);  // r4, i4, r5, i5
            __m128 a4 = _mm_loadu_ps(fftOut + i + 12); // r6, i6, r7, i7

            // square all components
            __m128 p1 = _mm_mul_ps(a1, a1); // r0^2, Q0^2, r1^2, Q1^2
            __m128 p2 = _mm_mul_ps(a2, a2); // r2^2, Q2^2, r3^2, Q3^2
            __m128 p3 = _mm_mul_ps(a3, a3); // r4^2, Q4^2, r5^2, Q5^2
            __m128 p4 = _mm_mul_ps(a4, a4); // r6^2, Q6^2, r6^2, Q7^2

            // permute components
            __m128 i1 = _mm_shuffle_ps(p1, p2, _MM_SHUFFLE(2, 0, 2, 0)); // r0^2, r1^2, r2^2, r3^2
            __m128 i2 = _mm_shuffle_ps(p3, p4, _MM_SHUFFLE(2, 0, 2, 0)); // r4^2, r5^2, r6^2, r7^2
            __m128 q1 = _mm_shuffle_ps(p1, p2, _MM_SHUFFLE(3, 1, 3, 1)); // Q0^2, Q1^2, Q2^2, Q3^2
            __m128 q2 = _mm_shuffle_ps(p3, p4, _MM_SHUFFLE(3, 1, 3, 1)); // Q4^2, Q5^2, Q6^2, Q7^2

            // add vector components
            __m128 r1 = _mm_add_ps(i1, q1); // r0^2+Q0^2, r1^2+Q1^2, r2^2+Q2^2, r3^2+Q3^2
            __m128 r2 = _mm_add_ps(i2, q2); // r4^2+Q4^2, r5^2+Q5^2, r6^2+Q6^2, r7^2+Q7^2

            // square-root vectors
            __m128 m1 = _mm_sqrt_ps(r1); // sqrt(I0^2+Q0^2), sqrt(I1^2+Q1^2), sqrt(I2^2+Q2^2), sqrt(I3^2+Q3^2)
            __m128 m2 = _mm_sqrt_ps(r2); // sqrt(I4^2+Q4^2), sqrt(I5^2+Q5^2), sqrt(I6^2+Q6^2), sqrt(I7^2+Q7^2)

            // store results
            _mm_storeu_ps(fftMag + w + 0, m1);
            _mm_storeu_ps(fftMag + w + 4, m2);
         }
#else
#pragma GCC ivdep
         for (int i = 0, w = 0; w < length; i += 8, w += 4)
         {
            fftMag[w + 0] = std::sqrt(fftOut[i + 0] * fftOut[i + 0] + fftOut[i + 1] * fftOut[i + 1]);
            fftMag[w + 1] = std::sqrt(fftOut[i + 2] * fftOut[i + 2] + fftOut[i + 3] * fftOut[i + 3]);
            fftMag[w + 2] = std::sqrt(fftOut[i + 4] * fftOut[i + 4] + fftOut[i + 5] * fftOut[i + 5]);
            fftMag[w + 3] = std::sqrt(fftOut[i + 6] * fftOut[i + 6] + fftOut[i + 7] * fftOut[i + 7]);
         }
#endif

         // create output buffer
         sdr::SignalBuffer result(length, 1, signalBuffer.sampleRate(), 0, decimation, sdr::SignalType::FREQUENCY_BIN);

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
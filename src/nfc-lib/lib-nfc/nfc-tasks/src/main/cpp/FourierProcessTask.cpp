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

#if defined(__SSE2__) && defined(USE_SSE2)

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
   int decimation = 1;
   int bandwith = 10E6 / 16;

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

         // calculate decimation for required bandwith
         decimation = int(signalBuffer.sampleRate() / bandwith);

         // apply signal windowing and decimation
#if defined(__SSE2__) && defined(USE_SSE2)
         for (int i = 0, w = 0; w < length; i += 8, w += 4)
         {
            __m128 w1 = _mm_set_ps(fftWin[w + 1], fftWin[w + 1], fftWin[w + 0], fftWin[w + 0]);
            __m128 w2 = _mm_set_ps(fftWin[w + 3], fftWin[w + 3], fftWin[w + 2], fftWin[w + 2]);

            __m128 a1 = _mm_load_ps(data + i * decimation + 0);
            __m128 a2 = _mm_load_ps(data + i * decimation + 4);

            __m128 r1 = _mm_mul_ps(a1, w1);
            __m128 r2 = _mm_mul_ps(a2, w2);

            _mm_store_ps(fftIn + i + 0, r1);
            _mm_store_ps(fftIn + i + 4, r2);
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
#if defined(__SSE2__) && defined(USE_SSE2)
         for (int i = 0, n = 0; i < length; i += 16, n += 32)
         {
            // load 16 I/Q vectors
            __m128 a0 = _mm_load_ps(fftOut + n + 0);  // I0, Q0, I1, Q1
            __m128 a1 = _mm_load_ps(fftOut + n + 4);  // I2, Q2, I3, Q3
            __m128 a2 = _mm_load_ps(fftOut + n + 8);  // I4, Q4, I5, Q5
            __m128 a3 = _mm_load_ps(fftOut + n + 12); // I6, Q6, I7, Q7
            __m128 a4 = _mm_load_ps(fftOut + n + 16);  // I8, Q8, I9, Q9
            __m128 a5 = _mm_load_ps(fftOut + n + 20); // I10, Q10, I11, Q11
            __m128 a6 = _mm_load_ps(fftOut + n + 24);  // I12, Q12, I13, Q13
            __m128 a7 = _mm_load_ps(fftOut + n + 28); // I14, Q14, I15, Q15

            // square all components
            __m128 p0 = _mm_mul_ps(a0, a0); // I0^2, Q0^2, I1^2, Q1^2
            __m128 p1 = _mm_mul_ps(a1, a1); // I2^2, Q2^2, I3^2, Q3^2
            __m128 p2 = _mm_mul_ps(a2, a2); // I4^2, Q4^2, I5^2, Q5^2
            __m128 p3 = _mm_mul_ps(a3, a3); // I6^2, Q6^2, I7^2, Q7^2
            __m128 p4 = _mm_mul_ps(a4, a4); // I8^2, Q8^2, I9^2, Q9^2
            __m128 p5 = _mm_mul_ps(a5, a5); // I10^2, Q10^2, I11^2, Q11^2
            __m128 p6 = _mm_mul_ps(a6, a6); // I12^2, Q12^2, I13^2, Q13^2
            __m128 p7 = _mm_mul_ps(a7, a7); // I14^2, Q14^2, I15^2, Q15^2

            // permute components
            __m128 i0 = _mm_shuffle_ps(p0, p1, _MM_SHUFFLE(2, 0, 2, 0)); // I0^2, I1^2, I2^2, I3^2
            __m128 i1 = _mm_shuffle_ps(p2, p3, _MM_SHUFFLE(2, 0, 2, 0)); // I4^2, I5^2, I6^2, I7^2
            __m128 i2 = _mm_shuffle_ps(p4, p5, _MM_SHUFFLE(2, 0, 2, 0)); // I8^2, I9^2, I10^2, I11^2
            __m128 i3 = _mm_shuffle_ps(p6, p7, _MM_SHUFFLE(2, 0, 2, 0)); // I12^2, I13^2, I14^2, I15^2
            __m128 q0 = _mm_shuffle_ps(p0, p1, _MM_SHUFFLE(3, 1, 3, 1)); // Q0^2, Q1^2, Q2^2, Q3^2
            __m128 q1 = _mm_shuffle_ps(p2, p3, _MM_SHUFFLE(3, 1, 3, 1)); // Q4^2, Q5^2, Q6^2, Q7^2
            __m128 q2 = _mm_shuffle_ps(p4, p5, _MM_SHUFFLE(3, 1, 3, 1)); // Q8^2, Q9^2, Q10^2, Q11^2
            __m128 q3 = _mm_shuffle_ps(p6, p7, _MM_SHUFFLE(3, 1, 3, 1)); // Q12^2, Q13^2, Q14^2, Q15^2

            // add vector components
            __m128 r0 = _mm_add_ps(i0, q0); // I0^2+Q0^2, I1^2+Q1^2, I2^2+Q2^2, I3^2+Q3^2
            __m128 r1 = _mm_add_ps(i1, q1); // I4^2+Q4^2, I5^2+Q5^2, I6^2+Q6^2, I7^2+Q7^2
            __m128 r2 = _mm_add_ps(i2, q2); // I8^2+Q8^2, I9^2+Q1^2, I10^2+Q10^2, I11^2+Q11^2
            __m128 r3 = _mm_add_ps(i3, q3); // I12^2+Q12^2, I13^2+Q13^2, I14^2+Q14^2, I15^2+Q15^2

            // square-root vectors
            __m128 m0 = _mm_sqrt_ps(r0); // sqrt(I0^2+Q0^2), sqrt(I1^2+Q1^2), sqrt(I2^2+Q2^2), sqrt(I3^2+Q3^2)
            __m128 m1 = _mm_sqrt_ps(r1); // sqrt(I4^2+Q4^2), sqrt(I5^2+Q5^2), sqrt(I6^2+Q6^2), sqrt(I7^2+Q7^2)
            __m128 m2 = _mm_sqrt_ps(r2); // sqrt(I8^2+Q8^2), sqrt(I9^2+Q9^2), sqrt(I10^2+Q10^2), sqrt(I11^2+Q11^2)
            __m128 m3 = _mm_sqrt_ps(r3); // sqrt(I12^2+Q12^2), sqrt(I13^2+Q13^2), sqrt(I14^2+Q14^2), sqrt(I15^2+Q15^2)

            // store results
            _mm_store_ps(fftMag + i + 0, m0);
            _mm_store_ps(fftMag + i + 4, m1);
            _mm_store_ps(fftMag + i + 8, m2);
            _mm_store_ps(fftMag + i + 12, m3);
         }
#else
#pragma GCC ivdep
         for (int i = 0, n = 0; i < length; i += 4, n += 8)
         {
            fftMag[i + 0] = std::sqrt(fftOut[n + 0] * fftOut[n + 0] + fftOut[n + 1] * fftOut[n + 1]);
            fftMag[i + 1] = std::sqrt(fftOut[n + 2] * fftOut[n + 2] + fftOut[n + 3] * fftOut[n + 3]);
            fftMag[i + 2] = std::sqrt(fftOut[n + 4] * fftOut[n + 4] + fftOut[n + 5] * fftOut[n + 5]);
            fftMag[i + 3] = std::sqrt(fftOut[n + 6] * fftOut[n + 6] + fftOut[n + 7] * fftOut[n + 7]);
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
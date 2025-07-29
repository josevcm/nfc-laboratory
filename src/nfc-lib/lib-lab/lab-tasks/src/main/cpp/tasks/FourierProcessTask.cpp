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

#if defined(__SSE2__) && defined(USE_SSE2)

#include <x86intrin.h>

#endif

#include <fft.h>
#include <cmath>
#include <mutex>

#include <rt/Throughput.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <lab/tasks/FourierProcessTask.h>

#include "AbstractTask.h"

namespace lab {

struct FourierProcessTask::Impl : FourierProcessTask, AbstractTask
{
   // FFT length and decimation
   int window;
   int length;
   int decimation = 1;
   int bandwidth = 10E6 / 16;

   // FFT buffers
   float *fftIn = nullptr;
   float *fftOut = nullptr;
   float *fftMag = nullptr;
   float *fftWin = nullptr;

   // complex to complex FFT plan
   mufft_plan_1d *fftC2C = nullptr;

   // signal buffer frame stream subject
   rt::Subject<hw::SignalBuffer> *signalIqStream = nullptr;

   // signal buffer frame stream subject
   rt::Subject<hw::SignalBuffer> *frequencyStream = nullptr;

   // signal stream subscription
   rt::Subject<hw::SignalBuffer>::Subscription signalIqSubscription;

   // last status sent
   std::chrono::time_point<std::chrono::steady_clock> lastStatus;

   // last signal buffer
   hw::SignalBuffer signalBuffer;

   // stream lock
   std::mutex signalMutex;

   // sample throughput meter
   rt::Throughput taskThroughput;

   // current task status
   bool fourierTaskEnabled = false;

   // current receiver status
   int fourierTaskStatus = Streaming;

   explicit Impl(int length = 1024, int window = Hamming) : AbstractTask("worker.FourierProcess", "fourier"), length(length), window(window)
   {
      // create fft buffers
      fftIn = static_cast<float *>(mufft_alloc(length * sizeof(float) * 2));
      fftOut = static_cast<float *>(mufft_alloc(length * sizeof(float) * 2));
      fftWin = static_cast<float *>(mufft_alloc(length * sizeof(float) * 2));
      fftMag = static_cast<float *>(mufft_alloc(length * sizeof(float)));

      // create FFT plans
      fftC2C = mufft_create_plan_1d_c2c(length, MUFFT_FORWARD, MUFFT_FLAG_CPU_NO_AVX);

      // access to signal subject stream
      signalIqStream = rt::Subject<hw::SignalBuffer>::name("radio.signal.iq");

      // access to signal subject stream
      frequencyStream = rt::Subject<hw::SignalBuffer>::name("signal.fft");

      // subscribe to signal events
      signalIqSubscription = signalIqStream->subscribe([=](const hw::SignalBuffer &buffer) {
         std::lock_guard lock(signalMutex);
         signalBuffer = buffer;
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
      switch (window)
      {
         case Hamming: // Hamming
            log->info("using Hamming window");
            for (int n = 0, i = 0; n < length; ++n, i += 2)
               fftWin[i + 0] = fftWin[i + 1] = static_cast<float>(std::pow(std::sin(static_cast<float>(M_PI * n / length)), 2));
            break;

         case Hann: // Hann
            log->info("using Hann window");
            for (int n = 0, i = 0; n < length; ++n, i += 2)
               fftWin[i + 0] = fftWin[i + 1] = static_cast<float>(0.5 * (1.0 - std::cos((2.0 * M_PI * n) / (length - 1))));
            break;

         default: // no window
            log->info("using no window");
            for (int n = 0, i = 0; n < length; ++n, i += 2)
               fftWin[i + 0] = fftWin[i + 1] = 1;
            break;
      }

      updateFourierStatus(Streaming);
   }

   void stop() override
   {
   }

   bool loop() override
   {
      /*
      * process pending commands
      */
      if (auto command = commandQueue.get())
      {
         log->debug("command [{}]", {command->code});

         switch (command->code)
         {
            case Configure:
               configure(command.value());
               break;

            default:
               log->warn("unknown command {}", {command->code});
               command->reject(UnknownCommand);
               return true;
         }
      }

      if (fourierTaskEnabled)
      {
         // process FFT at 100 fps (10ms / frame)
         wait(10);

         // compute fast fourier transform
         process();
      }
      else
      {
         wait(100);
      }

      // update recorder status
      if (std::chrono::steady_clock::now() - lastStatus > std::chrono::milliseconds(1000))
      {
         if (taskThroughput.average() > 0)
            log->info("average throughput {.2} Ksps", {taskThroughput.average() / 1E3});

         // store last search time
         lastStatus = std::chrono::steady_clock::now();
      }

      return true;
   }

   void configure(const rt::Event &command)
   {
      if (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log->debug("change config: {}", {config.dump()});

         if (config.contains("enabled"))
         {
            fourierTaskEnabled = config["enabled"];
         }

         command.resolve();

         updateFourierStatus(fourierTaskStatus);
      }
      else
      {
         log->warn("invalid config data");

         command.reject(InvalidConfig);
      }
   }

   void process()
   {
      hw::SignalBuffer localBuffer;

      {
         std::lock_guard lock(signalMutex);

         localBuffer = signalBuffer;
      }

      // IQ complex signal to real FFT transform
      if (localBuffer.isValid() && localBuffer.type() == hw::SignalType::SIGNAL_TYPE_RADIO_IQ)
      {
         float *data = localBuffer.data();

         // calculate decimation for required bandwidth
         decimation = static_cast<int>(localBuffer.sampleRate() / bandwidth);

         // offset from the start of the signal buffer to apply FFT, must be 16 byte aligned
         int offset = 0;

         // apply signal windowing and decimation
#if defined(__SSE2__) && defined(USE_SSE2)
         for (int i = offset, n = 0; n < (length << 1); i += 8, n += 8)
         {
            __m128 w1 = _mm_load_ps(fftWin + n + 0);
            __m128 w2 = _mm_load_ps(fftWin + n + 4);

            __m128 a1 = _mm_load_ps(data + i * decimation + 0);
            __m128 a2 = _mm_load_ps(data + i * decimation + 4);

            __m128 r1 = _mm_mul_ps(a1, w1);
            __m128 r2 = _mm_mul_ps(a2, w2);

            _mm_store_ps(fftIn + n + 0, r1);
            _mm_store_ps(fftIn + n + 4, r2);
         }
#else
#pragma GCC ivdep
         for (int i = offset, n = 0; n < (length << 1); i += 4, n += 4)
         {
            fftIn[n + 0] = data[decimation * i + 0] * fftWin[n + 0];
            fftIn[n + 1] = data[decimation * i + 1] * fftWin[n + 1];
            fftIn[n + 2] = data[decimation * i + 2] * fftWin[n + 2];
            fftIn[n + 3] = data[decimation * i + 3] * fftWin[n + 3];
         }
#endif

         // execute FFT
         mufft_execute_plan_1d(fftC2C, fftOut, fftIn);

         // transform complex FFT to real
#if defined(__SSE2__) && defined(USE_SSE2)
         for (int i = 0, n = 0; i < length; i += 16, n += 32)
         {
            // load 16 I/Q vectors
            __m128 a0 = _mm_load_ps(fftOut + n + 0); // I0, Q0, I1, Q1
            __m128 a1 = _mm_load_ps(fftOut + n + 4); // I2, Q2, I3, Q3
            __m128 a2 = _mm_load_ps(fftOut + n + 8); // I4, Q4, I5, Q5
            __m128 a3 = _mm_load_ps(fftOut + n + 12); // I6, Q6, I7, Q7
            __m128 a4 = _mm_load_ps(fftOut + n + 16); // I8, Q8, I9, Q9
            __m128 a5 = _mm_load_ps(fftOut + n + 20); // I10, Q10, I11, Q11
            __m128 a6 = _mm_load_ps(fftOut + n + 24); // I12, Q12, I13, Q13
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
         hw::SignalBuffer result(length, 1, 1, localBuffer.sampleRate(), 0, decimation, hw::SignalType::SIGNAL_TYPE_FFT_BIN);

         // add data width negative / positive frequency shift
         result.put(fftMag + (length >> 1), (length >> 1)).put(fftMag, (length >> 1)).flip();

         // publish to observers
         frequencyStream->next(result);

         // update frame throughput
         taskThroughput.update(length);
      }
   }

   void updateFourierStatus(int status)
   {
      json data;

      fourierTaskStatus = status;

      data["status"] = fourierTaskEnabled ? "streaming" : "disabled";

      updateStatus(status, data);
   }
};

FourierProcessTask::FourierProcessTask() : Worker("FourierProcess")
{
}

rt::Worker *FourierProcessTask::construct()
{
   return new Impl;
}

}

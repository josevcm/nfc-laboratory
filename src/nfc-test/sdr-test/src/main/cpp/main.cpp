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

#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <rt/Logger.h>
#include <rt/FileSystem.h>
#include <rt/BlockingQueue.h>

#include <sdr/SignalBuffer.h>
#include <sdr/SignalType.h>
#include <sdr/AirspyDevice.h>
#include <sdr/LimeDevice.h>
#include <sdr/RealtekDevice.h>
#include <sdr/RecordDevice.h>

using namespace rt;

Logger logger {"main"};

/*
 * signal capture test
 */

int limeTest(int argc, char *argv[])
{
   char file[128];
   struct tm timeinfo {};

   std::time_t rawTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
   localtime_s(&timeinfo, &rawTime);
   strftime(file, sizeof(file), "record-%Y%m%d%H%M%S.wav", &timeinfo);

   // open first available receiver
   for (const auto &name: sdr::LimeDevice::listDevices())
   {
      // create receiver
      sdr::LimeDevice receiver(name);

      // default parameters
      receiver.setCenterFreq(40.68E6);
      receiver.setSampleRate(8E6);
      receiver.setGainMode(2);
      receiver.setGainValue(30);
      receiver.setMixerAgc(0);
      receiver.setTunerAgc(0);

      // try to open Lime
      if (receiver.open(sdr::LimeDevice::Read))
      {
         logger.info("device {} connected!", {name});

         sdr::RecordDevice recorder(file);

         recorder.setChannelCount(3);
         recorder.setSampleRate(receiver.sampleRate());
         recorder.open(sdr::RecordDevice::Write);

         // open recorder
         if (recorder.open(sdr::RecordDevice::OpenMode::Write))
         {
            // signal stream queue buffer
            rt::BlockingQueue<sdr::SignalBuffer> signalQueue;

            // start receive stream
            receiver.start([&signalQueue](sdr::SignalBuffer &buffer) {
               signalQueue.add(buffer);
            });

            int count = 0;

            while (auto buffer = signalQueue.get(-1))
            {
               if (++count == 1000)
                  break;

               if (!buffer->isEmpty())
               {
//                  recorder.write(buffer.value());

                  int samples = buffer->elements() / buffer->stride();

                  // convert I/Q samples to Real sample
                  sdr::SignalBuffer result(samples * 3, 3, buffer->sampleRate(), 0, 0, 0);

                  switch (buffer->type())
                  {
                     case sdr::SignalType::SAMPLE_IQ:
                     {
                        buffer->stream([&result](const float *value, int stride) {
                           result.put(value[0]).put(value[1]).put(sqrtf(value[0] * value[0] + value[1] * value[1]));
                        });
                        break;
                     }

                     case sdr::SignalType::SAMPLE_REAL:
                     {
                        buffer->stream([&result](const float *value, int stride) {
                           result.put(value[0]).put(0.0f).put(0.0f);
                        });
                        break;
                     }
                  }

                  result.flip();

                  recorder.write(result);
               }
            }

            receiver.close();
         }

         logger.info("capture finished");
      }
   }

   return 0;
}

int main(int argc, char *argv[])
{
   return limeTest(argc, argv);
}


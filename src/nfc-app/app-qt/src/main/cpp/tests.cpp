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

#include <rt/Logger.h>
#include <rt/Executor.h>
#include <rt/Subject.h>
#include <rt/Event.h>
#include <rt/BlockingQueue.h>

#include <sdr/SignalType.h>
#include <sdr/RecordDevice.h>
#include <sdr/AirspyDevice.h>
#include <sdr/RealtekDevice.h>

#include <nfc/AdaptiveSamplingTask.h>
#include <nfc/FourierProcessTask.h>
#include <nfc/FrameDecoderTask.h>
#include <nfc/FrameStorageTask.h>
#include <nfc/SignalReceiverTask.h>
#include <nfc/SignalRecorderTask.h>

#include <nfc/NfcFrame.h>
#include <nfc/NfcDecoder.h>

#include <libusb.h>

#include "QtApplication.h"

using namespace rt;

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
   QByteArray localMsg = msg.toLocal8Bit();

   if (qlog)
   {
      switch (type)
      {
         case QtDebugMsg:
            qlog->debug(localMsg.constData());
            break;
         case QtInfoMsg:
            qlog->info(localMsg.constData());
            break;
         case QtWarningMsg:
            qlog->warn(localMsg.constData());
            break;
         case QtCriticalMsg:
            qlog->error(localMsg.constData());
            break;
         case QtFatalMsg:
            qlog->error(localMsg.constData());
            abort();
      }
   }
}

/*
 * frame decoder test
 */
int startTest1(int argc, char *argv[])
{
   Logger log {"main"};

   log.info("***********************************************************************");
   log.info("NFC laboratory, 2022 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   log.info("***********************************************************************");

   nfc::NfcDecoder decoder;

   decoder.setEnableNfcA(false);
   decoder.setEnableNfcB(false);
   decoder.setEnableNfcF(true);
   decoder.setEnableNfcV(false);

   sdr::RecordDevice source(argv[1]);

   if (source.open(sdr::RecordDevice::OpenMode::Read))
   {
      while (!source.isEof())
      {
         sdr::SignalBuffer samples(65536 * source.channelCount(), source.channelCount(), source.sampleRate(), 0, 0, sdr::SignalType::SAMPLE_REAL);

         if (source.read(samples) > 0)
         {
            std::list<nfc::NfcFrame> frames = decoder.nextFrames(samples);

            for (const nfc::NfcFrame &frame: frames)
            {
               if (frame.isPollFrame())
               {
                  log.info("frame at {} -> {}: TX {}", {frame.sampleStart(), frame.sampleEnd(), frame});
               }
               else if (frame.isListenFrame())
               {
                  log.info("frame at {} -> {}: RX {}", {frame.sampleStart(), frame.sampleEnd(), frame});
               }
            }
         }
      }
   }

   return 0;
}

/*
 * airspy signal capture test
 */
int startTest2(int argc, char *argv[])
{
   Logger log {"main"};

   log.info("***********************************************************************");
   log.info("NFC laboratory, 2022 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   log.info("***********************************************************************");

   char file[128];
   struct tm timeinfo {};

   std::time_t rawTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

#ifdef _WIN32
   localtime_s(&timeinfo, &rawTime);
#else
   localtime_r(&rawTime, &timeinfo);
#endif

   strftime(file, sizeof(file), "record-%Y%m%d%H%M%S.wav", &timeinfo);

   // open first available receiver
   for (const auto &name: sdr::AirspyDevice::listDevices())
   {
      // create receiver
      sdr::AirspyDevice receiver(name);

      // default parameters
      receiver.setCenterFreq(40.68E6);
      receiver.setSampleRate(10E6);
      receiver.setGainMode(2);
      receiver.setGainValue(3);
      receiver.setMixerAgc(0);
      receiver.setTunerAgc(0);

      // try to open Airspy
      if (receiver.open(sdr::SignalDevice::Read))
      {
         log.info("device {} connected!", {name});

         sdr::RecordDevice recorder(file);

         recorder.setChannelCount(3);
         recorder.setSampleRate(receiver.sampleRate());
         recorder.open(sdr::RecordDevice::Write);

         // open recorder
         if (recorder.open(sdr::RecordDevice::OpenMode::Write))
         {
            log.info("start streaming for device {}", {receiver.name()});

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

            log.info("stop streaming for device {}", {receiver.name()});
         }

         log.info("capture finished");
      }
   }

   return 0;
}

/*
 * rtl-sdr signal capture test
 */
int startTest3(int argc, char *argv[])
{
   Logger log {"main"};

   log.info("***********************************************************************");
   log.info("NFC laboratory, 2022 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   log.info("***********************************************************************");

   char file[128];
   struct tm timeinfo {};

   std::time_t rawTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

#ifdef _WIN32
   localtime_s(&timeinfo, &rawTime);
#else
   localtime_r(&rawTime, &timeinfo);
#endif

   strftime(file, sizeof(file), "record-%Y%m%d%H%M%S.wav", &timeinfo);

   // open first available receiver
   for (const auto &name: sdr::RealtekDevice::listDevices())
   {
      // create receiver
      sdr::RealtekDevice receiver(name);

      // default parameters
      receiver.setCenterFreq(27.12E6);
      receiver.setSampleRate(2400000);
      receiver.setGainMode(sdr::RealtekDevice::Manual);
      receiver.setGainValue(77);
      receiver.setMixerAgc(0);
      receiver.setTunerAgc(0);
      receiver.setTestMode(0);

      // try to open Realtek
      if (receiver.open(sdr::SignalDevice::Read))
      {
         log.info("device {} connected!", {name});

         for (auto gain: receiver.supportedSampleRates())
         {
            log.info("available sample rate {} = {}", {gain.first, gain.second});
         }

         for (auto gain: receiver.supportedGainValues())
         {
            log.info("available gain {} = {}", {gain.first, gain.second});
         }

         sdr::RecordDevice recorder(file);

         recorder.setChannelCount(3);
         recorder.setSampleRate(receiver.sampleRate());
         recorder.open(sdr::RecordDevice::Write);

         // open recorder
         if (recorder.open(sdr::RecordDevice::OpenMode::Write))
         {
            log.info("start streaming for device {}", {receiver.name()});

            // signal stream queue buffer
            rt::BlockingQueue<sdr::SignalBuffer> signalQueue;

            // start receive stream
            receiver.start([&signalQueue](sdr::SignalBuffer &buffer) {
               signalQueue.add(buffer);
            });

            int count = 0;

            while (auto buffer = signalQueue.get(-1))
            {
               if (++count == 100)
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

            log.info("stop streaming for device {}", {receiver.name()});
         }

         log.info("capture finished");
      }
   }

   return 0;
}

//int main(int argc, char *argv[])
//{
////   return startTest1(argc, argv);
////   return startTest2(argc, argv);
////   return startTest3(argc, argv);
//}


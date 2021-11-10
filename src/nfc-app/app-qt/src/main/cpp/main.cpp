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

#include <nfc/AdaptiveSamplingTask.h>
#include <nfc/FourierProcessTask.h>
#include <nfc/FrameDecoderTask.h>
#include <nfc/FrameStorageTask.h>
#include <nfc/SignalReceiverTask.h>
#include <nfc/SignalRecorderTask.h>

#include <nfc/NfcFrame.h>
#include <nfc/NfcDecoder.h>

#include "QtApplication.h"

// https://beesbuzz.biz/code/4399-Embedding-binary-resources-with-CMake-and-C-11

using namespace rt;

Logger logger("main");

Logger qlog("QT");

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
   QByteArray localMsg = msg.toLocal8Bit();

   switch (type)
   {
      case QtDebugMsg:
         qlog.debug(localMsg.constData());
         break;
      case QtInfoMsg:
         qlog.info(localMsg.constData());
         break;
      case QtWarningMsg:
         qlog.warn(localMsg.constData());
         break;
      case QtCriticalMsg:
         qlog.error(localMsg.constData());
         break;
      case QtFatalMsg:
         qlog.error(localMsg.constData());
         abort();
   }
}

/*
 * frame decoder test
 */
int startTest1(int argc, char *argv[])
{
   logger.info("***********************************************************************");
   logger.info("NFC laboratory, 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   logger.info("***********************************************************************");

   nfc::NfcDecoder decoder;

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
                  logger.info("frame at {} -> {}: TX {}", {frame.sampleStart(), frame.sampleEnd(), frame});
               }
               else if (frame.isListenFrame())
               {
                  logger.info("frame at {} -> {}: RX {}", {frame.sampleStart(), frame.sampleEnd(), frame});
               }
            }
         }
      }
   }

   return 0;
}

/*
 * signal capture test
 */

int startTest2(int argc, char *argv[])
{
   logger.info("***********************************************************************");
   logger.info("NFC laboratory, 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   logger.info("***********************************************************************");

   char file[128];
   struct tm timeinfo {};

   std::time_t rawTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
   localtime_s(&timeinfo, &rawTime);
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
      receiver.setGainValue(4);
      receiver.setMixerAgc(0);
      receiver.setTunerAgc(0);

      // try to open Airspy
      if (receiver.open(sdr::SignalDevice::Read))
      {
         logger.info("device {} connected!", {name});

         sdr::RecordDevice recorder(file);

         recorder.setChannelCount(4);
         recorder.setSampleRate(receiver.sampleRate());
         recorder.open(sdr::RecordDevice::Write);

         // open recorder
         if (recorder.open(sdr::RecordDevice::OpenMode::Write))
         {
            logger.info("start streaming for device {}", {receiver.name()});

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
                  recorder.write(buffer.value());

//                     // convert I/Q samples to Real sample
//                     sdr::SignalBuffer result(buffer->elements(), 3, buffer->sampleRate(), 0, 0, 0);
//
//                     switch (buffer->type())
//                     {
//                        case sdr::SignalType::SAMPLE_IQ:
//                        {
//                           buffer->stream([&result](const float *value, int stride) {
//                              result.put(value[0]).put(value[1]).put(sqrtf(value[0] * value[0] + value[1] * value[1]));
//                           });
//                           break;
//                        }
//
//                        case sdr::SignalType::SAMPLE_REAL:
//                        {
//                           buffer->stream([&result](const float *value, int stride) {
//                              result.put(value[0]).put(0.0f).put(0.0f);
//                           });
//                           break;
//                        }
//                     }
//
//                     result.flip();
//
//                     recorder.write(result);
               }
            }

            receiver.stop();

            logger.info("stop streaming for device {}", {receiver.name()});
         }

         logger.info("capture finished");
      }
   }

   return 0;
}

int startApp(int argc, char *argv[])
{
   logger.info("***********************************************************************");
   logger.info("NFC laboratory, 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   logger.info("***********************************************************************");

   // create executor service
   Executor executor(128, 10);

   // startup signal resampling task
   executor.submit(nfc::AdaptiveSamplingTask::construct());

   // startup fourier transform task
   executor.submit(nfc::FourierProcessTask::construct());

   // startup signal decoder task
   executor.submit(nfc::FrameDecoderTask::construct());

   // startup frame writer task
   executor.submit(nfc::FrameStorageTask::construct());

   // startup signal reader task
   executor.submit(nfc::SignalRecorderTask::construct());

   // startup signal receiver task
   executor.submit(nfc::SignalReceiverTask::construct());

   // set logging handler
   qInstallMessageHandler(messageOutput);

   // configure scaling
   QtApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

   // initialize QT interface
   QtApplication app(argc, argv);

   // start application
   return QtApplication::exec();
}

int main(int argc, char *argv[])
{
//   return startTest1(argc, argv);
//   return startTest2(argc, argv);
   return startApp(argc, argv);
}


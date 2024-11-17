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

#include <cmath>

#include <rt/Logger.h>
#include <rt/Executor.h>
#include <rt/Subject.h>
#include <rt/Event.h>
#include <rt/BlockingQueue.h>

#include <hw/SignalType.h>
#include <hw/RecordDevice.h>

#include <hw/radio/AirspyDevice.h>
#include <hw/radio/RealtekDevice.h>

#include <lab/data/RawFrame.h>
#include <lab/nfc/NfcDecoder.h>

#include <lab/tasks/FrameStorageTask.h>
#include <lab/tasks/FourierProcessTask.h>
#include <lab/tasks/RadioDecoderTask.h>
#include <lab/tasks/RadioReceiverTask.h>
#include <lab/tasks/SignalResamplingTask.h>
#include <lab/tasks/SignalStorageTask.h>

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

   lab::NfcDecoder decoder;

   decoder.setEnableNfcA(false);
   decoder.setEnableNfcB(false);
   decoder.setEnableNfcF(true);
   decoder.setEnableNfcV(false);

   hw::RecordDevice source(argv[1]);

   if (source.open(hw::RecordDevice::OpenMode::Read))
   {
      while (!source.isEof())
      {
         hw::SignalBuffer samples(65536 * source.channelCount(), source.channelCount(), source.sampleRate(), 0, 0, hw::SignalType::SIGNAL_TYPE_RAW_REAL);

         if (source.read(samples) > 0)
         {
            std::list<lab::RawFrame> frames = decoder.nextFrames(samples);

            for (const lab::RawFrame &frame: frames)
            {
               if (frame.frameType() == lab::FrameType::PollFrame)
               {
                  log.info("frame at {} -> {}: TX {}", {frame.sampleStart(), frame.sampleEnd(), frame});
               }
               else if (frame.frameType() == lab::FrameType::ListenFrame)
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
   for (const auto &name: hw::AirspyDevice::listDevices())
   {
      // create receiver
      hw::AirspyDevice receiver(name);

      // default parameters
      receiver.setCenterFreq(40.68E6);
      receiver.setSampleRate(10E6);
      receiver.setGainMode(2);
      receiver.setGainValue(3);
      receiver.setMixerAgc(0);
      receiver.setTunerAgc(0);

      // try to open Airspy
      if (receiver.open(hw::SignalDevice::Read))
      {
         log.info("device {} connected!", {name});

         hw::RecordDevice recorder(file);

         recorder.setChannelCount(3);
         recorder.setSampleRate(receiver.sampleRate());
         recorder.open(hw::RecordDevice::Write);

         // open recorder
         if (recorder.open(hw::RecordDevice::OpenMode::Write))
         {
            log.info("start streaming for device {}", {receiver.name()});

            // signal stream queue buffer
            rt::BlockingQueue<hw::SignalBuffer> signalQueue;

            // start receive stream
            receiver.start([&signalQueue](hw::SignalBuffer &buffer) {
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
                  hw::SignalBuffer result(samples * 3, 3, buffer->sampleRate(), 0, 0, 0);

                  switch (buffer->type())
                  {
                     case hw::SignalType::SIGNAL_TYPE_RAW_IQ:
                     {
                        buffer->stream([&result](const float *value, int stride) {
                           result.put(value[0]).put(value[1]).put(sqrtf(value[0] * value[0] + value[1] * value[1]));
                        });
                        break;
                     }

                     case hw::SignalType::SIGNAL_TYPE_RAW_REAL:
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
   for (const auto &name: hw::RealtekDevice::listDevices())
   {
      // create receiver
      hw::RealtekDevice receiver(name);

      // default parameters
      receiver.setCenterFreq(27.12E6);
      receiver.setSampleRate(2400000);
      receiver.setGainMode(hw::RealtekDevice::Manual);
      receiver.setGainValue(77);
      receiver.setMixerAgc(0);
      receiver.setTunerAgc(0);
      receiver.setTestMode(0);

      // try to open Realtek
      if (receiver.open(hw::SignalDevice::Read))
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

         hw::RecordDevice recorder(file);

         recorder.setChannelCount(3);
         recorder.setSampleRate(receiver.sampleRate());
         recorder.open(hw::RecordDevice::Write);

         // open recorder
         if (recorder.open(hw::RecordDevice::OpenMode::Write))
         {
            log.info("start streaming for device {}", {receiver.name()});

            // signal stream queue buffer
            rt::BlockingQueue<hw::SignalBuffer> signalQueue;

            // start receive stream
            receiver.start([&signalQueue](hw::SignalBuffer &buffer) {
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
                  hw::SignalBuffer result(samples * 3, 3, buffer->sampleRate(), 0, 0, 0);

                  switch (buffer->type())
                  {
                     case hw::SignalType::SIGNAL_TYPE_RAW_IQ:
                     {
                        buffer->stream([&result](const float *value, int stride) {
                           result.put(value[0]).put(value[1]).put(sqrtf(value[0] * value[0] + value[1] * value[1]));
                        });
                        break;
                     }

                     case hw::SignalType::SIGNAL_TYPE_RAW_REAL:
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


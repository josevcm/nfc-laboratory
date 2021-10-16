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

#include <rt/Logger.h>
#include <rt/Executor.h>
#include <rt/Subject.h>
#include <rt/Event.h>

#include <sdr/RecordDevice.h>

#include <nfc/SignalReceiverTask.h>
#include <nfc/SignalRecorderTask.h>
#include <nfc/FrameDecoderTask.h>
#include <nfc/FrameStorageTask.h>
#include <nfc/FourierProcessTask.h>

#include <nfc/NfcFrame.h>
#include <nfc/NfcDecoder.h>

#include "QtApplication.h"

// https://beesbuzz.biz/code/4399-Embedding-binary-resources-with-CMake-and-C-11

using namespace rt;

Logger root("main");

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

int startTest(int argc, char *argv[])
{
   root.info("***********************************************************************");
   root.info("NFC laboratory, 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   root.info("***********************************************************************");

   nfc::NfcDecoder decoder;

   sdr::RecordDevice source(argv[1]);

   if (source.open(sdr::RecordDevice::OpenMode::Read))
   {
      while (!source.isEof())
      {
         sdr::SignalBuffer samples(65536 * source.channelCount(), source.channelCount(), source.sampleRate());

         if (source.read(samples) > 0)
         {
            std::list<nfc::NfcFrame> frames = decoder.nextFrames(samples);

            for (const nfc::NfcFrame &frame : frames)
            {
               if (frame.isPollFrame())
               {
                  root.info("frame at {} -> {}: TX {}", {frame.sampleStart(), frame.sampleEnd(), frame});
               }
               else if (frame.isListenFrame())
               {
                  root.info("frame at {} -> {}: RX {}", {frame.sampleStart(), frame.sampleEnd(), frame});
               }
            }
         }
      }
   }

   return 0;
}

int startApp(int argc, char *argv[])
{
   root.info("***********************************************************************");
   root.info("NFC laboratory, 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   root.info("***********************************************************************");

   // create executor service
   Executor executor(128, 10);

   // startup signal decoder task
   executor.submit(nfc::FrameDecoderTask::construct());

   // startup frame writer task
   executor.submit(nfc::FrameStorageTask::construct());

   // startup signal reader task
   executor.submit(nfc::SignalRecorderTask::construct());

   // startup signal receiver task
   executor.submit(nfc::SignalReceiverTask::construct());

   // startup fourier transform task
   executor.submit(nfc::FourierProcessTask::construct());

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
   return startTest(argc, argv);
//   return startApp(argc, argv);
}


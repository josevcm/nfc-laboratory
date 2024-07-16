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

#include <QDir>
#include <QStandardPaths>
#include <QDebug>

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

#include "QtConfig.h"
#include "QtApplication.h"

using namespace rt;

Logger *qlog = nullptr;

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

int startApp(int argc, char *argv[])
{
   Logger log {"main"};

   log.info("***********************************************************************");
   log.info("NFC laboratory, 2024 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   log.info("***********************************************************************");

   for (int i = 0; i < argc; i++)
      log.info("\t{}", {argv[i]});

   const struct libusb_version *lusbv = libusb_get_version();

   log.info("using libusb version: {}.{}.{}", {lusbv->major, lusbv->minor, lusbv->micro});

   // configure application
   QtApplication::setApplicationName(NFC_LAB_APPLICATION_NAME);
   QtApplication::setApplicationVersion(NFC_LAB_VERSION_STRING);
   QtApplication::setOrganizationName(NFC_LAB_COMPANY_NAME);
   QtApplication::setOrganizationDomain(NFC_LAB_DOMAIN_NAME);
   QtApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

   // configure settings location and format
   QSettings::setDefaultFormat(QSettings::IniFormat);

   // initialize QT interface
   QtApplication app(argc, argv);

   // create executor service
   Executor executor(128, 10);

   executor.submit(nfc::AdaptiveSamplingTask::construct()); // startup signal resampling task
   executor.submit(nfc::FourierProcessTask::construct()); // startup fourier transform task
   executor.submit(nfc::FrameDecoderTask::construct()); // startup signal decoder task
   executor.submit(nfc::FrameStorageTask::construct());
   executor.submit(nfc::SignalRecorderTask::construct()); // startup signal reader task
   executor.submit(nfc::SignalReceiverTask::construct()); // startup signal receiver task

   // start application
   QtApplication::exec();

   log.info("closing application...");

   return 0;
}

int main(int argc, char *argv[])
{
   // initialize logging system
#if NFC_LAB_VERSION_MAJOR > 0
   QDir appPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + NFC_LAB_COMPANY_NAME + "/" + NFC_LAB_APPLICATION_NAME);

   std::ofstream stream;

   if (appPath.mkpath("log"))
   {
      QString logFile = appPath.filePath("log/tracer.log");

      stream.open(logFile.toStdString(), std::ios::out | std::ios::app);

      if (stream.is_open())
      {
         Logger::init(stream);
      }
      else
      {
         std::cerr << "unable to open log file: " << logFile.toStdString() << std::endl;

         Logger::init(std::cout);
      }
   }
   else
   {
      std::cerr << "unable to create log path: " << appPath.absolutePath().toStdString() << std::endl;

      Logger::init(std::cout);
   }
#else
   Logger::init(std::cout);
   Logger::setWriterLevel(rt::Logger::DEBUG_LEVEL);
#endif

   // create QT logger
   qlog = new Logger("QT");

   // set logging handler for QT components
   qInstallMessageHandler(messageOutput);

   // start application!
   return startApp(argc, argv);
}


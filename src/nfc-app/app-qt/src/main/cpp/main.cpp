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
#include <fstream>
#include <iostream>

#include <QDir>
#include <QLocale>
#include <QSslSocket>
#include <QSettings>
#include <QStandardPaths>

#include <rt/Logger.h>
#include <rt/Executor.h>

#include <lab/tasks/FourierProcessTask.h>
#include <lab/tasks/LogicDecoderTask.h>
#include <lab/tasks/LogicDeviceTask.h>
#include <lab/tasks/RadioDecoderTask.h>
#include <lab/tasks/RadioDeviceTask.h>
#include <lab/tasks/SignalResamplingTask.h>
#include <lab/tasks/SignalStorageTask.h>
#include <lab/tasks/TraceStorageTask.h>

#include <styles/IconStyle.h>

#include <libusb.h>

#include "QtConfig.h"
#include "QtApplication.h"

rt::Logger *qlog = nullptr;

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
   rt::Logger *log = rt::Logger::getLogger("app.main", rt::Logger::INFO_LEVEL);

   log->info("***********************************************************************");
   log->info("NFC-LAB {}", {NFC_LAB_VERSION_STRING});
   log->info("***********************************************************************");

   if (argc > 1)
   {
      log->info("command line arguments:");

      for (int i = 1; i < argc; i++)
         log->info("\t{}", {argv[i]});
   }

   const libusb_version *lusbv = libusb_get_version();

   log->info("using usb library: {}.{}.{}", {lusbv->major, lusbv->minor, lusbv->micro});
   log->info("using ssl library: {}", {QSslSocket::sslLibraryBuildVersionString().toStdString()});
   log->info("using locale: {}", {QLocale().name().toStdString()});

   // override icons styles
   QtApplication::setStyle(new IconStyle());

   // configure application
   QtApplication::setApplicationName(NFC_LAB_APPLICATION_NAME);
   QtApplication::setApplicationVersion(NFC_LAB_VERSION_STRING);
   QtApplication::setOrganizationName(NFC_LAB_COMPANY_NAME);
   QtApplication::setOrganizationDomain(NFC_LAB_DOMAIN_NAME);

   // configure settings location and format
   QSettings::setDefaultFormat(QSettings::IniFormat);
   //   QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

   // configure loggers
   QSettings settings;

   settings.beginGroup("logger");

   for (const auto &key: settings.childKeys())
   {
      if (key == "root")
         rt::Logger::setRootLevel(settings.value(key).toString().toStdString());
      else
         rt::Logger::getLogger(key.toStdString())->setLevel(settings.value(key).toString().toStdString());
   }

   settings.endGroup();

   // create executor service
   rt::Executor executor(128, 10);

   executor.submit(lab::FourierProcessTask::construct()); // startup fourier transform
   executor.submit(lab::LogicDecoderTask::construct()); // startup logic decoder
   executor.submit(lab::LogicDeviceTask::construct()); // startup logic receiver
   executor.submit(lab::RadioDecoderTask::construct()); // startup radio decoder
   executor.submit(lab::RadioDeviceTask::construct()); // startup signal receiver
   executor.submit(lab::TraceStorageTask::construct()); // startup frame writer
   executor.submit(lab::SignalStorageTask::construct()); // startup signal reader
   executor.submit(lab::SignalResamplingTask::construct()); // startup signal resampling

   // initialize application
   QtApplication app(argc, argv);

   // start application
   return QtApplication::exec();
}

int main(int argc, char *argv[])
{
   // initialize logging system
#ifdef ENABLE_CONSOLE_LOGGING
   rt::Logger::init(std::cout);
#else

   QDir appPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + NFC_LAB_COMPANY_NAME + "/" + NFC_LAB_APPLICATION_NAME);

   std::ofstream stream;

   if (appPath.mkpath("log"))
   {
      QString logFile = appPath.filePath(QString("log/") + NFC_LAB_APPLICATION_NAME + ".log");

      stream.open(logFile.toStdString(), std::ios::out | std::ios::app);

      if (stream.is_open())
      {
         rt::Logger::init(stream);
      }
      else
      {
         std::cerr << "unable to open log file: " << logFile.toStdString() << std::endl;

         rt::Logger::init(std::cout);
      }
   }
   else
   {
      std::cerr << "unable to create log path: " << appPath.absolutePath().toStdString() << std::endl;

      rt::Logger::init(std::cout);
   }
#endif

   // create QT logger
   qlog = rt::Logger::getLogger("app.qt");

   // set logging handler for QT components
   qInstallMessageHandler(messageOutput);

   // start application!
   return startApp(argc, argv);
}

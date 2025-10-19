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

#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QSettings>
#include <QPointer>
#include <QThreadPool>
#include <QSplashScreen>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "QtCache.h"
#include "QtControl.h"
#include "QtWindow.h"

#include "features/Caps.h"

#include "styles/Theme.h"

#include "events/StreamFrameEvent.h"
#include "events/SystemStartupEvent.h"
#include "events/SystemShutdownEvent.h"
#include "events/DecoderControlEvent.h"

#include "QtApplication.h"

struct QtApplication::Impl
{
   // app reference
   QtApplication *app;

   // global settings
   QSettings settings;

   // decoder control
   QPointer<QtCache> cache;

   // decoder control
   QPointer<QtControl> control;

   // interface control
   QPointer<QtWindow> window;

   // splash screen
   QSplashScreen splash;

   // console output stream
   mutable QTextStream console;

   // signal connections
   QMetaObject::Connection applicationShutdownConnection;
   QMetaObject::Connection splashScreenCloseConnection;
   QMetaObject::Connection windowReloadConnection;

   // print frames flag
   bool printFramesEnabled = false;

   //  shutdown flag
   static bool shuttingDown;

   explicit Impl(QtApplication *app) : app(app), splash(QPixmap(":/app/app-splash"), Qt::WindowStaysOnTopHint), console(stdout)
   {
      // show splash screen
      showSplash(settings.value("settings/splashScreen", "2500").toInt());

      // create cache interface
      cache = new QtCache();

      // create decoder control interface
      control = new QtControl(cache);

      // create user interface window
      window = new QtWindow(cache);

      // connect shutdown signal
      applicationShutdownConnection = connect(app, &QtApplication::aboutToQuit, app, &QtApplication::shutdown);

      // connect window show signal
      splashScreenCloseConnection = connect(window, &QtWindow::ready, &splash, &QSplashScreen::close);

      // connect reload signal
      windowReloadConnection = connect(window, &QtWindow::reload, [=] { reload(); });
   }

   ~Impl()
   {
      disconnect(windowReloadConnection);
      disconnect(splashScreenCloseConnection);
      disconnect(applicationShutdownConnection);
   }

   void startup()
   {
      qInfo() << "startup QT Interface";

      selectTheme();

      QMap<QString, QString> meta;

      meta["devices"] = ".*";
      meta["features"] = "featureMenu";

      settings.beginGroup("features");

      for (const QString &entry: Caps::features())
      {
         if (settings.value(entry, true).toBool())
         {
            meta["features"] += "|" + entry;
         }
      }

      settings.endGroup();

      qDebug() << "features fields:" << (meta.isEmpty() ? "none" : "");

      for (const QString &entry: meta.keys())
      {
         qDebug().noquote().nospace() << "\t" << entry << ":" << meta[entry];
      }

      postEvent(instance(), new SystemStartupEvent(meta));

      if (arguments().size() > 1)
      {
         qInfo() << "with file" << arguments().at(1);

         QFile file(arguments().at(1));

         if (file.exists())
         {
            post(new DecoderControlEvent(DecoderControlEvent::ReadFile, {
                                            {"fileName", file.fileName()}
                                         }));
         }
      }

      // open main window in dark mode
      Theme::showInDarkMode(window);
   }

   void reload()
   {
      qInfo() << "reload QT Interface";

      // hide window
      window->hide();

      // restart interface
      startup();
   }

   void shutdown()
   {
      qInfo() << "shutdown QT Interface";

      postEvent(instance(), new SystemShutdownEvent);

      shuttingDown = true;
   }

   void showSplash(int timeout)
   {
      if (timeout > 0)
      {
         // show splash screen
         splash.show();

         // note, window is not valid until full initialization is completed! and execute event loop
         QTimer::singleShot(timeout, &splash, &QSplashScreen::close);
      }
   }

   void selectTheme() const
   {
      //      QString style = settings.value("settings/style", "Fusion").toString();
      QString theme = settings.value("settings/theme", "dark").toString();

      //      qInfo() << "selected style:" << style;
      qInfo().noquote() << "selected theme:" << theme;

      // set style: NOTE, this is not working properly, it seems that the IconStyle is not being applied correctly
      //      if (QStyleFactory::keys().contains(style))
      //      {
      //         setStyle(style);
      //      }
      //      else
      //      {
      //         qWarning() << "style" << style << "not found, available list:" << QStyleFactory::keys();
      //      }

      // configure stylesheet
      QFile styleFile(":qdarkstyle/" + theme + "/style.qss");

      if (styleFile.open(QFile::ReadOnly | QFile::Text))
      {
         app->setStyleSheet(QTextStream(&styleFile).readAll());
      }
      else
      {
         qWarning() << "unable to set stylesheet, file not found: " << styleFile.fileName();
      }

      QIcon::setThemeName(theme);
   }

   void setPrintFramesEnabled(bool enabled)
   {
      printFramesEnabled = enabled;
   }

   void handleEvent(QEvent *event) const
   {
      window->handleEvent(event);
      control->handleEvent(event);

      if (printFramesEnabled && event->type() == StreamFrameEvent::Type)
      {
         const auto *frameEvent = dynamic_cast<StreamFrameEvent *>(event);

         printFrameToTerminal(frameEvent->frame());
      }
   }

   void printFrameToTerminal(const lab::RawFrame &frame) const
   {
      if (!frame.isValid())
         return;

      char buffer[100];
      auto techType = "";
      auto frameType = "";
      std::string hexData;

      // Get tech type string
      switch (frame.techType())
      {
         case lab::NfcATech: techType = "NfcA";
            break;
         case lab::NfcBTech: techType = "NfcB";
            break;
         case lab::NfcFTech: techType = "NfcF";
            break;
         case lab::NfcVTech: techType = "NfcV";
            break;
         default: techType = "UNKNOWN";
      }

      // Get frame type string
      switch (frame.frameType())
      {
         case lab::NfcCarrierOff: frameType = "CarrierOff";
            break;
         case lab::NfcCarrierOn: frameType = "CarrierOn";
            break;
         case lab::NfcPollFrame: frameType = "Poll";
            break;
         case lab::NfcListenFrame: frameType = "Listen";
            break;
         default: frameType = "UNKNOWN";
      }

      // Build JSON output matching TRZ structure
      QJsonObject frameObject;

      frameObject["timestamp"] = static_cast<qint64>(frame.sampleStart());
      frameObject["tech"] = techType;
      frameObject["type"] = frameType;

      // Add numeric enum values (matching TRZ)
      frameObject["tech_type"] = static_cast<qint64>(frame.techType());
      frameObject["frame_type"] = static_cast<qint64>(frame.frameType());

      // Add time_start and time_end (matching TRZ)
      frameObject["time_start"] = frame.timeStart();
      frameObject["time_end"] = frame.timeEnd();

      // Add sample info if available
      frameObject["sample_start"] = static_cast<qint64>(frame.sampleStart());
      frameObject["sample_end"] = static_cast<qint64>(frame.sampleEnd());
      frameObject["sample_rate"] = static_cast<qint64>(frame.sampleRate());

      // Add datetime if available
      if (frame.dateTime() > 0)
         frameObject["date_time"] = frame.dateTime();

      // Add rate if available
      if (frame.frameRate() > 0)
         frameObject["rate"] = static_cast<qint64>(frame.frameRate());

      // Add data if available
      if (!frame.isEmpty())
      {
         QByteArray dataArray(reinterpret_cast<const char *>(frame.ptr()), frame.limit());
         frameObject["data"] = QString(dataArray.toHex(':'));
         frameObject["length"] = static_cast<qint64>(frame.limit());
      }

      // Add flags array for easy parsing
      QJsonArray flagsList;

      if (frame.hasFrameFlags(lab::CrcError))
         flagsList.append("crc-error");

      if (frame.hasFrameFlags(lab::ParityError))
         flagsList.append("parity-error");

      if (frame.hasFrameFlags(lab::SyncError))
         flagsList.append("sync-error");

      if (frame.hasFrameFlags(lab::Truncated))
         flagsList.append("truncated");

      if (frame.hasFrameFlags(lab::Encrypted))
         flagsList.append("encrypted");

      // Add frame type to flags (matching TRZ behavior)
      if (frame.frameType() == lab::NfcPollFrame || frame.frameType() == lab::IsoRequestFrame)
         flagsList.append("request");
      else if (frame.frameType() == lab::NfcListenFrame || frame.frameType() == lab::IsoResponseFrame)
         flagsList.append("response");

      if (!flagsList.isEmpty())
         frameObject["flags"] = flagsList;

      const QJsonDocument doc(frameObject);

      // Print to console and flush
      console << doc.toJson(QJsonDocument::Compact) << Qt::endl;
      console.flush();
   }
};

bool QtApplication::Impl::shuttingDown = false;

QtApplication::QtApplication(int &argc, char **argv) : QApplication(argc, argv), impl(new Impl(this))
{
   // setup thread pool
   QThreadPool::globalInstance()->setMaxThreadCount(8);

   // startup interface
   QTimer::singleShot(0, this, [=] { startup(); });
}

void QtApplication::startup()
{
   impl->startup();
}

void QtApplication::shutdown()
{
   impl->shutdown();
}

void QtApplication::post(QEvent *event, int priority)
{
   if (!Impl::shuttingDown)
      postEvent(instance(), event, priority);
}

QDir QtApplication::dataPath()
{
   return {QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/data"};
}

QDir QtApplication::tempPath()
{
   return {QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/tmp"};
}

QFile QtApplication::dataFile(const QString &fileName)
{
   QDir dataPath = QtApplication::dataPath();

   if (!dataPath.exists())
      dataPath.mkpath(".");

   return QFile(dataPath.absoluteFilePath(fileName));
}

QFile QtApplication::tempFile(const QString &fileName)
{
   QDir tempPath = QtApplication::tempPath();

   if (!tempPath.exists())
      tempPath.mkpath(".");

   return QFile(tempPath.absoluteFilePath(fileName));
}

void QtApplication::customEvent(QEvent *event)
{
   impl->handleEvent(event);
}

void QtApplication::setPrintFramesEnabled(bool enabled)
{
   impl->setPrintFramesEnabled(enabled);
}

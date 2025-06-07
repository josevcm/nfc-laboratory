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

#include "QtDecoder.h"
#include "QtWindow.h"

#include "features/Caps.h"

#include "styles/Theme.h"

#include "events/SystemStartupEvent.h"
#include "events/SystemShutdownEvent.h"
#include "events/DecoderControlEvent.h"

#include "dialogs/LicenseDialog.h"

#include "QtApplication.h"

struct QtApplication::Impl
{
   // app reference
   QtApplication *app;

   // global settings
   QSettings settings;

   // interface control
   QPointer<QtWindow> window;

   // decoder control
   QPointer<QtDecoder> decoder;

   // splash screen
   QSplashScreen splash;

   // signal connections
   QMetaObject::Connection applicationShutdownConnection;
   QMetaObject::Connection splashScreenCloseConnection;
   QMetaObject::Connection windowReloadConnection;

   //  shutdown flag
   static bool shuttingDown;

   explicit Impl(QtApplication *app) : app(app), splash(QPixmap(":/app/app-splash"), Qt::WindowStaysOnTopHint)
   {
      // show splash screen
      showSplash(settings.value("settings/splashScreen", "2500").toInt());

      // create user interface window
      window = new QtWindow();

      // create decoder control interface
      decoder = new QtDecoder();

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

   void handleEvent(QEvent *event) const
   {
      window->handleEvent(event);
      decoder->handleEvent(event);
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

void QtApplication::customEvent(QEvent *event)
{
   impl->handleEvent(event);
}

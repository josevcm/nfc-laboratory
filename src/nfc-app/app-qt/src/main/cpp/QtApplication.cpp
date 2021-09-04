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

#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QSettings>
#include <QPointer>
#include <QThreadPool>

#include "QtWindow.h"
#include "QtDecoder.h"

#include "events/SystemStartupEvent.h"
#include "events/SystemShutdownEvent.h"
#include "events/DecoderControlEvent.h"

#include "QtApplication.h"

struct QtApplication::Impl
{
   // Configuracion
   QSettings settings;

   // interface control
   QPointer<QtWindow> window;

   // receiver control
   QPointer<QtDecoder> control;

   Impl() : settings("conf/nfc-lab.conf", QSettings::IniFormat)
   {
      // create user interface window
      window = new QtWindow(settings);

      // create decoder control interface
      control = new QtDecoder(settings);
   }

   void startup()
   {
      qInfo() << "startup QT Interface";

      QApplication::postEvent(QApplication::instance(), new SystemStartupEvent());

      QFile file("startup.wav");

      if (file.exists())
      {
         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReadFile, "file", file.fileName()));
      }
   }

   void shutdown()
   {
      QApplication::postEvent(QApplication::instance(), new SystemShutdownEvent);

      qInfo() << "shutdown QT Interface";
   }

   void handleEvent(QEvent *event)
   {
      window->handleEvent(event);
      control->handleEvent(event);
   }
};

QtApplication::QtApplication(int argc, char **argv) : QApplication(argc, argv), impl(new Impl)
{
   // initialize application name
   setApplicationName("nfc-lab");

   // setup thread pool
   QThreadPool::globalInstance()->setMaxThreadCount(8);

   // connect startup signal
   QTimer::singleShot(0, this, &QtApplication::startup);

   // connect shutdown signal
   QObject::connect(this, &QtApplication::aboutToQuit, this, &QtApplication::shutdown);

   // configure stylesheet
   QFile style(":qdarkstyle/dark/style.qss");

   if (style.exists())
   {
      style.open(QFile::ReadOnly | QFile::Text);
      QTextStream ts(&style);
      setStyleSheet(ts.readAll());
   }
   else
   {
      qDebug() << "Unable to set stylesheet, file not found";
   }
}

void QtApplication::startup()
{
   impl->startup();
}

void QtApplication::shutdown()
{
   impl->shutdown();
}

void QtApplication::post(QEvent *event)
{
   QApplication::postEvent(QApplication::instance(), event);
}

void QtApplication::customEvent(QEvent *event)
{
   impl->handleEvent(event);
}


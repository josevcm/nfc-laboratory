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

#include "QtDecoder.h"
#include "QtMemory.h"
#include "QtWindow.h"

#include "events/SystemStartupEvent.h"
#include "events/SystemShutdownEvent.h"
#include "events/DecoderControlEvent.h"

#include "QtApplication.h"

struct QtApplication::Impl
{
   // Configuracion
   QSettings settings;

   // interface control
   QPointer<QtMemory> memory;

   // interface control
   QPointer<QtWindow> window;

   // decoder control
   QPointer<QtDecoder> decoder;

   Impl() : settings("nfc-lab.conf", QSettings::IniFormat)
   {
      // create signal cache
      memory = new QtMemory(settings);

      // create user interface window
      window = new QtWindow(settings, memory);

      // create decoder control interface
      decoder = new QtDecoder(settings, memory);
   }

   void startup()
   {
      qInfo() << "startup QT Interface";

      QApplication::postEvent(QApplication::instance(), new SystemStartupEvent());

      if (arguments().size() > 1)
      {
         qInfo() << "with file" << arguments().at(1);

         QFile file(arguments().at(1));

         if (file.exists())
         {
            QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReadFile, {
                  {"fileName", file.fileName()}
            }));
         }
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
      decoder->handleEvent(event);
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

void QtApplication::post(QEvent *event, int priority)
{
   QApplication::postEvent(QApplication::instance(), event, priority);
}

void QtApplication::customEvent(QEvent *event)
{
   impl->handleEvent(event);
}


/*

  Copyright (c) 2020 Jose Vicente Campos Martinez - <josevcm@gmail.com>

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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <QtCore>

#include <interface/MainWindow.h>

#include <decoder/NfcStream.h>
#include <decoder/NfcService.h>

#include <storage/StorageService.h>

#include <events/StorageControlEvent.h>

#include <support/ElapsedLogger.h>

#include <Application.h>
#include <Dispatcher.h>

Application::Application(int argc, char** argv) :
   QApplication(argc, argv), mSettings("conf/nfy.conf", QSettings::IniFormat)
{
   qInfo() << "*******************************************************************";
   qInfo() << "Starting" << applicationName() << ", pid" << applicationPid();
   qInfo() << "*******************************************************************";

   // creamos flujo de tramas
   mStream = new NfcStream();

   // interface de usuario
   mWindow = new MainWindow(mSettings, mStream);

   // creamos decodificador de tramas
   mDecoder = new NfcService(mSettings, mStream);

   // creamos almacenamiento de tramas
   mStorage = new StorageService(mSettings, mStream);

   // connect startup signal
   QTimer::singleShot(0, this, &Application::startup);

   // configure stylesheed
   QFile style(":qdarkstyle/style.qss");

   if (!style.exists())
   {
      qDebug() << "Unable to set stylesheet, file not found";
   }
   else
   {
      style.open(QFile::ReadOnly | QFile::Text);
      QTextStream ts(&style);
      setStyleSheet(ts.readAll());
   }
}

Application::~Application()
{
   delete mStorage;
   delete mDecoder;
   delete mWindow;
   delete mStream;

   qInfo() << "*******************************************************************";
   qInfo() << "Terminate" << applicationName() << ", pid" << applicationPid();
   qInfo() << "*******************************************************************";
}

void Application::startup()
{
   mDecoder->searchDevices();

   QStringList params = arguments();

   if (params.length() > 1)
   {
      QFileInfo fileInfo(params[1]);

      if (fileInfo.isFile())
      {
         qDebug() << "autoload file " << fileInfo.filePath();

         postEvent(this, new StorageControlEvent(StorageControlEvent::Read, "file", fileInfo.filePath()));
      }
   }
}

void Application::customEvent(QEvent * event)
{
   // send event to UI
   mWindow->customEvent(event);

   // send event to decoder
   mDecoder->customEvent(event);

   // send event to storage
   mStorage->customEvent(event);
}


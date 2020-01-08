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

#include <Application.h>

//#include <devices/LimeDevice.h>

QString toString(const QByteArray &value);

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
   qInstallMessageHandler(messageOutput);

   QCoreApplication::setApplicationName("nfy");

   QSettings::setDefaultFormat(QSettings::IniFormat);

   QThreadPool::globalInstance()->setMaxThreadCount(8);

   Application app(argc, argv);

   return app.exec();
}

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
   QByteArray localMsg = msg.toLocal8Bit();
   QByteArray localNow = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz").toLocal8Bit();

   switch (type)
   {
      case QtDebugMsg:
         fprintf(stdout, "%s DEBUG - %s\n", localNow.constData(), localMsg.constData());
         break;
      case QtInfoMsg:
         fprintf(stdout, "%s INFO  - %s\n", localNow.constData(), localMsg.constData());
         break;
      case QtWarningMsg:
         fprintf(stdout, "%s WARN  - %s\n", localNow.constData(), localMsg.constData());
         break;
      case QtCriticalMsg:
         fprintf(stdout, "%s CRIT  - %s\n", localNow.constData(), localMsg.constData());
         break;
      case QtFatalMsg:
         fprintf(stdout, "%s FATAL - %s\n", localNow.constData(), localMsg.constData());
         abort();
   }

   fflush(stdout);
}

QString toString(const QByteArray &value)
{
   QString text;

   for (QByteArray::ConstIterator it = value.begin(); it != value.end(); it++)
   {
      text.append(QString("%1 ").arg(*it & 0xff, 2, 16, QLatin1Char('0')));
   }

   return text;
}


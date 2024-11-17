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

#ifndef APP_QTAPPLICATION_H
#define APP_QTAPPLICATION_H

#include <QSettings>
#include <QApplication>
#include <QMainWindow>
#include <QSharedPointer>

class QtApplication : public QApplication
{
   Q_OBJECT

      struct Impl;

   public:

      QtApplication(int &argc, char **argv);

      static void post(QEvent *event, int priority = Qt::NormalEventPriority);

   protected:

      void startup();

      void shutdown();

      void customEvent(QEvent *event) override;

   private:

      QSharedPointer<Impl> impl;
};

#endif

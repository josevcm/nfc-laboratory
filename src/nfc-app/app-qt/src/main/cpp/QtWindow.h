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

#ifndef APP_QTWINDOW_H
#define APP_QTWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QSharedPointer>

class QtWindow : public QMainWindow
{
      Q_OBJECT

      struct Impl;

   public:

      explicit QtWindow();

      void handleEvent(QEvent *event);

   public Q_SLOTS:

      void openFile();

      void saveFile();

      void saveSelection();

      void openConfig();

      void toggleListen();

      void toggleRecord();

      void toggleStop();

      void toggleTime();

      void toggleFollow();

      void toggleFilter();

      void toggleFeature();

      void toggleProtocol();

      void showAboutInfo();

      void showHelpInfo();

      void changeGainValue(int value);

      void trackGainValue(int value);

      void clearView();

      void resetView();

      void zoomReset();

      void zoomSelection();

   Q_SIGNALS:

      void ready();

      void reload();

   protected:

      void keyPressEvent(QKeyEvent *event) override;

      void closeEvent(QCloseEvent *event) override;

   private:

      QSharedPointer<Impl> impl;
};

#endif /* MAINWINDOW_H */

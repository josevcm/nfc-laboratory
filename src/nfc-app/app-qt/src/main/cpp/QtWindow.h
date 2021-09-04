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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#ifndef APP_QTWINDOW_H
#define APP_QTWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QSharedPointer>

class QItemSelection;
class QCPRange;

class QtWindow : public QMainWindow
{
      struct Impl;

   Q_OBJECT

   public:

      explicit QtWindow(QSettings &settings);

      void handleEvent(QEvent *event);

   public Q_SLOTS:

      void clearView();

      void openFile();

      void saveFile();

      void toggleListen();

      void toggleRecord();

      void toggleStop();

      void toggleLive();

      void toggleFollow();

      void toggleFilter();

      void toggleTunerAgc(bool value);

      void toggleMixerAgc(bool value);

      void changeGainMode(int index);

      void changeGainValue(int value);

   protected:

      void keyPressEvent(QKeyEvent *event) override;

   private:

      QSharedPointer<Impl> impl;
};

#endif /* MAINWINDOW_H */

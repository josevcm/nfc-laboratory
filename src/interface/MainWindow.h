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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

class QItemSelection;

class NfcStream;
class FrameModel;
class StreamStatusEvent;
class RecordDecodeEvent;
class ConsoleLogEvent;
class StreamFrameEvent;
class PlotMarker;

class QCPRange;
class QCPAbstractPlottable;

namespace Ui
{
   class MainWindow;
}

class MainWindow: public QMainWindow
{
      Q_OBJECT

   public:

      MainWindow(QSettings &settings, NfcStream *stream);

      virtual ~MainWindow();

      void customEvent(QEvent *event);

   public Q_SLOTS:

      void openFile();
      void saveFile();
      void toggleListen();
      void toggleRecord();
      void toggleStop();
      void toggleLive();
      void toggleFollow();
      void toggleFilter();
      void openSettings();
      void gainChanged(int value);
      void refreshView();
      void clearView();
      void clearModel();
      void clearGraph();

      void plotMousePress(QMouseEvent *event);
      void plotSelectionChanged();
      void viewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
      void plotRangeChanged(const QCPRange &newRange);

//      void plottableClick(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event);

   protected:

      void setupUi();

      void keyPressEvent(QKeyEvent *event);
      void consoleLogEvent(ConsoleLogEvent *event);
      void streamStatusEvent(StreamStatusEvent *event);
      void streamFrameEvent(StreamFrameEvent *event);

      void setDecoderStatus(int decoderStatus);
      void setSourceDevice(const QString &sourceDevice);
      void setFrequency(float frequency);
      void setSampleRate(float sampleRate);
      void setSampleCount(long sampleCount);
      void setSignalPower(float signalPower);
      void setStreamProgress(float timeLimit);
      void setLiveEnabled(bool liveEnabled);
      void setFilterEnabled(bool filterEnabled);
      void setFollowEnabled(bool followEnabled);
      void setDeviceList(const QStringList &deviceList);
      void setFrequencyList(const QVector<long> &frequencyList);
      void setTunerGainList(const QVector<float> &tunerGainList);

      void updateHeader();
      void updateRange(double start, double end);

   private:

      Ui::MainWindow *ui;

   protected:

      // configuration
      QSettings &mSettings;

      // frame stream data
      NfcStream *mStream;

      // current selected source
      QString mSourceDevice;

      // devices detected
      QStringList mDeviceList;

      // available frequencies
      QVector<long> mFrequencyList;

      // available gains
      QVector<float> mTunerGainList;

      // Clipboard data
      QString mClipboard;

      // decoder parameters
      int mDecoderStatus;
      float mFrequency;
      long mSampleRate;
      long mSampleCount;

      // Toolbar status
      int mSelectionMode;
      bool mLiveEnabled;
      bool mRecordEnabled;
      bool mFollowEnabled;
      bool mFilterEnabled;

      // refresh timer
      QTimer *mRefreshTimer;

      // Frame view model
      FrameModel *mFrameModel;

      // chart marker
      PlotMarker *mPlotMarker;

      // plot range limits
      double mLowerSignalRange;
      double mUpperSignalRange;
};

#endif /* MAINWINDOW_H */

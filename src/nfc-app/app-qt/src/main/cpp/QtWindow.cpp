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
#include <QKeyEvent>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QPointer>
#include <QTimer>
#include <QDateTime>
#include <QScrollBar>
#include <QItemSelection>

#include <rt/Subject.h>
#include <sdr/SignalBuffer.h>

#include <model/StreamModel.h>
#include <model/ParserModel.h>

#include <events/ConsoleLogEvent.h>
#include <events/DecoderControlEvent.h>
#include <events/DecoderStatusEvent.h>
#include <events/ReceiverStatusEvent.h>
#include <events/StreamFrameEvent.h>
#include <events/SystemStartupEvent.h>
#include <events/SystemShutdownEvent.h>
#include <events/StorageStatusEvent.h>
#include <events/SignalBufferEvent.h>

#include <styles/StreamStyle.h>
#include <styles/ParserStyle.h>

#include <views/ui_MainView.h>

#include "QtApplication.h"

#include "QtMemory.h"
#include "QtWindow.h"

struct QtWindow::Impl
{
   // configuration
   QSettings &settings;

   // signal memory cache
   QtMemory *cache;

   // Toolbar status
   bool liveEnabled = false;
   bool recordEnabled = false;
   bool followEnabled = false;
   bool filterEnabled = false;

   // receiver parameters
   QStringList deviceList;
   QMap<int, QString> receiverGainModes;
   QMap<int, QString> receiverGainValues;

   // current device parameters
   QString receiverName;
   QString receiverType;
   QString receiverStatus;
   int receiverFrequency = 0;
   int receiverSampleRate = 0;
   int receiverSampleCount = 0;
   int receiverGainMode = -1;
   int receiverGainValue = -1;
   int receiverTunerAgc = -1;
   int receiverMixerAgc = -1;

   // interface
   QSharedPointer<Ui_MainView> ui;

   // Frame view model
   QPointer<StreamModel> streamModel;

   // Frame view model
   QPointer<ParserModel> parserModel;

   // refresh timer
   QPointer<QTimer> refreshTimer;

   // Clipboard data
   QString clipboard;

   // IQ signal data subject
   rt::Subject<sdr::SignalBuffer> *signalIqStream = nullptr;

   // fft signal data subject
   rt::Subject<sdr::SignalBuffer> *frequencyStream = nullptr;

   // IQ signal stream subscription
   rt::Subject<sdr::SignalBuffer>::Subscription signalIqSubscription;

   // fft signal stream subscription
   rt::Subject<sdr::SignalBuffer>::Subscription frequencySubscription;

   explicit Impl(QSettings &settings, QtMemory *cache) : settings(settings), cache(cache), ui(new Ui_MainView()), streamModel(new StreamModel()), parserModel(new ParserModel()), refreshTimer(new QTimer())
   {
      // IQ signal subject stream
      signalIqStream = rt::Subject<sdr::SignalBuffer>::name("signal.iq");

      // fft signal subject stream
      frequencyStream = rt::Subject<sdr::SignalBuffer>::name("signal.fft");

      // subscribe to signal events
      signalIqSubscription = signalIqStream->subscribe([=](const sdr::SignalBuffer &buffer) {
         ui->quadratureView->refresh(buffer);
      });

      // subscribe to signal events
      frequencySubscription = frequencyStream->subscribe([=](const sdr::SignalBuffer &buffer) {
         ui->frequencyView->refresh(buffer);
      });
   }

   void setupUi(QtWindow *mainWindow)
   {
      ui->setupUi(mainWindow);

      // setup default controls status
      ui->gainMode->setEnabled(false);
      ui->gainValue->setEnabled(false);
      ui->tunerAgc->setEnabled(false);
      ui->mixerAgc->setEnabled(false);

      ui->listenButton->setEnabled(false);
      ui->recordButton->setEnabled(false);
      ui->stopButton->setEnabled(false);

      // setup display stretch
      ui->workbench->setStretchFactor(0, 3);
      ui->workbench->setStretchFactor(1, 2);

      // setup frame view model
      ui->streamView->setModel(streamModel);
      ui->streamView->setColumnWidth(StreamModel::Id, 75);
      ui->streamView->setColumnWidth(StreamModel::Time, 100);
      ui->streamView->setColumnWidth(StreamModel::Delta, 75);
      ui->streamView->setColumnWidth(StreamModel::Rate, 60);
      ui->streamView->setColumnWidth(StreamModel::Tech, 60);
      ui->streamView->setColumnWidth(StreamModel::Cmd, 100);
      ui->streamView->setColumnWidth(StreamModel::Flags, 48);
      ui->streamView->setItemDelegate(new StreamStyle(ui->streamView));

      // setup protocol view model
      ui->parserView->setModel(parserModel);
      ui->parserView->setColumnWidth(ParserModel::Cmd, 120);
      ui->parserView->setColumnWidth(ParserModel::Flags, 32);
      ui->parserView->setItemDelegate(new ParserStyle(ui->parserView));

      // connect selection signal from frame model
//      QObject::connect(ui->streamView->verticalScrollBar(), &QScrollBar::valueChanged, [=](int position) {
//         streamScrollChanged();
//      });

      // connect selection signal from frame model
      QObject::connect(ui->streamView->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &deselected) {
         streamSelectionChanged();
      });

      // connect selection signal from timing graph
      QObject::connect(ui->framesView, &FramesWidget::selectionChanged, [=](double from, double to) {
         timingSelectionChanged(from, to);
      });

      // connect selection signal from timing graph
      QObject::connect(ui->signalView, &SignalWidget::selectionChanged, [=](double from, double to) {
         signalSelectionChanged(from, to);
      });

      // connect selection signal from frame model
      QObject::connect(ui->signalView, &SignalWidget::rangeChanged, [=](float from, float to) {
         signalRangeChanged(from, to);
      });

      // connect refresh timer signal
      QObject::connect(refreshTimer, &QTimer::timeout, [=]() {
         refreshView();
      });

      // start timer
      refreshTimer->start(250);
   }

   void systemStartup(SystemStartupEvent *event)
   {
      // configure decoder parameters on startup
      DecoderControlEvent *decoderControlEvent = new DecoderControlEvent(DecoderControlEvent::DecoderConfig);

      for (QString &group: settings.childGroups())
      {
         if (group.startsWith("decoder"))
         {
            settings.beginGroup(group);

            int sep = group.indexOf(".");

            for (QString &key: settings.childKeys())
            {
               if (sep > 0)
               {
                  QString nfc = group.mid(sep + 1);

                  if (key.toLower().contains("enabled"))
                     decoderControlEvent->setBoolean(nfc + "/" + key, settings.value(key).toBool());
                  else
                     decoderControlEvent->setFloat(nfc + "/" + key, settings.value(key).toFloat());
               }
               else
               {
                  decoderControlEvent->setFloat(key, settings.value(key).toFloat());
               }
            }

            settings.endGroup();
         }
      }

      QtApplication::post(decoderControlEvent);
   }

   void systemShutdown(SystemShutdownEvent *event)
   {
   }

   void decoderStatusEvent(DecoderStatusEvent *event)
   {
      if (event->hasStatus())
      {
         if (event->status() == DecoderStatusEvent::Idle)
         {
            ui->framesView->refresh();
            ui->signalView->refresh();
         }
      }
   }

   void receiverStatusEvent(ReceiverStatusEvent *event)
   {
      if (event->hasGainModeList())
         setReceiverGainModes(event->gainModeList());

      if (event->hasGainValueList())
         setReceiverGainValues(event->gainValueList());

      if (event->hasReceiverName())
         setReceiverDevice(event->source());

      if (event->hasReceiverStatus())
         setReceiverStatus(event->status());

      if (event->hasSignalPower())
         setSignalPower(event->signalPower());

      if (event->hasSampleCount())
         setReceiverSampleCount(event->sampleCount());
   }

   void storageStatusEvent(StorageStatusEvent *event) const
   {
      if (event->hasFileName())
      {
         ui->headerLabel->setText(event->fileName());
      }
   }

   void streamFrameEvent(StreamFrameEvent *event) const
   {
      const auto &frame = event->frame();

      // add data frames to stream model (omit carrier lost and empty frames)
      if (frame.isPollFrame() || frame.isListenFrame())
      {
         streamModel->append(frame);
      }

      // add all frames to timing graph
      ui->framesView->append(frame);
   }

   void signalBufferEvent(SignalBufferEvent *event) const
   {
      ui->signalView->append(event->buffer());
   }

   void consoleLogEvent(ConsoleLogEvent *event)
   {
   }

   void setReceiverStatus(const QString &value)
   {
      if (receiverStatus != value)
      {
         qInfo() << "receiver status changed:" << value;

         receiverStatus = value;

         if (receiverStatus == ReceiverStatusEvent::NoDevice)
         {
            ui->listenButton->setEnabled(false);
            ui->recordButton->setEnabled(false);
            ui->stopButton->setEnabled(false);

            ui->gainMode->setEnabled(false);
            ui->gainValue->setEnabled(false);
            ui->tunerAgc->setEnabled(false);
            ui->mixerAgc->setEnabled(false);

            ui->statusBar->showMessage("No device found");
         }

         else if (receiverStatus == ReceiverStatusEvent::Idle)
         {
            ui->listenButton->setEnabled(true);
            ui->recordButton->setEnabled(true);
            ui->stopButton->setEnabled(false);

            ui->gainMode->setEnabled(true);
            ui->gainValue->setEnabled(true);
            ui->tunerAgc->setEnabled(true);
            ui->mixerAgc->setEnabled(true);
         }

         else if (receiverStatus == ReceiverStatusEvent::Streaming)
         {
            ui->listenButton->setEnabled(false);
            ui->recordButton->setEnabled(false);
            ui->stopButton->setEnabled(true);

            ui->gainMode->setEnabled(true);
            ui->gainValue->setEnabled(true);
            ui->tunerAgc->setEnabled(true);
            ui->mixerAgc->setEnabled(true);

            setLiveEnabled(true);
         }
      }
   }

   void setReceiverDevice(const QString &value)
   {
      if (receiverName != value)
      {
         qInfo() << "receiver device changed:" << value;

         receiverName = value;

         if (!receiverName.isEmpty())
         {
            receiverType = receiverName.left(receiverName.indexOf("://"));

            ui->statusBar->showMessage(receiverName);

            setReceiverFrequency(settings.value("device." + receiverType + "/centerFreq", "13560000").toInt());
            setReceiverSampleRate(settings.value("device." + receiverType + "/sampleRate", "10000000").toInt());
            setReceiverGainMode(settings.value("device." + receiverType + "/gainMode", "1").toInt());
            setReceiverGainValue(settings.value("device." + receiverType + "/gainValue", "6").toInt());
            setReceiverMixerAgc(settings.value("device." + receiverType + "/mixerAgc", "0").toInt());
            setReceiverTunerAgc(settings.value("device." + receiverType + "/tunerAgc", "0").toInt());

            ui->eventsLog->append(QString("Detected device %1").arg(receiverName));
         }

         updateHeader();
      }
   }

   void setReceiverGainModes(const QMap<int, QString> &value)
   {
      if (receiverGainModes != value)
      {
         qInfo() << "receiver gains modes changed:" << value;

         receiverGainModes = value;

         ui->gainMode->blockSignals(true);
         ui->gainMode->clear();

         for (auto const &mode: receiverGainModes.keys())
            ui->gainMode->addItem(receiverGainModes[mode], mode);

         ui->gainMode->setCurrentIndex(ui->gainMode->findData(receiverGainMode));
         ui->gainMode->blockSignals(false);
      }
   }

   void setReceiverGainValues(const QMap<int, QString> &value)
   {
      if (receiverGainValues != value)
      {
         qInfo() << "receiver gains values changed:" << value;

         receiverGainValues = value;

         if (!receiverGainValues.isEmpty())
         {
            ui->gainValue->setRange(receiverGainValues.keys().first(), receiverGainValues.keys().last());
            ui->gainValue->setValue(receiverGainValue);

         }
         else
         {
            ui->gainValue->setRange(0, 0);
         }
      }
   }

   void setReceiverFrequency(long value)
   {
      if (receiverFrequency != value)
      {
         qInfo() << "receiver frequency changed:" << value;

         receiverFrequency = value;

         ui->frequencyView->setCenterFreq(receiverFrequency);
         ui->quadratureView->setCenterFreq(receiverFrequency);

         if (!receiverType.isEmpty())
            settings.setValue("device." + receiverType + "/centerFreq", receiverFrequency);

         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverConfig, {{"centerFreq", receiverFrequency}}));

         updateHeader();
      }
   }

   void setReceiverSampleRate(long value)
   {
      if (receiverSampleRate != value)
      {
         qInfo() << "receiver samplerate changed:" << value;

         receiverSampleRate = value;

         ui->frequencyView->setSampleRate(receiverSampleRate);
         ui->quadratureView->setSampleRate(receiverSampleRate);

         if (!receiverType.isEmpty())
            settings.setValue("device." + receiverType + "/sampleRate", receiverSampleRate);

         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverConfig, {{"sampleRate", receiverSampleRate}}));

         updateHeader();
      }
   }

   void setReceiverGainMode(int value)
   {
      if (receiverGainMode != value)
      {
         qInfo() << "receiver gain mode changed:" << value;

         receiverGainMode = value;

         if (!receiverType.isEmpty())
            settings.setValue("device." + receiverType + "/gainMode", receiverGainMode);

         if (ui->gainMode->count() > 0)
         {
            if (receiverGainMode)
            {
               ui->gainValue->setValue(receiverGainValue);
               ui->gainLabel->setText(QString("Gain %1").arg(receiverGainValues[receiverGainValue]));
            }
            else
            {
               ui->gainValue->setValue(0);
               ui->gainLabel->setText(QString("Gain AUTO"));
            }

            ui->tunerAgc->setEnabled(receiverGainMode == 0);
            ui->mixerAgc->setEnabled(receiverGainMode == 0);
            ui->gainValue->setEnabled(receiverGainMode != 0);
            ui->gainMode->setCurrentIndex(ui->gainMode->findData(receiverGainMode));

            QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverConfig, {
                  {"gainMode",  receiverGainMode},
                  {"gainValue", receiverGainValue}
            }));
         }
      }
   }

   void setReceiverGainValue(int value)
   {
      if (receiverGainValue != value)
      {
         qInfo() << "receiver gain value changed:" << value;

         receiverGainValue = value;

         ui->gainValue->setValue(receiverGainValue);
         ui->gainLabel->setText(QString("Gain %1").arg(receiverGainValues[receiverGainValue]));

         if (!receiverType.isEmpty())
            settings.setValue("device." + receiverType + "/gainValue", receiverGainValue);

         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverConfig, {
               {"gainMode",  receiverGainMode},
               {"gainValue", receiverGainValue}
         }));
      }
   }

   void setReceiverTunerAgc(int value)
   {
      if (receiverTunerAgc != value)
      {
         qInfo() << "receiver tuner AGC changed:" << value;

         receiverTunerAgc = value;

         ui->tunerAgc->setEnabled(receiverTunerAgc >= 0);
         ui->tunerAgc->setChecked(receiverTunerAgc == 1);

         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverConfig, {{"tunerAgc", receiverTunerAgc}}));
      }
   }

   void setReceiverMixerAgc(int value)
   {
      if (receiverMixerAgc != value)
      {
         qInfo() << "receiver mixer AGC changed:" << value;

         receiverMixerAgc = value;

         ui->mixerAgc->setEnabled(receiverMixerAgc >= 0);
         ui->mixerAgc->setChecked(receiverMixerAgc == 1);

         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverConfig, {{"mixerAgc", receiverMixerAgc}}));
      }
   }

   void setReceiverSampleCount(long value)
   {
      if (receiverSampleCount != value)
      {
         receiverSampleCount = value;

         updateHeader();
      }
   }

   void setSignalPower(float value)
   {
      ui->signalStrength->setValue(value * 100);
   }

   void setLiveEnabled(bool value)
   {
      liveEnabled = value;

      ui->streamView->setVisible(liveEnabled);
      ui->actionLive->setChecked(liveEnabled);
   }

   void setFollowEnabled(bool value)
   {
      followEnabled = value;

      ui->actionFollow->setChecked(followEnabled);
   }

   void setFilterEnabled(bool value)
   {
      filterEnabled = value;

      ui->actionFilter->setChecked(filterEnabled);
   }

   void toggleListen()
   {
      clearView();

      ui->listenButton->setEnabled(false);
      ui->recordButton->setEnabled(false);

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverDecode));
   }

   void toggleRecord()
   {
      clearView();

      ui->listenButton->setEnabled(false);
      ui->recordButton->setEnabled(false);

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverRecord));
   }

   void toggleStop()
   {
      ui->stopButton->setEnabled(false);

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::StopDecode));
   }

   void toggleLive()
   {
      setLiveEnabled(!liveEnabled);
   }

   void toggleFollow()
   {
      setFollowEnabled(!followEnabled);
   }

   void toggleFilter()
   {
      setFilterEnabled(!filterEnabled);
   }

   void clearView()
   {
      clearModel();
      clearGraph();
   }

   void clearModel()
   {
      streamModel->resetModel();
   }

   void clearGraph()
   {
      ui->framesView->clear();
      ui->signalView->clear();
   }

   void refreshView()
   {
      if (streamModel->canFetchMore())
      {
         streamModel->fetchMore();

         if (followEnabled)
         {
            ui->streamView->scrollToBottom();
         }
      }
   }

   void updateHeader()
   {
      if (receiverType == "airspy")
      {
         QString info = QString("Airspy, %1MHz %2Msp (%3MB)").arg(receiverFrequency / 1E6, -1, 'f', 2).arg(receiverSampleRate / 1E6, -1, 'f', 2).arg(receiverSampleCount >> 19);

         ui->headerLabel->setText(info);
      }
   }

   void streamScrollChanged()
   {
      QModelIndex firstRow = ui->streamView->indexAt(ui->streamView->verticalScrollBar()->rect().topLeft());
      QModelIndex lastRow = ui->streamView->indexAt(ui->streamView->verticalScrollBar()->rect().bottomLeft() - QPoint(0, 10));

      if (firstRow.isValid() && lastRow.isValid())
      {
         nfc::NfcFrame *firstFrame = streamModel->frame(firstRow);
         nfc::NfcFrame *lastFrame = streamModel->frame(lastRow);

         if (firstFrame && lastFrame)
         {
            ui->signalView->range(firstFrame->timeStart(), lastFrame->timeEnd());
         }
      }
   }

   void streamSelectionChanged()
   {
      QModelIndexList indexList = ui->streamView->selectionModel()->selectedIndexes();

      if (!indexList.isEmpty())
      {
         QString text;

         double startTime = -1;
         double endTime = -1;

         QModelIndex previous;

         for (const QModelIndex &current: indexList)
         {
            if (!previous.isValid() || current.row() != previous.row())
            {
               if (nfc::NfcFrame *frame = streamModel->frame(current))
               {
                  text.append(QString("%1;").arg(current.row()));

                  for (int i = 0; i < frame->available(); i++)
                  {
                     text.append(QString("%1 ").arg((*frame)[i], 2, 16, QLatin1Char('0')));
                  }

                  text.append(QLatin1Char('\n'));

                  // detect data start / end timing
                  if (startTime < 0 || frame->timeStart() < startTime)
                     startTime = frame->timeStart();

                  if (endTime < 0 || frame->timeEnd() > endTime)
                     endTime = frame->timeEnd();
               }
            }

            previous = current;
         }

         // copy data to clipboard buffer
         clipboard = text.toUpper();

         // select first request-response
         parserModel->resetModel();

         auto firstIndex = indexList.first();

         if (auto firstFrame = streamModel->frame(firstIndex))
         {
            if (firstFrame->isPollFrame())
            {
               parserModel->append(*firstFrame);

               auto secondIndex = streamModel->index(firstIndex.row() + 1, 0);

               if (secondIndex.isValid())
               {
                  if (auto secondFrame = streamModel->frame(secondIndex))
                  {
                     if (secondFrame->isListenFrame())
                     {
                        parserModel->append(*secondFrame);
                     }
                  }
               }
            }
            else if (firstFrame->isListenFrame())
            {
               auto secondIndex = streamModel->index(firstIndex.row() - 1, 0);

               if (secondIndex.isValid())
               {
                  if (auto secondFrame = streamModel->frame(secondIndex))
                  {
                     if (secondFrame->isPollFrame())
                     {
                        parserModel->append(*secondFrame);
                        parserModel->append(*firstFrame);
                     }
                  }
               }
            }
         }

         // expand protocol information
         ui->parserView->expandAll();

         // select frames un timing view
         ui->framesView->blockSignals(true);
         ui->framesView->select(startTime, endTime);
         ui->framesView->blockSignals(false);

         // select frames un timing view
         ui->signalView->blockSignals(true);
         ui->signalView->select(startTime, endTime);
         ui->signalView->blockSignals(false);
      }
   }

   void timingSelectionChanged(double from, double to)
   {
      QModelIndexList selectionList = streamModel->modelRange(from, to);

      if (!selectionList.isEmpty())
      {
         QItemSelection selection(selectionList.first(), selectionList.last());

         ui->streamView->selectionModel()->blockSignals(true);
         ui->streamView->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
         ui->streamView->selectionModel()->blockSignals(false);
      }

      ui->signalView->blockSignals(true);
      ui->signalView->select(from, to);
      ui->signalView->blockSignals(false);
   }

   void signalSelectionChanged(double from, double to)
   {
      QModelIndexList selectionList = streamModel->modelRange(from, to);

      if (!selectionList.isEmpty())
      {
         QItemSelection selection(selectionList.first(), selectionList.last());

         ui->streamView->selectionModel()->blockSignals(true);
         ui->streamView->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
         ui->streamView->selectionModel()->blockSignals(false);
      }

      ui->framesView->blockSignals(true);
      ui->framesView->select(from, to);
      ui->framesView->blockSignals(false);
   }

   void signalRangeChanged(float from, float to)
   {
      float span = to - from;
      float center = from + span / 2;

      qDebug() << "signalRangeChanged(" << from << "," << to << ") span" << span << "center" << center;
      qDebug() << "signalScroll->setValue(" << qRound(center * 1000) << ")";
      qDebug() << "signalScroll->setPageStep(" << qRound(2 * span * 1000) << ")";

      ui->signalScroll->setValue(qRound(center * 1000));
      ui->signalScroll->setPageStep(qRound(2 * span * 1000));
   }

   void clipboardCopy() const
   {
      QApplication::clipboard()->setText(clipboard);
   }
};

QtWindow::QtWindow(QSettings &settings, QtMemory *cache) : impl(new Impl(settings, cache))
{
   impl->setupUi(this);

   // restore interface preferences
   impl->setLiveEnabled(settings.value("window/liveEnabled", false).toBool());
   impl->setFollowEnabled(settings.value("window/followEnabled", true).toBool());
   impl->setFilterEnabled(settings.value("window/filterEnabled", true).toBool());

   // update window size
   setMinimumSize(settings.value("window/defaultWidth", 1024).toInt(), settings.value("window/defaultHeight", 720).toInt());

   // configure window properties
   setAttribute(Qt::WA_OpaquePaintEvent, true);
   setAttribute(Qt::WA_PaintOnScreen, true);
   setAttribute(Qt::WA_DontCreateNativeAncestors, true);
   setAttribute(Qt::WA_NativeWindow, true);
   setAttribute(Qt::WA_NoSystemBackground, true);
   setAttribute(Qt::WA_MSWindowsUseDirect3D, true);
   setAutoFillBackground(false);

   // and show!
   showNormal();
}

void QtWindow::clearView()
{
   impl->clearView();
}

void QtWindow::openFile()
{
   QString fileName = QFileDialog::getOpenFileName(this, tr("Open capture file"), "", tr("Capture (*.wav *.xml *.json);;All Files (*)"));

   if (!fileName.isEmpty())
   {
      QFile file(fileName);

      if (!file.open(QIODevice::ReadOnly))
      {
         QMessageBox::information(this, tr("Unable to open file"), file.errorString());

         return;
      }

      clearView();

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReadFile, "file", fileName));
   }
}

void QtWindow::saveFile()
{
   QString date = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
   QString name = QString("record-%2.json").arg(date);

   QString fileName = QFileDialog::getSaveFileName(this, tr("Save record file"), name, tr("Capture (*.xml *.json);;All Files (*)"));

   if (!fileName.isEmpty())
   {
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::WriteFile, "file", fileName));
   }
}

void QtWindow::toggleListen()
{
   impl->toggleListen();
}

void QtWindow::toggleRecord()
{
   impl->toggleRecord();
}

void QtWindow::toggleStop()
{
   impl->toggleStop();
}

void QtWindow::toggleLive()
{
   impl->toggleLive();
}

void QtWindow::toggleFollow()
{
   impl->toggleFollow();
}

void QtWindow::toggleFilter()
{
   impl->toggleFilter();
}

void QtWindow::toggleTunerAgc(bool value)
{
   impl->setReceiverTunerAgc(value);
}

void QtWindow::toggleMixerAgc(bool value)
{
   impl->setReceiverMixerAgc(value);
}

void QtWindow::changeGainMode(int index)
{
   impl->setReceiverGainMode(impl->ui->gainMode->itemData(index).toInt());
}

void QtWindow::changeGainValue(int value)
{
   impl->setReceiverGainValue(value);
}

void QtWindow::keyPressEvent(QKeyEvent *event)
{
   if (event->key() == Qt::Key_C && (event->modifiers() & Qt::ControlModifier))
      impl->clipboardCopy();
   else
      QMainWindow::keyPressEvent(event);
}

void QtWindow::handleEvent(QEvent *event)
{
   if (event->type() == SignalBufferEvent::Type)
      impl->signalBufferEvent(dynamic_cast<SignalBufferEvent *>(event));
   else if (event->type() == StreamFrameEvent::Type)
      impl->streamFrameEvent(dynamic_cast<StreamFrameEvent *>(event));
   else if (event->type() == DecoderStatusEvent::Type)
      impl->decoderStatusEvent(dynamic_cast<DecoderStatusEvent *>(event));
   else if (event->type() == ReceiverStatusEvent::Type)
      impl->receiverStatusEvent(dynamic_cast<ReceiverStatusEvent *>(event));
   else if (event->type() == StorageStatusEvent::Type)
      impl->storageStatusEvent(dynamic_cast<StorageStatusEvent *>(event));
   else if (event->type() == ConsoleLogEvent::Type)
      impl->consoleLogEvent(dynamic_cast<ConsoleLogEvent *>(event));
   else if (event->type() == SystemStartupEvent::Type)
      impl->systemStartup(dynamic_cast<SystemStartupEvent *>(event));
   else if (event->type() == SystemShutdownEvent::Type)
      impl->systemShutdown(dynamic_cast<SystemShutdownEvent *>(event));
}


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

#include "QtConfig.h"
#include "QtMemory.h"
#include "QtWindow.h"

struct QtWindow::Impl
{
   // configuration
   QSettings &settings;

   // signal memory cache
   QtMemory *cache;

   // Toolbar status
   bool recordEnabled = false;
   bool followEnabled = false;
   bool filterEnabled = false;

   // receiver parameters
   QStringList deviceList;
   QList<int> deviceGainList;
   QMap<int, QString> deviceGainValues;
   QMap<int, QString> deviceGainModes;

   // current device parameters
   QString deviceName;
   QString deviceType;
   QString deviceStatus;
   int deviceFrequency = 0;
   int deviceSampleRate = 0;
   int deviceSampleCount = 0;
   int deviceGainMode = -1;
   int deviceGainValue = -1;

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
      frequencySubscription = frequencyStream->subscribe([=](const sdr::SignalBuffer &buffer) {
         ui->frequencyView->refresh(buffer);
      });
   }

   void setupUi(QtWindow *mainWindow)
   {
      ui->setupUi(mainWindow);

      // update window caption
      mainWindow->setWindowTitle(NFC_LAB_VENDOR_STRING);

      // setup default controls status
      ui->gainMode->setEnabled(false);
      ui->gainValue->setEnabled(false);

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
      QObject::connect(ui->framesView, &FramesWidget::selectionChanged, [=](float from, float to) {
         timingSelectionChanged(from, to);
      });

      // connect selection signal from timing graph
      QObject::connect(ui->signalView, &SignalWidget::selectionChanged, [=](float from, float to) {
         signalSelectionChanged(from, to);
      });

      // connect selection signal from frame model
      QObject::connect(ui->signalView, &SignalWidget::rangeChanged, [=](float from, float to) {
         signalRangeChanged(from, to);
      });

      // connect selection signal from frame model
      QObject::connect(ui->signalScroll, &QScrollBar::valueChanged, [=](int value) {
         signalScrollChanged(value);
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

         QJsonObject data = event->content();

         if (data.contains("nfca"))
         {
            QJsonObject nfca = data["nfca"].toObject();

            ui->actionNfcA->setChecked(nfca["enabled"].toBool());
         }

         if (data.contains("nfcb"))
         {
            QJsonObject nfcb = data["nfcb"].toObject();

            ui->actionNfcB->setChecked(nfcb["enabled"].toBool());
         }

         if (data.contains("nfcf"))
         {
            QJsonObject nfcf = data["nfcf"].toObject();

            ui->actionNfcF->setChecked(nfcf["enabled"].toBool());
         }

         if (data.contains("nfcv"))
         {
            QJsonObject nfcv = data["nfcv"].toObject();

            ui->actionNfcV->setChecked(nfcv["enabled"].toBool());
         }
      }
   }

   void receiverStatusEvent(ReceiverStatusEvent *event)
   {
      if (event->hasGainModeList())
         updateGainModes(event->gainModeList());

      if (event->hasGainValueList())
         updateGainValues(event->gainValueList());

      if (event->hasReceiverName())
         updateDeviceName(event->source());

      if (event->hasReceiverStatus())
         updateDeviceStatus(event->status());

      if (event->hasSignalPower())
         updateSignalPower(event->signalPower());

      if (event->hasSampleCount())
         updateSampleCount(event->sampleCount());
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

   void updateDeviceName(const QString &value)
   {
      if (deviceName != value)
      {
         qInfo() << "receiver device changed:" << value;

         deviceName = value;

         if (!deviceName.isEmpty())
         {
            deviceType = deviceName.left(deviceName.indexOf("://"));

            ui->statusBar->showMessage(deviceName);

            updateFrequency(settings.value("device." + deviceType + "/centerFreq", "13560000").toInt());
            updateSampleRate(settings.value("device." + deviceType + "/sampleRate", "10000000").toInt());
            updateGainMode(settings.value("device." + deviceType + "/gainMode", "1").toInt());
            updateGainValue(settings.value("device." + deviceType + "/gainValue", "6").toInt());

            ui->eventsLog->append(QString("Detected device %1").arg(deviceName));
         }

         updateHeader();
      }
   }

   void updateDeviceStatus(const QString &value)
   {
      if (deviceStatus != value)
      {
         qInfo() << "receiver status changed:" << value;

         deviceStatus = value;

         if (deviceStatus == ReceiverStatusEvent::NoDevice)
         {
            ui->listenButton->setEnabled(false);
            ui->recordButton->setEnabled(false);
            ui->stopButton->setEnabled(false);

            ui->gainMode->setEnabled(false);
            ui->gainValue->setEnabled(false);

            ui->statusBar->showMessage("No device found");
         }

         else if (deviceStatus == ReceiverStatusEvent::Idle)
         {
            ui->listenButton->setEnabled(true);
            ui->recordButton->setEnabled(true);
            ui->stopButton->setEnabled(false);

            ui->gainMode->setEnabled(true);
            ui->gainValue->setEnabled(true);

            updateGainMode(deviceGainMode);
            updateGainValue(deviceGainValue);
         }

         else if (deviceStatus == ReceiverStatusEvent::Streaming)
         {
            ui->listenButton->setEnabled(false);
            ui->recordButton->setEnabled(false);
            ui->stopButton->setEnabled(true);

            updateGainMode(deviceGainMode);
            updateGainValue(deviceGainValue);
         }
      }
   }

   void updateGainModes(const QMap<int, QString> &value)
   {
      if (deviceGainModes != value)
      {
         qInfo() << "receiver gains modes changed:" << value;

         deviceGainModes = value;

         ui->gainMode->blockSignals(true);
         ui->gainMode->clear();

         for (auto const &mode: deviceGainModes.keys())
            ui->gainMode->addItem(deviceGainModes[mode], mode);

         ui->gainMode->setCurrentIndex(ui->gainMode->findData(deviceGainMode));
         ui->gainMode->blockSignals(false);
      }
   }

   void updateGainValues(const QMap<int, QString> &value)
   {
      if (deviceGainValues != value)
      {
         qInfo() << "receiver gains values changed:" << value;

         deviceGainValues = value;
         deviceGainList = deviceGainValues.keys();

         if (!deviceGainList.isEmpty())
         {
            ui->gainValue->setRange(0, deviceGainList.count());
            ui->gainValue->setValue(deviceGainList.indexOf(deviceGainValue));
         }
         else
         {
            ui->gainValue->setRange(0, 0);
         }
      }
   }

   void updateFrequency(long value)
   {
      if (deviceFrequency != value)
      {
         qInfo() << "receiver frequency changed:" << value;

         deviceFrequency = value;

         ui->frequencyView->setCenterFreq(deviceFrequency);

         if (!deviceType.isEmpty())
            settings.setValue("device." + deviceType + "/centerFreq", deviceFrequency);

         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverConfig, {
               {"centerFreq", deviceFrequency}
         }));

         updateHeader();
      }
   }

   void updateSampleRate(long value)
   {
      if (deviceSampleRate != value)
      {
         qInfo() << "receiver samplerate changed:" << value;

         deviceSampleRate = value;

         ui->frequencyView->setSampleRate(deviceSampleRate);

         if (!deviceType.isEmpty())
            settings.setValue("device." + deviceType + "/sampleRate", deviceSampleRate);

         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverConfig, {{"sampleRate", deviceSampleRate}}));

         updateHeader();
      }
   }

   void updateGainMode(int value)
   {
      if (deviceGainMode != value)
      {
         deviceGainMode = value;

         qInfo() << "receiver gain mode changed:" << value;

         if (!deviceType.isEmpty())
            settings.setValue("device." + deviceType + "/gainMode", deviceGainMode);

         if (ui->gainMode->count() > 0)
         {
            if (deviceGainMode)
            {
               ui->gainValue->setValue(deviceGainList.indexOf(deviceGainValue));
               ui->gainLabel->setText(QString("Gain %1").arg(deviceGainValues[deviceGainValue]));
            }
            else
            {
               ui->gainValue->setValue(0);
               ui->gainLabel->setText(QString("Gain AUTO"));
            }

            ui->gainMode->setCurrentIndex(ui->gainMode->findData(deviceGainMode));

            QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverConfig, {
                  {"gainMode",  deviceGainMode},
                  {"gainValue", deviceGainValue}
            }));
         }
      }

//      if (ui->gainMode->count() > 0)
//      {
//         ui->gainMode->setEnabled(true);
//         ui->gainValue->setEnabled(receiverGainMode != 0);
//      }
   }

   void updateGainValue(int value)
   {
      if (deviceGainValue != value)
      {
         deviceGainValue = value;

         if (deviceGainMode != 0)
         {
            qInfo() << "receiver gain value changed:" << value;

            ui->gainValue->setValue(deviceGainList.indexOf(deviceGainValue));
            ui->gainLabel->setText(QString("Gain %1").arg(deviceGainValues[deviceGainValue]));

            if (!deviceType.isEmpty())
               settings.setValue("device." + deviceType + "/gainValue", deviceGainValue);

            QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverConfig, {
                  {"gainMode",  deviceGainMode},
                  {"gainValue", deviceGainValue}
            }));
         }
      }
   }

   void updateSampleCount(long value)
   {
      if (deviceSampleCount != value)
      {
         deviceSampleCount = value;

         updateHeader();
      }
   }

   void updateSignalPower(float value)
   {
      ui->signalStrength->setValue(value * 100);
   }

   void setFollowEnabled(bool value)
   {
      followEnabled = value;

      ui->actionFollow->setChecked(followEnabled);

      settings.setValue("window/followEnabled", followEnabled);
   }

   void setFilterEnabled(bool value)
   {
      filterEnabled = value;

      ui->actionFilter->setChecked(filterEnabled);

      settings.setValue("window/filterEnabled", filterEnabled);
   }

   void setNfcAEnabled(bool value) const
   {
      ui->actionNfcA->setChecked(value);

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::DecoderConfig, {
            {"nfca/enabled", ui->actionNfcA->isChecked()}
      }));
   }

   void setNfcBEnabled(bool value) const
   {
      ui->actionNfcB->setChecked(value);

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::DecoderConfig, {
            {"nfcb/enabled", ui->actionNfcB->isChecked()}
      }));
   }

   void setNfcFEnabled(bool value) const
   {
      ui->actionNfcF->setChecked(value);

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::DecoderConfig, {
            {"nfcf/enabled", ui->actionNfcF->isChecked()}
      }));
   }

   void setNfcVEnabled(bool value) const
   {
      ui->actionNfcV->setChecked(value);

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::DecoderConfig, {
            {"nfcv/enabled", ui->actionNfcV->isChecked()}
      }));
   }

   void trackGainValue(int index)
   {
      int value = deviceGainList[index];

      qInfo() << "receiver gain value changed:" << value;

      ui->gainLabel->setText(QString("Gain %1").arg(deviceGainValues[value]));
   }

   void toggleListen()
   {
      clearView();

      ui->listenButton->setEnabled(false);
      ui->recordButton->setEnabled(false);
      ui->statusTabs->setCurrentWidget(ui->receiverTab);

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverDecode));
   }

   void toggleRecord()
   {
      clearView();

      ui->listenButton->setEnabled(false);
      ui->recordButton->setEnabled(false);

      QString fileName = QString("record-%1.wav").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReceiverRecord, {
            {"fileName",   fileName},
            {"sampleRate", deviceSampleRate}
      }));
   }

   void toggleStop()
   {
      ui->stopButton->setEnabled(false);

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::StopDecode));
   }

   void toggleFollow()
   {
      setFollowEnabled(ui->actionFollow->isChecked());
   }

   void toggleFilter()
   {
      setFilterEnabled(ui->actionFilter->isChecked());
   }

   void toggleNfcA()
   {
      setNfcAEnabled(ui->actionNfcA->isChecked());
   }

   void toggleNfcB()
   {
      setNfcBEnabled(ui->actionNfcB->isChecked());
   }

   void toggleNfcF()
   {
      setNfcFEnabled(ui->actionNfcF->isChecked());
   }

   void toggleNfcV()
   {
      setNfcVEnabled(ui->actionNfcV->isChecked());
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
      if (deviceType == "airspy")
      {
         QString info = QString("Airspy, %1MHz %2Msp (%3MB)").arg(deviceFrequency / 1E6, -1, 'f', 2).arg(deviceSampleRate / 1E6, -1, 'f', 2).arg(deviceSampleCount >> 19);

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
            ui->signalView->setRange(firstFrame->timeStart(), lastFrame->timeEnd());
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
      float range = to - from;
      float length = ui->signalView->maximumRange() - ui->signalView->minimumRange();
      float value = (from - ui->signalView->minimumRange()) / length;
      float step = (range / length);

      ui->signalScroll->blockSignals(true);
      ui->signalScroll->setPageStep(qRound(step * 1000));
      ui->signalScroll->setMaximum(1000 - ui->signalScroll->pageStep());
      ui->signalScroll->setValue(qRound(value * 1000));
      ui->signalScroll->blockSignals(false);
   }

   void signalScrollChanged(int value)
   {
      float length = ui->signalView->maximumRange() - ui->signalView->minimumRange();
      float from = ui->signalView->minimumRange() + length * (value / 1000.0f);
      float to = from + length * (ui->signalScroll->pageStep() / 1000.0f);

      ui->signalView->blockSignals(true);
      ui->signalView->setRange(from, to);
      ui->signalView->blockSignals(false);
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

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReadFile, {
            {"fileName", fileName}
      }));
   }
}

void QtWindow::saveFile()
{
   QString date = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
   QString name = QString("record-%2.json").arg(date);

   QString fileName = QFileDialog::getSaveFileName(this, tr("Save record file"), name, tr("Capture (*.xml *.json);;All Files (*)"));

   if (!fileName.isEmpty())
   {
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::WriteFile, {
            {"fileName",   fileName},
            {"sampleRate", impl->deviceSampleRate}
      }));
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

void QtWindow::toggleFollow()
{
   impl->toggleFollow();
}

void QtWindow::toggleFilter()
{
   impl->toggleFilter();
}

void QtWindow::toggleNfcA()
{
   impl->toggleNfcA();
}

void QtWindow::toggleNfcB()
{
   impl->toggleNfcB();
}

void QtWindow::toggleNfcF()
{
   impl->toggleNfcF();
}

void QtWindow::toggleNfcV()
{
   impl->toggleNfcV();
}

void QtWindow::changeGainMode(int index)
{
   impl->updateGainMode(impl->ui->gainMode->itemData(index).toInt());
}

void QtWindow::changeGainValue(int index)
{
   impl->updateGainValue(impl->deviceGainList[index]);
}

void QtWindow::trackGainValue(int index)
{
   impl->trackGainValue(index);
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


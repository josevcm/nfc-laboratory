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

#include <QDebug>
#include <QKeyEvent>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QTimer>

#include <events/DecoderControlEvent.h>
#include <events/StreamStatusEvent.h>
#include <events/StorageControlEvent.h>
#include <events/GainControlEvent.h>
#include <events/ConsoleLogEvent.h>
#include <events/StreamFrameEvent.h>

#include <model/FrameModel.h>

#include <protocol/ProtocolFrame.h>

#include <support/ElapsedLogger.h>

#include <Dispatcher.h>

#include "MainWindow.h"
#include "PlotMarker.h"
#include "PlotView.h"

#include "ui_MainWindow.h"

#include "SetupDialog.h"

// https://iconos8.es/icon/2306/logo-de-nfc

enum SelectMode
{
   None = 0, PlotSelection = 1, ViewSelection = 2
};

MainWindow::MainWindow(QSettings &settings, NfcStream *stream) :
   ui(new Ui::MainWindow),
   mSettings(settings),
   mStream(stream),
   mDecoderStatus(-1),
   mFrequency(0),
   mSampleRate(0),
   mSampleCount(0),
   mSelectionMode(0),
   mLiveEnabled(false),
   mRecordEnabled(false),
   mFollowEnabled(false),
   mFilterEnabled(false),
   mRefreshTimer(new QTimer(this)),
   mFrameModel(new FrameModel(stream, this)),
   mLowerSignalRange(INFINITY),
   mUpperSignalRange(0)
{
   // setup user interface
   setupUi();

   // restore interface preferences
   setLiveEnabled(mSettings.value("window/liveEnabled", false).toBool());
   setFollowEnabled(mSettings.value("window/followEnabled", true).toBool());
   setFilterEnabled(mSettings.value("window/filterEnabled", true).toBool());

   // connect selection model signal
   QObject::connect(ui->liveView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::viewSelectionChanged);
   //   QObject::connect(ui->signalPlot, &PlotView::selectionChanged, this, &MainWindow::plotSelectionChanged);

   // connect refresh timer signal
   QObject::connect(mRefreshTimer, &QTimer::timeout, this, &MainWindow::refreshView);

   // and start it!
   mRefreshTimer->start(50);

   // finally, set minimun window size
   setMinimumSize(mSettings.value("window/defaultWidth", 1024).toInt(), mSettings.value("window/defaultHeight", 640).toInt());

   //setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

   // and go on!!!
   show();
}

MainWindow::~MainWindow()
{
   delete ui;
}

void MainWindow::setupUi()
{
   ui->setupUi(this);

   // setup display stretch
   ui->workbench->setStretchFactor(0, 2);
   ui->workbench->setStretchFactor(1, 1);

   // setup frame view model
   ui->liveView->setModel(mFrameModel);
   ui->liveView->setColumnWidth(FrameModel::Id, 75);
   ui->liveView->setColumnWidth(FrameModel::Time, 100);
   ui->liveView->setColumnWidth(FrameModel::Rate, 60);
   ui->liveView->setColumnWidth(FrameModel::Type, 100);
   ui->liveView->header()->setSectionResizeMode(FrameModel::Data, QHeaderView::ResizeToContents);

   // setup signal plot
   ui->signalPlot->setBackground(Qt::NoBrush);
   ui->signalPlot->setInteraction(QCP::iRangeDrag, true); // enable graph drag
   ui->signalPlot->setInteraction(QCP::iRangeZoom, true); // enable graph zoom
   ui->signalPlot->setInteraction(QCP::iSelectPlottables, true); // enable graph selection
   ui->signalPlot->setInteraction(QCP::iMultiSelect, true); // enable graph multiple selection
   ui->signalPlot->axisRect()->setRangeDrag(Qt::Horizontal); // only drag horizontal axis
   ui->signalPlot->axisRect()->setRangeZoom(Qt::Horizontal); // only zoom horizontal axis

   // setup time axis
   ui->signalPlot->xAxis->setBasePen(QPen(Qt::white, 1));
   ui->signalPlot->xAxis->setTickPen(QPen(Qt::white, 1));
   ui->signalPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
   ui->signalPlot->xAxis->setTickLabelColor(Qt::white);
   ui->signalPlot->xAxis->setRange(0, 1);

   // setup frame types for Y-axis
   QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
   textTicker->addTick(1, "REQ");
   textTicker->addTick(2, "SEL");
   textTicker->addTick(3, "INF");

   ui->signalPlot->yAxis->setBasePen(QPen(Qt::white, 1));
   ui->signalPlot->yAxis->setTickPen(QPen(Qt::white, 1));
   ui->signalPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
   ui->signalPlot->yAxis->setTickLabelColor(Qt::white);
   ui->signalPlot->yAxis->setTicker(textTicker);
   ui->signalPlot->yAxis->setRange(0, 4);

   // graph for RF status
   QCPGraph *graphRf = ui->signalPlot->addGraph();
   graphRf->setPen(QPen(Qt::cyan));
   graphRf->setBrush(QBrush(QColor(0, 0, 255, 20)));
   graphRf->setSelectable(QCP::stDataRange);
   graphRf->selectionDecorator()->setBrush(graphRf->brush());

   // graph for sense request phase (REQ / WUPA)
   QCPGraph *graphReq = ui->signalPlot->addGraph();
   graphReq->setPen(QPen(Qt::green));
   graphReq->setBrush(QBrush(QColor(0, 255, 0, 20)));
   graphReq->setSelectable(QCP::stDataRange);
   graphReq->selectionDecorator()->setBrush(graphReq->brush());

   // graph for selection and anti-collision phase (SELx / PPS / ATS)
   QCPGraph *graphSel = ui->signalPlot->addGraph();
   graphSel->setPen(QPen(Qt::red));
   graphSel->setBrush(QBrush(QColor(255, 0, 0, 20)));
   graphSel->setSelectable(QCP::stDataRange);
   graphSel->selectionDecorator()->setBrush(graphSel->brush());

   // graph for information frames phase (other types)
   QCPGraph *graphInf = ui->signalPlot->addGraph();
   graphInf->setPen(QPen(Qt::gray));
   graphInf->setBrush(QBrush(QColor(255, 255, 255, 20)));
   graphInf->setSelectable(QCP::stDataRange);
   graphInf->selectionDecorator()->setBrush(graphInf->brush());

   // setup time measure and graph tracer
   mPlotMarker = new PlotMarker(graphInf->keyAxis());
   mPlotMarker->setPen(QPen(Qt::gray));
   mPlotMarker->setBrush(QBrush(Qt::white));

   // connect graph signals
   QObject::connect(ui->signalPlot, SIGNAL(selectionChangedByUser()), this, SLOT(plotSelectionChanged()));
   QObject::connect(ui->signalPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(plotMousePress(QMouseEvent*)));
   QObject::connect(ui->signalPlot->xAxis, SIGNAL(rangeChanged(const QCPRange&)), this, SLOT(plotRangeChanged(const QCPRange&)));

   // initialize
   clearGraph();
}

void MainWindow::customEvent(QEvent * event)
{
   if (event->type() == StreamStatusEvent::Type)
      streamStatusEvent(static_cast<StreamStatusEvent*>(event));
   else if (event->type() == StreamFrameEvent::Type)
      streamFrameEvent(static_cast<StreamFrameEvent*>(event));
   else if (event->type() == ConsoleLogEvent::Type)
      consoleLogEvent(static_cast<ConsoleLogEvent*>(event));
}

void MainWindow::streamStatusEvent(StreamStatusEvent *event)
{
   if (event->hasSource())
      setSourceDevice(event->source());

   if (event->hasFrequency())
      setFrequency(event->frequency());

   if (event->hasSampleRate())
      setSampleRate(event->sampleRate());

   if (event->hasSampleCount())
      setSampleCount(event->sampleCount());

   if (event->hasSignalPower())
      setSignalPower(event->signalPower());

   if (event->hasStreamProgress())
      setStreamProgress(event->streamProgress());

   if (event->hasSourceList())
      setDeviceList(event->sourceList());

   if (event->hasFrequencyList())
      setFrequencyList(event->frequencyList());

   if (event->hasTunerGainList())
      setTunerGainList(event->tunerGainList());

   if (event->hasStatus())
      setDecoderStatus(event->status());

   if (event->hasTunerGain() || event->hasFrequency() || event->hasSampleRate())
   {
      QString info = "Set tuner";

      float frequency = event->frequency();
      float sampleRate = event->sampleRate();
      float tunerGain = event->tunerGain();

      if (event->hasFrequency())
         info += QString(" frequency %1 MHz").arg(frequency / 1E6, 5, 'f', 2);

      if (event->hasSampleRate())
         info += QString(" sampling %1 Mbps").arg(sampleRate / 1E6, 5, 'f', 2);

      if (event->hasTunerGain())
      {
         info += QString(" tuner gain %1 dbs").arg(tunerGain, 5, 'f', 2);

         ui->gainDial->setValue(tunerGain);
         ui->groupBox->setTitle(QString("Gain %1 db").arg(tunerGain, 5, 'f', 2));
      }

      ui->eventsLog->append(info);
   }
}

void MainWindow::consoleLogEvent(ConsoleLogEvent *event)
{
   const QStringList &messages = event->messages();

   for (QStringList::ConstIterator it = messages.begin();it != messages.end(); it++)
   {
      ui->eventsLog->append(*it);
   }
}

void MainWindow::streamFrameEvent(StreamFrameEvent *event)
{
   NfcFrame frame = event->frame();

   // signal present
   if (frame.isRequestFrame() || frame.isResponseFrame() || frame.isNoFrame())
   {
      if (!frame.isNoFrame())
      {
         // update signal ranges
         if (mLowerSignalRange > frame.timeStart())
            mLowerSignalRange = frame.timeStart();

         if (mUpperSignalRange < frame.timeEnd())
            mUpperSignalRange = frame.timeEnd();

         // update view range
         ui->signalPlot->xAxis->setRange(mLowerSignalRange, mUpperSignalRange);

         int graphIndex = 0;
         int graphValue = 0;

         switch(frame.framePhase())
         {
            case NfcFrame::SenseFrame:
            {
               graphIndex = 1;
               graphValue = 1;
               break;
            }
            case NfcFrame::SelectionFrame:
            {
               graphIndex = 2;
               graphValue = 2;
               break;
            }
            case NfcFrame::InformationFrame:
            {
               graphIndex = 3;
               graphValue = 3;
               break;
            }
         }

         if (graphIndex > 0)
         {
            // move previous marker
            QCPGraphDataContainer::iterator it = ui->signalPlot->graph(graphIndex)->data()->end() - 1;

            it->key = frame.timeStart();

            // add request / response pulse
            if (frame.isResponseFrame())
            {
               ui->signalPlot->graph(graphIndex)->addData(frame.timeStart(), graphValue + 0.15);
               ui->signalPlot->graph(graphIndex)->addData(frame.timeEnd(), graphValue + 0.15);
            }
            else
            {
               ui->signalPlot->graph(graphIndex)->addData(frame.timeStart(), graphValue + 0.5);
               ui->signalPlot->graph(graphIndex)->addData(frame.timeEnd(), graphValue + 0.5);
            }

            // add pulse end (return to 0)
            ui->signalPlot->graph(graphIndex)->addData(frame.timeEnd(), graphValue);

            // add end marker
            ui->signalPlot->graph(graphIndex)->addData(frame.timeEnd(), graphValue);
         }

         // add padding moving end markers
         for (int i = 1; i < ui->signalPlot->graphCount(); i++)
         {
            if (i != graphIndex)
            {
               QCPGraphDataContainer::iterator it = ui->signalPlot->graph(i)->data()->end() - 1;

               if (it != ui->signalPlot->graph(i)->data()->begin())
               {
                  it->key = frame.timeEnd();
               }
            }
         }
      }
   }

   // signal not present
   else
   {
      for (int i = 0; i < ui->signalPlot->graphCount(); i++)
      {
         if (ui->signalPlot->graph(i)->data()->isEmpty())
         {
            ui->signalPlot->graph(i)->addData(frame.timeStart(), 0);
            ui->signalPlot->graph(i)->addData(frame.timeEnd(), 0);
         }
         else
         {
            QCPGraphDataContainer::iterator it = ui->signalPlot->graph(i)->data()->end() - 1;

            it->key = frame.timeEnd();
         }
      }
   }

   ui->signalPlot->replot();
}

void MainWindow::setDecoderStatus(int decoderStatus)
{
   if (mDecoderStatus != decoderStatus)
   {
      mDecoderStatus = decoderStatus;

      switch (mDecoderStatus)
      {
         case StreamStatusEvent::Stopped:
         {
            ui->listenButton->setEnabled(!mDeviceList.isEmpty());
            ui->recordButton->setEnabled(!mDeviceList.isEmpty());
            ui->stopButton->setEnabled(false);

            ui->gainDial->setEnabled(false);
            ui->eventsLog->append("Decoder stopped");

            break;
         }

         case StreamStatusEvent::Streaming:
         {
            ui->listenButton->setEnabled(false);
            ui->recordButton->setEnabled(false);
            ui->stopButton->setEnabled(true);

            ui->gainDial->setEnabled(true);
            ui->eventsLog->append("Decoder started");

            mLowerSignalRange = INFINITY;
            mUpperSignalRange = 0;

            setLiveEnabled(true);

            break;
         }

         case StreamStatusEvent::Recording:
         {
            ui->listenButton->setEnabled(false);
            ui->recordButton->setEnabled(false);
            ui->stopButton->setEnabled(true);

            ui->gainDial->setEnabled(true);
            ui->eventsLog->append("Capture started");

            mLowerSignalRange = INFINITY;
            mUpperSignalRange = 0;

            setLiveEnabled(false);

            break;
         }
      }
   }
}

void MainWindow::setSourceDevice(const QString &sourceDevice)
{
   if (mSourceDevice != sourceDevice)
   {
      mSourceDevice = sourceDevice;

      updateHeader();
   }
}

void MainWindow::setFrequency(float frequency)
{
   if (mFrequency != frequency)
   {
      mFrequency = frequency;

      updateHeader();
   }
}

void MainWindow::setSampleRate(float sampleRate)
{
   if (mSampleRate != sampleRate)
   {
      mSampleRate = sampleRate;

      updateHeader();
   }
}

void MainWindow::setSampleCount(long sampleCount)
{
   if (mSampleCount != sampleCount)
   {
      mSampleCount = sampleCount;

      updateHeader();
   }
}

void MainWindow::setSignalPower(float signalPower)
{
   ui->signalStrength->setValue(signalPower * 100);
}

void MainWindow::setStreamProgress(float timeLimit)
{
   ui->streamProgress->setValue(timeLimit * 100);
}

void MainWindow::setLiveEnabled(bool liveEnabled)
{
   if (mLiveEnabled != liveEnabled)
   {
      mLiveEnabled = liveEnabled;

      ui->liveView->setVisible(mLiveEnabled);
      ui->streamView->setVisible(!mLiveEnabled);
      ui->actionLive->setChecked(mLiveEnabled);
   }
}

void MainWindow::setFollowEnabled(bool followEnabled)
{
   if (mFollowEnabled != followEnabled)
   {
      mFollowEnabled = followEnabled;

      ui->actionFollow->setChecked(mFollowEnabled);
   }
}

void MainWindow::setFilterEnabled(bool filterEnabled)
{
   if (mFilterEnabled != filterEnabled)
   {
      mFilterEnabled = filterEnabled;

      mFrameModel->setGroupRepeated(mFilterEnabled);

      ui->actionFilter->setChecked(mFilterEnabled);
   }
}

void MainWindow::setDeviceList(const QStringList &deviceList)
{
   if (mDeviceList != deviceList)
   {
      mDeviceList = deviceList;

      if (mDeviceList.isEmpty())
      {
         ui->statusBar->showMessage("No devices found");
      }
      else
      {
         ui->statusBar->showMessage(mDeviceList.first());

         for (QStringList::Iterator it = mDeviceList.begin(); it != mDeviceList.end(); it++)
         {
            ui->eventsLog->append(QString("Detected device %1").arg(*it));
         }
      }
   }
}

void MainWindow::setFrequencyList(const QVector<long> &frequencyList)
{
   if (mFrequencyList != frequencyList)
   {
      mFrequencyList = frequencyList;

      for (QVector<long>::Iterator it = mFrequencyList.begin(); it != mFrequencyList.end(); it++)
      {
         ui->eventsLog->append(QString("Available frequency %1").arg(*it));
      }
   }
}

void MainWindow::setTunerGainList(const QVector<float> &tunerGainList)
{
   if (mTunerGainList != tunerGainList)
   {
      mTunerGainList = tunerGainList;

      if (mTunerGainList.isEmpty())
      {
         ui->gainDial->setEnabled(false);
      }
      else
      {
         ui->gainDial->setEnabled(true);
         ui->gainDial->setMinimum(mTunerGainList.first());
         ui->gainDial->setMaximum(mTunerGainList.last());
      }
   }
}


void MainWindow::openFile()
{
   QString fileName = QFileDialog::getOpenFileName(this, tr("Open capture file"), "", tr("Capture (*.wav *.xml);;All Files (*)"));

   if (!fileName.isEmpty())
   {
      QFile file(fileName);

      if (!file.open(QIODevice::ReadOnly))
      {
         QMessageBox::information(this, tr("Unable to open file"), file.errorString());

         return;
      }

      clearView();

      Dispatcher::post(new StorageControlEvent(StorageControlEvent::Read, "file", fileName));
   }
}

void MainWindow::saveFile()
{
   QString date = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
   QString name = QString("record-%2.xml").arg(date);

   QString fileName = QFileDialog::getSaveFileName(this, tr("Save record file"), name, tr("Capture (*.xml);;All Files (*)"));

   if (!fileName.isEmpty())
   {
      Dispatcher::post(new StorageControlEvent(StorageControlEvent::Write, "file", fileName));
   }
}

void MainWindow::toggleListen()
{
   clearView();

   ui->listenButton->setEnabled(false);
   ui->recordButton->setEnabled(false);

   Dispatcher::post(new DecoderControlEvent(DecoderControlEvent::Start));
}

void MainWindow::toggleRecord()
{
   clearView();

   ui->listenButton->setEnabled(false);
   ui->recordButton->setEnabled(false);

   Dispatcher::post(new DecoderControlEvent(DecoderControlEvent::Record));
}

void MainWindow::toggleStop()
{
   ui->stopButton->setEnabled(false);

   Dispatcher::post(new DecoderControlEvent(DecoderControlEvent::Stop));
}

void MainWindow::toggleLive()
{
   setLiveEnabled(!mLiveEnabled);
}

void MainWindow::toggleFollow()
{
   setFollowEnabled(!mFollowEnabled);
}

void MainWindow::toggleFilter()
{
   setFilterEnabled(!mFilterEnabled);
}

void MainWindow::openSettings()
{
   SetupDialog *dialog = new SetupDialog(this);

   dialog->setDeviceList(mDeviceList);
   dialog->setFrequencyList(mFrequencyList);

   dialog->exec();
}

void MainWindow::gainChanged(int value)
{
   Dispatcher::post(new GainControlEvent(value));
}

void MainWindow::updateHeader()
{
   if (mSourceDevice.startsWith("airspy"))
   {
      QString info = QString("Airspy, @%1MHz %2Msp (%3Ms)").arg(mFrequency / 1E6, -1, 'f', 2).arg(mSampleRate / 1E6, -1, 'f', 2).arg(mSampleCount / 1E6, -1, 'f', 2);

      ui->headerLabel->setText(info);
   }
   else if (mSourceDevice.endsWith(".xml"))
   {
      QString info = mSourceDevice;

      ui->headerLabel->setText(info);
   }
   else if (mSourceDevice.endsWith(".wav"))
   {
      QString info = mSourceDevice;

      ui->headerLabel->setText(info);
   }
}

void MainWindow::updateRange(double start, double end)
{
   QString text;

   double elapsed = end - start;

   if (elapsed < 1E-3)
      text = QString("%1 us").arg(elapsed * 1000000, 3, 'f', 0);
   else if (elapsed < 1)
      text = QString("%1 ms").arg(elapsed * 1000, 7, 'f', 3);
   else
      text = QString("%1 s").arg(elapsed, 7, 'f', 5);

   mPlotMarker->setRange(start, end);

   if (start < end)
      mPlotMarker->setText(text);
   else
      mPlotMarker->setText(QString("%1 s").arg(start, 7, 'f', 5));

   ui->signalPlot->replot();
}

void MainWindow::viewSelectionChanged(const QItemSelection & /* selected */, const QItemSelection & /* deselected */)
{
   QModelIndexList indexList = ui->liveView->selectionModel()->selectedIndexes();

   if (!indexList.isEmpty())
   {
      QString text;

      double startTime = -1;
      double endTime = -1;

      QModelIndex current, previous;

      for (int i = 0; i < indexList.size(); i++)
      {
         current = indexList.at(i);

         if (!previous.isValid() || current.row() != previous.row())
         {
            ProtocolFrame *frame = static_cast<ProtocolFrame*>(current.internalPointer());

            // prepara data for clipboard copy&paste
            text.append(QString("%1;").arg(frame->data(ProtocolFrame::Id).toInt()));
            text.append(QString("%1;").arg(frame->data(ProtocolFrame::Time).toDouble()));
            text.append(QString("%1;").arg(frame->data(ProtocolFrame::End).toDouble()));
            text.append(QString("%1;").arg(frame->data(ProtocolFrame::Rate).toInt()));

            QByteArray value = frame->data(ProtocolFrame::Data).toByteArray();

            for (QByteArray::ConstIterator it = value.begin(); it != value.end(); it++)
            {
               text.append(QString("%1").arg(*it & 0xff, 2, 16, QLatin1Char('0')));
            }

            text.append(QLatin1Char('\n'));

            // detect data start / end timming
            if (startTime < 0 || frame->data(ProtocolFrame::Time).toDouble() < startTime)
               startTime = frame->data(ProtocolFrame::Time).toDouble();

            if (endTime < 0 || frame->data(ProtocolFrame::End).toDouble() > endTime)
               endTime = frame->data(ProtocolFrame::End).toDouble();
         }

         previous = current;
      }

      // copy data to clipboard buffer
      mClipboard = text;

      if (!mSelectionMode)
      {
         for (int i = 0; i < ui->signalPlot->graphCount(); i++)
         {
            QCPDataSelection selection;

            QCPGraph *graph = ui->signalPlot->graph(i);

            int begin = graph->findBegin(startTime, false);
            int end = graph->findEnd(endTime, false);

            selection.addDataRange(QCPDataRange(begin, end));

            graph->setSelection(selection);
         }

         updateRange(startTime, endTime);
      }
   }
}

void MainWindow::plotSelectionChanged()
{
   QList<QCPGraph*> selectedGraphs = ui->signalPlot->selectedGraphs();

   if (selectedGraphs.size() > 0)
   {
      QString text;
      double startTime = -1;
      double endTime = -1;

      QList<QCPGraph*>::Iterator itGraph = selectedGraphs.begin();

      while (itGraph != selectedGraphs.end())
      {
         QCPGraph *graph = *itGraph++;

         QCPDataSelection selection = graph->selection();

         for (int i = 0; i < selection.dataRangeCount(); i++)
         {
            QCPDataRange range = selection.dataRange(i);

            qDebug().noquote() << "selected" << graph->name() << "from"  << range.begin() << "to" << range.end();

            QCPGraphDataContainer::const_iterator data = graph->data()->at(range.begin());
            QCPGraphDataContainer::const_iterator end = graph->data()->at(range.end());

            while(data != end)
            {
               double timestamp = data->key;

               if (startTime < 0 || timestamp < startTime)
                  startTime = timestamp;

               if (endTime < 0 || timestamp > endTime)
                  endTime = timestamp;

               data++;
            }
         }
      }

      QModelIndexList selectionList = mFrameModel->modelRange(startTime, endTime);

      if (!selectionList.isEmpty())
      {
         QItemSelection selection(selectionList.first(), selectionList.last());

         ui->liveView->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
      }

      if (mSelectionMode == SelectMode::PlotSelection)
      {
         updateRange(startTime, endTime);
      }
   }

   // clear view selection
   else
   {
      ui->liveView->selectionModel()->clearSelection();
   }

   mSelectionMode = 0;
}

void MainWindow::plotRangeChanged(const QCPRange &newRange)
{
   if (newRange.lower != INFINITY && mLowerSignalRange != INFINITY && newRange.lower < mLowerSignalRange)
      ui->signalPlot->xAxis->setRangeLower(mLowerSignalRange);

   if (newRange.upper != INFINITY && mUpperSignalRange != INFINITY && newRange.upper > mUpperSignalRange)
      ui->signalPlot->xAxis->setRangeUpper(mUpperSignalRange);
}

void MainWindow::refreshView()
{
   if (mFrameModel->canFetchMore())
   {
      mFrameModel->fetchMore();

      if (mFollowEnabled)
      {
         ui->liveView->scrollToBottom();
      }
   }
}

void MainWindow::clearView()
{
   mStream->clear();
   clearModel();
   clearGraph();
}

void MainWindow::clearModel()
{
   mFrameModel->resetModel();
}

void MainWindow::clearGraph()
{
   ui->signalPlot->xAxis->setRange(0, 1);

   for (int i = 0; i < ui->signalPlot->graphCount(); i++)
   {
      QCPGraph *graph = ui->signalPlot->graph(i);

      graph->data()->clear();
      graph->addData(0, i); // reference start point
      graph->addData(0, i); // reference end point
      graph->setSelection(QCPDataSelection());
   }

   mPlotMarker->setRange(0, 0);
   mPlotMarker->setText("");

   ui->signalPlot->replot();
}

void MainWindow::plotMousePress(QMouseEvent *event)
{
   Qt::KeyboardModifiers keyModifiers = QGuiApplication::queryKeyboardModifiers();

   if (keyModifiers & Qt::ControlModifier)
   {
      mSelectionMode = SelectMode::PlotSelection;
      ui->signalPlot->setSelectionRectMode(QCP::srmSelect);
   }
   else
   {
      ui->signalPlot->setSelectionRectMode(QCP::srmNone);
   }
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
   if (event->key() == Qt::Key_C && (event->modifiers() & Qt::ControlModifier))
   {
      QApplication::clipboard()->setText(mClipboard);
   }
   else
   {
      QMainWindow::keyPressEvent(event);
   }
}


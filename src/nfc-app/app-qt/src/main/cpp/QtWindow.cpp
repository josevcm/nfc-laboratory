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

#include <QDebug>
#include <QKeyEvent>
#include <QClipboard>
#include <QComboBox>
#include <QTimer>
#include <QStandardPaths>
#include <QScreen>
#include <QRegularExpression>
#include <QDesktopServices>

#include <rt/Subject.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <model/StreamFilter.h>
#include <model/StreamModel.h>
#include <model/ParserModel.h>

#include <events/ConsoleLogEvent.h>
#include <events/DecoderControlEvent.h>
#include <events/FourierStatusEvent.h>
#include <events/LogicDecoderStatusEvent.h>
#include <events/LogicDeviceStatusEvent.h>
#include <events/RadioDecoderStatusEvent.h>
#include <events/RadioDeviceStatusEvent.h>
#include <events/SignalBufferEvent.h>
#include <events/StorageStatusEvent.h>
#include <events/StreamFrameEvent.h>
#include <events/SystemShutdownEvent.h>
#include <events/SystemStartupEvent.h>

#include <dialogs/InspectDialog.h>

#include <styles/Theme.h>

#include <features/Caps.h>

#include "ui_QtWindow.h"

#include "QtApplication.h"

#include "QtConfig.h"
#include "QtStorage.h"
#include "QtWindow.h"

#define DEFAULT_WINDOW_WIDTH 1024
#define DEFAULT_WINDOW_HEIGHT 720

struct QtWindow::Impl
{
   // data cache
   QtStorage *storage;

   // application window
   QtWindow *window;

   // configuration
   QSettings settings;

   // licensed devices
   QRegularExpression allowedDevices;

   // licensed features
   QRegularExpression allowedFeatures;

   // Toolbar status
   bool followEnabled = false;
   bool filterEnabled = false;

   // features metadata
   QMap<QString, QString> features;

   // running time list
   QMap<int, QString> timeList = {
      {5, "5 sec"},
      {10, "10 sec"},
      {20, "20 sec"},
      {30, "30 sec"},
      {40, "40 sec"},
      {60, "1 min"},
      {120, "2 min"},
      {300, "5 min"}
   };

   // global parameters
   int timeLimit = 10;

   // logic device parameters
   bool logicDeviceLicensed = false;
   QString logicDeviceName;
   QString logicDeviceModel;
   QString logicDeviceSerial;
   QString logicDeviceType;
   QString logicDeviceStatus;
   int logicSampleRate = 0;
   long long logicSampleCount = 0;

   // radio device parameters
   bool radioDeviceLicensed = false;
   QString radioDeviceName;
   QString radioDeviceModel;
   QString radioDeviceSerial;
   QString radioDeviceType;
   QString radioDeviceStatus;
   int radioCenterFrequency = 0;
   int radioSampleRate = 0;
   int radioGainMode = -1;
   int radioGainValue = -1;
   int radioBiasTee = 0;
   int radioDirectSampling = 0;
   long long radioSampleCount = 0;

   QList<int> radioGainKeys;
   QMap<int, QString> radioGainValues;
   QMap<int, QString> radioGainModes;

   // fourier parameters
   QString fourierStatus;

   // detected devices
   QStringList enabledDevices;
   QStringList disabledDevices;

   // feature flags
   bool featureFrameStream = true;
   bool featureLogicAcquire = true;
   bool featureLogicDecoder = true;
   bool featureRadioAcquire = true;
   bool featureRadioDecoder = true;
   bool featureRadioSpectrum = true;
   bool featureSignalRecord = true;

   // last decoder status received
   QString logicDecoderStatus = LogicDecoderStatusEvent::Disabled;
   QString radioDecoderStatus = RadioDecoderStatusEvent::Disabled;

   // working range
   double receivedTimeFrom = 0.0;
   double receivedTimeTo = 0.0;

   // selected range
   double selectedTimeFrom = 0.0;
   double selectedTimeTo = 0.0;

   // interface
   QSharedPointer<Ui_QtWindow> ui;

   // Acquire time selector
   QPointer<QComboBox> acquireLimit;

   // Frame view model
   QPointer<StreamModel> streamModel;

   // Frame view model
   QPointer<ParserModel> parserModel;

   // Frame filter
   QPointer<StreamFilter> streamFilter;

   // event inspect dialog
   QPointer<InspectDialog> inspectDialog;

   // refresh timer
   QPointer<QTimer> refreshTimer;

   // acquire timer
   QPointer<QTimer> acquireTimer;

   // Clipboard data
   QString clipboard;

   // fft signal data subject
   rt::Subject<hw::SignalBuffer> *frequencyStream = nullptr;

   // IQ signal stream subscription
   rt::Subject<hw::SignalBuffer>::Subscription signalIqSubscription;

   // fft signal stream subscription
   rt::Subject<hw::SignalBuffer>::Subscription frequencySubscription;

   // signal connections
   QMetaObject::Connection decodeViewDoubleClickedConnection;
   QMetaObject::Connection decodeViewSelectionChangedConnection;
   QMetaObject::Connection decodeViewValueChangedConnection;
   QMetaObject::Connection decodeViewIndicatorChangedConnection;
   QMetaObject::Connection framesViewToggleProtocolConnection;
   QMetaObject::Connection framesViewSelectionChangedConnection;
   QMetaObject::Connection framesViewRangeChangedConnection;
   QMetaObject::Connection framesScrollValueChangedConnection;
   QMetaObject::Connection logicViewSelectionChangedConnection;
   QMetaObject::Connection logicViewRangeChangedConnection;
   QMetaObject::Connection logicViewValueChangedConnection;
   QMetaObject::Connection radioViewSelectionChangedConnection;
   QMetaObject::Connection radioViewRangeChangedConnection;
   QMetaObject::Connection signalScrollValueChangedConnection;
   QMetaObject::Connection parserViewSelectionChangedConnection;
   QMetaObject::Connection timeLimitChangedConnection;
   QMetaObject::Connection refreshTimerTimeoutConnection;
   QMetaObject::Connection acquireTimerTimeoutConnection;

   explicit Impl(QtStorage *storage, QtWindow *window) : storage(storage),
                                                         window(window),
                                                         ui(new Ui_QtWindow()),
                                                         allowedDevices(".*"),
                                                         allowedFeatures(".*"),
                                                         acquireLimit(new QComboBox()),
                                                         streamModel(new StreamModel()),
                                                         parserModel(new ParserModel()),
                                                         streamFilter(new StreamFilter()),
                                                         inspectDialog(new InspectDialog(window)),
                                                         refreshTimer(new QTimer()),
                                                         acquireTimer(new QTimer())
   {
      // fft signal subject stream
      frequencyStream = rt::Subject<hw::SignalBuffer>::name("signal.fft");

      // subscribe to signal events
      frequencySubscription = frequencyStream->subscribe([=](const hw::SignalBuffer &buffer) {
         if (radioDeviceStatus == RadioDeviceStatusEvent::Streaming)
            ui->frequencyView->update(buffer);
      });
   }

   ~Impl()
   {
      disconnect(refreshTimerTimeoutConnection);
      disconnect(timeLimitChangedConnection);
      disconnect(parserViewSelectionChangedConnection);
      disconnect(signalScrollValueChangedConnection);
      disconnect(radioViewRangeChangedConnection);
      disconnect(radioViewSelectionChangedConnection);
      disconnect(logicViewValueChangedConnection);
      disconnect(logicViewRangeChangedConnection);
      disconnect(logicViewSelectionChangedConnection);
      disconnect(framesScrollValueChangedConnection);
      disconnect(framesViewRangeChangedConnection);
      disconnect(framesViewSelectionChangedConnection);
      disconnect(framesViewToggleProtocolConnection);
      disconnect(decodeViewIndicatorChangedConnection);
      disconnect(decodeViewValueChangedConnection);
      disconnect(decodeViewSelectionChangedConnection);
      disconnect(decodeViewDoubleClickedConnection);
   }

   void setupUi()
   {
      ui->setupUi(window);

      // set default upper tab
      ui->upperTabs->setCurrentWidget(ui->signalTab);

      // temporally hide unused elements
      ui->lowerTabs->setTabVisible(ui->lowerTabs->indexOf(ui->eventsTab), false);

      // setup capture time combo box
      acquireLimit->setMinimumWidth(100);

      // set combo from timeList
      for (auto it = timeList.begin(); it != timeList.end(); ++it)
         acquireLimit->addItem(it.value(), it.key());

      // fix upper toolbar
      auto *mainToolBarSeparator = new QWidget();
      mainToolBarSeparator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
      mainToolBarSeparator->setStyleSheet("QWidget{background-color: transparent;}");
      ui->mainToolBar->insertWidget(ui->actionInfo, acquireLimit);
      ui->mainToolBar->insertWidget(ui->actionInfo, mainToolBarSeparator);

      // fix decoder toolbar
      auto *decoderToolBarSeparator = new QWidget();
      decoderToolBarSeparator->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
      decoderToolBarSeparator->setStyleSheet("QWidget{background-color: transparent;}");
      ui->decoderToolBar->widgetForAction(ui->actionFollow)->setFixedSize(30, 32);
      ui->decoderToolBar->widgetForAction(ui->actionFilter)->setFixedSize(30, 32);
      ui->decoderToolBar->widgetForAction(ui->actionTime)->setFixedSize(30, 32);
      ui->decoderToolBar->widgetForAction(ui->actionReset)->setFixedSize(30, 32);
      ui->decoderToolBarLayout->addWidget(decoderToolBarSeparator);

      // setup filter
      streamFilter->setSourceModel(streamModel);

      // update window caption
      window->setWindowTitle(NFC_LAB_VENDOR_STRING);

      // setup default controls status
      ui->gainValue->setEnabled(false);

      // setup default action status
      ui->actionListen->setEnabled(false);
      ui->actionRecord->setEnabled(false);
      ui->actionStop->setEnabled(false);
      ui->actionPause->setEnabled(false);
      ui->actionPause->setChecked(false);

      // setup display stretch
      ui->workbench->setStretchFactor(0, 3);
      ui->workbench->setStretchFactor(1, 2);

      // setup display stretch
      ui->decoding->setStretchFactor(0, 3);
      ui->decoding->setStretchFactor(1, 2);

      // setup frame view model
      ui->decodeView->setModel(streamFilter);
      ui->decodeView->setColumnWidth(StreamModel::Id, 70);
      ui->decodeView->setColumnWidth(StreamModel::Time, 180);
      ui->decodeView->setColumnWidth(StreamModel::Delta, 80);
      ui->decodeView->setColumnWidth(StreamModel::Rate, 80);
      ui->decodeView->setColumnWidth(StreamModel::Tech, 80);
      ui->decodeView->setColumnWidth(StreamModel::Event, 100);
      ui->decodeView->setColumnWidth(StreamModel::Flags, 80);

      // disable sort for frame column
      ui->decodeView->setSortingEnabled(StreamModel::Id, true);
      ui->decodeView->setSortingEnabled(StreamModel::Time, true);
      ui->decodeView->setSortingEnabled(StreamModel::Delta, true);
      ui->decodeView->setSortingEnabled(StreamModel::Rate, true);
      ui->decodeView->setSortingEnabled(StreamModel::Tech, true);
      ui->decodeView->setSortingEnabled(StreamModel::Event, true);

      // initialize column display type
      ui->decodeView->setColumnType(StreamModel::Id, StreamWidget::Integer);
      ui->decodeView->setColumnType(StreamModel::Time, StreamWidget::Seconds);
      ui->decodeView->setColumnType(StreamModel::Delta, StreamWidget::Elapsed);
      ui->decodeView->setColumnType(StreamModel::Rate, StreamWidget::Rate);
      ui->decodeView->setColumnType(StreamModel::Tech, StreamWidget::String);
      ui->decodeView->setColumnType(StreamModel::Flags, StreamWidget::None);
      ui->decodeView->setColumnType(StreamModel::Event, StreamWidget::String);
      ui->decodeView->setColumnType(StreamModel::Data, StreamWidget::Hex);

      // disable move columns
      ui->decodeView->horizontalHeader()->setSectionsMovable(false);

      // setup channels view model
      ui->framesView->setModel(streamModel);

      // setup logic view model
      ui->logicView->setModel(streamModel);

      // setup radio view model
      ui->radioView->setModel(streamModel);

      // setup protocol view model
      ui->parserView->setModel(parserModel);
      ui->parserView->setColumnWidth(ParserModel::Name, 120);
      ui->parserView->setColumnWidth(ParserModel::Flags, 48);

      // hide parser view
      ui->parserWidget->setVisible(false);

      // connect channel view selection signal
      framesViewToggleProtocolConnection = connect(ui->framesView, &FramesWidget::toggleProtocol, [=](FramesWidget::Protocol proto, bool enabled) {
         framesToggleProtocol(proto, enabled);
      });

      // connect channel view selection signal
      framesViewSelectionChangedConnection = connect(ui->framesView, &FramesWidget::selectionChanged, [=](double from, double to) {
         framesSelectionChanged(from, to);
      });

      // connect channel view range signal
      framesViewRangeChangedConnection = connect(ui->framesView, &FramesWidget::rangeChanged, [=](double from, double to) {
         framesRangeChanged(from, to);
      });

      // connect channel scrollbar signal
      framesScrollValueChangedConnection = connect(ui->framesScroll, &QScrollBar::valueChanged, [=](int value) {
         framesScrollChanged(value);
      });

      // connect signal view selection signal
      logicViewSelectionChangedConnection = connect(ui->logicView, &LogicWidget::selectionChanged, [=](double from, double to) {
         logicSelectionChanged(from, to);
      });

      // connect signal view range signal
      logicViewRangeChangedConnection = connect(ui->logicView, &LogicWidget::rangeChanged, [=](double from, double to) {
         logicRangeChanged(from, to);
      });

      // connect signal view selection signal
      radioViewSelectionChangedConnection = connect(ui->radioView, &RadioWidget::selectionChanged, [=](double from, double to) {
         radioSelectionChanged(from, to);
      });

      // connect signal view range signal
      radioViewRangeChangedConnection = connect(ui->radioView, &RadioWidget::rangeChanged, [=](double from, double to) {
         radioRangeChanged(from, to);
      });

      // connect signal scrollbar signal
      signalScrollValueChangedConnection = connect(ui->signalScroll, &QScrollBar::valueChanged, [=](int value) {
         signalScrollChanged(value);
      });

      // connect stream view double click signal
      decodeViewDoubleClickedConnection = connect(ui->decodeView, &QTableView::doubleClicked, [=](const QModelIndex &index) {
         updateInspectDialog(index);
      });

      // connect stream view selection signal
      decodeViewSelectionChangedConnection = connect(ui->decodeView->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &deselected) {
         decoderSelectionChanged(selected, deselected);
      });

      // connect stream view scroll changed
      decodeViewValueChangedConnection = connect(ui->decodeView->verticalScrollBar(), &QScrollBar::valueChanged, [=](int value) {
         decoderScrollChanged(value);
      });

      // connect stream view sort changed
      decodeViewIndicatorChangedConnection = connect(ui->decodeView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, [=](int section, Qt::SortOrder order) {
         decoderSortChanged(section, order);
      });

      // connect selection signal from parser model
      parserViewSelectionChangedConnection = connect(ui->parserView->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &deselected) {
         parserSelectionChanged();
      });

      // connect time limit combo signal
      timeLimitChangedConnection = connect(acquireLimit, &QComboBox::currentIndexChanged, [=](int index) {
         changeTimeLimit(index);
      });

      // connect acquire timer signal
      acquireTimerTimeoutConnection = acquireTimer->callOnTimeout([=] {
         toggleStop();
      });

      // connect refresh timer signal
      refreshTimerTimeoutConnection = refreshTimer->callOnTimeout([=] {
         refreshView();
      });

      // create logic channels
      ui->logicView->addChannel(1, {Theme::defaultSignalPen, Theme::defaultLogicCLKPen, Theme::defaultLogicCLKBrush, Theme::defaultTextPen, Theme::defaultLabelFont, "CLK"});
      ui->logicView->addChannel(0, {Theme::defaultSignalPen, Theme::defaultLogicIOPen, Theme::defaultLogicIOBrush, Theme::defaultTextPen, Theme::defaultLabelFont, "IO"});
      ui->logicView->addChannel(2, {Theme::defaultSignalPen, Theme::defaultLogicRSTPen, Theme::defaultLogicRSTBrush, Theme::defaultSignalPen, Theme::defaultLabelFont, "RST"});
      ui->logicView->addChannel(3, {Theme::defaultSignalPen, Theme::defaultLogicVCCPen, Theme::defaultLogicVCCBrush, Theme::defaultSignalPen, Theme::defaultLabelFont, "VCC"});

      // acquire timer is one shot
      acquireTimer->setSingleShot(true);

      // start timer
      refreshTimer->start(500);

      // pre-select time limit
      acquireLimit->setCurrentIndex(acquireLimit->findData(timeLimit));
   }

   // event handlers
   void systemStartup(SystemStartupEvent *event)
   {
      features = event->meta;

      if (features.contains("devices"))
         allowedDevices.setPattern(event->meta["devices"]);

      if (features.contains("features"))
         allowedFeatures.setPattern(event->meta["features"]);

      qInfo() << "allowedDevices" << allowedDevices.pattern();
      qInfo() << "allowedFeatures" << allowedFeatures.pattern();

      // update features
      updateFeatures();

      // update actions status
      updateActions();

      // update header view
      updateStatus();

      // trigger ready signal
      window->ready();
   }

   void systemShutdown(SystemShutdownEvent *event)
   {
   }

   void logicDecoderStatusEvent(LogicDecoderStatusEvent *event)
   {
      bool updated = false;

      if (event->hasStatus())
         updated |= updateLogicDecoderStatus(event->status());

      if (event->hasIso7816())
         updated |= updateLogicDecoderProtocol(FramesWidget::ISO7816, event->iso7816());

      if (updated)
      {
         updateStatus();
         updateActions();
      }
   }

   void logicDeviceStatusEvent(LogicDeviceStatusEvent *event)
   {
      bool updated = false;

      // detect device changes
      if (event->hasName())
         updated |= updateLogicDeviceName(event->name());

      if (event->hasModel())
         updated |= updateLogicDeviceModel(event->model());

      if (event->hasSerial())
         updated |= updateLogicDeviceSerial(event->serial());

      if (event->hasStatus())
         updated |= updateLogicDeviceStatus(event->status());

      if (event->hasSampleRate())
         updated |= updateLogicDeviceSampleRate(event->sampleRate());

      if (event->hasSampleCount())
         updated |= updateLogicDeviceSampleCount(event->sampleCount());

      if (updated)
      {
         updateStatus();
         updateActions();
      }
   }

   void radioDecoderStatusEvent(RadioDecoderStatusEvent *event)
   {
      bool updated = false;

      if (event->hasStatus())
         updated |= updateRadioDecoderStatus(event->status());

      if (event->hasNfcA())
         updated |= updateRadioDecoderProtocol(FramesWidget::NfcA, event->nfca());

      if (event->hasNfcB())
         updated |= updateRadioDecoderProtocol(FramesWidget::NfcB, event->nfcb());

      if (event->hasNfcF())
         updated |= updateRadioDecoderProtocol(FramesWidget::NfcF, event->nfcf());

      if (event->hasNfcV())
         updated |= updateRadioDecoderProtocol(FramesWidget::NfcV, event->nfcv());

      if (updated)
      {
         updateStatus();
         updateActions();
      }
   }

   void radioDeviceStatusEvent(RadioDeviceStatusEvent *event)
   {
      bool updated = false;

      // catalogs must be update first
      if (event->hasGainModeList())
         updated |= updateRadioDeviceGainModes(event->gainModeList());

      if (event->hasGainValueList())
         updated |= updateRadioDeviceGainValues(event->gainValueList());

      // detect device changes
      if (event->hasName())
         updated |= updateRadioDeviceName(event->name());

      if (event->hasModel())
         updated |= updateRadioDeviceModel(event->model());

      if (event->hasSerial())
         updated |= updateRadioDeviceSerial(event->serial());

      if (event->hasStatus())
         updated |= updateRadioDeviceStatus(event->status());

      if (event->hasCenterFreq())
         updated |= updateRadioCenterFrequency(event->centerFreq());

      if (event->hasSampleRate())
         updated |= updateRadioDeviceSampleRate(event->sampleRate());

      if (event->hasGainMode())
         updated |= updateRadioDeviceGainMode(event->gainMode());

      if (event->hasGainValue())
         updated |= updateRadioDeviceGainValue(event->gainValue());

      if (event->hasSignalPower())
         updated |= updateRadioDeviceSignalPower(event->signalPower());

      if (event->hasSampleCount())
         updated |= updateRadioDeviceSampleCount(event->sampleCount());

      if (event->hasBiasTee())
         updated |= updateRadioDeviceBiasTee(event->biasTee());

      if (event->hasDirectSampling())
         updated |= updateRadioDeviceDirectSampling(event->directSampling());

      if (updated)
      {
         updateStatus();
         updateActions();
      }
   }

   void fourierStatusEvent(FourierStatusEvent *event)
   {
      bool updated = false;

      // detect changes
      if (event->hasStatus())
         updated |= updateFourierStatus(event->status());

      if (updated)
      {
         updateStatus();
         updateActions();
      }
   }

   void storageStatusEvent(StorageStatusEvent *event) const
   {
      // show message on storage error
      if (event->isError())
      {
         Theme::messageDialog(window, tr("Storage status"), event->message());
         return;
      }

      // reset view on storage stop
      if (event->isComplete())
      {
         refreshView();

         ui->framesView->reset();
         ui->framesView->refresh();
      }

      if (event->hasFileName())
      {
         ui->actionInfo->setText(event->fileName());
      }

      updateActions();
   }

   void streamFrameEvent(StreamFrameEvent *event) const
   {
      if (event->frame().isValid())
      {
         streamModel->append(event->frame());
      }
      else
      {
         // trigger channel view refresh on EOF
         ui->framesView->reset();
         ui->framesView->refresh();
      }
   }

   void signalBufferEvent(SignalBufferEvent *event)
   {
      if (event->buffer().isValid())
      {
         if (event->buffer().type() == hw::SignalType::SIGNAL_TYPE_LOGIC_SIGNAL || event->buffer().type() == hw::SignalType::SIGNAL_TYPE_LOGIC_SAMPLES)
         {
            ui->logicView->append(event->buffer());

            // update received time range
            if (ui->logicView->dataLowerRange() < receivedTimeFrom || receivedTimeFrom == 0.0)
               receivedTimeFrom = ui->logicView->dataLowerRange();

            if (ui->logicView->dataUpperRange() > receivedTimeTo || receivedTimeTo == 0.0)
               receivedTimeTo = ui->logicView->dataUpperRange();
         }
         else if (event->buffer().type() == hw::SignalType::SIGNAL_TYPE_RADIO_SIGNAL || event->buffer().type() == hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES)
         {
            ui->radioView->append(event->buffer());

            // update received time range
            if (ui->radioView->dataLowerRange() < receivedTimeFrom || receivedTimeFrom == 0.0)
               receivedTimeFrom = ui->logicView->dataLowerRange();

            if (ui->radioView->dataUpperRange() > receivedTimeTo || receivedTimeTo == 0.0)
               receivedTimeTo = ui->logicView->dataUpperRange();

            qInfo() << "Received radio signal from" << receivedTimeFrom << "to" << receivedTimeTo;
         }
      }
      else
      {
         // trigger signal view refresh on EOF
         ui->logicView->reset();
         ui->radioView->reset();

         ui->logicView->refresh();
         ui->radioView->refresh();
      }

      syncDataRanges();
   }

   void consoleLogEvent(ConsoleLogEvent *event)
   {
   }

   // setters
   void setTimeFormat(bool value) const
   {
      ui->actionTime->setChecked(value);

      if (value)
      {
         streamModel->setTimeSource(StreamModel::DateTime);
         ui->decodeView->setColumnWidth(StreamModel::Time, 250);
         ui->decodeView->setColumnType(StreamModel::Time, StreamWidget::DateTime);
      }
      else
      {
         streamModel->setTimeSource(StreamModel::Elapsed);
         ui->decodeView->setColumnWidth(StreamModel::Time, 125);
         ui->decodeView->setColumnType(StreamModel::Time, StreamWidget::Seconds);
      }

      ui->decodeView->update();
   }

   void setFollowEnabled(bool enabled)
   {
      followEnabled = enabled;

      ui->actionFollow->setChecked(followEnabled);
   }

   void setFilterEnabled(bool enabled)
   {
      filterEnabled = enabled;

      ui->actionFilter->setChecked(filterEnabled);

      streamFilter->setEnabled(filterEnabled);
   }

   void setFeatureLogicAcquire(bool enabled) const
   {
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::LogicDeviceConfig, {{"enabled", enabled}}));
   }

   void setFeatureLogicDecoder(bool enabled) const
   {
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::LogicDecoderConfig, {{"enabled", enabled}}));
   }

   void setFeatureRadioAcquire(bool enabled) const
   {
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDeviceConfig, {{"enabled", enabled}}));
   }

   void setFeatureRadioDecoder(bool enabled) const
   {
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDecoderConfig, {{"enabled", enabled}}));
   }

   void setFeatureFourierProcess(bool enabled) const
   {
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::FourierConfig, {{"enabled", enabled}}));
   }

   void setProtocolISO7816Enabled(bool enable) const
   {
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::LogicDecoderConfig, {{"protocol/iso7816/enabled", enable}}));
   }

   void setProtocolNfcAEnabled(bool enable) const
   {
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDecoderConfig, {{"protocol/nfca/enabled", enable}}));
   }

   void setProtocolNfcBEnabled(bool enable) const
   {
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDecoderConfig, {{"protocol/nfcb/enabled", enable}}));
   }

   void setProtocolNfcFEnabled(bool enable) const
   {
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDecoderConfig, {{"protocol/nfcf/enabled", enable}}));
   }

   void setProtocolNfcVEnabled(bool enable) const
   {
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDecoderConfig, {{"protocol/nfcv/enabled", enable}}));
   }

   // interface status update
   void syncDataRanges()
   {
      // sync logic && radio view ranges
      if (ui->logicView->dataUpperRange() > ui->radioView->dataUpperRange())
         ui->radioView->setDataUpperRange(ui->logicView->dataUpperRange());
      else if (ui->radioView->dataUpperRange() > ui->logicView->dataUpperRange())
         ui->logicView->setDataUpperRange(ui->radioView->dataUpperRange());

      if (ui->logicView->dataLowerRange() < ui->radioView->dataLowerRange())
         ui->radioView->setDataLowerRange(ui->logicView->dataLowerRange());
      else if (ui->radioView->dataLowerRange() < ui->logicView->dataLowerRange())
         ui->logicView->setDataLowerRange(ui->radioView->dataLowerRange());
   }

   /*
    * global status updates
    */
   void updateFeatures()
   {
      featureLogicAcquire = allowedFeatures.match(Caps::LOGIC_DEVICE).hasMatch();
      featureLogicDecoder = allowedFeatures.match(Caps::LOGIC_DECODE).hasMatch();
      featureRadioAcquire = allowedFeatures.match(Caps::RADIO_DEVICE).hasMatch();
      featureRadioDecoder = allowedFeatures.match(Caps::RADIO_DECODE).hasMatch();
      featureRadioSpectrum = allowedFeatures.match(Caps::RADIO_SPECTRUM).hasMatch();
      featureSignalRecord = allowedFeatures.match(Caps::SIGNAL_RECORD).hasMatch();

      // show available actions based on licensed features
      ui->featureLogicAcquire->setVisible(featureLogicAcquire);
      ui->featureLogicDecoder->setVisible(featureLogicDecoder);
      ui->featureRadioAcquire->setVisible(featureRadioAcquire);
      ui->featureRadioDecoder->setVisible(featureRadioDecoder);
      ui->featureRadioSpectrum->setVisible(featureRadioSpectrum);

      ui->featureLogicAcquire->setChecked(featureLogicAcquire);
      ui->featureLogicDecoder->setChecked(featureLogicDecoder);
      ui->featureRadioAcquire->setChecked(featureRadioAcquire);
      ui->featureRadioDecoder->setChecked(featureRadioDecoder);
      ui->featureRadioSpectrum->setChecked(featureRadioSpectrum);

      ui->actionRecord->setVisible(featureSignalRecord);

      // spectrum tab is visible if radio acquire and spectrum are enabled
      ui->upperTabs->setTabVisible(ui->upperTabs->indexOf(ui->spectrumTab), featureRadioAcquire && featureRadioSpectrum);
   }

   void updateActions() const
   {
      // flags for device status
      const bool logicDevicePresent = logicDeviceStatus != LogicDeviceStatusEvent::Absent;
      const bool radioDevicePresent = radioDeviceStatus != RadioDeviceStatusEvent::Absent;

      const bool logicDeviceEnabled = logicDeviceStatus != LogicDeviceStatusEvent::Disabled;
      const bool radioDeviceEnabled = radioDeviceStatus != RadioDeviceStatusEvent::Disabled;

      const bool logicDecoderEnabled = logicDecoderStatus != LogicDecoderStatusEvent::Disabled;
      const bool radioDecoderEnabled = radioDecoderStatus != RadioDecoderStatusEvent::Disabled;

      const bool fourierTaskEnabled = fourierStatus != FourierStatusEvent::Disabled;

      const bool devicePaused = logicDeviceStatus == LogicDeviceStatusEvent::Paused || radioDeviceStatus == RadioDeviceStatusEvent::Paused;
      const bool deviceStreaming = logicDeviceStatus == LogicDeviceStatusEvent::Streaming || radioDeviceStatus == RadioDeviceStatusEvent::Streaming;
      const bool decoderStreaming = logicDecoderStatus == LogicDecoderStatusEvent::Decoding || radioDecoderStatus == RadioDecoderStatusEvent::Decoding;

      // update feature status
      ui->featureLogicAcquire->setChecked(logicDeviceEnabled);
      ui->featureLogicDecoder->setChecked(logicDecoderEnabled);
      ui->featureRadioAcquire->setChecked(radioDeviceEnabled);
      ui->featureRadioDecoder->setChecked(radioDecoderEnabled);
      ui->featureRadioSpectrum->setChecked(fourierTaskEnabled);

      // disable / enable actions based on streaming status
      if (devicePaused || deviceStreaming || decoderStreaming)
      {
         // disable features during streaming
         ui->featureLogicAcquire->setEnabled(false);
         ui->featureLogicDecoder->setEnabled(false);
         ui->featureRadioAcquire->setEnabled(false);
         ui->featureRadioDecoder->setEnabled(false);
         ui->featureRadioSpectrum->setEnabled(false);

         // disable actions during streaming
         ui->actionListen->setEnabled(false);
         ui->actionRecord->setEnabled(false);
         ui->actionStop->setEnabled(true);
         ui->actionPause->setEnabled(true);

         // disable acquire limit combo
         acquireLimit->setEnabled(false);
      }
      else
      {
         // enable actions based on licensed devices
         ui->featureLogicAcquire->setEnabled(featureLogicAcquire && logicDevicePresent);
         ui->featureLogicDecoder->setEnabled(featureLogicDecoder);
         ui->featureRadioAcquire->setEnabled(featureRadioAcquire && radioDevicePresent);
         ui->featureRadioDecoder->setEnabled(featureRadioDecoder);
         ui->featureRadioSpectrum->setEnabled(featureRadioSpectrum && isActive(ui->featureRadioAcquire) && radioDevicePresent);

         // reset actions to default state
         ui->actionListen->setEnabled(isActive(ui->featureLogicAcquire) || isActive(ui->featureRadioAcquire));
         ui->actionRecord->setEnabled(isActive(ui->featureLogicAcquire) || isActive(ui->featureRadioAcquire));
         ui->actionStop->setEnabled(false);
         ui->actionPause->setEnabled(false);
         ui->actionPause->setChecked(false);

         // enable acquire limit combo
         acquireLimit->setEnabled(isActive(ui->featureLogicAcquire) || isActive(ui->featureRadioAcquire));
      }

      // flags for acquire status
      // const bool logicAcquireEnabled = isActive(ui->featureLogicAcquire);
      // const bool radioAcquireEnabled = isActive(ui->featureRadioAcquire);
      const bool spectrumEnabled = isActive(ui->featureRadioSpectrum);
      // const bool acquireEnabled = logicAcquireEnabled || radioAcquireEnabled;

      // flags for data status
      const bool logicSignalPresent = ui->logicView->hasData();
      const bool radioSignalPresent = ui->radioView->hasData();
      const bool logicSignalWide = ui->logicView->viewSizeRange() >= ui->logicView->dataSizeRange();
      const bool radioSignalWide = ui->radioView->viewSizeRange() >= ui->radioView->dataSizeRange();
      const bool signalSelected = ui->logicView->selectionSizeRange() > 0 || ui->radioView->selectionSizeRange() > 0;
      const bool signalPresent = logicSignalPresent || radioSignalPresent;
      const bool signalWide = logicSignalWide || radioSignalWide;
      const bool decoderEnabled = logicDecoderEnabled || radioDecoderEnabled;

      // update signal label
      if (signalPresent && !decoderEnabled)
         ui->signalLabel->setText(tr("Enable decoder to view acquired signal"));
      else if (!decoderEnabled)
         ui->signalLabel->setText(tr("Enable decoder and start acquisition or open a trace file"));
      else
         ui->signalLabel->setText(tr("Start acquisition or open a trace file"));

      // show views based on selected features
      ui->logicView->setVisible(logicSignalPresent);
      ui->radioView->setVisible(radioSignalPresent);
      ui->signalScroll->setVisible(signalPresent);
      ui->signalLabel->setVisible(!signalPresent);
      ui->frequencyView->setEnabled(spectrumEnabled);

      // if no data is present, disable related actions
      ui->actionClear->setEnabled(signalPresent);
      ui->actionSave->setEnabled(signalPresent);
      ui->actionTime->setEnabled(signalPresent);
      ui->actionZoom->setEnabled(signalSelected);
      ui->actionWide->setEnabled(!signalWide);
      ui->actionFilter->setEnabled(streamModel->rowCount() != 0);
      ui->decodeView->setEnabled(streamModel->rowCount() != 0);

      // update protocol status
      ui->protocolISO14443A->setEnabled(radioDecoderEnabled);
      ui->protocolISO14443B->setEnabled(radioDecoderEnabled);
      ui->protocolISO18092->setEnabled(radioDecoderEnabled);
      ui->protocolISO15693->setEnabled(radioDecoderEnabled);
      ui->protocolISO7816->setEnabled(logicDecoderEnabled);

      // radio controls
      ui->gainValue->setEnabled(radioDeviceStatus != RadioDeviceStatusEvent::Absent);
   }

   void updateStatus() const
   {
      // flags for device status
      const bool devicePaused = logicDeviceStatus == LogicDeviceStatusEvent::Paused || radioDeviceStatus == RadioDeviceStatusEvent::Paused;

      ui->actionInfo->setText("");

      if (enabledDevices.isEmpty())
         ui->statusBar->showMessage(tr("No devices available"));
      else
         ui->statusBar->showMessage(QString(tr("Detected %1").arg(enabledDevices.join(", "))));

      if (devicePaused)
         ui->actionInfo->setText(QString("PAUSED, Remaining time %1:%2").arg(timeLimit / 60, 2, 10, QChar('0')).arg(timeLimit % 60, 2, 10, QChar('0')));

      else if (!enabledDevices.isEmpty())
         ui->actionInfo->setText(tr("Devices Ready"));

      // show if not licensed devices are present
      if (!disabledDevices.isEmpty())
         ui->statusBar->showMessage(tr("Device %1 is not supported").arg(disabledDevices.join(", ")));

      // show owner
      window->setWindowTitle(QString(NFC_LAB_VENDOR_STRING));
   }

   /*
    * logic status updates
    */
   bool updateLogicDecoderStatus(const QString &value)
   {
      if (logicDecoderStatus == value)
         return false;

      qInfo().noquote() << "logic decoder status changed from [" << logicDecoderStatus << "] to [" << value << "]";

      // reset view on decoder stop
      if (value == LogicDecoderStatusEvent::Idle && logicDecoderStatus == LogicDecoderStatusEvent::Decoding)
      {
         refreshView();

         ui->framesView->reset();
         ui->framesView->refresh();
      }

      logicDecoderStatus = value;

      return true;
   }

   bool updateLogicDecoderProtocol(const int proto, const QJsonObject &status) const
   {
      bool enabled = status["enabled"].toBool();

      switch (proto)
      {
         case FramesWidget::ISO7816:
         {
            if (ui->protocolISO7816->isChecked() == enabled && ui->framesView->hasProtocol(FramesWidget::ISO7816) == enabled)
               return false;

            qInfo().noquote() << "decoder proto ISO-7816 enabled:" << enabled;

            ui->protocolISO7816->setChecked(enabled);
            ui->framesView->setProtocol(FramesWidget::ISO7816, enabled);

            return true;
         }

         default:
            return false;
      }
   }

   bool updateLogicDeviceName(const QString &value)
   {
      if (logicDeviceName == value)
         return false;

      qInfo().noquote() << "logic device name changed from [" << logicDeviceName << "] to [" << value << "]";

      logicDeviceName = value;

      return true;
   }

   bool updateLogicDeviceModel(const QString &value)
   {
      if (logicDeviceModel == value)
         return false;

      qInfo().noquote() << "logic device model changed from [" << logicDeviceModel << "] to [" << value << "]";

      logicDeviceModel = value;

      if (!logicDeviceSerial.isEmpty())
      {
         if (logicDeviceLicensed)
         {
            if (!enabledDevices.contains(logicDeviceModel))
               enabledDevices << logicDeviceModel;
         }
         else
         {
            if (!disabledDevices.contains(logicDeviceModel))
               disabledDevices << logicDeviceModel;
         }
      }

      return true;
   }

   bool updateLogicDeviceSerial(const QString &value)
   {
      if (logicDeviceSerial == value)
         return false;

      qInfo().noquote() << "logic device serial changed from [" << logicDeviceSerial << "] to [" << value << "]";

      logicDeviceSerial = value;
      logicDeviceLicensed = allowedDevices.match(logicDeviceSerial).hasMatch();

      if (!logicDeviceModel.isEmpty())
      {
         if (logicDeviceLicensed)
         {
            if (!enabledDevices.contains(logicDeviceModel))
               enabledDevices << logicDeviceModel;
         }
         else
         {
            if (!disabledDevices.contains(logicDeviceModel))
               disabledDevices << logicDeviceModel;
         }

         qInfo() << "logic device" << logicDeviceModel << "serial changed to" << logicDeviceSerial << "licensed" << logicDeviceLicensed;
      }
      else
      {
         qInfo() << "logic device serial changed to" << logicDeviceSerial << "licensed" << logicDeviceLicensed;
      }

      ui->featureLogicAcquire->setEnabled(logicDeviceLicensed);

      return true;
   }

   bool updateLogicDeviceStatus(const QString &value)
   {
      if (logicDeviceStatus == value)
         return false;

      qInfo().noquote() << "logic device status changed from [" << logicDeviceStatus << "] to [" << value << "]";

      logicDeviceStatus = value;

      if (logicDeviceStatus == LogicDeviceStatusEvent::Absent)
      {
         enabledDevices.removeAll(logicDeviceModel);

         // clear device information
         logicDeviceName.clear();
         logicDeviceSerial.clear();
         logicDeviceModel.clear();
         logicDeviceType.clear();
         logicDeviceLicensed = false;
      }

      return true;
   }

   bool updateLogicDeviceSampleRate(long value)
   {
      if (logicSampleRate == value)
         return false;

      qInfo().noquote() << "logic device sample rate changed from [" << logicSampleRate << "] to [" << value << "]";

      logicSampleRate = value;

      return true;
   }

   bool updateLogicDeviceSampleCount(long long value)
   {
      if (logicSampleCount == value)
         return false;

      qInfo().noquote() << "logic device sample count changed from [" << logicSampleCount << "] to [" << value << "]";

      logicSampleCount = value;

      return true;
   }

   /*
    * radio status updates
    */
   bool updateRadioDecoderStatus(const QString &value)
   {
      if (radioDecoderStatus == value)
         return false;

      qInfo().noquote() << "radio decoder status changed from [" << radioDecoderStatus << "] to [" << value << "]";

      // reset view on decoder stop
      if (value == RadioDecoderStatusEvent::Idle && radioDecoderStatus == RadioDecoderStatusEvent::Decoding)
      {
         refreshView();

         ui->framesView->reset();
         ui->framesView->refresh();
      }

      radioDecoderStatus = value;

      return true;
   }

   bool updateRadioDecoderProtocol(const int proto, const QJsonObject &status) const
   {
      bool enabled = status["enabled"].toBool();

      switch (proto)
      {
         case FramesWidget::NfcA:
         {
            if (ui->protocolISO14443A->isChecked() == enabled && ui->framesView->hasProtocol(FramesWidget::NfcA) == enabled)
               return false;

            qInfo().noquote() << "radio decoder protocol" << ui->protocolISO14443A->objectName() << "enabled:" << enabled;

            ui->protocolISO14443A->setChecked(enabled);
            ui->framesView->setProtocol(FramesWidget::NfcA, enabled);

            return true;
         }

         case FramesWidget::NfcB:
         {
            if (ui->protocolISO14443B->isChecked() == enabled && ui->framesView->hasProtocol(FramesWidget::NfcB) == enabled)
               return false;

            qInfo().noquote() << "radio decoder protocol" << ui->protocolISO14443B->objectName() << "enabled:" << enabled;

            ui->protocolISO14443B->setChecked(enabled);
            ui->framesView->setProtocol(FramesWidget::NfcB, enabled);

            return true;
         }

         case FramesWidget::NfcF:
         {
            if (ui->protocolISO18092->isChecked() == enabled && ui->framesView->hasProtocol(FramesWidget::NfcF) == enabled)
               return false;

            qInfo().noquote() << "radio decoder protocol" << ui->protocolISO18092->objectName() << "enabled:" << enabled;

            ui->protocolISO18092->setChecked(enabled);
            ui->framesView->setProtocol(FramesWidget::NfcF, enabled);

            return true;
         }

         case FramesWidget::NfcV:
         {
            if (ui->protocolISO15693->isChecked() == enabled && ui->framesView->hasProtocol(FramesWidget::NfcV) == enabled)
               return false;

            qInfo().noquote() << "radio decoder protocol" << ui->protocolISO15693->objectName() << "enabled:" << enabled;

            ui->protocolISO15693->setChecked(enabled);
            ui->framesView->setProtocol(FramesWidget::NfcV, enabled);

            return true;
         }

         default:
            return false;
      }
   }

   bool updateRadioDeviceName(const QString &value)
   {
      if (radioDeviceName == value)
         return false;

      qInfo().noquote() << "radio device name changed from [" << radioDeviceName << "] to [" << value << "]";

      radioDeviceName = value;

      qInfo() << "radio device name changed to:" << radioDeviceName;

      return true;
   }

   bool updateRadioDeviceModel(const QString &value)
   {
      if (radioDeviceModel == value)
         return false;

      qInfo().noquote() << "radio device model changed from [" << radioDeviceModel << "] to [" << value << "]";

      radioDeviceModel = value;

      if (!radioDeviceSerial.isEmpty())
      {
         if (radioDeviceLicensed)
         {
            if (!enabledDevices.contains(radioDeviceModel))
               enabledDevices << radioDeviceModel;
         }
         else
         {
            if (!disabledDevices.contains(radioDeviceModel))
               disabledDevices << radioDeviceModel;
         }
      }

      return true;
   }

   bool updateRadioDeviceSerial(const QString &value)
   {
      if (radioDeviceSerial == value)
         return false;

      qInfo().noquote() << "radio device name serial from [" << radioDeviceSerial << "] to [" << value << "]";

      radioDeviceSerial = value;
      radioDeviceLicensed = allowedDevices.match(radioDeviceSerial).hasMatch();

      if (!radioDeviceModel.isEmpty())
      {
         if (radioDeviceLicensed)
         {
            if (!enabledDevices.contains(radioDeviceModel))
               enabledDevices << radioDeviceModel;
         }
         else
         {
            if (!disabledDevices.contains(radioDeviceModel))
               disabledDevices << radioDeviceModel;
         }

         qInfo() << "radio device" << radioDeviceModel << "serial changed to" << radioDeviceSerial << "licensed" << radioDeviceLicensed;
      }
      else
      {
         qInfo() << "radio device serial changed to" << radioDeviceSerial << "licensed" << radioDeviceLicensed;
      }

      ui->featureRadioAcquire->setEnabled(radioDeviceLicensed);
      ui->featureRadioSpectrum->setEnabled(radioDeviceLicensed);

      return true;
   }

   bool updateRadioDeviceStatus(const QString &value)
   {
      if (radioDeviceStatus == value)
         return false;

      qInfo().noquote() << "radio device device status from [" << radioDeviceStatus << "] to [" << value << "]";

      radioDeviceStatus = value;

      if (radioDeviceStatus == RadioDeviceStatusEvent::Absent)
      {
         enabledDevices.removeAll(radioDeviceModel);

         radioDeviceName.clear();
         radioDeviceSerial.clear();
         radioDeviceModel.clear();
         radioDeviceType.clear();
         radioGainMode = -1;
         radioGainValue = -1;
         radioDeviceLicensed = false;
      }

      changeGainMode(radioGainMode);
      changeGainValue(radioGainValue);

      return true;
   }

   bool updateRadioDeviceGainModes(const QMap<int, QString> &value)
   {
      if (radioGainModes == value)
         return false;

      qInfo() << "radio device available gain modes:" << value;

      radioGainModes = value;

      return true;
   }

   bool updateRadioDeviceGainValues(const QMap<int, QString> &value)
   {
      if (radioGainValues == value)
         return false;

      radioGainValues = value;
      radioGainKeys = radioGainValues.keys();

      if (!radioGainKeys.isEmpty())
      {
         ui->gainValue->setRange(0, static_cast<int>(radioGainKeys.count()));

         if (radioGainKeys.contains(radioGainValue))
            ui->gainValue->setValue(static_cast<int>(radioGainKeys.indexOf(radioGainValue)));
         else
            ui->gainValue->setValue(0);
      }
      else
      {
         ui->gainValue->setRange(0, 0);
      }

      qInfo() << "radio device available gain values:" << radioGainValues;

      return true;
   }

   bool updateRadioCenterFrequency(long value)
   {
      if (radioCenterFrequency == value)
         return false;

      qInfo().noquote() << "radio device device frequency from [" << radioCenterFrequency << "] to [" << value << "]";

      radioCenterFrequency = value;

      ui->frequencyView->setCenterFreq(radioCenterFrequency);

      return true;
   }

   bool updateRadioDeviceSampleRate(long value)
   {
      if (radioSampleRate == value)
         return false;

      qInfo().noquote() << "radio device device sample rate from [" << radioSampleRate << "] to [" << value << "]";

      radioSampleRate = value;

      ui->frequencyView->setSampleRate(radioSampleRate);

      return true;
   }

   bool updateRadioDeviceGainMode(long value)
   {
      if (radioGainMode == value)
         return false;

      qInfo().noquote() << "radio device device gain mode from [" << radioGainMode << "] to [" << value << "]";

      radioGainMode = value;

      return true;
   }

   bool updateRadioDeviceGainValue(long value)
   {
      if (radioGainValue == value || !radioGainKeys.contains(value))
         return false;

      qInfo().noquote() << "radio device device gain value from [" << radioGainValue << "] to [" << value << "]";

      radioGainValue = value;

      ui->gainValue->setValue(static_cast<int>(radioGainKeys.indexOf(radioGainValue)));
      ui->gainLabel->setText(radioGainValues[radioGainValue]);

      return true;
   }

   bool updateRadioDeviceBiasTee(int value)
   {
      if (radioBiasTee == value)
         return false;

      qInfo().noquote() << "radio device device bias-tee from [" << radioBiasTee << "] to [" << value << "]";

      radioBiasTee = value;

      return true;
   }

   bool updateRadioDeviceDirectSampling(int value)
   {
      if (radioDirectSampling == value)
         return false;

      qInfo().noquote() << "radio device device direct sampling from [" << radioDirectSampling << "] to [" << value << "]";

      radioDirectSampling = value;

      return true;
   }

   bool updateRadioDeviceSampleCount(long long value)
   {
      if (radioSampleCount == value)
         return false;

      qInfo().noquote() << "radio device device direct sample count from [" << radioSampleCount << "] to [" << value << "]";

      radioSampleCount = value;

      return true;
   }

   bool updateRadioDeviceSignalPower(float value) const
   {
      //      ui->signalStrength->setValue(value * 100);

      return false;
   }

   /*
    * spectrum status updates
    */
   bool updateFourierStatus(const QString &value)
   {
      if (fourierStatus == value)
         return false;

      qInfo().noquote() << "update fourier transformer state from [" << fourierStatus << "] to [" << value << "]";

      fourierStatus = value;

      return true;
   }

   /*
    * data status updates
    */
   void updateInspectDialog(const QModelIndex &index) const
   {
      auto firstIndex = index;

      if (!index.isValid())
         return;

      inspectDialog->clear();

      if (const auto firstFrame = streamFilter->frame(firstIndex))
      {
         switch (firstFrame->frameType())
         {
            case lab::FrameType::NfcPollFrame:
            case lab::FrameType::IsoRequestFrame:
            {
               inspectDialog->addFrame(*firstFrame);

               auto secondIndex = streamFilter->index(firstIndex.row() + 1, 0);

               if (secondIndex.isValid())
               {
                  if (auto secondFrame = streamFilter->frame(secondIndex))
                  {
                     if (secondFrame->frameType() == lab::FrameType::NfcListenFrame || secondFrame->frameType() == lab::FrameType::IsoResponseFrame)
                     {
                        inspectDialog->addFrame(*secondFrame);
                     }
                  }
               }

               break;
            }

            case lab::FrameType::NfcListenFrame:
            case lab::FrameType::IsoResponseFrame:
            {
               auto secondIndex = streamFilter->index(firstIndex.row() - 1, 0);

               if (secondIndex.isValid())
               {
                  if (auto secondFrame = streamFilter->frame(secondIndex))
                  {
                     if (secondFrame->frameType() == lab::FrameType::NfcPollFrame || secondFrame->frameType() == lab::FrameType::IsoRequestFrame)
                     {
                        inspectDialog->addFrame(*secondFrame);
                        inspectDialog->addFrame(*firstFrame);
                     }
                  }
               }

               break;
            }

            default:
            {
               inspectDialog->addFrame(*firstFrame);
               break;
            }
         }
      }

      inspectDialog->showModal();
   }

   void updateParserView()
   {
      // get selected rows
      QModelIndexList indexList = ui->decodeView->selectionModel()->selectedIndexes();

      if (indexList.isEmpty())
      {
         // hide dialog if no event is selected
         inspectDialog->close();

         // hide parser view
         ui->parserWidget->hide();

         // and finish...
         return;
      }

      // show parser view
      ui->parserWidget->show();

      // prepare clipboard content
      updateParserModel(indexList);

      // update inspect dialog if is already open
      if (inspectDialog->isVisible())
         updateInspectDialog(indexList.first());
   }

   void updateParserModel(QModelIndexList &indexList) const
   {
      // select first request-response
      parserModel->resetModel();

      auto firstIndex = indexList.first();

      if (auto firstFrame = streamFilter->frame(firstIndex))
      {
         ui->dataView->setData(toByteArray(*firstFrame));

         switch (firstFrame->frameType())
         {
            case lab::FrameType::NfcPollFrame:
            case lab::FrameType::IsoRequestFrame:
            {
               parserModel->append(*firstFrame);

               auto secondIndex = streamFilter->index(firstIndex.row() + 1, 0);

               if (secondIndex.isValid())
               {
                  if (auto secondFrame = streamFilter->frame(secondIndex))
                  {
                     if (secondFrame->frameType() == lab::FrameType::NfcListenFrame || secondFrame->frameType() == lab::FrameType::IsoResponseFrame)
                     {
                        parserModel->append(*secondFrame);
                     }
                  }
               }

               break;
            }

            case lab::FrameType::NfcListenFrame:
            case lab::FrameType::IsoResponseFrame:
            {
               auto secondIndex = streamFilter->index(firstIndex.row() - 1, 0);

               if (secondIndex.isValid())
               {
                  if (auto secondFrame = streamFilter->frame(secondIndex))
                  {
                     if (secondFrame->frameType() == lab::FrameType::NfcPollFrame || secondFrame->frameType() == lab::FrameType::IsoRequestFrame)
                     {
                        parserModel->append(*secondFrame);
                        parserModel->append(*firstFrame);
                     }
                  }
               }

               break;
            }

            default:
            {
               parserModel->append(*firstFrame);
               break;
            }
         }
      }

      // expand protocol information
      ui->parserView->expandAll();

      // update columns width
      ui->parserView->resizeColumnToContents(ParserModel::Data);
   }

   /*
    * clipboard update
    */
   void clipboardPrepare(QModelIndexList &indexList)
   {
      QString text;

      QModelIndex previous;

      for (const QModelIndex &current: indexList)
      {
         if (!previous.isValid() || current.row() != previous.row())
         {
            if (const lab::RawFrame *frame = streamFilter->frame(current))
            {
               for (int i = 0; i < frame->limit(); i++)
               {
                  text.append(QString("%1 ").arg((*frame)[i], 2, 16, QLatin1Char('0')));
               }

               text.append(QLatin1Char('\n'));
            }
         }

         previous = current;
      }

      // copy data to clipboard buffer
      clipboard = text.toUpper();
   }

   void clipboardCopy() const
   {
      QApplication::clipboard()->setText(clipboard);
   }

   /*
    * slots for interface actions
    */
   void openFile()
   {
      qInfo() << "open file";

      QDir dataPath = QtApplication::dataPath();

      QString fileName = Theme::openFileDialog(window, tr("Open trace file"), dataPath.absolutePath(), tr("Capture (*.wav *.trz)"));

      if (fileName.isEmpty())
         return;

      if (!(fileName.endsWith(".wav") || fileName.endsWith(".trz")))
      {
         Theme::messageDialog(window, tr("Unable to open file"), tr("Invalid file name: %1").arg(fileName));
         return;
      }

      QFile file(fileName);

      if (!file.open(QIODevice::ReadOnly))
      {
         Theme::messageDialog(window, tr("Unable to open file"), file.errorString());
         return;
      }

      clearView();

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::ReadFile, {
                                                     {"fileName", fileName}
                                                  }));
   }

   void saveFile() const
   {
      qInfo() << "save signal trace";

      bool hasSelection = selectedTimeTo > selectedTimeFrom;

      double writeTimeFrom = hasSelection ? selectedTimeFrom : receivedTimeFrom;
      double writeTimeTo = hasSelection ? selectedTimeTo : receivedTimeTo;

      QString path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
      QString date = QDateTime::currentDateTime().toString("yyyy_MM_dd-HH_mm_ss");
      QString name = QString("%1-trace.trz").arg(date);

      QString fileName = Theme::saveFileDialog(window, hasSelection ? tr("Save selection to trace file") : tr("Save all to trace file"), path + "/" + name, tr("Trace (*.trz)"));

      if (!fileName.isEmpty())
      {
         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::WriteFile, {
                                                        {"fileName", fileName},
                                                        {"sampleRate", radioSampleRate},
                                                        {"timeStart", writeTimeFrom},
                                                        {"timeEnd", writeTimeTo}
                                                     }));
      }
   }

   void openStorage() const
   {
      qInfo() << "open storage folder";

      QDesktopServices::openUrl(QUrl::fromLocalFile(QtApplication::dataPath().absolutePath()));
   }

   void openConfig() const
   {
      QString filePath = settings.fileName();

      QFileInfo info(filePath);

      if (!info.exists())
      {
         qWarning("File not found: %s", qUtf8Printable(filePath));
         return;
      }

      QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
   }

   void toggleStart(bool recording)
   {
      qInfo() << "decoder starting";

      clearView();

      // prepare views for start
      ui->logicView->start();
      ui->radioView->start();
      ui->framesView->start();
      ui->frequencyView->start();

      // disable action to avoid multiple start
      ui->actionListen->setEnabled(false);
      ui->actionRecord->setEnabled(false);

      // switch to receiver tab
      if (featureRadioAcquire)
         ui->upperTabs->setCurrentWidget(ui->spectrumTab);

      // enable follow
      setFollowEnabled(true);

      // get selected time limit
      timeLimit = acquireLimit->currentData().toInt();

      // start decoder
      if (recording || settings.value("settings/recordEnabled", false).toBool())
      {
         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::Start, {{"storagePath", QtApplication::dataPath().absolutePath()}}));
      }
      else
      {
         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::Start, {}));
      }

      acquireTimer->start(timeLimit * 1000);
   }

   void togglePause()
   {
      if (ui->actionPause->isChecked())
      {
         qInfo() << "decoder pause, remaining time:" << acquireTimer->remainingTime();

         // get remaining time
         timeLimit = acquireTimer->remainingTime() / 1000;

         // stopping timer
         acquireTimer->stop();

         // sync logic && radio view ranges
         syncDataRanges();

         // pause decoder
         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::Pause));
      }
      else
      {
         qInfo() << "decoder resume, remaining time:" << timeLimit;

         // resume timer
         acquireTimer->start(timeLimit * 1000);

         // resume decoder
         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::Resume));
      }
   }

   void toggleStop()
   {
      qInfo() << "decoder stopping";

      // stopping timer
      acquireTimer->stop();

      // update views for stop
      ui->logicView->stop();
      ui->radioView->stop();
      ui->framesView->stop();
      ui->frequencyView->stop();

      // disable action to avoid multiple pause / stop
      ui->actionStop->setEnabled(false);
      ui->actionPause->setEnabled(false);
      ui->actionPause->setChecked(false);

      // sync logic && radio view ranges
      syncDataRanges();

      // stop decoder
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::Stop));
   }

   void toggleTime()
   {
      setTimeFormat(ui->actionTime->isChecked());
   }

   void toggleFollow()
   {
      setFollowEnabled(ui->actionFollow->isChecked());
   }

   void toggleFilter()
   {
      setFilterEnabled(ui->actionFilter->isChecked());
   }

   void zoomReset() const
   {
      ui->logicView->zoomReset();
      ui->radioView->zoomReset();
      ui->framesView->zoomReset();
      ui->frequencyView->zoomReset();
   }

   void zoomSelection() const
   {
      ui->logicView->zoomSelection();
      ui->radioView->zoomSelection();
      ui->framesView->zoomSelection();
   }

   void toggleFeature(QAction *action) const
   {
      qInfo().noquote() << "toggle feature" << action->objectName() << "enabled:" << action->isChecked();

      if (action == ui->featureLogicAcquire)
         setFeatureLogicAcquire(action->isChecked());
      else if (action == ui->featureLogicDecoder)
         setFeatureLogicDecoder(action->isChecked());
      else if (action == ui->featureRadioAcquire)
         setFeatureRadioAcquire(action->isChecked());
      else if (action == ui->featureRadioDecoder)
         setFeatureRadioDecoder(action->isChecked());
      else if (action == ui->featureRadioSpectrum)
         setFeatureFourierProcess(action->isChecked());

      updateActions();
   }

   void toggleProtocol(QAction *action) const
   {
      qInfo().noquote() << "toggle protocol" << action->objectName() << "enabled:" << action->isChecked();

      if (action == ui->protocolISO14443A)
         setProtocolNfcAEnabled(action->isChecked());
      else if (action == ui->protocolISO14443B)
         setProtocolNfcBEnabled(action->isChecked());
      else if (action == ui->protocolISO18092)
         setProtocolNfcFEnabled(action->isChecked());
      else if (action == ui->protocolISO15693)
         setProtocolNfcVEnabled(action->isChecked());
      else if (action == ui->protocolISO7816)
         setProtocolISO7816Enabled(action->isChecked());

      updateActions();
   }

   void showAboutInfo()
   {
   }

   void showHelpInfo()
   {
   }

   void changeTimeLimit(int index)
   {
      if (index < 0 || index >= acquireLimit->count())
         return;

      int value = acquireLimit->currentData().toInt();

      if (timeLimit != value)
      {
         timeLimit = value;
         qInfo() << "capture time limit changed to" << timeLimit;
      }
   }

   void changeGainMode(int value)
   {
      if (radioGainMode == value)
         return;

      radioGainMode = value;

      qInfo() << "receiver gain mode changed to" << value;

      ui->gainLabel->setText(radioGainValues[radioGainValue]);

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDeviceConfig, {
                                                     {"gainMode", radioGainMode},
                                                     {"gainValue", radioGainValue}
                                                  }));
   }

   void changeGainValue(int index)
   {
      if (index < 0 || index >= radioGainKeys.count())
         return;

      int value = radioGainKeys[index];

      if (radioGainValue == value || !radioGainKeys.contains(value))
         return;

      radioGainValue = value;

      if (radioGainMode != 0)
      {
         qInfo() << "receiver gain value changed to" << value;

         ui->gainValue->setValue(index);
         ui->gainLabel->setText(radioGainValues[radioGainValue]);

         QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDeviceConfig, {
                                                        {"gainMode", radioGainMode},
                                                        {"gainValue", radioGainValue}
                                                     }));
      }
   }

   void trackGainValue(int index)
   {
      if (index < 0 || index >= radioGainKeys.count())
         return;

      int value = radioGainKeys[index];

      qInfo() << "receiver gain value changed to" << value;

      ui->gainLabel->setText(radioGainValues[value]);
   }

   void clearSelection() const
   {
      qInfo() << "clear selection";

      // clear stream model selection
      ui->decodeView->clearSelection();

      // clean select frames in signal view
      ui->logicView->select(-1, -1);
      ui->logicView->repaint();

      // clean select frames in signal view
      ui->radioView->select(-1, -1);
      ui->radioView->repaint();

      // clean select frames in channels view
      ui->framesView->select(-1, -1);
      ui->framesView->repaint();
   }

   void clearView()
   {
      qInfo() << "clear events and views";

      // clear stream model
      streamModel->resetModel();

      // clear parser model
      parserModel->resetModel();

      // hide parser view
      ui->parserWidget->hide();

      // clear al views
      ui->logicView->clear();
      ui->radioView->clear();
      ui->frequencyView->clear();

      // clear ranges
      receivedTimeFrom = 0.0;
      receivedTimeTo = 0.0;
      selectedTimeFrom = 0.0;
      selectedTimeTo = 0.0;

      // signal clear to decoder control
      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::Clear));
   }

   void resetView() const
   {
      qInfo() << "reset view";

      // reset columns width
      ui->decodeView->setColumnWidth(StreamModel::Id, 70);
      ui->decodeView->setColumnWidth(StreamModel::Time, 180);
      ui->decodeView->setColumnWidth(StreamModel::Delta, 80);
      ui->decodeView->setColumnWidth(StreamModel::Rate, 80);
      ui->decodeView->setColumnWidth(StreamModel::Tech, 80);
      ui->decodeView->setColumnWidth(StreamModel::Event, 100);
      ui->decodeView->setColumnWidth(StreamModel::Flags, 80);

      // reset sort indicator to first column
      ui->decodeView->horizontalHeader()->setSortIndicator(StreamModel::Id, Qt::AscendingOrder);

      // clear all filters
      ui->decodeView->clearFilters();
   }

   void refreshView() const
   {
      if (acquireTimer->isActive())
      {
         int seconds = acquireTimer->remainingTime() / 1000;
         int minutes = seconds / 60;

         ui->actionInfo->setText(QString("Remaining time %1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds % 60, 2, 10, QChar('0')));
      }

      if (!streamModel->canFetchMore())
         return;

      // fetch pending data from model
      streamModel->fetchMore();

      // enable view if data is present
      if (!ui->decodeView->isEnabled() && streamModel->rowCount() > 0)
         ui->decodeView->setEnabled(true);

      if (followEnabled)
         ui->decodeView->scrollToBottom();

      // update view to fit all content
      ui->decodeView->resizeColumnToContents(StreamModel::Data);
   }

   void decoderSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
   {
      QModelIndexList indexList = ui->decodeView->selectionModel()->selectedIndexes();

      if (indexList.isEmpty())
      {
         // hide dialog if no event is selected
         inspectDialog->close();

         // hide parser view
         ui->parserWidget->hide();

         // and finish...
         return;
      }

      double startTime, endTime;

      // prepare clipboard content
      clipboardPrepare(indexList);

      // update parser/decoder view
      updateParserModel(indexList);

      // get selected time range
      getSelectedRange(indexList, startTime, endTime);

      // show parser view
      ui->parserWidget->show();

      // update inspect dialog if is already open
      if (inspectDialog->isVisible())
         updateInspectDialog(indexList.first());

      // block signals
      QSignalBlocker logicViewBlocker(ui->logicView);
      QSignalBlocker radioViewBlocker(ui->radioView);
      QSignalBlocker framesViewBlocker(ui->framesView);

      // select frames in logic view
      ui->logicView->select(startTime, endTime);
      ui->logicView->repaint();

      // select frames in signal view
      ui->radioView->select(startTime, endTime);
      ui->radioView->repaint();

      // select frames in channels view
      ui->framesView->select(startTime, endTime);
      ui->framesView->repaint();

      // update interface
      updateActions();
   }

   void decoderScrollChanged(int value)
   {
      setFollowEnabled(ui->decodeView->isLastRowVisible());
   }

   void decoderSortChanged(int section, Qt::SortOrder order)
   {
      clearSelection();
   }

   void framesToggleProtocol(FramesWidget::Protocol proto, bool enabled)
   {
      switch (proto)
      {
         case FramesWidget::NfcA:
            setProtocolNfcAEnabled(enabled);
            break;

         case FramesWidget::NfcB:
            setProtocolNfcBEnabled(enabled);
            break;

         case FramesWidget::NfcF:
            setProtocolNfcFEnabled(enabled);
            break;

         case FramesWidget::NfcV:
            setProtocolNfcVEnabled(enabled);
            break;

         case FramesWidget::ISO7816:
            setProtocolISO7816Enabled(enabled);
            break;
      }
   }

   void framesSelectionChanged(double from, double to)
   {
      QSignalBlocker logicViewBlocker(ui->logicView);
      QSignalBlocker radioViewBlocker(ui->radioView);
      QSignalBlocker decodeViewBlocker(ui->decodeView->selectionModel());

      selectedTimeFrom = from;
      selectedTimeTo = to;

      ui->logicView->select(from, to);
      ui->logicView->repaint();

      ui->radioView->select(from, to);
      ui->radioView->repaint();

      ui->decodeView->select(from, to);
      ui->decodeView->repaint();

      // update interface
      updateActions();
      updateParserView();
   }

   void framesRangeChanged(double from, double to) const
   {
      QSignalBlocker block(ui->framesScroll);

      double range = to - from;
      double size = ui->framesView->dataSizeRange();

      if (size > 0)
      {
         double value = (from - ui->framesView->dataLowerRange()) / size;
         double step = (range / size);

         ui->framesScroll->setPageStep(qRound(step * 1000));
         ui->framesScroll->setMaximum(1000 - ui->framesScroll->pageStep());
         ui->framesScroll->setValue(qRound(value * 1000));
      }
      else
      {
         qWarning() << "Channels invalid data range size: " << size;
      }
   }

   void framesScrollChanged(int value) const
   {
      QSignalBlocker block(ui->framesView);

      double length = ui->framesView->dataSizeRange();
      double from = ui->framesView->dataLowerRange() + length * (double(value) / 1000.0f);
      double to = from + length * (double(ui->framesScroll->pageStep()) / 1000.0f);

      ui->framesView->setViewRange(from, to);
   }

   void logicSelectionChanged(double from, double to)
   {
      // single point selection, trigger selection and enable propagation
      QSignalBlocker radioViewBlocker(ui->radioView);
      QSignalBlocker framesViewBlocker(ui->framesView);
      QSignalBlocker decodeViewBlocker(ui->decodeView->selectionModel());

      selectedTimeFrom = from;
      selectedTimeTo = to;

      ui->radioView->select(from, to);
      ui->radioView->repaint();

      ui->decodeView->select(from, to);
      ui->decodeView->repaint();

      ui->framesView->select(from, to);
      ui->framesView->repaint();

      // update interface
      updateActions();
      updateParserView();
   }

   void logicRangeChanged(double from, double to) const
   {
      QSignalBlocker radioViewBlocker(ui->radioView);
      QSignalBlocker signalScrollBlocker(ui->signalScroll);

      double range = to - from;
      double size = ui->logicView->dataSizeRange();

      if (size > 0)
      {
         // update scroll bar
         double value = (from - ui->logicView->dataLowerRange()) / size;
         double step = (range / size);

         ui->signalScroll->setPageStep(qRound(step * 1000));
         ui->signalScroll->setMaximum(1000 - ui->signalScroll->pageStep());
         ui->signalScroll->setValue(qRound(value * 1000));

         // sync radio view
         ui->radioView->setViewRange(from, to);
      }
      else
      {
         qWarning() << "Signal invalid data range size: " << size;
      }

      updateActions();
   }

   void radioSelectionChanged(double from, double to)
   {
      // single point selection, trigger selection and enable propagation
      QSignalBlocker logicViewBlocker(ui->logicView);
      QSignalBlocker framesViewBlocker(ui->framesView);
      QSignalBlocker decodeViewBlocker(ui->decodeView->selectionModel());

      selectedTimeFrom = from;
      selectedTimeTo = to;

      ui->logicView->select(from, to);
      ui->logicView->repaint();

      ui->decodeView->select(from, to);
      ui->decodeView->repaint();

      ui->framesView->select(from, to);
      ui->framesView->repaint();

      // update interface
      updateActions();
      updateParserView();
   }

   void radioRangeChanged(double from, double to) const
   {
      QSignalBlocker signalScrollBlocker(ui->signalScroll);
      QSignalBlocker logicViewBlocker(ui->logicView);

      double range = to - from;
      double size = ui->radioView->dataSizeRange();

      if (size > 0)
      {
         // update scroll bar
         double value = (from - ui->radioView->dataLowerRange()) / size;
         double step = (range / size);

         ui->signalScroll->setPageStep(qRound(step * 1000));
         ui->signalScroll->setMaximum(1000 - ui->signalScroll->pageStep());
         ui->signalScroll->setValue(qRound(value * 1000));

         // sync logic view
         ui->logicView->setViewRange(from, to);
      }
      else
      {
         qWarning() << "Signal invalid data range size: " << size;
      }

      updateActions();
   }

   void signalScrollChanged(int value) const
   {
      QSignalBlocker logicViewBlocker(ui->logicView);
      QSignalBlocker radioViewBlocker(ui->radioView);

      double radioLength = ui->radioView->dataSizeRange();
      double radioFrom = ui->radioView->dataLowerRange() + radioLength * (static_cast<double>(value) / 1000.0f);
      double radioTo = radioFrom + radioLength * (static_cast<double>(ui->signalScroll->pageStep()) / 1000.0f);

      double logicLength = ui->logicView->dataSizeRange();
      double logicFrom = ui->logicView->dataLowerRange() + logicLength * (static_cast<double>(value) / 1000.0f);
      double logicTo = logicFrom + logicLength * (static_cast<double>(ui->signalScroll->pageStep()) / 1000.0f);

      ui->logicView->setViewRange(logicFrom, logicTo);
      ui->radioView->setViewRange(radioFrom, radioTo);
   }

   void parserSelectionChanged() const
   {
      QModelIndexList indexList = ui->parserView->selectionModel()->selectedIndexes();

      if (indexList.isEmpty())
         return;

      auto firstIndex = indexList.first();

      if (auto firstEntry = parserModel->entry(firstIndex))
      {
         ui->dataView->setData(toByteArray(firstEntry->frame()));
         ui->dataView->setSelection(firstEntry->rangeStart(), firstEntry->rangeEnd());
      }
   }

   // utility functions
   void getSelectedRange(QModelIndexList indexList, double &outStartTime, double &outEndTime) const
   {
      outStartTime = -1;
      outEndTime = -1;

      for (const QModelIndex &current: indexList)
      {
         if (const lab::RawFrame *frame = streamFilter->frame(current))
         {
            // detect data start / end timing
            if (outStartTime < 0 || frame->timeStart() < outStartTime)
               outStartTime = frame->timeStart();

            if (outEndTime < 0 || frame->timeEnd() > outEndTime)
               outEndTime = frame->timeEnd();
         }
      }
   }

   static QByteArray toByteArray(const lab::RawFrame &frame)
   {
      QByteArray data;

      for (int i = 0; i < frame.limit(); i++)
      {
         data.append(frame[i]);
      }

      return data;
   }

   void readSettings()
   {
      settings.beginGroup("window");

      QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();

      Qt::WindowState windowState = static_cast<Qt::WindowState>(settings.value("windowState", Qt::WindowState::WindowNoState).toInt());

      window->setWindowState(windowState);

      if (!(windowState & Qt::WindowMaximized))
      {
         int windowWidth = settings.value("windowWidth", DEFAULT_WINDOW_WIDTH).toInt();
         int windowHeight = settings.value("windowHeight", DEFAULT_WINDOW_HEIGHT).toInt();
         int windowTop = (screenGeometry.height() - windowHeight) / 2;
         int windowLeft = (screenGeometry.width() - windowWidth) / 2;

         QRect windowGeometry;

         windowGeometry.setTop(std::clamp(windowTop, screenGeometry.top(), screenGeometry.bottom()));
         windowGeometry.setLeft(std::clamp(windowLeft, screenGeometry.left(), screenGeometry.right()));
         windowGeometry.setBottom(std::clamp(windowTop + windowHeight, screenGeometry.top(), screenGeometry.bottom()));
         windowGeometry.setRight(std::clamp(windowLeft + windowWidth, screenGeometry.left(), screenGeometry.right()));

         window->setGeometry(windowGeometry);
      }

      // restore interface preferences
      setTimeFormat(settings.value("timeFormat", false).toBool());
      setFollowEnabled(settings.value("followEnabled", true).toBool());
      setFilterEnabled(settings.value("filterEnabled", true).toBool());

      settings.endGroup();
   }

   void writeSettings()
   {
      settings.beginGroup("window");
      settings.setValue("timeFormat", ui->actionTime->isChecked());
      settings.setValue("followEnabled", ui->actionFollow->isChecked());
      settings.setValue("filterEnabled", ui->actionFilter->isChecked());
      settings.setValue("windowWidth", window->geometry().width());
      settings.setValue("windowHeight", window->geometry().height());
      settings.setValue("windowState", (int)window->windowState());
      settings.endGroup();
   }

   bool userReallyWantsToQuit() const
   {
      if (settings.value("settings/quitConfirmation", true).toBool())
         return Theme::messageDialog(window, tr("Confirmation"), tr("Do you want to quit?"), QMessageBox::Question, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

      return true;
   }

   bool isActive(QAction *action) const
   {
      return action->isEnabled() && action->isChecked();
   }
};

QtWindow::QtWindow(QtStorage *storage) : impl(new Impl(storage, this))
{
   // configure window properties
   setAttribute(Qt::WA_OpaquePaintEvent, true);
   setAttribute(Qt::WA_DontCreateNativeAncestors, true);
   setAttribute(Qt::WA_NativeWindow, true);
   setAttribute(Qt::WA_NoSystemBackground, true);
   setAutoFillBackground(false);

#ifdef WIN32
   setAttribute(Qt::WA_PaintOnScreen, true);
#endif

   impl->setupUi();

   // update window size
   impl->readSettings();
}

void QtWindow::clearView()
{
   if (Theme::messageDialog(this, tr("Confirmation"), tr("Do you want to remove all events?"), QMessageBox::Question, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
      return;

   impl->clearView();
}

void QtWindow::resetView()
{
   impl->resetView();
}

void QtWindow::openFile()
{
   impl->openFile();
}

void QtWindow::saveFile()
{
   impl->saveFile();
}

void QtWindow::openConfig()
{
   impl->openConfig();
}

void QtWindow::openStorage()
{
   impl->openStorage();
}

void QtWindow::toggleListen()
{
   impl->toggleStart(false);
}

void QtWindow::toggleRecord()
{
   impl->toggleStart(true);
}

void QtWindow::togglePause()
{
   impl->togglePause();
}

void QtWindow::toggleStop()
{
   impl->toggleStop();
}

void QtWindow::toggleTime()
{
   impl->toggleTime();
}

void QtWindow::toggleFollow()
{
   impl->toggleFollow();
}

void QtWindow::toggleFilter()
{
   impl->toggleFilter();
}

void QtWindow::toggleFeature()
{
   impl->toggleFeature(dynamic_cast<QAction *>(this->sender()));
}

void QtWindow::toggleProtocol()
{
   impl->toggleProtocol(dynamic_cast<QAction *>(this->sender()));
}

void QtWindow::showAboutInfo()
{
   impl->showAboutInfo();
}

void QtWindow::showHelpInfo()
{
   impl->showHelpInfo();
}

void QtWindow::changeGainValue(int index)
{
   impl->changeGainValue(index);
}

void QtWindow::trackGainValue(int index)
{
   impl->trackGainValue(index);
}

void QtWindow::zoomReset()
{
   impl->zoomReset();
}

void QtWindow::zoomSelection()
{
   impl->zoomSelection();
}

void QtWindow::keyPressEvent(QKeyEvent *event)
{
   // key press with control modifier
   if (event->modifiers() & Qt::ControlModifier)
   {
      if (event->key() == Qt::Key_C)
      {
         impl->clipboardCopy();
         return;
      }
   }

   // key press without modifiers
   else
   {
      if (event->key() == Qt::Key_Escape)
      {
         impl->clearSelection();
         return;
      }
   }

   QMainWindow::keyPressEvent(event);
}

void QtWindow::handleEvent(QEvent *event)
{
   if (event->type() == SignalBufferEvent::Type)
      impl->signalBufferEvent(dynamic_cast<SignalBufferEvent *>(event));
   else if (event->type() == StreamFrameEvent::Type)
      impl->streamFrameEvent(dynamic_cast<StreamFrameEvent *>(event));
   else if (event->type() == LogicDecoderStatusEvent::Type)
      impl->logicDecoderStatusEvent(dynamic_cast<LogicDecoderStatusEvent *>(event));
   else if (event->type() == LogicDeviceStatusEvent::Type)
      impl->logicDeviceStatusEvent(dynamic_cast<LogicDeviceStatusEvent *>(event));
   else if (event->type() == RadioDecoderStatusEvent::Type)
      impl->radioDecoderStatusEvent(dynamic_cast<RadioDecoderStatusEvent *>(event));
   else if (event->type() == RadioDeviceStatusEvent::Type)
      impl->radioDeviceStatusEvent(dynamic_cast<RadioDeviceStatusEvent *>(event));
   else if (event->type() == FourierStatusEvent::Type)
      impl->fourierStatusEvent(dynamic_cast<FourierStatusEvent *>(event));
   else if (event->type() == StorageStatusEvent::Type)
      impl->storageStatusEvent(dynamic_cast<StorageStatusEvent *>(event));
   else if (event->type() == ConsoleLogEvent::Type)
      impl->consoleLogEvent(dynamic_cast<ConsoleLogEvent *>(event));
   else if (event->type() == SystemStartupEvent::Type)
      impl->systemStartup(dynamic_cast<SystemStartupEvent *>(event));
   else if (event->type() == SystemShutdownEvent::Type)
      impl->systemShutdown(dynamic_cast<SystemShutdownEvent *>(event));
}

void QtWindow::closeEvent(QCloseEvent *event)
{
   if (impl->userReallyWantsToQuit())
   {
      impl->writeSettings();

      event->accept();
   }
   else
   {
      event->ignore();
   }
}

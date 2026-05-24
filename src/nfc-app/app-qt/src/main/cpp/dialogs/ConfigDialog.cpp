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

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QSettings>
#include <QSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>

#include <rt/Logger.h>

#include <events/DecoderControlEvent.h>

#include "QtApplication.h"
#include "ui_ConfigDialog.h"
#include "ConfigDialog.h"

static const QMap<int, int> ROW_TO_PAGE = {
   {1, 0}, {2, 1}, {4, 2}, {5, 3}, {6, 4},
   {8, 5}, {9, 6}, {11, 7}, {12, 8}
};

static const QStringList LOGGER_LEVELS = {"DEBUG", "INFO", "WARN", "ERROR", "NONE"};

static const QStringList LOGGER_SUBSYSTEMS = {
   "app.main", "app.qt",
   "decoder.NfcA", "decoder.NfcB", "decoder.NfcF", "decoder.NfcV", "decoder.Iso7816",
   "worker.RadioDecoder", "worker.RadioDevice",
   "worker.LogicDecoder", "worker.LogicDevice",
   "worker.FourierProcess",
   "hw.AirspyDevice", "hw.RealtekDevice", "hw.DSLogicDevice",
   "rt.Executor"
};

struct ConfigDialog::Impl
{
   ConfigDialog *dialog = nullptr;

   QSharedPointer<Ui_ConfigDialog> ui;

   // Page 0 — General
   QSpinBox *splashScreen = nullptr;
   QCheckBox *quitConfirmation = nullptr;
   QComboBox *timeFormat = nullptr;
   QCheckBox *followEnabled = nullptr;
   QCheckBox *filterEnabled = nullptr;

   // Page 1 — Features
   QCheckBox *featRadioDevice = nullptr;
   QCheckBox *featLogicDevice = nullptr;
   QCheckBox *featRadioDecode = nullptr;
   QCheckBox *featLogicDecode = nullptr;
   QCheckBox *featRadioSpectrum = nullptr;
   QCheckBox *featSignalRecord = nullptr;

   // Page 2 — AirSpy
   QCheckBox *airspyEnabled = nullptr;
   QSpinBox *airspyCenterFreq = nullptr;
   QSpinBox *airspySampleRate = nullptr;
   QComboBox *airspyGainMode = nullptr;
   QSpinBox *airspyGainValue = nullptr;
   QCheckBox *airspyMixerAgc = nullptr;
   QCheckBox *airspyTunerAgc = nullptr;
   QCheckBox *airspyBiasTee = nullptr;

   // Page 3 — HydraSDR
   QCheckBox *hydrasdrEnabled = nullptr;
   QSpinBox *hydrasdrCenterFreq = nullptr;
   QSpinBox *hydrasdrSampleRate = nullptr;
   QComboBox *hydrasdrGainMode = nullptr;
   QSpinBox *hydrasdrGainValue = nullptr;
   QCheckBox *hydrasdrMixerAgc = nullptr;
   QCheckBox *hydrasdrTunerAgc = nullptr;
   QCheckBox *hydrasdrBiasTee = nullptr;

   // Page 4 — RTL-SDR
   QCheckBox *rtlsdrEnabled = nullptr;
   QSpinBox *rtlsdrCenterFreq = nullptr;
   QSpinBox *rtlsdrSampleRate = nullptr;
   QComboBox *rtlsdrGainMode = nullptr;
   QSpinBox *rtlsdrGainValue = nullptr;
   QCheckBox *rtlsdrMixerAgc = nullptr;
   QCheckBox *rtlsdrTunerAgc = nullptr;
   QComboBox *rtlsdrDirectSampling = nullptr;

   // Page 5 — Radio NFC
   QCheckBox *radioEnabled = nullptr;
   QCheckBox *radioNfcA = nullptr;
   QCheckBox *radioNfcB = nullptr;
   QCheckBox *radioNfcF = nullptr;
   QCheckBox *radioNfcV = nullptr;

   // Page 6 — Logic / ISO7816
   QCheckBox *logicEnabled = nullptr;
   QCheckBox *logicIso7816 = nullptr;

   // Page 7 — gRPC API
   QSpinBox *grpcPort = nullptr;

   // Page 8 — Logger
   QComboBox *loggerRoot = nullptr;
   QTableWidget *loggerTable = nullptr;

   explicit Impl(ConfigDialog *dialog) : dialog(dialog), ui(new Ui_ConfigDialog())
   {
      setup();
   }

   // ── Helpers ──────────────────────────────────────────────────────────────

   static QWidget *pageWidget(const QString &title, QFormLayout *&form)
   {
      auto *page = new QWidget;
      auto *vbox = new QVBoxLayout(page);
      vbox->setContentsMargins(16, 12, 16, 0);
      vbox->setSpacing(6);

      auto *titleLabel = new QLabel(title);
      QFont f = titleLabel->font();
      f.setBold(true);
      f.setPointSize(11);
      titleLabel->setFont(f);
      vbox->addWidget(titleLabel);

      auto *line = new QFrame;
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);
      vbox->addWidget(line);

      form = new QFormLayout;
      form->setSpacing(6);
      form->setContentsMargins(0, 4, 0, 0);
      form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
      vbox->addLayout(form);
      vbox->addStretch();

      return page;
   }

   static QListWidgetItem *headerItem(const QString &text)
   {
      auto *item = new QListWidgetItem(text.toUpper());
      item->setFlags(Qt::NoItemFlags);
      QFont f = item->font();
      f.setPointSize(8);
      item->setFont(f);
      item->setForeground(QColor("#5a5a7a"));
      return item;
   }

   static QListWidgetItem *sectionItem(const QString &text)
   {
      auto *item = new QListWidgetItem(text);
      item->setForeground(QColor("#888888"));
      return item;
   }

   static QComboBox *gainModeCombo()
   {
      auto *cb = new QComboBox;
      cb->addItem("Sensitivity", 0);
      cb->addItem("Linearity", 1);
      return cb;
   }

   // ── Setup ─────────────────────────────────────────────────────────────────

   void setup()
   {
      ui->setupUi(dialog);
      dialog->setFixedSize(640, 460);

      applyNavStyle();
      buildNav();
      buildPages();
      connectSignals();
      loadSettings();
   }

   void applyNavStyle()
   {
      ui->navList->setStyleSheet(
         "QListWidget {"
         "  background-color: transparent;"
         "  border: none;"
         "  border-right: 1px solid #383850;"
         "  outline: none;"
         "}"
         "QListWidget::item {"
         "  padding: 5px 12px;"
         "  color: #888888;"
         "}"
         "QListWidget::item:selected {"
         "  background-color: #35355a;"
         "  color: #a8c0ff;"
         "  border-left: 2px solid #6878f0;"
         "  padding-left: 10px;"
         "}"
         "QListWidget::item:disabled {"
         "  padding: 4px 8px 2px;"
         "  color: #5a5a7a;"
         "  background-color: transparent;"
         "}"
      );
   }

   void buildNav()
   {
      auto *nav = ui->navList;

      nav->addItem(headerItem("Settings"));
      nav->addItem(sectionItem(QString::fromUtf8("    ⚙  General")));         // row 1 → page 0
      nav->addItem(sectionItem(QString::fromUtf8("    🔌  Features")));        // row 2 → page 1

      nav->addItem(headerItem("Radio SDR"));
      nav->addItem(sectionItem(QString::fromUtf8("    📡  AirSpy")));        // row 4 → page 2
      nav->addItem(sectionItem(QString::fromUtf8("    📡  HydraSDR")));      // row 5 → page 3
      nav->addItem(sectionItem(QString::fromUtf8("    📡  RTL-SDR")));       // row 6 → page 4

      nav->addItem(headerItem("Decoders"));
      nav->addItem(sectionItem(QString::fromUtf8("    📻  Radio NFC")));       // row 8 → page 5
      nav->addItem(sectionItem(QString::fromUtf8("    💻  Logic / ISO7816"))); // row 9 → page 6

      nav->addItem(headerItem("Advanced"));
      nav->addItem(sectionItem(QString::fromUtf8("    🌐  gRPC API"))); // row 11 → page 7
      nav->addItem(sectionItem(QString::fromUtf8("    📋  Logger")));   // row 12 → page 8

      nav->setCurrentRow(1);
   }

   void buildPages()
   {
      ui->contentStack->addWidget(buildPageGeneral());    // page 0
      ui->contentStack->addWidget(buildPageFeatures());   // page 1
      ui->contentStack->addWidget(buildPageAirSpy());     // page 2
      ui->contentStack->addWidget(buildPageHydraSdr());   // page 3
      ui->contentStack->addWidget(buildPageRtlSdr());     // page 4
      ui->contentStack->addWidget(buildPageRadioNfc());   // page 5
      ui->contentStack->addWidget(buildPageLogic());      // page 6
      ui->contentStack->addWidget(buildPageGrpc());       // page 7
      ui->contentStack->addWidget(buildPageLogger());     // page 8

      ui->contentStack->setCurrentIndex(0);
   }

   QWidget *buildPageGeneral()
   {
      QFormLayout *form;
      QWidget *page = pageWidget("General", form);

      splashScreen = new QSpinBox;
      splashScreen->setRange(0, 9999);
      splashScreen->setSuffix(" ms");
      form->addRow("Splash screen:", splashScreen);

      quitConfirmation = new QCheckBox;
      form->addRow("Confirm on exit:", quitConfirmation);

      timeFormat = new QComboBox;
      timeFormat->addItem("Elapsed", false);
      timeFormat->addItem("Date/Time", true);
      form->addRow("Time format:", timeFormat);

      followEnabled = new QCheckBox;
      form->addRow("Follow last frame:", followEnabled);

      filterEnabled = new QCheckBox;
      form->addRow("Active filter:", filterEnabled);

      return page;
   }

   QWidget *buildPageFeatures()
   {
      QFormLayout *form;
      QWidget *page = pageWidget("Features", form);

      featRadioDevice = new QCheckBox;
      form->addRow("Radio receiver (SDR):", featRadioDevice);

      featLogicDevice = new QCheckBox;
      form->addRow("Logic analyzer:", featLogicDevice);

      featRadioDecode = new QCheckBox;
      form->addRow("NFC radio decoder:", featRadioDecode);

      featLogicDecode = new QCheckBox;
      form->addRow("ISO7816 logic decoder:", featLogicDecode);

      featRadioSpectrum = new QCheckBox;
      form->addRow("Radio spectrum (FFT):", featRadioSpectrum);

      featSignalRecord = new QCheckBox;
      form->addRow("Signal recording:", featSignalRecord);

      auto *note = new QLabel("Feature changes require restarting the interface.");
      note->setWordWrap(true);
      note->setStyleSheet("color: #888; font-style: italic;");
      form->addRow(note);

      return page;
   }

   QWidget *buildPageAirSpy()
   {
      QFormLayout *form;
      QWidget *page = pageWidget("AirSpy", form);

      airspyEnabled = new QCheckBox;
      form->addRow("Enabled:", airspyEnabled);

      airspyCenterFreq = new QSpinBox;
      airspyCenterFreq->setRange(1, 2000000000);
      airspyCenterFreq->setSuffix(" Hz");
      form->addRow("Center frequency:", airspyCenterFreq);

      airspySampleRate = new QSpinBox;
      airspySampleRate->setRange(1, 2000000000);
      airspySampleRate->setSuffix(" Hz");
      form->addRow("Sample rate:", airspySampleRate);

      airspyGainMode = gainModeCombo();
      form->addRow("Gain mode:", airspyGainMode);

      airspyGainValue = new QSpinBox;
      airspyGainValue->setRange(0, 21);
      form->addRow("Gain value:", airspyGainValue);

      airspyMixerAgc = new QCheckBox;
      form->addRow("Mixer AGC:", airspyMixerAgc);

      airspyTunerAgc = new QCheckBox;
      form->addRow("Tuner AGC:", airspyTunerAgc);

      airspyBiasTee = new QCheckBox;
      form->addRow("Bias-Tee:", airspyBiasTee);

      return page;
   }

   QWidget *buildPageHydraSdr()
   {
      QFormLayout *form;
      QWidget *page = pageWidget("HydraSDR", form);

      hydrasdrEnabled = new QCheckBox;
      form->addRow("Enabled:", hydrasdrEnabled);

      hydrasdrCenterFreq = new QSpinBox;
      hydrasdrCenterFreq->setRange(1, 2000000000);
      hydrasdrCenterFreq->setSuffix(" Hz");
      form->addRow("Center frequency:", hydrasdrCenterFreq);

      hydrasdrSampleRate = new QSpinBox;
      hydrasdrSampleRate->setRange(1, 2000000000);
      hydrasdrSampleRate->setSuffix(" Hz");
      form->addRow("Sample rate:", hydrasdrSampleRate);

      hydrasdrGainMode = gainModeCombo();
      form->addRow("Gain mode:", hydrasdrGainMode);

      hydrasdrGainValue = new QSpinBox;
      hydrasdrGainValue->setRange(0, 21);
      form->addRow("Gain value:", hydrasdrGainValue);

      hydrasdrMixerAgc = new QCheckBox;
      form->addRow("Mixer AGC:", hydrasdrMixerAgc);

      hydrasdrTunerAgc = new QCheckBox;
      form->addRow("Tuner AGC:", hydrasdrTunerAgc);

      hydrasdrBiasTee = new QCheckBox;
      form->addRow("Bias-Tee:", hydrasdrBiasTee);

      return page;
   }

   QWidget *buildPageRtlSdr()
   {
      QFormLayout *form;
      QWidget *page = pageWidget("RTL-SDR", form);

      rtlsdrEnabled = new QCheckBox;
      form->addRow("Enabled:", rtlsdrEnabled);

      rtlsdrCenterFreq = new QSpinBox;
      rtlsdrCenterFreq->setRange(1, 2000000000);
      rtlsdrCenterFreq->setSuffix(" Hz");
      form->addRow("Center frequency:", rtlsdrCenterFreq);

      rtlsdrSampleRate = new QSpinBox;
      rtlsdrSampleRate->setRange(1, 2000000000);
      rtlsdrSampleRate->setSuffix(" Hz");
      form->addRow("Sample rate:", rtlsdrSampleRate);

      rtlsdrGainMode = gainModeCombo();
      form->addRow("Gain mode:", rtlsdrGainMode);

      rtlsdrGainValue = new QSpinBox;
      rtlsdrGainValue->setRange(0, 500);
      form->addRow("Gain value:", rtlsdrGainValue);

      rtlsdrMixerAgc = new QCheckBox;
      form->addRow("Mixer AGC:", rtlsdrMixerAgc);

      rtlsdrTunerAgc = new QCheckBox;
      form->addRow("Tuner AGC:", rtlsdrTunerAgc);

      rtlsdrDirectSampling = new QComboBox;
      rtlsdrDirectSampling->addItem("Off", 0);
      rtlsdrDirectSampling->addItem("I-branch", 1);
      rtlsdrDirectSampling->addItem("Q-branch", 2);
      form->addRow("Direct sampling:", rtlsdrDirectSampling);

      return page;
   }

   QWidget *buildPageRadioNfc()
   {
      QFormLayout *form;
      QWidget *page = pageWidget("Radio NFC", form);

      radioEnabled = new QCheckBox;
      form->addRow("Decoder enabled:", radioEnabled);

      radioNfcA = new QCheckBox;
      form->addRow("NFC-A (ISO14443-A):", radioNfcA);

      radioNfcB = new QCheckBox;
      form->addRow("NFC-B (ISO14443-B):", radioNfcB);

      radioNfcF = new QCheckBox;
      form->addRow("NFC-F (ISO18092):", radioNfcF);

      radioNfcV = new QCheckBox;
      form->addRow("NFC-V (ISO15693):", radioNfcV);

      return page;
   }

   QWidget *buildPageLogic()
   {
      QFormLayout *form;
      QWidget *page = pageWidget("Logic / ISO7816", form);

      logicEnabled = new QCheckBox;
      form->addRow("Decoder enabled:", logicEnabled);

      logicIso7816 = new QCheckBox;
      form->addRow("ISO7816 enabled:", logicIso7816);

      auto *note = new QLabel("Channel assignments (IO/CLK/RST/VCC) are fixed and cannot be modified.");
      note->setWordWrap(true);
      note->setStyleSheet("color: #888; font-style: italic;");
      form->addRow(note);

      return page;
   }

   QWidget *buildPageGrpc()
   {
      QFormLayout *form;
      QWidget *page = pageWidget("gRPC API", form);

      grpcPort = new QSpinBox;
      grpcPort->setRange(0, 65535);
      form->addRow("Port:", grpcPort);

      auto *note = new QLabel("0 = disabled. Port changes require restarting the application.");
      note->setWordWrap(true);
      note->setStyleSheet("color: #888; font-style: italic;");
      form->addRow(note);

      return page;
   }

   QWidget *buildPageLogger()
   {
      auto *page = new QWidget;
      auto *vbox = new QVBoxLayout(page);
      vbox->setContentsMargins(16, 12, 16, 0);
      vbox->setSpacing(6);

      auto *titleLabel = new QLabel("Logger");
      QFont f = titleLabel->font();
      f.setBold(true);
      f.setPointSize(11);
      titleLabel->setFont(f);
      vbox->addWidget(titleLabel);

      auto *line = new QFrame;
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);
      vbox->addWidget(line);

      auto *rootLayout = new QHBoxLayout;
      rootLayout->addWidget(new QLabel("Root level:"));
      loggerRoot = new QComboBox;
      for (const auto &lvl : LOGGER_LEVELS)
         loggerRoot->addItem(lvl, lvl);
      loggerRoot->setCurrentText("WARN");
      rootLayout->addWidget(loggerRoot);
      rootLayout->addStretch();
      vbox->addLayout(rootLayout);

      loggerTable = new QTableWidget(LOGGER_SUBSYSTEMS.size(), 2);
      loggerTable->setHorizontalHeaderLabels({"Subsystem", "Level"});
      loggerTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
      loggerTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
      loggerTable->horizontalHeader()->resizeSection(1, 100);
      loggerTable->verticalHeader()->hide();
      loggerTable->setSelectionMode(QAbstractItemView::NoSelection);
      loggerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

      for (int i = 0; i < LOGGER_SUBSYSTEMS.size(); ++i)
      {
         loggerTable->setItem(i, 0, new QTableWidgetItem(LOGGER_SUBSYSTEMS[i]));

         auto *cb = new QComboBox;
         for (const auto &lvl : LOGGER_LEVELS)
            cb->addItem(lvl, lvl);
         cb->setCurrentText("WARN");
         loggerTable->setCellWidget(i, 1, cb);
         loggerTable->setRowHeight(i, 24);
      }

      vbox->addWidget(loggerTable);

      auto *note = new QLabel("Log level changes are applied immediately.");
      note->setStyleSheet("color: #888; font-style: italic;");
      vbox->addWidget(note);

      return page;
   }

   // ── Connect signals ───────────────────────────────────────────────────────

   void connectSignals()
   {
      QObject::connect(ui->navList, &QListWidget::currentRowChanged, dialog, [this](int row) {
         if (ROW_TO_PAGE.contains(row))
            ui->contentStack->setCurrentIndex(ROW_TO_PAGE[row]);
      });

      QObject::connect(ui->cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);
      QObject::connect(ui->applyButton, &QPushButton::clicked, dialog, [this] { applySettings(); });
   }

   // ── Load from QSettings ───────────────────────────────────────────────────

   void loadSettings()
   {
      QSettings s;

      // Page 0 — General
      splashScreen->setValue(s.value("settings/splashScreen", 2500).toInt());
      quitConfirmation->setChecked(s.value("settings/quitConfirmation", true).toBool());
      timeFormat->setCurrentIndex(s.value("window/timeFormat", false).toBool() ? 1 : 0);
      followEnabled->setChecked(s.value("window/followEnabled", true).toBool());
      filterEnabled->setChecked(s.value("window/filterEnabled", true).toBool());

      // Page 1 — Features
      s.beginGroup("features");
      featRadioDevice->setChecked(s.value("radioDevice", true).toBool());
      featLogicDevice->setChecked(s.value("logicDevice", true).toBool());
      featRadioDecode->setChecked(s.value("radioDecode", true).toBool());
      featLogicDecode->setChecked(s.value("logicDecode", true).toBool());
      featRadioSpectrum->setChecked(s.value("radioSpectrum", true).toBool());
      featSignalRecord->setChecked(s.value("signalRecord", true).toBool());
      s.endGroup();

      // Page 2 — AirSpy
      s.beginGroup("device.radio.airspy");
      airspyEnabled->setChecked(s.value("enabled", true).toBool());
      airspyCenterFreq->setValue(s.value("centerFreq", 40680000).toInt());
      airspySampleRate->setValue(s.value("sampleRate", 10000000).toInt());
      airspyGainMode->setCurrentIndex(airspyGainMode->findData(s.value("gainMode", 1).toInt()));
      airspyGainValue->setValue(s.value("gainValue", 4).toInt());
      airspyMixerAgc->setChecked(s.value("mixerAgc", false).toBool());
      airspyTunerAgc->setChecked(s.value("tunerAgc", false).toBool());
      airspyBiasTee->setChecked(s.value("biasTee", false).toBool());
      s.endGroup();

      // Page 3 — HydraSDR
      s.beginGroup("device.radio.hydrasdr");
      hydrasdrEnabled->setChecked(s.value("enabled", true).toBool());
      hydrasdrCenterFreq->setValue(s.value("centerFreq", 40680000).toInt());
      hydrasdrSampleRate->setValue(s.value("sampleRate", 10000000).toInt());
      hydrasdrGainMode->setCurrentIndex(hydrasdrGainMode->findData(s.value("gainMode", 1).toInt()));
      hydrasdrGainValue->setValue(s.value("gainValue", 4).toInt());
      hydrasdrMixerAgc->setChecked(s.value("mixerAgc", false).toBool());
      hydrasdrTunerAgc->setChecked(s.value("tunerAgc", false).toBool());
      hydrasdrBiasTee->setChecked(s.value("biasTee", false).toBool());
      s.endGroup();

      // Page 4 — RTL-SDR
      s.beginGroup("device.radio.rtlsdr");
      rtlsdrEnabled->setChecked(s.value("enabled", false).toBool());
      rtlsdrCenterFreq->setValue(s.value("centerFreq", 27120000).toInt());
      rtlsdrSampleRate->setValue(s.value("sampleRate", 3200000).toInt());
      rtlsdrGainMode->setCurrentIndex(rtlsdrGainMode->findData(s.value("gainMode", 1).toInt()));
      rtlsdrGainValue->setValue(s.value("gainValue", 77).toInt());
      rtlsdrMixerAgc->setChecked(s.value("mixerAgc", false).toBool());
      rtlsdrTunerAgc->setChecked(s.value("tunerAgc", false).toBool());
      rtlsdrDirectSampling->setCurrentIndex(s.value("directSampling", 0).toInt());
      s.endGroup();

      // Page 5 — Radio NFC
      radioEnabled->setChecked(s.value("decoder.radio/enabled", true).toBool());
      radioNfcA->setChecked(s.value("decoder.radio.protocol.nfca/enabled", true).toBool());
      radioNfcB->setChecked(s.value("decoder.radio.protocol.nfcb/enabled", true).toBool());
      radioNfcF->setChecked(s.value("decoder.radio.protocol.nfcf/enabled", true).toBool());
      radioNfcV->setChecked(s.value("decoder.radio.protocol.nfcv/enabled", true).toBool());

      // Page 6 — Logic
      logicEnabled->setChecked(s.value("decoder.logic/enabled", true).toBool());
      logicIso7816->setChecked(s.value("decoder.logic.protocol.iso7816/enabled", true).toBool());

      // Page 7 — gRPC
      grpcPort->setValue(s.value("grpc/port", 0).toInt());

      // Page 8 — Logger
      s.beginGroup("logger");
      loggerRoot->setCurrentText(s.value("root", "WARN").toString().toUpper());
      for (int i = 0; i < LOGGER_SUBSYSTEMS.size(); ++i)
      {
         auto *cb = qobject_cast<QComboBox *>(loggerTable->cellWidget(i, 1));
         if (cb)
            cb->setCurrentText(s.value(LOGGER_SUBSYSTEMS[i], "WARN").toString().toUpper());
      }
      s.endGroup();
   }

   // ── Apply (save + post events) ────────────────────────────────────────────

   void applySettings()
   {
      QSettings s;

      // Page 0 — General
      s.setValue("settings/splashScreen", splashScreen->value());
      s.setValue("settings/quitConfirmation", quitConfirmation->isChecked());
      s.setValue("window/timeFormat", timeFormat->currentData().toBool());
      s.setValue("window/followEnabled", followEnabled->isChecked());
      s.setValue("window/filterEnabled", filterEnabled->isChecked());

      // Page 1 — Features
      bool featuresModified = false;

      s.beginGroup("features");
      auto checkFeat = [&](const QString &key, QCheckBox *cb) {
         bool prev = s.value(key, true).toBool();
         bool now = cb->isChecked();
         s.setValue(key, now);
         if (prev != now)
            featuresModified = true;
      };
      checkFeat("radioDevice", featRadioDevice);
      checkFeat("logicDevice", featLogicDevice);
      checkFeat("radioDecode", featRadioDecode);
      checkFeat("logicDecode", featLogicDecode);
      checkFeat("radioSpectrum", featRadioSpectrum);
      checkFeat("signalRecord", featSignalRecord);
      s.endGroup();

      // Page 2 — AirSpy
      s.beginGroup("device.radio.airspy");
      s.setValue("enabled", airspyEnabled->isChecked());
      s.setValue("centerFreq", airspyCenterFreq->value());
      s.setValue("sampleRate", airspySampleRate->value());
      s.setValue("gainMode", airspyGainMode->currentData().toInt());
      s.setValue("gainValue", airspyGainValue->value());
      s.setValue("mixerAgc", airspyMixerAgc->isChecked());
      s.setValue("tunerAgc", airspyTunerAgc->isChecked());
      s.setValue("biasTee", airspyBiasTee->isChecked());
      s.endGroup();

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDeviceConfig, {
         {"enabled",    airspyEnabled->isChecked()},
         {"centerFreq", airspyCenterFreq->value()},
         {"sampleRate", airspySampleRate->value()},
         {"gainMode",   airspyGainMode->currentData().toInt()},
         {"gainValue",  airspyGainValue->value()},
         {"mixerAgc",   (int) airspyMixerAgc->isChecked()},
         {"tunerAgc",   (int) airspyTunerAgc->isChecked()},
         {"biasTee",    (int) airspyBiasTee->isChecked()}
      }));

      // Page 3 — HydraSDR
      s.beginGroup("device.radio.hydrasdr");
      s.setValue("enabled", hydrasdrEnabled->isChecked());
      s.setValue("centerFreq", hydrasdrCenterFreq->value());
      s.setValue("sampleRate", hydrasdrSampleRate->value());
      s.setValue("gainMode", hydrasdrGainMode->currentData().toInt());
      s.setValue("gainValue", hydrasdrGainValue->value());
      s.setValue("mixerAgc", hydrasdrMixerAgc->isChecked());
      s.setValue("tunerAgc", hydrasdrTunerAgc->isChecked());
      s.setValue("biasTee", hydrasdrBiasTee->isChecked());
      s.endGroup();

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDeviceConfig, {
         {"enabled",    hydrasdrEnabled->isChecked()},
         {"centerFreq", hydrasdrCenterFreq->value()},
         {"sampleRate", hydrasdrSampleRate->value()},
         {"gainMode",   hydrasdrGainMode->currentData().toInt()},
         {"gainValue",  hydrasdrGainValue->value()},
         {"mixerAgc",   (int) hydrasdrMixerAgc->isChecked()},
         {"tunerAgc",   (int) hydrasdrTunerAgc->isChecked()},
         {"biasTee",    (int) hydrasdrBiasTee->isChecked()}
      }));

      // Page 4 — RTL-SDR
      s.beginGroup("device.radio.rtlsdr");
      s.setValue("enabled", rtlsdrEnabled->isChecked());
      s.setValue("centerFreq", rtlsdrCenterFreq->value());
      s.setValue("sampleRate", rtlsdrSampleRate->value());
      s.setValue("gainMode", rtlsdrGainMode->currentData().toInt());
      s.setValue("gainValue", rtlsdrGainValue->value());
      s.setValue("mixerAgc", rtlsdrMixerAgc->isChecked());
      s.setValue("tunerAgc", rtlsdrTunerAgc->isChecked());
      s.setValue("directSampling", rtlsdrDirectSampling->currentIndex());
      s.endGroup();

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDeviceConfig, {
         {"enabled",        rtlsdrEnabled->isChecked()},
         {"centerFreq",     rtlsdrCenterFreq->value()},
         {"sampleRate",     rtlsdrSampleRate->value()},
         {"gainMode",       rtlsdrGainMode->currentData().toInt()},
         {"gainValue",      rtlsdrGainValue->value()},
         {"mixerAgc",       (int) rtlsdrMixerAgc->isChecked()},
         {"tunerAgc",       (int) rtlsdrTunerAgc->isChecked()},
         {"directSampling", rtlsdrDirectSampling->currentIndex()}
      }));

      // Page 5 — Radio NFC
      s.setValue("decoder.radio/enabled", radioEnabled->isChecked());
      s.setValue("decoder.radio.protocol.nfca/enabled", radioNfcA->isChecked());
      s.setValue("decoder.radio.protocol.nfcb/enabled", radioNfcB->isChecked());
      s.setValue("decoder.radio.protocol.nfcf/enabled", radioNfcF->isChecked());
      s.setValue("decoder.radio.protocol.nfcv/enabled", radioNfcV->isChecked());

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDecoderConfig, {
         {"enabled",               radioEnabled->isChecked()},
         {"protocol/nfca/enabled", radioNfcA->isChecked()},
         {"protocol/nfcb/enabled", radioNfcB->isChecked()},
         {"protocol/nfcf/enabled", radioNfcF->isChecked()},
         {"protocol/nfcv/enabled", radioNfcV->isChecked()}
      }));

      // Page 6 — Logic / ISO7816
      s.setValue("decoder.logic/enabled", logicEnabled->isChecked());
      s.setValue("decoder.logic.protocol.iso7816/enabled", logicIso7816->isChecked());

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::LogicDecoderConfig, {
         {"enabled",                  logicEnabled->isChecked()},
         {"protocol/iso7816/enabled", logicIso7816->isChecked()}
      }));

      // Page 7 — gRPC
      s.setValue("grpc/port", grpcPort->value());

      // Page 8 — Logger
      s.beginGroup("logger");
      const QString rootLevel = loggerRoot->currentText();
      s.setValue("root", rootLevel);
      rt::Logger::setRootLevel(rootLevel.toStdString());

      for (int i = 0; i < LOGGER_SUBSYSTEMS.size(); ++i)
      {
         auto *cb = qobject_cast<QComboBox *>(loggerTable->cellWidget(i, 1));
         if (!cb)
            continue;
         const QString level = cb->currentText();
         s.setValue(LOGGER_SUBSYSTEMS[i], level);
         rt::Logger::getLogger(LOGGER_SUBSYSTEMS[i].toStdString())->setLevel(level.toStdString());
      }
      s.endGroup();

      s.sync();

      if (featuresModified)
         emit dialog->featuresChanged();

      dialog->accept();
   }
};

ConfigDialog::ConfigDialog(QWidget *parent) : QDialog(parent), impl(new Impl(this))
{
}

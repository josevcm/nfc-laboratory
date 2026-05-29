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
#include <QHeaderView>
#include <QListWidgetItem>
#include <QSettings>
#include <QSpinBox>
#include <QTableWidget>

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

   explicit Impl(ConfigDialog *dialog) : dialog(dialog), ui(new Ui_ConfigDialog())
   {
      setup();
   }

   void setup()
   {
      ui->setupUi(dialog);

      fillCategories();
      fillGainModes();
      fillLoggers();

      connectSignals();

      loadSettings();
   }

   void fillCategories()
   {
      ui->navList->addItem(headerItem("Settings"));
      ui->navList->addItem(sectionItem(QString::fromUtf8("    \xe2\x9a\x99  General")));
      ui->navList->addItem(sectionItem(QString::fromUtf8("    \xf0\x9f\x94\x8c  Features")));

      ui->navList->addItem(headerItem("Radio SDR"));
      ui->navList->addItem(sectionItem(QString::fromUtf8("    \xf0\x9f\x93\xa1  AirSpy")));
      ui->navList->addItem(sectionItem(QString::fromUtf8("    \xf0\x9f\x93\xa1  HydraSDR")));
      ui->navList->addItem(sectionItem(QString::fromUtf8("    \xf0\x9f\x93\xa1  RTL-SDR")));

      ui->navList->addItem(headerItem("Decoders"));
      ui->navList->addItem(sectionItem(QString::fromUtf8("    \xf0\x9f\x93\xbb  Radio NFC")));
      ui->navList->addItem(sectionItem(QString::fromUtf8("    \xf0\x9f\x92\xbb  Logic / ISO7816")));

      ui->navList->addItem(headerItem("Advanced"));
      ui->navList->addItem(sectionItem(QString::fromUtf8("    \xf0\x9f\x8c\x90  gRPC API")));
      ui->navList->addItem(sectionItem(QString::fromUtf8("    \xf0\x9f\x93\x8b  Logger")));

      ui->navList->setCurrentRow(1);
   }

   void fillGainModes()
   {
      for (auto *cb: {ui->airspyGainMode, ui->hydrasdrGainMode, ui->rtlsdrGainMode})
      {
         cb->addItem("Sensitivity", 0);
         cb->addItem("Linearity", 1);
      }
   }

   void fillLoggers()
   {
      for (const auto &lvl: LOGGER_LEVELS)
         ui->loggerRoot->addItem(lvl, lvl);
      ui->loggerRoot->setCurrentText("WARN");

      ui->loggerTable->setColumnCount(2);
      ui->loggerTable->setHorizontalHeaderLabels({"Subsystem", "Level"});
      ui->loggerTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
      ui->loggerTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
      ui->loggerTable->horizontalHeader()->resizeSection(1, 100);
      ui->loggerTable->verticalHeader()->hide();
      ui->loggerTable->setSelectionMode(QAbstractItemView::NoSelection);
      ui->loggerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
      ui->loggerTable->setRowCount(LOGGER_SUBSYSTEMS.size());

      for (int i = 0; i < LOGGER_SUBSYSTEMS.size(); ++i)
      {
         ui->loggerTable->setItem(i, 0, new QTableWidgetItem(LOGGER_SUBSYSTEMS[i]));

         auto *cb = new QComboBox;

         for (const auto &lvl: LOGGER_LEVELS)
            cb->addItem(lvl, lvl);

         cb->setCurrentText("WARN");

         ui->loggerTable->setCellWidget(i, 1, cb);
         ui->loggerTable->setRowHeight(i, 24);
      }
   }

   // ── Nav helpers ───────────────────────────────────────────────────────────

   static QListWidgetItem *headerItem(const QString &text)
   {
      auto *item = new QListWidgetItem(text.toUpper());
      item->setFlags(Qt::NoItemFlags);
      item->setSizeHint(QSize(0, 24));

      QFont font = item->font();
      font.setPointSize(8);
      item->setFont(font);
      item->setForeground(QColor("#5a5a7a"));

      return item;
   }

   static QListWidgetItem *sectionItem(const QString &text)
   {
      auto *item = new QListWidgetItem(text);
      item->setSizeHint(QSize(0, 30));
      item->setForeground(QColor("#888888"));
      return item;
   }

   void connectSignals()
   {
      connect(ui->navList, &QListWidget::currentRowChanged, dialog, [this](int row) {
         if (ROW_TO_PAGE.contains(row))
            ui->contentStack->setCurrentIndex(ROW_TO_PAGE[row]);
      });

      connect(ui->cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);
      connect(ui->applyButton, &QPushButton::clicked, dialog, [this] { applySettings(); });
   }

   void loadSettings()
   {
      QSettings s;

      // Page 0 — General
      ui->splashScreen->setValue(s.value("settings/splashScreen", 2500).toInt());
      ui->quitConfirmation->setChecked(s.value("settings/quitConfirmation", true).toBool());

      // Page 1 — Features
      s.beginGroup("features");
      ui->featRadioDevice->setChecked(s.value("radioDevice", true).toBool());
      ui->featLogicDevice->setChecked(s.value("logicDevice", true).toBool());
      ui->featRadioDecode->setChecked(s.value("radioDecode", true).toBool());
      ui->featLogicDecode->setChecked(s.value("logicDecode", true).toBool());
      ui->featRadioSpectrum->setChecked(s.value("radioSpectrum", true).toBool());
      ui->featSignalRecord->setChecked(s.value("signalRecord", true).toBool());
      s.endGroup();

      // Page 2 — AirSpy
      s.beginGroup("device.radio.airspy");
      ui->airspyEnabled->setChecked(s.value("enabled", true).toBool());
      ui->airspyCenterFreq->setValue(s.value("centerFreq", 40680000).toInt());
      ui->airspySampleRate->setValue(s.value("sampleRate", 10000000).toInt());
      ui->airspyGainMode->setCurrentIndex(ui->airspyGainMode->findData(s.value("gainMode", 1).toInt()));
      ui->airspyGainValue->setValue(s.value("gainValue", 4).toInt());
      ui->airspyMixerAgc->setChecked(s.value("mixerAgc", false).toBool());
      ui->airspyTunerAgc->setChecked(s.value("tunerAgc", false).toBool());
      ui->airspyBiasTee->setChecked(s.value("biasTee", false).toBool());
      s.endGroup();

      // Page 3 — HydraSDR
      s.beginGroup("device.radio.hydrasdr");
      ui->hydrasdrEnabled->setChecked(s.value("enabled", true).toBool());
      ui->hydrasdrCenterFreq->setValue(s.value("centerFreq", 40680000).toInt());
      ui->hydrasdrSampleRate->setValue(s.value("sampleRate", 10000000).toInt());
      ui->hydrasdrGainMode->setCurrentIndex(ui->hydrasdrGainMode->findData(s.value("gainMode", 1).toInt()));
      ui->hydrasdrGainValue->setValue(s.value("gainValue", 4).toInt());
      ui->hydrasdrMixerAgc->setChecked(s.value("mixerAgc", false).toBool());
      ui->hydrasdrTunerAgc->setChecked(s.value("tunerAgc", false).toBool());
      ui->hydrasdrBiasTee->setChecked(s.value("biasTee", false).toBool());
      s.endGroup();

      // Page 4 — RTL-SDR
      s.beginGroup("device.radio.rtlsdr");
      ui->rtlsdrEnabled->setChecked(s.value("enabled", false).toBool());
      ui->rtlsdrCenterFreq->setValue(s.value("centerFreq", 27120000).toInt());
      ui->rtlsdrSampleRate->setValue(s.value("sampleRate", 3200000).toInt());
      ui->rtlsdrGainMode->setCurrentIndex(ui->rtlsdrGainMode->findData(s.value("gainMode", 1).toInt()));
      ui->rtlsdrGainValue->setValue(s.value("gainValue", 77).toInt());
      ui->rtlsdrMixerAgc->setChecked(s.value("mixerAgc", false).toBool());
      ui->rtlsdrTunerAgc->setChecked(s.value("tunerAgc", false).toBool());
      ui->rtlsdrDirectSampling->setCurrentIndex(s.value("directSampling", 0).toInt());
      s.endGroup();

      // Page 5 — Radio NFC
      ui->radioEnabled->setChecked(s.value("decoder.radio/enabled", true).toBool());
      ui->radioNfcA->setChecked(s.value("decoder.radio.protocol.nfca/enabled", true).toBool());
      ui->radioNfcB->setChecked(s.value("decoder.radio.protocol.nfcb/enabled", true).toBool());
      ui->radioNfcF->setChecked(s.value("decoder.radio.protocol.nfcf/enabled", true).toBool());
      ui->radioNfcV->setChecked(s.value("decoder.radio.protocol.nfcv/enabled", true).toBool());

      // Page 6 — Logic
      ui->logicEnabled->setChecked(s.value("decoder.logic/enabled", true).toBool());
      ui->logicIso7816->setChecked(s.value("decoder.logic.protocol.iso7816/enabled", true).toBool());

      // Page 7 — gRPC
      ui->grpcPort->setValue(s.value("grpc/port", 0).toInt());

      // Page 8 — Logger
      s.beginGroup("logger");

      ui->loggerRoot->setCurrentText(s.value("root", "WARN").toString().toUpper());

      for (int i = 0; i < LOGGER_SUBSYSTEMS.size(); ++i)
      {
         if (auto *cb = qobject_cast<QComboBox *>(ui->loggerTable->cellWidget(i, 1)))
            cb->setCurrentText(s.value(LOGGER_SUBSYSTEMS[i], "WARN").toString().toUpper());
      }

      s.endGroup();
   }

   void applySettings()
   {
      QSettings s;

      // Page 0 — General
      s.setValue("settings/splashScreen", ui->splashScreen->value());
      s.setValue("settings/quitConfirmation", ui->quitConfirmation->isChecked());

      // Page 1 — Features
      bool featuresModified = false;
      s.beginGroup("features");

      auto checkFeat = [&](const QString &key, const QCheckBox *cb) {
         bool prev = s.value(key, true).toBool();
         bool now = cb->isChecked();
         s.setValue(key, now);
         if (prev != now) featuresModified = true;
      };

      checkFeat("radioDevice", ui->featRadioDevice);
      checkFeat("logicDevice", ui->featLogicDevice);
      checkFeat("radioDecode", ui->featRadioDecode);
      checkFeat("logicDecode", ui->featLogicDecode);
      checkFeat("radioSpectrum", ui->featRadioSpectrum);
      checkFeat("signalRecord", ui->featSignalRecord);
      s.endGroup();

      // Page 2 — AirSpy
      s.beginGroup("device.radio.airspy");
      s.setValue("enabled", ui->airspyEnabled->isChecked());
      s.setValue("centerFreq", ui->airspyCenterFreq->value());
      s.setValue("sampleRate", ui->airspySampleRate->value());
      s.setValue("gainMode", ui->airspyGainMode->currentData().toInt());
      s.setValue("gainValue", ui->airspyGainValue->value());
      s.setValue("mixerAgc", ui->airspyMixerAgc->isChecked());
      s.setValue("tunerAgc", ui->airspyTunerAgc->isChecked());
      s.setValue("biasTee", ui->airspyBiasTee->isChecked());
      s.endGroup();

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDeviceConfig, {
                                                     {"enabled", ui->airspyEnabled->isChecked()},
                                                     {"centerFreq", ui->airspyCenterFreq->value()},
                                                     {"sampleRate", ui->airspySampleRate->value()},
                                                     {"gainMode", ui->airspyGainMode->currentData().toInt()},
                                                     {"gainValue", ui->airspyGainValue->value()},
                                                     {"mixerAgc", static_cast<int>(ui->airspyMixerAgc->isChecked())},
                                                     {"tunerAgc", static_cast<int>(ui->airspyTunerAgc->isChecked())},
                                                     {"biasTee", static_cast<int>(ui->airspyBiasTee->isChecked())}
                                                  }));

      // Page 3 — HydraSDR
      s.beginGroup("device.radio.hydrasdr");
      s.setValue("enabled", ui->hydrasdrEnabled->isChecked());
      s.setValue("centerFreq", ui->hydrasdrCenterFreq->value());
      s.setValue("sampleRate", ui->hydrasdrSampleRate->value());
      s.setValue("gainMode", ui->hydrasdrGainMode->currentData().toInt());
      s.setValue("gainValue", ui->hydrasdrGainValue->value());
      s.setValue("mixerAgc", ui->hydrasdrMixerAgc->isChecked());
      s.setValue("tunerAgc", ui->hydrasdrTunerAgc->isChecked());
      s.setValue("biasTee", ui->hydrasdrBiasTee->isChecked());
      s.endGroup();

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDeviceConfig, {
                                                     {"enabled", ui->hydrasdrEnabled->isChecked()},
                                                     {"centerFreq", ui->hydrasdrCenterFreq->value()},
                                                     {"sampleRate", ui->hydrasdrSampleRate->value()},
                                                     {"gainMode", ui->hydrasdrGainMode->currentData().toInt()},
                                                     {"gainValue", ui->hydrasdrGainValue->value()},
                                                     {"mixerAgc", static_cast<int>(ui->hydrasdrMixerAgc->isChecked())},
                                                     {"tunerAgc", static_cast<int>(ui->hydrasdrTunerAgc->isChecked())},
                                                     {"biasTee", static_cast<int>(ui->hydrasdrBiasTee->isChecked())}
                                                  }));

      // Page 4 — RTL-SDR
      s.beginGroup("device.radio.rtlsdr");
      s.setValue("enabled", ui->rtlsdrEnabled->isChecked());
      s.setValue("centerFreq", ui->rtlsdrCenterFreq->value());
      s.setValue("sampleRate", ui->rtlsdrSampleRate->value());
      s.setValue("gainMode", ui->rtlsdrGainMode->currentData().toInt());
      s.setValue("gainValue", ui->rtlsdrGainValue->value());
      s.setValue("mixerAgc", ui->rtlsdrMixerAgc->isChecked());
      s.setValue("tunerAgc", ui->rtlsdrTunerAgc->isChecked());
      s.setValue("directSampling", ui->rtlsdrDirectSampling->currentIndex());
      s.endGroup();

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDeviceConfig, {
                                                     {"enabled", ui->rtlsdrEnabled->isChecked()},
                                                     {"centerFreq", ui->rtlsdrCenterFreq->value()},
                                                     {"sampleRate", ui->rtlsdrSampleRate->value()},
                                                     {"gainMode", ui->rtlsdrGainMode->currentData().toInt()},
                                                     {"gainValue", ui->rtlsdrGainValue->value()},
                                                     {"mixerAgc", static_cast<int>(ui->rtlsdrMixerAgc->isChecked())},
                                                     {"tunerAgc", static_cast<int>(ui->rtlsdrTunerAgc->isChecked())},
                                                     {"directSampling", ui->rtlsdrDirectSampling->currentIndex()}
                                                  }));

      // Page 5 — Radio NFC
      s.setValue("decoder.radio/enabled", ui->radioEnabled->isChecked());
      s.setValue("decoder.radio.protocol.nfca/enabled", ui->radioNfcA->isChecked());
      s.setValue("decoder.radio.protocol.nfcb/enabled", ui->radioNfcB->isChecked());
      s.setValue("decoder.radio.protocol.nfcf/enabled", ui->radioNfcF->isChecked());
      s.setValue("decoder.radio.protocol.nfcv/enabled", ui->radioNfcV->isChecked());

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::RadioDecoderConfig, {
                                                     {"enabled", ui->radioEnabled->isChecked()},
                                                     {"protocol/nfca/enabled", ui->radioNfcA->isChecked()},
                                                     {"protocol/nfcb/enabled", ui->radioNfcB->isChecked()},
                                                     {"protocol/nfcf/enabled", ui->radioNfcF->isChecked()},
                                                     {"protocol/nfcv/enabled", ui->radioNfcV->isChecked()}
                                                  }));

      // Page 6 — Logic / ISO7816
      s.setValue("decoder.logic/enabled", ui->logicEnabled->isChecked());
      s.setValue("decoder.logic.protocol.iso7816/enabled", ui->logicIso7816->isChecked());

      QtApplication::post(new DecoderControlEvent(DecoderControlEvent::LogicDecoderConfig, {
                                                     {"enabled", ui->logicEnabled->isChecked()},
                                                     {"protocol/iso7816/enabled", ui->logicIso7816->isChecked()}
                                                  }));

      // Page 7 — gRPC
      s.setValue("grpc/port", ui->grpcPort->value());

      // Page 8 — Logger
      s.beginGroup("logger");

      const QString rootLevel = ui->loggerRoot->currentText();
      s.setValue("root", rootLevel);
      rt::Logger::setRootLevel(rootLevel.toStdString());

      for (int i = 0; i < LOGGER_SUBSYSTEMS.size(); ++i)
      {
         if (const auto *cb = qobject_cast<QComboBox *>(ui->loggerTable->cellWidget(i, 1)))
         {
            const QString level = cb->currentText();
            s.setValue(LOGGER_SUBSYSTEMS[i], level);
            rt::Logger::getLogger(LOGGER_SUBSYSTEMS[i].toStdString())->setLevel(level.toStdString());
         }
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

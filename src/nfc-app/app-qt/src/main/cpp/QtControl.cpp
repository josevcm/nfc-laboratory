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

#include <filesystem>

#include <QDebug>

#include <QJsonDocument>
#include <QJsonArray>

#include <rt/Event.h>
#include <rt/Subject.h>

#include <hw/SignalBuffer.h>
#include <hw/RecordDevice.h>

#include <lab/data/RawFrame.h>

#include <lab/tasks/FourierProcessTask.h>
#include <lab/tasks/LogicDecoderTask.h>
#include <lab/tasks/LogicDeviceTask.h>
#include <lab/tasks/RadioDecoderTask.h>
#include <lab/tasks/RadioDeviceTask.h>
#include <lab/tasks/SignalStorageTask.h>
#include <lab/tasks/TraceStorageTask.h>

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

#include <features/Caps.h>

#include "QtApplication.h"

#include "QtControl.h"

struct QtControl::Impl
{
   // configuration
   QSettings settings;

   // status subjects
   rt::Subject<rt::Event> *logicDecoderStatusStream = nullptr;
   rt::Subject<rt::Event> *logicDeviceStatusStream = nullptr;
   rt::Subject<rt::Event> *radioDecoderStatusStream = nullptr;
   rt::Subject<rt::Event> *radioDeviceStatusStream = nullptr;
   rt::Subject<rt::Event> *fourierStatusStream = nullptr;
   rt::Subject<rt::Event> *recorderStatusStream = nullptr;
   rt::Subject<rt::Event> *storageStatusStream = nullptr;

   // command subjects
   rt::Subject<rt::Event> *logicDecoderCommandStream = nullptr;
   rt::Subject<rt::Event> *logicDeviceCommandStream = nullptr;
   rt::Subject<rt::Event> *radioDecoderCommandStream = nullptr;
   rt::Subject<rt::Event> *radioDeviceCommandStream = nullptr;
   rt::Subject<rt::Event> *fourierCommandStream = nullptr;
   rt::Subject<rt::Event> *recorderCommandStream = nullptr;
   rt::Subject<rt::Event> *storageCommandStream = nullptr;

   // frame data subjects
   rt::Subject<lab::RawFrame> *logicDecoderFrameStream = nullptr;
   rt::Subject<lab::RawFrame> *radioDecoderFrameStream = nullptr;
   rt::Subject<lab::RawFrame> *storageFrameStream = nullptr;

   // signal data subjects
   rt::Subject<hw::SignalBuffer> *adaptiveSignalStream = nullptr;
   rt::Subject<hw::SignalBuffer> *storageSignalStream = nullptr;

   // status subscriptions
   rt::Subject<rt::Event>::Subscription logicDecoderStatusSubscription;
   rt::Subject<rt::Event>::Subscription radioDecoderStatusSubscription;
   rt::Subject<rt::Event>::Subscription recorderStatusSubscription;
   rt::Subject<rt::Event>::Subscription storageStatusSubscription;
   rt::Subject<rt::Event>::Subscription logicDeviceStatusSubscription;
   rt::Subject<rt::Event>::Subscription radioDeviceStatusSubscription;
   rt::Subject<rt::Event>::Subscription fourierStatusSubscription;

   // frame stream subscriptions
   rt::Subject<lab::RawFrame>::Subscription logicDecoderFrameSubscription;
   rt::Subject<lab::RawFrame>::Subscription radioDecoderFrameSubscription;
   rt::Subject<lab::RawFrame>::Subscription storageFrameSubscription;

   // signal stream subscriptions
   rt::Subject<hw::SignalBuffer>::Subscription adaptiveSignalSubscription;
   rt::Subject<hw::SignalBuffer>::Subscription storageSignalSubscription;

   // storage status
   QString storagePath;

   // device names and type
   QString logicDeviceName;
   QString logicDeviceType;
   QString radioDeviceName;
   QString radioDeviceType;

   // device enabled flags
   bool logicDeviceEnabled = false;
   bool radioDeviceEnabled = false;

   // decoder enabled flags
   bool logicDecoderEnabled = false;
   bool radioDecoderEnabled = false;

   // default parameters for receivers
   QJsonObject defaultDeviceConfig = {
      {
         "radio.airspy", QJsonObject({
            {"enabled", true},
            {"centerFreq", 40680000},
            {"sampleRate", 10000000},
            {"gainMode", 1}, // linearity
            {"gainValue", 4}, // 4db
            {"mixerAgc", 0},
            {"tunerAgc", 0},
            {"biasTee", 0},
            {"directSampling", 0}
         })
      },
      {
         "radio.hydrasdr", QJsonObject({
            {"enabled", true},
            {"centerFreq", 40680000},
            {"sampleRate", 10000000},
            {"gainMode", 1}, // linearity
            {"gainValue", 4}, // 4db
            {"mixerAgc", 0},
            {"tunerAgc", 0},
            {"biasTee", 0},
            {"directSampling", 0}
         })
      },
      {
         "radio.rtlsdr", QJsonObject({
            {"enabled", true},
            {"centerFreq", 27120000},
            {"sampleRate", 3200000},
            {"gainMode", 1}, // manual
            {"gainValue", 77}, // 7.7db
            {"mixerAgc", 0},
            {"tunerAgc", 0},
            {"biasTee", 0},
            {"directSampling", 0}
         })
      },
      {
         "radio.miri", QJsonObject({
            {"enabled", true},
            {"centerFreq", 13560000},
            {"sampleRate", 10000000},
            {"gainMode", 1}, // manual
            {"gainValue", 0}, // 0db
            {"mixerAgc", 0},
            {"tunerAgc", 0},
            {"biasTee", 0},
            {"directSampling", 0}
         })
      },
      {
         "logic.dslogic", QJsonObject({
            {"enabled", true},
            {"sampleRate", 10000000},
            {"vThreshold", 1.0},
            {"channels", QJsonArray {0, 2, 3}}
         })
      }
   };

   explicit Impl()
   {
      // create status subjects
      logicDecoderStatusStream = rt::Subject<rt::Event>::name("logic.decoder.status");
      logicDeviceStatusStream = rt::Subject<rt::Event>::name("logic.receiver.status");
      radioDecoderStatusStream = rt::Subject<rt::Event>::name("radio.decoder.status");
      radioDeviceStatusStream = rt::Subject<rt::Event>::name("radio.receiver.status");
      fourierStatusStream = rt::Subject<rt::Event>::name("fourier.status");
      recorderStatusStream = rt::Subject<rt::Event>::name("recorder.status");
      storageStatusStream = rt::Subject<rt::Event>::name("storage.status");

      // create decoder control subject
      logicDecoderCommandStream = rt::Subject<rt::Event>::name("logic.decoder.command");
      logicDeviceCommandStream = rt::Subject<rt::Event>::name("logic.receiver.command");
      radioDecoderCommandStream = rt::Subject<rt::Event>::name("radio.decoder.command");
      radioDeviceCommandStream = rt::Subject<rt::Event>::name("radio.receiver.command");
      recorderCommandStream = rt::Subject<rt::Event>::name("recorder.command");
      storageCommandStream = rt::Subject<rt::Event>::name("storage.command");
      fourierCommandStream = rt::Subject<rt::Event>::name("fourier.command");

      // create frame subject
      logicDecoderFrameStream = rt::Subject<lab::RawFrame>::name("logic.decoder.frame");
      radioDecoderFrameStream = rt::Subject<lab::RawFrame>::name("radio.decoder.frame");
      storageFrameStream = rt::Subject<lab::RawFrame>::name("storage.frame");

      // create signal subject
      adaptiveSignalStream = rt::Subject<hw::SignalBuffer>::name("adaptive.signal");
      storageSignalStream = rt::Subject<hw::SignalBuffer>::name("storage.signal");
   }

   /*
    * system event executed after startup
    */
   void systemStartupEvent(SystemStartupEvent *event)
   {
      // subscribe to status events
      logicDeviceStatusSubscription = logicDeviceStatusStream->subscribe([this](const rt::Event &params) {
         logicDeviceStatusChange(params);
      });

      logicDecoderStatusSubscription = logicDecoderStatusStream->subscribe([this](const rt::Event &params) {
         logicDecoderStatusChange(params);
      });

      logicDecoderFrameSubscription = logicDecoderFrameStream->subscribe([this](const lab::RawFrame &frame) {
         logicDecoderFrameEvent(frame);
      });

      radioDeviceStatusSubscription = radioDeviceStatusStream->subscribe([this](const rt::Event &params) {
         radioDeviceStatusChange(params);
      });

      radioDecoderStatusSubscription = radioDecoderStatusStream->subscribe([this](const rt::Event &params) {
         radioDecoderStatusChange(params);
      });

      radioDecoderFrameSubscription = radioDecoderFrameStream->subscribe([this](const lab::RawFrame &frame) {
         radioDecoderFrameEvent(frame);
      });

      recorderStatusSubscription = recorderStatusStream->subscribe([this](const rt::Event &params) {
         recorderStatusChange(params);
      });

      storageStatusSubscription = storageStatusStream->subscribe([this](const rt::Event &params) {
         storageStatusChange(params);
      });

      fourierStatusSubscription = fourierStatusStream->subscribe([this](const rt::Event &params) {
         fourierStatusChange(params);
      });

      storageFrameSubscription = storageFrameStream->subscribe([this](const lab::RawFrame &frame) {
         radioDecoderFrameEvent(frame);
      });

      adaptiveSignalSubscription = adaptiveSignalStream->subscribe([this](const hw::SignalBuffer &buffer) {
         signalBufferEvent(buffer);
      });

      storageSignalSubscription = storageSignalStream->subscribe([this](const hw::SignalBuffer &buffer) {
         signalBufferEvent(buffer);
      });

      if (event->meta.contains("features"))
      {
         const QRegularExpression allowedFeatures(event->meta["features"]);

         if (allowedFeatures.match(Caps::LOGIC_DEVICE).hasMatch())
            logicDeviceInitialize();

         if (allowedFeatures.match(Caps::LOGIC_DECODE).hasMatch())
            logicDecoderInitialize();

         if (allowedFeatures.match(Caps::RADIO_DEVICE).hasMatch())
            radioDeviceInitialize();

         if (allowedFeatures.match(Caps::RADIO_DECODE).hasMatch())
            radioDecoderInitialize();

         if (allowedFeatures.match(Caps::RADIO_SPECTRUM).hasMatch())
            fourierInitialize();
      }

      storageInitialize();
   }

   /*
    * system event executed before shutdown
    */
   void systemShutdownEvent(SystemShutdownEvent *event)
   {
   }

   /*
   * process decoder control event
   */
   void decoderControlEvent(DecoderControlEvent *event)
   {
      switch (event->command())
      {
         case DecoderControlEvent::Start:
         {
            doStartDecode(event);
            break;
         }
         case DecoderControlEvent::Stop:
         {
            doStopDecode(event);
            break;
         }
         case DecoderControlEvent::Pause:
         {
            doPauseDecode(event);
            break;
         }
         case DecoderControlEvent::Resume:
         {
            doResumeDecode(event);
            break;
         }
         case DecoderControlEvent::Clear:
         {
            doClearBuffers(event);
            break;
         }
         case DecoderControlEvent::ReadFile:
         {
            doReadFile(event);
            break;
         }
         case DecoderControlEvent::WriteFile:
         {
            doWriteFile(event);
            break;
         }
         case DecoderControlEvent::LogicDeviceConfig:
         {
            doLogicDeviceConfig(event);
            break;
         }
         case DecoderControlEvent::LogicDecoderConfig:
         {
            doLogicDecoderConfig(event);
            break;
         }
         case DecoderControlEvent::RadioDeviceConfig:
         {
            doRadioDeviceConfig(event);
            break;
         }
         case DecoderControlEvent::RadioDecoderConfig:
         {
            doRadioDecoderConfig(event);
            break;
         }
         case DecoderControlEvent::FourierConfig:
         {
            doFourierConfig(event);
            break;
         }
      }
   }

   /*
    * start decoder and receiver task
    */
   void doStartDecode(DecoderControlEvent *event)
   {
      qInfo() << "start decoder and receiver tasks";

      // if event contains file name and sample rate start recorder
      if (event->contains("storagePath"))
      {
         storagePath = event->getString("storagePath");

         // clear storage queue
         taskStorageClear([=] {

            // start recorder and...
            taskRecorderWrite({{"storagePath", storagePath}}, [=] {

               if (!logicDeviceType.isEmpty() && logicDeviceEnabled)
               {
                  // start decoder and...
                  taskLogicDecoderStart([=] {
                                           taskLogicDeviceStart();
                                        }, [=](int, const std::string &) {
                                           taskLogicDeviceStart();
                                        });
               }

               if (!radioDeviceType.isEmpty() && radioDeviceEnabled)
               {
                  // start decoder and...
                  taskRadioDecoderStart([=] {
                                           taskRadioDeviceStart();
                                        }, [=](int, const std::string &) {
                                           taskRadioDeviceStart();
                                        });
               }
            });
         });
      }
      else
      {
         storagePath = QString();

         // clear storage queue
         taskStorageClear([=] {

            if (!logicDeviceType.isEmpty() && logicDeviceEnabled)
            {
               // start decoder and...
               taskLogicDecoderStart([=] {
                                        taskLogicDeviceStart();
                                     }, [=](int, const std::string &) {
                                        taskLogicDeviceStart();
                                     });
            }

            if (!radioDeviceType.isEmpty() && radioDeviceEnabled)
            {
               // start decoder and...
               taskRadioDecoderStart([=] {
                                        taskRadioDeviceStart();
                                     }, [=](int, const std::string &) {
                                        taskRadioDeviceStart();
                                     });
            }
         });
      }
   }

   /*
    * stop all tasks to finish decoding
    */
   void doStopDecode(DecoderControlEvent *event) const
   {
      qInfo() << "stop decoder and receiver tasks";

      // stop radio receiver task
      if (!logicDeviceType.isEmpty())
         taskLogicDeviceStop();

      // stop radio receiver task
      if (!radioDeviceType.isEmpty())
         taskRadioDeviceStop();

      // stop radio receiver task
      if (!storagePath.isEmpty())
         taskRecorderStop();
   }

   /*
    * pause all tasks to finish decoding
    */
   void doPauseDecode(DecoderControlEvent *event) const
   {
      qInfo() << "pause decoder and receiver tasks";

      // stop radio receiver task
      if (!logicDeviceType.isEmpty())
         taskLogicDevicePause();

      // stop radio receiver task
      if (!radioDeviceType.isEmpty())
         taskRadioDevicePause();
   }

   /*
    * resume all tasks
    */
   void doResumeDecode(DecoderControlEvent *event) const
   {
      qInfo() << "resume decoder and receiver tasks";

      // stop radio receiver task
      if (!logicDeviceType.isEmpty())
         taskLogicDeviceResume();

      // stop radio receiver task
      if (!radioDeviceType.isEmpty())
         taskRadioDeviceResume();
   }

   /*
    * reconfigure logic parameters
    */
   void doLogicDeviceConfig(DecoderControlEvent *event)
   {
      qInfo() << "configure logic device";

      QJsonObject config;

      if (event->contains("enabled"))
         config["enabled"] = event->getBoolean("enabled");

      // update logic config
      if (!config.isEmpty())
      {
         logicDeviceConfigure(config);
      }
   }

   /*
    * reconfigure logic decoder parameters
    */
   void doLogicDecoderConfig(DecoderControlEvent *event)
   {
      qInfo() << "configure logic decoder";

      QJsonObject config;
      QJsonObject iso7816;

      // logic decoder parameters
      if (event->contains("enabled"))
         config["enabled"] = event->getBoolean("enabled");

      if (event->contains("sampleRate"))
         config["sampleRate"] = event->getInteger("sampleRate");

      if (event->contains("streamTime"))
         config["streamTime"] = event->getInteger("streamTime");

      if (event->contains("debugEnabled"))
         config["debugEnabled"] = event->getBoolean("debugEnabled");

      // NFC-A parameters
      if (event->contains("protocol/iso7816/enabled"))
         iso7816["enabled"] = event->getBoolean("protocol/iso7816/enabled");

      // set radio protocol configuration
      if (!iso7816.isEmpty())
      {
         QJsonObject proto;

         // add logic protocols
         if (!iso7816.isEmpty())
            proto["iso7816"] = iso7816;

         config["protocol"] = proto;
      }

      // update decoder config
      if (!config.isEmpty())
      {
         logicDecoderConfigure(config);
      }
   }

   /*
    * reconfigure radio parameters
    */
   void doRadioDeviceConfig(DecoderControlEvent *event)
   {
      QJsonObject config;

      if (event->contains("enabled"))
         config["enabled"] = event->getBoolean("enabled");

      if (event->contains("centerFreq"))
         config["centerFreq"] = event->getInteger("centerFreq");

      if (event->contains("sampleRate"))
         config["sampleRate"] = event->getInteger("sampleRate");

      if (event->contains("gainMode"))
         config["gainMode"] = event->getInteger("gainMode");

      if (event->contains("gainValue"))
         config["gainValue"] = event->getInteger("gainValue");

      if (event->contains("mixerAgc"))
         config["mixerAgc"] = event->getInteger("mixerAgc");

      if (event->contains("tunerAgc"))
         config["tunerAgc"] = event->getInteger("tunerAgc");

      if (event->contains("biasTee"))
         config["biasTee"] = event->getInteger("biasTee");

      if (event->contains("directSampling"))
         config["directSampling"] = event->getInteger("directSampling");

      // update radio config
      if (!config.isEmpty())
      {
         radioDeviceConfigure(config);
      }
   }

   /*
   * reconfigure radio decoder parameters
   */
   void doRadioDecoderConfig(DecoderControlEvent *event)
   {
      QJsonObject config;
      QJsonObject nfca;
      QJsonObject nfcb;
      QJsonObject nfcf;
      QJsonObject nfcv;

      // radio decoder parameters
      if (event->contains("enabled"))
         config["enabled"] = event->getBoolean("enabled");

      if (event->contains("sampleRate"))
         config["sampleRate"] = event->getInteger("sampleRate");

      if (event->contains("streamTime"))
         config["streamTime"] = event->getInteger("streamTime");

      if (event->contains("debugEnabled"))
         config["debugEnabled"] = event->getBoolean("debugEnabled");

      if (event->contains("powerLevelThreshold"))
         config["powerLevelThreshold"] = event->getFloat("powerLevelThreshold");

      // NFC-A parameters
      if (event->contains("protocol/nfca/enabled"))
         nfca["enabled"] = event->getBoolean("protocol/nfca/enabled");

      if (event->contains("protocol/nfca/correlationThreshold"))
         nfca["correlationThreshold"] = event->getFloat("protocol/nfca/correlationThreshold");

      if (event->contains("protocol/nfca/minimumModulationDeep"))
         nfca["minimumModulationDeep"] = event->getFloat("protocol/nfca/minimumModulationDeep");

      if (event->contains("protocol/nfca/maximumModulationDeep"))
         nfca["maximumModulationDeep"] = event->getFloat("protocol/nfca/maximumModulationDeep");

      // NFC-B parameters
      if (event->contains("protocol/nfcb/enabled"))
         nfcb["enabled"] = event->getBoolean("protocol/nfcb/enabled");

      if (event->contains("protocol/nfcb/correlationThreshold"))
         nfcb["correlationThreshold"] = event->getFloat("protocol/nfcb/correlationThreshold");

      if (event->contains("protocol/nfcb/minimumModulationDeep"))
         nfcb["minimumModulationDeep"] = event->getFloat("protocol/nfcb/minimumModulationDeep");

      if (event->contains("protocol/nfcb/maximumModulationDeep"))
         nfcb["maximumModulationDeep"] = event->getFloat("protocol/nfcb/maximumModulationDeep");

      // NFC-F parameters
      if (event->contains("protocol/nfcf/enabled"))
         nfcf["enabled"] = event->getBoolean("protocol/nfcf/enabled");

      if (event->contains("protocol/nfcf/correlationThreshold"))
         nfcf["correlationThreshold"] = event->getFloat("protocol/nfcf/correlationThreshold");

      if (event->contains("protocol/nfcf/minimumModulationDeep"))
         nfcf["minimumModulationDeep"] = event->getFloat("protocol/nfcf/minimumModulationDeep");

      if (event->contains("protocol/nfcf/maximumModulationDeep"))
         nfcf["maximumModulationDeep"] = event->getFloat("protocol/nfcf/maximumModulationDeep");

      // NFC-V parameters
      if (event->contains("protocol/nfcv/enabled"))
         nfcv["enabled"] = event->getBoolean("protocol/nfcv/enabled");

      if (event->contains("protocol/nfcv/correlationThreshold"))
         nfcv["correlationThreshold"] = event->getFloat("protocol/nfcv/correlationThreshold");

      if (event->contains("protocol/nfcv/minimumModulationDeep"))
         nfcv["minimumModulationDeep"] = event->getFloat("protocol/nfcv/minimumModulationDeep");

      if (event->contains("protocol/nfcv/maximumModulationDeep"))
         nfcv["maximumModulationDeep"] = event->getFloat("protocol/nfcv/maximumModulationDeep");

      // set radio protocol configuration
      if (!nfca.isEmpty() || !nfcb.isEmpty() || !nfcf.isEmpty() || !nfcv.isEmpty())
      {
         QJsonObject proto;

         // add radio protocols
         if (!nfca.isEmpty())
            proto["nfca"] = nfca;

         if (!nfcb.isEmpty())
            proto["nfcb"] = nfcb;

         if (!nfcf.isEmpty())
            proto["nfcf"] = nfcf;

         if (!nfcv.isEmpty())
            proto["nfcv"] = nfcv;

         config["protocol"] = proto;
      }

      // update decoder config
      if (!config.isEmpty())
      {
         radioDecoderConfigure(config);
      }
   }

   /*
    * reconfigure fourier parameters
    */
   void doFourierConfig(DecoderControlEvent *event)
   {
      QJsonObject config;

      if (event->contains("enabled"))
         config["enabled"] = event->getBoolean("enabled");

      if (!config.isEmpty())
      {
         taskFourierConfig(config);
      }
   }

   /*
    * read frames from file
    */
   void doReadFile(DecoderControlEvent *event) const
   {
      const QString fileName = event->getString("fileName");
      const std::filesystem::path path(fileName.toStdString());
      const QJsonObject command {{"fileName", fileName}};

      if (path.extension() == ".trz")
      {
         // clear storage queue
         taskStorageClear([=] {
            // start XML file read
            taskStorageRead(command);
         });

         return;
      }

      if (path.extension() == ".wav")
      {
         hw::RecordDevice file(fileName.toStdString());

         if (!file.open(hw::RecordDevice::Mode::Read))
         {
            qWarning() << "unable to open file: " << fileName;
            return;
         }

         unsigned int channelCount = std::get<unsigned int>(file.get(hw::SignalDevice::PARAM_CHANNEL_COUNT));

         // clear storage queue
         taskStorageClear([=] {

            // if contains 4 channels... trigger logic decoder start
            if (channelCount >= 3)
            {
               taskLogicDecoderStart([=] {
                  taskRecorderRead(command);
               });
            }

            // if contains 1 or 2 channels... trigger radio decoder start
            else if (channelCount <= 2)
            {
               taskRadioDecoderStart([=] {
                  taskRecorderRead(command);
               });
            }
         });
      }
   }

   /*
    * write frames to file
    */
   void doWriteFile(DecoderControlEvent *event) const
   {
      QString fileName = event->getString("fileName");
      double timeStart = event->getDouble("timeStart", 0);
      double timeEnd = event->getDouble("timeEnd", 0);

      QJsonObject command {
         {"fileName", fileName},
         {"timeStart", timeStart},
         {"timeEnd", timeEnd}
      };

      if (fileName.endsWith(".trz"))
      {
         taskStorageWrite(command);
      }
   }

   /*
    * write frames to file
    */
   void doClearBuffers(DecoderControlEvent *event) const
   {
      // start decoder and...
      // if (!logicDeviceType.isEmpty())
      // {
      //    taskLogicDeviceClear([=] {
      //       taskLogicDecoderClear();
      //    });
      // }
      //
      // // clear radio receiver and decoder
      // if (!logicDeviceType.isEmpty())
      // {
      //    taskRadioDeviceClear([=] {
      //       taskRadioDecoderClear();
      //    });
      // }

      // clear storage
      taskStorageClear();
   }

   /*
    * read object from settings file
    */
   QJsonObject readConfig(const QString &group)
   {
      QJsonObject config;

      static QRegularExpression boolExpr("true|false");

      settings.beginGroup(group);

      for (const QString &key: settings.childKeys())
      {
         auto value = settings.value(key);

         if (value.toString().contains(boolExpr))
         {
            config[key] = value.toBool();
         }
         else if (value.toString().contains("/"))
         {
            config[key] = value.toString();
         }
         else if (value.canConvert<float>())
         {
            config[key] = value.toFloat();
         }
         else if (value.canConvert<QVariantList>())
         {
            config[key] = value.toJsonArray();
         }
      }

      settings.endGroup();

      for (const QString &entry: settings.childGroups())
      {
         if (entry.length() > group.length() && entry.startsWith(group))
         {
            const auto next = entry.indexOf('.', group.length() + 1);

            QString path = entry.left(next);
            QString name = path.mid(group.length() + 1, next - group.length() - 1);

            config[name] = readConfig(path);
         }
      }

      return config;
   }

   /*
    * save object to settings file
    */
   void saveConfig(const QJsonObject &config, const QString &group = "")
   {
      for (QString &key: config.keys())
      {
         if (config[key].isObject())
         {
            saveConfig(config[key].toObject(), group.isEmpty() ? key : group + "." + key);
         }
         else
         {
            settings.beginGroup(group);
            settings.setValue(key, config.value(key).toVariant());
            settings.endGroup();
         }
      }
   }

   /*
    * read receiver parameters from settings file
    */
   void logicDeviceInitialize()
   {
      QJsonObject command;

      if (!logicDeviceType.isEmpty())
      {
         if (!defaultDeviceConfig.contains(logicDeviceType))
         {
            qWarning() << "unable to configure logic, unknown device type: " << logicDeviceType;
            return;
         }

         const QJsonObject &defaults = defaultDeviceConfig[logicDeviceType].toObject();

         QJsonObject config = readConfig("device." + logicDeviceType);

         for (const QString &key: defaults.keys())
         {
            if (config.contains(key))
               command[key] = config[key];
            else
               command[key] = defaults[key];
         }
      }

      // override enabled flag
      if (!command.contains("enabled"))
         command["enabled"] = true;

      // set firmware path if not present
      if (!command.contains("firmwarePath"))
         command["firmwarePath"] = QCoreApplication::applicationDirPath() + "/firmware";

      // configure receiver
      taskLogicDeviceConfig(command);
   }

   /*
    *save receiver parameters to settings file
    */
   void logicDeviceConfigure(const QJsonObject &config)
   {
      // send configuration to device
      taskLogicDeviceConfig(config);

      // update settings
      if (!logicDeviceType.isEmpty())
      {
         saveConfig(config, "device." + logicDeviceType);
      }
   }

   /*
    * process logic status event
    */
   void logicDeviceStatusChange(const rt::Event &event)
   {
      if (const auto data = event.get<std::string>("data"))
      {
         QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         // update enabled flag
         if (status.contains("status"))
            logicDeviceEnabled = status["status"].toString() != "disabled";

         // configure device for first time
         if (logicDeviceName != status["name"].toString())
         {
            logicDeviceName = status["name"].toString();
            logicDeviceType = logicDeviceName.left(logicDeviceName.indexOf("://"));

            logicDeviceInitialize();
         }
         else
         {
            QJsonObject forward;
            static QJsonObject lastForward;

            // forward streamTime and samplerate to decoder
            if (status.contains("streamTime"))
               forward["streamTime"] = status["streamTime"].toInt();

            if (status.contains("sampleRate"))
               forward["sampleRate"] = status["sampleRate"].toInt();

            if (!forward.isEmpty() && forward != lastForward)
            {
               lastForward = forward;
               taskLogicDecoderConfig(forward);
            }

            QtApplication::post(LogicDeviceStatusEvent::create(status));
         }
      }
   }

   /*
    * read decoder parameters from settings file
    */
   void logicDecoderInitialize()
   {
      QJsonObject config = readConfig("decoder.logic");

      // override enabled flag
      if (!config.contains("enabled"))
         config["enabled"] = true;

      // configure receiver
      taskLogicDecoderConfig(config);
   }

   /*
    * configure radio parameters
    */
   void logicDecoderConfigure(const QJsonObject &config)
   {
      // send configuration to device
      taskLogicDecoderConfig(config);

      // update settings
      saveConfig(config, "decoder.logic");
   }

   /*
    * process decoder status event
    */
   void logicDecoderStatusChange(const rt::Event &event)
   {
      if (auto data = event.get<std::string>("data"))
      {
         QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         if (status.contains("status"))
            logicDecoderEnabled = status["status"].toString() != "disabled";

         QtApplication::post(LogicDecoderStatusEvent::create(status));
      }
   }

   /*
   * process new received frame
   */
   void logicDecoderFrameEvent(const lab::RawFrame &frame)
   {
      QtApplication::post(new StreamFrameEvent(frame), Qt::HighEventPriority);
   }

   /*
    * read radio parameters from settings file
    */
   void radioDeviceInitialize()
   {
      QJsonObject command;

      if (!radioDeviceType.isEmpty())
      {
         if (!defaultDeviceConfig.contains(radioDeviceType))
         {
            qWarning() << "unable to configure radio, unknown device type: " << radioDeviceType;
            return;
         }

         const QJsonObject &defaults = defaultDeviceConfig[radioDeviceType].toObject();

         QJsonObject config = readConfig("device." + radioDeviceType);

         for (const QString &key: defaults.keys())
         {
            if (config.contains(key))
               command[key] = config[key];
            else
               command[key] = defaults[key];
         }
      }

      // override enabled flag
      if (!command.contains("enabled"))
         command["enabled"] = true;

      // configure receiver
      taskRadioDeviceConfig(command);
   }

   /*
    * configure radio parameters
    */
   void radioDeviceConfigure(const QJsonObject &config)
   {
      // send configuration to device
      taskRadioDeviceConfig(config);

      // update settings
      if (!radioDeviceType.isEmpty())
      {
         saveConfig(config, "device." + radioDeviceType);
      }
   }

   /*
    * process radio status event
    */
   void radioDeviceStatusChange(const rt::Event &event)
   {
      if (const auto data = event.get<std::string>("data"))
      {
         const QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         // update enabled flag
         if (status.contains("status"))
            radioDeviceEnabled = status["status"].toString() != "disabled";

         // configure device for first time
         if (radioDeviceName != status["name"].toString())
         {
            radioDeviceName = status["name"].toString();
            radioDeviceType = radioDeviceName.left(radioDeviceName.indexOf("://"));

            radioDeviceInitialize();
         }
         else
         {
            QJsonObject forward;
            static QJsonObject lastForward;

            // forward streamTime and samplerate to decoder
            if (status.contains("streamTime"))
               forward["streamTime"] = status["streamTime"].toInt();

            if (status.contains("sampleRate"))
               forward["sampleRate"] = status["sampleRate"].toInt();

            if (!forward.isEmpty() && forward != lastForward)
            {
               lastForward = forward;
               taskRadioDecoderConfig(forward);
            }

            QtApplication::post(RadioDeviceStatusEvent::create(status));
         }
      }
   }

   /*
   * read radio decoder parameters from settings file
   */
   void radioDecoderInitialize()
   {
      QJsonObject config = readConfig("decoder.radio");

      // override enabled flag
      if (!config.contains("enabled"))
         config["enabled"] = true;

      // configure receiver
      taskRadioDecoderConfig(config);
   }

   /*
    * configure logic parameters
    */
   void radioDecoderConfigure(const QJsonObject &config)
   {
      // send configuration to device
      taskRadioDecoderConfig(config);

      // update settings
      saveConfig(config, "decoder.radio");
   }

   /*
    * process decoder status event
    */
   void radioDecoderStatusChange(const rt::Event &event)
   {
      if (const auto data = event.get<std::string>("data"))
      {
         const QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         if (status.contains("status"))
            radioDecoderEnabled = status["status"].toString() != "disabled";

         QtApplication::post(RadioDecoderStatusEvent::create(status));
      }
   }

   /*
    * process new received frame
    */
   void radioDecoderFrameEvent(const lab::RawFrame &frame)
   {
      QtApplication::post(new StreamFrameEvent(frame), Qt::HighEventPriority);
   }

   /*
    * setup fourier task
    */
   void fourierInitialize() const
   {
      QJsonObject config {{"enabled", true}};

      taskFourierConfig(config);
   }

   /*
   * process fourier status event
   */
   void fourierStatusChange(const rt::Event &event)
   {
      if (const auto data = event.get<std::string>("data"))
      {
         QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         QtApplication::post(FourierStatusEvent::create(status));
      }
   }

   /*
    * process recorder status event
    */
   void recorderStatusChange(const rt::Event &event)
   {
      if (const auto data = event.get<std::string>("data"))
      {
         QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         QtApplication::post(StorageStatusEvent::create(status));

         // forward streamTime to decoder
         if (status.contains("streamTime"))
         {
            // send streamTime to logic decoder
            taskLogicDecoderConfig({{"streamTime", status["streamTime"].toInt()}});

            // send streamTime to radio decoder
            taskRadioDecoderConfig({{"streamTime", status["streamTime"].toInt()}});
         }
      }
   }

   /*
    * setup storage task
    */
   void storageInitialize() const
   {
      QJsonObject config {{"tempPath", QtApplication::tempPath().absolutePath()}};

      taskStorageConfig(config);
   }

   /*
    * process storage status event
    */
   void storageStatusChange(const rt::Event &event) const
   {
      if (const auto data = event.get<std::string>("data"))
      {
         QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         QtApplication::post(StorageStatusEvent::create(status));
      }
   }

   /*
    * process new received buffer
    */
   void signalBufferEvent(const hw::SignalBuffer &buffer)
   {
      QtApplication::post(new SignalBufferEvent(buffer), Qt::LowEventPriority);
   }

   /*
    * start decoder task
    */
   void taskLogicDecoderStart(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "start logic decoder task";

      logicDecoderCommandStream->next({lab::LogicDecoderTask::Start, onComplete, onReject});
   }

   /*
    * stop decoder task
    */
   void taskLogicDecoderStop(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "stop logic decoder task";

      logicDecoderCommandStream->next({lab::LogicDecoderTask::Stop, onComplete, onReject});
   }

   /*
    * query decoder task
    */
   void taskLogicDecoderQuery(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "query logic decoder task";

      logicDecoderCommandStream->next({lab::LogicDecoderTask::Query, onComplete, onReject});
   }

   /*
    * configure decoder task
    */
   void taskLogicDecoderConfig(const QJsonObject &data, const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      const QJsonDocument doc(data);

      qInfo() << "configure logic decoder task";

      logicDecoderCommandStream->next({lab::LogicDecoderTask::Configure, onComplete, onReject, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * clear decoder queue
    */
   void taskLogicDecoderClear(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "clear logic decoder task";

      logicDecoderCommandStream->next({lab::LogicDecoderTask::Clear, onComplete, onReject});
   }

   /*
    * start decoder task
    */
   void taskRadioDecoderStart(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "start radio decoder task";

      radioDecoderCommandStream->next({lab::RadioDecoderTask::Start, onComplete, onReject});
   }

   /*
    * stop decoder task
    */
   void taskRadioDecoderStop(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "stop radio decoder task";

      radioDecoderCommandStream->next({lab::RadioDecoderTask::Stop, onComplete, onReject});
   }

   /*
    * query decoder task
    */
   void taskRadioDecoderQuery(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "query radio decoder task";

      radioDecoderCommandStream->next({lab::RadioDecoderTask::Query, onComplete, onReject});
   }

   /*
    * configure decoder task
    */
   void taskRadioDecoderConfig(const QJsonObject &data, const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      const QJsonDocument doc(data);

      qInfo() << "configure radio decoder task";

      radioDecoderCommandStream->next({lab::RadioDecoderTask::Configure, onComplete, onReject, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * clear decoder queue
    */
   void taskRadioDecoderClear(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "clear radio decoder task";

      radioDecoderCommandStream->next({lab::RadioDecoderTask::Clear, onComplete, onReject});
   }

   /*
    * start logic task
    */
   void taskLogicDeviceStart(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "start logic device task";

      logicDeviceCommandStream->next({lab::LogicDeviceTask::Start, onComplete, onReject});
   }

   /*
    * stop logic task
    */
   void taskLogicDeviceStop(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "stop logic device task";

      logicDeviceCommandStream->next({lab::LogicDeviceTask::Stop, onComplete, onReject});
   }

   /*
    * pause logic task
    */
   void taskLogicDevicePause(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "pause logic device task";

      logicDeviceCommandStream->next({lab::LogicDeviceTask::Pause, onComplete, onReject});
   }

   /*
    * resume logic task
    */
   void taskLogicDeviceResume(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "resume logic device task";

      logicDeviceCommandStream->next({lab::LogicDeviceTask::Resume, onComplete, onReject});
   }

   /*
    * enquire logic status
    */
   void taskLogicDeviceQuery(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "query logic device task";

      logicDeviceCommandStream->next({lab::LogicDeviceTask::Query, onComplete, onReject});
   }

   /*
    * configure logic task
    */
   void taskLogicDeviceConfig(const QJsonObject &data, const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      const QJsonDocument doc(data);

      qInfo() << "configure logic device task ";

      logicDeviceCommandStream->next({lab::LogicDeviceTask::Configure, onComplete, onReject, {{"data", doc.toJson().toStdString()}}});
   }

   /*
   * clear logic queue
   */
   void taskLogicDeviceClear(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "clear logic device task";

      logicDeviceCommandStream->next({lab::LogicDeviceTask::Clear, onComplete, onReject});
   }

   /*
    * start radio task
    */
   void taskRadioDeviceStart(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "start radio device task";

      radioDeviceCommandStream->next({lab::RadioDeviceTask::Start, onComplete, onReject});
   }

   /*
    * stop radio task
    */
   void taskRadioDeviceStop(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "stop radio device task";

      radioDeviceCommandStream->next({lab::RadioDeviceTask::Stop, onComplete, onReject});
   }

   /*
    * pause radio task
    */
   void taskRadioDevicePause(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "pause radio device task";

      radioDeviceCommandStream->next({lab::RadioDeviceTask::Pause, onComplete, onReject});
   }

   /*
    * resume radio task
    */
   void taskRadioDeviceResume(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "resume radio device task";

      radioDeviceCommandStream->next({lab::RadioDeviceTask::Resume, onComplete, onReject});
   }

   /*
    * enquire radio status
    */
   void taskRadioDeviceQuery(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "query radio device task";

      radioDeviceCommandStream->next({lab::RadioDeviceTask::Query, onComplete, onReject});
   }

   /*
    * configure radio task
    */
   void taskRadioDeviceConfig(const QJsonObject &data, const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      const QJsonDocument doc(data);

      qInfo() << "configure radio device task ";

      radioDeviceCommandStream->next({lab::RadioDeviceTask::Configure, onComplete, onReject, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * clear radio queue
    */
   void taskRadioDeviceClear(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "clear radio device task";

      radioDeviceCommandStream->next({lab::RadioDeviceTask::Clear, onComplete, onReject});
   }

   /*
   * configure fourier task
   */
   void taskFourierConfig(const QJsonObject &data, const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      const QJsonDocument doc(data);

      qInfo() << "configure fourier task ";

      fourierCommandStream->next({lab::FourierProcessTask::Configure, onComplete, onReject, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * start recorder task to read file
    */
   void taskRecorderRead(const QJsonObject &data, const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      const QJsonDocument doc(data);

      qInfo() << "start recorder task to read file:" << doc.toJson();

      recorderCommandStream->next({lab::SignalStorageTask::Read, onComplete, onReject, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * start recorder task to write file
    */
   void taskRecorderWrite(const QJsonObject &data, const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      const QJsonDocument doc(data);

      qInfo() << "start recorder task to write file:" << doc.toJson();

      recorderCommandStream->next({lab::SignalStorageTask::Write, onComplete, onReject, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * stop recorder task
    */
   void taskRecorderStop(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "stop recorder task";

      recorderCommandStream->next({lab::SignalStorageTask::Stop, onComplete, onReject});
   }

   /*
    * config storage task
    */
   void taskStorageConfig(const QJsonObject &data, const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      const QJsonDocument doc(data);

      qInfo() << "configure storage task";

      storageCommandStream->next({lab::TraceStorageTask::Config, onComplete, onReject, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * start storage task to read frames from file
    */
   void taskStorageRead(const QJsonObject &data, const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      const QJsonDocument doc(data);

      qInfo() << "start storage task to read file:" << doc.toJson();

      storageCommandStream->next({lab::TraceStorageTask::Read, onComplete, onReject, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * start storage task for write frames to file
    */
   void taskStorageWrite(const QJsonObject &data, const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      const QJsonDocument doc(data);

      qInfo() << "start storage task to write file:" << doc.toJson();

      storageCommandStream->next({lab::TraceStorageTask::Write, onComplete, onReject, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * clear storage task frames from internal buffer
    */
   void taskStorageClear(const std::function<void()> &onComplete = nullptr, const std::function<void(int, const std::string &)> &onReject = nullptr) const
   {
      qInfo() << "clear storage task";

      storageCommandStream->next({lab::TraceStorageTask::Clear, onComplete, onReject});
   }
};

QtControl::QtControl() : impl(new Impl())
{
}

void QtControl::handleEvent(QEvent *event)
{
   if (event->type() == SystemStartupEvent::Type)
      impl->systemStartupEvent(dynamic_cast<SystemStartupEvent *>(event));
   else if (event->type() == SystemShutdownEvent::Type)
      impl->systemShutdownEvent(dynamic_cast<SystemShutdownEvent *>(event));
   else if (event->type() == DecoderControlEvent::Type)
      impl->decoderControlEvent(dynamic_cast<DecoderControlEvent *>(event));
}

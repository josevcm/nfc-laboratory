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
#include <QDateTime>

#include <QJsonDocument>
#include <QJsonObject>
#include <utility>

#include <rt/Event.h>
#include <rt/Subject.h>

#include <sdr/SignalBuffer.h>

#include <nfc/NfcFrame.h>

#include <nfc/FrameDecoderTask.h>
#include <nfc/FrameStorageTask.h>
#include <nfc/SignalReceiverTask.h>
#include <nfc/SignalRecorderTask.h>

#include <events/DecoderControlEvent.h>
#include <events/DecoderStatusEvent.h>
#include <events/StreamFrameEvent.h>
#include <events/SystemStartupEvent.h>
#include <events/SystemShutdownEvent.h>
#include <events/ReceiverStatusEvent.h>
#include <events/StorageStatusEvent.h>
#include <events/SignalBufferEvent.h>

#include "QtApplication.h"

#include "QtMemory.h"
#include "QtDecoder.h"

struct QtDecoder::Impl
{
   // configuration
   QSettings &settings;

   // signal memory cache
   QtMemory *cache;

   // current device name
   QString currentDevice;

   // status subjects
   rt::Subject<rt::Event> *decoderStatusStream = nullptr;
   rt::Subject<rt::Event> *recorderStatusStream = nullptr;
   rt::Subject<rt::Event> *storageStatusStream = nullptr;
   rt::Subject<rt::Event> *receiverStatusStream = nullptr;

   // command subjects
   rt::Subject<rt::Event> *decoderCommandStream = nullptr;
   rt::Subject<rt::Event> *recorderCommandStream = nullptr;
   rt::Subject<rt::Event> *storageCommandStream = nullptr;
   rt::Subject<rt::Event> *receiverCommandStream = nullptr;

   // frame data subjects
   rt::Subject<nfc::NfcFrame> *decoderFrameStream = nullptr;
   rt::Subject<nfc::NfcFrame> *storageFrameStream = nullptr;

   // signal data subjects
   rt::Subject<sdr::SignalBuffer> *signalStream = nullptr;

   // subscriptions
   rt::Subject<rt::Event>::Subscription decoderStatusSubscription;
   rt::Subject<rt::Event>::Subscription recorderStatusSubscription;
   rt::Subject<rt::Event>::Subscription storageStatusSubscription;
   rt::Subject<rt::Event>::Subscription receiverStatusSubscription;

   // frame stream subscription
   rt::Subject<nfc::NfcFrame>::Subscription decoderFrameSubscription;
   rt::Subject<nfc::NfcFrame>::Subscription storageFrameSubscription;

   // signal stream subscription
   rt::Subject<sdr::SignalBuffer>::Subscription signalSubscription;

   explicit Impl(QSettings &settings, QtMemory *cache) : settings(settings), cache(cache)
   {
      // create status subjects
      decoderStatusStream = rt::Subject<rt::Event>::name("decoder.status");
      recorderStatusStream = rt::Subject<rt::Event>::name("recorder.status");
      storageStatusStream = rt::Subject<rt::Event>::name("storage.status");
      receiverStatusStream = rt::Subject<rt::Event>::name("receiver.status");

      // create decoder control subject
      decoderCommandStream = rt::Subject<rt::Event>::name("decoder.command");
      recorderCommandStream = rt::Subject<rt::Event>::name("recorder.command");
      storageCommandStream = rt::Subject<rt::Event>::name("storage.command");
      receiverCommandStream = rt::Subject<rt::Event>::name("receiver.command");

      // create frame subject
      decoderFrameStream = rt::Subject<nfc::NfcFrame>::name("decoder.frame");
      storageFrameStream = rt::Subject<nfc::NfcFrame>::name("storage.frame");

      // create signal subject
      signalStream = rt::Subject<sdr::SignalBuffer>::name("signal.adp");
   }

   /*
    * system event executed after startup
    */
   void systemStartup(SystemStartupEvent *event)
   {
      // subscribe to status events
      decoderStatusSubscription = decoderStatusStream->subscribe([this](const rt::Event &params) {
         decoderStatusChange(params);
      });

      recorderStatusSubscription = recorderStatusStream->subscribe([this](const rt::Event &params) {
         recorderStatusChange(params);
      });

      storageStatusSubscription = storageStatusStream->subscribe([this](const rt::Event &params) {
         storageStatusChange(params);
      });

      receiverStatusSubscription = receiverStatusStream->subscribe([this](const rt::Event &params) {
         receiverStatusChange(params);
      });

      decoderFrameSubscription = decoderFrameStream->subscribe([this](const nfc::NfcFrame &frame) {
         frameEvent(frame);
      });

      storageFrameSubscription = storageFrameStream->subscribe([this](const nfc::NfcFrame &frame) {
         frameEvent(frame);
      });

      signalSubscription = signalStream->subscribe([this](const sdr::SignalBuffer &buffer) {
         bufferEvent(buffer);
      });

      // enquire status of receiver task
      taskReceiverQuery();

      // restore decoder configuration from .ini file
      readDecoderConfig();
   }

   /*
    * system event executed before shutdown
    */
   void systemShutdown(SystemShutdownEvent *event)
   {
   }

   /*
    * read decoder parameters from settings file
    */
   void readDecoderConfig()
   {
      auto *decoderConfigEvent = new DecoderControlEvent(DecoderControlEvent::DecoderConfig);

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
                     decoderConfigEvent->setBoolean(nfc + "/" + key, settings.value(key).toBool());
                  else
                     decoderConfigEvent->setFloat(nfc + "/" + key, settings.value(key).toFloat());
               }
               else
               {
                  decoderConfigEvent->setFloat(key, settings.value(key).toFloat());
               }
            }

            settings.endGroup();
         }
      }

      QtApplication::post(decoderConfigEvent);
   }

   /*
    * save decoder parameters to settings file
    */
   void saveDecoderConfig(const QJsonObject &status)
   {
      QStringList list = {"nfca", "nfcb", "nfcf", "nfcv"};

      for (QString &name: list)
      {
         if (status.contains(name))
         {
            QJsonObject config = status[name].toObject();

            settings.beginGroup("decoder." + name);

            for (QString &entry: config.keys())
            {
               QVariant newValue = config.value(entry).toVariant();

               if (settings.value(entry) != newValue)
                  settings.setValue(entry, newValue);
            }

            settings.endGroup();
         }
      }
   }

   /*
    * process decoder control event
    */
   void decoderControl(DecoderControlEvent *event)
   {
      switch (event->command())
      {
         case DecoderControlEvent::ReceiverDecode:
         {
            doReceiverDecode(event);
            break;
         }
         case DecoderControlEvent::ReceiverRecord:
         {
            doReceiverRecord(event);
            break;
         }
         case DecoderControlEvent::ReceiverConfig:
         {
            doReceiverConfig(event);
            break;
         }
         case DecoderControlEvent::StopDecode:
         {
            doStopDecode(event);
            break;
         }
         case DecoderControlEvent::DecoderConfig:
         {
            doDecoderConfig(event);
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
      }
   }

   /*
    * process decoder status event
    */
   void decoderStatusChange(const rt::Event &event)
   {
      if (auto data = event.get<std::string>("data"))
      {
         QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         QtApplication::post(DecoderStatusEvent::create(status));

         saveDecoderConfig(status);
      }
   }

   /*
    * process recorder status event
    */
   void recorderStatusChange(const rt::Event &event)
   {
      if (auto data = event.get<std::string>("data"))
      {
         QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         QtApplication::post(StorageStatusEvent::create(status));
      }
   }

   /*
    * process receiver status event
    */
   void receiverStatusChange(const rt::Event &event)
   {
      if (auto data = event.get<std::string>("data"))
      {
         QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         QtApplication::post(ReceiverStatusEvent::create(status));
      }
   }

   /*
    * process storage status event
    */
   void storageStatusChange(const rt::Event &event)
   {
      if (auto data = event.get<std::string>("data"))
      {
         QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         QtApplication::post(StorageStatusEvent::create(status));
      }
   }

   /*
    * process new received frame
    */
   void frameEvent(const nfc::NfcFrame &frame)
   {
      QtApplication::post(new StreamFrameEvent(frame), Qt::HighEventPriority);
   }

   /*
    * process new received buffer
    */
   void bufferEvent(const sdr::SignalBuffer &buffer)
   {
//      cache->append(buffer);
      QtApplication::post(new SignalBufferEvent(buffer), Qt::LowEventPriority);
   }

   /*
    * start decoder and receiver task
    */
   void doReceiverDecode(DecoderControlEvent *event) const
   {
      // clear signal cache
      cache->clear();

      // clear storage queue
      taskStorageClear();

      // start decoder and...
      taskDecoderStart([=] {
         // enable receiver
         taskReceiverStart();
      });
   }

   /*
    * start decoder, receiver and write data to file
    */
   void doReceiverRecord(DecoderControlEvent *event) const
   {
      QJsonObject json;

      json["fileName"] = event->getString("fileName");
      json["sampleRate"] = event->getInteger("sampleRate");

      // clear signal cache
      cache->clear();

      // clear storage queue
      taskStorageClear();

      // start recorder and...
      taskRecorderWrite(json, [=] {
         // start decoder and...
         taskDecoderStart([=] {
            // enable receiver
            taskReceiverStart();
         });
      });
   }

   /*
    * stop all tasks to finish decoding
    */
   void doStopDecode(DecoderControlEvent *event) const
   {
      // stop all taks
      taskDecoderStop();
      taskReceiverStop();
      taskRecorderStop();
   }

   /*
    * reconfigure decoder parameters
    */
   void doDecoderConfig(DecoderControlEvent *event) const
   {
      QJsonObject json;
      QJsonObject nfca;
      QJsonObject nfcb;
      QJsonObject nfcf;
      QJsonObject nfcv;

      if (event->contains("powerLevelThreshold"))
         json["powerLevelThreshold"] = event->getFloat("powerLevelThreshold");

      // NFC-A parameters
      if (event->contains("nfca/enabled"))
         nfca["enabled"] = event->getBoolean("nfca/enabled");

      if (event->contains("nfca/minimumModulationDeep"))
         nfca["minimumModulationDeep"] = event->getFloat("nfca/minimumModulationDeep");

      if (event->contains("nfca/maximumModulationDeep"))
         nfca["maximumModulationDeep"] = event->getFloat("nfca/maximumModulationDeep");

      // NFC-B parameters
      if (event->contains("nfcb/enabled"))
         nfcb["enabled"] = event->getBoolean("nfcb/enabled");

      if (event->contains("nfcb/minimumModulationDeep"))
         nfcb["minimumModulationDeep"] = event->getFloat("nfcb/minimumModulationDeep");

      if (event->contains("nfcb/maximumModulationDeep"))
         nfcb["maximumModulationDeep"] = event->getFloat("nfcb/maximumModulationDeep");

      // NFC-F parameters
      if (event->contains("nfcf/enabled"))
         nfcf["enabled"] = event->getBoolean("nfcf/enabled");

      if (event->contains("nfcf/minimumModulationDeep"))
         nfcf["minimumModulationDeep"] = event->getFloat("nfcf/minimumModulationDeep");

      if (event->contains("nfcf/maximumModulationDeep"))
         nfcf["maximumModulationDeep"] = event->getFloat("nfcf/maximumModulationDeep");

      // NFC-V parameters
      if (event->contains("nfcv/enabled"))
         nfcv["enabled"] = event->getBoolean("nfcv/enabled");

      if (event->contains("nfcv/minimumModulationDeep"))
         nfcv["minimumModulationDeep"] = event->getFloat("nfcv/minimumModulationDeep");

      if (event->contains("nfcv/maximumModulationDeep"))
         nfcv["maximumModulationDeep"] = event->getFloat("nfcv/maximumModulationDeep");

      if (!nfca.isEmpty())
         json["nfca"] = nfca;

      if (!nfcb.isEmpty())
         json["nfcb"] = nfcb;

      if (!nfcf.isEmpty())
         json["nfcf"] = nfcf;

      if (!nfcv.isEmpty())
         json["nfcv"] = nfcv;

      taskDecoderConfig(json);
   }

   /*
    * reconfigure receiver parameters
    */
   void doReceiverConfig(DecoderControlEvent *event) const
   {
      QJsonObject json;

      if (event->contains("centerFreq"))
         json["centerFreq"] = event->getInteger("centerFreq");

      if (event->contains("sampleRate"))
         json["sampleRate"] = event->getInteger("sampleRate");

      if (event->contains("gainMode"))
         json["gainMode"] = event->getInteger("gainMode");

      if (event->contains("gainValue"))
         json["gainValue"] = event->getInteger("gainValue");

      if (event->contains("mixerAgc"))
         json["mixerAgc"] = event->getInteger("mixerAgc");

      if (event->contains("tunerAgc"))
         json["tunerAgc"] = event->getInteger("tunerAgc");

      taskReceiverConfig(json);
   }

   /*
    * read frames from file
    */
   void doReadFile(DecoderControlEvent *event) const
   {
      QJsonObject json;

      QString fileName = event->getString("fileName");

      json["fileName"] = fileName;

      if (fileName.endsWith(".wav"))
      {
         // clear storage queue
         taskStorageClear();

         // start decoder and...
         taskDecoderStart([=] {

            // read file
            taskRecorderRead(json);
         });
      }
      else if (fileName.endsWith(".xml") || fileName.endsWith(".json"))
      {
         // clear storage queue
         taskStorageClear();

         // start XML file read
         taskStorageRead(json);
      }
   }

   /*
    * write frames to file
    */
   void doWriteFile(DecoderControlEvent *event) const
   {
      QJsonObject json;

      QString fileName = event->getString("fileName");

      json["fileName"] = fileName;

      if (fileName.endsWith(".wav"))
      {
      }
      else if (fileName.endsWith(".xml") || fileName.endsWith(".json"))
      {
         // start XML file write
         taskStorageWrite(json);
      }
   }

   /*
    * start decoder task
    */
   void taskDecoderStart(std::function<void()> onComplete = nullptr) const
   {
      decoderCommandStream->next({nfc::FrameDecoderTask::Start, std::move(onComplete)});
   }

   /*
    * stop decoder task
    */
   void taskDecoderStop(std::function<void()> onComplete = nullptr) const
   {
      decoderCommandStream->next({nfc::FrameDecoderTask::Stop, std::move(onComplete)});
   }

   /*
    * configure decoder task
    */
   void taskDecoderConfig(const QJsonObject &data, std::function<void()> onComplete = nullptr) const
   {
      QJsonDocument doc(data);

      decoderCommandStream->next({nfc::FrameDecoderTask::Configure, std::move(onComplete), nullptr, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * start receiver task
    */
   void taskReceiverStart(std::function<void()> onComplete = nullptr) const
   {
      receiverCommandStream->next({nfc::SignalReceiverTask::Start, std::move(onComplete)});
   }

   /*
    * stop receiver task
    */
   void taskReceiverStop(std::function<void()> onComplete = nullptr) const
   {
      receiverCommandStream->next({nfc::SignalReceiverTask::Stop, std::move(onComplete)});
   }

   /*
    * enquire receiver status
    */
   void taskReceiverQuery(std::function<void()> onComplete = nullptr) const
   {
      receiverCommandStream->next({nfc::SignalReceiverTask::Query, std::move(onComplete)});
   }

   /*
    * configure receiver task
    */
   void taskReceiverConfig(const QJsonObject &data, std::function<void()> onComplete = nullptr) const
   {
      QJsonDocument doc(data);

      receiverCommandStream->next({nfc::SignalReceiverTask::Configure, std::move(onComplete), nullptr, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * start recorder task to read file
    */
   void taskRecorderRead(const QJsonObject &data, std::function<void()> onComplete = nullptr) const
   {
      QJsonDocument doc(data);

      recorderCommandStream->next({nfc::SignalRecorderTask::Read, std::move(onComplete), nullptr, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * start recorder task to write file
    */
   void taskRecorderWrite(const QJsonObject &data, std::function<void()> onComplete = nullptr) const
   {
      QJsonDocument doc(data);

      recorderCommandStream->next({nfc::SignalRecorderTask::Write, std::move(onComplete), nullptr, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * stop recorder task
    */
   void taskRecorderStop(std::function<void()> onComplete = nullptr) const
   {
      recorderCommandStream->next({nfc::SignalRecorderTask::Stop, std::move(onComplete)});
   }

   /*
    * start storage task to read frames from file
    */
   void taskStorageRead(const QJsonObject &data, std::function<void()> onComplete = nullptr) const
   {
      QJsonDocument doc(data);

      // read frame data from file
      storageCommandStream->next({nfc::FrameStorageTask::Read, std::move(onComplete), nullptr, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * start storage task for write frames to file
    */
   void taskStorageWrite(const QJsonObject &data, std::function<void()> onComplete = nullptr) const
   {
      QJsonDocument doc(data);

      // write frame data to file
      storageCommandStream->next({nfc::FrameStorageTask::Write, std::move(onComplete), nullptr, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * clear storage task frames from internal buffer
    */
   void taskStorageClear(std::function<void()> onComplete = nullptr) const
   {
      storageCommandStream->next({nfc::FrameStorageTask::Clear, std::move(onComplete)});
   }
};

QtDecoder::QtDecoder(QSettings &settings, QtMemory *cache) : impl(new Impl(settings, cache))
{
}

void QtDecoder::handleEvent(QEvent *event)
{
   if (event->type() == SystemStartupEvent::Type)
      impl->systemStartup(dynamic_cast<SystemStartupEvent *>(event));
   else if (event->type() == SystemShutdownEvent::Type)
      impl->systemShutdown(dynamic_cast<SystemShutdownEvent *>(event));
   else if (event->type() == DecoderControlEvent::Type)
      impl->decoderControl(dynamic_cast<DecoderControlEvent *>(event));
}

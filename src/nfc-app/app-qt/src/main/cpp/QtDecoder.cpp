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

#include <nfc/NfcFrame.h>

#include <nfc/FrameDecoderTask.h>
#include <nfc/FrameStorageTask.h>
#include <nfc/SignalReceiverTask.h>
#include <nfc/SignalRecorderTask.h>

#include <events/DecoderControlEvent.h>
#include <events/StreamFrameEvent.h>
#include <events/SystemStartupEvent.h>
#include <events/SystemShutdownEvent.h>
#include <events/ReceiverStatusEvent.h>
#include <events/StorageStatusEvent.h>

#include "QtApplication.h"
#include "QtDecoder.h"

struct QtDecoder::Impl
{
   // configuration
   QSettings &settings;

   // current device name
   QString currentDevice;

   // status subjects
   rt::Subject<rt::Event> *decoderStatusSubject = nullptr;
   rt::Subject<rt::Event> *recorderStatusSubject = nullptr;
   rt::Subject<rt::Event> *storageStatusSubject = nullptr;
   rt::Subject<rt::Event> *receiverStatusSubject = nullptr;

   // command subjects
   rt::Subject<rt::Event> *decoderCommandSubject = nullptr;
   rt::Subject<rt::Event> *recorderCommandSubject = nullptr;
   rt::Subject<rt::Event> *storageCommandSubject = nullptr;
   rt::Subject<rt::Event> *receiverCommandSubject = nullptr;

   // frame data subjects
   rt::Subject<nfc::NfcFrame> *decoderFrameSubject = nullptr;
   rt::Subject<nfc::NfcFrame> *storageFrameSubject = nullptr;

   // subscriptions
   rt::Subject<rt::Event>::Subscription decoderStatusSubscription;
   rt::Subject<rt::Event>::Subscription recorderStatusSubscription;
   rt::Subject<rt::Event>::Subscription storageStatusSubscription;
   rt::Subject<rt::Event>::Subscription receiverStatusSubscription;

   // frame stream subscription
   rt::Subject<nfc::NfcFrame>::Subscription decoderFrameSubscription;
   rt::Subject<nfc::NfcFrame>::Subscription storageFrameSubscription;

   explicit Impl(QSettings &settings) : settings(settings)
   {
      // create status subjects
      decoderStatusSubject = rt::Subject<rt::Event>::name("decoder.status");
      recorderStatusSubject = rt::Subject<rt::Event>::name("recorder.status");
      storageStatusSubject = rt::Subject<rt::Event>::name("storage.status");
      receiverStatusSubject = rt::Subject<rt::Event>::name("receiver.status");

      // create decoder control subject
      decoderCommandSubject = rt::Subject<rt::Event>::name("decoder.command");
      recorderCommandSubject = rt::Subject<rt::Event>::name("recorder.command");
      storageCommandSubject = rt::Subject<rt::Event>::name("storage.command");
      receiverCommandSubject = rt::Subject<rt::Event>::name("receiver.command");

      // create frame subject
      decoderFrameSubject = rt::Subject<nfc::NfcFrame>::name("decoder.frame");
      storageFrameSubject = rt::Subject<nfc::NfcFrame>::name("storage.frame");
   }

   void systemStartup(SystemStartupEvent *event)
   {
      // subscribe to status events
      decoderStatusSubscription = decoderStatusSubject->subscribe([this](const rt::Event &params) {
         decoderStatusChange(params);
      });

      recorderStatusSubscription = recorderStatusSubject->subscribe([this](const rt::Event &params) {
         recorderStatusChange(params);
      });

      storageStatusSubscription = storageStatusSubject->subscribe([this](const rt::Event &params) {
         storageStatusChange(params);
      });

      receiverStatusSubscription = receiverStatusSubject->subscribe([this](const rt::Event &params) {
         receiverStatusChange(params);
      });

      decoderFrameSubscription = decoderFrameSubject->subscribe([this](const nfc::NfcFrame &frame) {
         frameEvent(frame);
      });

      storageFrameSubscription = storageFrameSubject->subscribe([this](const nfc::NfcFrame &frame) {
         frameEvent(frame);
      });

      taskReceiverQuery();
   }

   void systemShutdown(SystemShutdownEvent *event)
   {
   }

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

   void decoderStatusChange(const rt::Event &status)
   {
   }

   void recorderStatusChange(const rt::Event &event)
   {
      if (auto data = event.get<std::string>("data"))
      {
         QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         QtApplication::post(StorageStatusEvent::create(status));
      }
   }

   void receiverStatusChange(const rt::Event &event)
   {
      if (auto data = event.get<std::string>("data"))
      {
         QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         QtApplication::post(ReceiverStatusEvent::create(status));
      }
   }

   void storageStatusChange(const rt::Event &event)
   {
      if (auto data = event.get<std::string>("data"))
      {
         QJsonObject status = QJsonDocument::fromJson(QByteArray::fromStdString(data.value())).object();

         QtApplication::post(StorageStatusEvent::create(status));
      }
   }

   void frameEvent(const nfc::NfcFrame &frame)
   {
      QtApplication::post(new StreamFrameEvent(frame));
   }

   void doReceiverDecode(DecoderControlEvent *event) const
   {
      // clear storage queue
      taskStorageClear();

      // start decoder and...
      taskDecoderStart([=] {
         // enable receiver
         taskReceiverStart();
      });
   }

   void doReceiverRecord(DecoderControlEvent *event) const
   {
      QString name = QString("record-%1.wav").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));

      // start recorder and...
      taskRecorderWrite(name, [=] {
         // enable receiver
         taskReceiverStart();
      });
   }

   void doStopDecode(DecoderControlEvent *event) const
   {
      // stop all taks
      taskDecoderStop();
      taskReceiverStop();
      taskRecorderStop();
   }

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

      if (event->contains("nfca/minimumModulationThreshold"))
         nfca["minimumModulationThreshold"] = event->getFloat("nfca/minimumModulationThreshold");

      if (event->contains("nfca/maximumModulationThreshold"))
         nfca["maximumModulationThreshold"] = event->getFloat("nfca/maximumModulationThreshold");

      // NFC-B parameters
      if (event->contains("nfcb/enabled"))
         nfcb["enabled"] = event->getBoolean("nfcb/enabled");

      if (event->contains("nfcb/minimumModulationThreshold"))
         nfcb["minimumModulationThreshold"] = event->getFloat("nfcb/minimumModulationThreshold");

      if (event->contains("nfcb/maximumModulationThreshold"))
         nfcb["maximumModulationThreshold"] = event->getFloat("nfcb/maximumModulationThreshold");

      // NFC-F parameters
      if (event->contains("nfcf/enabled"))
         nfcf["enabled"] = event->getBoolean("nfcf/enabled");

      if (event->contains("nfca/minimumModulationThreshold"))
         nfcf["minimumModulationThreshold"] = event->getFloat("nfcf/minimumModulationThreshold");

      if (event->contains("nfca/maximumModulationThreshold"))
         nfcf["maximumModulationThreshold"] = event->getFloat("nfcf/maximumModulationThreshold");

      // NFC-V parameters
      if (event->contains("nfcv/enabled"))
         nfcv["enabled"] = event->getBoolean("nfcv/enabled");

      if (event->contains("nfcv/minimumModulationThreshold"))
         nfcv["minimumModulationThreshold"] = event->getFloat("nfcv/minimumModulationThreshold");

      if (event->contains("nfcv/maximumModulationThreshold"))
         nfcv["maximumModulationThreshold"] = event->getFloat("nfcv/maximumModulationThreshold");

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

   void doReadFile(DecoderControlEvent *event) const
   {
      QString name = event->getString("file");

      if (name.endsWith(".wav"))
      {
         // clear storage queue
         taskStorageClear();

         // start decoder and...
         taskDecoderStart([=] {
            // read file
            taskRecorderRead(name);
         });
      }
      else if (name.endsWith(".xml") || name.endsWith(".json"))
      {
         // clear storage queue
         taskStorageClear();

         // start XML file read
         taskStorageRead(name);
      }
   }

   void doWriteFile(DecoderControlEvent *event) const
   {
      QString name = event->getString("file");

      if (name.endsWith(".wav"))
      {
      }
      else if (name.endsWith(".xml") || name.endsWith(".json"))
      {
         // start XML file write
         taskStorageWrite(name);
      }
   }

   /*
    * Decoder Task control
    */
   void taskDecoderStart(std::function<void()> complete = nullptr) const
   {
      decoderCommandSubject->next({nfc::FrameDecoderTask::Start, std::move(complete)});
   }

   void taskDecoderStop(std::function<void()> complete = nullptr) const
   {
      decoderCommandSubject->next({nfc::FrameDecoderTask::Stop, std::move(complete)});
   }

   void taskDecoderConfig(const QJsonObject &data, std::function<void()> complete = nullptr) const
   {
      QJsonDocument doc(data);

      decoderCommandSubject->next({nfc::FrameDecoderTask::Configure, std::move(complete), nullptr, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * Receiver Task control
    */
   void taskReceiverStart(std::function<void()> complete = nullptr) const
   {
      receiverCommandSubject->next({nfc::SignalReceiverTask::Start, std::move(complete)});
   }

   void taskReceiverStop(std::function<void()> complete = nullptr) const
   {
      receiverCommandSubject->next({nfc::SignalReceiverTask::Stop, std::move(complete)});
   }

   void taskReceiverQuery(std::function<void()> complete = nullptr) const
   {
      receiverCommandSubject->next({nfc::SignalReceiverTask::Query, std::move(complete)});
   }

   void taskReceiverConfig(const QJsonObject &data, std::function<void()> complete = nullptr) const
   {
      QJsonDocument doc(data);

      receiverCommandSubject->next({nfc::SignalReceiverTask::Configure, std::move(complete), nullptr, {{"data", doc.toJson().toStdString()}}});
   }

   /*
    * Recorder Task control
    */
   void taskRecorderRead(const QString &name, std::function<void()> complete = nullptr) const
   {
      recorderCommandSubject->next({nfc::SignalRecorderTask::Read, std::move(complete), nullptr, {{"file", name.toStdString()}}});
   }

   void taskRecorderWrite(const QString &name, std::function<void()> complete = nullptr) const
   {
      recorderCommandSubject->next({nfc::SignalRecorderTask::Write, std::move(complete), nullptr, {{"file", name.toStdString()}}});
   }

   void taskRecorderStop(std::function<void()> complete = nullptr) const
   {
      recorderCommandSubject->next({nfc::SignalRecorderTask::Stop, std::move(complete)});
   }

   /*
    * Storage Task control
    */
   void taskStorageRead(const QString &name, std::function<void()> complete = nullptr) const
   {
      // read frame data from file
      storageCommandSubject->next({nfc::FrameStorageTask::Read, std::move(complete), nullptr, {{"file", name.toStdString()}}});
   }

   void taskStorageWrite(const QString &name, std::function<void()> complete = nullptr) const
   {
      // write frame data to file
      storageCommandSubject->next({nfc::FrameStorageTask::Write, std::move(complete), nullptr, {{"file", name.toStdString()}}});
   }

   void taskStorageClear(std::function<void()> complete = nullptr) const
   {
      storageCommandSubject->next({nfc::FrameStorageTask::Clear, std::move(complete)});
   }
};

QtDecoder::QtDecoder(QSettings &settings) : impl(new Impl(settings))
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

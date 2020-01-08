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
#include <QMutexLocker>
#include <QThreadPool>
#include <QElapsedTimer>
#include <QDateTime>

#include <support/TaskRunner.h>

#include <devices/RecordDevice.h>
#include <devices/AirspyDevice.h>
#include <devices/RealtekDevice.h>
//#include <devices/LimeDevice.h>

#include <decoder/NfcFrame.h>
#include <decoder/NfcStream.h>
#include <decoder/NfcDecoder.h>
#include <decoder/NfcCapture.h>

#include <events/DecoderControlEvent.h>
#include <events/ConsoleLogEvent.h>
#include <events/GainControlEvent.h>
#include <events/StreamStatusEvent.h>
#include <events/StreamFrameEvent.h>

#include <protocol/ProtocolParser.h>

#include <Dispatcher.h>

#include "NfcService.h"

NfcService::NfcService(QSettings &settings, NfcStream *stream) :
   mSettings(settings),
   mStream(stream),
   mDeviceList(""),
   mSignalSource("none"),
   mSignalRecord("none"),
   mRecordChannels(0),
   mServiceStatus(0),
   mScannerStatus(0),
   mFrequency(0),
   mSampleRate(0),
   mTunerGain(0)
{
   qDebug() << "created decoder service";

   // set default values
   mFrequency = mSettings.value("device.default/frequency", "13056000").toInt();
   mSampleRate = mSettings.value("device.default/sampleRate", "2400000").toInt();
   mTunerGain = mSettings.value("device.default/tunerGain", "0").toInt();

   // predefined armonics
   mFrequencyList.append("13.56 MHz (NFC band");
   mFrequencyList.append("27.12 MHz (13.56 2nd armonic)");
   mFrequencyList.append("40.68 MHz (13.56 3th armonic)");

   // create device scan thread
   //   QThreadPool::globalInstance()->start(new TaskRunner<DecoderService>(this, &DecoderService::scanner), QThread::LowestPriority);
}

NfcService::~NfcService()
{
   mScannerStatus = Stopped;
   mServiceStatus = Stopped;

   QMutexLocker scannerLock(&mScannerMutex);
   QMutexLocker decoderLock(&mServiceMutex);

   qDebug() << "destroy decoder service";
}

void NfcService::customEvent(QEvent * event)
{
   if (event->type() == DecoderControlEvent::Type)
      decoderControlEvent(static_cast<DecoderControlEvent*>(event));
   else if (event->type() == GainControlEvent::Type)
      gainControlEvent(static_cast<GainControlEvent*>(event));
}

void NfcService::decoderControlEvent(DecoderControlEvent *event)
{
   switch(event->command())
   {
      case DecoderControlEvent::Start:
      {
         decodeStart(event->getString("source"));
         break;
      }

      case DecoderControlEvent::Record:
      {
         decodeRecord(event->getString("source"));
         break;
      }

      case DecoderControlEvent::Stop:
      {
         decodeStop();
         break;
      }
   }
}

void NfcService::decodeStart(const QString &source)
{
   QString signalSource = source;

   if (signalSource.isNull() && !mDeviceList.isEmpty())
      signalSource = mDeviceList.first();

   qInfo() << "start decoding from" << signalSource;

   if (!signalSource.isEmpty())
   {
      // if decoder is running, terminate to change signal source
      if (mServiceStatus != Stopped)
      {
         qDebug() << "waiting for stop current decoding";

         mServiceStatus = Stopped;

         QMutexLocker decoderLock(&mServiceMutex);
      }

      if (!signalSource.isNull())
      {
         QString device = signalSource.left(signalSource.indexOf("://"));

         if (device != "record" || signalSource.endsWith(".wav"))
         {
            mSignalSource = signalSource;
            mSignalRecord = QString();

            if (mSettings.value("decoder/debugEnabled", "false").toBool())
            {
               mSignalRecord = mSettings.value("decoder/debugOutput", "decoder.wav").toString();
               mRecordChannels = mSettings.value("decoder/debugChannels", "0").toInt();
            }

            // capture time limit
            mTimeLimit = mSettings.value("decoder/timeLimit", "-1").toInt();

            // device default parameters
            mFrequency = mSettings.value("device." + device + "/frequency", "13056000").toInt();
            mSampleRate = mSettings.value("device." + device + "/sampleRate", "2400000").toInt();
            mTunerGain =  mSettings.value("device." + device + "/tunerGain", "0").toFloat();;

            qDebug() << "signal decoding from" << signalSource;

            QThreadPool::globalInstance()->start(new TaskRunner<NfcService>(this, &NfcService::decoderHandler), QThread::HighestPriority);
         }
      }
   }
}

void NfcService::decodeRecord(const QString &source)
{
   QString signalSource = source;

   if (signalSource.isNull() && !mDeviceList.isEmpty())
      signalSource = mDeviceList.first();

   qInfo() << "start recording from" << signalSource;

   if (!signalSource.isEmpty())
   {
      // if decoder is running, terminate to change signal source
      if (mServiceStatus != Stopped)
      {
         qDebug() << "waiting for stop current decoding";

         mServiceStatus = Stopped;

         QMutexLocker decoderLock(&mServiceMutex);
      }

      if (!signalSource.isNull())
      {
         QString device = signalSource.left(signalSource.indexOf("://"));

         if (device != "record")
         {
            mSignalSource = signalSource;
            mSignalRecord = mSettings.value("capture/captureOutput", "capture.wav").toString();
            mTimeLimit = mSettings.value("capture/timeLimit", "-1").toInt();

            // device default parameters
            mFrequency = mSettings.value("device." + device + "/frequency", "13056000").toInt();
            mSampleRate = mSettings.value("device." + device + "/sampleRate", "2400000").toInt();
            mTunerGain =  mSettings.value("device." + device + "/tunerGain", "0").toFloat();;

            qDebug() << "signal capture from" << mSignalSource << "to" << mSignalRecord;

            QThreadPool::globalInstance()->start(new TaskRunner<NfcService>(this, &NfcService::captureHandler), QThread::HighestPriority);
         }
      }
   }
}

void NfcService::decodeStop()
{
   qDebug() << "decoder stop";

   mServiceStatus = Stopped;
}


void NfcService::gainControlEvent(GainControlEvent *event)
{
   qDebug() << "decoder setTunerGain" << event->value();

   mTunerGain = event->value();
}

void NfcService::searchDevices()
{
   qInfo() << "decoder search devices";

   if (mServiceStatus == Stopped)
   {
      QStringList devices;

      devices.append(AirspyDevice::listDevices());
      devices.append(RealtekDevice::listDevices());
      //      devices.append(LimeDevice::listDevices());

      mDeviceList = devices;

      RadioDevice *radio;

      for (QStringList::ConstIterator device = devices.begin(); device != devices.end(); device++)
      {
         qInfo() << "device detected:" << *device;

         SignalDevice *source = SignalDevice::newInstance(*device);

         if ((radio = dynamic_cast<RadioDevice*>(source)))
         {
            radio->open(SignalDevice::ReadOnly);

            QVector<int> rates = radio->supportedSampleRates();

            for (QVector<int>::ConstIterator rate = rates.begin(); rate != rates.end(); rate++)
            {
               qInfo() << " supported samplerate:" << *rate;
            }

            radio->close();
         }

         delete source;
      }

      Dispatcher::post(StreamStatusEvent::create(Stopped)->setSourceList(devices));
   }
}

void NfcService::scannerHandler()
{
   try
   {
      qInfo() << "started device scanner";

      QMutexLocker lock(&mScannerMutex);

      mScannerStatus = Decoding;

      while (mScannerStatus != Stopped)
      {
         if (mServiceStatus == Stopped)
         {
            QStringList devices;

            devices.append(AirspyDevice::listDevices());
            devices.append(RealtekDevice::listDevices());
            //            devices.append(LimeDevice::listDevices());

            RadioDevice *radio;

            if (mDeviceList != devices)
            {
               mDeviceList = devices;

               for (QStringList::ConstIterator device = devices.begin(); device != devices.end(); device++)
               {
                  qInfo() << "device detected:" << *device;

                  SignalDevice *source = SignalDevice::newInstance(*device);

                  if ((radio = dynamic_cast<RadioDevice*>(source)))
                  {
                     radio->open(SignalDevice::ReadOnly);

                     QVector<int> rates = radio->supportedSampleRates();

                     for (QVector<int>::ConstIterator rate = rates.begin(); rate != rates.end(); rate++)
                     {
                        qInfo() << " supported samplerate:" << *rate;
                     }

                     radio->close();
                  }

                  delete source;
               }

               Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setSourceList(mDeviceList));
            }
         }

         QThread::msleep(1000);
      }

      qInfo() << "finished device scanner";
   }
   catch (...)
   {
      qInfo() << "finished device scanner, unexpected error!";
   }

   mScannerStatus = Stopped;
}

void NfcService::decoderHandler()
{
   try
   {
      qInfo() << "started decoder worker";

      QMutexLocker lock(&mServiceMutex);

      QElapsedTimer statusUpdateTimer;
      QElapsedTimer captureLimitTimer;

      statusUpdateTimer.start();
      captureLimitTimer.start();

      int count = 0;
      int status = Stopped;

      int frequency = 0;
      int sampleRate = 0;
      int tunerGain = 0;

      QStringList logEvent;

      qInfo() << "source device" << mSignalSource;

      // create source device
      SignalDevice *device = SignalDevice::newInstance(mSignalSource);

      // get interface for radio devices
      RadioDevice *radio = dynamic_cast<RadioDevice*>(device);

      // set radio parameters before open
      if (radio)
      {
         radio->setCenterFrequency(mFrequency);
         radio->setSampleRate(mSampleRate);
         radio->setReceiverGain(mTunerGain);
         radio->setAgcMode(RadioDevice::MIXER_AUTO);

         qInfo() << "set tuner gain" << tunerGain << "db";
         qInfo() << "set sample rate" << sampleRate;
         qInfo() << "set tuner frequency" << frequency << "Hz";
      }

      // create record device
      SignalDevice *capure = SignalDevice::newInstance(mSignalRecord);

      // open source device
      if (device && device->open(SignalDevice::ReadOnly))
      {
         qInfo() << "device open success";

         // notify supported receiver gains
         if (radio)
         {
            logEvent.append(QString("Radio Device : %1").arg(mSignalSource));
            logEvent.append(QString("  Tuner Gain  : %1 db").arg(radio->receiverGain(), -1, 'f', 2));
            logEvent.append(QString("  Frenquency  : %1 Mhz").arg(radio->centerFrequency() / 1E6, -1, 'f', 2));
            logEvent.append(QString("  Sample Rate : %1 Mhz").arg(radio->sampleRate() / 1E6, -1, 'f', 2));
            logEvent.append(QString("  Sample Size : %1 bits").arg(radio->sampleSize()));
         }
         else
         {
            logEvent.append(QString("File Source : %1").arg(mSignalSource));
            logEvent.append(QString("  Sample Rate : %1 Mhz").arg(device->sampleRate() / 1E6, -1, 'f', 2));
            logEvent.append(QString("  Sample Size : %1 bits").arg(device->sampleSize()));
         }

         // get interface for recorder
         RecordDevice *file = dynamic_cast<RecordDevice*>(capure);

         // open recording device
         if (file)
         {
            file->setSampleSize(16);
            file->setSampleRate(device->sampleRate());
            file->setSampleType(RecordDevice::INTEGER);
            file->setChannelCount(mRecordChannels);

            qInfo() << "set recording enabled";

            // open recording device
            if (file->open(SignalDevice::WriteOnly))
            {
               qInfo() << "recorder open success" << mSignalRecord;
            }
            else
            {
               qInfo() << "recorder open failed" << mSignalRecord;

               delete file;

               file = nullptr;
            }
         }

         Dispatcher::post(new ConsoleLogEvent(logEvent));

         NfcFrame frame;

         NfcDecoder decoder(device, file);

         decoder.setMaximunFrameLength(mSettings.value("decoder/maximunFrameLength", "64").toInt());
         decoder.setModulationThreshold(mSettings.value("decoder/modulationThrehold", "0.050").toFloat());
         decoder.setPowerLevelThreshold(mSettings.value("decoder/powerLevelThrehold", "0.050").toFloat());

         mServiceStatus = Decoding;

         qInfo() << "stream decoder started";

         while (mServiceStatus == Decoding)
         {
            // detect capture time limt
            if (captureLimitTimer.hasExpired(mTimeLimit))
               mServiceStatus = Stopped;

            // update decoder status
            if (status != mServiceStatus)
            {
               status = mServiceStatus;
               Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setSource(mSignalSource)->setFrequency(frequency)->setSampleRate(sampleRate)->setTunerGain(tunerGain));
            }

            // update receiver parameters
            if (radio)
            {
               if (tunerGain != mTunerGain)
               {
                  tunerGain = mTunerGain;
                  radio->setReceiverGain(tunerGain);
                  qInfo() << "set tuner gain" << tunerGain << "db";
                  Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setTunerGain(tunerGain));
               }

               if (sampleRate != mSampleRate)
               {
                  sampleRate = mSampleRate;
                  radio->setSampleRate(sampleRate);
                  qInfo() << "set sample rate" << sampleRate;
                  Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setSampleRate(sampleRate));
               }

               if (frequency != mFrequency)
               {
                  frequency = mFrequency;
                  radio->setCenterFrequency(frequency);
                  qInfo() << "set tuner frequency" << frequency;
                  Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setFrequency(frequency));
               }
            }

            if ((frame = decoder.nextFrame(100)))
            {
               count++;

               // send received frames
               if (mServiceStatus == Decoding)
               {
                  if (frame.isRequestFrame() || frame.isResponseFrame())
                  {
                     mStream->append(frame);
                  }

                  Dispatcher::post(new StreamFrameEvent(frame));
               }
            }
            else
            {
               qInfo() << "received EOF frame, finish decoder";

               mServiceStatus = Stopped;
            }

            // send signal changes
            if (statusUpdateTimer.hasExpired(250))
            {
               statusUpdateTimer.restart();
               Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setSignalPower(decoder.signalStrength() * 4)->setSampleCount(decoder.sampleCount())->setStreamProgress(float(captureLimitTimer.elapsed()) / float(mTimeLimit)));
            }
         }

         qInfo() << "stream decoder stopped";
         qInfo() << "total decoded frames" << count;
         qInfo() << "device"<< device->name() << "closed";

         device->close();

         logEvent.clear();
         logEvent.append(QString("Total decoded frames: %1").arg(count));

         Dispatcher::post(new ConsoleLogEvent(logEvent));
      }
      else
      {
         qWarning() << "stream open error";
      }

      qInfo() << "finished decoder worker";
   }
   catch (...)
   {
      qInfo() << "finished decoder worker, unexpected error!";
   }

   mServiceStatus = Stopped;

   // signal end of capture
   Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setStreamProgress(1));
}

void NfcService::captureHandler()
{
   try
   {
      qInfo() << "started recorder worker";

      QMutexLocker lock(&mServiceMutex);

      QElapsedTimer statusUpdateTimer;
      QElapsedTimer captureLimitTimer;

      statusUpdateTimer.start();
      captureLimitTimer.start();

      int count = 0;
      int status = Stopped;

      int frequency = mFrequency;
      int sampleRate = mSampleRate;
      int tunerGain = mTunerGain;

      QStringList logEvent;

      qInfo() << "source device" << mSignalSource;
      qInfo() << "recorder device" << mSignalRecord;

      // create source device
      SignalDevice *source = SignalDevice::newInstance(mSignalSource);

      // get interface for radio devices
      RadioDevice *radio = dynamic_cast<RadioDevice*>(source);

      // set radio parameters before open
      if (radio)
      {
         radio->setCenterFrequency(mFrequency);
         radio->setSampleRate(mSampleRate);
         radio->setReceiverGain(mTunerGain);
         radio->setAgcMode(RadioDevice::MIXER_AUTO);

         qInfo() << "set tuner gain" << tunerGain << "db";
         qInfo() << "set sample rate" << sampleRate;
         qInfo() << "set tuner frequency" << frequency << "Hz";
      }

      // create record device
      SignalDevice *target = SignalDevice::newInstance(mSignalRecord);

      // get interface for record devices
      RecordDevice *record = dynamic_cast<RecordDevice*>(target);

      // set recording parameters before open
      if (record)
      {
         record->setSampleSize(16);
         record->setSampleRate(source->sampleRate());
         record->setSampleType(RecordDevice::INTEGER);
         record->setChannelCount(1);
      }

      // open source device
      if (source && source->open(SignalDevice::ReadOnly))
      {
         qInfo() << "device open success";

         // open source device
         if (target && target->open(SignalDevice::WriteOnly))
         {
            qInfo() << "record open success";
         }

         // notify supported receiver gains
         if (radio)
         {
            logEvent.append(QString("Radio Device : %1").arg(mSignalSource));
            logEvent.append(QString("  Tuner Gain  : %1 db").arg(radio->receiverGain(), -1, 'f', 2));
            logEvent.append(QString("  Frenquency  : %1 Mhz").arg(radio->centerFrequency() / 1E6, -1, 'f', 2));
            logEvent.append(QString("  Sample Rate : %1 Mhz").arg(radio->sampleRate() / 1E6, -1, 'f', 2));
            logEvent.append(QString("  Sample Size : %1 bits").arg(radio->sampleSize()));
         }
         else
         {
            logEvent.append(QString("File Source : %1").arg(mSignalSource));
            logEvent.append(QString("  Sample Rate : %1 Mhz").arg(source->sampleRate() / 1E6, -1, 'f', 2));
            logEvent.append(QString("  Sample Size : %1 bits").arg(source->sampleSize()));
         }

         Dispatcher::post(new ConsoleLogEvent(logEvent));

         NfcCapture capture(source, target);

         mServiceStatus = Capture;

         qInfo() << "stream recorder started";

         while(mServiceStatus == Capture)
         {
            // detect capture time limt
            if (captureLimitTimer.hasExpired(mTimeLimit))
               mServiceStatus = Stopped;

            // update decoder status
            if (status != mServiceStatus)
            {
               status = mServiceStatus;
               Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setSource(mSignalSource)->setFrequency(frequency)->setSampleRate(sampleRate)->setTunerGain(tunerGain));
            }

            // update receiver parameters
            if (radio)
            {
               if (tunerGain != mTunerGain)
               {
                  tunerGain = mTunerGain;
                  radio->setReceiverGain(tunerGain);
                  qInfo() << "set tuner gain" << tunerGain << "db";
                  Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setTunerGain(tunerGain));
               }

               if (sampleRate != mSampleRate)
               {
                  sampleRate = mSampleRate;
                  radio->setSampleRate(sampleRate);
                  qInfo() << "set sample rate" << sampleRate;
                  Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setSampleRate(sampleRate));
               }

               if (frequency != mFrequency)
               {
                  frequency = mFrequency;
                  radio->setCenterFrequency(frequency);
                  qInfo() << "set tuner frequency" << frequency;
                  Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setFrequency(frequency));
               }
            }

            // capture raw blocks
            if (capture.process(100) > 0)
            {
               count++;
            }
            else
            {
               qInfo() << "received EOF, finish recording";

               mServiceStatus = Stopped;
            }

            // send signal changes
            if (statusUpdateTimer.hasExpired(250))
            {
               statusUpdateTimer.restart();
               Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setSignalPower(capture.signalStrength() * 4)->setSampleCount(capture.sampleCount())->setStreamProgress(float(captureLimitTimer.elapsed()) / float(mTimeLimit)));
            }
         }

         qInfo() << "stream recorder stopped";
         qInfo() << "total recorder blocks" << count;

         source->close();
         target->close();

         qInfo() << "source"<< source->name() << "closed";
         qInfo() << "target"<< target->name() << "closed";

         logEvent.clear();
         logEvent.append(QString("Total recorder frames: %1").arg(count));

         Dispatcher::post(new ConsoleLogEvent(logEvent));
      }
      else
      {
         qWarning() << "stream open error";
      }

      qInfo() << "finished recorder worker";
   }
   catch (...)
   {
      qInfo() << "finished recorder worker, unexpected error!";
   }

   mServiceStatus = Stopped;

   // signal end of capture
   Dispatcher::post(StreamStatusEvent::create(mServiceStatus)->setStreamProgress(1));

   // trigger process of recorded stream
   Dispatcher::post(new DecoderControlEvent(DecoderControlEvent::Start, "source", mSignalRecord));
}

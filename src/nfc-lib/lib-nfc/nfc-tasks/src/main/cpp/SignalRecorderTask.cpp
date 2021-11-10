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

#include <rt/Logger.h>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>
#include <sdr/RecordDevice.h>

#include <nfc/SignalRecorderTask.h>

#include "AbstractTask.h"

namespace nfc {

struct SignalRecorderTask::Impl : SignalRecorderTask, AbstractTask
{
   // decoder status
   int status;

   // signal buffer frame stream subject
   rt::Subject<sdr::SignalBuffer> *signalIqStream = nullptr;
   rt::Subject<sdr::SignalBuffer> *signalRvStream = nullptr;

   // signal stream subscription
   rt::Subject<sdr::SignalBuffer>::Subscription signalIqSubscription;
   rt::Subject<sdr::SignalBuffer>::Subscription signalRvSubscription;

   // signal stream queue buffer
   rt::BlockingQueue<sdr::SignalBuffer> signalQueue;

   // last status sent
   std::chrono::time_point<std::chrono::steady_clock> lastStatus;

   // record device
   std::shared_ptr<sdr::RecordDevice> device;

   Impl() : AbstractTask("SignalRecorderTask", "recorder"), status(SignalRecorderTask::Idle)
   {
      // access to signal subject stream
      signalIqStream = rt::Subject<sdr::SignalBuffer>::name("signal.iq");
      signalRvStream = rt::Subject<sdr::SignalBuffer>::name("signal.real");

      // subscribe to signal events
//      signalIqSubscription = signalIqStream->subscribe([this](const sdr::SignalBuffer &buffer) {
//         if (status == SignalRecorderTask::Writing || status == SignalRecorderTask::Capture)
//            signalQueue.add(buffer);
//      });

      signalRvSubscription = signalRvStream->subscribe([this](const sdr::SignalBuffer &buffer) {
         if (status == SignalRecorderTask::Writing || status == SignalRecorderTask::Capture)
            signalQueue.add(buffer);
      });
   }

   void start() override
   {
   }

   void stop() override
   {
      close();
   }

   bool loop() override
   {
      /*
       * first process pending commands
       */
      if (auto command = commandQueue.get())
      {
         log.info("recorder command [{}]", {command->code});

         if (command->code == SignalRecorderTask::Read)
         {
            readFile(command.value());
         }
         else if (command->code == SignalRecorderTask::Write)
         {
            writeFile(command.value());
         }
         else if (command->code == SignalRecorderTask::Stop)
         {
            closeFile(command.value());
         }
         else if (command->code == SignalRecorderTask::Capture)
         {
            startCapture(command.value());
         }
         else if (command->code == SignalRecorderTask::Replay)
         {
            startReplay(command.value());
         }
      }

      /*
       * now process device reading
       */
      if (status == SignalRecorderTask::Reading)
      {
         signalRead();
      }
      else if (status == SignalRecorderTask::Writing)
      {
         signalWrite();
      }
      else if (status == SignalRecorderTask::Capture)
      {
         signalCapture();
      }
      else if (status == SignalRecorderTask::Replaying)
      {
         signalReplay();
      }
      else
      {
         wait(50);
      }

      /*
      * update recorder status
      */
//      if ((std::chrono::steady_clock::now() - lastStatus) > std::chrono::milliseconds(500))
//      {
//         updateRecorderStatus();
//      }

      return true;
   }

   void readFile(const rt::Event &command)
   {
      if (auto file = command.get<std::string>("file"))
      {
         device = std::make_shared<sdr::RecordDevice>(file.value());

         signalQueue.clear();

         if (device->open(sdr::SignalDevice::Read))
         {
            if (device->channelCount() <= 2)
            {
               log.info("streaming started for file [{}]", {device->name()});

               command.resolve();

               updateRecorderStatus(SignalRecorderTask::Reading);
            }
            else
            {
               log.warn("too many channels in file [{}]", {device->name()});

               device.reset();

               command.reject();

               updateRecorderStatus(SignalRecorderTask::Idle);
            }
         }
         else
         {
            log.warn("unable to open file [{}]", {device->name()});

            device.reset();

            command.reject();

            updateRecorderStatus(SignalRecorderTask::Idle);
         }
      }
      else
      {
         command.reject();
      }
   }

   void writeFile(const rt::Event &command)
   {
      if (auto file = command.get<std::string>("file"))
      {
         device = std::make_shared<sdr::RecordDevice>(file.value());

         device->setSampleRate(10E6);
         device->setChannelCount(1);

         signalQueue.clear();

         if (device->open(sdr::SignalDevice::Write))
         {
            log.info("enable recording {}", {device->name()});

            command.resolve();

            updateRecorderStatus(SignalRecorderTask::Writing);
         }
         else
         {
            log.info("enable recording {} failed!", {device->name()});

            device.reset();

            command.reject();

            updateRecorderStatus(SignalRecorderTask::Idle);
         }
      }
      else
      {
         command.reject();
      }
   }

   void closeFile(const rt::Event &command)
   {
      close();

      command.resolve();

      updateRecorderStatus(SignalRecorderTask::Idle);
   }

   void startCapture(const rt::Event &command)
   {
      command.resolve();

      updateRecorderStatus(SignalRecorderTask::Buffering);
   }

   void startReplay(const rt::Event &command)
   {
      command.resolve();

      updateRecorderStatus(SignalRecorderTask::Replaying);
   }

   void signalRead()
   {
      if (device && device->isOpen())
      {
         int sampleRate = device->sampleRate();
         int channelCount = device->channelCount();
         int sampleOffset = device->sampleOffset();

         switch (channelCount)
         {
            case 1:
            {
               sdr::SignalBuffer buffer(65536 * channelCount, 1, sampleRate, sampleOffset, 0, sdr::SignalType::SAMPLE_REAL);

               if (device->read(buffer) > 0)
               {
                  signalRvStream->next(buffer);
               }

               break;
            }
            case 2:
            {
               sdr::SignalBuffer buffer(65536 * channelCount * 2, 2, sampleRate, sampleOffset, 0, sdr::SignalType::SAMPLE_IQ);

               if (device->read(buffer) > 0)
               {
                  sdr::SignalBuffer result(65536 * channelCount, 1, sampleRate, sampleOffset >> 1, 0, sdr::SignalType::SAMPLE_REAL);

                  buffer.stream([&result](const float *value, int stride) {
                     result.put(sqrtf(value[0] * value[0] + value[1] * value[1]));
                  });

                  result.flip();

                  signalIqStream->next(buffer);
                  signalRvStream->next(result);
               }

               break;
            }
         }

         if (device->isEof())
         {
            log.info("streaming finished for file [{}]", {device->name()});

            // send null buffer for EOF
            signalIqStream->next({});
            signalRvStream->next({});

            // close file
            device.reset();

            // update status
            updateRecorderStatus(SignalRecorderTask::Idle);
         }
      }
   }

   void signalWrite()
   {
      if (device && device->isOpen())
      {
         if (auto buffer = signalQueue.get())
         {
            if (!buffer->isEmpty())
            {
               device->write(buffer.value());
            }
         }
      }
   }

   void signalCapture()
   {
   }

   void signalReplay()
   {
   }

   void close()
   {
      device.reset();
   }

   void updateRecorderStatus(int value)
   {
      status = value;

      json data;

      // data status
      switch (status)
      {
         case Idle:
            data["status"] = "idle";
            break;
         case Reading:
            data["status"] = "reading";
            break;
         case Writing:
            data["status"] = "writing";
            break;
         case Buffering:
            data["status"] = "buffering";
            break;
         case Replaying:
            data["status"] = "replaying";
            break;
      }

      if (device)
      {
         data["file"] = device->name();
         data["channelCount"] = device->channelCount();
         data["sampleCount"] = device->sampleCount();
         data["sampleOffset"] = device->sampleOffset();
         data["sampleRate"] = device->sampleRate();
         data["sampleSize"] = device->sampleSize();
         data["sampleType"] = device->sampleType();
      }

      updateStatus(status, data);

      lastStatus = std::chrono::steady_clock::now();
   }
};

SignalRecorderTask::SignalRecorderTask() : rt::Worker("SignalRecorderTask")
{
}

rt::Worker *SignalRecorderTask::construct()
{
   return new SignalRecorderTask::Impl;
}

}

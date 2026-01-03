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

#if defined(__SSE2__) && defined(USE_SSE2)

#include <x86intrin.h>

#endif

#include <iomanip>
#include <sstream>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>
#include <hw/RecordDevice.h>

#include <lab/tasks/SignalStorageTask.h>

#include "AbstractTask.h"

namespace lab {

struct SignalStorageTask::Impl : SignalStorageTask, AbstractTask
{
   // decoder status
   int status;

   // signal buffer frame stream subject
   rt::Subject<hw::SignalBuffer> *radioSignalIqStream = nullptr;
   rt::Subject<hw::SignalBuffer> *radioSignalRawStream = nullptr;
   rt::Subject<hw::SignalBuffer> *logicSignalRawStream = nullptr;

   // signal stream subscription
   rt::Subject<hw::SignalBuffer>::Subscription radioSignalIqSubscription;
   rt::Subject<hw::SignalBuffer>::Subscription radioSignalRawSubscription;
   rt::Subject<hw::SignalBuffer>::Subscription logicSignalRawSubscription;

   // record device
   std::shared_ptr<hw::RecordDevice> logicStorage;
   std::shared_ptr<hw::RecordDevice> radioStorage;

   // signal stream queue buffer
   rt::BlockingQueue<hw::SignalBuffer> logicSignalQueue;
   rt::BlockingQueue<hw::SignalBuffer> radioSignalQueue;

   // signal keys vector
   std::vector<int> logicBufferKeys;
   std::vector<int> radioBufferKeys;

   // base filename
   std::string storagePath;

   Impl() : AbstractTask("worker.SignalStorage", "recorder"), status(Idle)
   {
      // access to signal subject stream
      radioSignalIqStream = rt::Subject<hw::SignalBuffer>::name("radio.signal.iq");
      radioSignalRawStream = rt::Subject<hw::SignalBuffer>::name("radio.signal.raw");
      logicSignalRawStream = rt::Subject<hw::SignalBuffer>::name("logic.signal.raw");

      // subscribe to signal events
      //      signalIqSubscription = signalIqStream->subscribe([this](const hw::SignalBuffer &buffer) {
      //         if (status == Writing || status == Capture)
      //            signalQueue.add(buffer);
      //      });

      radioSignalRawSubscription = radioSignalRawStream->subscribe([this](const hw::SignalBuffer &buffer) {
         if (status == Writing)
            radioSignalQueue.add(buffer);
      });

      logicSignalRawSubscription = logicSignalRawStream->subscribe([this](const hw::SignalBuffer &buffer) {
         if (status == Writing)
            logicSignalQueue.add(buffer);
      });
   }

   void start() override
   {
   }

   void stop() override
   {
   }

   bool loop() override
   {
      /*
       * first process pending commands
       */
      if (auto command = commandQueue.get())
      {
         log->debug("command [{}]", {command->code});

         switch (command->code)
         {
            case Read:
               readStorage(command.value());
               break;

            case Write:
               writeStorage(command.value());
               break;

            case Stop:
               closeStorage(command.value());
               break;

            default:
               log->warn("unknown command {}", {command->code});
               command->reject(UnknownCommand);
               return true;
         }
      }

      /*
       * now process device reading
       */
      if (status == Reading)
      {
         signalRead();
      }
      else if (status == Writing)
      {
         signalWrite();
      }
      else
      {
         wait(50);
      }

      return true;
   }

   void readStorage(const rt::Event &command)
   {
      int error = MissingParameters;

      while (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log->info("read file command: {}", {config.dump()});

         if (!config.contains("fileName"))
         {
            log->error("missing file name parameter!");
            error = MissingFileName;
            break;
         }

         std::vector<int> keys;

         auto storage = open(config["fileName"], 0, 0, 0, keys, hw::RecordDevice::Mode::Read);

         if (!storage)
         {
            error = FileOpenFailed;
            break;
         }

         if (storage->get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_SIZE) == hw::SAMPLE_SIZE_8)
         {
            logicStorage = storage;
            logicBufferKeys = keys;
         }
         else if (storage->get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_SIZE) == hw::SAMPLE_SIZE_16)
         {
            radioStorage = storage;
            radioBufferKeys = keys;
         }
         else
         {
            log->error("invalid storage format");
            error = InvalidStorageFormat;
            break;
         }

         logicSignalQueue.clear();
         radioSignalQueue.clear();

         command.resolve();

         updateStorageStatus(Reading);

         return;
      }

      command.reject(error);

      updateStorageStatus(Idle);
   }

   void writeStorage(const rt::Event &command)
   {
      int error = MissingParameters;

      while (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log->info("write command: {}", {config.dump()});

         if (!config.contains("storagePath"))
         {
            log->error("missing storage path parameter!");
            error = MissingStoragePath;
            break;
         }

         storagePath = config["storagePath"];

         log->info("data storage path: {}", {storagePath});

         logicSignalQueue.clear();
         radioSignalQueue.clear();

         command.resolve();

         updateStorageStatus(Writing);

         return;
      }

      command.reject(error);

      updateStorageStatus(Idle);
   }

   void closeStorage(const rt::Event &command)
   {
      if (logicStorage)
      {
         log->info("close storage file: {}", {logicStorage->get<std::string>(hw::SignalDevice::PARAM_DEVICE_NAME)});
         logicStorage->close();
         logicStorage.reset();
      }

      if (radioStorage)
      {
         log->info("close storage file: {}", {radioStorage->get<std::string>(hw::SignalDevice::PARAM_DEVICE_NAME)});
         radioStorage->close();
         radioStorage.reset();
      }

      command.resolve();

      updateStorageStatus(Idle);
   }

   void signalRead()
   {
      if (logicStorage)
         readLogic();

      if (radioStorage)
         readRadio();

      // check if all sources are closed
      if (!logicStorage && !radioStorage)
      {
         log->info("storage read finished!");

         updateStorageStatus(Idle);
      }
   }

   void signalWrite()
   {
      if (const auto buffer = logicSignalQueue.get())
         writeLogic(buffer.value());

      if (const auto buffer = radioSignalQueue.get())
         writeRadio(buffer.value());
   }

   std::shared_ptr<hw::RecordDevice> open(const std::string &filename, unsigned int sampleRate, unsigned int sampleSize, unsigned int channels, std::vector<int> &keys, hw::RecordDevice::Mode mode) const
   {
      auto storage = std::make_shared<hw::RecordDevice>(filename);

      if (mode == hw::RecordDevice::Mode::Write)
      {
         log->info("creating storage file {}, sampleRate {} sampleSize {} channels {}", {filename, sampleRate, sampleSize, channels});

         storage->set(hw::SignalDevice::PARAM_SAMPLE_RATE, sampleRate);
         storage->set(hw::SignalDevice::PARAM_SAMPLE_SIZE, sampleSize);
         storage->set(hw::SignalDevice::PARAM_CHANNEL_COUNT, channels);
         storage->set(hw::SignalDevice::PARAM_CHANNEL_KEYS, keys);
      }

      if (storage->open(mode))
      {
         log->info("successfully opened storage file: {}", {storage->get<std::string>(hw::SignalDevice::PARAM_DEVICE_NAME)});

         // read storage keys for channel interleaving
         keys = storage->get<std::vector<int>>(hw::SignalDevice::PARAM_CHANNEL_KEYS);

         return storage;
      }

      log->warn("unable to open storage file [{}]", {storage->get<std::string>(hw::SignalDevice::PARAM_DEVICE_NAME)});

      return nullptr;
   }

   void readLogic()
   {
      const unsigned int sampleRate = logicStorage->get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_RATE);
      const unsigned int channelCount = logicStorage->get<unsigned int>(hw::SignalDevice::PARAM_CHANNEL_COUNT);
      const unsigned int sampleOffset = logicStorage->get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_OFFSET);

      hw::SignalBuffer buffer(65536 * channelCount, channelCount, 1, sampleRate, sampleOffset, 0, hw::SignalType::SIGNAL_TYPE_LOGIC_SAMPLES);

      if (logicStorage->read(buffer) > 0)
      {
         log->debug("streaming logic [{}]: {} length {}", {buffer.id(), buffer.offset(), buffer.elements()});

         logicSignalRawStream->next(buffer);
      }

      if (logicStorage->isEof() || !logicStorage->isOpen())
      {
         log->info("streaming finished for file [{}]", {logicStorage->get<std::string>(hw::SignalDevice::PARAM_DEVICE_NAME)});

         // send null buffer for EOF
         logicSignalRawStream->next({});

         // close file
         logicStorage.reset();
      }
   }

   void readRadio()
   {
      const unsigned int sampleRate = radioStorage->get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_RATE);
      const unsigned int channelCount = radioStorage->get<unsigned int>(hw::SignalDevice::PARAM_CHANNEL_COUNT);
      const unsigned int sampleOffset = radioStorage->get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_OFFSET);

      switch (channelCount)
      {
         case 1:
         {
            hw::SignalBuffer buffer(65536 * channelCount, 1, 1, sampleRate, sampleOffset, 0, hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES);

            if (radioStorage->read(buffer) > 0)
            {
               log->debug("streaming radio [{}]: {} length {}", {buffer.id(), buffer.offset(), buffer.elements()});

               radioSignalRawStream->next(buffer);
            }

            break;
         }

         case 2:
         {
            hw::SignalBuffer buffer(65536 * channelCount, 2, 1, sampleRate, sampleOffset, 0, hw::SignalType::SIGNAL_TYPE_RADIO_IQ);

            if (radioStorage->read(buffer) > 0)
            {
               hw::SignalBuffer result(buffer.elements(), 1, 1, buffer.sampleRate(), buffer.offset(), 0, hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES);

               float *src = buffer.data();
               float *dst = result.push(buffer.elements());

               // compute real signal value
#if defined(__SSE2__) && defined(USE_SSE2)
               for (int j = 0, n = 0; j < buffer.elements(); j += 8, n += 16)
               {
                  // load 8 I/Q vectors
                  __m128 a1 = _mm_load_ps(src + n + 0); // I0, Q0, I1, Q1
                  __m128 a2 = _mm_load_ps(src + n + 4); // I2, Q2, I3, Q3
                  __m128 a3 = _mm_load_ps(src + n + 8); // I4, Q4, I5, Q5
                  __m128 a4 = _mm_load_ps(src + n + 12); // I6, Q6, I7, Q7

                  // square all components
                  __m128 p1 = _mm_mul_ps(a1, a1); // I0^2, Q0^2, I1^2, Q1^2
                  __m128 p2 = _mm_mul_ps(a2, a2); // I2^2, Q2^2, I3^2, Q3^2
                  __m128 p3 = _mm_mul_ps(a3, a3); // I4^2, Q4^2, I5^2, Q5^2
                  __m128 p4 = _mm_mul_ps(a4, a4); // I6^2, Q6^2, I6^2, Q7^2

                  // permute components
                  __m128 i1 = _mm_shuffle_ps(p1, p2, _MM_SHUFFLE(2, 0, 2, 0)); // I0^2, I1^2, I2^2, I3^2
                  __m128 i2 = _mm_shuffle_ps(p3, p4, _MM_SHUFFLE(2, 0, 2, 0)); // I4^2, I5^2, I6^2, I7^2
                  __m128 q1 = _mm_shuffle_ps(p1, p2, _MM_SHUFFLE(3, 1, 3, 1)); // Q0^2, Q1^2, Q2^2, Q3^2
                  __m128 q2 = _mm_shuffle_ps(p3, p4, _MM_SHUFFLE(3, 1, 3, 1)); // Q4^2, Q5^2, Q6^2, Q7^2

                  // add vector components
                  __m128 r1 = _mm_add_ps(i1, q1); // I0^2+Q0^2, I1^2+Q1^2, I2^2+Q2^2, I3^2+Q3^2
                  __m128 r2 = _mm_add_ps(i2, q2); // I4^2+Q4^2, I5^2+Q5^2, I6^2+Q6^2, I7^2+Q7^2

                  // square-root vectors
                  __m128 m1 = _mm_sqrt_ps(r1); // sqrt(I0^2+Q0^2), sqrt(I1^2+Q1^2), sqrt(I2^2+Q2^2), sqrt(I3^2+Q3^2)
                  __m128 m2 = _mm_sqrt_ps(r2); // sqrt(I4^2+Q4^2), sqrt(I5^2+Q5^2), sqrt(I6^2+Q6^2), sqrt(I7^2+Q7^2)

                  // store results
                  _mm_store_ps(dst + j + 0, m1);
                  _mm_store_ps(dst + j + 4, m2);
               }
#else
#pragma GCC ivdep
               for (int j = 0, n = 0; j < buffer.elements(); j += 4, n += 8)
               {
                  dst[j + 0] = sqrtf(src[n + 0] * src[n + 0] + src[n + 1] * src[n + 1]);
                  dst[j + 1] = sqrtf(src[n + 2] * src[n + 2] + src[n + 3] * src[n + 3]);
                  dst[j + 2] = sqrtf(src[n + 4] * src[n + 4] + src[n + 5] * src[n + 5]);
                  dst[j + 3] = sqrtf(src[n + 6] * src[n + 6] + src[n + 7] * src[n + 7]);
               }
#endif
               // flip buffer pointers
               result.flip();

               log->debug("streaming radio [{}]: {} length {}", {result.id(), result.offset(), result.elements()});

               // send IQ value buffer
               radioSignalIqStream->next(buffer);

               // send Real value buffer
               radioSignalRawStream->next(result);
            }

            break;
         }

         default:
         {
            radioStorage->close();
         }
      }

      if (radioStorage->isEof() || !radioStorage->isOpen())
      {
         log->info("streaming finished for file [{}]", {radioStorage->get<std::string>(hw::SignalDevice::PARAM_DEVICE_NAME)});

         // send null buffer for EOF
         radioSignalIqStream->next({});
         radioSignalRawStream->next({});

         // close file
         radioStorage.reset();
      }
   }

   bool writeLogic(const hw::SignalBuffer &buffer)
   {
      // buffer empty = EOF buffer, close storage and finish writing
      if (!buffer.isValid())
      {
         if (logicStorage)
         {
            log->warn("closing storage file: {}", {logicStorage->get<std::string>(hw::SignalDevice::PARAM_DEVICE_NAME)});

            // close storage
            logicStorage->close();
            logicStorage.reset();
         }

         return false;
      }

      // buffer type must be logic samples
      if (buffer.type() != hw::SignalType::SIGNAL_TYPE_LOGIC_SAMPLES)
         return false;

      // create storage file when first buffer is processed
      if (!logicStorage)
      {
         if (!((logicStorage = open(fileName("logic"), buffer.sampleRate(), hw::SAMPLE_SIZE_8, buffer.stride(), logicBufferKeys, hw::RecordDevice::Mode::Write))))
            return false;
      }

      // write buffer to storage
      return logicStorage->write(buffer) >= 0;
   }

   bool writeRadio(const hw::SignalBuffer &buffer)
   {
      // buffer empty = EOF buffer, close storage and finish writing
      if (!buffer.isValid())
      {
         if (radioStorage)
         {
            log->warn("closing storage file: {}", {logicStorage->get<std::string>(hw::SignalDevice::PARAM_DEVICE_NAME)});

            // close storage
            radioStorage->close();
            radioStorage.reset();
         }

         return false;
      }

      // buffer type must be radio samples or IQ samples
      if (buffer.type() != hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES)
         return false;

      // create new storage file before first buffer is completed
      if (!radioStorage)
      {
         if (!((radioStorage = open(fileName("radio"), buffer.sampleRate(), hw::SAMPLE_SIZE_16, buffer.stride(), radioBufferKeys, hw::RecordDevice::Mode::Write))))
            return false;
      }

      // write buffer to storage
      return radioStorage->write(buffer) >= 0;
   }

   std::string fileName(const std::string &type) const
   {
      std::ostringstream oss;
      std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      const std::tm *tm = std::localtime(&time);

      oss << storagePath << "/" << type << "-" << std::put_time(tm, "%Y%m%dT%H%M%S") << ".wav";

      return oss.str();
   }

   void updateStorageStatus(int value, const std::string &message = {})
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
         case Error:
            data["status"] = "error";
            break;
      }

      if (radioStorage)
      {
         data["file"] = radioStorage->get<std::string>(hw::SignalDevice::PARAM_DEVICE_NAME);
         data["channelCount"] = radioStorage->get<unsigned int>(hw::SignalDevice::PARAM_CHANNEL_COUNT);
         data["sampleCount"] = radioStorage->get<unsigned int>(hw::SignalDevice::PARAM_SAMPLES_READ);
         data["sampleOffset"] = radioStorage->get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_OFFSET);
         data["sampleRate"] = radioStorage->get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_RATE);
         data["sampleSize"] = radioStorage->get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_SIZE);
         data["sampleType"] = radioStorage->get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_TYPE);
         data["streamTime"] = radioStorage->get<unsigned int>(hw::SignalDevice::PARAM_STREAM_TIME);
      }

      if (!message.empty())
         data["message"] = message;

      updateStatus(status, data);
   }
};

SignalStorageTask::SignalStorageTask() : Worker("SignalStorage")
{
}

rt::Worker *SignalStorageTask::construct()
{
   return new Impl;
}

}

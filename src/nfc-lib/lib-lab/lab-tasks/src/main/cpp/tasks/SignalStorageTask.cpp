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

   // signal buffer cache
   hw::SignalBuffer logicBufferCache;
   hw::SignalBuffer radioBufferCache;

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

         if (command->code == Read)
         {
            storageRead(command.value());
         }
         else if (command->code == Write)
         {
            storageWrite(command.value());
         }
         else if (command->code == Stop)
         {
            storageClose(command.value());
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

   void storageRead(const rt::Event &command)
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

         if (std::get<unsigned int>(storage->get(hw::SignalDevice::PARAM_SAMPLE_SIZE)) == hw::SAMPLE_SIZE_8)
         {
            logicStorage = storage;
            logicBufferKeys = keys;
         }
         else if (std::get<unsigned int>(storage->get(hw::SignalDevice::PARAM_SAMPLE_SIZE)) == hw::SAMPLE_SIZE_16)
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

   void storageWrite(const rt::Event &command)
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

   void storageClose(const rt::Event &command)
   {
      if (logicStorage)
      {
         log->info("close storage file: {}", {std::get<std::string>(logicStorage->get(hw::SignalDevice::PARAM_DEVICE_NAME))});
         logicStorage->close();
         logicStorage.reset();
      }

      if (radioStorage)
      {
         log->info("close storage file: {}", {std::get<std::string>(radioStorage->get(hw::SignalDevice::PARAM_DEVICE_NAME))});
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

         // update status
         updateStorageStatus(Idle);
      }
   }

   void signalWrite()
   {
      bool writeFinished = false;

      if (auto buffer = logicSignalQueue.get())
      {
         if (!buffer->isEmpty())
         {
            switch (buffer->type())
            {
               case hw::SignalType::SIGNAL_TYPE_STM_LOGIC:
               {
                  // integrate new buffer interleaving with previous one store every offset change
                  if (interleaveBuffer(logicBufferKeys, logicBufferCache, *buffer))
                  {
                     // create new storage file before first buffer is completed
                     if (!logicStorage)
                     {
                        logicStorage = open(fileName("logic"), logicBufferCache.sampleRate(), hw::SAMPLE_SIZE_8, logicBufferCache.stride(), logicBufferKeys, hw::RecordDevice::Mode::Write);
                        writeFinished = !logicStorage;
                     }

                     if (!writeFinished)
                     {
                        // write buffer to storage
                        logicStorage->write(logicBufferCache);

                        // and start again with new buffer
                        logicBufferCache = *buffer;
                     }
                  }

                  break;
               }

               case hw::SignalType::SIGNAL_TYPE_RAW_LOGIC:
               {
                  // create new storage file before first buffer is completed
                  if (!logicStorage)
                  {
                     logicBufferKeys = std::vector<int>();

                     for (int i = 0; i < buffer->stride(); i++)
                        logicBufferKeys.push_back(i);

                     logicStorage = open(fileName("logic"), buffer->sampleRate(), hw::SAMPLE_SIZE_8, buffer->stride(), logicBufferKeys, hw::RecordDevice::Mode::Write);
                     writeFinished = !logicStorage;
                  }

                  if (!writeFinished)
                  {
                     // write buffer to storage
                     logicStorage->write(*buffer);
                  }
               }
            }
         }
         else if (logicStorage)
         {
            // write cache if has pending buffer
            if (logicBufferCache)
               logicStorage->write(logicBufferCache);

            // close storage
            logicStorage->close();
            logicStorage.reset();

            // mark as write finished if radio storage is also closed
            writeFinished = !radioStorage;
         }
      }

      if (auto buffer = radioSignalQueue.get())
      {
         if (!buffer->isEmpty())
         {
            // integrate new buffer interleaving with previous one store every offset change
            if (interleaveBuffer(radioBufferKeys, radioBufferCache, *buffer))
            {
               // create new storage file before first buffer is completed
               if (!radioStorage)
               {
                  radioStorage = open(fileName("radio"), radioBufferCache.sampleRate(), hw::SAMPLE_SIZE_16, radioBufferCache.stride(), radioBufferKeys, hw::RecordDevice::Mode::Write);
                  writeFinished = !radioStorage;
               }

               if (!writeFinished)
               {
                  // write buffer to storage
                  radioStorage->write(radioBufferCache);

                  // and start again with new buffer
                  radioBufferCache = *buffer;
               }
            }
         }
         else if (radioStorage)
         {
            // write cache if has pending buffer
            if (radioBufferCache)
               radioStorage->write(radioBufferCache);

            radioStorage->close();
            radioStorage.reset();

            // mark as write finished if radio storage is also closed
            writeFinished = !logicStorage;
         }
      }

      if (writeFinished)
      {
         log->info("storage write finished!");

         // reset storage
         logicStorage.reset();
         radioStorage.reset();

         // update status
         updateStorageStatus(Idle);
      }
   }

   static bool interleaveBuffer(std::vector<int> &keys, hw::SignalBuffer &cache, hw::SignalBuffer &buffer)
   {
      // integrate new buffer interleaving with previous cache
      if (cache && cache.offset() == buffer.offset())
      {
         // integrate new buffer interleaving with previous cache
         hw::SignalBuffer interleaved(cache.size() + buffer.size(), cache.stride() + buffer.stride(), 1, cache.sampleRate(), cache.offset(), cache.decimation(), cache.type());

         for (int i = 0; i < cache.elements(); i++)
         {
            // copy current buffer keep space to interleave new buffer
            cache.get(interleaved.pull(cache.stride()), cache.stride());

            // add incoming buffer
            buffer.get(interleaved.pull(buffer.stride()), buffer.stride());
         }

         // flip buffer pointers
         interleaved.flip();

         // use interleaved buffer as new cache
         cache = interleaved;
      }

      // initialize cache on first buffer
      else if (!cache)
      {
         cache = buffer;
      }

      // check if keys contains buffer id
      if (std::find(keys.begin(), keys.end(), buffer.id()) == keys.end())
         keys.push_back(static_cast<int>(buffer.id()));

      // return true if buffer offset has changed (cache is full and must be written)
      return cache.offset() != buffer.offset();
   }

   std::shared_ptr<hw::RecordDevice> open(const std::string &filename, unsigned int sampleRate, unsigned int sampleSize, unsigned int channels, std::vector<int> &keys, hw::RecordDevice::Mode mode)
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
         log->info("successfully opened storage file: {}", {std::get<std::string>(storage->get(hw::SignalDevice::PARAM_DEVICE_NAME))});

         // read storage keys for channel interleaving
         keys = std::get<std::vector<int>>(storage->get(hw::SignalDevice::PARAM_CHANNEL_KEYS));

         return storage;
      }

      log->warn("unable to open storage file [{}]", {std::get<std::string>(storage->get(hw::SignalDevice::PARAM_DEVICE_NAME))});

      return nullptr;
   }

   void readLogic()
   {
      unsigned int sampleRate = std::get<unsigned int>(logicStorage->get(hw::SignalDevice::PARAM_SAMPLE_RATE));
      unsigned int channelCount = std::get<unsigned int>(logicStorage->get(hw::SignalDevice::PARAM_CHANNEL_COUNT));
      unsigned int sampleOffset = std::get<unsigned int>(logicStorage->get(hw::SignalDevice::PARAM_SAMPLE_OFFSET));

      hw::SignalBuffer block(65536 * channelCount, channelCount, 1, 0, 0, 0, hw::SignalType::SIGNAL_TYPE_STM_LOGIC);

      if (logicStorage->read(block) > 0)
      {
         for (int c = 0; c < block.stride(); c++)
         {
            hw::SignalBuffer buffer(65536, 1, 1, sampleRate, sampleOffset / channelCount, 0, hw::SignalType::SIGNAL_TYPE_STM_LOGIC, c < logicBufferKeys.size() ? logicBufferKeys[c] : c);

            for (unsigned int i = 0; i < block.size(); i += block.stride())
            {
               buffer.put(block[c + i]);
            }

            buffer.flip();

            log->debug("streaming logic [{}]: {} length {}", {buffer.id(), buffer.offset(), buffer.elements()});

            logicSignalRawStream->next(buffer);
         }
      }

      if (logicStorage->isEof() || !logicStorage->isOpen())
      {
         log->info("streaming finished for file [{}]", {std::get<std::string>(logicStorage->get(hw::SignalDevice::PARAM_DEVICE_NAME))});

         // send null buffer for EOF
         logicSignalRawStream->next({});

         // close file
         logicStorage.reset();
      }
   }

   void readRadio()
   {
      unsigned int sampleRate = std::get<unsigned int>(radioStorage->get(hw::SignalDevice::PARAM_SAMPLE_RATE));
      unsigned int channelCount = std::get<unsigned int>(radioStorage->get(hw::SignalDevice::PARAM_CHANNEL_COUNT));
      unsigned int sampleOffset = std::get<unsigned int>(radioStorage->get(hw::SignalDevice::PARAM_SAMPLE_OFFSET));

      switch (channelCount)
      {
         case 1:
         {
            hw::SignalBuffer buffer(65536 * channelCount, 1, 1, sampleRate, sampleOffset, 0, hw::SignalType::SIGNAL_TYPE_RAW_REAL);

            if (radioStorage->read(buffer) > 0)
            {
               log->debug("streaming radio [{}]: {} length {}", {buffer.id(), buffer.offset(), buffer.elements()});

               radioSignalRawStream->next(buffer);
            }

            break;
         }

         case 2:
         {
            hw::SignalBuffer buffer(65536 * channelCount, 2, 1, sampleRate, sampleOffset >> 1, 0, hw::SignalType::SIGNAL_TYPE_RAW_IQ);

            if (radioStorage->read(buffer) > 0)
            {
               hw::SignalBuffer result(buffer.elements(), 1, 1, buffer.sampleRate(), buffer.offset(), 0, hw::SignalType::SIGNAL_TYPE_RAW_REAL);

               float *src = buffer.data();
               float *dst = result.pull(buffer.elements());

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
         log->info("streaming finished for file [{}]", {std::get<std::string>(radioStorage->get(hw::SignalDevice::PARAM_DEVICE_NAME))});

         // send null buffer for EOF
         radioSignalIqStream->next({});
         radioSignalRawStream->next({});

         // close file
         radioStorage.reset();
      }
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
         data["file"] = std::get<std::string>(radioStorage->get(hw::SignalDevice::PARAM_DEVICE_NAME));
         data["channelCount"] = std::get<unsigned int>(radioStorage->get(hw::SignalDevice::PARAM_CHANNEL_COUNT));
         data["sampleCount"] = std::get<unsigned int>(radioStorage->get(hw::SignalDevice::PARAM_SAMPLES_READ));
         data["sampleOffset"] = std::get<unsigned int>(radioStorage->get(hw::SignalDevice::PARAM_SAMPLE_OFFSET));
         data["sampleRate"] = std::get<unsigned int>(radioStorage->get(hw::SignalDevice::PARAM_SAMPLE_RATE));
         data["sampleSize"] = std::get<unsigned int>(radioStorage->get(hw::SignalDevice::PARAM_SAMPLE_SIZE));
         data["sampleType"] = std::get<unsigned int>(radioStorage->get(hw::SignalDevice::PARAM_SAMPLE_TYPE));
         data["streamTime"] = std::get<unsigned int>(radioStorage->get(hw::SignalDevice::PARAM_STREAM_TIME));
      }

      if (!message.empty())
         data["message"] = message;

      updateStatus(status, data);
   }
};

SignalStorageTask::SignalStorageTask() : Worker("SignalStorageTask")
{
}

rt::Worker *SignalStorageTask::construct()
{
   return new Impl;
}

}

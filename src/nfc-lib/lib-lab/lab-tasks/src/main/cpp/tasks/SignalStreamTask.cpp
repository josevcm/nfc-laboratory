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

#include <mutex>

#include <xtensor/xio.hpp>

#include <xtensor-zarr/xzarr_array.hpp>
#include <xtensor-zarr/xzarr_hierarchy.hpp>
#include <xtensor-zarr/xzarr_file_system_store.hpp>

#include <rt/BlockingQueue.h>
#include <rt/Downsampler.h>
#include <rt/Throughput.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <lab/tasks/SignalStreamTask.h>

#include "AbstractTask.h"

#define WINDOW 51
#define THRESHOLD 0.005
#define LOGIC_INTERVAL 255 // max 2^8-1 (1 byte)
#define RADIO_INTERVAL 255 // max 2^8-1 (1 byte)

namespace lab {

struct SignalStreamTask::Impl : SignalStreamTask, AbstractTask
{
   // signal subjects
   rt::Subject<hw::SignalBuffer> *logicSignalStream = nullptr;
   rt::Subject<hw::SignalBuffer> *radioSignalStream = nullptr;
   rt::Subject<hw::SignalBuffer> *signalStream = nullptr;

   // signal stream subscriptions
   rt::Subject<hw::SignalBuffer>::Subscription logicSignalSubscription;
   rt::Subject<hw::SignalBuffer>::Subscription radioSignalSubscription;

   // signal stream queue buffer
   rt::BlockingQueue<hw::SignalBuffer> signalQueue;

   // throughput meter
   rt::Throughput taskThroughput;

   // base filename
   rt::Downsampler radioDownsampler;

   // zarr hierarchy store
   xt::xzarr_file_system_store zarrFileStore;

   // create zarr hierarchy
   xt::xzarr_hierarchy<xt::xzarr_file_system_store> zarrHierarchy;

   xt::zarray signalArray;

   // stream lock
   std::mutex signalMutex;

   // last status sent
   std::chrono::time_point<std::chrono::steady_clock> lastStatus;

   unsigned int count = 0;

   explicit Impl() : AbstractTask("worker.SignalStream", "stream"),
                     radioDownsampler({0.000001}),
                     zarrFileStore("signal.zr3"),
                     zarrHierarchy(zarrFileStore),
                     lastStatus(std::chrono::steady_clock::now())
   {
      // access to radio signal subject stream
      logicSignalStream = rt::Subject<hw::SignalBuffer>::name("logic.signal.raw");

      // access to logic signal subject stream
      radioSignalStream = rt::Subject<hw::SignalBuffer>::name("radio.signal.raw");

      // access to signal subject stream
      signalStream = rt::Subject<hw::SignalBuffer>::name("stream.signal");

      // subscribe to logic signal events
      logicSignalSubscription = logicSignalStream->subscribe([=](const hw::SignalBuffer &buffer) {
         signalQueue.add(buffer);
      });

      // subscribe to radio signal events
      radioSignalSubscription = radioSignalStream->subscribe([=](const hw::SignalBuffer &buffer) {
         signalQueue.add(buffer);
      });

      createHierarchy();
   }

   ~Impl() override = default;

   void start() override
   {
      taskThroughput.begin();
   }

   void stop() override
   {
      taskThroughput.end();
   }

   bool loop() override
   {
      /*
       * first process pending commands
       */
      if (auto command = commandQueue.get())
      {
         switch (command->code)
         {
            case Query:
               // queryStream(command.value());
               break;
         }
      }

      /*
       * process signal queue
       */
      if (const auto buffer = signalQueue.get(10))
      {
         if (buffer.has_value())
         {
            // stream(buffer.value());
            // process(buffer.value());
         }
      }

      // trace task throughput
      if (std::chrono::steady_clock::now() - lastStatus > std::chrono::milliseconds(1000))
      {
         if (taskThroughput.average() > 0)
            log->info("average throughput {.2} Msps", {taskThroughput.average() / 1E6});

         // store last search time
         lastStatus = std::chrono::steady_clock::now();
      }

      return true;
   }

   void createHierarchy()
   {
      // create zarr hierarchy
      zarrHierarchy.create_hierarchy();

      // create radio signal array
      std::vector<size_t> shape = {0, 2};
      std::vector<size_t> chunk = {1000000, 2};

      // array options
      xt::xzarr_create_array_options options;
      options.chunk_memory_layout = 'C';
      options.chunk_separator = '/';
      options.chunk_pool_size = 10;
      options.fill_value = 0;

      signalArray = zarrHierarchy.create_array("lod/level0", shape, chunk, "<f4", options);
   }

   void stream(const hw::SignalBuffer &buffer)
   {
      const float *data = buffer.data();

      // propagate EOF
      if (!buffer.isValid())
      {
         queryRadioSignal(0, INFINITY);

         signalStream->next({});

         return;
      }

      switch (buffer.type())
      {
         // adaptive resample for raw real signal
         case hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES:
         {
            const double sampleStep = 1.0 / static_cast<double>(buffer.sampleRate());
            const double startTime = static_cast<double>(buffer.offset()) * sampleStep;

            // append raw samples to downsampler
            for (unsigned int i = 0; i < buffer.elements(); ++i)
            {
               const double time = std::fma(sampleStep, i, startTime); // time = sampleStep * i + startTime

               radioDownsampler.append(time, data[i]);
            }

            break;
         }

         // adaptive resample for stream logic signal
         case hw::SignalType::SIGNAL_TYPE_LOGIC_SAMPLES:
         {
            break;
         }

         default:
            break;
      }
   }

   void stream2(const hw::SignalBuffer &buffer)
   {
      const float *data = buffer.data();

      xt::xzarr_file_system_store store("test.zr3");
      auto root = xt::create_zarr_hierarchy(store);

      // xt::xio_blosc_config();

      // propagate EOF
      if (!buffer.isValid())
      {
         queryRadioSignal(0, INFINITY);

         signalStream->next({});

         return;
      }

      switch (buffer.type())
      {
         // adaptive resample for raw real signal
         case hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES:
         {
            const double sampleStep = 1.0 / static_cast<double>(buffer.sampleRate());
            const double startTime = static_cast<double>(buffer.offset()) * sampleStep;

            // append raw samples to downsampler
            for (unsigned int i = 0; i < buffer.elements(); ++i)
            {
               const double time = std::fma(sampleStep, i, startTime); // time = sampleStep * i + startTime

               radioDownsampler.append(time, data[i]);
            }

            break;
         }

         // adaptive resample for stream logic signal
         case hw::SignalType::SIGNAL_TYPE_LOGIC_SAMPLES:
         {
            break;
         }

         default:
            break;
      }
   }

   void queryStream(const rt::Event &command)
   {
      int error = MissingParameters;

      while (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log->info("query stream command: {}", {config.dump()});

         if (!config.contains("start"))
         {
            log->error("missing start time parameter!");
            error = MissingParameters;
            break;
         }

         if (!config.contains("end"))
         {
            log->error("missing end time parameter!");
            error = MissingParameters;
            break;
         }

         const auto start = static_cast<float>(config["start"]);
         const auto end = static_cast<float>(config["end"]);

         queryRadioSignal(start, end);

         command.resolve();

         return;
      }

      command.reject(error);
   }

   void queryRadioSignal(const double start, const double end) const
   {
      unsigned int querySampleCount = 1024;

      hw::SignalBuffer buffer(querySampleCount, 1, 1, 1, 0LL, 0, hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES, 0);

      auto buckets = radioDownsampler.query(start, end, 0.000010);

      if (auto it = buckets.begin(); it != buckets.end())
      {
         // limit start between start and start of first bucket
         const double queryTimeStart = std::max(start, buckets.front().t_min);
         const double queryTimeEnd = std::min(end, buckets.back().t_max);
         const double queryTimeSpan = std::abs(queryTimeEnd - queryTimeStart);

         // calculate query sample count based on time span
         const double querySampleRate = querySampleCount / queryTimeSpan;
         const double querySampleStep = 1.0 / querySampleRate;
         const double querySampleOffset = queryTimeStart * querySampleRate;

         buffer = hw::SignalBuffer(querySampleCount, 1, 1, static_cast<unsigned int>(querySampleRate), static_cast<unsigned long long>(querySampleOffset), 0, hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES, 0);

         for (int i = 0; i < buffer.elements() && it != buckets.end(); i++)
         {
            const double time = std::fma(querySampleStep, i, queryTimeStart);

            while (!(it->t_min <= time && it->t_max >= time)) ++it;

            buffer.put(it->y_min);
         }
      }

      buffer.flip();

      signalStream->next(buffer);
   }

   void process(const hw::SignalBuffer &buffer)
   {
      // propagate EOF
      if (!buffer.isValid())
      {
         signalStream->next({});
         return;
      }

      switch (buffer.type())
      {
         // adaptive resample for raw real signal
         case hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES:
         {
            processRadioSignal(buffer);
            break;
         }

         // adaptive resample for stream logic signal
         case hw::SignalType::SIGNAL_TYPE_LOGIC_SAMPLES:
         {
            processLogicSignal(buffer);
            break;
         }

         default:
            break;
      }
   }

   void processRadioSignal(const hw::SignalBuffer &buffer)
   {
      hw::SignalBuffer resampled(buffer.elements() * 2, 2, 1, buffer.sampleRate(), buffer.offset(), 0, hw::SignalType::SIGNAL_TYPE_RADIO_SIGNAL, buffer.id());

      float avrg = 0;
      float last = buffer[0];
      float filter = THRESHOLD;

      // initialize average
      for (int i = 0; i < (WINDOW / 2); i++)
         avrg += buffer[i];

      // always store first sample
      resampled.put(buffer[0]).put(0.0);

      // index of current point and last control point
      int i = 0, c = 0, p = -1;

      // adaptive resample based on maximum average deviation
      for (int r = i - (WINDOW / 2) - 1, a = i + (WINDOW / 2); i < buffer.limit(); ++i, ++p, ++a, ++r)
      {
         float value = buffer[i];

         // add new sample
         if (a < buffer.limit())
            avrg += buffer[a];

         // remove old sample
         if (r >= 0)
            avrg -= buffer[r];

         // detect deviation from average
         float stdev = std::abs(value - (avrg / static_cast<float>(WINDOW)));

         // store new sample if different from last or every RADIO_INTERVAL samples
         if (stdev > filter || (i - c) >= RADIO_INTERVAL)
         {
            // append control point
            if (stdev > filter && c < p)
               resampled.put(last).put(static_cast<float>(p));

            // append new value
            resampled.put(value).put(static_cast<float>(i));

            // update control point index
            c = i;
         }

         // store last value
         last = value;
      }

      // store last sample
      if (c < p)
         resampled.put(last).put(static_cast<float>(p));

      resampled.flip();

      signalStream->next(resampled);

      taskThroughput.update(buffer.elements());
   }

   void processLogicSignal(const hw::SignalBuffer &buffer)
   {
      unsigned int ch = buffer.stride();

#pragma omp parallel for default(none) shared(buffer, ch, signalStream, taskThroughput) schedule(static)
      for (unsigned int n = 0; n < ch; ++n)
      {
         if (n == 1)
            continue;

         hw::SignalBuffer resampled(buffer.elements() * 2, 2, 1, buffer.sampleRate(), buffer.offset(), 0, hw::SignalType::SIGNAL_TYPE_LOGIC_SIGNAL, n);

         // get value of the first sample of a channel
         float last = buffer[n];

         // and store in resampled buffer
         resampled.put(last).put(0.0);

         // adaptive resample based values changes (logic)
         for (unsigned int s = 1, i = ch + n, c = 0; i < buffer.limit(); ++s, i += ch)
         {
            float value = buffer[i];

            // store new sample if different from last or every LOGIC_INTERVAL samples
            if (value != last || (s - c) >= LOGIC_INTERVAL)
            {
               resampled.put(value).put(static_cast<float>(s));

               // update last value
               last = value;

               // update control point index
               c = s;
            }
         }

         resampled.flip();

         signalStream->next(resampled);
      }

      taskThroughput.update(buffer.elements());
   }
};

SignalStreamTask::SignalStreamTask() : Worker("SignalResampling")
{
}

rt::Worker *SignalStreamTask::construct()
{
   return new Impl;
}

}

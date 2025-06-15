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

#include <memory>

#include <rt/BlockingQueue.h>
#include <rt/Throughput.h>

#include <hw/DeviceFactory.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <hw/radio/RadioDevice.h>
#include <hw/radio/AirspyDevice.h>
#include <hw/radio/MiriDevice.h>
#include <hw/radio/RealtekDevice.h>

#include <lab/tasks/RadioDeviceTask.h>

#include "AbstractTask.h"

#define LOWER_GAIN_THRESHOLD 0.05
#define UPPER_GAIN_THRESHOLD 0.25

namespace lab {

struct RadioDeviceTask::Impl : RadioDeviceTask, AbstractTask
{
   // radio device
   std::shared_ptr<hw::RadioDevice> device;

   // signal stream subject for IQ data
   rt::Subject<hw::SignalBuffer> *signalIqStream = nullptr;

   // signal stream subject for raw data (magnitude)
   rt::Subject<hw::SignalBuffer> *signalRawStream = nullptr;

   // signal stream queue buffer
   rt::BlockingQueue<hw::SignalBuffer> signalQueue;

   // throughput meter
   rt::Throughput taskThroughput;

   // last detection attempt
   std::chrono::time_point<std::chrono::steady_clock> lastSearch;

   // current task status
   bool radioReceiverEnabled = false;

   // current receiver status
   int radioReceiverStatus = Idle;

   // current receiver gain mode
   bool receiverGainAuto = false;

   // last control offset
   unsigned int receiverGainChange = 0;

   // signal power
   double receiverSignalPower = 0;

   // current device configuration
   json currentConfig;

   Impl() : AbstractTask("worker.RadioDevice", "radio.receiver")
   {
      signalIqStream = rt::Subject<hw::SignalBuffer>::name("radio.signal.iq");
      signalRawStream = rt::Subject<hw::SignalBuffer>::name("radio.signal.raw");
   }

   void start() override
   {
      log->info("registering devices");

      hw::DeviceFactory::registerDevice("radio.airspy", []() -> std::vector<std::string> { return hw::AirspyDevice::enumerate(); }, [](const std::string &name) -> hw::RadioDevice *{ return new hw::AirspyDevice(name); });
      hw::DeviceFactory::registerDevice("radio.rtlsdr", []() -> std::vector<std::string> { return hw::RealtekDevice::enumerate(); }, [](const std::string &name) -> hw::RealtekDevice *{ return new hw::RealtekDevice(name); });
      hw::DeviceFactory::registerDevice("radio.miri", []() -> std::vector<std::string> { return hw::MiriDevice::enumerate(); }, [](const std::string &name) -> hw::MiriDevice *{ return new hw::MiriDevice(name); });
   }

   void stop() override
   {
      if (device)
      {
         log->info("shutdown device {}", {std::get<std::string>(device->get(hw::RadioDevice::PARAM_DEVICE_NAME))});
         device.reset();
      }

      updateDeviceStatus(Idle);
   }

   bool loop() override
   {
      /*
       * process pending commands
       */
      if (auto command = commandQueue.get())
      {
         log->debug("command [{}]", {command->code});

         switch (command->code)
         {
            case Start:
               startDevice(command.value());
               break;

            case Stop:
               stopDevice(command.value());
               break;

            case Pause:
               pauseDevice(command.value());
               break;

            case Resume:
               resumeDevice(command.value());
               break;

            case Query:
               queryDevice(command.value());
               break;

            case Configure:
               configDevice(command.value());
               break;

            case Clear:
               clearDevice(command.value());
               break;

            default:
               log->warn("unknown command {}", {command->code});
               command->reject(UnknownCommand);
               return true;
         }
      }

      /*
       * process device refresh
       */
      if (radioReceiverEnabled)
      {
         if (std::chrono::steady_clock::now() - lastSearch > std::chrono::milliseconds(1000))
         {
            if (!device || !device->isStreaming())
            {
               refresh();
            }
            else if (taskThroughput.average() > 0)
            {
               log->info("average throughput {.2} Msps, {} pending buffers", {taskThroughput.average() / 1E6, signalQueue.size()});
            }

            // store last search time
            lastSearch = std::chrono::steady_clock::now();
         }

         processQueue(50);
      }
      else
      {
         wait(100);
      }

      return true;
   }

   void refresh()
   {
      if (!device)
      {
         for (const auto &name: hw::DeviceFactory::enumerate("radio"))
         {
            log->info("detected device {}", {name});

            // create device instance
            device.reset(hw::DeviceFactory::newInstance<hw::RadioDevice>(name));

            if (!device)
               continue;

            // try to open...
            if (device->open(hw::RadioDevice::Mode::Read))
            {
               log->info("device {} connected!", {name});

               setup(currentConfig);

               updateDeviceStatus(Idle, true);

               return;
            }

            device.reset();

            log->warn("device {} open failed", {name});
         }
      }
      else if (!device->isReady())
      {
         log->warn("device {} disconnected", {std::get<std::string>(device->get(hw::RadioDevice::PARAM_DEVICE_NAME))});

         // send null buffer for EOF
         signalIqStream->next({});

         // send null buffer for EOF
         signalRawStream->next({});

         // close device
         device.reset();

         // update receiver status
         updateDeviceStatus(Absent);

         return;
      }

      // update receiver status
      updateDeviceStatus(radioReceiverStatus);
   }

   void setup(const json &config)
   {
      log->info("applying configuration: {}", {config.dump()});

      // set parameters
      if (config.contains("centerFreq"))
         device->set(hw::RadioDevice::PARAM_TUNE_FREQUENCY, static_cast<unsigned int>(config["centerFreq"]));

      if (config.contains("sampleRate"))
         device->set(hw::RadioDevice::PARAM_SAMPLE_RATE, static_cast<unsigned int>(config["sampleRate"]));

      if (config.contains("tunerAgc"))
         device->set(hw::RadioDevice::PARAM_TUNER_AGC, static_cast<unsigned int>(config["tunerAgc"]));

      if (config.contains("mixerAgc"))
         device->set(hw::RadioDevice::PARAM_MIXER_AGC, static_cast<unsigned int>(config["mixerAgc"]));

      if (config.contains("biasTee"))
         device->set(hw::RadioDevice::PARAM_BIAS_TEE, static_cast<unsigned int>(config["biasTee"]));

      if (config.contains("directSampling"))
         device->set(hw::RadioDevice::PARAM_DIRECT_SAMPLING, static_cast<unsigned int>(config["directSampling"]));

      if (config.contains("gainValue"))
         device->set(hw::RadioDevice::PARAM_GAIN_VALUE, static_cast<unsigned int>(config["gainValue"]));

      if (config.contains("gainMode"))
      {
         receiverGainAuto = config["gainMode"] == 0;

         if (receiverGainAuto)
         {
            device->set(hw::RadioDevice::PARAM_GAIN_MODE, 1);
            device->set(hw::RadioDevice::PARAM_GAIN_VALUE, 0);
         }
         else
         {
            device->set(hw::RadioDevice::PARAM_GAIN_MODE, static_cast<unsigned int>(config["gainMode"]));
         }
      }
   }

   void startDevice(const rt::Event &command)
   {
      if (!radioReceiverEnabled)
      {
         log->warn("device is disabled");
         command.reject(TaskDisabled);
         return;
      }

      if (device)
      {
         log->info("start streaming for device {}", {std::get<std::string>(device->get(hw::RadioDevice::PARAM_DEVICE_NAME))});

         // reset receiver values
         receiverGainChange = 0;

         // reset throughput meter
         taskThroughput.begin();

         // start receiving
         device->start([this](hw::SignalBuffer &buffer) {
            signalQueue.add(buffer);
         });

         // resolve command
         command.resolve();

         updateDeviceStatus(Streaming);
      }
   }

   void stopDevice(const rt::Event &command)
   {
      if (!radioReceiverEnabled)
      {
         log->warn("device is disabled");
         command.reject(TaskDisabled);
         return;
      }

      if (device)
      {
         log->info("stop streaming for device {}", {std::get<std::string>(device->get(hw::RadioDevice::PARAM_DEVICE_NAME))});

         // stop underline receiver
         device->stop();

         // resolve command
         command.resolve();

         // set receiver in flush buffers status
         updateDeviceStatus(Flush);
      }
   }

   void pauseDevice(const rt::Event &command)
   {
      if (!radioReceiverEnabled)
      {
         log->warn("device is disabled");
         command.reject(TaskDisabled);
         return;
      }

      if (device)
      {
         log->info("pause streaming for device {}", {std::get<std::string>(device->get(hw::RadioDevice::PARAM_DEVICE_NAME))});

         // pause underline receiver
         device->pause();

         // resolve command
         command.resolve();

         // set receiver in flush buffers status
         updateDeviceStatus(Paused);
      }
   }

   void resumeDevice(const rt::Event &command)
   {
      if (!radioReceiverEnabled)
      {
         log->warn("device is disabled");
         command.reject(TaskDisabled);
         return;
      }

      if (device)
      {
         log->info("resume streaming for device {}", {std::get<std::string>(device->get(hw::RadioDevice::PARAM_DEVICE_NAME))});

         // resume underline receiver
         device->resume();

         // resolve command
         command.resolve();

         // set receiver in flush buffers status
         updateDeviceStatus(Streaming);
      }
   }

   void queryDevice(const rt::Event &command)
   {
      log->debug("query status");

      command.resolve();

      updateDeviceStatus(radioReceiverStatus, true);
   }

   void configDevice(const rt::Event &command)
   {
      if (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log->info("change config: {}", {config.dump()});

         // update current configuration
         currentConfig.merge_patch(config);

         // check if receiver is enabled
         if (config.contains("enabled"))
            radioReceiverEnabled = config["enabled"];

         // apply configuration to device
         if (device)
         {
            // update device configuration
            setup(currentConfig);

            // stop streaming if device is streaming when disabled is requested
            if (!radioReceiverEnabled && device->isStreaming())
            {
               log->info("stop streaming");

               // stop underline receiver
               device->stop();

               // flush pending buffers before disable
               radioReceiverStatus = Flush;
            }
         }

         command.resolve();

         updateDeviceStatus(radioReceiverStatus, config.contains("enabled") && radioReceiverEnabled);
      }
      else
      {
         log->warn("invalid config data");

         command.reject(InvalidConfig);
      }
   }

   void clearDevice(const rt::Event &command)
   {
      log->info("clear signal queue with {} pending buffers", {signalQueue.size()});

      signalQueue.clear();

      command.resolve();
   }

   void updateDeviceStatus(int status, bool full = false)
   {
      json data;

      radioReceiverStatus = status;

      if (device)
      {
         // device name and status
         data["name"] = std::get<std::string>(device->get(hw::RadioDevice::PARAM_DEVICE_NAME));
         data["vendor"] = std::get<std::string>(device->get(hw::RadioDevice::PARAM_DEVICE_VENDOR));
         data["model"] = std::get<std::string>(device->get(hw::RadioDevice::PARAM_DEVICE_MODEL));
         data["version"] = std::get<std::string>(device->get(hw::RadioDevice::PARAM_DEVICE_VERSION));
         data["serial"] = std::get<std::string>(device->get(hw::RadioDevice::PARAM_DEVICE_SERIAL));

         // device parameters
         data["centerFreq"] = std::get<unsigned int>(device->get(hw::RadioDevice::PARAM_TUNE_FREQUENCY));
         data["sampleRate"] = std::get<unsigned int>(device->get(hw::RadioDevice::PARAM_SAMPLE_RATE));
         data["streamTime"] = std::get<unsigned int>(device->get(hw::RadioDevice::PARAM_STREAM_TIME));
         data["gainMode"] = std::get<unsigned int>(device->get(hw::RadioDevice::PARAM_GAIN_MODE));
         data["gainValue"] = std::get<unsigned int>(device->get(hw::RadioDevice::PARAM_GAIN_VALUE));
         data["mixerAgc"] = std::get<unsigned int>(device->get(hw::RadioDevice::PARAM_MIXER_AGC));
         data["tunerAgc"] = std::get<unsigned int>(device->get(hw::RadioDevice::PARAM_TUNER_AGC));
         data["biasTee"] = std::get<unsigned int>(device->get(hw::RadioDevice::PARAM_BIAS_TEE));
         data["directSampling"] = std::get<unsigned int>(device->get(hw::RadioDevice::PARAM_DIRECT_SAMPLING));

         // device statistics
         data["samplesRead"] = std::get<long long>(device->get(hw::RadioDevice::PARAM_SAMPLES_READ));
         data["samplesLost"] = std::get<long long>(device->get(hw::RadioDevice::PARAM_SAMPLES_LOST));

         if (!radioReceiverEnabled)
            data["status"] = "disabled";
         else if (device->isPaused())
            data["status"] = "paused";
         else if (device->isStreaming())
            data["status"] = "streaming";
         else if (status == Flush)
            data["status"] = "flush";
         else
            data["status"] = "idle";

         // send capabilities on data attach
         if (full)
         {
            data["gainModes"].push_back({
               {"value", 0},
               {"name", "Auto"}
            });

            // get device capabilities
            rt::Catalog gainModes = std::get<rt::Catalog>(device->get(hw::RadioDevice::PARAM_SUPPORTED_GAIN_MODES));
            rt::Catalog gainValues = std::get<rt::Catalog>(device->get(hw::RadioDevice::PARAM_SUPPORTED_GAIN_VALUES));
            rt::Catalog sampleRates = std::get<rt::Catalog>(device->get(hw::RadioDevice::PARAM_SUPPORTED_SAMPLE_RATES));

            for (const auto &entry: gainModes)
            {
               if (entry.first > 0)
               {
                  data["gainModes"].push_back({
                     {"value", entry.first},
                     {"name", entry.second}
                  });
               }
            }

            for (const auto &entry: gainValues)
            {
               data["gainValues"].push_back({
                  {"value", entry.first},
                  {"name", entry.second}
               });
            }

            for (const auto &entry: sampleRates)
            {
               data["sampleRates"].push_back({
                  {"value", entry.first},
                  {"name", entry.second}
               });
            }
         }
      }
      else
      {
         data["status"] = radioReceiverEnabled ? "absent" : "disabled";
      }

      updateStatus(status, data);
   }

   void processQueue(int timeout)
   {
      if (auto entry = signalQueue.get(timeout))
      {
         hw::SignalBuffer buffer = entry.value();
         hw::SignalBuffer result(buffer.elements(), 1, 1, buffer.sampleRate(), buffer.offset(), 0, hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES, buffer.id());

         float *src = buffer.data();
         float *dst = result.pull(buffer.elements());
         float avrg = 0;
         float powr = 0;

         // compute real signal value and average value
#if defined(__SSE2__) && defined(USE_SSE2)
         for (int j = 0, n = 0; j < buffer.elements(); j += 16, n += 32)
         {
            // load 16 I/Q vectors
            __m128 a0 = _mm_load_ps(src + n + 0); // I0, Q0, I1, Q1
            __m128 a1 = _mm_load_ps(src + n + 4); // I2, Q2, I3, Q3
            __m128 a2 = _mm_load_ps(src + n + 8); // I4, Q4, I5, Q5
            __m128 a3 = _mm_load_ps(src + n + 12); // I6, Q6, I7, Q7
            __m128 a4 = _mm_load_ps(src + n + 16); // I8, Q8, I9, Q9
            __m128 a5 = _mm_load_ps(src + n + 20); // I10, Q10, I11, Q11
            __m128 a6 = _mm_load_ps(src + n + 24); // I12, Q12, I13, Q13
            __m128 a7 = _mm_load_ps(src + n + 28); // I14, Q14, I15, Q15

            // square all components
            __m128 p0 = _mm_mul_ps(a0, a0); // I0^2, Q0^2, I1^2, Q1^2
            __m128 p1 = _mm_mul_ps(a1, a1); // I2^2, Q2^2, I3^2, Q3^2
            __m128 p2 = _mm_mul_ps(a2, a2); // I4^2, Q4^2, I5^2, Q5^2
            __m128 p3 = _mm_mul_ps(a3, a3); // I6^2, Q6^2, I7^2, Q7^2
            __m128 p4 = _mm_mul_ps(a4, a4); // I8^2, Q8^2, I9^2, Q9^2
            __m128 p5 = _mm_mul_ps(a5, a5); // I10^2, Q10^2, I11^2, Q11^2
            __m128 p6 = _mm_mul_ps(a6, a6); // I12^2, Q12^2, I13^2, Q13^2
            __m128 p7 = _mm_mul_ps(a7, a7); // I14^2, Q14^2, I15^2, Q15^2

            // permute components
            __m128 i0 = _mm_shuffle_ps(p0, p1, _MM_SHUFFLE(2, 0, 2, 0)); // I0^2, I1^2, I2^2, I3^2
            __m128 i1 = _mm_shuffle_ps(p2, p3, _MM_SHUFFLE(2, 0, 2, 0)); // I4^2, I5^2, I6^2, I7^2
            __m128 i2 = _mm_shuffle_ps(p4, p5, _MM_SHUFFLE(2, 0, 2, 0)); // I8^2, I9^2, I10^2, I11^2
            __m128 i3 = _mm_shuffle_ps(p6, p7, _MM_SHUFFLE(2, 0, 2, 0)); // I12^2, I13^2, I14^2, I15^2
            __m128 q0 = _mm_shuffle_ps(p0, p1, _MM_SHUFFLE(3, 1, 3, 1)); // Q0^2, Q1^2, Q2^2, Q3^2
            __m128 q1 = _mm_shuffle_ps(p2, p3, _MM_SHUFFLE(3, 1, 3, 1)); // Q4^2, Q5^2, Q6^2, Q7^2
            __m128 q2 = _mm_shuffle_ps(p4, p5, _MM_SHUFFLE(3, 1, 3, 1)); // Q8^2, Q9^2, Q10^2, Q11^2
            __m128 q3 = _mm_shuffle_ps(p6, p7, _MM_SHUFFLE(3, 1, 3, 1)); // Q12^2, Q13^2, Q14^2, Q15^2

            // add vector components
            __m128 r0 = _mm_add_ps(i0, q0); // I0^2+Q0^2,   I1^2+Q1^2,   I2^2+Q2^2,   I3^2+Q3^2
            __m128 r1 = _mm_add_ps(i1, q1); // I4^2+Q4^2,   I5^2+Q5^2,   I6^2+Q6^2,   I7^2+Q7^2
            __m128 r2 = _mm_add_ps(i2, q2); // I8^2+Q8^2,   I9^2+Q1^2,   I10^2+Q10^2, I11^2+Q11^2
            __m128 r3 = _mm_add_ps(i3, q3); // I12^2+Q12^2, I13^2+Q13^2, I14^2+Q14^2, I15^2+Q15^2

            // add all components
            __m128 w0 = _mm_add_ps(r0, r1); // I0^2+Q0^2+I4^2+Q4^2,   I1^2+Q1^2+I5^2+Q5^2,   I2^2+Q2^2+I6^2+Q6^2,     I3^2+Q3^2+I7^2+Q7^2
            __m128 w1 = _mm_add_ps(r2, r3); // I8^2+Q8^2+I12^2+Q12^2, I9^2+Q1^2+I13^2+Q13^2, I10^2+Q10^2+I14^2+Q14^2, I11^2+Q11^2+I15^2+Q15^2
            __m128 pt = _mm_add_ps(w0, w1); // sum ALL

            // square-root vectors
            __m128 m0 = _mm_sqrt_ps(r0); // sqrt(I0^2+Q0^2), sqrt(I1^2+Q1^2), sqrt(I2^2+Q2^2), sqrt(I3^2+Q3^2)
            __m128 m1 = _mm_sqrt_ps(r1); // sqrt(I4^2+Q4^2), sqrt(I5^2+Q5^2), sqrt(I6^2+Q6^2), sqrt(I7^2+Q7^2)
            __m128 m2 = _mm_sqrt_ps(r2); // sqrt(I8^2+Q8^2), sqrt(I9^2+Q9^2), sqrt(I10^2+Q10^2), sqrt(I11^2+Q11^2)
            __m128 m3 = _mm_sqrt_ps(r3); // sqrt(I12^2+Q12^2), sqrt(I13^2+Q13^2), sqrt(I14^2+Q14^2), sqrt(I15^2+Q15^2)

            // store results
            _mm_store_ps(dst + j + 0, m0);
            _mm_store_ps(dst + j + 4, m1);
            _mm_store_ps(dst + j + 8, m2);
            _mm_store_ps(dst + j + 12, m3);

            // compute exponential average
            avrg = avrg * (1 - 0.001f) + dst[j + 0] * 0.001f;
            avrg = avrg * (1 - 0.001f) + dst[j + 4] * 0.001f;
            avrg = avrg * (1 - 0.001f) + dst[j + 8] * 0.001f;
            avrg = avrg * (1 - 0.001f) + dst[j + 12] * 0.001f;

            // sum all squares to compute signal power
            powr += pt[0] + pt[1] + pt[2] + pt[3];
         }
#else
#pragma GCC ivdep
         for (int j = 0, n = 0; j < buffer.elements(); j += 4, n += 8)
         {
            float p0 = src[n + 0] * src[n + 0] + src[n + 1] * src[n + 1]; // I0^2 + Q0^2
            float p1 = src[n + 2] * src[n + 2] + src[n + 3] * src[n + 3]; // I1^2 + Q1^2
            float p2 = src[n + 4] * src[n + 4] + src[n + 5] * src[n + 5]; // I2^2 + Q2^2
            float p3 = src[n + 6] * src[n + 6] + src[n + 7] * src[n + 7]; // I3^2 + Q3^2

            dst[j + 0] = sqrtf(p0); // sqrt(I0^2+Q0^2)
            dst[j + 1] = sqrtf(p1); // sqrt(I1^2+Q1^2)
            dst[j + 2] = sqrtf(p2); // sqrt(I2^2+Q2^2)
            dst[j + 3] = sqrtf(p3); // sqrt(I3^2+Q3^2)

            avrg = avrg * (1 - 0.001f) + dst[j + 0] * 0.001f;

            powr += p0 + p1 + p2 + p3; // I0^2 + Q0^2 + I1^2 + Q1^2 + I2^2 + Q2^2 + I3^2 + Q3^2
         }
#endif

         // update current signal power
         receiverSignalPower = powr / buffer.elements();

         // flip buffer pointers
         result.flip();

         // send IQ value buffer
         signalIqStream->next(buffer);

         // send Real value buffer
         signalRawStream->next(result);

         // update receiver throughput
         taskThroughput.update(buffer.elements());

         // if automatic gain control is engaged adjust gain dynamically
         if (receiverGainAuto && buffer.offset() > receiverGainChange)
         {
            int gainValue = std::get<int>(device->get(hw::RadioDevice::PARAM_GAIN_VALUE));

            // for weak signals, increase receiver gain
            if (avrg < LOWER_GAIN_THRESHOLD && gainValue < 6)
            {
               receiverGainChange = buffer.offset() + buffer.elements();
               device->set(hw::RadioDevice::PARAM_GAIN_VALUE, gainValue + 1);
               log->info("increase gain {}", {std::get<int>(device->get(hw::RadioDevice::PARAM_GAIN_VALUE))});
            }

            // for strong signals, decrease receiver gain
            if (avrg > UPPER_GAIN_THRESHOLD && gainValue > 0)
            {
               receiverGainChange = buffer.offset() + buffer.elements();
               device->set(hw::RadioDevice::PARAM_GAIN_VALUE, gainValue - 1);
               log->info("decrease gain {}", {std::get<int>(device->get(hw::RadioDevice::PARAM_GAIN_VALUE))});
            }
         }
      }
      else if (radioReceiverStatus == Flush)
      {
         log->info("flush receiver buffers");

         // send null buffer for EOF
         signalIqStream->next({});
         signalRawStream->next({});

         // flush finished
         updateDeviceStatus(Idle);
      }
   }
};

RadioDeviceTask::RadioDeviceTask() : Worker("RadioDeviceTask")
{
}

rt::Worker *RadioDeviceTask::construct()
{
   return new Impl;
}
}

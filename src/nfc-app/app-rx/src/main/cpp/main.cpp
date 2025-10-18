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

#ifdef __WIN32

#include <windows.h>

#else
#include <signal.h>
#endif

#include <unistd.h>

#include <mutex>
#include <condition_variable>
#include <iostream>

#include <rt/Executor.h>
#include <rt/Subject.h>
#include <rt/Event.h>
#include <rt/Logger.h>
#include <rt/BlockingQueue.h>

#include <lab/nfc/Nfc.h>
#include <lab/data/RawFrame.h>

#include <lab/tasks/RadioDecoderTask.h>
#include <lab/tasks/RadioDeviceTask.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Main
{
   rt::Logger *log = rt::Logger::getLogger("app:main");

   // frame type catalog
   const std::map<unsigned int, std::string> frameType {
      {lab::FrameType::NfcCarrierOff, "CarrierOff"},
      {lab::FrameType::NfcCarrierOn, "CarrierOn"},
      {lab::FrameType::NfcPollFrame, "PCD->PICC"},
      {lab::FrameType::NfcListenFrame, "PICC->PCD"}
   };

   // frame tech catalog
   const std::map<unsigned int, std::string> frameTech {
      {lab::FrameTech::NoneTech, "None"},
      {lab::FrameTech::NfcATech, "NfcA"},
      {lab::FrameTech::NfcBTech, "NfcB"},
      {lab::FrameTech::NfcFTech, "NfcD"},
      {lab::FrameTech::NfcVTech, "NfcV"}
   };

   // frame rate catalog
   const std::map<unsigned int, std::string> frameRate {
      {lab::NfcRateType::r106k, "106"},
      {lab::NfcRateType::r212k, "212"},
      {lab::NfcRateType::r424k, "424"},
      {lab::NfcRateType::r848k, "848"},
   };

   // default receiver paramerers
   const json defaultReceiverParams = {

      // airspy
      {
         "radio.airspy", {
            {"centerFreq", 40680000},
            {"sampleRate", 10000000},
            {"gainMode", 1}, // linearity
            {"gainValue", 4}, // 4db
            {"mixerAgc", 0},
            {"tunerAgc", 0},
            {"biasTee", 0},
            {"directSampling", 0}
         }
      },

      // RTLSDR
      {
         "radio.rtlsdr", {
            {"centerFreq", 27120000},
            {"sampleRate", 3200000},
            {"gainMode", 1}, // manual
            {"gainValue", 77}, // 7.7db
            {"mixerAgc", 0},
            {"tunerAgc", 0},
            {"biasTee", 0},
            {"directSampling", 0}
         }
      }
   };

   std::mutex mutex;
   std::condition_variable sync;
   std::atomic_bool terminate = false;

   // create executor service
   rt::Executor executor {1, 4};

   // streams subjects
   rt::Subject<rt::Event> *receiverStatusStream = nullptr;
   rt::Subject<rt::Event> *receiverCommandStream = nullptr;
   rt::Subject<rt::Event> *decoderStatusStream = nullptr;
   rt::Subject<rt::Event> *decoderCommandStream = nullptr;
   rt::Subject<lab::RawFrame> *decoderFrameStream = nullptr;

   // streams subscriptions
   rt::Subject<rt::Event>::Subscription receiverStatusSubscription;
   rt::Subject<rt::Event>::Subscription decoderStatusSubscription;
   rt::Subject<lab::RawFrame>::Subscription decoderFrameSubscription;

   // frame stream queue buffer
   rt::BlockingQueue<lab::RawFrame> frameQueue;

   // decoder status and default parameters
   bool decoderConfigured = false;
   json decoderStatus {};
   json decoderParams {
      {"debugEnabled", false},
      {
         "protocol", {
            {"nfca", {{"enabled", true}}},
            {"nfcb", {{"enabled", true}}},
            {"nfcf", {{"enabled", true}}},
            {"nfcv", {{"enabled", true}}}
         }
      }
   };

   // receiver status and parameters
   bool receiverConfigured = false;
   json receiverStatus {};
   json receiverParams {};

   Main()
   {
   }

   void init()
   {
      log->info("NFC laboratory, 2024 Jose Vicente Campos Martinez");

      // create processing tasks
      executor.submit(lab::RadioDecoderTask::construct());
      executor.submit(lab::RadioDeviceTask::construct());

      // create receiver streams
      receiverStatusStream = rt::Subject<rt::Event>::name("radio.receiver.status");
      receiverCommandStream = rt::Subject<rt::Event>::name("radio.receiver.command");

      // create decoder streams
      decoderStatusStream = rt::Subject<rt::Event>::name("radio.decoder.status");
      decoderCommandStream = rt::Subject<rt::Event>::name("radio.decoder.command");
      decoderFrameStream = rt::Subject<lab::RawFrame>::name("radio.decoder.frame");

      // handler for decoder status events
      receiverStatusSubscription = receiverStatusStream->subscribe([&](const rt::Event &event) {
         receiverStatus = json::parse(event.get<std::string>("data").value());
      });

      // subscribe to decoder status
      decoderStatusSubscription = decoderStatusStream->subscribe([&](const rt::Event &event) {
         decoderStatus = json::parse(event.get<std::string>("data").value());
      });

      // subscribe to decoder frames
      decoderFrameSubscription = decoderFrameStream->subscribe([&](const lab::RawFrame &frame) {
         frameQueue.add(frame);
      });

      // enable receiver & decoder
      const json enable = {{"enabled", true}};
      receiverCommandStream->next({lab::RadioDeviceTask::Configure, {{"data", enable.dump()}}});
      decoderCommandStream->next({lab::RadioDecoderTask::Configure, {{"data", enable.dump()}}});
   }

   int checkReceiverStatus()
   {
      // wait until receiver status is available
      if (receiverStatus.empty() || receiverStatus["status"].is_null() || receiverStatus["status"] == "absent" || !receiverStatus["name"].is_string())
         return 0;

      // update decoder sample rate
      decoderParams["sampleRate"] = receiverStatus["sampleRate"];

      // check receiver parameters
      std::string name = receiverStatus["name"];
      std::string type = name.substr(0, name.find(':'));

      // check ir receiver is supported
      if (!defaultReceiverParams.contains(type))
      {
         log->error("unknown receiver: {}", {name});
         return -1;
      }

      // get required settings from default values for this receiver
      json required = defaultReceiverParams[type];

      // override with command line parameters
      if (!receiverParams.is_null())
         required.update(receiverParams);

      // detect required changes
      const json config = detectChanges(receiverStatus, required);

      // if no configuration needed, just start receiver
      receiverConfigured = config.empty();

      // send configuration update
      if (!receiverConfigured)
      {
         printf("receiver configuration: %s\n", config.dump().c_str());
         receiverCommandStream->next({lab::RadioDeviceTask::Configure, [=] { receiverConfigured = true; }, nullptr, {{"data", config.dump()}}});
      }

      // if receiver is configured and idle, start it
      if (receiverConfigured && receiverStatus["status"] == "idle")
      {
         log->info("start receiver streaming");
         receiverCommandStream->next({lab::RadioDeviceTask::Start, [=] { receiverStatus["status"] = "waiting"; }});
      }

      return 0;
   }

   int checkDecoderStatus()
   {
      // wait until receiver status is available
      if (decoderStatus.empty())
         return 0;

      // check decoder status
      if (decoderStatus["status"].is_null())
      {
         log->info("invalid decoder!");
         return -1;
      }

      // wait until samplerate is configured
      if (decoderParams["sampleRate"].is_null())
         return 0;

      // detect required changes
      const json config = detectChanges(decoderStatus, decoderParams);

      // if no configuration needed, just start decoder
      decoderConfigured = config.empty();

      // send configuration update
      if (!decoderConfigured)
      {
         printf("decoder configuration: %s\n", config.dump().c_str());
         decoderCommandStream->next({lab::RadioDecoderTask::Configure, [=] { decoderConfigured = true; }, nullptr, {{"data", config.dump()}}});
      }

      // if decoder is configured and idle, start it
      if (decoderConfigured && decoderStatus["status"] == "idle")
      {
         log->info("start decoder streaming");
         decoderCommandStream->next({lab::RadioDecoderTask::Start, [=] { decoderStatus["status"] = "waiting"; }});
      }

      return 0;
   }

   static json detectChanges(const json &ref, const json &set)
   {
      json result;

      for (auto &entry: set.items())
      {
         if (!ref.contains(entry.key()))
            continue;

         if (entry.value().is_object())
         {
            if (json tmp = detectChanges(ref[entry.key()], entry.value()); !tmp.empty())
               result[entry.key()] = tmp;
         }

         else if (ref[entry.key()] != entry.value())
         {
            result[entry.key()] = entry.value();
         }
      }

      return result;
   }

   void printFrame(const lab::RawFrame &frame) const
   {
      int offset = 0;
      char buffer[16384];

      // add datagram time
      offset += snprintf(buffer + offset, sizeof(buffer), "%010.3f ", frame.timeStart());

      // add frame type
      offset += snprintf(buffer + offset, sizeof(buffer), "(%s) ", frameType.at(frame.frameType()).c_str());

      // data frames
      if (frame.frameType() == lab::FrameType::NfcPollFrame || frame.frameType() == lab::FrameType::NfcListenFrame)
      {
         // add tech type
         offset += snprintf(buffer + offset, sizeof(buffer), "[%s@%.0f]: ", frameTech.at(frame.techType()).c_str(), roundf(float(frame.frameRate()) / 1000.0f));

         // add data as HEX string
         for (int i = 0; i < frame.size(); i++)
         {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%02X ", static_cast<unsigned int>(frame[i]));
         }
      }

      // send to stdout
      fprintf(stdout, "%s\n", buffer);
   }

   void finish()
   {
      // shutdown all tasks
      executor.shutdown();

      // shutdown main loop
      terminate = true;

      // notify
      sync.notify_all();
   }

   int run(const int argc, char *argv[])
   {
      int opt;
      int nsecs = -1;
      char *endptr = nullptr;

      while ((opt = getopt(argc, argv, "vdp:t:f:s:")) != -1)
      {
         switch (opt)
         {
            // enable verbose mode
            case 'v':
            {
               // first level, INFO
               if (rt::Logger::getRootLevel() < rt::Logger::INFO_LEVEL)
                  rt::Logger::setRootLevel(rt::Logger::INFO_LEVEL);

                  // consecutive levels, up tu TRACE
               else if (rt::Logger::getRootLevel() < rt::Logger::TRACE_LEVEL)
                  rt::Logger::setRootLevel(rt::Logger::getRootLevel() + 1);

               break;
            }

            // enable signal debug mode
            case 'd':
            {
               decoderParams["debugEnabled"] = true;
               break;
            }

            // enable protocols
            case 'p':
            {
               std::string protocols = optarg;
               decoderParams["protocol"]["nfca"]["enabled"] = protocols.find("nfca") != std::string::npos;
               decoderParams["protocol"]["nfcb"]["enabled"] = protocols.find("nfcb") != std::string::npos;
               decoderParams["protocol"]["nfcf"]["enabled"] = protocols.find("nfcf") != std::string::npos;
               decoderParams["protocol"]["nfcv"]["enabled"] = protocols.find("nfcv") != std::string::npos;
               break;
            }

            // set running frequency
            case 'f':
            {
               receiverParams["centerFreq"] = strtol(optarg, &endptr, 10);

               if (endptr == optarg)
               {
                  fprintf(stderr, "Invalid value for 'f' argument\n");
                  showUsage();
                  return -1;
               }

               break;
            }

            // set running frequency
            case 's':
            {
               receiverParams["sampleRate"] = strtol(optarg, &endptr, 10);

               if (endptr == optarg)
               {
                  fprintf(stderr, "Invalid value for 's' argument\n");
                  showUsage();
                  return -1;
               }

               break;
            }

            // limit running time
            case 't':
            {
               nsecs = strtol(optarg, &endptr, 10);

               if (endptr == optarg)
               {
                  fprintf(stderr, "Invalid value for 't' argument\n");
                  showUsage();
                  return -1;
               }

               break;
            }

            default:
               showUsage();
               return -1;
         }
      }

      // get start time
      const auto start = std::chrono::steady_clock::now();

      // initialize
      init();

      // main loot until capture finished
      while (!terminate)
      {
         std::unique_lock lock(mutex);

         // wait for signal or timeout
         sync.wait_for(lock, std::chrono::milliseconds(500));

         // check termination flag and exit now
         if (terminate)
            break;

         // check receiver status
         if (checkReceiverStatus() < 0)
         {
            fprintf(stdout, "Finish capture, invalid receiver!\n");
            finish();
         }

         // check decoder status
         if (checkDecoderStatus() < 0)
         {
            fprintf(stdout, "Finish capture, invalid decoder!\n");
            finish();
         }

         // wait until time limit reached and exit
         if (nsecs > 0 && std::chrono::steady_clock::now() - start > std::chrono::seconds(nsecs))
         {
            fprintf(stdout, "Finish capture, time limit reached!\n");
            finish();
         }

         // process received frames
         while (auto frame = frameQueue.get())
         {
            printFrame(frame.value());
         }

         // flush console output
         fflush(stdout);
      }

      return 0;
   }

   static void showUsage()
   {
      fprintf(stderr, "Usage: [-v] [-d] [-p nfca,nfcb,nfcf,nfcv] [-f frequency] [-s samplerate] [-t nsecs]\n");
      fprintf(stderr, "\tv: verbose mode, write logging information to stderr\n");
      fprintf(stderr, "\td: debug mode, write WAV file with raw decoding signals (highly affected performance!)\n");
      fprintf(stderr, "\tp: enable protocols, by default all are enabled\n");
      fprintf(stderr, "\tf: receiver frequency in Hz\n");
      fprintf(stderr, "\ts: receiver samplerate\n");
      fprintf(stderr, "\tt: stop capture after number of seconds\n");
   }

} *app;

#ifdef __WIN32

WINBOOL intHandler(DWORD sig)
{
   fprintf(stderr, "Terminate on signal %lu\n", sig);
   app->finish();
   return true;
}

#else
void intHandler(int sig)
{
   fprintf(stderr, "Terminate on signal %d\n", sig);
   app->finish();
}
#endif

int main(int argc, char *argv[])
{
   // send logging events to stderr
   rt::Logger::init(std::cerr);

   // disable logging at all (can be enabled with -v option)
   rt::Logger::setRootLevel(rt::Logger::NONE_LEVEL);

   // register signals handlers
#ifdef __WIN32
   SetConsoleCtrlHandler(intHandler, TRUE);
#else
   signal(SIGINT, intHandler);
   signal(SIGTERM, intHandler);
#endif

   // create main object
   Main main;

   // set global pointer for signal handlers
   app = &main;

   // and run
   return main.run(argc, argv);
}

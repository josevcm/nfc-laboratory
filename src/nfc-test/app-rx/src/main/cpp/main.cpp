/*

  Copyright (c) 2024 Jose Vicente Campos Martinez - <josevcm@gmail.com>

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

#include <nfc/Nfc.h>
#include <nfc/NfcFrame.h>

#include <nfc/FrameDecoderTask.h>
#include <nfc/SignalReceiverTask.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Main
{
   rt::Logger log {"Main"};

   // frame type catalog
   std::map<unsigned int, std::string> frameType {
         {nfc::FrameType::CarrierOff,  "CarrierOff"},
         {nfc::FrameType::CarrierOn,   "CarrierOn"},
         {nfc::FrameType::PollFrame,   "PCD->PICC"},
         {nfc::FrameType::ListenFrame, "PICC->PCD"}
   };

   // frame tech catalog
   std::map<unsigned int, std::string> frameTech {
         {nfc::TechType::None, "None"},
         {nfc::TechType::NfcA, "NfcA"},
         {nfc::TechType::NfcB, "NfcB"},
         {nfc::TechType::NfcF, "NfcD"},
         {nfc::TechType::NfcV, "NfcV"}
   };

   // frame rate catalog
   std::map<unsigned int, std::string> frameRate {
         {nfc::RateType::r106k, "106"},
         {nfc::RateType::r212k, "212"},
         {nfc::RateType::r424k, "424"},
         {nfc::RateType::r848k, "848"},
   };

   // default receiver parameters for rtlsdr device
   json rtlsdrReceiverParams = {
         {"centerFreq",     27120000},
         {"sampleRate",     3200000},
         {"gainMode",       1},
         {"gainValue",      77},
         {"directSampling", 0}
   };

   // default receiver parameters for airspy device
   json airspyReceiverParams = {
         {"centerFreq", 40680000},
         {"sampleRate", 10000000},
         {"gainMode",   1},
         {"gainValue",  3},
         {"mixerAgc",   0},
         {"biasTee",    0}
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
   rt::Subject<nfc::NfcFrame> *decoderFrameStream = nullptr;

   // streams subscriptions
   rt::Subject<rt::Event>::Subscription receiverStatusSubscription;
   rt::Subject<rt::Event>::Subscription decoderStatusSubscription;
   rt::Subject<nfc::NfcFrame>::Subscription decoderFrameSubscription;

   // current decoder status
   bool decoderConfigured = false;
   json decoderStatus {};
   json decoderParams {
         {"debugEnabled", false},
         {"nfca",         {{"enabled", true}}},
         {"nfcb",         {{"enabled", true}}},
         {"nfcf",         {{"enabled", true}}},
         {"nfcv",         {{"enabled", true}}}
   };

   // current receiver status
   bool receiverConfigured = false;
   json receiverStatus {};
   json receiverParams {};

   Main()
   {
      log.info("NFC laboratory, 2024 Jose Vicente Campos Martinez");
   }

   void init()
   {
      // create processing tasks
      executor.submit(nfc::FrameDecoderTask::construct());
      executor.submit(nfc::SignalReceiverTask::construct());

      // create receiver streams
      receiverStatusStream = rt::Subject<rt::Event>::name("receiver.status");
      receiverCommandStream = rt::Subject<rt::Event>::name("receiver.command");

      // create decoder streams
      decoderStatusStream = rt::Subject<rt::Event>::name("decoder.status");
      decoderCommandStream = rt::Subject<rt::Event>::name("decoder.command");
      decoderFrameStream = rt::Subject<nfc::NfcFrame>::name("decoder.frame");

      // handler for decoder status events
      receiverStatusSubscription = receiverStatusStream->subscribe([&](const rt::Event &event) {
         receiverStatus = json::parse(event.get<std::string>("data").value());
      });

      // subscribe to decoder status
      decoderStatusSubscription = decoderStatusStream->subscribe([&](const rt::Event &event) {
         decoderStatus = json::parse(event.get<std::string>("data").value());
      });

      // subscribe to decoder frames
      decoderFrameSubscription = decoderFrameStream->subscribe([&](const nfc::NfcFrame &frame) {
         handleDecoderFrame(frame);
      });
   }

   void handleDecoderFrame(const nfc::NfcFrame &frame)
   {
      int offset = 0;
      char buffer[16384];

      // add datagram time
      offset += snprintf(buffer + offset, sizeof(buffer), "%010.3f ", frame.timeStart());

      // add frame type
      offset += snprintf(buffer + offset, sizeof(buffer), "(%s) ", frameType[frame.frameType()].c_str());

      // data frames
      if (frame.isPollFrame() || frame.isListenFrame())
      {
         // add tech type
         offset += snprintf(buffer + offset, sizeof(buffer), "[%s@%.0f]: ", frameTech[frame.techType()].c_str(), roundf(float(frame.frameRate()) / 1000.0f));

         // add data as HEX string
         for (int i = 0; i < frame.size(); i++)
         {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%02X ", (unsigned int) frame[i]);
         }
      }

      // send to stdout
      fprintf(stdout, "%s\n", buffer);
   }

   int checkReceiverStatus()
   {
      // wait until receiver status is available
      if (receiverStatus.empty())
         return 0;

      // if no receiver detected, finish...
      if (receiverStatus["status"].is_null() || receiverStatus["status"] == "absent")
      {
         log.info("no receiver found!");
         return -1;
      }

      // if no receiver name, finish...
      if (!receiverStatus["name"].is_string())
      {
         log.info("no receiver name found!");
         return -1;
      }

      // update decoder sample rate
      if (receiverStatus["sampleRate"].is_number())
         decoderParams["sampleRate"] = receiverStatus["sampleRate"];

      // check receiver parameters
      json params;
      json config;
      std::string name = receiverStatus["name"];

      // default parameters for AirSpy
      if (name.find("airspy") == 0)
      {
         params = airspyReceiverParams;
      }
         // default parameters for Rtl SDR
      else if (name.find("rtlsdr") == 0)
      {
         params = rtlsdrReceiverParams;
      }
      else
      {
         log.error("unknown receiver: {}", {name});
         return -1;
      }

      for (auto &param: params.items())
      {
         if (receiverStatus[param.key()] != param.value())
         {
            config[param.key()] = param.value();
         }
      }

      // if no configuration needed, just start receiver
      receiverConfigured = config.empty();

      // send configuration update
      if (!receiverConfigured)
      {
         log.info("set receiver configuration: {}", {config.dump()});
         receiverCommandStream->next({nfc::SignalReceiverTask::Configure, [=]() { receiverConfigured = true; }, nullptr, {{"data", config.dump()}}});
      }

      // if receiver is configured and idle, start it
      if (receiverConfigured && receiverStatus["status"] == "idle")
      {
         receiverCommandStream->next({nfc::SignalReceiverTask::Start, [=]() { receiverStatus["status"] = "waiting"; }});
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
         log.info("invalid decoder!");
         return -1;
      }

      json config;

      for (auto &param: decoderParams.items())
      {
         if (decoderStatus[param.key()] != param.value())
         {
            config[param.key()] = param.value();
         }
      }

      // if no configuration needed, just start decoder
      decoderConfigured = config.empty();

      // send configuration update
      if (!decoderConfigured)
      {
         log.info("set decoder configuration: {}", {config.dump()});
         decoderCommandStream->next({nfc::FrameDecoderTask::Configure, [=]() { decoderConfigured = true; }, nullptr, {{"data", config.dump()}}});
      }

      // if decoder is configured and idle, start it
      if (decoderConfigured && decoderStatus["status"] == "idle")
      {
         decoderCommandStream->next({nfc::FrameDecoderTask::Start, [=]() { decoderStatus["status"] = "waiting"; }});
      }

      return 0;
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

   int run(int argc, char *argv[])
   {
      int opt;
      int nsecs = -1;
      char *endptr = nullptr;

      while ((opt = getopt(argc, argv, "vdp:t:")) != -1)
      {
         switch (opt)
         {
            // enable verbose mode
            case 'v':
            {
               if (rt::Logger::getWriterLevel() < rt::Logger::INFO_LEVEL)
                  rt::Logger::setWriterLevel(rt::Logger::INFO_LEVEL);
               else if (rt::Logger::getWriterLevel() < rt::Logger::TRACE_LEVEL)
                  rt::Logger::setWriterLevel(rt::Logger::getWriterLevel() + 1);

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
               decoderParams["nfca"]["enabled"] = protocols.find("nfca") != std::string::npos;
               decoderParams["nfcb"]["enabled"] = protocols.find("nfcb") != std::string::npos;
               decoderParams["nfcf"]["enabled"] = protocols.find("nfcf") != std::string::npos;
               decoderParams["nfcv"]["enabled"] = protocols.find("nfcv") != std::string::npos;
               break;
            }

               // limit running time
            case 't':
            {
               nsecs = strtol(optarg, &endptr, 10);

               if (endptr == optarg)
               {
                  printf("Invalid value for 't' argument\n");
                  showUsage();
                  return -1;
               }

               break;
            }

            default: /* '?' */
               printf("Unknown option '%c'\n", (char) opt);
               showUsage();
               return -1;
         }
      }

      // get start time
      auto start = std::chrono::steady_clock::now();

      // initialize
      init();

      // main loot until capture finished
      while (!terminate)
      {
         std::unique_lock<std::mutex> lock(mutex);

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
         if (nsecs > 0 && (std::chrono::steady_clock::now() - start) > std::chrono::seconds(nsecs))
         {
            fprintf(stdout, "Finish capture, time limit reached!\n");
            finish();
         }

         // flush console output
         fflush(stdout);
      }

      return 0;
   }

   static void showUsage()
   {
      printf("Usage: [-v] [-d] [-p nfca,nfcb,nfcf,nfcv] [-t nsecs]\n");
      printf("\tv: verbose mode, write logging information to stderr\n");
      printf("\td: debug mode, write WAV file with raw decoding signals (highly affected performance!)\n");
      printf("\tp: enable protocols, by default all are enabled\n");
      printf("\tt: stop capture after number of seconds\n");
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
   rt::Logger::setWriterLevel(rt::Logger::NONE_LEVEL);

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


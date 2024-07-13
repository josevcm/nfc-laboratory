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
#include <nfc/SignalRecorderTask.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Main
{
   rt::Logger log {"Main"};

   std::map<unsigned int, std::string> frameType {
         {nfc::FrameType::CarrierOff,  "CarrierOff"},
         {nfc::FrameType::CarrierOn,   "CarrierOn"},
         {nfc::FrameType::PollFrame,   "PCD->PICC"},
         {nfc::FrameType::ListenFrame, "PICC->PCD"}
   };

   std::map<unsigned int, std::string> frameTech {
         {nfc::TechType::None, "None"},
         {nfc::TechType::NfcA, "NfcA"},
         {nfc::TechType::NfcB, "NfcB"},
         {nfc::TechType::NfcF, "NfcD"},
         {nfc::TechType::NfcV, "NfcV"}
   };

   std::map<unsigned int, std::string> frameRate {
         {nfc::RateType::r106k, "106"},
         {nfc::RateType::r212k, "212"},
         {nfc::RateType::r424k, "424"},
         {nfc::RateType::r848k, "848"},
   };

   std::mutex mutex;
   std::condition_variable sync;
   std::atomic_bool terminate = false;

   // create executor service
   rt::Executor executor {128, 10};

   // streams subjects
   rt::Subject<rt::Event> *receiverStatusStream = nullptr;
   rt::Subject<rt::Event> *receiverCommandStream = nullptr;
   rt::Subject<rt::Event> *decoderStatusStream = nullptr;
   rt::Subject<rt::Event> *decoderCommandStream = nullptr;
   rt::Subject<nfc::NfcFrame> *decoderFrameStream = nullptr;

   // streams subscriptions
   rt::Subject<rt::Event>::Subscription receiverStatusSubscription;
   rt::Subject<rt::Event>::Subscription recorderStatusSubscription;
   rt::Subject<rt::Event>::Subscription decoderStatusSubscription;
   rt::Subject<nfc::NfcFrame>::Subscription decoderFrameSubscription;

   // receiver parameters
   int centerFreq = 40680000;
   int sampleRate = 10000000;
   int directSampling = 0;
   int gainMode = 1;
   int gainValue = 3;
   int mixerAgc = 0;
   int biasTee = 0;

   bool nfcaEnabled = true;
   bool nfcbEnabled = true;
   bool nfcfEnabled = true;
   bool nfcvEnabled = true;

   std::string decoderStatus = "idle";
   std::string receiverStatus = "idle";

   bool debugEnabled = false;

   Main()
   {
      log.info("NFC laboratory, 2024 Jose Vicente Campos Martinez");
   }

   void init()
   {
      // create processing tasks
      executor.submit(nfc::FrameDecoderTask::construct());
      executor.submit(nfc::SignalReceiverTask::construct());
      executor.submit(nfc::SignalRecorderTask::construct());

      // create receiver streams
      receiverStatusStream = rt::Subject<rt::Event>::name("receiver.status");
      receiverCommandStream = rt::Subject<rt::Event>::name("receiver.command");

      // create decoder streams
      decoderStatusStream = rt::Subject<rt::Event>::name("decoder.status");
      decoderCommandStream = rt::Subject<rt::Event>::name("decoder.command");
      decoderFrameStream = rt::Subject<nfc::NfcFrame>::name("decoder.frame");

      // handler for decoder status events
      receiverStatusSubscription = receiverStatusStream->subscribe([&](const rt::Event &event) {
         handleReceiverStatus(event);
      });

      // subscribe to decoder status
      decoderStatusSubscription = decoderStatusStream->subscribe([&](const rt::Event &event) {
         handleDecoderStatus(event);
      });

      // subscribe to decoder frames
      decoderFrameSubscription = decoderFrameStream->subscribe([&](const nfc::NfcFrame &frame) {
         handleDecoderFrame(frame);
      });
   }

   void handleReceiverStatus(const rt::Event &event)
   {
      if (auto data = event.get<std::string>("data"))
      {
         auto receiver = json::parse(data.value());

         log.info("receiver status: {}", {data.value()});

         // update receiver status
         receiverStatus = receiver["status"];

         // if no receiver detected, finish...
         if (receiverStatus == "absent")
         {
            log.info("no receiver found!");
            finish();
            return;
         }

         json config;

         if (receiver["centerFreq"] != centerFreq)
            config["centerFreq"] = centerFreq;

         if (receiver["sampleRate"] != sampleRate)
            config["sampleRate"] = sampleRate;

         if (receiver["directSampling"] != directSampling)
            config["directSampling"] = directSampling;

         if (receiver["biasTee"] != biasTee)
            config["biasTee"] = biasTee;

         if (receiver["mixerAgc"] != mixerAgc)
            config["mixerAgc"] = mixerAgc;

         if (receiver["gainMode"] != gainMode || receiver["gainValue"] != gainValue)
         {
            config["gainMode"] = gainMode;
            config["gainValue"] = gainValue;
         }

         // send configuration update
         if (!config.empty())
         {
            log.info("set receiver configuration: {}", {config.dump()});
            receiverCommandStream->next({nfc::SignalReceiverTask::Configure, nullptr, nullptr, {{"data", config.dump()}}});
         }

            // if receiver is configured, start decoding
         else if (receiverStatus == "idle")
         {
            receiverCommandStream->next({nfc::SignalReceiverTask::Start});
         }
      }
   }

   void handleDecoderStatus(const rt::Event &event)
   {
      if (auto data = event.get<std::string>("data"))
      {
         auto decoder = json::parse(data.value());

         log.info("decoder status: {}", {data.value()});

         // update receiver status
         decoderStatus = decoder["status"];

         json config;

         if (decoder["sampleRate"] != sampleRate)
            config["sampleRate"] = sampleRate;

         if (decoder["debugEnabled"] != debugEnabled)
            config["debugEnabled"] = debugEnabled;

         if (decoder["nfca"]["enabled"] != nfcaEnabled)
            config["nfca"]["enabled"] = nfcaEnabled;

         if (decoder["nfcb"]["enabled"] != nfcbEnabled)
            config["nfcb"]["enabled"] = nfcbEnabled;

         if (decoder["nfcf"]["enabled"] != nfcfEnabled)
            config["nfcf"]["enabled"] = nfcfEnabled;

         if (decoder["nfcv"]["enabled"] != nfcvEnabled)
            config["nfcv"]["enabled"] = nfcvEnabled;

         // send configuration update
         if (!config.empty())
         {
            log.info("set decoder configuration: {}", {config.dump()});
            decoderCommandStream->next({nfc::FrameDecoderTask::Configure, nullptr, nullptr, {{"data", config.dump()}}});
         }
            // if decoder is configured, start decoding
         else if (decoderStatus == "idle")
         {
            decoderCommandStream->next({nfc::FrameDecoderTask::Start});
         }
      }
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
      int nsecs = -1;
      int opt;

      while ((opt = getopt(argc, argv, "vdp:t:")) != -1)
      {
         switch (opt)
         {
            case 'v':
            {
               rt::Logger::setWriterLevel(rt::Logger::DEBUG_LEVEL);
               break;
            }

            case 'd':
            {
               debugEnabled = true;
               break;
            }

            case 'p':
            {
               std::string protocols = optarg;

               nfcaEnabled = protocols.find("nfca") != std::string::npos;
               nfcbEnabled = protocols.find("nfcb") != std::string::npos;
               nfcfEnabled = protocols.find("nfcf") != std::string::npos;
               nfcvEnabled = protocols.find("nfcv") != std::string::npos;

               break;
            }

            case 't':
            {
               char *endptr = nullptr;

               nsecs = strtol(optarg, &endptr, 10);

               if (endptr == optarg)
               {
                  log.error("invalid argument: {}", {optarg});
                  return 1;
               }

               break;
            }

            default: /* '?' */
               printf("Usage: [-v] [-d] [-p nfca,nfcb,nfcf,nfcv] [-t nsecs]\n");
               printf("\tv: verbose mode, write logging information to stderr\n");
               printf("\td: debug mode, write WAV file with raw decoding signals (highly affected performance!)\n");
               printf("\tp: enable protocols, by default all are enabled\n");
               printf("\tt: stop capture after number of seconds\n");
               return -1;
         }
      }

      // get start time
      auto start = std::chrono::steady_clock::now();

      // initialize
      init();

      // wait until capture finished
      while (!terminate)
      {
         std::unique_lock<std::mutex> lock(mutex);

         sync.wait_for(lock, std::chrono::milliseconds(500));

         if (nsecs > 0 && (std::chrono::steady_clock::now() - start) > std::chrono::seconds(nsecs))
         {
            finish();
         }
      }

      return 0;
   }

} app;

#ifdef __WIN32

WINBOOL intHandler(DWORD sig)
{
   fprintf(stderr, "Terminate on signal %lu\n", sig);
   app.finish();
   return true;
}

#else
void intHandler(int sig)
{
   fprintf(stderr, "Terminate on signal %d\n", sig);
   app.finish();
}
#endif

int main(int argc, char *argv[])
{
   // send logging events to stderr
   rt::Logger::init(std::cerr);

   // disable logging at all (can be enabled with -v option)
   rt::Logger::setWriterLevel(rt::Logger::NONE_LEVEL);

   // register signals
#ifdef __WIN32
   SetConsoleCtrlHandler(intHandler, TRUE);
#else
   signal(SIGINT, intHandler);
   signal(SIGTERM, intHandler);
#endif

   // and run
   return app.run(argc, argv);
}


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

#include <unistd.h>

#include <mutex>
#include <condition_variable>

#include <rt/Executor.h>
#include <rt/Subject.h>
#include <rt/Event.h>

#include <sdr/SignalType.h>
#include <sdr/AirspyDevice.h>

#include <nfc/NfcFrame.h>
#include <nfc/NfcDecoder.h>

#include <nfc/FrameDecoderTask.h>
#include <nfc/SignalReceiverTask.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Main
{
   std::mutex mutex;
   std::condition_variable sync;
   std::atomic_bool terminate = false;

   // streams subjects
   rt::Subject<rt::Event> *decoderStatusStream = nullptr;
   rt::Subject<rt::Event> *receiverStatusStream = nullptr;
   rt::Subject<rt::Event> *decoderCommandStream = nullptr;
   rt::Subject<rt::Event> *receiverCommandStream = nullptr;
   rt::Subject<nfc::NfcFrame> *decoderFrameStream = nullptr;

   // streams subscriptions
   rt::Subject<rt::Event>::Subscription decoderStatusSubscription;
   rt::Subject<rt::Event>::Subscription receiverStatusSubscription;
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

   Main()
   {
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

         fprintf(stderr, "receiver status: %s\n", data.value().c_str());

         // update receiver status
         receiverStatus = receiver["status"];

         // if no receiver detected, finish...
         if (receiverStatus == "absent")
         {
            fprintf(stderr, "no receiver found!\n");
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
            fprintf(stderr, "Set receiver configuration: %s\n", config.dump().c_str());
            receiverCommandStream->next({nfc::SignalReceiverTask::Configure, nullptr, nullptr, {{"data", config.dump()}}});
         }

            // if receiver is configured, start decoding
         else if (decoderStatus == "idle")
         {
            decoderCommandStream->next({nfc::FrameDecoderTask::Start, [=]() {
               receiverCommandStream->next({nfc::SignalReceiverTask::Start, [=]() {
               }});
            }});
         }
      }
   }

   void handleDecoderStatus(const rt::Event &event)
   {
      if (auto data = event.get<std::string>("data"))
      {
         auto decoder = json::parse(data.value());

         fprintf(stderr, "decoder status: %s\n", data.value().c_str());

         // update receiver status
         decoderStatus = decoder["status"];

         json config;

         if (decoder["sampleRate"] != sampleRate)
            config["sampleRate"] = sampleRate;

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
            fprintf(stderr, "Set decoder configuration: %s\n", config.dump().c_str());
            decoderCommandStream->next({nfc::FrameDecoderTask::Configure, nullptr, nullptr, {{"data", config.dump()}}});
         }
      }
   }

   void handleDecoderFrame(const nfc::NfcFrame &frame)
   {
      fprintf(stderr, "frame type %s\n", frame.frameType());

      if (frame.isPollFrame())
      {
         fprintf(stderr, "%4.3f >> \n", frame.timeStart());
      }
      else if (frame.isListenFrame())
      {
         fprintf(stderr, "%4.3f << \n", frame.timeStart());
      }
   }

   void finish()
   {
      terminate = true;
      sync.notify_all();
   }

   // no reentrant!
   const char *hex(const nfc::NfcFrame &frame)
   {
      int offset = 0;
      static char buffer[4096];

      for (int i = 0; i < frame.size(); i++)
      {
         offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%02X ", (unsigned int) frame[i]);
      }

      return buffer;
   }

   int run(int argc, char *argv[])
   {
      // wait until capture finished
      while (!terminate)
      {
         std::unique_lock<std::mutex> lock(mutex);

         sync.wait_for(lock, std::chrono::milliseconds(500));
      }

      return 0;
   }
};


int main(int argc, char *argv[])
{
   fprintf(stderr, "NFC laboratory, 2024 Jose Vicente Campos Martinez\n");

   // create executor service
   rt::Executor executor(128, 10);

   // create processing tasks
   executor.submit(nfc::FrameDecoderTask::construct());
   executor.submit(nfc::SignalReceiverTask::construct());

   // create app instance
   Main app;

   // and run
   return app.run(argc, argv);
}


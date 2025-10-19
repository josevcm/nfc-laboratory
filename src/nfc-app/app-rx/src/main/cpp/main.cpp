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

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cmath>
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
   rt::Logger *log = rt::Logger::getLogger("app.main");

   // frame type catalog
   const std::map<unsigned int, std::string> frameType {
      {lab::FrameType::NfcCarrierOff, "CarrierOff"},
      {lab::FrameType::NfcCarrierOn, "CarrierOn"},
      {lab::FrameType::NfcPollFrame, "Poll"},
      {lab::FrameType::NfcListenFrame, "Listen"}
   };

   // frame tech catalog
   const std::map<unsigned int, std::string> frameTech {
      {lab::FrameTech::NoneTech, "None"},
      {lab::FrameTech::NfcATech, "NfcA"},
      {lab::FrameTech::NfcBTech, "NfcB"},
      {lab::FrameTech::NfcFTech, "NfcF"},
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

   // output mode
   bool jsonOutputEnabled = false;

   Main()
   {
   }

   void init(bool jsonOutput = false)
   {
      log->info("NFC laboratory, 2024 Jose Vicente Campos Martinez");

      jsonOutputEnabled = jsonOutput;

      if (jsonOutputEnabled)
      {
         log->info("JSON frame output enabled");
      }

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

      // forward streamTime from receiver to decoder (for frame dateTime)
      if (receiverStatus.contains("streamTime"))
         decoderParams["streamTime"] = receiverStatus["streamTime"];

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
         receiverCommandStream->next({lab::RadioDeviceTask::Configure, [this] { receiverConfigured = true; }, nullptr, {{"data", config.dump()}}});
      }

      // if receiver is configured and idle, start it
      if (receiverConfigured && receiverStatus["status"] == "idle")
      {
         log->info("start receiver streaming");
         receiverCommandStream->next({lab::RadioDeviceTask::Start, [this] { receiverStatus["status"] = "waiting"; }});
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

      // wait until streamTime is available before starting decoder (important for dateTime in frames)
      // Note: streamTime must be > 0, not just non-null (0 means not yet set by device)
      if (decoderParams["streamTime"].is_null() || decoderParams["streamTime"] == 0)
         return 0;

      // detect required changes
      const json config = detectChanges(decoderStatus, decoderParams);

      // if no configuration needed, just start decoder
      decoderConfigured = config.empty();

      // send configuration update
      if (!decoderConfigured)
      {
         printf("decoder configuration: %s\n", config.dump().c_str());
         decoderCommandStream->next({lab::RadioDecoderTask::Configure, [this] { decoderConfigured = true; }, nullptr, {{"data", config.dump()}}});
      }

      // if decoder is configured and idle, start it
      if (decoderConfigured && decoderStatus["status"] == "idle")
      {
         log->info("start decoder streaming");
         decoderCommandStream->next({lab::RadioDecoderTask::Start, [this] { decoderStatus["status"] = "waiting"; }});
      }

      return 0;
   }

   static json detectChanges(const json &ref, const json &set)
   {
      json result;

      for (auto &entry: set.items())
      {
         // New field that doesn't exist in ref - include it
         if (!ref.contains(entry.key()))
         {
            result[entry.key()] = entry.value();
            continue;
         }

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

   void printFrameJSON(const lab::RawFrame &frame) const
   {
      if (!frame.isValid())
         return;

      json frameObject;

      // basic frame info
      frameObject["timestamp"] = static_cast<int64_t>(frame.sampleStart());
      frameObject["tech"] = frameTech.count(frame.techType()) ? frameTech.at(frame.techType()) : "UNKNOWN";
      frameObject["type"] = frameType.count(frame.frameType()) ? frameType.at(frame.frameType()) : "UNKNOWN";

      // numeric enum values (matching nfc-lab TRZ format)
      frameObject["tech_type"] = static_cast<int64_t>(frame.techType());
      frameObject["frame_type"] = static_cast<int64_t>(frame.frameType());

      // time info (output as int if exactly 0, matching nfc-lab format)
      if (frame.timeStart() == 0.0)
         frameObject["time_start"] = 0;
      else
         frameObject["time_start"] = frame.timeStart();

      if (frame.timeEnd() == 0.0)
         frameObject["time_end"] = 0;
      else
         frameObject["time_end"] = frame.timeEnd();

      // sample info
      frameObject["sample_start"] = static_cast<int64_t>(frame.sampleStart());
      frameObject["sample_end"] = static_cast<int64_t>(frame.sampleEnd());
      frameObject["sample_rate"] = static_cast<int64_t>(frame.sampleRate());

      // datetime (output as int if whole number, matching nfc-lab format)
      double dateTime = frame.dateTime();
      if (dateTime == std::floor(dateTime))
         frameObject["date_time"] = static_cast<int64_t>(dateTime);
      else
         frameObject["date_time"] = dateTime;

      // rate if available
      if (frame.frameRate() > 0)
         frameObject["rate"] = static_cast<int64_t>(frame.frameRate());

      // data if available
      if (!frame.isEmpty())
      {
         std::string hexData;
         for (int i = 0; i < frame.limit(); i++)
         {
            if (i > 0) hexData += ":";
            char hex[3];
            snprintf(hex, sizeof(hex), "%02x", static_cast<unsigned int>(frame[i]));
            hexData += hex;
         }
         frameObject["data"] = hexData;
         frameObject["length"] = static_cast<int64_t>(frame.limit());
      }

      // flags array
      json flagsList = json::array();

      if (frame.hasFrameFlags(lab::CrcError))
         flagsList.push_back("crc-error");

      if (frame.hasFrameFlags(lab::ParityError))
         flagsList.push_back("parity-error");

      if (frame.hasFrameFlags(lab::SyncError))
         flagsList.push_back("sync-error");

      if (frame.hasFrameFlags(lab::Truncated))
         flagsList.push_back("truncated");

      if (frame.hasFrameFlags(lab::Encrypted))
         flagsList.push_back("encrypted");

      // frame direction flags
      if (frame.frameType() == lab::NfcPollFrame || frame.frameType() == lab::IsoRequestFrame)
         flagsList.push_back("request");
      else if (frame.frameType() == lab::NfcListenFrame || frame.frameType() == lab::IsoResponseFrame)
         flagsList.push_back("response");

      if (!flagsList.empty())
         frameObject["flags"] = flagsList;

      // print compact JSON
      fprintf(stdout, "%s\n", frameObject.dump().c_str());
   }

   void printFrame(const lab::RawFrame &frame) const
   {
      int offset = 0;
      char buffer[16384];

      // add datagram time
      offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%010.3f ", frame.timeStart());

      // add frame type
      const char* ft = frameType.count(frame.frameType()) ? frameType.at(frame.frameType()).c_str() : "UNKNOWN";
      offset += snprintf(buffer + offset, sizeof(buffer) - offset, "(%s) ", ft);

      // data frames
      if (frame.frameType() == lab::FrameType::NfcPollFrame || frame.frameType() == lab::FrameType::NfcListenFrame)
      {
         // add tech type
         const char* tech = frameTech.count(frame.techType()) ? frameTech.at(frame.techType()).c_str() : "UNKNOWN";
         offset += snprintf(buffer + offset, sizeof(buffer) - offset, "[%s@%.0f]: ", tech, roundf(float(frame.frameRate()) / 1000.0f));

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
      bool jsonOutput = false;

      // define long options
      static struct option long_options[] = {
         {"help",       no_argument,       nullptr, 'h'},
         {"version",    no_argument,       nullptr, 'v'},
         {"log-level",  required_argument, nullptr, 'l'},
         {"json-frames", no_argument,      nullptr, 'j'},
         {nullptr, 0, nullptr, 0}
      };

      int option_index = 0;
      while ((opt = getopt_long(argc, argv, "hvjl:dp:t:f:s:", long_options, &option_index)) != -1)
      {
         switch (opt)
         {
            case 'h':
            {
               printUsage(argc > 0 ? argv[0] : "nfc-rx");
               return 0;
            }

            case 'v':
            {
               printf("nfc-rx %s\n", PROJECT_VERSION);
               return 0;
            }

            case 'l':
            {
               std::string level = optarg;
               if (level == "DEBUG")
                  rt::Logger::setRootLevel(rt::Logger::DEBUG_LEVEL);
               else if (level == "INFO")
                  rt::Logger::setRootLevel(rt::Logger::INFO_LEVEL);
               else if (level == "WARN")
                  rt::Logger::setRootLevel(rt::Logger::WARN_LEVEL);
               else if (level == "ERROR")
                  rt::Logger::setRootLevel(rt::Logger::ERROR_LEVEL);
               else if (level == "TRACE")
                  rt::Logger::setRootLevel(rt::Logger::TRACE_LEVEL);
               else
               {
                  fprintf(stderr, "Invalid log level: %s (use DEBUG, INFO, WARN, ERROR, or TRACE)\n", optarg);
                  showUsage();
                  return -1;
               }
               break;
            }

            case 'j':
            {
               jsonOutput = true;
               break;
            }

            case 'd':
            {
               decoderParams["debugEnabled"] = true;
               break;
            }

            case 'p':
            {
               std::string protocols = optarg;
               decoderParams["protocol"]["nfca"]["enabled"] = protocols.find("nfca") != std::string::npos;
               decoderParams["protocol"]["nfcb"]["enabled"] = protocols.find("nfcb") != std::string::npos;
               decoderParams["protocol"]["nfcf"]["enabled"] = protocols.find("nfcf") != std::string::npos;
               decoderParams["protocol"]["nfcv"]["enabled"] = protocols.find("nfcv") != std::string::npos;
               break;
            }

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
      init(jsonOutput);

      // main loop until capture finished
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
            if (jsonOutputEnabled)
               printFrameJSON(frame.value());
            else
               printFrame(frame.value());
         }

         // flush console output
         fflush(stdout);
      }

      return 0;
   }

   static void printUsage(const char *programName)
   {
      std::cout << "NFC Laboratory - Live SDR Receiver" << std::endl;
      std::cout << std::endl;
      std::cout << "Usage: " << programName << " [OPTIONS]" << std::endl;
      std::cout << std::endl;
      std::cout << "Description:" << std::endl;
      std::cout << "  Live NFC signal decoder for Software Defined Radio (SDR) devices." << std::endl;
      std::cout << "  Captures and decodes NFC signals in real-time." << std::endl;
      std::cout << std::endl;
      std::cout << "Options:" << std::endl;
      std::cout << "  -h, --help            Show this help message and exit" << std::endl;
      std::cout << "  -v, --version         Show version information and exit" << std::endl;
      std::cout << "  -l, --log-level LEVEL Set log level: DEBUG, INFO, WARN, ERROR, TRACE" << std::endl;
      std::cout << "                        Default: no logging (silent)" << std::endl;
      std::cout << "  -j, --json-frames     Output decoded NFC frames as JSON to stdout" << std::endl;
      std::cout << "                        Default: human-readable text format" << std::endl;
      std::cout << "  -d                    Enable debug mode - write WAV file with raw signals" << std::endl;
      std::cout << "                        (WARNING: significantly affects performance!)" << std::endl;
      std::cout << "  -p PROTOCOLS          Enable specific protocols (comma-separated)" << std::endl;
      std::cout << "                        Options: nfca, nfcb, nfcf, nfcv" << std::endl;
      std::cout << "                        Default: all protocols enabled" << std::endl;
      std::cout << "  -f FREQUENCY          Set receiver center frequency in Hz" << std::endl;
      std::cout << "                        Default: auto-configured (depends on hardware)" << std::endl;
      std::cout << "  -s SAMPLERATE         Set receiver sample rate in Hz" << std::endl;
      std::cout << "                        Default: auto-configured by device" << std::endl;
      std::cout << "  -t SECONDS            Stop capture after specified number of seconds" << std::endl;
      std::cout << "                        Default: run until interrupted (Ctrl+C)" << std::endl;
      std::cout << std::endl;
      std::cout << "Output Formats:" << std::endl;
      std::cout << "  Text (default):  0001234.567 (Poll) [NfcA@106]: 26 00" << std::endl;
      std::cout << "  JSON (-j flag):  {\"timestamp\":1234.567,\"type\":\"Poll\",\"tech\":\"NfcA\",\"rate\":106,\"data\":\"26:00\"}" << std::endl;
      std::cout << std::endl;
      std::cout << "Examples:" << std::endl;
      std::cout << "  " << programName << std::endl;
      std::cout << "    Start capturing with default settings (text output, all protocols)" << std::endl;
      std::cout << std::endl;
      std::cout << "  " << programName << " --json-frames > capture.json" << std::endl;
      std::cout << "    Capture with JSON output and save to file" << std::endl;
      std::cout << std::endl;
      std::cout << "  " << programName << " -l INFO -j" << std::endl;
      std::cout << "    JSON output with INFO-level logging (logs go to stderr)" << std::endl;
      std::cout << std::endl;
      std::cout << "  " << programName << " -p nfca,nfcb -t 60" << std::endl;
      std::cout << "    Capture only NFC-A and NFC-B for 60 seconds" << std::endl;
      std::cout << std::endl;
      std::cout << "  " << programName << " -f 40680000 -s 10000000 -j" << std::endl;
      std::cout << "    Capture with specific frequency/sample-rate (Airspy settings)" << std::endl;
      std::cout << std::endl;
      std::cout << "Supported Hardware:" << std::endl;
      std::cout << "  - RTL-SDR dongles" << std::endl;
      std::cout << "  - Airspy (Mini, R2, HF+)" << std::endl;
      std::cout << "  - HackRF One" << std::endl;
      std::cout << "  - Other SDR devices compatible with the driver library" << std::endl;
      std::cout << std::endl;
      std::cout << "Compatibility:" << std::endl;
      std::cout << "  This tool is compatible with 'nfc-lab --json-frames' output format." << std::endl;
      std::cout << "  Use -j/--json-frames and -l/--log-level for identical behavior." << std::endl;
      std::cout << std::endl;
      std::cout << "Note:" << std::endl;
      std::cout << "  Press Ctrl+C to stop capturing and exit gracefully." << std::endl;
      std::cout << "  Logging output goes to stderr, frame data goes to stdout." << std::endl;
      std::cout << std::endl;
   }

   static void showUsage()
   {
      printUsage("nfc-rx");
   }

} *app;

#ifdef _WIN32

BOOL WINAPI intHandler(DWORD sig)
{
   fprintf(stderr, "Terminate on signal %lu\n", sig);
   app->finish();
   return TRUE;
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
#ifdef _WIN32
   SetConsoleCtrlHandler((PHANDLER_ROUTINE)intHandler, TRUE);
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

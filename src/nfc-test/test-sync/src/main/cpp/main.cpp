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

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <rt/Logger.h>
#include <rt/FileSystem.h>

#include <hw/SignalType.h>
#include <hw/RecordDevice.h>

#include <lab/data/RawFrame.h>

#include <lab/iso/IsoDecoder.h>
#include <lab/nfc/NfcDecoder.h>

using namespace rt;

Logger *logger = Logger::getLogger("main");

// number of samples the stream is displaced on the biased pass, must not be a multiple of the read block
constexpr unsigned long long SAMPLE_BIAS = 1234567;

// samples read on each decoder call
constexpr unsigned int BLOCK_SIZE = 65536;

// decoded frames inspected per file, logic captures without smart card activity emit one frame per signal edge
constexpr unsigned int FRAME_LIMIT = 4096;

// maximum time deviation accepted between both passes
constexpr double TIME_EPSILON = 1E-9;

// outcome of decoding one recording
enum Result
{
   ResultError = -1,
   ResultOk = 1
};

/*
 * Timing of one decoded frame
 */
struct Timing
{
   unsigned long sampleStart;
   double timeStart;
};

/*
 * Decode radio signal, displacing the stream by the given number of samples
 */
int readRadioSignal(hw::RecordDevice &source, unsigned long long bias, std::vector<Timing> &list)
{
   lab::NfcDecoder decoder;

   decoder.setEnableNfcA(true);
   decoder.setEnableNfcB(true);
   decoder.setEnableNfcF(true);
   decoder.setEnableNfcV(true);

   const unsigned int channelCount = source.get<unsigned int>(hw::SignalDevice::PARAM_CHANNEL_COUNT);
   const unsigned int sampleRate = source.get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_RATE);

   unsigned long long offset = bias;

   while (!source.isEof() && list.size() < FRAME_LIMIT)
   {
      hw::SignalBuffer samples(BLOCK_SIZE * channelCount, channelCount, 1, sampleRate, offset, 0, hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES, 0);

      if (source.read(samples) > 0)
      {
         for (const lab::RawFrame &frame: decoder.nextFrames(samples))
         {
            if (list.size() == FRAME_LIMIT)
               break;

            if (frame.frameType() == lab::FrameType::NfcPollFrame || frame.frameType() == lab::FrameType::NfcListenFrame)
               list.push_back({frame.sampleStart(), frame.timeStart()});
         }

         // elements() already counts samples per channel, not buffer values
         offset += samples.elements();
      }
   }

   return ResultOk;
}

/*
 * Decode logic signal, displacing the stream by the given number of samples
 */
int readLogicSignal(hw::RecordDevice &source, unsigned long long bias, std::vector<Timing> &list)
{
   lab::IsoDecoder decoder;

   decoder.setEnableISO7816(true);

   const unsigned int channelCount = source.get<unsigned int>(hw::SignalDevice::PARAM_CHANNEL_COUNT);
   const unsigned int sampleRate = source.get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_RATE);

   unsigned long long offset = bias;

   while (!source.isEof() && list.size() < FRAME_LIMIT)
   {
      hw::SignalBuffer samples(BLOCK_SIZE * channelCount, channelCount, 1, sampleRate, offset, 0, hw::SignalType::SIGNAL_TYPE_LOGIC_SAMPLES, 0);

      if (source.read(samples) > 0)
      {
         for (const lab::RawFrame &frame: decoder.nextFrames(samples))
         {
            if (list.size() == FRAME_LIMIT)
               break;

            list.push_back({frame.sampleStart(), frame.timeStart()});
         }

         // elements() already counts samples per channel, not buffer values
         offset += samples.elements();
      }
   }

   return ResultOk;
}

/*
 * Decode one recording, displacing the stream by the given number of samples
 */
int readSignal(const std::string &path, unsigned long long bias, std::vector<Timing> &list, unsigned int &sampleRate)
{
   if (!FileSystem::exists(path))
      return ResultError;

   hw::RecordDevice source(path);

   if (!source.open(hw::RecordDevice::Mode::Read))
      return ResultError;

   sampleRate = source.get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_RATE);

   // storage format tells both devices apart, same rule used when loading a capture into the application
   switch (source.get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_SIZE))
   {
      case hw::SAMPLE_SIZE_8:
         return readLogicSignal(source, bias, list);

      case hw::SAMPLE_SIZE_16:
         return readRadioSignal(source, bias, list);

      default:
         logger->error("invalid storage format for file {}", {path});
         return ResultError;
   }
}

/*
 * Check decoded frames follow the absolute sample offset published on the signal buffers
 */
int testFile(const std::string &signal)
{
   size_t pos1 = signal.find(".wav");
   size_t pos2 = signal.find_last_of("/\\");

   if (pos1 == std::string::npos)
      return -1;

   std::string filename = signal;

   if (pos2 != std::string::npos)
      filename = signal.substr(pos2 + 1, pos1 - pos2 - 1);

   std::vector<Timing> list1;
   std::vector<Timing> list2;

   unsigned int sampleRate1 = 0;
   unsigned int sampleRate2 = 0;

   // decode the stream starting at sample zero, as when replaying a capture from file
   if (readSignal(signal, 0, list1, sampleRate1) != ResultOk)
      return -1;

   // decode the same stream displaced to a shared capture epoch, as device tasks do on live captures
   if (readSignal(signal, SAMPLE_BIAS, list2, sampleRate2) != ResultOk)
      return -1;

   if (list1.empty())
   {
      std::cout << "TEST FILE " << filename << ": SKIPPED, no frames decoded" << std::endl;
      return 0;
   }

   if (list1.size() != list2.size())
   {
      std::cout << "TEST FILE " << filename << ": FAIL, frame count differs, " << list1.size() << " vs " << list2.size() << std::endl;
      return 1;
   }

   // frames must be displaced exactly as the signal is, otherwise view markers drift away from the signal
   const double timeBias = static_cast<double>(SAMPLE_BIAS) / static_cast<double>(sampleRate1);

   unsigned int failed = 0;

   for (size_t i = 0; i < list1.size(); i++)
   {
      const long long sampleShift = static_cast<long long>(list2[i].sampleStart) - static_cast<long long>(list1[i].sampleStart);
      const double timeShift = list2[i].timeStart - list1[i].timeStart;

      if (sampleShift != static_cast<long long>(SAMPLE_BIAS) || std::fabs(timeShift - timeBias) > TIME_EPSILON)
      {
         if (failed < 3)
            logger->warn("frame {} displaced {} samples ({} s), expected {} samples ({} s)", {i, sampleShift, timeShift, SAMPLE_BIAS, timeBias});

         failed++;
      }
   }

   std::cout << "TEST FILE " << filename << ": " << (failed ? "FAIL" : "PASS") << ", " << list1.size() << " frames, " << failed << " misaligned" << std::endl;

   return failed ? 1 : 0;
}

int testPath(const std::string &path)
{
   int failed = 0;

   for (const auto &entry: FileSystem::directoryList(path))
   {
      if (entry.name.find(".wav") != std::string::npos)
      {
         if (testFile(entry.name) > 0)
            failed++;
      }
   }

   return failed;
}

void printUsage(const char *programName)
{
   std::cout << "NFC Decoder Time Base - Test Tool" << std::endl;
   std::cout << std::endl;
   std::cout << "Usage: " << programName << " [OPTIONS] <wav-file|directory>" << std::endl;
   std::cout << std::endl;
   std::cout << "Description:" << std::endl;
   std::cout << "  Check that decoded frames are timed on the absolute sample offset carried by" << std::endl;
   std::cout << "  the signal buffers. Signal views draw sample i of a buffer at" << std::endl;
   std::cout << "  (buffer.offset() + i) / sampleRate, and device tasks bias that offset to align" << std::endl;
   std::cout << "  several devices on a shared capture epoch, so frames must follow the same" << std::endl;
   std::cout << "  displacement or markers and protocol labels drift away from the signal." << std::endl;
   std::cout << std::endl;
   std::cout << "Options:" << std::endl;
   std::cout << "  --help, -h    Show this help message and exit" << std::endl;
   std::cout << std::endl;
   std::cout << "Arguments:" << std::endl;
   std::cout << "  <wav-file>    Path to a WAV file containing radio or logic recordings" << std::endl;
   std::cout << "  <directory>   Path to a directory containing multiple WAV files" << std::endl;
   std::cout << std::endl;
   std::cout << "Test Behavior:" << std::endl;
   std::cout << "  - Decodes each WAV file twice, starting at sample 0 and displaced " << SAMPLE_BIAS << " samples" << std::endl;
   std::cout << "  - 8 bit recordings are decoded as logic signal, 16 bit ones as radio signal" << std::endl;
   std::cout << "  - If every frame is displaced exactly the same amount: PASS" << std::endl;
   std::cout << "  - If any frame keeps its original timing: FAIL" << std::endl;
   std::cout << std::endl;
   std::cout << "Examples:" << std::endl;
   std::cout << "  " << programName << " wav/test_NFC-A_106kbps_001.wav" << std::endl;
   std::cout << "    Test a single WAV file" << std::endl;
   std::cout << std::endl;
   std::cout << "  " << programName << " wav/" << std::endl;
   std::cout << "    Test all WAV files in the wav/ directory" << std::endl;
   std::cout << std::endl;
}

int main(int argc, char *argv[])
{
   logger->info("***********************************************************************");
   logger->info("NFC laboratory, 2024 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   logger->info("***********************************************************************");

   if (argc < 2)
   {
      printUsage(argv[0]);
      return 1;
   }

   // Check for help flag
   for (int i = 1; i < argc; i++)
   {
      std::string arg(argv[i]);
      if (arg == "--help" || arg == "-h")
      {
         printUsage(argv[0]);
         return 0;
      }
   }

   int failed = 0;

   for (int i = 1; i < argc; i++)
   {
      std::string path {argv[i]};

      if (FileSystem::isDirectory(path))
      {
         logger->info("processing path {}", {path});

         failed += testPath(path);
      }
      else if (FileSystem::isRegularFile(path))
      {
         logger->info("processing file {}", {path});

         if (testFile(path) > 0)
            failed++;
      }
      else
      {
         logger->error("invalid path: {}", {path});
         return 1;
      }
   }

   return failed ? 1 : 0;
}

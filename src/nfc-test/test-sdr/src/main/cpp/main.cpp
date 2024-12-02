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

#include <iostream>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>

#include <rt/Logger.h>
#include <rt/FileSystem.h>

#include <hw/SignalType.h>
#include <hw/RecordDevice.h>

#include <lab/data/RawFrame.h>

#include <lab/nfc/Nfc.h>
#include <lab/nfc/NfcDecoder.h>

using namespace rt;
using namespace nlohmann;

Logger *logger = Logger::getLogger("main");

/*
 * Read frames from JSON storage
 */
bool readFrames(const std::string &path, std::list<lab::RawFrame> &list)
{
   if (!FileSystem::exists(path))
      return false;

   json data;

   // open file
   std::ifstream input(path);

   // read json
   input >> data;

   if (!data.contains("frames"))
      return false;

   // read frames from file
   for (const auto &entry: data["frames"])
   {
      lab::RawFrame frame(256);

      frame.setTechType(entry["techType"]);
      frame.setDateTime(entry["dateTime"]);
      frame.setFrameType(entry["frameType"]);
      frame.setFramePhase(entry["framePhase"]);
      frame.setFrameFlags(entry["frameFlags"]);
      frame.setFrameRate(entry["frameRate"]);
      frame.setTimeStart(entry["timeStart"]);
      frame.setTimeEnd(entry["timeEnd"]);
      frame.setSampleStart(entry["sampleStart"]);
      frame.setSampleEnd(entry["sampleEnd"]);
      frame.setSampleRate(entry["sampleRate"]);

      std::string bytes = entry["frameData"];

      for (size_t index = 0, size = 0; index < bytes.length(); index += size + 1)
      {
         frame.put(std::stoi(bytes.c_str() + index, &size, 16));
      }

      frame.flip();

      list.push_back(frame);
   }

   return true;
}

/*
 * Write frames to JSON storage
 */
bool writeFrames(const std::string &path, std::list<lab::RawFrame> &list)
{
   json frames = json::array();

   for (const auto &frame: list)
   {
      if (frame.frameType() == lab::FrameType::NfcPollFrame || frame.frameType() == lab::FrameType::NfcListenFrame)
      {
         char buffer[4096];

         frame.reduce<int>(0, [&buffer](int offset, unsigned char value) {
            return offset + snprintf(buffer + offset, sizeof(buffer) - offset, offset > 0 ? ":%02X" : "%02X", value);
         });

         frames.push_back({
            {"techType", frame.techType()},
            {"dateTime", frame.dateTime()},
            {"sampleStart", frame.sampleStart()},
            {"sampleEnd", frame.sampleEnd()},
            {"sampleRate", frame.sampleRate()},
            {"timeStart", frame.timeStart()},
            {"timeEnd", frame.timeEnd()},
            {"frameType", frame.frameType()},
            {"frameRate", frame.frameRate()},
            {"frameFlags", frame.frameFlags()},
            {"framePhase", frame.framePhase()},
            {"frameData", buffer}
         });
      }
   }

   json info({{"frames", frames}});

   std::ofstream output(path);

   output << std::setw(3) << info.dump(2) << std::endl;

   return true;
}

/*
 * Read frames from WAV file
 */
bool readSignal(const std::string &path, std::list<lab::RawFrame> &list)
{
   if (!FileSystem::exists(path))
      return false;

   hw::RecordDevice source(path);

   if (!source.open(hw::RecordDevice::Mode::Read))
      return false;

   lab::NfcDecoder decoder;

   decoder.setEnableNfcA(true);
   decoder.setEnableNfcB(true);
   decoder.setEnableNfcF(true);
   decoder.setEnableNfcV(true);

   std::list<lab::RawFrame> frames;

   unsigned int channelCount = std::get<unsigned int>(source.get(hw::SignalDevice::PARAM_CHANNEL_COUNT));
   unsigned int sampleRate = std::get<unsigned int>(source.get(hw::SignalDevice::PARAM_SAMPLE_RATE));

   while (!source.isEof())
   {
      hw::SignalBuffer samples(65536 * channelCount, channelCount, 1, sampleRate, 0, 0, hw::SignalType::SIGNAL_TYPE_RAW_REAL, 0);

      if (source.read(samples) > 0)
      {
         for (const lab::RawFrame &frame: decoder.nextFrames(samples))
         {
            if (frame.frameType() == lab::FrameType::NfcPollFrame || frame.frameType() == lab::FrameType::NfcListenFrame)
            {
               list.push_back(frame);
            }
         }
      }
   }

   return true;
}

int testFile(const std::string &signal)
{
   size_t pos1 = signal.find(".wav");
   size_t pos2 = signal.rfind("/");

   if (pos1 == std::string::npos)
      return -1;

   std::string target = signal.substr(0, pos1) + ".json";
   std::string filename = signal;

   if (pos2 != std::string::npos)
      filename = signal.substr(pos2 + 1, pos1 - 4);

   std::list<lab::RawFrame> list1;
   std::list<lab::RawFrame> list2;

   // decode frames from signal
   if (readSignal(signal, list1))
   {
      // read frames from json file
      if (readFrames(target, list2))
      {
         // show result
         std::cout << "TEST FILE " << filename << ": " << (list1 == list2 ? "PASS" : "FAIL") << std::endl;
      }
      else
      {
         // or create json file if not exists at first time
         writeFrames(target, list1);

         std::cout << "TEST FILE " << filename << ": TEST UPDATED!" << std::endl;
      }
   }

   return 0;
}

int testPath(const std::string &path)
{
   for (const auto &entry: FileSystem::directoryList(path))
   {
      if (entry.name.find(".wav") != std::string::npos)
      {
         testFile(entry.name);
      }
   }

   return 0;
}

int main(int argc, char *argv[])
{
   //   Logger::init(std::cout, false);

   logger->info("***********************************************************************");
   logger->info("NFC laboratory, 2024 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   logger->info("***********************************************************************");

   for (int i = 1; i < argc; i++)
   {
      std::string path {argv[i]};

      if (FileSystem::isDirectory(path))
      {
         logger->info("processing path {}", {path});

         testPath(path);
      }
      else if (FileSystem::isRegularFile(path))
      {
         logger->info("processing file {}", {path});

         testFile(path);
      }
   }

   return 0;
}

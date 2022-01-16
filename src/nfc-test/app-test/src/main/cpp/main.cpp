/*

  Copyright (c) 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>

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

#include <iostream>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>

#include <rt/Logger.h>
#include <rt/FileSystem.h>

#include <sdr/SignalType.h>
#include <sdr/RecordDevice.h>

#include <nfc/NfcFrame.h>
#include <nfc/NfcDecoder.h>

using namespace rt;
using namespace nlohmann;

Logger logger {"main"};

/*
 * Read frames from JSON storage
 */
bool readFrames(const std::string &path, std::list<nfc::NfcFrame> &list)
{
   if (!rt::FileSystem::exists(path))
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
      nfc::NfcFrame frame;

      frame.setTechType(entry["techType"]);
      frame.setFrameType(entry["frameType"]);
      frame.setFramePhase(entry["framePhase"]);
      frame.setFrameFlags(entry["frameFlags"]);
      frame.setFrameRate(entry["frameRate"]);
      frame.setTimeStart(entry["timeStart"]);
      frame.setTimeEnd(entry["timeEnd"]);
      frame.setSampleStart(entry["sampleStart"]);
      frame.setSampleEnd(entry["sampleEnd"]);

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
bool writeFrames(const std::string &path, std::list<nfc::NfcFrame> &list)
{
   json frames = json::array();

   for (const auto &frame: list)
   {
      if (frame.isPollFrame() || frame.isListenFrame())
      {
         char buffer[4096];

         frame.reduce<int>(0, [&buffer](int offset, unsigned char value) {
            return offset + snprintf(buffer + offset, sizeof(buffer) - offset, offset > 0 ? ":%02X" : "%02X", value);
         });

         frames.push_back({
                                {"sampleStart", frame.sampleStart()},
                                {"sampleEnd",   frame.sampleEnd()},
                                {"timeStart",   frame.timeStart()},
                                {"timeEnd",     frame.timeEnd()},
                                {"techType",    frame.techType()},
                                {"frameType",   frame.frameType()},
                                {"frameRate",   frame.frameRate()},
                                {"frameFlags",  frame.frameFlags()},
                                {"framePhase",  frame.framePhase()},
                                {"frameData",   buffer}
                          });
      }
   }

   json info({{"frames", frames}});

   std::ofstream output(path);

   output << std::setw(3) << info << std::endl;

   return true;
}

/*
 * Read frames from WAV file
 */
bool readSignal(const std::string &path, std::list<nfc::NfcFrame> &list)
{
   if (!rt::FileSystem::exists(path))
      return false;

   sdr::RecordDevice source(path);

   if (!source.open(sdr::RecordDevice::OpenMode::Read))
      return false;

   nfc::NfcDecoder decoder;

   decoder.setEnableNfcA(true);
   decoder.setEnableNfcB(true);
   decoder.setEnableNfcF(true);
   decoder.setEnableNfcV(true);

   std::list<nfc::NfcFrame> frames;

   while (!source.isEof())
   {
      sdr::SignalBuffer samples(65536 * source.channelCount(), source.channelCount(), source.sampleRate(), 0, 0, sdr::SignalType::SAMPLE_REAL);

      if (source.read(samples) > 0)
      {
         for (const nfc::NfcFrame &frame: decoder.nextFrames(samples))
         {
            if (frame.isPollFrame() || frame.isListenFrame())
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

   std::list<nfc::NfcFrame> list1;
   std::list<nfc::NfcFrame> list2;

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
   for (const auto &entry: rt::FileSystem::directoryList(path))
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
   logger.info("***********************************************************************");
   logger.info("NFC laboratory, 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   logger.info("***********************************************************************");

   for (int i = 1; i < argc; i++)
   {
      std::string path {argv[i]};

      if (rt::FileSystem::isDirectory(path))
      {
         logger.info("processing path {}", {path});

         testPath(path);
      }
      else if (rt::FileSystem::isRegularFile(path))
      {
         logger.info("processing file {}", {path});

         testFile(path);
      }
   }

   return 0;
}



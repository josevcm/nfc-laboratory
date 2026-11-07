/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2026 Jose Vicente Campos Martinez, <josevcm@gmail.com>

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

/*
 * Decoder regression test.
 *
 * Decodes one or more radio captures (.wav) with lab::NfcDecoder and compares the decoded frames against the
 * expected frames stored in a JSON file with the same name (.json), using the same format as the trace storage.
 *
 * Only the fields present in each expected frame are compared, so reference captures can check exact sample
 * positions while synthetic captures may check only frame contents. Carrier frames (NfcAnyTech) are ignored.
 *
 * Exit code is 0 when all captures match, 1 otherwise.
 */

#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include <rt/Logger.h>

#include <hw/RecordDevice.h>
#include <hw/SignalBuffer.h>
#include <hw/SignalType.h>

#include <lab/data/RawFrame.h>
#include <lab/nfc/NfcDecoder.h>

using json = nlohmann::json;

static bool verbose = false;

/*
 * Format frame data as colon separated hex bytes, same format as trace storage
 */
static std::string frameData(const lab::RawFrame &frame)
{
   std::ostringstream out;

   for (unsigned int i = 0; i < frame.limit(); i++)
   {
      if (i > 0)
         out << ":";

      out << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(frame[i]);
   }

   return out.str();
}

/*
 * Convert decoded frame to JSON object with the fields used for comparison
 */
static json frameObject(const lab::RawFrame &frame)
{
   return {
      {"techType", frame.techType()},
      {"frameType", frame.frameType()},
      {"framePhase", frame.framePhase()},
      {"frameFlags", frame.frameFlags()},
      {"frameRate", frame.frameRate()},
      {"sampleStart", frame.sampleStart()},
      {"sampleEnd", frame.sampleEnd()},
      {"frameData", frameData(frame)}
   };
}

/*
 * Decode all frames from a radio capture
 */
static bool decodeCapture(const std::string &path, std::vector<lab::RawFrame> &frames)
{
   hw::RecordDevice device(path);

   if (!device.open(hw::SignalDevice::Mode::Read))
   {
      std::cerr << "unable to open capture " << path << std::endl;
      return false;
   }

   const auto sampleRate = device.get<unsigned int>(hw::SignalDevice::PARAM_SAMPLE_RATE);
   const auto channelCount = device.get<unsigned int>(hw::SignalDevice::PARAM_CHANNEL_COUNT);

   if (channelCount != 1 && channelCount != 2)
   {
      std::cerr << "unsupported channel count " << channelCount << " in " << path << std::endl;
      return false;
   }

   lab::NfcDecoder decoder;

   decoder.setSampleRate(sampleRate);
   decoder.initialize();

   while (device.isOpen() && !device.isEof())
   {
      const hw::SignalType type = channelCount == 1 ? hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES : hw::SignalType::SIGNAL_TYPE_RADIO_IQ;

      hw::SignalBuffer buffer(65536 * channelCount, channelCount, 1, sampleRate, 0, 0, type);

      if (device.read(buffer) <= 0)
         break;

      // convert I/Q samples to magnitude, as signal storage task does
      if (channelCount == 2)
      {
         hw::SignalBuffer magnitude(buffer.elements(), 1, 1, sampleRate, 0, 0, hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES);

         const float *src = buffer.data();
         float *dst = magnitude.push(buffer.elements());

         for (unsigned int i = 0; i < buffer.elements(); i++)
            dst[i] = std::sqrt(src[2 * i] * src[2 * i] + src[2 * i + 1] * src[2 * i + 1]);

         magnitude.flip();

         buffer = magnitude;
      }

      for (const auto &frame: decoder.nextFrames(buffer))
      {
         if (frame.techType() != lab::NfcAnyTech)
            frames.push_back(frame);
      }
   }

   decoder.cleanup();

   return true;
}

/*
 * Run test for one capture, returns true if decoded frames match expected frames
 */
static bool testCapture(const std::string &capture)
{
   const std::string expectedPath = capture.substr(0, capture.find_last_of('.')) + ".json";

   std::ifstream input(expectedPath);

   if (!input)
   {
      std::cerr << "FAIL " << capture << ": unable to read expected frames " << expectedPath << std::endl;
      return false;
   }

   json expected;

   try
   {
      input >> expected;
   }
   catch (const std::exception &e)
   {
      std::cerr << "FAIL " << capture << ": invalid json " << expectedPath << ": " << e.what() << std::endl;
      return false;
   }

   std::vector<json> expectedFrames;

   for (const auto &frame: expected["frames"])
   {
      if (frame.value("techType", 0) != lab::NfcAnyTech)
         expectedFrames.push_back(frame);
   }

   std::vector<lab::RawFrame> decoded;

   if (!decodeCapture(capture, decoded))
   {
      std::cerr << "FAIL " << capture << std::endl;
      return false;
   }

   int errors = 0;

   const size_t count = std::max(expectedFrames.size(), decoded.size());

   for (size_t i = 0; i < count; i++)
   {
      if (i >= expectedFrames.size())
      {
         std::cerr << "  frame " << i << ": unexpected " << frameObject(decoded[i]).dump() << std::endl;
         errors++;
         continue;
      }

      if (i >= decoded.size())
      {
         std::cerr << "  frame " << i << ": missing " << expectedFrames[i].dump() << std::endl;
         errors++;
         continue;
      }

      const json &want = expectedFrames[i];
      const json got = frameObject(decoded[i]);

      for (const auto &field: got.items())
      {
         if (!want.contains(field.key()))
            continue;

         json value = want[field.key()];

         // frame data comparison is case insensitive
         if (field.key() == "frameData")
         {
            std::string data = value.get<std::string>();

            for (auto &c: data)
               c = static_cast<char>(std::toupper(c));

            value = data;
         }

         if (value != field.value())
         {
            std::cerr << "  frame " << i << ": " << field.key() << " expected " << value.dump() << " decoded " << field.value().dump() << std::endl;
            errors++;
         }
      }

      if (verbose)
         std::cout << "  frame " << i << ": " << got.dump() << std::endl;
   }

   std::cout << (errors ? "FAIL " : "PASS ") << capture << " (" << decoded.size() << " frames decoded, " << expectedFrames.size() << " expected)" << std::endl;

   return errors == 0;
}

static void printUsage(const char *program)
{
   std::cout << "Usage: " << program << " [-v] <capture.wav> [<capture.wav> ...]" << std::endl;
   std::cout << std::endl;
   std::cout << "Decodes each capture and compares the decoded frames with <capture>.json." << std::endl;
   std::cout << "Only the fields present in the expected frames are compared." << std::endl;
   std::cout << std::endl;
   std::cout << "  -v   print decoded frames" << std::endl;
}

int main(int argc, char *argv[])
{
   rt::Logger::init(std::cerr, rt::Logger::WARN_LEVEL, false);

   std::vector<std::string> captures;

   for (int i = 1; i < argc; i++)
   {
      if (std::strcmp(argv[i], "-v") == 0)
         verbose = true;
      else if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0)
      {
         printUsage(argv[0]);
         return 0;
      }
      else
         captures.emplace_back(argv[i]);
   }

   if (captures.empty())
   {
      printUsage(argv[0]);
      return 1;
   }

   int failed = 0;

   for (const auto &capture: captures)
   {
      if (!testCapture(capture))
         failed++;
   }

   if (captures.size() > 1)
      std::cout << (captures.size() - failed) << " of " << captures.size() << " captures passed" << std::endl;

   rt::Logger::flush();

   return failed ? 1 : 0;
}

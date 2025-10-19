/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2025 Jose Vicente Campos Martinez, <josevcm@gmail.com>
  Copyright (C) 2025 Steffen Wittemeier, <>

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

#include <lab/data/RawFrame.h>

#include <lab/nfc/Nfc.h>

#include <QCoreApplication>

#include <protocol/ProtocolParser.h>
#include <protocol/ProtocolFrame.h>

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
 * Convert ProtocolFrame tree to JSON
 */
json protocolFrameToJson(ProtocolFrame *frame)
{
   if (!frame)
      return nullptr;

   json result;

   // Get frame data
   QString name = frame->data(ProtocolFrame::Name).toString();
   QVariant dataVar = frame->data(ProtocolFrame::Data);
   int flags = frame->data(ProtocolFrame::Flags).toInt();

   result["name"] = name.toStdString();

   // Add data if it's a byte array
   if (dataVar.typeId() == QMetaType::QByteArray)
   {
      QByteArray bytes = dataVar.toByteArray();

      if (!bytes.isEmpty())
      {
         std::ostringstream hexStr;
         for (int i = 0; i < bytes.size(); i++)
         {
            if (i > 0) hexStr << " ";
            hexStr << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
                   << (static_cast<unsigned int>(static_cast<unsigned char>(bytes[i])));
         }
         result["data"] = hexStr.str();
      }
   }
   else if (!dataVar.toString().isEmpty())
   {
      result["data"] = dataVar.toString().toStdString();
   }

   // Add flags
   json flagsArray = json::array();
   if (flags & ProtocolFrame::CrcError)
      flagsArray.push_back("CRC_ERROR");
   if (flags & ProtocolFrame::ParityError)
      flagsArray.push_back("PARITY_ERROR");
   if (flags & ProtocolFrame::SyncError)
      flagsArray.push_back("SYNC_ERROR");

   if (!flagsArray.empty())
      result["flags"] = flagsArray;

   // Recursively add children
   if (frame->childCount() > 0)
   {
      json children = json::array();
      for (int i = 0; i < frame->childCount(); i++)
      {
         children.push_back(protocolFrameToJson(frame->child(i)));
      }
      result["children"] = children;
   }

   return result;
}

/*
 * Convert ProtocolFrame tree to formatted string
 */
void printProtocolFrame(ProtocolFrame *frame, std::ostream &out, int depth = 0)
{
   if (!frame)
      return;

   // Create indentation
   std::string indent(depth * 2, ' ');

   // Get frame data
   QString name = frame->data(ProtocolFrame::Name).toString();
   QVariant dataVar = frame->data(ProtocolFrame::Data);
   int flags = frame->data(ProtocolFrame::Flags).toInt();

   // Print frame name
   out << indent << name.toStdString();

   // Print data if it's a byte array
   if (dataVar.typeId() == QMetaType::QByteArray)
   {
      QByteArray bytes = dataVar.toByteArray();

      if (!bytes.isEmpty())
      {
         out << ": ";

         for (int i = 0; i < bytes.size(); i++)
         {
            if (i > 0) out << " ";
            out << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
                << (static_cast<unsigned int>(static_cast<unsigned char>(bytes[i])));
         }
         out << std::dec;
      }

      // Print error flags if present
      if (flags & ProtocolFrame::CrcError)
         out << " [CRC ERROR]";
      if (flags & ProtocolFrame::ParityError)
         out << " [PARITY ERROR]";
      if (flags & ProtocolFrame::SyncError)
         out << " [SYNC ERROR]";
   }
   else if (!dataVar.toString().isEmpty())
   {
      // Print non-byte array data
      out << ": " << dataVar.toString().toStdString();
   }

   out << std::endl;

   // Recursively print children
   for (int i = 0; i < frame->childCount(); i++)
   {
      printProtocolFrame(frame->child(i), out, depth + 1);
   }
}

/*
 * Get techType as string
 */
std::string techTypeToString(int techType)
{
   switch (techType)
   {
      case lab::FrameTech::NfcATech: return "NFC-A";
      case lab::FrameTech::NfcBTech: return "NFC-B";
      case lab::FrameTech::NfcFTech: return "NFC-F";
      case lab::FrameTech::NfcVTech: return "NFC-V";
      case lab::FrameTech::Iso7816Tech: return "ISO7816";
      default: return "UNKNOWN";
   }
}

/*
 * Get frameType as string
 */
std::string frameTypeToString(int frameType)
{
   switch (frameType)
   {
      case lab::FrameType::NfcPollFrame: return "POLL";
      case lab::FrameType::NfcListenFrame: return "LISTEN";
      case lab::FrameType::IsoATRFrame: return "ATR";
      case lab::FrameType::IsoRequestFrame: return "REQUEST";
      case lab::FrameType::IsoResponseFrame: return "RESPONSE";
      case lab::FrameType::IsoExchangeFrame: return "EXCHANGE";
      default: return "UNKNOWN";
   }
}

/*
 * Parse frames and output as JSON
 */
int parseFileJson(const std::string &jsonFile)
{
   std::list<lab::RawFrame> frames;

   // read frames from json file
   if (!readFrames(jsonFile, frames))
   {
      logger->error("failed to read frames from {}", {jsonFile});
      return -1;
   }

   logger->info("loaded {} frames from {}", {frames.size(), jsonFile});

   // create protocol parser
   ProtocolParser parser;

   json output;
   output["file"] = jsonFile;
   output["frames"] = json::array();

   int frameNumber = 0;

   // parse each frame
   for (const auto &rawFrame: frames)
   {
      frameNumber++;

      json frameJson;
      frameJson["frameNumber"] = frameNumber;
      frameJson["techType"] = techTypeToString(rawFrame.techType());
      frameJson["frameType"] = frameTypeToString(rawFrame.frameType());

      // Add frame flags
      json flagsArray = json::array();
      if (rawFrame.hasFrameFlags(lab::FrameFlags::CrcError))
         flagsArray.push_back("CRC-ERROR");
      if (rawFrame.hasFrameFlags(lab::FrameFlags::ParityError))
         flagsArray.push_back("PARITY-ERROR");
      if (rawFrame.hasFrameFlags(lab::FrameFlags::Encrypted))
         flagsArray.push_back("ENCRYPTED");

      if (!flagsArray.empty())
         frameJson["flags"] = flagsArray;

      // Add raw data bytes
      std::ostringstream hexStr;
      for (int i = 0; i < rawFrame.limit(); i++)
      {
         if (i > 0) hexStr << " ";
         hexStr << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
                << (static_cast<unsigned int>(static_cast<unsigned char>(rawFrame[i])));
      }
      frameJson["rawData"] = hexStr.str();

      // parse frame
      ProtocolFrame *parsed = parser.parse(rawFrame);

      if (parsed)
      {
         frameJson["parsed"] = protocolFrameToJson(parsed);
         delete parsed;
      }
      else
      {
         frameJson["parsed"] = nullptr;
         logger->warn("failed to parse frame {}", {frameNumber});
      }

      output["frames"].push_back(frameJson);
   }

   output["totalFrames"] = frameNumber;

   // Output JSON
   std::cout << output.dump(2) << std::endl;

   return 0;
}

/*
 * Parse frames and output protocol interpretation (text format)
 */
int parseFile(const std::string &jsonFile)
{
   std::list<lab::RawFrame> frames;

   // read frames from json file
   if (!readFrames(jsonFile, frames))
   {
      logger->error("failed to read frames from {}", {jsonFile});
      return -1;
   }

   logger->info("loaded {} frames from {}", {frames.size(), jsonFile});

   // create protocol parser
   ProtocolParser parser;

   std::cout << std::endl;
   std::cout << "================================================================================" << std::endl;
   std::cout << "Protocol Parser Output: " << jsonFile << std::endl;
   std::cout << "================================================================================" << std::endl;
   std::cout << std::endl;

   int frameNumber = 0;

   // parse each frame
   for (const auto &rawFrame: frames)
   {
      frameNumber++;

      // Print frame metadata that affects parsing
      std::cout << "Frame " << frameNumber << " [";
      std::cout << techTypeToString(rawFrame.techType()) << ", ";
      std::cout << frameTypeToString(rawFrame.frameType());

      // FrameFlags (only those that affect parsing)
      if (rawFrame.hasFrameFlags(lab::FrameFlags::CrcError))
         std::cout << ", CRC-ERROR";
      if (rawFrame.hasFrameFlags(lab::FrameFlags::ParityError))
         std::cout << ", PARITY-ERROR";
      if (rawFrame.hasFrameFlags(lab::FrameFlags::Encrypted))
         std::cout << ", ENCRYPTED";

      std::cout << "]: ";

      // Print raw data bytes
      for (int i = 0; i < rawFrame.limit(); i++)
      {
         if (i > 0) std::cout << " ";
         std::cout << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
                   << (static_cast<unsigned int>(static_cast<unsigned char>(rawFrame[i])));
      }
      std::cout << std::dec << std::endl;

      // parse frame
      ProtocolFrame *parsed = parser.parse(rawFrame);

      if (parsed)
      {
         std::cout << "  [PARSED]" << std::endl;
         printProtocolFrame(parsed, std::cout, 2);
         std::cout << std::endl;

         // cleanup
         delete parsed;
      }
      else
      {
         logger->warn("failed to parse frame {}", {frameNumber});
      }
   }

   std::cout << "================================================================================" << std::endl;
   std::cout << "Total frames parsed: " << frameNumber << std::endl;
   std::cout << "================================================================================" << std::endl;
   std::cout << std::endl;

   return 0;
}

int parsePath(const std::string &path, bool jsonOutput)
{
   int totalFiles = 0;
   int totalSuccess = 0;

   for (const auto &entry: FileSystem::directoryList(path))
   {
      if (entry.name.find(".json") != std::string::npos)
      {
         totalFiles++;

         int result = jsonOutput ? parseFileJson(entry.name) : parseFile(entry.name);
         if (result == 0)
            totalSuccess++;
      }
   }

   logger->info("processed {} files, {} successful", {totalFiles, totalSuccess});

   return 0;
}

void printUsage(const char *programName)
{
   std::cout << "NFC Protocol Parser - Test Tool" << std::endl;
   std::cout << std::endl;
   std::cout << "Usage: " << programName << " [OPTIONS] <json-file|directory>" << std::endl;
   std::cout << std::endl;
   std::cout << "Description:" << std::endl;
   std::cout << "  Parse NFC protocol frames from JSON test files and output the protocol" << std::endl;
   std::cout << "  interpretation. Supports NFC-A, NFC-B, NFC-F, NFC-V, and ISO7816." << std::endl;
   std::cout << std::endl;
   std::cout << "Options:" << std::endl;
   std::cout << "  --json        Output in JSON format instead of human-readable text" << std::endl;
   std::cout << "  --help, -h    Show this help message and exit" << std::endl;
   std::cout << std::endl;
   std::cout << "Arguments:" << std::endl;
   std::cout << "  <json-file>   Path to a JSON file containing raw NFC frames" << std::endl;
   std::cout << "  <directory>   Path to a directory containing multiple JSON files" << std::endl;
   std::cout << std::endl;
   std::cout << "Output Formats:" << std::endl;
   std::cout << "  Text format (default):" << std::endl;
   std::cout << "    - Human-readable hierarchical structure" << std::endl;
   std::cout << "    - Shows frame metadata (TechType, FrameType, Flags)" << std::endl;
   std::cout << "    - Displays parsed protocol fields with interpretations" << std::endl;
   std::cout << std::endl;
   std::cout << "  JSON format (--json):" << std::endl;
   std::cout << "    - Machine-readable structured data" << std::endl;
   std::cout << "    - Includes all frame metadata and parsed protocol tree" << std::endl;
   std::cout << "    - Easy to process with scripts and automation tools" << std::endl;
   std::cout << std::endl;
   std::cout << "Examples:" << std::endl;
   std::cout << "  " << programName << " wav/test_NFC-A_106kbps_001.json" << std::endl;
   std::cout << "    Parse a single file and output in text format" << std::endl;
   std::cout << std::endl;
   std::cout << "  " << programName << " --json wav/test_NFC-A_106kbps_001.json" << std::endl;
   std::cout << "    Parse a single file and output in JSON format" << std::endl;
   std::cout << std::endl;
   std::cout << "  " << programName << " wav/" << std::endl;
   std::cout << "    Parse all JSON files in the wav/ directory" << std::endl;
   std::cout << std::endl;
   std::cout << "  " << programName << " --json wav/ > output.json" << std::endl;
   std::cout << "    Parse all files and redirect JSON output to a file" << std::endl;
   std::cout << std::endl;
}

int main(int argc, char *argv[])
{
   // Initialize Qt Core Application (required for QObject-based classes)
   QCoreApplication app(argc, argv);

   logger->info("***********************************************************************");
   logger->info("NFC laboratory, 2024 Jose Vicente Campos Martinez - <josevcm@gmail.com>");
   logger->info("***********************************************************************");

   if (argc < 2)
   {
      printUsage(argv[0]);
      return 1;
   }

   bool jsonOutput = false;
   int startIndex = 1;

   // Check for flags
   for (int i = 1; i < argc; i++)
   {
      std::string arg(argv[i]);

      if (arg == "--help" || arg == "-h")
      {
         printUsage(argv[0]);
         return 0;
      }
      else if (arg == "--json")
      {
         jsonOutput = true;
         startIndex = i + 1;
      }
      else
      {
         // First non-flag argument
         startIndex = i;
         break;
      }
   }

   if (startIndex >= argc)
   {
      std::cerr << "Error: No input file or directory specified" << std::endl;
      std::cerr << "Use --help for usage information" << std::endl;
      return 1;
   }

   for (int i = startIndex; i < argc; i++)
   {
      std::string path {argv[i]};

      if (FileSystem::isDirectory(path))
      {
         logger->info("processing path {}", {path});

         parsePath(path, jsonOutput);
      }
      else if (FileSystem::isRegularFile(path))
      {
         logger->info("processing file {}", {path});

         if (jsonOutput)
            parseFileJson(path);
         else
            parseFile(path);
      }
      else
      {
         logger->error("invalid path: {}", {path});
         return 1;
      }
   }

   return 0;
}

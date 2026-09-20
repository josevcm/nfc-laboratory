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

#include <algorithm>
#include <cstdint>
#include <vector>

#include <rt/Format.h>
#include <rt/Package.h>
#include <rt/FileSystem.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <lab/data/RawFrame.h>

#include <lab/logic/ClockDetector.h>
#include <lab/tasks/TraceStorageTask.h>

#include "AbstractTask.h"

#define INFO_FLAGS 0
#define INFO_START_OFFSET 1
#define INFO_TOTAL_SAMPLES 2
#define INFO_STREAM_ID 3
#define INFO_SAMPLE_RATE 4
#define INFO_STREAM_BYTES 5 // byte length of the offset stream in APCM v3 (SoA layout)

// maximum batch size (in points) used when decoding a v3 entry into SignalBuffer chunks
#define DECODE_BATCH_SIZE 16384

using value_t = nlohmann::detail::value_t;

namespace lab {

static const std::map<int, std::string> storageError = {
   {TraceStorageTask::NoError, ""},
   {TraceStorageTask::MissingParameters, "Missing parameters"},
   {TraceStorageTask::MissingFileName, "Missing file name"},
   {TraceStorageTask::FileOpenFailed, "File open failed"},
   {TraceStorageTask::ReadDataFailed, "Read data failed"},
   {TraceStorageTask::WriteDataFailed, "Write data failed"},
   {TraceStorageTask::InvalidStorageFormat, "Invalid storage format"},
};

struct SampleHdr
{
   char magic[4];
   uint32_t version;
   uint32_t info[6];
};

// LEB128 unsigned varint, used for APCM v3 offset deltas (unbounded, unlike the 1-byte v1/v2 delta)
static void putVarUInt(std::vector<uint8_t> &out, uint32_t value)
{
   while (value >= 0x80)
   {
      out.push_back(static_cast<uint8_t>(value) | 0x80);
      value >>= 7;
   }

   out.push_back(static_cast<uint8_t>(value));
}

// reads a LEB128 unsigned varint, advancing 'p'; returns false on truncated or malformed input
static bool getVarUInt(const uint8_t *&p, const uint8_t *end, uint32_t &value)
{
   value = 0;

   for (int shift = 0; p < end && shift <= 28; shift += 7)
   {
      const uint8_t b = *p++;

      value |= static_cast<uint32_t>(b & 0x7F) << shift;

      if (!(b & 0x80))
         return true;
   }

   return false;
}

// zig-zag encoding maps signed deltas to unsigned so small negative and positive values both stay short
static inline uint32_t zigzagEncode(int32_t value)
{
   return (static_cast<uint32_t>(value) << 1) ^ static_cast<uint32_t>(value >> 31);
}

static inline int32_t zigzagDecode(uint32_t value)
{
   return static_cast<int32_t>(value >> 1) ^ -static_cast<int32_t>(value & 1);
}

struct TraceStorageTask::Impl : TraceStorageTask, AbstractTask
{
   // out storage subject for frames
   rt::Subject<RawFrame> *storageFrameStream = nullptr;

   // out storage subject for signals
   rt::Subject<hw::SignalBuffer> *storageSignalStream = nullptr;

   // input signal stream subject from receiver
   rt::Subject<hw::SignalBuffer> *adaptiveSignalStream = nullptr;

   // input frame stream subject from decoder
   rt::Subject<RawFrame> *logicDecoderFrameStream = nullptr;
   rt::Subject<RawFrame> *radioDecoderFrameStream = nullptr;

   // frame stream subscription
   rt::Subject<RawFrame>::Subscription logicDecoderFrameSubscription;
   rt::Subject<RawFrame>::Subscription radioDecoderFrameSubscription;

   // signal stream subscription
   rt::Subject<hw::SignalBuffer>::Subscription adaptiveSignalSubscription;

   // frame stream queue buffer
   rt::BlockingQueue<RawFrame> frameQueue;

   // signal stream queue buffer
   rt::BlockingQueue<hw::SignalBuffer> logicSignalQueue;
   rt::BlockingQueue<hw::SignalBuffer> radioSignalQueue;
   rt::BlockingQueue<hw::SignalBuffer> clockSignalQueue;

   Impl() : AbstractTask("worker.TraceStorage", "storage")
   {
      // create storage stream subjects
      storageFrameStream = rt::Subject<RawFrame>::name("storage.frame");
      storageSignalStream = rt::Subject<hw::SignalBuffer>::name("storage.signal");

      // create input frame and signal subjects
      logicDecoderFrameStream = rt::Subject<RawFrame>::name("logic.decoder.frame");
      radioDecoderFrameStream = rt::Subject<RawFrame>::name("radio.decoder.frame");
      adaptiveSignalStream = rt::Subject<hw::SignalBuffer>::name("adaptive.signal");

      // subscribe to logic decoder
      logicDecoderFrameSubscription = logicDecoderFrameStream->subscribe([this](const RawFrame &frame) {
         if (frame.isValid())
            frameQueue.add(frame);
      });

      // subscribe to radio decoder
      radioDecoderFrameSubscription = radioDecoderFrameStream->subscribe([this](const RawFrame &frame) {
         if (frame.isValid())
            frameQueue.add(frame);
      });

      // subscribe to signal events
      adaptiveSignalSubscription = adaptiveSignalStream->subscribe([this](const hw::SignalBuffer &buffer) {
         if (buffer.isValid())
         {
            switch (buffer.type())
            {
               case hw::SignalType::SIGNAL_TYPE_LOGIC_SIGNAL:
                  logicSignalQueue.add(buffer);
                  break;
               case hw::SignalType::SIGNAL_TYPE_RADIO_SIGNAL:
                  radioSignalQueue.add(buffer);
                  break;
               case hw::SignalType::SIGNAL_TYPE_CLK_SIGNAL:
                  clockSignalQueue.add(buffer);
                  break;
               default:
                  break;
            }
         }
      });
   }

   void start() override
   {
   }

   void stop() override
   {
   }

   bool loop() override
   {
      /*
       * process pending commands
       */
      if (auto command = commandQueue.get())
      {
         log->debug("command [{}]", {command->code});

         switch (command->code)
         {
            case Read:
               readFile(command.value());
               break;

            case Write:
               writeFile(command.value());
               break;

            case Clear:
               clearQueue(command.value());
               break;

            default:
               log->warn("unknown command {}", {command->code});
               command->reject(UnknownCommand);
               return true;
         }
      }

      wait(50);

      return true;
   }

   void readFile(const rt::Event &command)
   {
      int error = MissingParameters;

      while (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log->info("read file command: {}", {config.dump()});

         if (!config.contains("fileName"))
         {
            log->info("reading failed, no fileName");
            error = MissingFileName;
            break;
         }

         if ((error = readTraceFile(config["fileName"])) == NoError)
         {
            command.resolve();

            return;
         }

         break;
      }

      // clear cache
      frameQueue.clear();
      logicSignalQueue.clear();
      radioSignalQueue.clear();
      clockSignalQueue.clear();

      command.reject(error, storageError.at(error));
   }

   void writeFile(const rt::Event &command)
   {
      int error = MissingParameters;

      while (auto data = command.get<std::string>("data"))
      {
         auto config = json::parse(data.value());

         log->info("write file command: {}", {config.dump()});

         if (!config.contains("fileName"))
         {
            log->info("writing failed, no fileName");
            error = MissingFileName;
            break;
         }

         const double rangeStart = config.contains("timeStart") ? static_cast<double>(config["timeStart"]) : 0.0;
         const double rangeEnd = config.contains("timeEnd") ? static_cast<double>(config["timeEnd"]) : 0.0;

         if ((error = writeTraceFile(config["fileName"], rangeStart, rangeEnd)) != NoError)
            break;

         command.resolve();

         return;
      }

      command.reject(error);
   }

   void clearQueue(const rt::Event &command)
   {
      log->info("clear {} entries from frame cache", {frameQueue.size()});
      frameQueue.clear();

      log->info("clear {} entries from logic buffer cache", {logicSignalQueue.size()});
      logicSignalQueue.clear();

      log->info("clear {} entries from radio buffer cache", {radioSignalQueue.size()});
      radioSignalQueue.clear();

      log->info("clear {} entries from clock buffer cache", {clockSignalQueue.size()});
      clockSignalQueue.clear();

      log->info("clear all entries completed!");

      command.resolve();
   }

   int readTraceFile(const std::string &file)
   {
      int result = NoError;

      log->info("read trace file {}", {file});

      // update storage status
      updateStorageStatus(Reading, 0);

      // open package
      if (rt::Package package(file); package.open(rt::Package::Read) == 0)
      {
         // clear current cache
         frameQueue.clear();
         logicSignalQueue.clear();
         radioSignalQueue.clear();
         clockSignalQueue.clear();

         std::string name;
         unsigned int length;

         while (package.getEntry(name, length) == 0)
         {
            log->info("read entry {} length {}", {name, length});

            if (name.find("frame") == 0)
            {
               result = readFrameEntry(package, length);
            }
            else if (name.find("logic") == 0)
            {
               result = readLogicEntry(package, length);
            }
            else if (name.find("radio") == 0)
            {
               result = readRadioEntry(package, length);
            }
            else if (name.find("clock") == 0)
            {
               result = readClockEntry(package, length);
            }
            else
            {
               log->warn("skip unknown entry: {}", {name});
            }

            if (result != NoError)
               break;

            package.nextEntry();
         }

         // close package
         package.close();
      }
      else
      {
         result = FileOpenFailed;
      }

      // finally update status
      if (result == NoError)
         updateStorageStatus(Complete, 100);
      else
         updateStorageStatus(Error, 100, storageError.at(result));

      return result;
   }

   int writeTraceFile(const std::string &file, double rangeStart, double rangeEnd)
   {
      int result = NoError;

      log->info("write trace file {}, range {} -> {}", {file, rangeStart, rangeEnd});

      // update storage status
      updateStorageStatus(Writing, 0, "writing trace file");

      // create package
      rt::Package package(file);

      // open package file
      while (package.open(rt::Package::Write) == 0)
      {
         // add frames entry
         if ((result = writeFrameEntry(package, rangeStart, rangeEnd)) != NoError)
            break;

         // add logic signal
         if ((result = writeLogicData(package, rangeStart, rangeEnd)) != NoError)
            break;

         // add radio signal
         if ((result = writeRadioData(package, rangeStart, rangeEnd)) != NoError)
            break;

         // add clock frequency band
         if ((result = writeClockData(package, rangeStart, rangeEnd)) != NoError)
            break;

         break;
      }

      if (package.isOpen())
         package.close();
      else
         result = FileOpenFailed;

      // finally update status
      if (result == NoError)
         updateStorageStatus(Complete, 100);
      else
         updateStorageStatus(Error, 100, storageError.at(result));

      return result;
   }

   int readFrameEntry(rt::Package &package, unsigned int length)
   {
      if (length <= 0)
      {
         log->error("invalid frames entry size");
         return InvalidStorageFormat;
      }

      const std::shared_ptr<char> data(new char[length + 1]);

      if (package.readData(data.get(), length) != 0)
      {
         log->error("failed to read frame data");
         return ReadDataFailed;
      }

      // parse
      json info = json::parse(data.get(), data.get() + length, nullptr, false);

      if (info == value_t::discarded)
      {
         log->error("failed to parse frame data");
         return InvalidStorageFormat;
      }

      if (!info.contains("frames"))
      {
         log->error("no frame data found");
         return InvalidStorageFormat;
      }

      // read frames from file
      for (const auto &frame: info["frames"])
      {
         RawFrame nfcFrame(256);

         // check mandatory fields
         if (!frame.contains("techType") ||
            !frame.contains("frameType") ||
            !frame.contains("framePhase") ||
            !frame.contains("frameFlags") ||
            !frame.contains("frameRate") ||
            !frame.contains("sampleStart") ||
            !frame.contains("sampleEnd") ||
            !frame.contains("sampleRate") ||
            !frame.contains("timeStart") ||
            !frame.contains("timeEnd") ||
            !frame.contains("dateTime"))
         {
            log->error("invalid frame format, missing one or more properties");
            return InvalidStorageFormat;
         }

         // load frame properties
         nfcFrame.setTechType(frame["techType"]);
         nfcFrame.setFrameType(frame["frameType"]);
         nfcFrame.setFramePhase(frame["framePhase"]);
         nfcFrame.setFrameFlags(frame["frameFlags"]);
         nfcFrame.setFrameRate(frame["frameRate"]);
         nfcFrame.setSampleStart(frame["sampleStart"]);
         nfcFrame.setSampleEnd(frame["sampleEnd"]);
         nfcFrame.setSampleRate(frame["sampleRate"]);
         nfcFrame.setTimeStart(frame["timeStart"]);
         nfcFrame.setTimeEnd(frame["timeEnd"]);
         nfcFrame.setDateTime(frame["dateTime"]);

         // check if frame contains data payload
         if (frame.contains("frameData"))
         {
            std::string frameData = frame["frameData"];

            for (size_t index = 0, size = 0; index < frameData.length(); index += size + 1)
            {
               nfcFrame.put(std::stoi(frameData.c_str() + index, &size, 16));
            }
         }

         nfcFrame.flip();

         // publish frame
         storageFrameStream->next(nfcFrame);

         // and store in local frame buffer
         frameQueue.add(nfcFrame);
      }

      // send final frame as EOF
      storageFrameStream->next({});

      return NoError;
   }

   int writeFrameEntry(rt::Package &package, double rangeStart, double rangeEnd)
   {
      json frames = json::array();

      for (const auto &frame: frameQueue)
      {
         json entry = json::object();

         if (frame.timeStart() < rangeStart || frame.timeEnd() > rangeEnd)
            continue;

         // prepare frame shift
         auto rangeOffset = static_cast<unsigned long>(frame.sampleRate() * rangeStart);

         // prepare frame properties
         entry["sampleStart"] = frame.sampleStart() - rangeOffset;
         entry["sampleEnd"] = frame.sampleEnd() - rangeOffset;
         entry["sampleRate"] = frame.sampleRate();
         entry["timeStart"] = frame.timeStart() - rangeStart;
         entry["timeEnd"] = frame.timeEnd() - rangeStart;
         entry["techType"] = frame.techType();
         entry["frameType"] = frame.frameType();
         entry["frameRate"] = frame.frameRate();
         entry["frameFlags"] = frame.frameFlags();
         entry["framePhase"] = frame.framePhase();
         entry["dateTime"] = frame.dateTime();

         // prepare frame data
         if (frame.size() > 0)
         {
            char buffer[16384];

            frame.reduce<int>(0, [&buffer](int offset, unsigned char value) {
               return offset + snprintf(buffer + offset, sizeof(buffer) - offset, offset > 0 ? ":%02X" : "%02X", value);
            });

            entry["frameData"] = buffer;
            entry["length"] = frame.size();
         }

         frames.push_back(entry);
      }

      // create json object
      const json info({{"frames", frames}});

      // convert to string
      const std::string content = info.dump();

      log->info("add frame entry with size {} and {} frames", {content.length(), static_cast<int>(frames.size())});

      // add entry header
      if (package.addEntry("frame.json", content.length()) != 0)
      {
         log->error("failed to add frames header");
         return WriteDataFailed;
      }

      // write json frame data
      if (package.writeData(content.c_str(), content.length()) != 0)
      {
         log->error("failed to write frames data");
         return WriteDataFailed;
      }

      return NoError;
   }

   int readLogicEntry(rt::Package &package, unsigned int length)
   {
      SampleHdr hdr {};

      unsigned int position;
      unsigned int sampleCount;
      unsigned int streamId = 0;
      unsigned int sampleRate = 0;
      unsigned char chunk[16384 * 2];

      if (length <= 0)
      {
         log->error("invalid signal entry size");
         return InvalidStorageFormat;
      }

      if (package.readData(&hdr, sizeof(hdr)) != 0)
      {
         log->error("failed to read signal chunk");
         return ReadDataFailed;
      }

      log->debug("read logic entry with size {}", {length});
      log->debug("\tstream id....: {}", {hdr.info[INFO_STREAM_ID]});
      log->debug("\tstream offset: {}", {hdr.info[INFO_START_OFFSET]});
      log->debug("\tsample rate..: {}", {hdr.info[INFO_SAMPLE_RATE]});
      log->debug("\ttotal samples: {}", {hdr.info[INFO_TOTAL_SAMPLES]});

      // APCM v3: varint offset deltas + raw level bytes, stored as two separate streams (SoA)
      if (hdr.version == 3)
      {
         if (std::strncmp(hdr.magic, "APCM", sizeof(hdr.magic)) != 0)
         {
            log->error("invalid signal chunk magic");
            return InvalidStorageFormat;
         }

         position = hdr.info[INFO_START_OFFSET];
         streamId = hdr.info[INFO_STREAM_ID];
         sampleCount = hdr.info[INFO_TOTAL_SAMPLES];
         sampleRate = hdr.info[INFO_SAMPLE_RATE];

         const unsigned int offsetBytes = hdr.info[INFO_STREAM_BYTES];
         const unsigned int payload = length - sizeof(hdr);

         if (offsetBytes > payload)
         {
            log->error("invalid signal chunk size");
            return InvalidStorageFormat;
         }

         const unsigned int valueBytes = payload - offsetBytes;

         std::vector<uint8_t> offsets(offsetBytes);
         std::vector<uint8_t> values(valueBytes);

         if (offsetBytes && package.readData(offsets.data(), offsetBytes) != 0)
         {
            log->error("failed to read signal offsets");
            return ReadDataFailed;
         }

         if (valueBytes && package.readData(values.data(), valueBytes) != 0)
         {
            log->error("failed to read signal values");
            return ReadDataFailed;
         }

         if (values.size() != sampleCount)
         {
            log->error("invalid signal chunk size");
            return InvalidStorageFormat;
         }

         const uint8_t *op = offsets.data(), *oe = op + offsets.size();

         unsigned int decoded = 0;

         while (decoded < sampleCount)
         {
            const unsigned int batch = std::min<unsigned int>(sampleCount - decoded, DECODE_BATCH_SIZE);
            const unsigned int base = position;

            unsigned int local = 0;

            hw::SignalBuffer buffer(batch * 2, 2, 1, sampleRate, base, 0, hw::SignalType::SIGNAL_TYPE_LOGIC_SIGNAL, streamId);

            for (unsigned int n = 0; n < batch; n++)
            {
               uint32_t deltaOffset;

               if (!getVarUInt(op, oe, deltaOffset))
               {
                  log->error("truncated signal offset stream");
                  return InvalidStorageFormat;
               }

               local += deltaOffset;

               buffer.put(static_cast<float>(values[decoded + n]));
               buffer.put(static_cast<float>(local));
            }

            position = base + local;
            decoded += batch;

            log->debug("\tread data, offset {} size {} start {}", {base, batch, base});

            buffer.flip();

            storageSignalStream->next(buffer);

            logicSignalQueue.add(buffer);
         }

         storageSignalStream->next({});

         return NoError;
      }

      switch (hdr.version)
      {
         case 1:
         {
            streamId = 0;
            position = hdr.info[INFO_START_OFFSET];
            sampleCount = hdr.info[INFO_TOTAL_SAMPLES];
            sampleRate = static_cast<unsigned int>(10E6);
            break;
         }

         case 2:
         {
            position = hdr.info[INFO_START_OFFSET];
            streamId = hdr.info[INFO_STREAM_ID];
            sampleCount = hdr.info[INFO_TOTAL_SAMPLES];
            sampleRate = hdr.info[INFO_SAMPLE_RATE];
            break;
         }
         default:
         {
            log->info("unsupported chunk version: {}", {hdr.version});
            return InvalidStorageFormat;
         }
      }

      // check header consistency
      if (length != sizeof(hdr) + sampleCount * 2)
      {
         log->error("invalid signal chunk size");
         return InvalidStorageFormat;
      }

      // check header magic
      if (std::strncmp(hdr.magic, "APCM", sizeof(hdr.magic)) != 0)
      {
         log->error("invalid signal chunk magic");
         return InvalidStorageFormat;
      }

      // update pending length
      length -= sizeof(hdr);

      // start reading
      while (length > 0)
      {
         unsigned int offset = 0;
         unsigned int size = length >= sizeof(chunk) ? sizeof(chunk) : length;

         if (package.readData(chunk, size) != 0)
         {
            log->error("failed to read signal data");
            return ReadDataFailed;
         }

         log->debug("\tread data, offset {} size {} start {}", {position, size, position + chunk[0]});

         hw::SignalBuffer buffer(size, 2, 1, sampleRate, position, 0, hw::SignalType::SIGNAL_TYPE_LOGIC_SIGNAL, streamId);

         for (int i = 0; i < size; i += 2)
         {
            offset += static_cast<int>(chunk[i + 0]);

            buffer.put(static_cast<float>(chunk[i + 1]));
            buffer.put(static_cast<float>(offset));
         }

         // update pending length
         length -= size;

         // compute position of next buffer
         position += offset;

         // flip buffer contents for transition
         buffer.flip();

         // publish buffer
         storageSignalStream->next(buffer);

         // and store in local signal buffer
         logicSignalQueue.add(buffer);
      }

      // send final buffer as EOF
      storageSignalStream->next({});

      return 0;
   }

   int writeLogicEntry(rt::Package &package, const std::string &name, unsigned int id, double rangeStart, double rangeEnd)
   {
      unsigned int sampleStart = 0;
      unsigned int sampleEnd = 0;
      unsigned int sampleCount = 0;

      // initialize header (v3: varint offset deltas + raw level bytes, stored as two separate streams)
      SampleHdr hdr {.magic = {'A', 'P', 'C', 'M'}, .version = 3, .info = {}};

      std::vector<uint8_t> offsets;
      std::vector<uint8_t> values;

      bool first = true;
      unsigned int lastOffset = 0;

      // encode samples in a single pass
      for (const auto &buffer: logicSignalQueue)
      {
         // skip other channels
         if (buffer.id() != id)
            continue;

         // catch first buffer
         if (first)
         {
            first = false;

            // compute sample range
            sampleStart = static_cast<unsigned int>(buffer.sampleRate() * rangeStart);
            sampleEnd = static_cast<unsigned int>(buffer.sampleRate() * rangeEnd);
            lastOffset = sampleStart;

            hdr.info[INFO_STREAM_ID] = buffer.id();
            // offsets are stored relative to the start of the saved range, as the frame and radio entries do, so
            // everything lands on the same time base when read back. Storing the capture offset here instead shifted
            // the signal ahead of the frames by the whole acquisition lead-in.
            hdr.info[INFO_START_OFFSET] = 0;
            hdr.info[INFO_SAMPLE_RATE] = buffer.sampleRate();
         }

         for (unsigned int i = 0; i < buffer.limit(); i += buffer.stride())
         {
            auto sample = buffer[i + 0] > 0.5 ? 1 : 0;
            auto offset = buffer.offset() + static_cast<unsigned int>(buffer[i + 1]);

            if (offset > sampleEnd)
               break;

            if (offset < sampleStart)
               continue;

            putVarUInt(offsets, offset - lastOffset);
            values.push_back(static_cast<uint8_t>(sample));

            // update differential values
            lastOffset = offset;
            sampleCount++;
         }
      }

      hdr.info[INFO_TOTAL_SAMPLES] = sampleCount;
      hdr.info[INFO_STREAM_BYTES] = static_cast<unsigned int>(offsets.size());

      const unsigned int size = sizeof(hdr) + offsets.size() + values.size();

      log->info("add logic entry {} with size {}", {name, size});
      log->debug("\tstream id....: {}", {hdr.info[INFO_STREAM_ID]});
      log->debug("\tstream offset: {}", {hdr.info[INFO_START_OFFSET]});
      log->debug("\tsample rate..: {}", {hdr.info[INFO_SAMPLE_RATE]});
      log->debug("\ttotal samples: {}", {hdr.info[INFO_TOTAL_SAMPLES]});

      // write entry header
      if (package.addEntry(name, size) != 0)
      {
         log->error("failed to add logic signal header");
         return WriteDataFailed;
      }

      // write signal header
      if (package.writeData(&hdr, sizeof(hdr)) != 0)
      {
         log->error("failed to write logic signal header");
         return WriteDataFailed;
      }

      // write offset stream, then value stream (SoA layout)
      if (!offsets.empty() && package.writeData(offsets.data(), offsets.size()) != 0)
      {
         log->error("failed to write logic signal offsets");
         return WriteDataFailed;
      }

      if (!values.empty() && package.writeData(values.data(), values.size()) != 0)
      {
         log->error("failed to write logic signal values");
         return WriteDataFailed;
      }

      log->info("\t{} samples stored for logic channel {}", {sampleCount, id});

      return NoError;
   }

   int readRadioEntry(rt::Package &package, unsigned int length)
   {
      SampleHdr hdr {};

      unsigned char chunk[16384 * 3];
      unsigned int position = 0;
      unsigned int sampleCount = 0;
      unsigned int streamId = 0;
      unsigned int sampleRate = 0;
      float scale = 1.0 / (1 << 15);

      if (length <= 0)
      {
         log->error("invalid signal entry size");
         return InvalidStorageFormat;
      }

      if (package.readData(&hdr, sizeof(hdr)) != 0)
      {
         log->error("failed to read signal chunk");
         return ReadDataFailed;
      }

      log->debug("read radio entry with size {}", {length});
      log->debug("\tstream id....: {}", {hdr.info[INFO_STREAM_ID]});
      log->debug("\tstream offset: {}", {hdr.info[INFO_START_OFFSET]});
      log->debug("\tsample rate..: {}", {hdr.info[INFO_SAMPLE_RATE]});
      log->debug("\ttotal samples: {}", {hdr.info[INFO_TOTAL_SAMPLES]});

      // APCM v3: varint offset deltas + zigzag-varint amplitude deltas, stored as two separate streams (SoA)
      if (hdr.version == 3)
      {
         if (std::strncmp(hdr.magic, "APCM", sizeof(hdr.magic)) != 0)
         {
            log->error("invalid signal chunk magic");
            return InvalidStorageFormat;
         }

         position = hdr.info[INFO_START_OFFSET];
         streamId = hdr.info[INFO_STREAM_ID];
         sampleCount = hdr.info[INFO_TOTAL_SAMPLES];
         sampleRate = hdr.info[INFO_SAMPLE_RATE];

         const unsigned int offsetBytes = hdr.info[INFO_STREAM_BYTES];
         const unsigned int payload = length - sizeof(hdr);

         if (offsetBytes > payload)
         {
            log->error("invalid signal chunk size");
            return InvalidStorageFormat;
         }

         const unsigned int valueBytes = payload - offsetBytes;

         std::vector<uint8_t> offsets(offsetBytes);
         std::vector<uint8_t> values(valueBytes);

         if (offsetBytes && package.readData(offsets.data(), offsetBytes) != 0)
         {
            log->error("failed to read signal offsets");
            return ReadDataFailed;
         }

         if (valueBytes && package.readData(values.data(), valueBytes) != 0)
         {
            log->error("failed to read signal values");
            return ReadDataFailed;
         }

         const uint8_t *op = offsets.data(), *oe = op + offsets.size();
         const uint8_t *vp = values.data(), *ve = vp + values.size();

         short sample = 0;
         unsigned int decoded = 0;

         while (decoded < sampleCount)
         {
            const unsigned int batch = std::min<unsigned int>(sampleCount - decoded, DECODE_BATCH_SIZE);
            const unsigned int base = position;

            unsigned int local = 0;

            hw::SignalBuffer buffer(batch * 2, 2, 1, sampleRate, base, 0, hw::SignalType::SIGNAL_TYPE_RADIO_SIGNAL, streamId);

            for (unsigned int n = 0; n < batch; n++)
            {
               uint32_t deltaOffset, deltaValue;

               if (!getVarUInt(op, oe, deltaOffset) || !getVarUInt(vp, ve, deltaValue))
               {
                  log->error("truncated signal stream");
                  return InvalidStorageFormat;
               }

               local += deltaOffset;
               sample = static_cast<short>(sample + zigzagDecode(deltaValue));

               buffer.put(static_cast<float>(sample) * scale);
               buffer.put(static_cast<float>(local));
            }

            position = base + local;
            decoded += batch;

            log->debug("\tread data, offset {} size {} start {}", {base, batch, base});

            buffer.flip();

            storageSignalStream->next(buffer);

            radioSignalQueue.add(buffer);
         }

         storageSignalStream->next({});

         return NoError;
      }

      switch (hdr.version)
      {
         case 1:
         {
            streamId = 0;
            position = hdr.info[INFO_START_OFFSET];
            sampleCount = hdr.info[INFO_TOTAL_SAMPLES];
            sampleRate = static_cast<unsigned int>(10E6);
            break;
         }

         case 2:
         {
            position = hdr.info[INFO_START_OFFSET];
            streamId = hdr.info[INFO_STREAM_ID];
            sampleCount = hdr.info[INFO_TOTAL_SAMPLES];
            sampleRate = hdr.info[INFO_SAMPLE_RATE];
            break;
         }
         default:
         {
            log->info("unsupported chunk version: {}", {hdr.version});
            return InvalidStorageFormat;
         }
      }

      // check header consistency
      if (length != sizeof(hdr) + sampleCount * 3)
      {
         log->error("invalid signal chunk size");
         return InvalidStorageFormat;
      }

      // check header magic
      if (std::strncmp(hdr.magic, "APCM", sizeof(hdr.magic)) != 0)
      {
         log->error("invalid signal chunk magic");
         return InvalidStorageFormat;
      }

      // update pending length
      length -= sizeof(hdr);

      short sample = 0;

      // start reading
      while (length > 0)
      {
         unsigned int offset = 0;
         unsigned int size = length >= sizeof(chunk) ? sizeof(chunk) : length;

         if (package.readData(chunk, size) != 0)
         {
            log->error("failed to read signal data");
            return ReadDataFailed;
         }

         log->debug("\tread data, offset {} size {} start {}", {position, size, position + chunk[0]});

         hw::SignalBuffer buffer((size / 3) * 2, 2, 1, sampleRate, position, 0, hw::SignalType::SIGNAL_TYPE_RADIO_SIGNAL, streamId);

         for (int i = 0; i < size; i += 3)
         {
            offset += chunk[i + 0];
            sample += static_cast<short>(chunk[i + 1] & 0xff | (chunk[i + 2] & 0xff) << 8);

            buffer.put(static_cast<float>(sample) * scale);
            buffer.put(static_cast<float>(offset));
         }

         // update pending length
         length -= size;

         // compute position of next buffer
         position += offset;

         // flip buffer contents for transition
         buffer.flip();

         // publish buffer
         storageSignalStream->next(buffer);

         // and store in local signal buffer
         radioSignalQueue.add(buffer);
      }

      // send final buffer as EOF
      storageSignalStream->next({});

      return 0;
   }

   int writeRadioEntry(rt::Package &package, const std::string &name, unsigned int id, double rangeStart, double rangeEnd)
   {
      unsigned int sampleStart = 0;
      unsigned int sampleEnd = 0;
      unsigned int sampleCount = 0;
      const float scale = (1 << 15);

      // initialize header (v3: varint offset deltas + zigzag-varint amplitude deltas, two separate streams)
      SampleHdr hdr {.magic = {'A', 'P', 'C', 'M'}, .version = 3, .info {}};

      std::vector<uint8_t> offsets;
      std::vector<uint8_t> values;

      bool first = true;
      short lastSample = 0;
      unsigned int lastOffset = 0;

      // encode samples in a single pass
      for (const auto &buffer: radioSignalQueue)
      {
         // skip other channels
         if (buffer.id() != id)
            continue;

         // catch first buffer
         if (first)
         {
            first = false;

            // compute sample range
            sampleStart = static_cast<unsigned int>(buffer.sampleRate() * rangeStart);
            sampleEnd = static_cast<unsigned int>(buffer.sampleRate() * rangeEnd);
            lastOffset = sampleStart;

            hdr.info[INFO_STREAM_ID] = buffer.id();
            hdr.info[INFO_START_OFFSET] = 0;
            hdr.info[INFO_SAMPLE_RATE] = buffer.sampleRate();
         }

         for (unsigned int i = 0; i < buffer.limit(); i += buffer.stride())
         {
            auto sample = static_cast<short>(buffer[i + 0] * scale);
            auto offset = buffer.offset() + static_cast<unsigned int>(buffer[i + 1]);

            if (offset > sampleEnd)
               break;

            if (offset < sampleStart)
               continue;

            putVarUInt(offsets, offset - lastOffset);
            putVarUInt(values, zigzagEncode(static_cast<int32_t>(sample) - static_cast<int32_t>(lastSample)));

            // update differential values
            lastOffset = offset;
            lastSample = sample;
            sampleCount++;
         }
      }

      hdr.info[INFO_TOTAL_SAMPLES] = sampleCount;
      hdr.info[INFO_STREAM_BYTES] = static_cast<unsigned int>(offsets.size());

      const unsigned int size = sizeof(hdr) + offsets.size() + values.size();

      log->info("add radio entry {} with size {}", {name, size});
      log->debug("\tstream id....: {}", {hdr.info[INFO_STREAM_ID]});
      log->debug("\tstream offset: {}", {hdr.info[INFO_START_OFFSET]});
      log->debug("\tsample rate..: {}", {hdr.info[INFO_SAMPLE_RATE]});
      log->debug("\ttotal samples: {}", {hdr.info[INFO_TOTAL_SAMPLES]});

      // add entry header
      if (package.addEntry(name, size) != 0)
      {
         log->error("failed to add radio signal header");
         return WriteDataFailed;
      }

      // write signal header
      if (package.writeData(&hdr, sizeof(hdr)) != 0)
      {
         log->error("failed to write radio signal header");
         return WriteDataFailed;
      }

      // write offset stream, then value stream (SoA layout)
      if (!offsets.empty() && package.writeData(offsets.data(), offsets.size()) != 0)
      {
         log->error("failed to write radio signal offsets");
         return WriteDataFailed;
      }

      if (!values.empty() && package.writeData(values.data(), values.size()) != 0)
      {
         log->error("failed to write radio signal values");
         return WriteDataFailed;
      }

      log->info("\t{} samples stored for radio channel {}", {sampleCount, id});

      return NoError;
   }

   int writeLogicData(rt::Package &package, double rangeStart, double rangeEnd)
   {
      int result;
      std::vector<unsigned int> channels;

      for (const auto &buffer: logicSignalQueue)
      {
         if (std::find(channels.begin(), channels.end(), buffer.id()) == channels.end())
            channels.push_back(buffer.id());
      }

      log->info("detected {} logic channels", {channels.size()});

      for (auto id: channels)
      {
         std::string name = rt::Format::format("logic-{}.apcm", {id});

         if ((result = writeLogicEntry(package, name, id, rangeStart, rangeEnd)) != NoError)
         {
            log->error("failed to write logic signal entry");
            return result;
         }
      }

      return NoError;
   }

   /*
    * Clock band entry: the frequency the clock probe runs at, as one point per change.
    *
    * It cannot ride along the logic entries because those binarize the value to a single byte, which is enough for a
    * probe level but not for a frequency. A capture holds tens of states rather than millions, so unlike the logic
    * entries this one skips varint packing and stores plain fixed width arrays: offsets as uint32, values as float.
    * The entry name shares no prefix with the ones older readers know, and readTraceFile skips what it does not
    * recognize, so files written here still open on builds that predate the band.
    */
   int readClockEntry(rt::Package &package, unsigned int length)
   {
      SampleHdr hdr {};

      if (length < sizeof(hdr))
      {
         log->error("invalid clock entry size");
         return InvalidStorageFormat;
      }

      if (package.readData(&hdr, sizeof(hdr)) != 0)
      {
         log->error("failed to read clock chunk");
         return ReadDataFailed;
      }

      if (std::strncmp(hdr.magic, "CLKF", sizeof(hdr.magic)) != 0)
      {
         log->error("invalid clock chunk magic");
         return InvalidStorageFormat;
      }

      if (hdr.version != 1)
      {
         log->info("unsupported clock chunk version: {}", {hdr.version});
         return InvalidStorageFormat;
      }

      const unsigned int streamId = hdr.info[INFO_STREAM_ID];
      const unsigned int stateCount = hdr.info[INFO_TOTAL_SAMPLES];
      const unsigned int sampleRate = hdr.info[INFO_SAMPLE_RATE];
      const unsigned int base = hdr.info[INFO_START_OFFSET];

      log->debug("read clock entry with size {}", {length});
      log->debug("\tstream id....: {}", {streamId});
      log->debug("\tstream offset: {}", {base});
      log->debug("\tsample rate..: {}", {sampleRate});
      log->debug("\ttotal states.: {}", {stateCount});

      if (length - sizeof(hdr) != static_cast<size_t>(stateCount) * (sizeof(uint32_t) + sizeof(float)))
      {
         log->error("invalid clock chunk size");
         return InvalidStorageFormat;
      }

      if (!stateCount)
      {
         storageSignalStream->next({});
         return NoError;
      }

      std::vector<uint32_t> offsets(stateCount);
      std::vector<float> values(stateCount);

      if (package.readData(offsets.data(), stateCount * sizeof(uint32_t)) != 0)
      {
         log->error("failed to read clock offsets");
         return ReadDataFailed;
      }

      if (package.readData(values.data(), stateCount * sizeof(float)) != 0)
      {
         log->error("failed to read clock values");
         return ReadDataFailed;
      }

      hw::SignalBuffer buffer(stateCount * 2, 2, 1, sampleRate, base, 0, hw::SignalType::SIGNAL_TYPE_CLK_SIGNAL, streamId);

      for (unsigned int n = 0; n < stateCount; n++)
      {
         if (offsets[n] < base)
         {
            log->error("clock offset before chunk start");
            return InvalidStorageFormat;
         }

         buffer.put(values[n]).put(static_cast<float>(offsets[n] - base));
      }

      buffer.flip();

      storageSignalStream->next(buffer);

      clockSignalQueue.add(buffer);

      // the clock entry is the last one written, so this closes the whole file and triggers the final view refresh
      storageSignalStream->next({});

      return NoError;
   }

   int writeClockEntry(rt::Package &package, const std::string &name, unsigned int id, double rangeStart, double rangeEnd)
   {
      unsigned int sampleRate = 0;

      std::vector<ClockState> states;

      for (const auto &buffer: clockSignalQueue)
      {
         if (buffer.id() != id)
            continue;

         if (!sampleRate)
            sampleRate = buffer.sampleRate();

         for (unsigned int i = 0; i < buffer.limit(); i += buffer.stride())
         {
            states.push_back({buffer.offset() + static_cast<unsigned long long>(buffer[i + 1]), buffer[i + 0]});
         }
      }

      const auto sampleStart = static_cast<unsigned long long>(sampleRate * rangeStart);
      const auto sampleEnd = static_cast<unsigned long long>(sampleRate * rangeEnd);

      const std::vector<ClockState> saved = clockStatesInRange(states, sampleStart, sampleEnd);

      std::vector<uint32_t> offsets;
      std::vector<float> values;

      offsets.reserve(saved.size());
      values.reserve(saved.size());

      // offsets are stored relative to the start of the saved range, the same base the frame, logic and radio
      // entries use, so every stream lands on one time base when the file is read back
      for (const ClockState &state: saved)
      {
         offsets.push_back(static_cast<uint32_t>(state.offset - sampleStart));
         values.push_back(state.frequency);
      }

      SampleHdr hdr {.magic = {'C', 'L', 'K', 'F'}, .version = 1, .info = {}};

      hdr.info[INFO_START_OFFSET] = 0;
      hdr.info[INFO_STREAM_ID] = id;
      hdr.info[INFO_SAMPLE_RATE] = sampleRate;
      hdr.info[INFO_TOTAL_SAMPLES] = static_cast<unsigned int>(offsets.size());

      const auto size = static_cast<unsigned int>(sizeof(hdr) + offsets.size() * sizeof(uint32_t) + values.size() * sizeof(float));

      log->info("add clock entry {} with size {}", {name, size});

      if (package.addEntry(name, size) != 0)
      {
         log->error("failed to add clock header");
         return WriteDataFailed;
      }

      if (package.writeData(&hdr, sizeof(hdr)) != 0)
      {
         log->error("failed to write clock header");
         return WriteDataFailed;
      }

      // write offset stream, then value stream (SoA layout)
      if (!offsets.empty() && package.writeData(offsets.data(), offsets.size() * sizeof(uint32_t)) != 0)
      {
         log->error("failed to write clock offsets");
         return WriteDataFailed;
      }

      if (!values.empty() && package.writeData(values.data(), values.size() * sizeof(float)) != 0)
      {
         log->error("failed to write clock values");
         return WriteDataFailed;
      }

      log->info("\t{} states stored for clock channel {}", {values.size(), id});

      return NoError;
   }

   int writeClockData(rt::Package &package, double rangeStart, double rangeEnd)
   {
      int result;
      std::vector<unsigned int> channels;

      for (const auto &buffer: clockSignalQueue)
      {
         if (std::find(channels.begin(), channels.end(), buffer.id()) == channels.end())
            channels.push_back(buffer.id());
      }

      log->info("detected {} clock channels", {channels.size()});

      for (auto id: channels)
      {
         std::string name = rt::Format::format("clock-{}.clkf", {id});

         if ((result = writeClockEntry(package, name, id, rangeStart, rangeEnd)) != NoError)
         {
            log->error("failed to write clock entry");
            return result;
         }
      }

      return NoError;
   }

   int writeRadioData(rt::Package &package, double timeStart, double timeEnd)
   {
      int result;
      std::vector<unsigned int> channels;

      for (const auto &buffer: radioSignalQueue)
      {
         if (std::find(channels.begin(), channels.end(), buffer.id()) == channels.end())
            channels.push_back(buffer.id());
      }

      log->info("detected {} radio channels", {channels.size()});

      for (auto id: channels)
      {
         std::string name = rt::Format::format("radio-{}.apcm", {id});

         if ((result = writeRadioEntry(package, name, id, timeStart, timeEnd)) != NoError)
         {
            log->error("failed to write radio signal entry");
            return result;
         }
      }

      return NoError;
   }

   void updateStorageStatus(int value, int progress = -1, const std::string &message = {})
   {
      json data;

      // data status
      switch (value)
      {
         case Reading:
            data["status"] = "reading";
            break;
         case Writing:
            data["status"] = "writing";
            break;
         case Progress:
            data["status"] = "progress";
            break;
         case Complete:
            data["status"] = "complete";
            break;
         case Error:
            data["status"] = "error";
            break;
      }

      data["progress"] = progress;

      if (!message.empty())
         data["message"] = message;

      updateStatus(value, data);
   }
};

TraceStorageTask::TraceStorageTask() : Worker("TraceStorage")
{
}

rt::Worker *TraceStorageTask::construct()
{
   return new Impl;
}

}

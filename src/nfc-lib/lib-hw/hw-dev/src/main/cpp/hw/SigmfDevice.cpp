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

#include <cstdio>
#include <ctime>
#include <chrono>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

#include <nlohmann/json.hpp>

#include <rt/Logger.h>
#include <rt/FileSystem.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>
#include <hw/SigmfDevice.h>

#define BUFFER_SIZE (1024)
#define SCALE (static_cast<float>(1 << 15))
#define SIGMF_DATATYPE "ci16_le"

using json = nlohmann::json;

namespace hw {

static std::string sigmfBaseName(const std::string &name)
{
   std::string path = name.find("sigmf://") == 0 ? name.substr(8) : name;

   if (path.size() >= 11 && path.compare(path.size() - 11, 11, ".sigmf-meta") == 0)
      return path.substr(0, path.size() - 11);

   if (path.size() >= 11 && path.compare(path.size() - 11, 11, ".sigmf-data") == 0)
      return path.substr(0, path.size() - 11);

   return path;
}

static std::string toIso8601(unsigned int epoch)
{
   const std::time_t time = epoch;
   const std::tm *tm = std::gmtime(&time);

   std::ostringstream oss;
   oss << std::put_time(tm, "%Y-%m-%dT%H:%M:%SZ");

   return oss.str();
}

static unsigned int fromIso8601(const std::string &value)
{
   std::tm tm {};
   std::istringstream iss(value);
   iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");

   if (iss.fail())
      return 0;

#if defined(_WIN32)
   return static_cast<unsigned int>(_mkgmtime(&tm));
#else
   return static_cast<unsigned int>(timegm(&tm));
#endif
}

struct SigmfDevice::Impl
{
   rt::Logger *log = rt::Logger::getLogger("hw.SigmfDevice");

   std::string name {};
   std::string dataFileName {};
   std::string metaFileName {};

   int openMode {};
   unsigned int sampleRate {};
   unsigned int sampleSize {16};
   unsigned int sampleType {SAMPLE_TYPE_INTEGER};
   unsigned int sampleCount {};
   unsigned int sampleOffset {};
   unsigned int channelCount {2};
   unsigned int streamTime {};
   std::vector<int> channelKeys {};

   std::fstream file;

   explicit Impl(std::string name) : name(std::move(name))
   {
      const std::string base = sigmfBaseName(this->name);

      dataFileName = base + ".sigmf-data";
      metaFileName = base + ".sigmf-meta";

      log->debug("created SigmfDevice for name [{}]", {this->name});
   }

   ~Impl()
   {
      close();

      log->debug("destroy SigmfDevice for name [{}]", {name});
   }

   bool open(Mode mode)
   {
      log->debug("open SigmfDevice for name [{}]", {name});

      close();

      openMode = mode;

      sampleCount = 0;
      sampleOffset = 0;

      switch (mode)
      {
         case Write:
         {
            rt::FileSystem::truncateFile(dataFileName);

            file.open(dataFileName, std::ios::out | std::ios::binary);

            if (file.is_open())
            {
               streamTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

               if (!writeMeta())
               {
                  file.close();
               }
            }

            return file.is_open();
         }

         case Read:
         {
            if (!readMeta())
               return false;

            file.open(dataFileName, std::ios::in | std::ios::binary);

            return file.is_open();
         }

         case Duplex:
         {
         }
      }

      return false;
   }

   void close()
   {
      if (file.is_open())
      {
         log->debug("close SigmfDevice for name [{}]", {name});

         file.close();
      }
   }

   bool isOpen() const
   {
      return file.is_open();
   }

   bool isEof() const
   {
      return file.eof();
   }

   bool isReady() const
   {
      return file.good();
   }

   // .sigmf-meta is fully known at open(Write) time (sample rate + start time), no
   // sizes to patch on close unlike a WAV RIFF header - written once, here.
   bool writeMeta()
   {
      json meta;

      meta["global"] = {
         {"core:datatype", SIGMF_DATATYPE},
         {"core:sample_rate", sampleRate},
         {"core:version", "1.0.0"},
         {"core:recorder", "nfc-laboratory"}
      };

      meta["captures"] = json::array({
         {{"core:sample_start", 0}, {"core:datetime", toIso8601(streamTime)}}
      });

      meta["annotations"] = json::array();

      std::ofstream out(metaFileName, std::ios::out | std::ios::trunc);

      if (!out.is_open())
      {
         log->error("failed to create meta file {}", {metaFileName});
         return false;
      }

      out << meta.dump(4);

      return out.good();
   }

   bool readMeta()
   {
      std::ifstream in(metaFileName);

      if (!in.is_open())
      {
         log->error("failed to open meta file {}", {metaFileName});
         return false;
      }

      json meta = json::parse(in, nullptr, false);

      if (meta.is_discarded() || !meta.contains("global"))
      {
         log->error("invalid meta file {}", {metaFileName});
         return false;
      }

      const json &global = meta["global"];

      if (!global.contains("core:datatype") || global["core:datatype"] != SIGMF_DATATYPE)
      {
         log->error("unsupported sigmf datatype in {}", {metaFileName});
         return false;
      }

      if (!global.contains("core:sample_rate"))
      {
         log->error("missing sample rate in {}", {metaFileName});
         return false;
      }

      sampleRate = global["core:sample_rate"];

      if (meta.contains("captures") && !meta["captures"].empty() && meta["captures"][0].contains("core:datetime"))
         streamTime = fromIso8601(meta["captures"][0]["core:datetime"]);

      struct stat st {};

      if (stat(dataFileName.c_str(), &st) == 0)
         sampleCount = static_cast<unsigned int>(st.st_size / (2 * sizeof(int16_t)));

      return true;
   }

   long read(SignalBuffer &buffer)
   {
      if (!file.is_open())
         return -1;

      int16_t block[BUFFER_SIZE];
      float vector[BUFFER_SIZE];

      while (buffer.remaining() && file)
      {
         file.read(reinterpret_cast<char *>(block), (buffer.remaining() < BUFFER_SIZE ? buffer.remaining() : BUFFER_SIZE) * sizeof(int16_t));

         const int samples = static_cast<int>(file.gcount() / sizeof(int16_t));

         for (int i = 0; i < samples; i++)
            vector[i] = static_cast<float>(block[i]) / SCALE;

         buffer.put(vector, samples);
      }

      buffer.flip();

      sampleOffset += buffer.elements();

      return static_cast<long>(buffer.limit());
   }

   long write(const SignalBuffer &buffer)
   {
      if (!file.is_open())
         return -1;

      int16_t block[BUFFER_SIZE];
      int converted = 0;

      buffer.stream([this, &converted, &block](const float *value, int stride) {

         for (int c = 0; c < stride; c++)
         {
            block[converted++] = static_cast<int16_t>(value[c] * SCALE);

            if (converted == BUFFER_SIZE)
            {
               file.write(reinterpret_cast<const char *>(block), sizeof(block));
               converted = 0;
            }
         }
      });

      if (converted)
      {
         file.write(reinterpret_cast<const char *>(block), converted * sizeof(int16_t));
      }

      sampleCount += buffer.elements();
      sampleOffset += buffer.elements();

      return static_cast<long>(buffer.position());
   }
};

SigmfDevice::SigmfDevice(const std::string &name) : impl(std::make_shared<Impl>(name))
{
}

bool SigmfDevice::open(Device::Mode mode)
{
   return impl->open(mode);
}

void SigmfDevice::close()
{
   impl->close();
}

rt::Variant SigmfDevice::get(int id, int channel) const
{
   switch (id)
   {
      case PARAM_DEVICE_NAME:
         return impl->name;

      case PARAM_DEVICE_SERIAL:
         return std::string {};

      case PARAM_DEVICE_VERSION:
         return std::string {};

      case PARAM_SAMPLE_RATE:
         return impl->sampleRate;

      case PARAM_SAMPLE_SIZE:
         return impl->sampleSize;

      case PARAM_SAMPLE_TYPE:
         return impl->sampleType;

      case PARAM_SAMPLE_OFFSET:
         return impl->sampleOffset;

      case PARAM_STREAM_TIME:
         return impl->streamTime;

      case PARAM_SAMPLES_READ:
         return impl->sampleCount;

      // always reports 2 (I and Q), matching the same "channel count" convention
      // SignalStorageTask::readRadio() already uses to detect an IQ file - unrelated
      // to the SigMF spec's own (and deliberately unused here) core:num_channels field
      case PARAM_CHANNEL_COUNT:
         return impl->channelCount;

      case PARAM_CHANNEL_KEYS:
         return impl->channelKeys;

      default:
         return {};
   }
}

bool SigmfDevice::set(int id, const rt::Variant &value, int channel)
{
   switch (id)
   {
      case PARAM_SAMPLE_RATE:
      {
         if (auto v = std::get_if<unsigned int>(&value))
         {
            impl->sampleRate = *v;
            return true;
         }

         impl->log->error("invalid value type for PARAM_SAMPLE_RATE");
         return false;
      }
      case PARAM_SAMPLE_SIZE:
      {
         // this device only ever writes/reads ci16_le, silently accepted for
         // interface symmetry with SignalStorageTask::open(), which always sets it
         return true;
      }
      case PARAM_SAMPLE_TYPE:
      {
         if (auto v = std::get_if<unsigned int>(&value))
         {
            impl->sampleType = *v;
            return true;
         }

         impl->log->error("invalid value type for PARAM_SAMPLE_TYPE");
         return false;
      }
      case PARAM_CHANNEL_COUNT:
      {
         // fixed at 2 (I/Q), see get() above; accepted but ignored for symmetry
         return true;
      }
      case PARAM_STREAM_TIME:
      {
         if (auto v = std::get_if<unsigned int>(&value))
         {
            impl->streamTime = *v;
            return true;
         }

         impl->log->error("invalid value type for PARAM_STREAM_TIME");
         return false;
      }
      case PARAM_CHANNEL_KEYS:
      {
         if (auto v = std::get_if<std::vector<int>>(&value))
         {
            impl->channelKeys = *v;
            return true;
         }

         impl->log->error("invalid value type for PARAM_CHANNEL_KEYS");
         return false;
      }
      default:
         impl->log->warn("unknown or unsupported configuration id {}", {id});
         return false;
   }
}

bool SigmfDevice::isOpen() const
{
   return impl->isOpen();
}

bool SigmfDevice::isEof() const
{
   return impl->isEof();
}

bool SigmfDevice::isReady() const
{
   return impl->isReady();
}

long SigmfDevice::read(SignalBuffer &buffer)
{
   return impl->read(buffer);
}

long SigmfDevice::write(const SignalBuffer &buffer)
{
   return impl->write(buffer);
}

}

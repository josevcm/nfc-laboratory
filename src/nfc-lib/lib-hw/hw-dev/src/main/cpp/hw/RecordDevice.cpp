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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <queue>
#include <fstream>
#include <iostream>
#include <cstring>
#include <utility>

#include <rt/Logger.h>
#include <rt/FileSystem.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>
#include <hw/RecordDevice.h>

#define BUFFER_SIZE (1024)
#define AUDIO_FORMAT_PCM (1)

#define RIFF_CHUNK_ID 0x46464952 // "RIFF"
#define FMT_CHUNK_ID 0x20746D66 // "fmt "
#define META_CHUNK_ID 0x4154454D // "META"
#define DATA_CHUNK_ID 0x61746164 // "data"
#define WAVE_TYPE_ID 0x45564157 // "WAVE"
#define META_INFO_ID 0x6174656D // "meta"

#define CHUNK_STRING(v) static_cast<char>(v & 0xFF), static_cast<char>(v >> 8 & 0xFF), static_cast<char>(v >> 16 & 0xFF), static_cast<char>(v >> 24 & 0xFF)

namespace hw {

struct FILEChunk
{
   unsigned int id;
   unsigned int size;
};

struct METAInfo
{
   unsigned int id;
   unsigned int epoch;
   unsigned int keys[8];
};

struct RIFFChunk
{
   FILEChunk chunk; // 8 bytes
   unsigned int type; // 4 bytes
};

struct WAVEChunk
{
   FILEChunk chunk; // 8 bytes
   unsigned short audioFormat; // 2 bytes
   unsigned short numChannels; // 2 bytes
   unsigned int sampleRate; // 4 bytes
   unsigned int byteRate; // 4 bytes
   unsigned short blockAlign; // 2 bytes
   unsigned short bitsPerSample; // 2 bytes
};

struct LISTChunk
{
   FILEChunk chunk;
   METAInfo meta;
};

struct DATAChunk
{
   FILEChunk chunk;
};

struct FILEHeader
{
   RIFFChunk riff {}; // 12 bytes
   WAVEChunk wave {}; // 24 bytes
   LISTChunk list {}; // 8 bytes
   DATAChunk data {}; // 8 bytes
};

struct RecordDevice::Impl
{
   rt::Logger *log = rt::Logger::getLogger("hw.RecordDevice");

   std::string name {};
   std::string serial {};
   std::string version {};
   int fileDesc {};
   int frequency {};
   int openMode {};
   unsigned int sampleRate {};
   unsigned int sampleSize {};
   unsigned int sampleType {};
   unsigned int sampleCount {};
   unsigned int sampleOffset {};
   unsigned int channelCount {};
   unsigned int streamTime {};
   std::vector<int> channelKeys {};

   std::fstream file;

   explicit Impl(std::string name) : name(std::move(name)), sampleSize(16), sampleRate(44100), sampleType(1), channelCount(1)
   {
      log->debug("created RecordDevice for name [{}]", {this->name});
   }

   ~Impl()
   {
      close();

      log->debug("destroy RecordDevice for name {}]", {name});
   }

   bool open(Mode mode)
   {
      log->debug("open RecordDevice for name [{}]", {name});

      if (name.find("://") != -1 && name.find("record://") == -1)
      {
         log->warn("invalid device name [{}]", {name});
         return false;
      }

      std::string path = name.find("record://") == 0 ? name.substr(9) : name;

      close();

      openMode = mode;

      // initialize
      sampleCount = 0;
      sampleOffset = 0;

      switch (mode)
      {
         case Write:
         {
            // create full file path and then truncate, if exists
            rt::FileSystem::truncateFile(path);

            // open for writing
            file.open(path, std::ios::out | std::ios::binary);

            if (file.is_open())
            {
               streamTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

               if (!writeHeader())
               {
                  file.close();
               }
            }

            log->debug("open successfully, current file offset: {}", {static_cast<unsigned long long>(file.tellp())});

            return file.is_open();
         }

         case Read:
         {
            file.open(path, std::ios::in | std::ios::binary);

            if (file.is_open())
            {
               if (!readHeader())
               {
                  file.close();
               }
            }

            log->debug("open successfully, current file offset: {}", {static_cast<unsigned long long>(file.tellp())});

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
         log->debug("close RecordDevice for name [{}]", {name});

         if (openMode == Write)
         {
            writeHeader();
         }

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

   bool isStreaming() const
   {
      return file.is_open();
   }

   int read(SignalBuffer &buffer)
   {
      if (!file.is_open())
         return -1;

      log->debug("reading {} bytes from offset {}", {buffer.size(), static_cast<unsigned long long>(file.tellp())});

      switch (sampleSize)
      {
         case 8:
            return readScaledSamples<unsigned char>(buffer, static_cast<float>(255));

         case 16:
            return readScaledSamples<short>(buffer, static_cast<float>(1 << 15));

         case 32:
            return readScaledSamples<int>(buffer, static_cast<float>(1 << 31));

         default:
            return -1;
      }
   }

   int write(SignalBuffer &buffer)
   {
      if (!file.is_open())
         return -1;

      log->debug("writing {} bytes to offset {}", {buffer.size(), static_cast<unsigned long long>(file.tellp())});

      switch (sampleSize)
      {
         case 8:
            return writeScaledSamples<unsigned char>(buffer, static_cast<float>(255));

         case 16:
            return writeScaledSamples<short>(buffer, static_cast<float>(1 << 15));

         case 32:
            return writeScaledSamples<int>(buffer, static_cast<float>(1 << 31));

         default:
            return -1;
      }
   }

   template <typename T>
   int readScaledSamples(SignalBuffer &buffer, float scale)
   {
      T block[BUFFER_SIZE];

      float vector[BUFFER_SIZE];

      while (buffer.available() && file)
      {
         // read one block of samples up to BUFFER_SIZE
         file.read(reinterpret_cast<char *>(block), (buffer.available() < BUFFER_SIZE ? buffer.available() : BUFFER_SIZE) * sizeof(T));

         // number of samples readed
         const int samples = file.gcount() / sizeof(T);

         // convert readed samples to float
         for (int i = 0; i < samples; i++)
         {
            vector[i] = fromLittleEndian<T>(block[i]) / scale;
         }

         // and store in buffer
         buffer.put(vector, samples);
      }

      buffer.flip();

      sampleOffset += buffer.limit();

      return static_cast<int>(buffer.limit());
   }

   template <typename T>
   int writeScaledSamples(SignalBuffer &buffer, float scale)
   {
      T block[BUFFER_SIZE];

      // number of samples converted
      int converted = 0;

      // process all buffer contents
      buffer.stream([this, &scale, &converted, &block](const float *value, int stride) {

         for (int c = 0; c < stride; c++)
         {
            // convert float sample to WAV sample
            block[converted++] = toLittleEndian<T>(static_cast<T>(value[c] * scale));

            // write buffered block
            if (converted == BUFFER_SIZE)
            {
               file.write(reinterpret_cast<const char *>(block), sizeof(block));
               converted = 0;
            }
         }
      });

      // write last remaining block
      if (converted)
      {
         file.write(reinterpret_cast<const char *>(block), converted * sizeof(T));
      }

      sampleCount += buffer.position();
      sampleOffset += buffer.position();

      return static_cast<int>(buffer.position());
   }

   bool readHeader()
   {
      log->debug("read RecordDevice header for name [{}]", {name});

      struct stat st {};

      file.seekg(0);

      RIFFChunk riff {};

      if (!file.read(reinterpret_cast<char *>(&riff), sizeof(riff)))
         return false;

      // trace RIFF chunk
      traceRiffChunk(riff);

      if (riff.chunk.id != RIFF_CHUNK_ID)
      {
         log->error("invalid RIFF chunk id");
         return false;
      }

      if (riff.type != WAVE_TYPE_ID)
      {
         log->error("invalid WAVE type id");
         return false;
      }

      FILEChunk entry {};

      while (file.read(reinterpret_cast<char *>(&entry), sizeof(FILEChunk)))
      {
         switch (entry.id)
         {
            // read FMT chunk with WAVE info
            case FMT_CHUNK_ID:
            {
               WAVEChunk wave {};

               if (entry.size != sizeof(wave) - sizeof(FILEChunk))
                  return false;

               // restore offset to read wave chunk
               file.seekg(-static_cast<int>(sizeof(FILEChunk)), std::ios_base::cur);

               // read wave chunk
               if (!file.read(reinterpret_cast<char *>(&wave), sizeof(wave)))
                  return false;

               // trace wave info
               traceWaveChunk(wave);

               if (wave.audioFormat != 1)
                  return false;

               // Establish format
               sampleType = SAMPLE_TYPE_FLOAT;
               sampleRate = fromLittleEndian<unsigned int>(wave.sampleRate);
               sampleSize = fromLittleEndian<unsigned short>(wave.bitsPerSample);
               channelCount = fromLittleEndian<unsigned short>(wave.numChannels);

               continue;
            }

            // read META chunk with META info
            case META_CHUNK_ID:
            {
               unsigned int type;

               if (entry.size != sizeof(METAInfo))
                  break;

               // read first LIST entry ID
               if (!file.read(reinterpret_cast<char *>(&type), sizeof(type)))
                  return false;

               // recover previous file offset
               file.seekg(-static_cast<int>(sizeof(type)), std::ios_base::cur);

               // check if meta information is present
               if (type != META_INFO_ID)
                  break;

               LISTChunk list {.chunk = entry};

               // read META info
               if (!file.read(reinterpret_cast<char *>(&list.meta), sizeof(list.meta)))
                  return false;

               // trace meta info
               traceListChunk(list);

               // for security, skip remain list chuck (should be 0)
               if (!file.seekg(entry.size - sizeof(list.meta), std::ios_base::cur))
                  return false;

               streamTime = fromLittleEndian<unsigned int>(list.meta.epoch);

               channelKeys.clear();

               for (unsigned int key: list.meta.keys)
                  channelKeys.push_back(static_cast<int>(key));

               continue;
            }

            // read DATA chunk with samples
            case DATA_CHUNK_ID:
            {
               DATAChunk data {.chunk = entry};

               // initialize values
               sampleCount = entry.size / (channelCount * sampleSize / 8);
               sampleOffset = 0;

               if (streamTime == 0)
               {
                  log->info("the file does not have a timestamp stored, it will default to the creation date");

                  // read default stream time from file creation date
                  if (stat(name.c_str(), &st) == 0)
                     streamTime = st.st_ctime;
               }

               traceDataChunk(data);

               return true;
            }

            default:
            {
               log->warn("unknown chunk id {}", {CHUNK_STRING(entry.id)});
            }
         }

         // if reach this point, skip chuck
         if (!file.seekg(entry.size, std::ios_base::cur))
            return false;
      }

      return false;
   }

   bool writeHeader()
   {
      log->debug("write RecordDevice header for name [{}]", {name});

      // get current file offset written
      const int length = file.tellp();

      FILEHeader header {};

      // initialize RIFF WAVE format
      header.riff.chunk.id = RIFF_CHUNK_ID;
      header.riff.chunk.size = toLittleEndian<unsigned int>((length < sizeof(FILEHeader) ? sizeof(FILEHeader) : length) - 8);
      header.riff.type = WAVE_TYPE_ID;

      // initialize FMT chunk
      header.wave.chunk.id = FMT_CHUNK_ID;
      header.wave.chunk.size = toLittleEndian<unsigned int>(sizeof(header.wave) - sizeof(header.wave.chunk));
      header.wave.audioFormat = toLittleEndian<unsigned short>(AUDIO_FORMAT_PCM);
      header.wave.numChannels = toLittleEndian<unsigned short>(channelCount);
      header.wave.sampleRate = toLittleEndian<unsigned int>(sampleRate);
      header.wave.byteRate = toLittleEndian<unsigned int>(channelCount * sampleRate * sampleSize / 8);
      header.wave.blockAlign = toLittleEndian<unsigned short>(channelCount * sampleSize / 8);
      header.wave.bitsPerSample = toLittleEndian<unsigned short>(sampleSize);

      // initialize LIST chunk
      header.list.chunk.id = META_CHUNK_ID;
      header.list.chunk.size = toLittleEndian<unsigned int>(sizeof(LISTChunk) - sizeof(FILEChunk));

      // initialize META info
      header.list.meta.id = META_INFO_ID;
      header.list.meta.epoch = toLittleEndian<unsigned int>(streamTime);

      // write channels ids
      for (int i = 0; i < channelCount && i < channelKeys.size(); i++)
         header.list.meta.keys[i] = toLittleEndian<int>(channelKeys[i]);

      // update data format chunk
      header.data.chunk.id = DATA_CHUNK_ID;
      header.data.chunk.size = toLittleEndian<unsigned int>(length > sizeof(FILEHeader) ? length - sizeof(FILEHeader) : 0);

      // jump to file start
      file.seekp(0);

      // write file header
      file.write(reinterpret_cast<char *>(&header), sizeof(header));

      // write logging info
      traceRiffChunk(header.riff);
      traceWaveChunk(header.wave);
      traceListChunk(header.list);
      traceDataChunk(header.data);

      return file.good();
   }

   void traceRiffChunk(const RIFFChunk &riff) const
   {
      log->debug("riff.chunk.id.....: {}{}{}{} ", {CHUNK_STRING(riff.chunk.id)});
      log->debug("riff.chunk.size...: {}", {riff.chunk.size});
      log->debug("riff.type.........: {}{}{}{} ", {CHUNK_STRING(riff.type)});
   }

   void traceWaveChunk(const WAVEChunk &wave) const
   {
      log->debug("wave.chunk.id.....: {}{}{}{} ", {CHUNK_STRING(wave.chunk.id)});
      log->debug("wave.chunk.size...: {}", {wave.chunk.size});
      log->debug("wave.audioFormat..: {}", {wave.audioFormat});
      log->debug("wave.numChannels..: {}", {wave.numChannels});
      log->debug("wave.sampleRate...: {}", {wave.sampleRate});
      log->debug("wave.byteRate.....: {}", {wave.byteRate});
      log->debug("wave.blockAlign...: {}", {wave.blockAlign});
      log->debug("wave.bitsPerSample: {}", {wave.bitsPerSample});
   }

   void traceListChunk(const LISTChunk &list) const
   {
      std::vector<int> keys;

      for (unsigned int key: list.meta.keys)
      {
         keys.push_back(static_cast<int>(key));
      }

      log->debug("list.chunk.id.....: {}{}{}{}", {CHUNK_STRING(list.chunk.id)});
      log->debug("list.chunk.size...: {}", {list.chunk.size});
      log->debug("list.meta.id......: {}{}{}{}", {CHUNK_STRING(list.meta.id)});
      log->debug("list.meta.epoch...: {}", {list.meta.epoch});
      log->debug("list.meta.keys....: {}", {keys});
   }

   void traceDataChunk(const DATAChunk &data) const
   {
      log->debug("data.chunk.id.....: {}{}{}{} ", {CHUNK_STRING(data.chunk.id)});
      log->debug("data.chunk.size...: {}", {data.chunk.size});
   }

   template <typename T>
   T toLittleEndian(T value)
   {
      return value;
   }

   template <typename T>
   T fromLittleEndian(T value)
   {
      return value;
   }
};

RecordDevice::RecordDevice(const std::string &name) : impl(std::make_shared<Impl>(name))
{
}

bool RecordDevice::open(Device::Mode mode)
{
   return impl->open(mode);
}

void RecordDevice::close()
{
   impl->close();
}

rt::Variant RecordDevice::get(int id, int channel) const
{
   switch (id)
   {
      case PARAM_DEVICE_NAME:
         return impl->name;

      case PARAM_DEVICE_SERIAL:
         return impl->serial;

      case PARAM_DEVICE_VERSION:
         return impl->version;

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

      case PARAM_CHANNEL_COUNT:
         return impl->channelCount;

      case PARAM_CHANNEL_KEYS:
         return impl->channelKeys;

      default:
         return {};
   }
}

bool RecordDevice::set(int id, const rt::Variant &value, int channel)
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
         if (auto v = std::get_if<unsigned int>(&value))
         {
            impl->sampleSize = *v;
            return true;
         }

         impl->log->error("invalid value type for PARAM_SAMPLE_SIZE");
         return false;
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
         if (auto v = std::get_if<unsigned int>(&value))
         {
            impl->channelCount = *v;
            return true;
         }

         impl->log->error("invalid value type for PARAM_CHANNEL_COUNT");
         return false;
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

bool RecordDevice::isOpen() const
{
   return impl->isOpen();
}

bool RecordDevice::isEof() const
{
   return impl->isEof();
}

bool RecordDevice::isReady() const
{
   return impl->isReady();
}

bool RecordDevice::isStreaming() const
{
   return impl->isStreaming();
}

int RecordDevice::read(SignalBuffer &buffer)
{
   return impl->read(buffer);
}

int RecordDevice::write(SignalBuffer &buffer)
{
   return impl->write(buffer);
}

}

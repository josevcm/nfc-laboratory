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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <queue>
#include <fstream>
#include <iostream>
#include <cstring>
#include <utility>

#include <rt/Logger.h>

#include <sdr/SignalBuffer.h>
#include <sdr/RecordDevice.h>

#define BUFFER_SIZE (1024)

namespace sdr {

struct chunk
{
   char id[4];
   unsigned int size;
};

struct DATEInfo
{
   char id[4];
   unsigned int epoch;
};

struct RIFFChunk
{
   chunk desc; // 8 bytes
   char type[4]; // 4 bytes
};

struct WAVEChunk
{
   chunk desc; // 8 bytes
   unsigned short audioFormat; // 2 bytes
   unsigned short numChannels; // 2 bytes
   unsigned int sampleRate; // 4 bytes
   unsigned int byteRate; // 4 bytes
   unsigned short blockAlign; // 2 bytes
   unsigned short bitsPerSample; // 2 bytes
};

struct LISTChunk
{
   chunk desc;
   DATEInfo time;
};

struct DATAChunk
{
   chunk desc;
};

struct FILEHeader
{
   RIFFChunk riff; // 12 bytes
   WAVEChunk wave; // 24 bytes
   LISTChunk list; // 8 bytes
   DATAChunk data; // 8 bytes
};

struct RecordDevice::Impl
{
   rt::Logger log {"RecordDevice"};

   std::string name {};
   std::string version {};
   int fileDesc {};
   int frequency {};
   int openMode {};
   int sampleRate {};
   int sampleSize {};
   int sampleType {};
   int sampleCount {};
   int sampleOffset {};
   int channelCount {};
   int streamTime {};

   std::fstream file;

   explicit Impl(std::string name) : name(std::move(name)), sampleSize(16), sampleRate(44100), sampleType(1), channelCount(1)
   {
      log.debug("created RecordDevice for name [{}]", {this->name});
   }

   ~Impl()
   {
      close();

      log.debug("destroy RecordDevice for name {}]", {name});
   }

   bool open(SignalDevice::OpenMode mode)
   {
      log.debug("open RecordDevice for name [{}]", {name});

      if (name.find("://") != -1 && name.find("record://") == -1)
      {
         log.warn("invalid device name [{}]", {name});
         return false;
      }

      std::string id = name.find("record://") == 0 ? name.substr(9) : name;

      close();

      openMode = mode;

      // initialize
      sampleCount = 0;
      sampleOffset = 0;

      switch (mode)
      {
         case SignalDevice::Write:
         {
            file.open(id, std::ios::out | std::ios::binary | std::ios::trunc);

            if (file.is_open())
            {
               streamTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

               if (!writeHeader())
               {
                  file.close();
               }
            }

            return file.is_open();
         }

         case SignalDevice::Read:
         {
            file.open(id, std::ios::in | std::ios::binary);

            if (file.is_open())
            {
               if (!readHeader())
               {
                  file.close();
               }
            }

            return file.is_open();
         }

         case SignalDevice::Duplex:
         {
         }
      }

      return false;
   }

   void close()
   {
      if (file.is_open())
      {
         log.debug("close RecordDevice for name [{}]", {name});

         if (openMode == SignalDevice::Write)
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
      return false;
   }

   int read(SignalBuffer &buffer)
   {
      switch (sampleSize)
      {
         case 8:
            return readSamples<char>(buffer);

         case 16:
            return readSamples<short>(buffer);

         case 32:
            return readSamples<int>(buffer);
      }

      buffer.flip();

      return buffer.limit();
   }

   int write(SignalBuffer &buffer)
   {
      switch (sampleSize)
      {
         case 8:
            return writeSamples<char>(buffer);

         case 16:
            return writeSamples<short>(buffer);

         case 32:
            return writeSamples<int>(buffer);
      }

      return buffer.position();
   }

   template<typename T>
   int readSamples(SignalBuffer &buffer)
   {
      T block[BUFFER_SIZE];

      // converted samples
      float vector[BUFFER_SIZE];

      // sample scale from float
      float scale = 1 << (8 * sizeof(T) - 1);

      while (buffer.available() && file)
      {
         // read one block of samples up to BUFFER_SIZE
         file.read(reinterpret_cast<char *>(block), (buffer.available() < BUFFER_SIZE ? buffer.available() : BUFFER_SIZE) * sizeof(T));

         // number of samples readed
         int samples = file.gcount() / sizeof(T);

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

      return buffer.limit();
   }

   template<typename T>
   int writeSamples(SignalBuffer &buffer)
   {
      T block[BUFFER_SIZE];

      // sample scale to float
      float scale = 1 << (8 * sizeof(T) - 1);

      // number of samples converted
      int converted = 0;

      // process all buffer contents
      buffer.stream([this, &scale, &converted, &block](const float *value, int stride) {

         for (int c = 0; c < stride; c++)
         {
            // convert float sample to WAV sample
            block[converted++] = toLittleEndian<T>((T) (value[c] * scale));

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

      return buffer.position();
   }

   bool readHeader()
   {
      log.debug("read RecordDevice header for name [{}]", {name});

      file.seekg(0);

      RIFFChunk riff;

      if (!file.read(reinterpret_cast<char *>(&riff), sizeof(riff)))
         return false;

      if (std::memcmp(&riff.desc.id, "RIFF", 4) != 0)
         return false;

      if (std::memcmp(&riff.type, "WAVE", 4) != 0)
         return false;

      chunk entry;

      while (file.read(reinterpret_cast<char *>(&entry), sizeof(chunk)))
      {
         // process FMT chuck
         if (std::memcmp(&entry.id, "fmt ", 4) == 0)
         {
            WAVEChunk wave;

            if (entry.size != sizeof(wave) - 8)
               return false;

            file.seekg(-(int) sizeof(chunk), std::ios_base::cur);

            if (!file.read(reinterpret_cast<char *>(&wave), sizeof(wave)))
               return false;

            if (wave.audioFormat != 1)
               return false;

            // Establish format
            sampleType = SignalDevice::Integer;
            sampleRate = fromLittleEndian<unsigned int>(wave.sampleRate);
            sampleSize = fromLittleEndian<unsigned short>(wave.bitsPerSample);
            channelCount = fromLittleEndian<unsigned short>(wave.numChannels);
         }
            // process LIST chuck
         else if (std::memcmp(&entry.id, "LIST", 4) == 0)
         {
            if (entry.size >= 8)
            {
               char type[4];

               if (!file.read(reinterpret_cast<char *>(type), sizeof(type)))
                  return false;

               // recover previous file offset
               file.seekg(-(int) sizeof(type), std::ios_base::cur);

               // check if date information is present
               if (std::memcmp(&type, "date", 4) == 0)
               {
                  DATEInfo date;

                  if (!file.read(reinterpret_cast<char *>(&date), sizeof(date)))
                     return false;

                  if (!file.seekg(entry.size - sizeof(date), std::ios_base::cur))
                     return false;

                  streamTime = fromLittleEndian<unsigned int>(date.epoch);

                  continue;
               }
            }

            // if not date found, skip remain list chuck
            if (!file.seekg(entry.size, std::ios_base::cur))
               return false;
         }
            // process DATA chuck
         else if (std::memcmp(&entry.id, "data", 4) == 0)
         {
            // initialize values
            sampleCount = entry.size / (channelCount * sampleSize / 8);
            sampleOffset = 0;

            return true;
         }

            // unknown chuck
         else
         {
            break;
         }
      }

      return false;
   }

   bool writeHeader()
   {
      FILEHeader header = {
            {{{'R', 'I', 'F', 'F'}, 0}, {'W',                  'A', 'V', 'E'}},
            {{{'f', 'm', 't', ' '}, 0}, 0, 0, 0, 0, 0, 0},
            {{{'L', 'I', 'S', 'T'}, 0}, {{'d', 'a', 't', 'e'}, 0}},
            {{{'d', 'a', 't', 'a'}, 0}}
      };

      log.debug("write RecordDevice header for name [{}]", {name});

      // get current file offset written
      int length = file.tellp();

      // calculate overall file size
      header.riff.desc.size = toLittleEndian<unsigned int>(length > sizeof(RIFFChunk) ? length - sizeof(RIFFChunk) + 4 : 0);

      // update wave format chunk
      header.wave.desc.size = toLittleEndian<unsigned int>(16);
      header.wave.audioFormat = toLittleEndian<unsigned short>(1);
      header.wave.numChannels = toLittleEndian<unsigned short>(channelCount);
      header.wave.sampleRate = toLittleEndian<unsigned int>(sampleRate);
      header.wave.byteRate = toLittleEndian<unsigned int>(channelCount * sampleRate * sampleSize / 8);
      header.wave.blockAlign = toLittleEndian<unsigned short>(channelCount * sampleSize / 8);
      header.wave.bitsPerSample = toLittleEndian<unsigned short>(sampleSize);

      // update list info chunk
      header.list.desc.size = toLittleEndian<unsigned int>(sizeof(DATEInfo));
      header.list.time.epoch = toLittleEndian<unsigned int>(streamTime);

      // update data format chunk
      header.data.desc.size = toLittleEndian<unsigned int>(length > sizeof(FILEHeader) ? length - sizeof(FILEHeader) : 0);

      // jump to file start
      file.seekp(0);

      // write file header
      file.write(reinterpret_cast<char *>(&header), sizeof(header));

      return file.good();
   }

   template<typename T>
   inline T toLittleEndian(T value)
   {
      return value;
   }

   template<typename T>
   inline T fromLittleEndian(T value)
   {
      return value;
   }
};

RecordDevice::RecordDevice(const std::string &name) : impl(std::make_shared<Impl>(name))
{
}

const std::string &RecordDevice::name()
{
   return impl->name;
}

const std::string &RecordDevice::version()
{
   return impl->version;
}

bool RecordDevice::open(OpenMode mode)
{
   return impl->open(mode);
}

void RecordDevice::close()
{
   impl->close();
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

int RecordDevice::sampleCount() const
{
   return impl->sampleCount;
}

int RecordDevice::sampleOffset() const
{
   return impl->sampleOffset;
}

int RecordDevice::sampleSize() const
{
   return impl->sampleSize;
}

int RecordDevice::setSampleSize(int value)
{
   impl->sampleSize = value;

   return 0;
}

long RecordDevice::sampleRate() const
{
   return impl->sampleRate;
}

int RecordDevice::setSampleRate(long value)
{
   impl->sampleRate = value;

   return 0;
}

int RecordDevice::sampleType() const
{
   return impl->sampleType;
}

int RecordDevice::setSampleType(int value)
{
   impl->sampleType = value;

   return 0;
}

int RecordDevice::channelCount() const
{
   return impl->channelCount;
}

void RecordDevice::setChannelCount(int value)
{
   impl->channelCount = value;
}

long RecordDevice::streamTime() const
{
   return impl->streamTime;
}

int RecordDevice::setStreamTime(long value)
{
   impl->streamTime = value;

   return 0;
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

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

struct RIFFHeader
{
   chunk desc;
   char type[4];
};

struct WAVEHeader
{
   chunk desc;
   unsigned short audioFormat;
   unsigned short numChannels;
   unsigned int sampleRate;
   unsigned int byteRate;
   unsigned short blockAlign;
   unsigned short bitsPerSample;
};

struct DATAHeader
{
   chunk desc;
};

struct FILEHeader
{
   RIFFHeader riff;
   WAVEHeader wave;
   DATAHeader data;
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
      FILEHeader header {};

      log.debug("read RecordDevice header for name [{}]", {name});

      file.seekg(0);

      if (!file.read(reinterpret_cast<char *>(&header), sizeof(FILEHeader)))
         return false;

      if (std::memcmp(&header.riff.desc.id, "RIFF", 4) != 0)
         return false;

      if (std::memcmp(&header.riff.type, "WAVE", 4) != 0)
         return false;

      if (std::memcmp(&header.wave.desc.id, "fmt ", 4) != 0)
         return false;

      if (header.wave.audioFormat != 1)
         return false;

      // Establish format
      sampleType = SignalDevice::Integer;
      sampleRate = fromLittleEndian<unsigned int>(header.wave.sampleRate);
      sampleSize = fromLittleEndian<unsigned short>(header.wave.bitsPerSample);
      channelCount = fromLittleEndian<unsigned short>(header.wave.numChannels);

      // initialize values
      sampleCount = header.data.desc.size / (channelCount * sampleSize / 8);
      sampleOffset = 0;

      // read stream time
//      file.
      
      return true;
   }

   bool writeHeader()
   {
      FILEHeader header = {
            {{{'R', 'I', 'F', 'F'}, 0},  {'W', 'A', 'V', 'E'}}, // RIFFHeader
            {{{'f', 'm', 't', ' '}, 16}, 0, 0, 0, 0, 0, 0}, // WAVEHeader
            {{{'d', 'a', 't', 'a'}, 0}} // DATAHeader
      };

      log.debug("write RecordDevice header for name [{}]", {name});

      // get current file offset written
      int length = file.tellp();

      // update header file
      header.wave.audioFormat = toLittleEndian<unsigned short>(1);
      header.wave.numChannels = toLittleEndian<unsigned short>(channelCount);
      header.wave.sampleRate = toLittleEndian<unsigned int>(sampleRate);
      header.wave.byteRate = toLittleEndian<unsigned int>(channelCount * sampleRate * sampleSize / 8);
      header.wave.blockAlign = toLittleEndian<unsigned short>(channelCount * sampleSize / 8);
      header.wave.bitsPerSample = toLittleEndian<unsigned short>(sampleSize);

      header.riff.desc.size = toLittleEndian<unsigned int>(length > 40 ? length - sizeof(RIFFHeader) + 4 : 40);
      header.data.desc.size = toLittleEndian<unsigned int>(length > 36 ? length - sizeof(FILEHeader) : 0);

      file.seekp(0);
      file.write(reinterpret_cast<char *>(&header), sizeof(FILEHeader));

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

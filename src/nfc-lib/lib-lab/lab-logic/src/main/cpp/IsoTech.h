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

#ifndef LOGIC_ISOTECH_H
#define LOGIC_ISOTECH_H

#include <cmath>
#include <algorithm>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>
#include <hw/RecordDevice.h>

#define DEBUG_CHANNELS 10
#define DEBUG_SIGNAL_DATA_CHANNEL 0
#define DEBUG_SIGNAL_EDGE_CHANNEL 4
#define DEBUG_SIGNAL_BIT_CHANNEL 8
#define DEBUG_SIGNAL_BYTE_CHANNEL 9

namespace lab {

/*
 * Signal debugger
 */
struct IsoSignalDebug
{
   unsigned int channels;
   unsigned int clock;

   hw::RecordDevice *recorder;
   hw::SignalBuffer buffer;

   float values[10] {};

   IsoSignalDebug(unsigned int channels, unsigned int sampleRate) : channels(channels), clock(0)
   {
      char file[128];
      tm timeinfo {};

      std::time_t rawTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

#ifdef _WIN32
      localtime_s(&timeinfo, &rawTime);
#else
      localtime_r(&rawTime, &timeinfo);
#endif

      strftime(file, sizeof(file), "logic-debug-%Y%m%d%H%M%S.wav", &timeinfo);

      recorder = new hw::RecordDevice(file);
      recorder->set(hw::SignalDevice::PARAM_CHANNEL_COUNT, channels);
      recorder->set(hw::SignalDevice::PARAM_SAMPLE_RATE, sampleRate);
      recorder->open(hw::RecordDevice::Mode::Write);
   }

   ~IsoSignalDebug()
   {
      delete recorder;
   }

   void block(unsigned int time)
   {
      if (clock != time)
      {
         // store sample buffer
         buffer.put(values, std::get<unsigned int>(recorder->get(hw::SignalDevice::PARAM_CHANNEL_COUNT)));

         // clear sample buffer
         for (auto &f: values)
         {
            f = 0;
         }

         clock = time;
      }
   }

   void set(int channel, float value)
   {
      if (channel >= 0 && channel < std::get<unsigned int>(recorder->get(hw::SignalDevice::PARAM_CHANNEL_COUNT)))
      {
         values[channel] = value;
      }
   }

   void begin(unsigned int sampleCount)
   {
      unsigned int channelCount = std::get<unsigned int>(recorder->get(hw::SignalDevice::PARAM_CHANNEL_COUNT));
      unsigned int sampleRate = std::get<unsigned int>(recorder->get(hw::SignalDevice::PARAM_SAMPLE_RATE));

      buffer = hw::SignalBuffer(sampleCount * channelCount, channelCount, 1, sampleRate, 0, 0, hw::SignalType::SIGNAL_TYPE_RAW_REAL);
   }

   void write()
   {
      buffer.flip();
      recorder->write(buffer);
   }

   void close()
   {
      recorder->close();
   }
};

/*
 * bitrate timing parameters (one for each symbol rate)
 */
struct IsoBitrateParams
{
   int rateType;
   int techType;
};

/*
 * modulation status (one for each symbol rate)
 */
struct IsoModulationStatus
{
   // symbol search status
   unsigned int searchModeState; // search mode / state control
   unsigned int searchStartTime; // sample start of symbol search window
   unsigned int searchEndTime; // sample end of symbol search window
   unsigned int searchSyncTime; // next symbol synchronization point

   // sync parameters
   unsigned int syncStartTime; // sample time for sync start
   unsigned int syncEndTime; // sample time for sync end
};

/*
 * status for one demodulated symbol
 */
struct IsoSymbolStatus
{
   unsigned int value; // symbol state L/H (0 / 1)
   unsigned int data; // symbol value after apply convention (0 / 1)
   unsigned long sync; // symbol sample time
   unsigned long start; // symbol start time
   unsigned long end; // symbol end time
};

/*
 * status for demodulate character
 */
struct IsoCharacterStatus
{
   unsigned int bits;
   unsigned int data;
   unsigned int flags;
   unsigned int parity;
   unsigned long start; // sample clocks for character start
   unsigned long end; // sample clocks for character end
};

/*
 * status for current frame
 */
struct IsoFrameStatus
{
   unsigned int lastCommand; // last received command
   unsigned int frameType; // frame type
   unsigned int symbolRate; // frame bit rate
   unsigned int frameStart; // sample clocks for start of last decoded symbol
   unsigned int frameEnd; // sample clocks for end of last decoded symbol
   unsigned int frameFlags; // frame flags, parity error, etc.
   unsigned int guardTime; // Minimum delay between two leading edges of two consecutive characters.
   unsigned int waitingTime; // Maximum delay between two leading edges of two consecutive characters.

   // frame bytes
   unsigned int frameSize;
   unsigned char frameData[1024];
};

/*
 * sample parameters
 */
struct IsoTimeSample
{
   unsigned int value[4]; // up to 4 channels per sample
};

struct IsoDecoderStatus
{
   // detected signal bitrate
   IsoBitrateParams *bitrate = nullptr;

   // detected modulation
   IsoModulationStatus *modulation = nullptr;

   // signal sample time = 1/FC
   double sampleTime = INFINITY;

   // signal sample rate
   unsigned int sampleRate = 0;

   // signal master clock
   unsigned int signalClock = 0;

   // reference time for all decoded frames
   unsigned int streamTime = 0;

   // signal debugger
   std::shared_ptr<IsoSignalDebug> debug;

   // interleaved signal buffer
   hw::SignalBuffer signalCache;

   // previous data samples
   float sampleLast[8];

   // current data samples
   float sampleData[8];

   // current sample changes
   float sampleEdge[8];

   // process next sample from signal buffer
   bool nextSample(hw::SignalBuffer &buffer);

   // check if there are samples remain to process in buffer
   bool hasSamples(const hw::SignalBuffer &samples) const;
};

struct IsoTech
{
};

}
#endif

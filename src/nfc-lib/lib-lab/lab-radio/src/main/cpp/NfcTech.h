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

#ifndef NFC_NFCTECH_H
#define NFC_NFCTECH_H

#include <cmath>
#include <algorithm>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>
#include <hw/RecordDevice.h>

#define DEBUG_CHANNELS 10
#define DEBUG_SIGNAL_VALUE_CHANNEL 0
#define DEBUG_SIGNAL_FILTERED_CHANNEL 1
#define DEBUG_SIGNAL_VARIANCE_CHANNEL 2
#define DEBUG_SIGNAL_AVERAGE_CHANNEL 3
#define DEBUG_SIGNAL_DECODER_CHANNEL 4

// Buffer length for signal integration, must be power of 2^n
#define BUFFER_SIZE 1024

namespace lab {

/*
 * Signal debugger
 */
struct NfcSignalDebug
{
   unsigned int channels;
   unsigned int clock;

   hw::RecordDevice *recorder;
   hw::SignalBuffer buffer;

   float values[10] {};

   NfcSignalDebug(unsigned int channels, unsigned int sampleRate) : channels(channels), clock(0)
   {
      char file[128];
      tm timeinfo {};

      std::time_t rawTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

#ifdef _WIN32
      localtime_s(&timeinfo, &rawTime);
#else
      localtime_r(&rawTime, &timeinfo);
#endif

      strftime(file, sizeof(file), "radio-debug-%Y%m%d%H%M%S.wav", &timeinfo);

      recorder = new hw::RecordDevice(file);
      recorder->set(hw::SignalDevice::PARAM_CHANNEL_COUNT, channels);
      recorder->set(hw::SignalDevice::PARAM_SAMPLE_RATE, sampleRate);
      recorder->open(hw::RecordDevice::Mode::Write);
   }

   ~NfcSignalDebug()
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

      buffer = hw::SignalBuffer(sampleCount * channelCount, channelCount, 1, sampleRate, 0, 0, hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES);
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
 * pulse slot parameters (for pulse position modulation NFC-V)
 */
struct NfcPulseSlot
{
   int start;
   int end;
   int value;
};

/*
 * baseband processor signal parameters
 */
struct NfcSignalParams
{
   // factors for IIR DC removal filter
   float signalIIRdcA;

   // factors for exponential signal power
   float signalEnveW0;
   float signalEnveW1;

   // factors for exponential signal envelope
   float signalMeanW0;
   float signalMeanW1;

   // factors for exponential signal variance
   float signalMdevW0;
   float signalMdevW1;

   // 1/FC
   double sampleTimeUnit;

   // maximum silence
   int elementaryTimeUnit;
};

/*
 * bitrate timing parameters (one for each symbol rate)
 */
struct NfcBitrateParams
{
   int rateType;
   int techType;

   // symbol parameters
   unsigned int symbolsPerSecond;
   unsigned int period0SymbolSamples;
   unsigned int period1SymbolSamples;
   unsigned int period2SymbolSamples;
   unsigned int period4SymbolSamples;
   unsigned int period8SymbolSamples;

   // modulation parameters
   unsigned int symbolDelayDetect;
   unsigned int offsetFutureIndex;
   unsigned int offsetSignalIndex;
   unsigned int offsetDelay0Index;
   unsigned int offsetDelay1Index;
   unsigned int offsetDelay2Index;
   unsigned int offsetDelay4Index;
   unsigned int offsetDelay8Index;

   // protocol specific parameters
   unsigned int preamble1Samples;
   unsigned int preamble2Samples;
};

/*
 * pulse position modulation parameters (for NFC-V)
 */
struct NfcPulseParams
{
   int bits;
   int length;
   int periods;
   NfcPulseSlot slots[256];
};

/*
 * sample parameters
 */
struct NfcTimeSample
{
   float samplingValue; // sample raw value
   float filteredValue; // IIR-DC filtered value
   float meanDeviation; // standard deviation at sample time
   float modulateDepth; // modulation deep at sample time
};

/*
 * modulation status (one for each symbol rate)
 */
struct NfcModulationStatus
{
   // symbol search status
   unsigned int searchModeState; // search mode / state control
   unsigned int searchStartTime; // sample start of symbol search window
   unsigned int searchEndTime; // sample end of symbol search window
   unsigned int searchSyncTime; // sample at next synchronization
   unsigned int searchPulseWidth; // detected signal pulse width
   float searchValueThreshold; // signal value threshold
   float searchPhaseThreshold; // signal phase threshold
   float searchLastPhase; // auxiliary signal for last symbol phase
   float searchLastValue; // auxiliary signal for value symbol
   float searchSyncValue; // auxiliary signal value at synchronization point
   float searchCorrDValue; // auxiliary value for last correlation difference
   float searchCorr0Value; // auxiliary value for last correlation 0
   float searchCorr1Value; // auxiliary value for last correlation 1

   // symbol parameters
   unsigned int symbolStartTime; // sample time for symbol start
   unsigned int symbolEndTime; // sample time for symbol end
   unsigned int symbolRiseTime; // sample time for last rise edge

   // integrator processor
   float filterIntegrate;
   float detectIntegrate;
   float phaseIntegrate;

   // auxiliary detector peak values
   float correlatedPeakValue;
   float detectorPeakValue;

   // auxiliary detector peak times
   unsigned int correlatedPeakTime; // sample time for maximum correlation peak
   unsigned int detectorPeakTime; // sample time for maximum detector peak

   // data buffers
   float integrationData[BUFFER_SIZE];
   float correlationData[BUFFER_SIZE];
};

/*
 * status for one demodulated symbol
 */
struct NfcSymbolStatus
{
   unsigned int pattern; // symbol pattern
   unsigned int value; // symbol value (0 / 1)
   unsigned long start; // sample clocks for start
   unsigned long end; // sample clocks for end
   unsigned long edge; // sample clocks for last rise edge
   unsigned int length; // length of samples for symbol
   unsigned int rate; // symbol rate
};

/*
 * status for demodulate bit stream
 */
struct NfcStreamStatus
{
   unsigned int previous;
   unsigned int pattern;
   unsigned int bits;
   unsigned int skip;
   unsigned int data;
   unsigned int flags;
   unsigned int parity;
   unsigned int bytes;
   unsigned char buffer[512];
};

/*
 * status for current frame
 */
struct NfcFrameStatus
{
   unsigned int lastCommand; // last received command
   unsigned int frameType; // frame type
   unsigned int symbolRate; // frame bit rate
   unsigned int frameStart; // sample clocks for start of last decoded symbol
   unsigned int frameEnd; // sample clocks for end of last decoded symbol
   unsigned int guardEnd; // frame guard end time
   unsigned int waitingEnd; // frame waiting end time

   // The frame delay time FDT is defined as the time between two frames transmitted in opposite directions
   unsigned int frameGuardTime;

   // The FWT defines the maximum time for a PICC to start its response after the end of a PCD frame.
   unsigned int frameWaitingTime;

   // The SFGT defines a specific guard time needed by the PICC before it is ready to receive the next frame after it has sent the ATS
   unsigned int startUpGuardTime;

   // The Request Guard Time is defined as the minimum time between the start bits of two consecutive REQA commands. It has the value 7000 / fc.
   unsigned int requestGuardTime;
};

struct NfcDecoderStatus
{
   // signal parameters
   NfcSignalParams signalParams {};

   // detected signal bitrate
   NfcBitrateParams *bitrate = nullptr;

   // detected pulse code (for NFC-V)
   NfcPulseParams *pulse = nullptr;

   // detected modulation
   NfcModulationStatus *modulation = nullptr;

   // signal data samples
   NfcTimeSample sample[BUFFER_SIZE];

   // signal sample rate
   unsigned int sampleRate = 0;

   // signal master clock
   unsigned int signalClock = -1;

   // reference time for all decoded frames
   unsigned int streamTime = 0;

   // signal pulse filter
   unsigned int pulseFilter = 0;

   // minimum signal level
   float powerLevelThreshold = 0.01f;

   // signal raw value
   float signalValue = 0;

   // signal DC removed value (IIR filter)
   float signalFiltered = 0;

   // signal envelope value
   float signalEnvelope = 0;

   // signal average value
   float signalAverage = 0;

   // signal variance value
   float signalDeviation = 0;

   // signal DC-removal IIR filter (n sample)
   float signalFilterN0 = 0;

   // signal DC-removal IIR filter (n-1 sample)
   float signalFilterN1 = 0;

   // signal low threshold
   float signalLowThreshold = 0.0090f;

   // signal high threshold
   float signalHighThreshold = 0.0110f;

   // carrier trigger peak value
   float carrierEdgePeak = 0;

   // carrier trigger peak time
   unsigned int carrierEdgeTime = 0;

   // silence start (no modulation detected)
   unsigned int carrierOffTime = 0;

   // silence end (modulation detected)
   unsigned int carrierOnTime = 0;

   // signal debugger
   std::shared_ptr<NfcSignalDebug> debug;

   // process next sample from signal buffer
   bool nextSample(hw::SignalBuffer &buffer);
};

struct NfcTech
{
};

}

#endif

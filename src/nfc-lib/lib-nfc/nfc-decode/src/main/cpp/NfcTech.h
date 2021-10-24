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

#ifndef NFC_NFCTECH_H
#define NFC_NFCTECH_H

#include <cmath>

#include <sdr/RecordDevice.h>

#include <nfc/Nfc.h>

//#define DEBUG_SIGNAL

#ifdef DEBUG_SIGNAL
#define DEBUG_CHANNELS 4
#define DEBUG_SIGNAL_CHANNEL 0
//#define DEBUG_POWER_CHANNEL 0
//#define DEBUG_AVERAGE_CHANNEL 1
//#define DEBUG_VARIANCE_CHANNEL 2
//#define DEBUG_SLOW_AVERAGE_CHANNEL 1
//#define DEBUG_FAST_AVERAGE_CHANNEL 2
#define DEBUG_EDGE_CHANNEL 3
#endif

namespace nfc {

// Buffer length for signal integration, must be power of 2^n
#define BUFFER_SIZE 4096

/*
 * Signal debugger
 */
struct SignalDebug
{
   unsigned int channels;
   unsigned int clock;

   sdr::RecordDevice *recorder;
   sdr::SignalBuffer buffer;

   float values[10] {0,};

   SignalDebug(unsigned int channels, unsigned int sampleRate) : channels(channels), clock(0)
   {
      char file[128];
      struct tm timeinfo {};

      std::time_t rawTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      localtime_s(&timeinfo, &rawTime);
      strftime(file, sizeof(file), "decoder-%Y%m%d%H%M%S.wav", &timeinfo);

      recorder = new sdr::RecordDevice(file);
      recorder->setChannelCount(channels);
      recorder->setSampleRate(sampleRate);
      recorder->open(sdr::RecordDevice::Write);
   }

   ~SignalDebug()
   {
      delete recorder;
   }

   inline void block(unsigned int time)
   {
      if (clock != time)
      {
         // store sample buffer
         buffer.put(values, recorder->channelCount());

         // clear sample buffer
         for (auto &f : values)
         {
            f = 0;
         }

         clock = time;
      }
   }

   inline void set(int channel, float value)
   {
      if (channel >= 0 && channel < recorder->channelCount())
      {
         values[channel] = value;
      }
   }

   inline void begin(int sampleCount)
   {
      buffer = sdr::SignalBuffer(sampleCount * recorder->channelCount(), recorder->channelCount(), recorder->sampleRate());
   }

   inline void write()
   {
      buffer.flip();
      recorder->write(buffer);
   }
};

/*
 * pulse slot parameters (for pulse position modulation NFC-V)
 */
struct PulseSlot
{
   int start;
   int end;
   int value;
};

/*
 * baseband processor signal parameters
 */
struct SignalParams
{
   // factors for exponential signal power
   float signalPowerW0;
   float signalPowerW1;

   // factors for exponential signal average
   float signalAverageW0;
   float signalAverageW1;

   // factors for exponential signal variance
   float signalStdevW0;
   float signalStdevW1;

   // factors for exponential edge detector
   float signalFilterW0;
   float signalFilterW1;
   float signalFilterW2;
   float signalFilterW3;

   // default decoder parameters
   double sampleTimeUnit;
};

/*
 * bitrate timing parameters (one for each symbol rate)
 */
struct BitrateParams
{
   int rateType;
   int techType;

   float symbolAverageW0;
   float symbolAverageW1;

   // symbol parameters
   unsigned int symbolsPerSecond;
   unsigned int period0SymbolSamples;
   unsigned int period1SymbolSamples;
   unsigned int period2SymbolSamples;
   unsigned int period4SymbolSamples;
   unsigned int period8SymbolSamples;

   // modulation parameters
   unsigned int symbolDelayDetect;
   unsigned int offsetSignalIndex;
   unsigned int offsetDelay0Index;
   unsigned int offsetDelay1Index;
   unsigned int offsetDelay2Index;
   unsigned int offsetDelay4Index;
   unsigned int offsetDelay8Index;
};

/*
 * pulse position modulation parameters (for NFC-V)
 */
struct PulseParams
{
   int bits;
   int length;
   int periods;
   PulseSlot slots[256];
};

/*
 * global decoder signal status
 */
struct SignalStatus
{
   // raw IQ sample data
   float sampleData[2];

   // signal parameters
   float signalValue; // instantaneous signal value
   float signalPower;  // very slow exponential average
   float signalAverage; // slow exponential average
   float signalStdev; // slow exponential average for st deviation

   // exponential average for edge detector
   float signalFilter0;
   float signalFilter1;

   // signal data buffer
   float signalData[BUFFER_SIZE];

   // signal edge detect buffer
   float signalEdge[BUFFER_SIZE];

   // signal modulation deep buffer
   float signalDeep[BUFFER_SIZE];

   // signal mean deviation buffer
   float signalMdev[BUFFER_SIZE];

   // silence start (no modulation detected)
   unsigned int carrierOff;

   // silence end (modulation detected)
   unsigned int carrierOn;
};

/*
 * modulation status (one for each symbol rate)
 */
struct ModulationStatus
{
   // symbol search status
   unsigned int searchStage;        // search stage control
   unsigned int searchStartTime;    // sample start of symbol search window
   unsigned int searchEndTime;      // sample end of symbol search window
   unsigned int searchPeakTime;     // sample time for maximum correlation peak
   unsigned int searchPulseWidth;   // detected signal pulse width
   float searchDeepValue;           // signal modulation deep during search
   float searchThreshold;           // signal threshold

   // symbol parameters
   unsigned int symbolStartTime;
   unsigned int symbolEndTime;
   unsigned int symbolSyncTime;
   float symbolCorr0;
   float symbolCorr1;
   float symbolPhase;
   float symbolAverage;

   // integrator processor
   float filterIntegrate;
   float detectIntegrate;
   float phaseIntegrate;
   float phaseThreshold;

   // integration indexes
   unsigned int signalIndex;
   unsigned int delay1Index;
   unsigned int delay2Index;
   unsigned int delay4Index;

   // correlation indexes
   unsigned int filterPoint1;
   unsigned int filterPoint2;
   unsigned int filterPoint3;

   // correlation values
   float correlatedS0;
   float correlatedS1;
   float correlatedSD;
   float correlationPeek;

   // edge detector values
   float detectorPeek;

   // exponential averages
   float slowAverage;
   float fastAverage;

   // data buffers
   float integrationData[BUFFER_SIZE];
   float correlationData[BUFFER_SIZE];
};

/*
 * status for one demodulated symbol
 */
struct SymbolStatus
{
   unsigned int pattern; // symbol pattern
   unsigned int value; // symbol value (0 / 1)
   unsigned long start;    // sample clocks for start of last decoded symbol
   unsigned long end; // sample clocks for end of last decoded symbol
   unsigned int length; // samples for next symbol synchronization
   unsigned int rate; // symbol rate
};

/*
 * status for demodulate bit stream
 */
struct StreamStatus
{
   unsigned int previous;
   unsigned int pattern;
   unsigned int bits;
   unsigned int data;
   unsigned int flags;
   unsigned int parity;
   unsigned int bytes;
   unsigned char buffer[512];
};

/*
 * status for current frame
 */
struct FrameStatus
{
   unsigned int lastCommand; // last received command
   unsigned int frameType; // frame type
   unsigned int symbolRate; // frame bit rate
   unsigned int frameStart;  // sample clocks for start of last decoded symbol
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

/*
 * status for protocol
 */
struct ProtocolStatus
{
   // The FSD defines the maximum size of a frame the PCD is able to receive
   unsigned int maxFrameSize;

   // The frame delay time FDT is defined as the time between two frames transmitted in opposite directions
   unsigned int frameGuardTime;

   // The FWT defines the maximum time for a PICC to start its response after the end of a PCD frame.
   unsigned int frameWaitingTime;

   // The SFGT defines a specific guard time needed by the PICC before it is ready to receive the next frame after it has sent the ATS
   unsigned int startUpGuardTime;

   // The Request Guard Time is defined as the minimum time between the start bits of two consecutive REQA commands. It has the value 7000 / fc.
   unsigned int requestGuardTime;
};

struct DecoderStatus
{
   // signal parameters
   SignalParams signalParams {0,};

   // signal processing status
   SignalStatus signalStatus {0,};

   // detected pulse code (for NFC-V)
   PulseParams *pulse = nullptr;

   // detected signal bitrate
   BitrateParams *bitrate = nullptr;

   // detected modulation
   ModulationStatus *modulation = nullptr;

   // signal sample rate
   unsigned int sampleRate = 0;

   // signal master clock
   unsigned int signalClock = 0;

   // minimum signal level
   float powerLevelThreshold = 0.010f;

   // signal debugger
   std::shared_ptr<SignalDebug> debug;

   // process next sample from signal buffer
   inline bool nextSample(sdr::SignalBuffer &buffer)
   {
      if (buffer.available() == 0)
         return false;

      // real-value signal
      if (buffer.stride() == 1)
      {
         // read next sample data
         buffer.get(signalStatus.signalValue);
      }

         // IQ channel signal
      else
      {
         // read next sample data
         buffer.get(signalStatus.sampleData, 2);

         // compute magnitude from IQ channels
         auto i = double(signalStatus.sampleData[0]);
         auto q = double(signalStatus.sampleData[1]);

         signalStatus.signalValue = sqrtf(i * i + q * q);
      }

      // update signal clock
      signalClock++;

      // compute power average (exponential)
      signalStatus.signalPower = signalStatus.signalPower * signalParams.signalPowerW0 + signalStatus.signalValue * signalParams.signalPowerW1;

      // compute signal average (exponential)
      signalStatus.signalAverage = signalStatus.signalAverage * signalParams.signalAverageW0 + signalStatus.signalValue * signalParams.signalAverageW1;

      // compute signal st deviation (exponential)
      signalStatus.signalStdev = signalStatus.signalStdev * signalParams.signalStdevW0 + std::abs(signalStatus.signalValue - signalStatus.signalAverage) * signalParams.signalStdevW1;

      // compute signal edge detector
      signalStatus.signalFilter0 = signalStatus.signalFilter0 * signalParams.signalFilterW0 + signalStatus.signalValue * signalParams.signalFilterW1;
      signalStatus.signalFilter1 = signalStatus.signalFilter1 * signalParams.signalFilterW2 + signalStatus.signalValue * signalParams.signalFilterW3;

      // store next signal value in sample buffer
      signalStatus.signalData[signalClock & (BUFFER_SIZE - 1)] = signalStatus.signalValue;

      // store next signal value in sample buffer
      signalStatus.signalMdev[signalClock & (BUFFER_SIZE - 1)] = signalStatus.signalStdev;

      // store next edge value in sample buffer
      signalStatus.signalEdge[signalClock & (BUFFER_SIZE - 1)] = std::fabs(signalStatus.signalFilter0 - signalStatus.signalFilter1);

      // store next edge value in sample buffer
      signalStatus.signalDeep[signalClock & (BUFFER_SIZE - 1)] = (signalStatus.signalPower - signalStatus.signalValue) / signalStatus.signalPower;

#ifdef DEBUG_SIGNAL
      debug->block(signalClock);
#endif

#ifdef DEBUG_POWER_CHANNEL
      debug->set(DEBUG_POWER_CHANNEL, signalStatus.signalPower);
#endif

#ifdef DEBUG_SIGNAL_CHANNEL
      debug->set(DEBUG_SIGNAL_CHANNEL, signalStatus.signalValue);
#endif

#ifdef DEBUG_AVERAGE_CHANNEL
      debug->set(DEBUG_AVERAGE_CHANNEL, signalStatus.signalAverage);
#endif

#ifdef DEBUG_VARIANCE_CHANNEL
      debug->set(DEBUG_VARIANCE_CHANNEL, signalStatus.signalStdev);
#endif

#ifdef DEBUG_SLOW_AVERAGE_CHANNEL
      debug->set(DEBUG_SLOW_AVERAGE_CHANNEL, signalStatus.slowAverage);
#endif

#ifdef DEBUG_FAST_AVERAGE_CHANNEL
      debug->set(DEBUG_FAST_AVERAGE_CHANNEL, signalStatus.fastAverage);
#endif

#ifdef DEBUG_EDGE_CHANNEL
      debug->set(DEBUG_EDGE_CHANNEL, std::fabs(signalStatus.signalFilter0 - signalStatus.signalFilter1));
      debug->set(DEBUG_EDGE_CHANNEL - 1, std::fabs(signalStatus.signalAverage - signalStatus.signalPower));
#endif

      return true;
   }
};

}

#endif

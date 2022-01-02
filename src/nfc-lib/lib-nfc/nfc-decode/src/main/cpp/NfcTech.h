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
#include <algorithm>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>
#include <sdr/RecordDevice.h>

#include <nfc/Nfc.h>

//#define DEBUG_SIGNAL

#ifdef DEBUG_SIGNAL
#define DEBUG_CHANNELS 6
#define DEBUG_SIGNAL_VALUE_CHANNEL 0
#define DEBUG_SIGNAL_AVERG_CHANNEL 4
#define DEBUG_SIGNAL_STDEV_CHANNEL 5
//#define DEBUG_SIGNAL_EDGE_CHANNEL 3
//#define DEBUG_SIGNAL_DEEP_CHANNEL 3
#endif

namespace nfc {

// Buffer length for signal integration, must be power of 2^n
#define BUFFER_SIZE 1024

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
         for (auto &f: values)
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
      buffer = sdr::SignalBuffer(sampleCount * recorder->channelCount(), recorder->channelCount(), recorder->sampleRate(), 0, 0, sdr::SignalType::SAMPLE_REAL);
   }

   inline void write()
   {
      buffer.flip();
      recorder->write(buffer);
   }

   inline void close()
   {
      recorder->close();
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
   float signalAvergW0;
   float signalAvergW1;

   // factors for exponential signal variance
   float signalStDevW0;
   float signalStDevW1;

   // factors for exponential edge detector
   float signalEdge0W0;
   float signalEdge0W1;
   float signalEdge1W0;
   float signalEdge1W1;

   // 1/FC
   double sampleTimeUnit;

   // maximum silence
   int elementaryTimeUnit;
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
   unsigned int offsetFutureIndex;
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
   // signal parameters
   float signalAverg; // signal exponential average
   float signalStDev; // signal exponential st deviation

   // exponential average for edge detector
   float signalEdge0;
   float signalEdge1;

   // signal data buffer
   float signalData[BUFFER_SIZE];

   // signal edge detect buffer
   float signalEdge[BUFFER_SIZE];

   // signal modulation deep buffer
   float signalDeep[BUFFER_SIZE];

   // signal average buffer
   float signalAvrg[BUFFER_SIZE];

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
   unsigned int searchModeState;    // search mode / state control
   unsigned int searchStartTime;    // sample start of symbol search window
   unsigned int searchEndTime;      // sample end of symbol search window
   unsigned int searchSyncTime;     // sample at next synchronization
   unsigned int searchPulseWidth;   // detected signal pulse width
   float searchValueThreshold;      // signal value threshold
   float searchPhaseThreshold;      // signal phase threshold
   float searchLastPhase;           // auxiliary signal for last symbol phase
   float searchLastValue;           // auxiliary signal for value symbol
   float searchSyncValue;           // auxiliary signal value at synchronization point

   // symbol parameters
   unsigned int symbolStartTime;
   unsigned int symbolEndTime;
   float symbolCorr0;
   float symbolCorr1;
   float symbolAverage;

   // signal thresholds

   // integrator processor
   float filterIntegrate;
   float detectIntegrate;
   float phaseIntegrate;

   // auxiliary detector peak values
   float correlatedPeakValue;
   float detectorPeakValue;

   // auxiliary detector peak times
   unsigned int correlatedPeakTime;     // sample time for maximum correlation peak
   unsigned int detectorPeakTime;     // sample time for maximum detector peak

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

   // Synchronization time between the start of the PICC subcarrier generation and the start of the PICC subcarrier modulation
   unsigned int tr1MinimumTime;
   unsigned int tr1MaximumTime;
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

   // Synchronization time between the start of the PICC subcarrier generation and the start of the PICC subcarrier modulation
   unsigned int tr1MinimumTime;
   unsigned int tr1MaximumTime;
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

   // signal pulse filter
   unsigned int pulseFilter = 0;

   // minimum signal level
   float powerLevelThreshold = 0.01f;

   // signal debugger
   std::shared_ptr<SignalDebug> debug;

   // process next sample from signal buffer
   inline bool nextSample(sdr::SignalBuffer &buffer)
   {
      if (buffer.available() == 0 || buffer.type() != sdr::SignalType::SAMPLE_REAL)
         return false;

      // update signal clock and pulse filter
      ++signalClock;
      ++pulseFilter;

      float signalValue;

      buffer.get(signalValue);

      float signalStDev = std::fabs(signalValue - signalStatus.signalAverg);

      float signalDiff = signalStDev / signalStatus.signalAverg;

      // signal average envelope detector
      if (signalDiff < 0.05f || pulseFilter > signalParams.elementaryTimeUnit)
      {
         // reset silence counter
         pulseFilter = 0;

         // compute signal average
         signalStatus.signalAverg = signalStatus.signalAverg * signalParams.signalAvergW0 + signalValue * signalParams.signalAvergW1;
      }

      // compute signal st deviation
      signalStatus.signalStDev = signalStatus.signalStDev * signalParams.signalStDevW0 + signalStDev * signalParams.signalStDevW1;

      // fast average edge detector
      signalStatus.signalEdge0 = signalStatus.signalEdge0 * signalParams.signalEdge0W0 + signalValue * signalParams.signalEdge0W1;

      // slow average edge detector
      signalStatus.signalEdge1 = signalStatus.signalEdge1 * signalParams.signalEdge1W0 + signalValue * signalParams.signalEdge1W1;

      // store next signal value in sample buffer
      signalStatus.signalData[signalClock & (BUFFER_SIZE - 1)] = signalValue;

      // store next signal average in sample buffer
      signalStatus.signalAvrg[signalClock & (BUFFER_SIZE - 1)] = signalStatus.signalAverg;

      // store next signal value in sample buffer
      signalStatus.signalMdev[signalClock & (BUFFER_SIZE - 1)] = signalStatus.signalStDev;

      // store next edge value in sample buffer
      signalStatus.signalEdge[signalClock & (BUFFER_SIZE - 1)] = (signalStatus.signalEdge0 - signalStatus.signalEdge1);

      // store next edge value in sample buffer
      signalStatus.signalDeep[signalClock & (BUFFER_SIZE - 1)] = (signalStatus.signalAverg - std::clamp(signalValue, 0.0f, signalStatus.signalAverg)) / signalStatus.signalAverg;

#ifdef DEBUG_SIGNAL
      debug->block(signalClock);
#endif

#ifdef DEBUG_SIGNAL_VALUE_CHANNEL
      debug->set(DEBUG_SIGNAL_VALUE_CHANNEL, signalValue);
#endif

#ifdef DEBUG_SIGNAL_AVERG_CHANNEL
      debug->set(DEBUG_SIGNAL_AVERG_CHANNEL, signalStatus.signalAverg);
#endif

#ifdef DEBUG_SIGNAL_STDEV_CHANNEL
      debug->set(DEBUG_SIGNAL_STDEV_CHANNEL, signalStatus.signalStDev);
#endif

#ifdef DEBUG_SIGNAL_EDGE_CHANNEL
      debug->set(DEBUG_SIGNAL_EDGE_CHANNEL, signalStatus.signalEdge[signalClock & (BUFFER_SIZE - 1)]);
#endif

#ifdef DEBUG_SIGNAL_DEEP_CHANNEL
      debug->set(DEBUG_SIGNAL_DEEP_CHANNEL, signalStatus.signalDeep[signalClock & (BUFFER_SIZE - 1)]);
#endif

      return true;
   }
};

struct NfcTech
{
   unsigned short crc16(NfcFrame &frame, int from, int to, unsigned short init, bool refin);
};

}

#endif

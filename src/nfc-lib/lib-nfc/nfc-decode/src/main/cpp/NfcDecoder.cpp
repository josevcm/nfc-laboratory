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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <cmath>
#include <chrono>
#include <functional>

#include <rt/Logger.h>

#include <nfc/Nfc.h>
#include <nfc/NfcDecoder.h>

#include "NfcStatus.h"
#include "NfcA.h"
#include "NfcB.h"

//#define DEBUG_SIGNAL

#ifdef DEBUG_SIGNAL
#define DEBUG_CHANNELS 8

#define DEBUG_SIGNAL_VALUE_CHANNEL 0
#define DEBUG_SIGNAL_POWER_CHANNEL 1
#define DEBUG_SIGNAL_AVERAGE_CHANNEL 2
#define DEBUG_SIGNAL_VARIANCE_CHANNEL 3
#define DEBUG_SIGNAL_EDGE_CHANNEL 4

#define DEBUG_ASK_CORRELATION_CHANNEL 5
#define DEBUG_ASK_INTEGRATION_CHANNEL 6
#define DEBUG_ASK_SYNCHRONIZATION_CHANNEL 7

#define DEBUG_BPSK_PHASE_INTEGRATION_CHANNEL 5
#define DEBUG_BPSK_PHASE_DEMODULATION_CHANNEL 4
#define DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL 7
#endif


namespace nfc {

/*
 * Signal debugger
 */
#ifdef DEBUG_SIGNAL
struct DecoderDebug
{
   unsigned int channels;
   unsigned int clock;

   sdr::RecordDevice *recorder;
   sdr::SignalBuffer buffer;

   float values[10] {0,};

   DecoderDebug(unsigned int channels, unsigned int decoder.sampleRate) : channels(channels), clock(0)
   {
      char file[128];
      struct tm timeinfo {};

      std::time_t rawTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      localtime_s(&timeinfo, &rawTime);
      strftime(file, sizeof(file), "decoder-%Y%m%d%H%M%S.wav", &timeinfo);

      recorder = new sdr::RecordDevice(file);
      recorder->setChannelCount(channels);
      recorder->setSampleRate(decoder.sampleRate);
      recorder->open(sdr::RecordDevice::Write);
   }

   ~DecoderDebug()
   {
      delete recorder;
   }

   void block(unsigned int time)
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

   void value(int channel, float value)
   {
      if (channel >= 0 && channel < recorder->channelCount())
      {
         values[channel] = value;
      }
   }

   void begin(int sampleCount)
   {
      buffer = sdr::SignalBuffer(sampleCount * recorder->channelCount(), recorder->channelCount(), recorder->decoder.sampleRate());
   }

   void commit()
   {
      buffer.flip();

      recorder->write(buffer);
   }
};
#endif

struct NfcDecoder::Impl
{
   // frame handler
   typedef std::function<bool(nfc::NfcFrame &frame)> FrameHandler;

   rt::Logger log {"NfcDecoder"};

   // global decoder status
   struct DecoderStatus decoder;

   // NFC-A Decoder
   struct NfcA nfca;

   // NFC-B Decoder
   struct NfcB nfcb;

   // decoder signal debugging
#ifdef DEBUG_SIGNAL
   std::shared_ptr<DecoderDebug> decoderDebug;
#endif

   // frame handlers
//   std::vector<FrameHandler> frameHandlers;

   Impl();

   inline void configure(long sampleRate);

   inline std::list<NfcFrame> nextFrames(sdr::SignalBuffer &samples);

   inline bool detectModulation(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames);

   inline bool nextSample(sdr::SignalBuffer &buffer);
};

NfcDecoder::NfcDecoder() : impl(std::make_shared<Impl>())
{
}

std::list<NfcFrame> NfcDecoder::nextFrames(sdr::SignalBuffer samples)
{
   return impl->nextFrames(samples);
}

void NfcDecoder::setSampleRate(long sampleRate)
{
   impl->configure(sampleRate);
}

void NfcDecoder::setPowerLevelThreshold(float value)
{
   impl->decoder.powerLevelThreshold = value;
}

float NfcDecoder::powerLevelThreshold() const
{
   return impl->decoder.powerLevelThreshold;
}

void NfcDecoder::setModulationThreshold(float value)
{
   impl->decoder.modulationThreshold = value;
}

float NfcDecoder::modulationThreshold() const
{
   return impl->decoder.modulationThreshold;
}

float NfcDecoder::signalStrength() const
{
   return impl->decoder.signalStatus.powerAverage;
}

NfcDecoder::Impl::Impl() : nfca(&decoder), nfcb(&decoder)
{
}

/**
 * Configure samplerate
 */
void NfcDecoder::Impl::configure(long newSampleRate)
{
   // clear signal parameters
   decoder.signalParams = {0,};

   // clear signal processing status
   decoder.signalStatus = {0,};

   // set decoder samplerate
   decoder.sampleRate = newSampleRate;

   // clear signal master clock
   decoder.signalClock = 0;

   // configure only if samplerate > 0
   if (decoder.sampleRate > 0)
   {
      // calculate sample time unit, (equivalent to 1/fc in ISO/IEC 14443-3 specifications)
      decoder.signalParams.sampleTimeUnit = double(decoder.sampleRate) / double(BaseFrequency);

      // configure NFC-A decoder
      nfca.configure(newSampleRate);
   }

   // starts without bitrate
   decoder.bitrate = nullptr;

   // starts without modulation
   decoder.modulation = nullptr;

#ifdef DEBUG_SIGNAL
   log.warn("DECODER DEBUGGER ENABLED!, performance may be impacted");
   decoderDebug = std::make_shared<DecoderDebug>(DEBUG_CHANNELS, decoder.sampleRate);
#endif
}

/**
 * Extract next frames
 */
std::list<NfcFrame> NfcDecoder::Impl::nextFrames(sdr::SignalBuffer &samples)
{
   // detected frames
   std::list<NfcFrame> frames;

   // only process valid sample buffer
   if (samples.isValid())
   {
      // re-configure decoder parameters on sample rate changes
      if (decoder.sampleRate != samples.sampleRate())
      {
         configure(samples.sampleRate());
      }

#ifdef DEBUG_SIGNAL
      decoderDebug->begin(samples.elements());
#endif

      while (!samples.isEmpty())
      {
         if (!decoder.modulation)
         {
            if (!nfca.detectModulation(samples, frames))
            {
               break;
            }
         }

         if (decoder.bitrate->techType == TechType::NfcA)
         {
            nfca.decodeFrameNfcA(samples, frames);
         }
         else if (decoder.bitrate->techType == TechType::NfcB)
         {
            nfcb.decodeFrameNfcB(samples, frames);
         }
      }

#ifdef DEBUG_SIGNAL
      decoderDebug->commit();
#endif
   }

      // if sample buffer is not valid only process remain carrier detector
   else
   {
      if (decoder.signalStatus.carrierOff)
      {
         NfcFrame silence = NfcFrame(TechType::None, FrameType::NoCarrier);

         silence.setFramePhase(FramePhase::CarrierFrame);
         silence.setSampleStart(decoder.signalStatus.carrierOff);
         silence.setSampleEnd(decoder.signalClock);
         silence.setTimeStart(double(decoder.signalStatus.carrierOff) / double(decoder.sampleRate));
         silence.setTimeEnd(double(decoder.signalClock) / double(decoder.sampleRate));

         frames.push_back(silence);
      }

      else if (decoder.signalStatus.carrierOn)
      {
         NfcFrame carrier = NfcFrame(TechType::None, FrameType::EmptyFrame);

         carrier.setFramePhase(FramePhase::CarrierFrame);
         carrier.setSampleStart(decoder.signalStatus.carrierOn);
         carrier.setSampleEnd(decoder.signalClock);
         carrier.setTimeStart(double(decoder.signalStatus.carrierOn) / double(decoder.sampleRate));
         carrier.setTimeEnd(double(decoder.signalClock) / double(decoder.sampleRate));

         frames.push_back(carrier);
      }
   }

   // return frame list
   return frames;
}

bool NfcDecoder::Impl::nextSample(sdr::SignalBuffer &buffer)
{
   if (buffer.available() == 0)
      return false;

   // real-value signal
   if (buffer.stride() == 1)
   {
      // read next sample data
      buffer.get(decoder.signalStatus.signalValue);
   }

      // IQ channel signal
   else
   {
      // read next sample data
      buffer.get(decoder.signalStatus.sampleData, 2);

      // compute magnitude from IQ channels
      auto i = double(decoder.signalStatus.sampleData[0]);
      auto q = double(decoder.signalStatus.sampleData[1]);

      decoder.signalStatus.signalValue = sqrtf(i * i + q * q);
   }

   // update signal clock
   decoder.signalClock++;

   // compute power average (exponential average)
   decoder.signalStatus.powerAverage = decoder.signalStatus.powerAverage * decoder.signalParams.powerAverageW0 + decoder.signalStatus.signalValue * decoder.signalParams.powerAverageW1;

   // compute signal average (exponential average)
   decoder.signalStatus.signalAverage = decoder.signalStatus.signalAverage * decoder.signalParams.signalAverageW0 + decoder.signalStatus.signalValue * decoder.signalParams.signalAverageW1;

   // compute signal variance (exponential variance)
   decoder.signalStatus.signalVariance = decoder.signalStatus.signalVariance * decoder.signalParams.signalVarianceW0 + std::abs(decoder.signalStatus.signalValue - decoder.signalStatus.signalAverage) * decoder.signalParams.signalVarianceW1;

   // store next signal value in sample buffer
   decoder.signalStatus.signalData[decoder.signalClock & (SignalBufferLength - 1)] = decoder.signalStatus.signalValue;

#ifdef DEBUG_SIGNAL
   decoderDebug->block(decoder.signalClock);
#endif

#ifdef DEBUG_SIGNAL_VALUE_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_VALUE_CHANNEL, decoder.signalStatus.signalValue);
#endif

#ifdef DEBUG_SIGNAL_POWER_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_POWER_CHANNEL, decoder.signalStatus.powerAverage);
#endif

#ifdef DEBUG_SIGNAL_AVERAGE_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_AVERAGE_CHANNEL, decoder.signalStatus.signalAverage);
#endif

#ifdef DEBUG_SIGNAL_VARIANCE_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_VARIANCE_CHANNEL, decoder.signalStatus.signalVariance);
#endif

#ifdef DEBUG_SIGNAL_EDGE_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_EDGE_CHANNEL, decoder.signalStatus.signalAverage - decoder.signalStatus.powerAverage);
#endif

   return true;
}

}
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

#include <rt/Logger.h>

#include <nfc/Nfc.h>
#include <nfc/NfcDecoder.h>

#include <tech/NfcA.h>
#include <tech/NfcB.h>
#include <tech/NfcF.h>
#include <tech/NfcV.h>

#include <cmath>

namespace nfc {

struct NfcDecoder::Impl
{
   rt::Logger log {"NfcDecoder"};

   static constexpr int ENABLED_NFCA = 1 << 0;
   static constexpr int ENABLED_NFCB = 1 << 1;
   static constexpr int ENABLED_NFCF = 1 << 2;
   static constexpr int ENABLED_NFCV = 1 << 3;

   // all tech enabled by default
   int enabledTech = ENABLED_NFCA | ENABLED_NFCB | ENABLED_NFCF | ENABLED_NFCV;

   // NFC-A Decoder
   struct NfcA nfca;

   // NFC-B Decoder
   struct NfcB nfcb;

   // NFC-F Decoder
   struct NfcF nfcf;

   // NFC-V Decoder
   struct NfcV nfcv;

   // global decoder status
   struct DecoderStatus decoder;

   Impl();

   inline void cleanup();

   inline void initialize();

   inline std::list<NfcFrame> nextFrames(sdr::SignalBuffer &samples);

   inline void detectCarrier(std::list<NfcFrame> &frames);
};

NfcDecoder::NfcDecoder() : impl(std::make_shared<Impl>())
{
}

void NfcDecoder::initialize()
{
   impl->initialize();
}

void NfcDecoder::cleanup()
{
   impl->cleanup();
}

std::list<NfcFrame> NfcDecoder::nextFrames(sdr::SignalBuffer samples)
{
   return impl->nextFrames(samples);
}

bool NfcDecoder::isNfcAEnabled() const
{
   return impl->enabledTech & Impl::ENABLED_NFCA;
}

void NfcDecoder::setEnableNfcA(bool enabled)
{
   if (enabled)
      impl->enabledTech |= Impl::ENABLED_NFCA;
   else
      impl->enabledTech &= ~Impl::ENABLED_NFCA;
}

bool NfcDecoder::isNfcBEnabled() const
{
   return impl->enabledTech & Impl::ENABLED_NFCB;
}

void NfcDecoder::setEnableNfcB(bool enabled)
{
   if (enabled)
      impl->enabledTech |= Impl::ENABLED_NFCB;
   else
      impl->enabledTech &= ~Impl::ENABLED_NFCB;
}

bool NfcDecoder::isNfcFEnabled() const
{
   return impl->enabledTech & Impl::ENABLED_NFCF;
}

void NfcDecoder::setEnableNfcF(bool enabled)
{
   if (enabled)
      impl->enabledTech |= Impl::ENABLED_NFCF;
   else
      impl->enabledTech &= ~Impl::ENABLED_NFCF;
}

bool NfcDecoder::isNfcVEnabled() const
{
   return impl->enabledTech & Impl::ENABLED_NFCV;
}

void NfcDecoder::setEnableNfcV(bool enabled)
{
   if (enabled)
      impl->enabledTech |= Impl::ENABLED_NFCV;
   else
      impl->enabledTech &= ~Impl::ENABLED_NFCV;
}

long NfcDecoder::sampleRate() const
{
   return impl->decoder.sampleRate;
}

void NfcDecoder::setSampleRate(long sampleRate)
{
   impl->decoder.sampleRate = sampleRate;
}

long NfcDecoder::streamTime() const
{
   return impl->decoder.streamTime;
}

void NfcDecoder::setStreamTime(long referenceTime)
{
   impl->decoder.streamTime = referenceTime;
}

void NfcDecoder::setPowerLevelThreshold(float value)
{
   impl->decoder.powerLevelThreshold = value;
}

void NfcDecoder::setModulationThresholdNfcA(float min, float max)
{
   impl->nfca.setModulationThreshold(min, max);
}

void NfcDecoder::setModulationThresholdNfcB(float min, float max)
{
   impl->nfcb.setModulationThreshold(min, max);
}

void NfcDecoder::setModulationThresholdNfcF(float min, float max)
{
   impl->nfcf.setModulationThreshold(min, max);
}

void NfcDecoder::setModulationThresholdNfcV(float min, float max)
{
   impl->nfcv.setModulationThreshold(min, max);
}

float NfcDecoder::powerLevelThreshold() const
{
   return impl->decoder.powerLevelThreshold;
}

NfcDecoder::Impl::Impl() : nfca(&decoder), nfcb(&decoder), nfcf(&decoder), nfcv(&decoder)
{
}

/**
 * Configure samplerate
 */
void NfcDecoder::Impl::initialize()
{
   // clear signal parameters
   decoder.signalParams = {0,};

   // clear signal master clock
   decoder.signalClock = 0;

   // configure only if samplerate > 0
   if (decoder.sampleRate > 0)
   {
      // calculate sample time unit, (equivalent to 1/fc in ISO/IEC 14443-3 specifications)
      decoder.signalParams.sampleTimeUnit = double(decoder.sampleRate) / double(NFC_FC);

      // base elementary time unit
      decoder.signalParams.elementaryTimeUnit = decoder.signalParams.sampleTimeUnit * 128;

      // initialize DC removal IIR filter scale factor
      decoder.signalParams.signalIIRdcA = float(0.9);

      // initialize exponential average factors for signal average
      decoder.signalParams.signalMeanW0 = float(1 - 5E5 / decoder.sampleRate);
      decoder.signalParams.signalMeanW1 = float(1 - decoder.signalParams.signalMeanW0);

      // initialize exponential average factors for signal mean deviation
      decoder.signalParams.signalMdevW0 = float(1 - 2E5 / decoder.sampleRate);
      decoder.signalParams.signalMdevW1 = float(1 - decoder.signalParams.signalMdevW0);

      // initialize exponential average factors for signal mean deviation
      decoder.signalParams.signalEnveW0 = float(1 - 5E4 / decoder.sampleRate);
      decoder.signalParams.signalEnveW1 = float(1 - decoder.signalParams.signalEnveW0);

      // configure carrier detector parameters
      decoder.signalLowThreshold = decoder.powerLevelThreshold / 1.25f;
      decoder.signalHighThreshold = decoder.powerLevelThreshold * 1.25f;

      // configure NFC-A decoder
      nfca.initialize(decoder.sampleRate);

      // configure NFC-B decoder
      nfcb.initialize(decoder.sampleRate);

      // configure NFC-F decoder
      nfcf.initialize(decoder.sampleRate);

      // configure NFC-V decoder
      nfcv.initialize(decoder.sampleRate);

#ifdef DEBUG_SIGNAL
      log.warn("SIGNAL DEBUGGER ENABLED!, highly affected performance!");
      decoder.debug = std::make_shared<SignalDebug>(DEBUG_CHANNELS, decoder.sampleRate);
#endif
   }

   // starts without bitrate
   decoder.bitrate = nullptr;

   // starts without modulation
   decoder.modulation = nullptr;
}

/**
 * Restart decoder status
 */
void NfcDecoder::Impl::cleanup()
{
#ifdef DEBUG_SIGNAL
   decoder.debug.reset();
#endif
}

/**
 * Extract next frames from signal buffer
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
         decoder.sampleRate = samples.sampleRate();

         initialize();
      }

#ifdef DEBUG_SIGNAL
      decoder.debug->begin(samples.elements());
#endif

      do
      {
         if (!decoder.modulation)
         {
            // clear bitrate
            decoder.bitrate = nullptr;

            // NFC modulation detector for NFC-A / B / F / V
            while (decoder.nextSample(samples))
            {
               // carrier detector
               detectCarrier(frames);

               if ((enabledTech & ENABLED_NFCA) && nfca.detect())
                  break;

               if ((enabledTech & ENABLED_NFCB) && nfcb.detect())
                  break;

               if ((enabledTech & ENABLED_NFCF) && nfcf.detect())
                  break;

               if ((enabledTech & ENABLED_NFCV) && nfcv.detect())
                  break;
            }
         }

         if (decoder.bitrate)
         {
            switch (decoder.bitrate->techType)
            {
               case TechType::NfcA:
                  nfca.decode(samples, frames);
                  break;

               case TechType::NfcB:
                  nfcb.decode(samples, frames);
                  break;

               case TechType::NfcF:
                  nfcf.decode(samples, frames);
                  break;

               case TechType::NfcV:
                  nfcv.decode(samples, frames);
                  break;
            }
         }

      } while (!samples.isEmpty());

#ifdef DEBUG_SIGNAL
      decoder.debug->write();
#endif
   }

      // if sample buffer is not valid only process remain carrier detector
   else
   {
      NfcFrame carrierFrame = NfcFrame(TechType::None, decoder.carrierOnTime ? FrameType::CarrierOn : FrameType::CarrierOff);

      carrierFrame.setFramePhase(FramePhase::CarrierFrame);
      carrierFrame.setSampleStart(decoder.signalClock);
      carrierFrame.setSampleEnd(decoder.signalClock);
      carrierFrame.setTimeStart(double(decoder.signalClock) / double(decoder.sampleRate));
      carrierFrame.setTimeEnd(double(decoder.signalClock) / double(decoder.sampleRate));
      carrierFrame.setDateTime(decoder.streamTime + carrierFrame.timeStart());
      carrierFrame.flip();

      frames.push_back(carrierFrame);
   }

   // return frame list
   return frames;
}

/**
 * Detect carrier from signal buffer
 */
void NfcDecoder::Impl::detectCarrier(std::list<NfcFrame> &frames)
{
   // carrier present if signal average is over power Level Threshold
   if (decoder.signalAverage > decoder.signalHighThreshold)
   {
      if (!decoder.carrierOnTime)
      {
         decoder.carrierOnTime = decoder.carrierEdgeTime ? decoder.carrierEdgeTime : decoder.signalClock;

         NfcFrame carrierOn = NfcFrame(TechType::None, FrameType::CarrierOn);

         carrierOn.setFramePhase(FramePhase::CarrierFrame);
         carrierOn.setSampleStart(decoder.carrierOnTime);
         carrierOn.setSampleEnd(decoder.carrierOnTime);
         carrierOn.setTimeStart(double(decoder.carrierOnTime) / double(decoder.sampleRate));
         carrierOn.setTimeEnd(double(decoder.carrierOnTime) / double(decoder.sampleRate));
         carrierOn.setDateTime(decoder.streamTime + carrierOn.timeStart());
         carrierOn.flip();

         frames.push_back(carrierOn);

         decoder.carrierOffTime = 0;
         decoder.carrierEdgeTime = 0;
      }
   }

      // carrier not present if signal average is below power Level Threshold
   else if (decoder.signalAverage < decoder.signalLowThreshold)
   {
      if (!decoder.carrierOffTime)
      {
         decoder.carrierOffTime = decoder.carrierEdgeTime ? decoder.carrierEdgeTime : decoder.signalClock;

         NfcFrame carrierOff = NfcFrame(TechType::None, FrameType::CarrierOff);

         carrierOff.setFramePhase(FramePhase::CarrierFrame);
         carrierOff.setSampleStart(decoder.carrierOffTime);
         carrierOff.setSampleEnd(decoder.carrierOffTime);
         carrierOff.setTimeStart(double(decoder.carrierOffTime) / double(decoder.sampleRate));
         carrierOff.setTimeEnd(double(decoder.carrierOffTime) / double(decoder.sampleRate));
         carrierOff.setDateTime(decoder.streamTime + carrierOff.timeStart());
         carrierOff.flip();

         frames.push_back(carrierOff);

         decoder.carrierOnTime = 0;
         decoder.carrierEdgeTime = 0;
      }
   }
}

}
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

#include <rt/Logger.h>

#include <lab/nfc/Nfc.h>
#include <lab/nfc/NfcDecoder.h>

#include <tech/NfcA.h>
#include <tech/NfcB.h>
#include <tech/NfcF.h>
#include <tech/NfcV.h>

#include <cmath>

namespace lab {

struct NfcDecoder::Impl
{
   rt::Logger *log = rt::Logger::getLogger("decoder.NfcDecoder");

   static constexpr int ENABLED_NFCA = 1 << 0;
   static constexpr int ENABLED_NFCB = 1 << 1;
   static constexpr int ENABLED_NFCF = 1 << 2;
   static constexpr int ENABLED_NFCV = 1 << 3;

   // debug disabled by default
   int debugEnabled = false;

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
   NfcDecoderStatus decoder;

   Impl();

   inline void cleanup();

   inline void initialize();

   inline std::list<RawFrame> nextFrames(hw::SignalBuffer &samples);

   inline void detectCarrier(std::list<RawFrame> &frames);
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

std::list<RawFrame> NfcDecoder::nextFrames(hw::SignalBuffer samples)
{
   return impl->nextFrames(samples);
}

bool NfcDecoder::isDebugEnabled() const
{
   return impl->debugEnabled;
}

void NfcDecoder::setEnableDebug(bool enabled)
{
   impl->debugEnabled = enabled;
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

float NfcDecoder::powerLevelThreshold() const
{
   return impl->decoder.powerLevelThreshold;
}

void NfcDecoder::setPowerLevelThreshold(float value)
{
   impl->decoder.powerLevelThreshold = value;
}

float NfcDecoder::modulationThresholdNfcAMin() const
{
   return impl->nfca.modulationThresholdMin();
}

float NfcDecoder::modulationThresholdNfcAMax() const
{
   return impl->nfca.modulationThresholdMax();
}

void NfcDecoder::setModulationThresholdNfcA(float min, float max)
{
   impl->nfca.setModulationThreshold(min, max);
}

float NfcDecoder::modulationThresholdNfcBMin() const
{
   return impl->nfcb.modulationThresholdMin();
}

float NfcDecoder::modulationThresholdNfcBMax() const
{
   return impl->nfcb.modulationThresholdMax();
}

void NfcDecoder::setModulationThresholdNfcB(float min, float max)
{
   impl->nfcb.setModulationThreshold(min, max);
}

float NfcDecoder::modulationThresholdNfcFMin() const
{
   return impl->nfcf.modulationThresholdMin();
}

float NfcDecoder::modulationThresholdNfcFMax() const
{
   return impl->nfcf.modulationThresholdMax();
}

void NfcDecoder::setModulationThresholdNfcF(float min, float max)
{
   impl->nfcf.setModulationThreshold(min, max);
}

float NfcDecoder::modulationThresholdNfcVMin() const
{
   return impl->nfcv.modulationThresholdMin();
}

float NfcDecoder::modulationThresholdNfcVMax() const
{
   return impl->nfcv.modulationThresholdMax();
}

void NfcDecoder::setModulationThresholdNfcV(float min, float max)
{
   impl->nfcv.setModulationThreshold(min, max);
}

float NfcDecoder::correlationThresholdNfcA() const
{
   return impl->nfca.correlationThreshold();
}

void NfcDecoder::setCorrelationThresholdNfcA(float value)
{
   impl->nfca.setCorrelationThreshold(value);
}

float NfcDecoder::correlationThresholdNfcB() const
{
   return impl->nfcb.correlationThreshold();
}

void NfcDecoder::setCorrelationThresholdNfcB(float value)
{
   impl->nfcb.setCorrelationThreshold(value);
}

float NfcDecoder::correlationThresholdNfcF() const
{
   return impl->nfcf.correlationThreshold();
}

void NfcDecoder::setCorrelationThresholdNfcF(float value)
{
   impl->nfcf.setCorrelationThreshold(value);
}

float NfcDecoder::correlationThresholdNfcV() const
{
   return impl->nfcv.correlationThreshold();
}

void NfcDecoder::setCorrelationThresholdNfcV(float value)
{
   impl->nfcv.setCorrelationThreshold(value);
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
   decoder.signalParams = {};

   // clear signal master clock
   decoder.signalClock = -1;

   // configure only if samplerate > 0
   if (decoder.sampleRate > 0)
   {
      // calculate sample time unit, (equivalent to 1/fc in ISO/IEC 14443-3 specifications)
      decoder.signalParams.sampleTimeUnit = static_cast<double>(decoder.sampleRate) / static_cast<double>(NFC_FC);

      // base elementary time unit
      decoder.signalParams.elementaryTimeUnit = decoder.signalParams.sampleTimeUnit * 128;

      // initialize DC removal IIR filter scale factor
      decoder.signalParams.signalIIRdcA = static_cast<float>(0.9);

      // initialize exponential average factors for signal envelope
      decoder.signalParams.signalEnveW0 = static_cast<float>(1 - 5E5 / decoder.sampleRate);
      decoder.signalParams.signalEnveW1 = static_cast<float>(1 - decoder.signalParams.signalEnveW0);

      // initialize exponential average factors for signal mean deviation
      decoder.signalParams.signalMdevW0 = static_cast<float>(1 - 2E5 / decoder.sampleRate);
      decoder.signalParams.signalMdevW1 = static_cast<float>(1 - decoder.signalParams.signalMdevW0);

      // initialize exponential average factors for signal mean
      decoder.signalParams.signalMeanW0 = static_cast<float>(1 - 5E4 / decoder.sampleRate);
      decoder.signalParams.signalMeanW1 = static_cast<float>(1 - decoder.signalParams.signalMeanW0);

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

      if (debugEnabled)
      {
         log->warn("---------------------------------------------------");
         log->warn("SIGNAL DEBUG ENABLED!, highly affected performance!");
         log->warn("---------------------------------------------------");

         decoder.debug = std::make_shared<NfcSignalDebug>(DEBUG_CHANNELS, decoder.sampleRate);

         log->warn("write signal debug data to file: {}", {decoder.debug->recorder->get<std::string>(hw::SignalDevice::PARAM_DEVICE_NAME)});
      }
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
   if (decoder.debug)
      decoder.debug.reset();
}

/**
 * Extract next frames from signal buffer
 */
std::list<RawFrame> NfcDecoder::Impl::nextFrames(hw::SignalBuffer &samples)
{
   // detected frames
   std::list<RawFrame> frames;

   // only process valid sample buffer
   if (samples.isValid())
   {
      // re-configure decoder parameters on sample rate changes
      if (decoder.sampleRate != samples.sampleRate())
      {
         decoder.sampleRate = samples.sampleRate();

         initialize();
      }

      if (decoder.debug)
         decoder.debug->begin(samples.elements());

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
               case NfcATech:
                  nfca.decode(samples, frames);
                  break;

               case NfcBTech:
                  nfcb.decode(samples, frames);
                  break;

               case NfcFTech:
                  nfcf.decode(samples, frames);
                  break;

               case NfcVTech:
                  nfcv.decode(samples, frames);
                  break;
            }
         }

      }
      while (!samples.isEmpty());

      if (decoder.debug)
         decoder.debug->write();
   }

   // if sample buffer is not valid only process remain carrier detector
   else
   {
      RawFrame carrierFrame = RawFrame(NfcAnyTech, decoder.carrierOnTime ? NfcCarrierOn : NfcCarrierOff);

      carrierFrame.setFramePhase(NfcCarrierPhase);
      carrierFrame.setSampleStart(decoder.signalClock);
      carrierFrame.setSampleEnd(decoder.signalClock);
      carrierFrame.setSampleRate(decoder.sampleRate);
      carrierFrame.setTimeStart(static_cast<double>(decoder.signalClock) / static_cast<double>(decoder.sampleRate));
      carrierFrame.setTimeEnd(static_cast<double>(decoder.signalClock) / static_cast<double>(decoder.sampleRate));
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
void NfcDecoder::Impl::detectCarrier(std::list<RawFrame> &frames)
{
   // carrier present if signal average is over power Level Threshold
   if (decoder.signalAverage > decoder.signalHighThreshold)
   {
      if (!decoder.carrierOnTime)
      {
         decoder.carrierOnTime = decoder.carrierEdgeTime ? decoder.carrierEdgeTime : decoder.signalClock;

         RawFrame carrierOn = RawFrame(NfcAnyTech, NfcCarrierOn);

         carrierOn.setFramePhase(NfcCarrierPhase);
         carrierOn.setSampleStart(decoder.carrierOnTime);
         carrierOn.setSampleEnd(decoder.carrierOnTime);
         carrierOn.setSampleRate(decoder.sampleRate);
         carrierOn.setTimeStart(static_cast<double>(decoder.carrierOnTime) / static_cast<double>(decoder.sampleRate));
         carrierOn.setTimeEnd(static_cast<double>(decoder.carrierOnTime) / static_cast<double>(decoder.sampleRate));
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

         RawFrame carrierOff = RawFrame(NfcAnyTech, NfcCarrierOff);

         carrierOff.setFramePhase(NfcCarrierPhase);
         carrierOff.setSampleStart(decoder.carrierOffTime);
         carrierOff.setSampleEnd(decoder.carrierOffTime);
         carrierOff.setSampleRate(decoder.sampleRate);
         carrierOff.setTimeStart(static_cast<double>(decoder.carrierOffTime) / static_cast<double>(decoder.sampleRate));
         carrierOff.setTimeEnd(static_cast<double>(decoder.carrierOffTime) / static_cast<double>(decoder.sampleRate));
         carrierOff.setDateTime(decoder.streamTime + carrierOff.timeStart());
         carrierOff.flip();

         frames.push_back(carrierOff);

         decoder.carrierOnTime = 0;
         decoder.carrierEdgeTime = 0;
      }
   }
}

}

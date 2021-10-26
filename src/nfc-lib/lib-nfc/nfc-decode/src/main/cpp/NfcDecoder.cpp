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

   inline void configure(long sampleRate);

   inline std::list<NfcFrame> nextFrames(sdr::SignalBuffer &samples);

   inline void detectCarrier(std::list<NfcFrame> &frames);
};

NfcDecoder::NfcDecoder() : impl(std::make_shared<Impl>())
{
}

std::list<NfcFrame> NfcDecoder::nextFrames(sdr::SignalBuffer samples)
{
   return impl->nextFrames(samples);
}

void NfcDecoder::setEnableNfcA(bool enabled)
{
   if (enabled)
      impl->enabledTech |= Impl::ENABLED_NFCA;
   else
      impl->enabledTech &= ~Impl::ENABLED_NFCA;
}

void NfcDecoder::setEnableNfcB(bool enabled)
{
   if (enabled)
      impl->enabledTech |= Impl::ENABLED_NFCB;
   else
      impl->enabledTech &= ~Impl::ENABLED_NFCB;
}

void NfcDecoder::setEnableNfcF(bool enabled)
{
   if (enabled)
      impl->enabledTech |= Impl::ENABLED_NFCF;
   else
      impl->enabledTech &= ~Impl::ENABLED_NFCF;
}

void NfcDecoder::setEnableNfcV(bool enabled)
{
   if (enabled)
      impl->enabledTech |= Impl::ENABLED_NFCV;
   else
      impl->enabledTech &= ~Impl::ENABLED_NFCV;
}

void NfcDecoder::setSampleRate(long sampleRate)
{
   impl->configure(sampleRate);
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

float NfcDecoder::signalStrength() const
{
   return impl->decoder.signalStatus.signalAverg;
}

NfcDecoder::Impl::Impl() : nfca(&decoder), nfcb(&decoder), nfcf(&decoder), nfcv(&decoder)
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
      decoder.signalParams.sampleTimeUnit = double(decoder.sampleRate) / double(NFC_FC);

      // maximum silence duration
      decoder.signalParams.silenceThreshold = decoder.signalParams.sampleTimeUnit * 2048;

      // initialize exponential average factors for power value
      decoder.signalParams.signalAvergW0 = float(1 - 1E4 / decoder.sampleRate);
      decoder.signalParams.signalAvergW1 = float(1 - decoder.signalParams.signalAvergW0);

      // initialize exponential average factors for signal variance
      decoder.signalParams.signalStDevW0 = float(1 - 1E5 / decoder.sampleRate);
      decoder.signalParams.signalStDevW1 = float(1 - decoder.signalParams.signalStDevW0);

      // initialize exponential slow average factors for edge detector
      decoder.signalParams.signalEdge0W0 = float(1 - 4E6 / decoder.sampleRate);
      decoder.signalParams.signalEdge0W1 = float(1 - decoder.signalParams.signalEdge0W0);

      // initialize exponential fast average factors for edge detector
      decoder.signalParams.signalEdge1W0 = float(1 - 3E6 / decoder.sampleRate);
      decoder.signalParams.signalEdge1W1 = float(1 - decoder.signalParams.signalEdge1W0);

      // configure NFC-A decoder
      if (enabledTech & ENABLED_NFCA)
         nfca.configure(newSampleRate);
      else
         log.info("NFC-A decoder is disabled");

      // configure NFC-B decoder
      if (enabledTech & ENABLED_NFCB)
         nfcb.configure(newSampleRate);
      else
         log.info("NFC-B decoder is disabled");

      // configure NFC-F decoder
      if (enabledTech & ENABLED_NFCF)
         nfcf.configure(newSampleRate);
      else
         log.info("NFC-F decoder is disabled");

      // configure NFC-V decoder
      if (enabledTech & ENABLED_NFCV)
         nfcv.configure(newSampleRate);
      else
         log.info("NFC-V decoder is disabled");

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

void NfcDecoder::Impl::detectCarrier(std::list<NfcFrame> &frames)
{
   /*
    * carrier presence detector
    */

   // carrier present if signal average is over power Level Threshold
   if (decoder.signalStatus.signalAverg > decoder.powerLevelThreshold)
   {
      if (!decoder.signalStatus.carrierOn)
      {
         decoder.signalStatus.carrierOn = decoder.signalClock;

         if (decoder.signalStatus.carrierOff)
         {
            NfcFrame silence = NfcFrame(TechType::None, FrameType::NoCarrier);

            silence.setFramePhase(FramePhase::CarrierFrame);
            silence.setSampleStart(decoder.signalStatus.carrierOff);
            silence.setSampleEnd(decoder.signalStatus.carrierOn);
            silence.setTimeStart(double(decoder.signalStatus.carrierOff) / double(decoder.sampleRate));
            silence.setTimeEnd(double(decoder.signalStatus.carrierOn) / double(decoder.sampleRate));

            frames.push_back(silence);
         }

         decoder.signalStatus.carrierOff = 0;
      }
   }

      // carrier not present if signal average is below power Level Threshold
   else if (decoder.signalStatus.signalAverg < decoder.powerLevelThreshold)
   {
      if (!decoder.signalStatus.carrierOff)
      {
         decoder.signalStatus.carrierOff = decoder.signalClock;

         if (decoder.signalStatus.carrierOn)
         {
            NfcFrame carrier = NfcFrame(TechType::None, FrameType::EmptyFrame);

            carrier.setFramePhase(FramePhase::CarrierFrame);
            carrier.setSampleStart(decoder.signalStatus.carrierOn);
            carrier.setSampleEnd(decoder.signalStatus.carrierOff);
            carrier.setTimeStart(double(decoder.signalStatus.carrierOn) / double(decoder.sampleRate));
            carrier.setTimeEnd(double(decoder.signalStatus.carrierOff) / double(decoder.sampleRate));

            frames.push_back(carrier);
         }

         decoder.signalStatus.carrierOn = 0;
      }
   }
}

//void NfcDecoder::Impl::detectCarrier(std::list<NfcFrame> &frames)
//{
//   /*
//    * carrier presence detector
//    */
//
//   // carrier present if signal average is over power Level Threshold
//   if (decoder.signalStatus.signalAverage > decoder.powerLevelThreshold)
//   {
//      if (!decoder.signalStatus.carrierOn)
//      {
//         decoder.signalStatus.carrierOn = decoder.signalClock;
//
//         if (decoder.signalStatus.carrierOff)
//         {
//            NfcFrame silence = NfcFrame(TechType::None, FrameType::NoCarrier);
//
//            silence.setFramePhase(FramePhase::CarrierFrame);
//            silence.setSampleStart(decoder.signalStatus.carrierOff);
//            silence.setSampleEnd(decoder.signalStatus.carrierOn);
//            silence.setTimeStart(double(decoder.signalStatus.carrierOff) / double(decoder.sampleRate));
//            silence.setTimeEnd(double(decoder.signalStatus.carrierOn) / double(decoder.sampleRate));
//
//            frames.push_back(silence);
//         }
//
//         decoder.signalStatus.carrierOff = 0;
//      }
//   }
//
//      // carrier not present if signal average is below power Level Threshold
//   else if (decoder.signalStatus.signalAverage < decoder.powerLevelThreshold)
//   {
//      if (!decoder.signalStatus.carrierOff)
//      {
//         decoder.signalStatus.carrierOff = decoder.signalClock;
//
//         if (decoder.signalStatus.carrierOn)
//         {
//            NfcFrame carrier = NfcFrame(TechType::None, FrameType::EmptyFrame);
//
//            carrier.setFramePhase(FramePhase::CarrierFrame);
//            carrier.setSampleStart(decoder.signalStatus.carrierOn);
//            carrier.setSampleEnd(decoder.signalStatus.carrierOff);
//            carrier.setTimeStart(double(decoder.signalStatus.carrierOn) / double(decoder.sampleRate));
//            carrier.setTimeEnd(double(decoder.signalStatus.carrierOff) / double(decoder.sampleRate));
//
//            frames.push_back(carrier);
//         }
//
//         decoder.signalStatus.carrierOn = 0;
//      }
//   }
//}

}
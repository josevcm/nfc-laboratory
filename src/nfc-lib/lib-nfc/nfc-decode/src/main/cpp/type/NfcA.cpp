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

#include <cmath>
#include <chrono>
#include <functional>

#include "NfcA.h"

namespace nfc {

// FSDI to FSD conversion (frame size)
static const int TABLE_FDS[] = {16, 24, 32, 40, 48, 64, 96, 128, 256, 0, 0, 0, 0, 0, 0, 256};

struct NfcA::Impl
{
   rt::Logger log {"NfcA"};

   DecoderStatus *decoder;

   // bitrate parameters
   BitrateParams bitrateParams[4] {0,};

   // detected symbol status
   SymbolStatus symbolStatus {0,};

   // bit stream status
   StreamStatus streamStatus {0,};

   // frame processing status
   FrameStatus frameStatus {0,};

   // protocol processing status
   ProtocolStatus protocolStatus {0,};

   // modulation status for each bitrate
   ModulationStatus modulationStatus[4] {0,};

   // last detected frame end
   unsigned int lastFrameEnd = 0;

   // chained frame flags
   unsigned int chainedFlags = 0;

   Impl(DecoderStatus *decoder) : decoder(decoder)
   {
   }
};

NfcA::NfcA(DecoderStatus *decoder) : self(new Impl(decoder))
{
}

NfcA::~NfcA()
{
   delete self;
}

void NfcA::configure(long sampleRate)
{
   self->log.info("--------------------------------------------");
   self->log.info("initializing NFC-A decoder");
   self->log.info("--------------------------------------------");
   self->log.info("\tsignalSampleRate     {}", {self->decoder->sampleRate});
   self->log.info("\tself->decoder->powerLevelThreshold  {}", {self->decoder->powerLevelThreshold});
   self->log.info("\tself->self->decoder->modulationThreshold  {}", {self->decoder->modulationThreshold});

   // clear detected symbol status
   self->symbolStatus = {0,};

   // clear bit stream status
   self->streamStatus = {0,};

   // clear frame processing status
   self->frameStatus = {0,};

   // clear last detected frame end
   self->lastFrameEnd = 0;

   // clear chained flags
   self->chainedFlags = 0;

   // compute symbol parameters for 106Kbps, 212Kbps, 424Kbps and 848Kbps
   for (int rate = r106k; rate <= r424k; rate++)
   {
      // clear bitrate parameters
      self->bitrateParams[rate] = {0,};

      // clear modulation parameters
      self->modulationStatus[rate] = {0,};

      // configure bitrate parametes
      BitrateParams *bitrate = self->bitrateParams + rate;

      // set tech type and rate
      bitrate->techType = TechType::NfcA;
      bitrate->rateType = rate;

      // symbol timing parameters
      bitrate->symbolsPerSecond = BaseFrequency / (128 >> rate);

      // number of samples per symbol
      bitrate->period1SymbolSamples = int(round(self->decoder->signalParams.sampleTimeUnit * (128 >> rate))); // full symbol samples
      bitrate->period2SymbolSamples = int(round(self->decoder->signalParams.sampleTimeUnit * (64 >> rate))); // half symbol samples
      bitrate->period4SymbolSamples = int(round(self->decoder->signalParams.sampleTimeUnit * (32 >> rate))); // quarter of symbol...
      bitrate->period8SymbolSamples = int(round(self->decoder->signalParams.sampleTimeUnit * (16 >> rate))); // and so on...

      // delay guard for each symbol rate
      bitrate->symbolDelayDetect = rate > r106k ? self->bitrateParams[rate - 1].symbolDelayDetect + self->bitrateParams[rate - 1].period1SymbolSamples : 0;

      // moving average offsets
      bitrate->offsetSignalIndex = SignalBufferLength - bitrate->symbolDelayDetect;
      bitrate->offsetFilterIndex = SignalBufferLength - bitrate->symbolDelayDetect - bitrate->period2SymbolSamples;
      bitrate->offsetSymbolIndex = SignalBufferLength - bitrate->symbolDelayDetect - bitrate->period1SymbolSamples;
      bitrate->offsetDetectIndex = SignalBufferLength - bitrate->symbolDelayDetect - bitrate->period4SymbolSamples;

      // exponential symbol average
      bitrate->symbolAverageW0 = float(1 - 5.0 / bitrate->period1SymbolSamples);
      bitrate->symbolAverageW1 = float(1 - bitrate->symbolAverageW0);

      self->log.info("{} kpbs parameters:", {round(bitrate->symbolsPerSecond / 1E3)});
      self->log.info("\tsymbolsPerSecond     {}", {bitrate->symbolsPerSecond});
      self->log.info("\tperiod1SymbolSamples {} ({} us)", {bitrate->period1SymbolSamples, 1E6 * bitrate->period1SymbolSamples / self->decoder->sampleRate});
      self->log.info("\tperiod2SymbolSamples {} ({} us)", {bitrate->period2SymbolSamples, 1E6 * bitrate->period2SymbolSamples / self->decoder->sampleRate});
      self->log.info("\tperiod4SymbolSamples {} ({} us)", {bitrate->period4SymbolSamples, 1E6 * bitrate->period4SymbolSamples / self->decoder->sampleRate});
      self->log.info("\tperiod8SymbolSamples {} ({} us)", {bitrate->period8SymbolSamples, 1E6 * bitrate->period8SymbolSamples / self->decoder->sampleRate});
      self->log.info("\tsymbolDelayDetect    {} ({} us)", {bitrate->symbolDelayDetect, 1E6 * bitrate->symbolDelayDetect / self->decoder->sampleRate});
      self->log.info("\toffsetSignalIndex    {}", {bitrate->offsetSignalIndex});
      self->log.info("\toffsetFilterIndex    {}", {bitrate->offsetFilterIndex});
      self->log.info("\toffsetSymbolIndex    {}", {bitrate->offsetSymbolIndex});
      self->log.info("\toffsetDetectIndex    {}", {bitrate->offsetDetectIndex});
   }

   // initialize default protocol parameters for start decoding
   self->protocolStatus.maxFrameSize = 256;
   self->protocolStatus.startUpGuardTime = int(self->decoder->signalParams.sampleTimeUnit * 256 * 16 * (1 << 0));
   self->protocolStatus.frameWaitingTime = int(self->decoder->signalParams.sampleTimeUnit * 256 * 16 * (1 << 4));
   self->protocolStatus.frameGuardTime = int(self->decoder->signalParams.sampleTimeUnit * 128 * 7);
   self->protocolStatus.requestGuardTime = int(self->decoder->signalParams.sampleTimeUnit * 7000);

   // initialize frame parameters to default protocol parameters
   self->frameStatus.startUpGuardTime = self->protocolStatus.startUpGuardTime;
   self->frameStatus.frameWaitingTime = self->protocolStatus.frameWaitingTime;
   self->frameStatus.frameGuardTime = self->protocolStatus.frameGuardTime;
   self->frameStatus.requestGuardTime = self->protocolStatus.requestGuardTime;

   // initialize exponential average factors for power value
   self->decoder->signalParams.powerAverageW0 = float(1 - 1E3 / self->decoder->sampleRate);
   self->decoder->signalParams.powerAverageW1 = float(1 - self->decoder->signalParams.powerAverageW0);

   // initialize exponential average factors for signal average
   self->decoder->signalParams.signalAverageW0 = float(1 - 1E5 / self->decoder->sampleRate);
   self->decoder->signalParams.signalAverageW1 = float(1 - self->decoder->signalParams.signalAverageW0);

   // initialize exponential average factors for signal variance
   self->decoder->signalParams.signalVarianceW0 = float(1 - 1E5 / self->decoder->sampleRate);
   self->decoder->signalParams.signalVarianceW1 = float(1 - self->decoder->signalParams.signalVarianceW0);

   self->log.info("Startup parameters");
   self->log.info("\tmaxFrameSize {} bytes", {self->protocolStatus.maxFrameSize});
   self->log.info("\tframeGuardTime {} samples ({} us)", {self->protocolStatus.frameGuardTime, 1000000.0 * self->protocolStatus.frameGuardTime / self->decoder->sampleRate});
   self->log.info("\tframeWaitingTime {} samples ({} us)", {self->protocolStatus.frameWaitingTime, 1000000.0 * self->protocolStatus.frameWaitingTime / self->decoder->sampleRate});
   self->log.info("\trequestGuardTime {} samples ({} us)", {self->protocolStatus.requestGuardTime, 1000000.0 * self->protocolStatus.requestGuardTime / self->decoder->sampleRate});
}

/*
 * Search for NFC-A modulated signal
 */
//bool NfcA::detectModulation(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
//{
//   self->symbolStatus.pattern = PatternType::Invalid;
//
//   while (nextSample(buffer) && self->symbolStatus.pattern == PatternType::Invalid)
//   {
//      // ignore low power signals
//      if (self->decoder->signalStatus.powerAverage > self->decoder->powerLevelThreshold)
//      {
//         // POLL frame ASK detector for  106Kbps, 212Kbps and 424Kbps
//         for (int rate = r106k; rate <= r424k; rate++)
//         {
//            self->decoder->bitrate = self->bitrateParams + rate;
//            self->decoder->modulation = self->modulationStatus + rate;
//
//            // compute signal pointers
//            self->decoder->modulation->signalIndex = (self->decoder->bitrate->offsetSignalIndex + self->decoder->signalClock);
//            self->decoder->modulation->filterIndex = (self->decoder->bitrate->offsetFilterIndex + self->decoder->signalClock);
//
//            // get signal samples
//            float currentData = self->decoder->signalStatus.signalData[self->decoder->modulation->signalIndex & (SignalBufferLength - 1)];
//            float delayedData = self->decoder->signalStatus.signalData[self->decoder->modulation->filterIndex & (SignalBufferLength - 1)];
//
//            // integrate signal data over 1/2 symbol
//            self->decoder->modulation->filterIntegrate += currentData; // add new value
//            self->decoder->modulation->filterIntegrate -= delayedData; // remove delayed value
//
//            // correlation points
//            self->decoder->modulation->filterPoint1 = (self->decoder->modulation->signalIndex % self->decoder->bitrate->period1SymbolSamples);
//            self->decoder->modulation->filterPoint2 = (self->decoder->modulation->signalIndex + self->decoder->bitrate->period2SymbolSamples) % self->decoder->bitrate->period1SymbolSamples;
//            self->decoder->modulation->filterPoint3 = (self->decoder->modulation->signalIndex + self->decoder->bitrate->period1SymbolSamples - 1) % self->decoder->bitrate->period1SymbolSamples;
//
//            // store integrated signal in correlation buffer
//            self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint1] = self->decoder->modulation->filterIntegrate;
//
//            // compute correlation factors
//            self->decoder->modulation->correlatedS0 = self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint1] - self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint2];
//            self->decoder->modulation->correlatedS1 = self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint2] - self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint3];
//            self->decoder->modulation->correlatedSD = std::fabs(self->decoder->modulation->correlatedS0 - self->decoder->modulation->correlatedS1) / float(self->decoder->bitrate->period2SymbolSamples);
//
//            // compute symbol average
//            self->decoder->modulation->symbolAverage = self->decoder->modulation->symbolAverage * self->decoder->bitrate->symbolAverageW0 + currentData * self->decoder->bitrate->symbolAverageW1;
//
//#ifdef DEBUG_ASK_CORRELATION_CHANNEL
//            self->decoder->debug->set(DEBUG_ASK_CORRELATION_CHANNEL, self->decoder->modulation->correlatedSD);
//#endif
//#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
//            self->decoder->debug->set(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.0f);
//#endif
//            // search for Pattern-Z in PCD to PICC request
//            if (self->decoder->modulation->correlatedSD > self->decoder->signalStatus.powerAverage * self->decoder->modulationThreshold)
//            {
//               // calculate symbol self->decoder->modulation deep
//               float modulationDeep = (self->decoder->signalStatus.powerAverage - currentData) / self->decoder->signalStatus.powerAverage;
//
//               if (self->decoder->modulation->searchDeepValue < modulationDeep)
//                  self->decoder->modulation->searchDeepValue = modulationDeep;
//
//               // max correlation peak detector
//               if (self->decoder->modulation->correlatedSD > self->decoder->modulation->correlationPeek)
//               {
//                  self->decoder->modulation->searchPulseWidth++;
//                  self->decoder->modulation->searchPeakTime = self->decoder->signalClock;
//                  self->decoder->modulation->searchEndTime = self->decoder->signalClock + self->decoder->bitrate->period4SymbolSamples;
//                  self->decoder->modulation->correlationPeek = self->decoder->modulation->correlatedSD;
//               }
//            }
//
//            // Check for SoF symbol
//            if (self->decoder->signalClock == self->decoder->modulation->searchEndTime)
//            {
//#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
//               self->decoder->debug->set(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.75f);
//#endif
//               // check self->decoder->modulation deep and Pattern-Z, signaling Start Of Frame (PCD->PICC)
//               if (self->decoder->modulation->searchDeepValue > self->decoder->modulationThreshold)
//               {
//                  // set lower threshold to detect valid response pattern
//                  self->decoder->modulation->searchThreshold = self->decoder->signalStatus.powerAverage * self->decoder->modulationThreshold;
//
//                  // set pattern search window
//                  self->decoder->modulation->symbolStartTime = self->decoder->modulation->searchPeakTime - self->decoder->bitrate->period2SymbolSamples;
//                  self->decoder->modulation->symbolEndTime = self->decoder->modulation->searchPeakTime + self->decoder->bitrate->period2SymbolSamples;
//
//                  // setup frame info
//                  self->frameStatus.frameType = PollFrame;
//                  self->frameStatus.symbolRate = self->decoder->bitrate->symbolsPerSecond;
//                  self->frameStatus.frameStart = self->decoder->modulation->symbolStartTime - self->decoder->bitrate->symbolDelayDetect;
//                  self->frameStatus.frameEnd = 0;
//
//                  // setup symbol info
//                  self->symbolStatus.value = 0;
//                  self->symbolStatus.start = self->decoder->modulation->symbolStartTime - self->decoder->bitrate->symbolDelayDetect;
//                  self->symbolStatus.end = self->decoder->modulation->symbolEndTime - self->decoder->bitrate->symbolDelayDetect;
//                  self->symbolStatus.length = self->symbolStatus.end - self->symbolStatus.start;
//                  self->symbolStatus.pattern = PatternType::PatternZ;
//
//                  break;
//               }
//
//               // reset self->decoder->modulation to continue search
//               self->decoder->modulation->searchStartTime = 0;
//               self->decoder->modulation->searchEndTime = 0;
//               self->decoder->modulation->searchDeepValue = 0;
//               self->decoder->modulation->correlationPeek = 0;
//            }
//         }
//      }
//
//      // carrier edge detector
//      float edge = std::fabs(self->decoder->signalStatus.signalAverage - self->decoder->signalStatus.powerAverage);
//
//      // positive edge
//      if (self->decoder->signalStatus.signalAverage > edge && self->decoder->signalStatus.powerAverage > self->decoder->powerLevelThreshold)
//      {
//         if (!self->decoder->signalStatus.carrierOn)
//         {
//            self->decoder->signalStatus.carrierOn = self->decoder->signalClock;
//
//            if (self->decoder->signalStatus.carrierOff)
//            {
//               NfcFrame silence = NfcFrame(TechType::None, FrameType::NoCarrier);
//
//               silence.setFramePhase(FramePhase::CarrierFrame);
//               silence.setSampleStart(self->decoder->signalStatus.carrierOff);
//               silence.setSampleEnd(self->decoder->signalStatus.carrierOn);
//               silence.setTimeStart(double(self->decoder->signalStatus.carrierOff) / double(self->decoder->sampleRate));
//               silence.setTimeEnd(double(self->decoder->signalStatus.carrierOn) / double(self->decoder->sampleRate));
//
//               frames.push_back(silence);
//            }
//
//            self->decoder->signalStatus.carrierOff = 0;
//         }
//      }
//
//         // negative edge
//      else if (self->decoder->signalStatus.signalAverage < edge || self->decoder->signalStatus.powerAverage < self->decoder->powerLevelThreshold)
//      {
//         if (!self->decoder->signalStatus.carrierOff)
//         {
//            self->decoder->signalStatus.carrierOff = self->decoder->signalClock;
//
//            if (self->decoder->signalStatus.carrierOn)
//            {
//               NfcFrame carrier = NfcFrame(TechType::None, FrameType::EmptyFrame);
//
//               carrier.setFramePhase(FramePhase::CarrierFrame);
//               carrier.setSampleStart(self->decoder->signalStatus.carrierOn);
//               carrier.setSampleEnd(self->decoder->signalStatus.carrierOff);
//               carrier.setTimeStart(double(self->decoder->signalStatus.carrierOn) / double(self->decoder->sampleRate));
//               carrier.setTimeEnd(double(self->decoder->signalStatus.carrierOff) / double(self->decoder->sampleRate));
//
//               frames.push_back(carrier);
//            }
//
//            self->decoder->signalStatus.carrierOn = 0;
//         }
//      }
//   }
//
//   if (self->symbolStatus.pattern != PatternType::Invalid)
//   {
//      // reset modulation to continue search
//      self->decoder->modulation->searchStartTime = 0;
//      self->decoder->modulation->searchEndTime = 0;
//      self->decoder->modulation->searchDeepValue = 0;
//      self->decoder->modulation->correlationPeek = 0;
//
//      // self->decoder->modulation detected
//      return true;
//   }
//
//   // no bitrate detected
//   self->decoder->bitrate = nullptr;
//
//   // no modulation detected
//   self->decoder->modulation = nullptr;
//
//   return false;
//}

bool NfcA::detectModulation()
{
   // ignore low power signals
   if (self->decoder->signalStatus.powerAverage > self->decoder->powerLevelThreshold)
   {
      // POLL frame ASK detector for  106Kbps, 212Kbps and 424Kbps
      for (int rate = r106k; rate <= r424k; rate++)
      {
         BitrateParams *bitrate = self->bitrateParams + rate;
         ModulationStatus *modulation = self->modulationStatus + rate;

         // compute signal pointers
         modulation->signalIndex = (bitrate->offsetSignalIndex + self->decoder->signalClock);
         modulation->filterIndex = (bitrate->offsetFilterIndex + self->decoder->signalClock);

         // get signal samples
         float currentData = self->decoder->signalStatus.signalData[modulation->signalIndex & (SignalBufferLength - 1)];
         float delayedData = self->decoder->signalStatus.signalData[modulation->filterIndex & (SignalBufferLength - 1)];

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += currentData; // add new value
         modulation->filterIntegrate -= delayedData; // remove delayed value

         // correlation points
         modulation->filterPoint1 = (modulation->signalIndex % bitrate->period1SymbolSamples);
         modulation->filterPoint2 = (modulation->signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
         modulation->filterPoint3 = (modulation->signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;

         // store integrated signal in correlation buffer
         modulation->correlationData[modulation->filterPoint1] = modulation->filterIntegrate;

         // compute correlation factors
         modulation->correlatedS0 = modulation->correlationData[modulation->filterPoint1] - modulation->correlationData[modulation->filterPoint2];
         modulation->correlatedS1 = modulation->correlationData[modulation->filterPoint2] - modulation->correlationData[modulation->filterPoint3];
         modulation->correlatedSD = std::fabs(modulation->correlatedS0 - modulation->correlatedS1) / float(bitrate->period2SymbolSamples);

         // compute symbol average
         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + currentData * bitrate->symbolAverageW1;

#ifdef DEBUG_ASK_CORRELATION_CHANNEL
         self->decoder->debug->set(DEBUG_ASK_CORRELATION_CHANNEL, modulation->correlatedSD);
#endif
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
         self->decoder->debug->set(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.0f);
#endif
         // search for Pattern-Z in PCD to PICC request
         if (modulation->correlatedSD > self->decoder->signalStatus.powerAverage * self->decoder->modulationThreshold)
         {
            // calculate symbol modulation deep
            float modulationDeep = (self->decoder->signalStatus.powerAverage - currentData) / self->decoder->signalStatus.powerAverage;

            if (modulation->searchDeepValue < modulationDeep)
               modulation->searchDeepValue = modulationDeep;

            // max correlation peak detector
            if (modulation->correlatedSD > modulation->correlationPeek)
            {
               modulation->searchPulseWidth++;
               modulation->searchPeakTime = self->decoder->signalClock;
               modulation->searchEndTime = self->decoder->signalClock + bitrate->period4SymbolSamples;
               modulation->correlationPeek = modulation->correlatedSD;
            }
         }

         // Check for SoF symbol
         if (self->decoder->signalClock == modulation->searchEndTime)
         {
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
            self->decoder->debug->set(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.75f);
#endif
            // check modulation deep and Pattern-Z, signaling Start Of Frame (PCD->PICC)
            if (modulation->searchDeepValue > self->decoder->modulationThreshold)
            {
               // set lower threshold to detect valid response pattern
               modulation->searchThreshold = self->decoder->signalStatus.powerAverage * self->decoder->modulationThreshold;

               // set pattern search window
               modulation->symbolStartTime = modulation->searchPeakTime - bitrate->period2SymbolSamples;
               modulation->symbolEndTime = modulation->searchPeakTime + bitrate->period2SymbolSamples;

               // setup frame info
               self->frameStatus.frameType = PollFrame;
               self->frameStatus.symbolRate = bitrate->symbolsPerSecond;
               self->frameStatus.frameStart = modulation->symbolStartTime - bitrate->symbolDelayDetect;
               self->frameStatus.frameEnd = 0;

               // setup symbol info
               self->symbolStatus.value = 0;
               self->symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
               self->symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
               self->symbolStatus.length = self->symbolStatus.end - self->symbolStatus.start;
               self->symbolStatus.pattern = PatternType::PatternZ;

               // reset modulation to continue search
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;
               modulation->searchDeepValue = 0;
               modulation->correlationPeek = 0;

               // modulation detected
               self->decoder->bitrate = bitrate;
               self->decoder->modulation = modulation;

               return true;
            }

            // reset modulation to continue search
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->searchDeepValue = 0;
            modulation->correlationPeek = 0;
         }
      }
   }

   return false;
}

/*
 * Decode next poll or listen frame
 */
void NfcA::decodeFrame(sdr::SignalBuffer &samples, std::list<NfcFrame> &frames)
{
   if (self->frameStatus.frameType == PollFrame)
   {
      decodePollFrame(samples, frames);
   }

   if (self->frameStatus.frameType == ListenFrame)
   {
      decodeListenFrame(samples, frames);
   }
}

/*
 * Decode next poll frame
 */
bool NfcA::decodePollFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   int pattern;

   // read NFC-A request request
   while ((pattern = decodePollFrameSymbolAsk(buffer)) > PatternType::NoPattern)
   {
      self->streamStatus.pattern = pattern;

      // detect end of request (Pattern-Y after Pattern-Z)
      if ((self->streamStatus.pattern == PatternType::PatternY && (self->streamStatus.previous == PatternType::PatternY || self->streamStatus.previous == PatternType::PatternZ)) || self->streamStatus.bytes == self->protocolStatus.maxFrameSize)
      {
         // frames must contains at least one full byte or 7 bits for short frames
         if (self->streamStatus.bytes > 0 || self->streamStatus.bits == 7)
         {
            // add remaining byte to request
            if (self->streamStatus.bits >= 7)
               self->streamStatus.buffer[self->streamStatus.bytes++] = self->streamStatus.data;

            // set last symbol timing
            if (self->streamStatus.previous == PatternType::PatternZ)
               self->frameStatus.frameEnd = self->symbolStatus.start - self->decoder->bitrate->period2SymbolSamples;
            else
               self->frameStatus.frameEnd = self->symbolStatus.start - self->decoder->bitrate->period1SymbolSamples;

            // build request frame
            NfcFrame request = NfcFrame(TechType::NfcA, FrameType::PollFrame);

            request.setFrameRate(self->frameStatus.symbolRate);
            request.setSampleStart(self->frameStatus.frameStart);
            request.setSampleEnd(self->frameStatus.frameEnd);
            request.setTimeStart(double(self->frameStatus.frameStart) / double(self->decoder->sampleRate));
            request.setTimeEnd(double(self->frameStatus.frameEnd) / double(self->decoder->sampleRate));

            if (self->streamStatus.flags & ParityError)
               request.setFrameFlags(FrameFlags::ParityError);

            if (self->streamStatus.bytes == self->protocolStatus.maxFrameSize)
               request.setFrameFlags(FrameFlags::Truncated);

            if (self->streamStatus.bytes == 1 && self->streamStatus.bits == 7)
               request.setFrameFlags(FrameFlags::ShortFrame);

            // add bytes to frame and flip to prepare read
            request.put(self->streamStatus.buffer, self->streamStatus.bytes).flip();

            // clear self->decoder->modulation status for next frame search
            self->decoder->modulation->symbolStartTime = 0;
            self->decoder->modulation->symbolEndTime = 0;
            self->decoder->modulation->filterIntegrate = 0;
            self->decoder->modulation->phaseIntegrate = 0;

            // clear stream status
            self->streamStatus = {0,};

            // process frame
            process(request);

            // add to frame list
            frames.push_back(request);

            // return request frame data
            return true;
         }

         // reset self->decoder->modulation and restart frame detection
         resetModulation();

         // no valid frame found
         return false;
      }

      if (self->streamStatus.previous)
      {
         int value = (self->streamStatus.previous == PatternType::PatternX);

         // decode next bit
         if (self->streamStatus.bits < 8)
         {
            self->streamStatus.data = self->streamStatus.data | (value << self->streamStatus.bits++);
         }

            // store full byte in stream buffer and check parity
         else if (self->streamStatus.bytes < self->protocolStatus.maxFrameSize)
         {
            self->streamStatus.buffer[self->streamStatus.bytes++] = self->streamStatus.data;
            self->streamStatus.flags |= !checkParity(self->streamStatus.data, value) ? ParityError : 0;
            self->streamStatus.data = self->streamStatus.bits = 0;
         }

            // too many bytes in frame, abort decoder
         else
         {
            // reset self->decoder->modulation status
            resetModulation();

            // no valid frame found
            return false;
         }
      }

      // update previous command state
      self->streamStatus.previous = self->streamStatus.pattern;
   }

   // no frame detected
   return false;
}

/*
 * Decode next listen frame
 */
bool NfcA::decodeListenFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   int pattern;

   // decode TAG ASK response
   if (self->decoder->bitrate->rateType == r106k)
   {
      if (!self->frameStatus.frameStart)
      {
         // search Start Of Frame pattern
         pattern = decodeListenFrameSymbolAsk(buffer);

         // Pattern-D found, mark frame start time
         if (pattern == PatternType::PatternD)
         {
            self->frameStatus.frameStart = self->symbolStatus.start;
         }
         else
         {
            //  end of frame waiting time, restart self->decoder->modulation search
            if (pattern == PatternType::NoPattern)
               resetModulation();

            // no frame found
            return NfcFrame::Nil;
         }
      }

      if (self->frameStatus.frameStart)
      {
         // decode remaining response
         while ((pattern = decodeListenFrameSymbolAsk(buffer)) > PatternType::NoPattern)
         {
            // detect end of response for ASK
            if (pattern == PatternType::PatternF || self->streamStatus.bytes == self->protocolStatus.maxFrameSize)
            {
               // a valid response must contains at least 4 bits of data
               if (self->streamStatus.bytes > 0 || self->streamStatus.bits == 4)
               {
                  // add remaining byte to request
                  if (self->streamStatus.bits == 4)
                     self->streamStatus.buffer[self->streamStatus.bytes++] = self->streamStatus.data;

                  self->frameStatus.frameEnd = self->symbolStatus.end;

                  NfcFrame response = NfcFrame(TechType::NfcA, FrameType::ListenFrame);

                  response.setFrameRate(self->decoder->bitrate->symbolsPerSecond);
                  response.setSampleStart(self->frameStatus.frameStart);
                  response.setSampleEnd(self->frameStatus.frameEnd);
                  response.setTimeStart(double(self->frameStatus.frameStart) / double(self->decoder->sampleRate));
                  response.setTimeEnd(double(self->frameStatus.frameEnd) / double(self->decoder->sampleRate));

                  if (self->streamStatus.flags & ParityError)
                     response.setFrameFlags(FrameFlags::ParityError);

                  if (self->streamStatus.bytes == self->protocolStatus.maxFrameSize)
                     response.setFrameFlags(FrameFlags::Truncated);

                  if (self->streamStatus.bytes == 1 && self->streamStatus.bits == 4)
                     response.setFrameFlags(FrameFlags::ShortFrame);

                  // add bytes to frame and flip to prepare read
                  response.put(self->streamStatus.buffer, self->streamStatus.bytes).flip();

                  // reset self->decoder->modulation status
                  resetModulation();

                  // process frame
                  process(response);

                  // add to frame list
                  frames.push_back(response);

                  return true;
               }

               // only detect first pattern-D without anymore, so can be spurious pulse, we try to find SoF again
               resetFrameSearch();

               // no valid frame found
               return false;
            }

            // decode next bit
            if (self->streamStatus.bits < 8)
            {
               self->streamStatus.data |= (self->symbolStatus.value << self->streamStatus.bits++);
            }

               // store full byte in stream buffer and check parity
            else if (self->streamStatus.bytes < self->protocolStatus.maxFrameSize)
            {
               self->streamStatus.buffer[self->streamStatus.bytes++] = self->streamStatus.data;
               self->streamStatus.flags |= !checkParity(self->streamStatus.data, self->symbolStatus.value) ? ParityError : 0;
               self->streamStatus.data = self->streamStatus.bits = 0;
            }

               // too many bytes in frame, abort decoder
            else
            {
               // reset self->decoder->modulation status
               resetModulation();

               // no valid frame found
               return false;
            }
         }
      }
   }

      // decode TAG BPSK response
   else if (self->decoder->bitrate->rateType == r212k || self->decoder->bitrate->rateType == r424k)
   {
      if (!self->frameStatus.frameStart)
      {
         // detect first pattern
         pattern = decodeListenFrameSymbolBpsk(buffer);

         // Pattern-M found, mark frame start time
         if (pattern == PatternType::PatternM)
         {
            self->frameStatus.frameStart = self->symbolStatus.start;
         }
         else
         {
            //  end of frame waiting time, restart self->decoder->modulation search
            if (pattern == PatternType::NoPattern)
               resetModulation();

            // no frame found
            return false;
         }
      }

      // frame SoF detected, decode frame stream...
      if (self->frameStatus.frameStart)
      {
         while ((pattern = decodeListenFrameSymbolBpsk(buffer)) > PatternType::NoPattern)
         {
            // detect end of response for BPSK
            if (pattern == PatternType::PatternO)
            {
               if (self->streamStatus.bits == 9)
               {
                  // store byte in stream buffer
                  self->streamStatus.buffer[self->streamStatus.bytes++] = self->streamStatus.data;

                  // last byte has even parity
                  self->streamStatus.flags |= checkParity(self->streamStatus.data, self->streamStatus.parity) ? ParityError : 0;
               }

               // frames must contains at least one full byte
               if (self->streamStatus.bytes > 0)
               {
                  // mark frame end at star of EoF symbol
                  self->frameStatus.frameEnd = self->symbolStatus.start;

                  // build responde frame
                  NfcFrame response = NfcFrame(TechType::NfcA, FrameType::ListenFrame);

                  response.setFrameRate(self->decoder->bitrate->symbolsPerSecond);
                  response.setSampleStart(self->frameStatus.frameStart);
                  response.setSampleEnd(self->frameStatus.frameEnd);
                  response.setTimeStart(double(self->frameStatus.frameStart) / double(self->decoder->sampleRate));
                  response.setTimeEnd(double(self->frameStatus.frameEnd) / double(self->decoder->sampleRate));

                  if (self->streamStatus.flags & ParityError)
                     response.setFrameFlags(FrameFlags::ParityError);

                  if (self->streamStatus.bytes == self->protocolStatus.maxFrameSize)
                     response.setFrameFlags(FrameFlags::Truncated);

                  // add bytes to frame and flip to prepare read
                  response.put(self->streamStatus.buffer, self->streamStatus.bytes).flip();

                  // reset self->decoder->modulation status
                  resetModulation();

                  // process frame
                  process(response);

                  // add to frame list
                  frames.push_back(response);

                  return true;
               }

               // reset self->decoder->modulation status
               resetModulation();

               // no valid frame found
               return false;
            }

            // decode next data bit
            if (self->streamStatus.bits < 8)
            {
               self->streamStatus.data |= (self->symbolStatus.value << self->streamStatus.bits);
            }

               // decode parity bit
            else if (self->streamStatus.bits < 9)
            {
               self->streamStatus.parity = self->symbolStatus.value;
            }

               // store full byte in stream buffer and check parity
            else if (self->streamStatus.bytes < self->protocolStatus.maxFrameSize)
            {
               // store byte in stream buffer
               self->streamStatus.buffer[self->streamStatus.bytes++] = self->streamStatus.data;

               // frame bytes has odd parity
               self->streamStatus.flags |= !checkParity(self->streamStatus.data, self->streamStatus.parity) ? ParityError : 0;

               // initialize next value from current symbol
               self->streamStatus.data = self->symbolStatus.value;

               // reset bit counter
               self->streamStatus.bits = 0;
            }

               // too many bytes in frame, abort decoder
            else
            {
               // reset self->decoder->modulation status
               resetModulation();

               // no valid frame found
               return false;
            }

            self->streamStatus.bits++;
         }
      }
   }

   // end of stream...
   return false;
}

/*
 * Decode one ASK modulated poll frame symbol
 */
int NfcA::decodePollFrameSymbolAsk(sdr::SignalBuffer &buffer)
{
   self->symbolStatus.pattern = PatternType::Invalid;

   while (nextSample(buffer))
   {
      // compute pointers
      self->decoder->modulation->signalIndex = (self->decoder->bitrate->offsetSignalIndex + self->decoder->signalClock);
      self->decoder->modulation->filterIndex = (self->decoder->bitrate->offsetFilterIndex + self->decoder->signalClock);

      // get signal samples
      float currentData = self->decoder->signalStatus.signalData[self->decoder->modulation->signalIndex & (SignalBufferLength - 1)];
      float delayedData = self->decoder->signalStatus.signalData[self->decoder->modulation->filterIndex & (SignalBufferLength - 1)];

      // integrate signal data over 1/2 symbol
      self->decoder->modulation->filterIntegrate += currentData; // add new value
      self->decoder->modulation->filterIntegrate -= delayedData; // remove delayed value

      // correlation pointers
      self->decoder->modulation->filterPoint1 = (self->decoder->modulation->signalIndex % self->decoder->bitrate->period1SymbolSamples);
      self->decoder->modulation->filterPoint2 = (self->decoder->modulation->signalIndex + self->decoder->bitrate->period2SymbolSamples) % self->decoder->bitrate->period1SymbolSamples;
      self->decoder->modulation->filterPoint3 = (self->decoder->modulation->signalIndex + self->decoder->bitrate->period1SymbolSamples - 1) % self->decoder->bitrate->period1SymbolSamples;

      // store integrated signal in correlation buffer
      self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint1] = self->decoder->modulation->filterIntegrate;

      // compute correlation factors
      self->decoder->modulation->correlatedS0 = self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint1] - self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint2];
      self->decoder->modulation->correlatedS1 = self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint2] - self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint3];
      self->decoder->modulation->correlatedSD = std::fabs(self->decoder->modulation->correlatedS0 - self->decoder->modulation->correlatedS1) / float(self->decoder->bitrate->period2SymbolSamples);

#ifdef DEBUG_ASK_CORRELATION_CHANNEL
      self->decoder->debug->set(DEBUG_ASK_CORRELATION_CHANNEL, self->decoder->modulation->correlatedSD);
#endif

#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
      self->decoder->debug->set(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.0f);
#endif
      // compute symbol average
      self->decoder->modulation->symbolAverage = self->decoder->modulation->symbolAverage * self->decoder->bitrate->symbolAverageW0 + currentData * self->decoder->bitrate->symbolAverageW1;

      // set next search sync window from previous state
      if (!self->decoder->modulation->searchStartTime)
      {
         // estimated symbol start and end
         self->decoder->modulation->symbolStartTime = self->decoder->modulation->symbolEndTime;
         self->decoder->modulation->symbolEndTime = self->decoder->modulation->symbolStartTime + self->decoder->bitrate->period1SymbolSamples;

         // timig search window
         self->decoder->modulation->searchStartTime = self->decoder->modulation->symbolEndTime - self->decoder->bitrate->period8SymbolSamples;
         self->decoder->modulation->searchEndTime = self->decoder->modulation->symbolEndTime + self->decoder->bitrate->period8SymbolSamples;

         // reset symbol parameters
         self->decoder->modulation->symbolCorr0 = 0;
         self->decoder->modulation->symbolCorr1 = 0;
      }

      // search max correlation peak
      if (self->decoder->signalClock >= self->decoder->modulation->searchStartTime && self->decoder->signalClock <= self->decoder->modulation->searchEndTime)
      {
         if (self->decoder->modulation->correlatedSD > self->decoder->modulation->correlationPeek)
         {
            self->decoder->modulation->correlationPeek = self->decoder->modulation->correlatedSD;
            self->decoder->modulation->symbolCorr0 = self->decoder->modulation->correlatedS0;
            self->decoder->modulation->symbolCorr1 = self->decoder->modulation->correlatedS1;
            self->decoder->modulation->symbolEndTime = self->decoder->signalClock;
         }
      }

      // capture next symbol
      if (self->decoder->signalClock == self->decoder->modulation->searchEndTime)
      {
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
         self->decoder->debug->set(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.50f);
#endif
         // detect Pattern-Y when no self->decoder->modulation occurs (below search detection threshold)
         if (self->decoder->modulation->correlationPeek < self->decoder->modulation->searchThreshold)
         {
            // estimate symbol end from start (peak detection not valid due lack of self->decoder->modulation)
            self->decoder->modulation->symbolEndTime = self->decoder->modulation->symbolStartTime + self->decoder->bitrate->period1SymbolSamples;

            // setup symbol info
            self->symbolStatus.value = 1;
            self->symbolStatus.start = self->decoder->modulation->symbolStartTime - self->decoder->bitrate->symbolDelayDetect;
            self->symbolStatus.end = self->decoder->modulation->symbolEndTime - self->decoder->bitrate->symbolDelayDetect;
            self->symbolStatus.length = self->symbolStatus.end - self->symbolStatus.start;
            self->symbolStatus.pattern = PatternType::PatternY;

            break;
         }

         // detect Pattern-Z
         if (self->decoder->modulation->symbolCorr0 > self->decoder->modulation->symbolCorr1)
         {
            // setup symbol info
            self->symbolStatus.value = 0;
            self->symbolStatus.start = self->decoder->modulation->symbolStartTime - self->decoder->bitrate->symbolDelayDetect;
            self->symbolStatus.end = self->decoder->modulation->symbolEndTime - self->decoder->bitrate->symbolDelayDetect;
            self->symbolStatus.length = self->symbolStatus.end - self->symbolStatus.start;
            self->symbolStatus.pattern = PatternType::PatternZ;

            break;
         }

         // detect Pattern-X, setup symbol info
         self->symbolStatus.value = 1;
         self->symbolStatus.start = self->decoder->modulation->symbolStartTime - self->decoder->bitrate->symbolDelayDetect;
         self->symbolStatus.end = self->decoder->modulation->symbolEndTime - self->decoder->bitrate->symbolDelayDetect;
         self->symbolStatus.length = self->symbolStatus.end - self->symbolStatus.start;
         self->symbolStatus.pattern = PatternType::PatternX;

         break;
      }
   }

   // reset search status if symbol has detected
   if (self->symbolStatus.pattern != PatternType::Invalid)
   {
      self->decoder->modulation->searchStartTime = 0;
      self->decoder->modulation->searchEndTime = 0;
      self->decoder->modulation->searchPulseWidth = 0;
      self->decoder->modulation->correlationPeek = 0;
      self->decoder->modulation->correlatedSD = 0;
   }

   return self->symbolStatus.pattern;
}

/*
 * Decode one ASK modulated listen frame symbol
 */
int NfcA::decodeListenFrameSymbolAsk(sdr::SignalBuffer &buffer)
{
   int pattern = PatternType::Invalid;

   while (nextSample(buffer))
   {
      // compute pointers
      self->decoder->modulation->signalIndex = (self->decoder->bitrate->offsetSignalIndex + self->decoder->signalClock);
      self->decoder->modulation->detectIndex = (self->decoder->bitrate->offsetDetectIndex + self->decoder->signalClock);

      // get signal samples
      float currentData = self->decoder->signalStatus.signalData[self->decoder->modulation->signalIndex & (SignalBufferLength - 1)];

      // compute symbol average (signal offset)
      self->decoder->modulation->symbolAverage = self->decoder->modulation->symbolAverage * self->decoder->bitrate->symbolAverageW0 + currentData * self->decoder->bitrate->symbolAverageW1;

      // signal value
      currentData -= self->decoder->modulation->symbolAverage;

      // store signal square in filter buffer
      self->decoder->modulation->integrationData[self->decoder->modulation->signalIndex & (SignalBufferLength - 1)] = currentData * currentData;

      // start correlation after frameGuardTime
      if (self->decoder->signalClock > (self->frameStatus.guardEnd - self->decoder->bitrate->period1SymbolSamples))
      {
         // compute correlation points
         self->decoder->modulation->filterPoint1 = (self->decoder->modulation->signalIndex % self->decoder->bitrate->period1SymbolSamples);
         self->decoder->modulation->filterPoint2 = (self->decoder->modulation->signalIndex + self->decoder->bitrate->period2SymbolSamples) % self->decoder->bitrate->period1SymbolSamples;
         self->decoder->modulation->filterPoint3 = (self->decoder->modulation->signalIndex + self->decoder->bitrate->period1SymbolSamples - 1) % self->decoder->bitrate->period1SymbolSamples;

         // integrate symbol (moving average)
         self->decoder->modulation->filterIntegrate += self->decoder->modulation->integrationData[self->decoder->modulation->signalIndex & (SignalBufferLength - 1)]; // add new value
         self->decoder->modulation->filterIntegrate -= self->decoder->modulation->integrationData[self->decoder->modulation->detectIndex & (SignalBufferLength - 1)]; // remove delayed value

         // store integrated signal in correlation buffer
         self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint1] = self->decoder->modulation->filterIntegrate;

         // compute correlation results for each symbol and distance
         self->decoder->modulation->correlatedS0 = self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint1] - self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint2];
         self->decoder->modulation->correlatedS1 = self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint2] - self->decoder->modulation->correlationData[self->decoder->modulation->filterPoint3];
         self->decoder->modulation->correlatedSD = std::fabs(self->decoder->modulation->correlatedS0 - self->decoder->modulation->correlatedS1);
      }

#ifdef DEBUG_ASK_CORRELATION_CHANNEL
      self->decoder->debug->set(DEBUG_ASK_CORRELATION_CHANNEL, self->decoder->modulation->correlatedSD);
#endif

#ifdef DEBUG_ASK_INTEGRATION_CHANNEL
      self->decoder->debug->set(DEBUG_ASK_INTEGRATION_CHANNEL, self->decoder->modulation->filterIntegrate);
#endif

#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
      self->decoder->debug->set(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.0f);
#endif

      // search for Start Of Frame pattern (SoF)
      if (!self->decoder->modulation->symbolEndTime)
      {
         if (self->decoder->signalClock > self->frameStatus.guardEnd)
         {
            if (self->decoder->modulation->correlatedSD > self->decoder->modulation->searchThreshold)
            {
               // max correlation peak detector
               if (self->decoder->modulation->correlatedSD > self->decoder->modulation->correlationPeek)
               {
                  self->decoder->modulation->searchPulseWidth++;
                  self->decoder->modulation->searchPeakTime = self->decoder->signalClock;
                  self->decoder->modulation->searchEndTime = self->decoder->signalClock + self->decoder->bitrate->period4SymbolSamples;
                  self->decoder->modulation->correlationPeek = self->decoder->modulation->correlatedSD;
               }
            }

            // Check for SoF symbol
            if (self->decoder->signalClock == self->decoder->modulation->searchEndTime)
            {
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
               self->decoder->debug->set(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.75f);
#endif
               if (self->decoder->modulation->searchPulseWidth > self->decoder->bitrate->period8SymbolSamples)
               {
                  // set pattern search window
                  self->decoder->modulation->symbolStartTime = self->decoder->modulation->searchPeakTime - self->decoder->bitrate->period2SymbolSamples;
                  self->decoder->modulation->symbolEndTime = self->decoder->modulation->searchPeakTime + self->decoder->bitrate->period2SymbolSamples;

                  // setup symbol info
                  self->symbolStatus.value = 1;
                  self->symbolStatus.start = self->decoder->modulation->symbolStartTime - self->decoder->bitrate->symbolDelayDetect;
                  self->symbolStatus.end = self->decoder->modulation->symbolEndTime - self->decoder->bitrate->symbolDelayDetect;
                  self->symbolStatus.length = self->symbolStatus.end - self->symbolStatus.start;

                  pattern = PatternType::PatternD;
                  break;
               }

               // reset search status
               self->decoder->modulation->searchStartTime = 0;
               self->decoder->modulation->searchEndTime = 0;
               self->decoder->modulation->correlationPeek = 0;
               self->decoder->modulation->searchPulseWidth = 0;
               self->decoder->modulation->correlatedSD = 0;
            }
         }

         // capture signal variance as lower level threshold
         if (self->decoder->signalClock == self->frameStatus.guardEnd)
            self->decoder->modulation->searchThreshold = self->decoder->signalStatus.signalVariance;

         // frame waiting time exceeded
         if (self->decoder->signalClock == self->frameStatus.waitingEnd)
         {
            pattern = PatternType::NoPattern;
            break;
         }
      }

         // search Response Bit Stream
      else
      {
         // set next search sync window from previous
         if (!self->decoder->modulation->searchStartTime)
         {
            // estimated symbol start and end
            self->decoder->modulation->symbolStartTime = self->decoder->modulation->symbolEndTime;
            self->decoder->modulation->symbolEndTime = self->decoder->modulation->symbolStartTime + self->decoder->bitrate->period1SymbolSamples;

            // timig search window
            self->decoder->modulation->searchStartTime = self->decoder->modulation->symbolEndTime - self->decoder->bitrate->period8SymbolSamples;
            self->decoder->modulation->searchEndTime = self->decoder->modulation->symbolEndTime + self->decoder->bitrate->period8SymbolSamples;

            // reset symbol parameters
            self->decoder->modulation->symbolCorr0 = 0;
            self->decoder->modulation->symbolCorr1 = 0;
         }

         // search symbol timings
         if (self->decoder->signalClock >= self->decoder->modulation->searchStartTime && self->decoder->signalClock <= self->decoder->modulation->searchEndTime)
         {
            if (self->decoder->modulation->correlatedSD > self->decoder->modulation->correlationPeek)
            {
               self->decoder->modulation->correlationPeek = self->decoder->modulation->correlatedSD;
               self->decoder->modulation->symbolCorr0 = self->decoder->modulation->correlatedS0;
               self->decoder->modulation->symbolCorr1 = self->decoder->modulation->correlatedS1;
               self->decoder->modulation->symbolEndTime = self->decoder->signalClock;
            }
         }

         // capture next symbol
         if (self->decoder->signalClock == self->decoder->modulation->searchEndTime)
         {
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
            self->decoder->debug->set(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.50f);
#endif
            if (self->decoder->modulation->correlationPeek > self->decoder->modulation->searchThreshold)
            {
               // setup symbol info
               self->symbolStatus.start = self->decoder->modulation->symbolStartTime - self->decoder->bitrate->symbolDelayDetect;
               self->symbolStatus.end = self->decoder->modulation->symbolEndTime - self->decoder->bitrate->symbolDelayDetect;
               self->symbolStatus.length = self->symbolStatus.end - self->symbolStatus.start;

               if (self->decoder->modulation->symbolCorr0 > self->decoder->modulation->symbolCorr1)
               {
                  self->symbolStatus.value = 0;
                  pattern = PatternType::PatternE;
                  break;
               }

               self->symbolStatus.value = 1;
               pattern = PatternType::PatternD;
               break;
            }

            // no self->decoder->modulation (End Of Frame) EoF
            pattern = PatternType::PatternF;
            break;
         }
      }
   }

   // reset search status
   if (pattern != PatternType::Invalid)
   {
      self->symbolStatus.pattern = pattern;

      self->decoder->modulation->searchStartTime = 0;
      self->decoder->modulation->searchEndTime = 0;
      self->decoder->modulation->correlationPeek = 0;
      self->decoder->modulation->searchPulseWidth = 0;
      self->decoder->modulation->correlatedSD = 0;
   }

   return pattern;
}

/*
 * Decode one BPSK modulated listen frame symbol
 */
int NfcA::decodeListenFrameSymbolBpsk(sdr::SignalBuffer &buffer)
{
   int pattern = PatternType::Invalid;

   while (nextSample(buffer))
   {
      self->decoder->modulation->signalIndex = (self->decoder->bitrate->offsetSignalIndex + self->decoder->signalClock);
      self->decoder->modulation->symbolIndex = (self->decoder->bitrate->offsetSymbolIndex + self->decoder->signalClock);
      self->decoder->modulation->detectIndex = (self->decoder->bitrate->offsetDetectIndex + self->decoder->signalClock);

      // get signal samples
      float currentSample = self->decoder->signalStatus.signalData[self->decoder->modulation->signalIndex & (SignalBufferLength - 1)];
      float delayedSample = self->decoder->signalStatus.signalData[self->decoder->modulation->symbolIndex & (SignalBufferLength - 1)];

      // compute symbol average
      self->decoder->modulation->symbolAverage = self->decoder->modulation->symbolAverage * self->decoder->bitrate->symbolAverageW0 + currentSample * self->decoder->bitrate->symbolAverageW1;

      // multiply 1 symbol delayed signal with incoming signal
      float phase = (currentSample - self->decoder->modulation->symbolAverage) * (delayedSample - self->decoder->modulation->symbolAverage);

      // store signal phase in filter buffer
      self->decoder->modulation->integrationData[self->decoder->modulation->signalIndex & (SignalBufferLength - 1)] = phase * 10;

      // integrate response from PICC after guard time (TR0)
      if (self->decoder->signalClock > (self->frameStatus.guardEnd - self->decoder->bitrate->period1SymbolSamples))
      {
         self->decoder->modulation->phaseIntegrate += self->decoder->modulation->integrationData[self->decoder->modulation->signalIndex & (SignalBufferLength - 1)]; // add new value
         self->decoder->modulation->phaseIntegrate -= self->decoder->modulation->integrationData[self->decoder->modulation->detectIndex & (SignalBufferLength - 1)]; // remove delayed value
      }

#ifdef DEBUG_BPSK_PHASE_INTEGRATION_CHANNEL
      self->decoder->debug->set(DEBUG_BPSK_PHASE_INTEGRATION_CHANNEL, self->decoder->modulation->phaseIntegrate);
#endif

#ifdef DEBUG_BPSK_PHASE_DEMODULATION_CHANNEL
      self->decoder->debug->set(DEBUG_BPSK_PHASE_DEMODULATION_CHANNEL, phase * 10);
#endif
      // search for Start Of Frame pattern (SoF)
      if (!self->decoder->modulation->symbolEndTime)
      {
         // detect first zero-cross
         if (self->decoder->modulation->phaseIntegrate > 0.00025f)
         {
            self->decoder->modulation->searchPeakTime = self->decoder->signalClock;
            self->decoder->modulation->searchEndTime = self->decoder->signalClock + self->decoder->bitrate->period2SymbolSamples;
         }

         if (self->decoder->signalClock == self->decoder->modulation->searchEndTime)
         {
#ifdef DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL
            self->decoder->debug->set(DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL, 0.75);
#endif
            // set symbol window
            self->decoder->modulation->symbolStartTime = self->decoder->modulation->searchPeakTime;
            self->decoder->modulation->symbolEndTime = self->decoder->modulation->searchPeakTime + self->decoder->bitrate->period1SymbolSamples;
            self->decoder->modulation->symbolPhase = self->decoder->modulation->phaseIntegrate;
            self->decoder->modulation->phaseThreshold = std::fabs(self->decoder->modulation->phaseIntegrate / 3);

            // set symbol info
            self->symbolStatus.value = 0;
            self->symbolStatus.start = self->decoder->modulation->symbolStartTime - self->decoder->bitrate->symbolDelayDetect;
            self->symbolStatus.end = self->decoder->modulation->symbolEndTime - self->decoder->bitrate->symbolDelayDetect;
            self->symbolStatus.length = self->symbolStatus.end - self->symbolStatus.start;

            pattern = PatternType::PatternM;
            break;
         }

            // frame waiting time exceeded
         else if (self->decoder->signalClock == self->frameStatus.waitingEnd)
         {
            pattern = PatternType::NoPattern;
            break;
         }
      }

         // search Response Bit Stream
      else
      {
         // edge detector for re-synchronization
         if ((self->decoder->modulation->phaseIntegrate > 0 && self->decoder->modulation->symbolPhase < 0) || (self->decoder->modulation->phaseIntegrate < 0 && self->decoder->modulation->symbolPhase > 0))
         {
            self->decoder->modulation->searchPeakTime = self->decoder->signalClock;
            self->decoder->modulation->searchEndTime = self->decoder->signalClock + self->decoder->bitrate->period2SymbolSamples;
            self->decoder->modulation->symbolStartTime = self->decoder->signalClock;
            self->decoder->modulation->symbolEndTime = self->decoder->signalClock + self->decoder->bitrate->period1SymbolSamples;
            self->decoder->modulation->symbolPhase = self->decoder->modulation->phaseIntegrate;
         }

         // set next search sync window from previous
         if (!self->decoder->modulation->searchEndTime)
         {
            // estimated symbol start and end
            self->decoder->modulation->symbolStartTime = self->decoder->modulation->symbolEndTime;
            self->decoder->modulation->symbolEndTime = self->decoder->modulation->symbolStartTime + self->decoder->bitrate->period1SymbolSamples;

            // timig next symbol
            self->decoder->modulation->searchEndTime = self->decoder->modulation->symbolStartTime + self->decoder->bitrate->period2SymbolSamples;
         }

            // search symbol timings
         else if (self->decoder->signalClock == self->decoder->modulation->searchEndTime)
         {
#ifdef DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL
            self->decoder->debug->set(DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL, 0.5);
#endif
            self->decoder->modulation->symbolPhase = self->decoder->modulation->phaseIntegrate;

            // setup symbol info
            self->symbolStatus.start = self->decoder->modulation->symbolStartTime - self->decoder->bitrate->symbolDelayDetect;
            self->symbolStatus.end = self->decoder->modulation->symbolEndTime - self->decoder->bitrate->symbolDelayDetect;
            self->symbolStatus.length = self->symbolStatus.end - self->symbolStatus.start;

            // no symbol change, keep previous symbol pattern
            if (self->decoder->modulation->phaseIntegrate > self->decoder->modulation->phaseThreshold)
            {
               pattern = self->symbolStatus.pattern;
               break;
            }

            // symbol change, invert pattern and value
            if (self->decoder->modulation->phaseIntegrate < -self->decoder->modulation->phaseThreshold)
            {
               self->symbolStatus.value = !self->symbolStatus.value;
               pattern = (self->symbolStatus.pattern == PatternType::PatternM) ? PatternType::PatternN : PatternType::PatternM;
               break;
            }

            // no self->decoder->modulation detected, generate End Of Frame symbol
            pattern = PatternType::PatternO;
            break;
         }
      }
   }

   // reset search status
   if (pattern != PatternType::Invalid)
   {
      self->symbolStatus.pattern = pattern;

      self->decoder->modulation->searchStartTime = 0;
      self->decoder->modulation->searchEndTime = 0;
      self->decoder->modulation->correlationPeek = 0;
      self->decoder->modulation->searchPulseWidth = 0;
      self->decoder->modulation->correlatedSD = 0;
   }

   return pattern;
}

/*
 * Reset frame search status
 */
void NfcA::resetFrameSearch()
{
   // reset frame search status
   if (self->decoder->modulation)
   {
      self->decoder->modulation->symbolEndTime = 0;
      self->decoder->modulation->searchPeakTime = 0;
      self->decoder->modulation->searchEndTime = 0;
      self->decoder->modulation->correlationPeek = 0;
   }

   // reset frame start time
   self->frameStatus.frameStart = 0;
}

/*
 * Reset modulation status
 */
void NfcA::resetModulation()
{
   // reset self->decoder->modulation detection for all rates
   for (int rate = r106k; rate <= r424k; rate++)
   {
      self->modulationStatus[rate].searchStartTime = 0;
      self->modulationStatus[rate].searchEndTime = 0;
      self->modulationStatus[rate].correlationPeek = 0;
      self->modulationStatus[rate].searchPulseWidth = 0;
      self->modulationStatus[rate].searchDeepValue = 0;
      self->modulationStatus[rate].symbolAverage = 0;
      self->modulationStatus[rate].symbolPhase = NAN;
   }

   // clear stream status
   self->streamStatus = {0,};

   // clear stream status
   self->symbolStatus = {0,};

   // clear frame status
   self->frameStatus.frameType = 0;
   self->frameStatus.frameStart = 0;
   self->frameStatus.frameEnd = 0;

   // restore bitrate
   self->decoder->bitrate = nullptr;

   // restore modulation
   self->decoder->modulation = nullptr;
}

/*
 * Process next signal sample
 */
bool NfcA::nextSample(sdr::SignalBuffer &buffer)
{
   if (buffer.available() == 0)
      return false;

   // real-value signal
   if (buffer.stride() == 1)
   {
      // read next sample data
      buffer.get(self->decoder->signalStatus.signalValue);
   }

      // IQ channel signal
   else
   {
      // read next sample data
      buffer.get(self->decoder->signalStatus.sampleData, 2);

      // compute magnitude from IQ channels
      auto i = double(self->decoder->signalStatus.sampleData[0]);
      auto q = double(self->decoder->signalStatus.sampleData[1]);

      self->decoder->signalStatus.signalValue = sqrtf(i * i + q * q);
   }

   // update signal clock
   self->decoder->signalClock++;

   // compute power average (exponential average)
   self->decoder->signalStatus.powerAverage = self->decoder->signalStatus.powerAverage * self->decoder->signalParams.powerAverageW0 + self->decoder->signalStatus.signalValue * self->decoder->signalParams.powerAverageW1;

   // compute signal average (exponential average)
   self->decoder->signalStatus.signalAverage = self->decoder->signalStatus.signalAverage * self->decoder->signalParams.signalAverageW0 + self->decoder->signalStatus.signalValue * self->decoder->signalParams.signalAverageW1;

   // compute signal variance (exponential variance)
   self->decoder->signalStatus.signalVariance = self->decoder->signalStatus.signalVariance * self->decoder->signalParams.signalVarianceW0 + std::abs(self->decoder->signalStatus.signalValue - self->decoder->signalStatus.signalAverage) * self->decoder->signalParams.signalVarianceW1;

   // store next signal value in sample buffer
   self->decoder->signalStatus.signalData[self->decoder->signalClock & (SignalBufferLength - 1)] = self->decoder->signalStatus.signalValue;

#ifdef DEBUG_SIGNAL
   self->decoder->debug->block(self->decoder->signalClock);
#endif

#ifdef DEBUG_SIGNAL_VALUE_CHANNEL
   self->decoder->debug->set(DEBUG_SIGNAL_VALUE_CHANNEL, self->decoder->signalStatus.signalValue);
#endif

#ifdef DEBUG_SIGNAL_POWER_CHANNEL
   self->decoder->debug->set(DEBUG_SIGNAL_POWER_CHANNEL, self->decoder->signalStatus.powerAverage);
#endif

#ifdef DEBUG_SIGNAL_AVERAGE_CHANNEL
   self->decoder->debug->set(DEBUG_SIGNAL_AVERAGE_CHANNEL, self->decoder->signalStatus.signalAverage);
#endif

#ifdef DEBUG_SIGNAL_VARIANCE_CHANNEL
   self->decoder->debug->set(DEBUG_SIGNAL_VARIANCE_CHANNEL, self->decoder->signalStatus.signalVariance);
#endif

#ifdef DEBUG_SIGNAL_EDGE_CHANNEL
   self->decoder->debug->set(DEBUG_SIGNAL_EDGE_CHANNEL, self->decoder->signalStatus.signalAverage - self->decoder->signalStatus.powerAverage);
#endif

   return true;
}

void NfcA::process(NfcFrame frame)
{
   // for request frame set default response timings, must be overridden by subsequent process functions
   if (frame.isPollFrame())
   {
      self->frameStatus.frameWaitingTime = self->protocolStatus.frameWaitingTime;
      self->frameStatus.frameWaitingTime = self->protocolStatus.frameWaitingTime;
   }

   while (true)
   {
      if (processREQA(frame))
         break;

      if (processHLTA(frame))
         break;

      if (!(self->chainedFlags & FrameFlags::Encrypted))
      {
         if (processSELn(frame))
            break;

         if (processRATS(frame))
            break;

         if (processPPSr(frame))
            break;

         if (processAUTH(frame))
            break;

         if (processIBlock(frame))
            break;

         if (processRBlock(frame))
            break;

         if (processSBlock(frame))
            break;

         processOther(frame);
      }

         // all encrypted frames are considered application frames
      else
      {
         frame.setFramePhase(FramePhase::ApplicationFrame);
      }

      break;
   }

   // set chained flags
   frame.setFrameFlags(self->chainedFlags);

   // for request frame set response timings
   if (frame.isPollFrame())
   {
      // update frame timing parameters for receive PICC frame
      if (self->decoder->bitrate)
      {
         // response guard time TR0min (PICC must not modulate reponse within this period)
         self->frameStatus.guardEnd = self->frameStatus.frameEnd + self->frameStatus.frameGuardTime + self->decoder->bitrate->symbolDelayDetect;

         // response delay time WFT (PICC must reply to command before this period)
         self->frameStatus.waitingEnd = self->frameStatus.frameEnd + self->frameStatus.frameWaitingTime + self->decoder->bitrate->symbolDelayDetect;

         // next frame must be ListenFrame
         self->frameStatus.frameType = ListenFrame;
      }
   }
   else
   {
      // switch to self->decoder->modulation search
      self->frameStatus.frameType = 0;

      // reset frame command
      self->frameStatus.lastCommand = 0;
   }

   // mark last processed frame
   self->lastFrameEnd = self->frameStatus.frameEnd;

   // reset farme start
   self->frameStatus.frameStart = 0;

   // reset farme end
   self->frameStatus.frameEnd = 0;
}

bool NfcA::processREQA(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if ((frame[0] == CommandType::NFCA_REQA || frame[0] == CommandType::NFCA_WUPA) && frame.limit() == 1)
      {
         frame.setFramePhase(FramePhase::SelectionFrame);

         self->frameStatus.lastCommand = frame[0];

         // This commands starts or wakeup card communication, so reset the protocol parameters to the default values
         self->protocolStatus.maxFrameSize = 256;
         self->protocolStatus.frameGuardTime = int(self->decoder->signalParams.sampleTimeUnit * 128 * 7);
         self->protocolStatus.frameWaitingTime = int(self->decoder->signalParams.sampleTimeUnit * 256 * 16 * (1 << 4));

         // The REQ-A Response must start exactly at 128 * n, n=9, decoder search between n=7 and n=18
         self->frameStatus.frameGuardTime = self->decoder->signalParams.sampleTimeUnit * 128 * 7; // REQ-A response guard
         self->frameStatus.frameWaitingTime = self->decoder->signalParams.sampleTimeUnit * 128 * 18; // REQ-A response timeout

         // clear chained flags
         self->chainedFlags = 0;

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (self->frameStatus.lastCommand == CommandType::NFCA_REQA || self->frameStatus.lastCommand == CommandType::NFCA_WUPA)
      {
         frame.setFramePhase(FramePhase::SelectionFrame);

         return true;
      }
   }

   return false;
}

bool NfcA::processHLTA(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if (frame[0] == CommandType::NFCA_HLTA && frame.limit() == 4)
      {
         frame.setFramePhase(FramePhase::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         self->frameStatus.lastCommand = frame[0];

         // After this command the PICC will stop and will not respond, set the protocol parameters to the default values
         self->protocolStatus.maxFrameSize = 256;
         self->protocolStatus.frameGuardTime = int(self->decoder->signalParams.sampleTimeUnit * 128 * 7);
         self->protocolStatus.frameWaitingTime = int(self->decoder->signalParams.sampleTimeUnit * 256 * 16 * (1 << 4));

         // clear chained flags
         self->chainedFlags = 0;

         // reset self->decoder->modulation status
         resetModulation();

         return true;
      }
   }

   return false;
}

bool NfcA::processSELn(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if (frame[0] == CommandType::NFCA_SEL1 || frame[0] == CommandType::NFCA_SEL2 || frame[0] == CommandType::NFCA_SEL3)
      {
         frame.setFramePhase(FramePhase::SelectionFrame);

         self->frameStatus.lastCommand = frame[0];

         // The selection commands has same timings as REQ-A
         self->frameStatus.frameGuardTime = self->decoder->signalParams.sampleTimeUnit * 128 * 7;
         self->frameStatus.frameWaitingTime = self->decoder->signalParams.sampleTimeUnit * 128 * 18;

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (self->frameStatus.lastCommand == CommandType::NFCA_SEL1 || self->frameStatus.lastCommand == CommandType::NFCA_SEL2 || self->frameStatus.lastCommand == CommandType::NFCA_SEL3)
      {
         frame.setFramePhase(FramePhase::SelectionFrame);

         return true;
      }
   }

   return false;
}

bool NfcA::processRATS(NfcFrame &frame)
{
   // capture parameters from ATS and reconfigure decoder timings
   if (frame.isPollFrame())
   {
      if (frame[0] == CommandType::NFCA_RATS)
      {
         auto fsdi = (frame[1] >> 4) & 0x0F;

         self->frameStatus.lastCommand = frame[0];

         // sets maximum frame length requested by reader
         self->protocolStatus.maxFrameSize = TABLE_FDS[fsdi];

         // sets the activation frame waiting time for ATS response, ISO/IEC 14443-4 defined a value of 65536/fc (~4833 μs).
         self->frameStatus.frameWaitingTime = int(self->decoder->signalParams.sampleTimeUnit * 65536);

         self->log.info("RATS frame parameters");
         self->log.info("  maxFrameSize {} bytes", {self->protocolStatus.maxFrameSize});

         frame.setFramePhase(FramePhase::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (self->frameStatus.lastCommand == CommandType::NFCA_RATS)
      {
         auto offset = 0;
         auto tl = frame[offset++];

         if (tl > 0)
         {
            auto t0 = frame[offset++];

            // if TA is transmitted, skip...
            if (t0 & 0x10)
               offset++;

            // if TB is transmitted capture timing parameters
            if (t0 & 0x20)
            {
               auto tb = frame[offset++];

               // get Start-up Frame Guard time Integer
               auto sfgi = (tb & 0x0f);

               // get Frame Waiting Time Integer
               auto fwi = (tb >> 4) & 0x0f;

               // A received value of SFGI = 15 MUST be treated by the NFC Forum Device as SFGI = 0.
               if (sfgi == 15)
                  sfgi = 0;

               // A received value of FWI = 15 MUST be treated by the NFC Forum Device as FWI = 4.
               if (fwi == 15)
                  fwi = 4;

               // calculate timing parameters
               self->protocolStatus.startUpGuardTime = int(self->decoder->signalParams.sampleTimeUnit * 256 * 16 * (1 << sfgi));
               self->protocolStatus.frameWaitingTime = int(self->decoder->signalParams.sampleTimeUnit * 256 * 16 * (1 << fwi));
            }
            else
            {
               // if TB is not transmitted establish default timing parameters
               self->protocolStatus.startUpGuardTime = int(self->decoder->signalParams.sampleTimeUnit * 256 * 16 * (1 << 0));
               self->protocolStatus.frameWaitingTime = int(self->decoder->signalParams.sampleTimeUnit * 256 * 16 * (1 << 4));
            }

            self->log.info("ATS protocol timing parameters");
            self->log.info("  startUpGuardTime {} samples ({} us)", {self->frameStatus.startUpGuardTime, 1000000.0 * self->frameStatus.startUpGuardTime / self->decoder->sampleRate});
            self->log.info("  frameWaitingTime {} samples ({} us)", {self->frameStatus.frameWaitingTime, 1000000.0 * self->frameStatus.frameWaitingTime / self->decoder->sampleRate});
         }

         frame.setFramePhase(FramePhase::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   return false;
}

bool NfcA::processPPSr(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if ((frame[0] & 0xF0) == CommandType::NFCA_PPS)
      {
         self->frameStatus.lastCommand = frame[0] & 0xF0;

         // Set PPS response waiting time to protocol default
         self->frameStatus.frameWaitingTime = self->protocolStatus.frameWaitingTime;

         frame.setFramePhase(FramePhase::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (self->frameStatus.lastCommand == CommandType::NFCA_PPS)
      {
         frame.setFramePhase(FramePhase::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   return false;
}

bool NfcA::processAUTH(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if (frame[0] == CommandType::NFCA_AUTH1 || frame[0] == CommandType::NFCA_AUTH2)
      {
         self->frameStatus.lastCommand = frame[0];
         //         self->frameStatus.frameWaitingTime = int(signalParams.sampleTimeUnit * 256 * 16 * (1 << 14);

         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         //      if (!self->frameStatus.lastCommand)
         //      {
         //         crypto1_init(&self->frameStatus.crypto1, 0x521284223154);
         //      }

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (self->frameStatus.lastCommand == CommandType::NFCA_AUTH1 || self->frameStatus.lastCommand == CommandType::NFCA_AUTH2)
      {
         //      unsigned int uid = 0x1e47fc35;
         //      unsigned int nc = frame[0] << 24 || frame[1] << 16 || frame[2] << 8 || frame[0];
         //
         //      crypto1_word(&self->frameStatus.crypto1, uid ^ nc, 0);

         // set chained flags

         self->chainedFlags = FrameFlags::Encrypted;

         frame.setFramePhase(FramePhase::ApplicationFrame);

         return true;
      }
   }

   return false;
}

bool NfcA::processIBlock(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if ((frame[0] & 0xE2) == CommandType::NFCA_IBLOCK)
      {
         self->frameStatus.lastCommand = frame[0] & 0xE2;

         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (self->frameStatus.lastCommand == CommandType::NFCA_IBLOCK)
      {
         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   return false;
}

bool NfcA::processRBlock(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if ((frame[0] & 0xE6) == CommandType::NFCA_RBLOCK)
      {
         self->frameStatus.lastCommand = frame[0] & 0xE6;

         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (self->frameStatus.lastCommand == CommandType::NFCA_RBLOCK)
      {
         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   return false;
}

bool NfcA::processSBlock(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if ((frame[0] & 0xC7) == CommandType::NFCA_SBLOCK)
      {
         self->frameStatus.lastCommand = frame[0] & 0xC7;

         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (self->frameStatus.lastCommand == CommandType::NFCA_SBLOCK)
      {
         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   return false;
}

void NfcA::processOther(NfcFrame &frame)
{
   frame.setFramePhase(FramePhase::ApplicationFrame);
   frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);
}

bool NfcA::checkCrc(NfcFrame &frame)
{
   unsigned short crc = 0, res = 0;

   int length = frame.limit();

   if (length <= 2)
      return false;

   if (frame.isNfcA())
      crc = 0x6363; // NFC-A ITU-V.41
   else if (frame.isNfcB())
      crc = 0xFFFF; // NFC-B ISO/IEC 13239

   for (int i = 0; i < length - 2; i++)
   {
      auto d = (unsigned char) frame[i];

      d = (d ^ (unsigned int) (crc & 0xff));
      d = (d ^ (d << 4));

      crc = (crc >> 8) ^ ((unsigned short) (d << 8)) ^ ((unsigned short) (d << 3)) ^ ((unsigned short) (d >> 4));
   }

   if (frame.isNfcB())
      crc = ~crc;

   res |= ((unsigned int) frame[length - 2] & 0xff);
   res |= ((unsigned int) frame[length - 1] & 0xff) << 8;

   return res == crc;
}

bool NfcA::checkParity(unsigned int value, unsigned int parity)
{
   for (unsigned int i = 0; i < 8; i++)
   {
      if ((value & (1 << i)) != 0)
      {
         parity = parity ^ 1;
      }
   }

   return parity;
}

}
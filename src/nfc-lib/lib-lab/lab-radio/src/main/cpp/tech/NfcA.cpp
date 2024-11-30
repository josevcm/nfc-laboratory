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

#include <cmath>
#include <cstring>
#include <functional>

#include <rt/Logger.h>

#include <lab/data/Crc.h>
#include <lab/nfc/Nfc.h>

#include <tech/NfcA.h>

namespace lab {

enum PatternType
{
   Invalid = 0,
   NoPattern = 1,
   PatternX = 2,
   PatternY = 3,
   PatternZ = 4,
   PatternD = 5,
   PatternE = 6,
   PatternF = 7,
   PatternM = 8,
   PatternN = 9,
   PatternS = 10,
   PatternO = 11
};

/*
 * status for protocol
 */
struct NfcProtocolStatus
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

struct NfcA::Impl : NfcTech
{
   rt::Logger *log = rt::Logger::getLogger("decoder.NfcA");

   // global signal status
   NfcDecoderStatus *decoder;

   // bitrate parameters
   NfcBitrateParams bitrateParams[4]{};

   // detected symbol status
   NfcSymbolStatus symbolStatus{};

   // bit stream status
   NfcStreamStatus streamStatus{};

   // frame processing status
   NfcFrameStatus frameStatus{};

   // protocol processing status
   NfcProtocolStatus protocolStatus{};

   // modulation status for each bitrate
   NfcModulationStatus modulationStatus[4]{};

   // minimum modulation deep to detect valid signal for NFC-A (default 90%)
   float minimumModulationDeep = 0.90f;

   // minimum modulation deep to detect valid signal for NFC-A (default 100%)
   float maximumModulationDeep = 1.00f;

   // correlation threshold to detect valid NFC-A pulse (default 75%)
   float correlationThreshold = 0.75f;

   // last detected frame end
   unsigned int lastFrameEnd = 0;

   // chained frame flags
   unsigned int chainedFlags = 0;

   explicit Impl(NfcDecoderStatus *decoder) : decoder(decoder)
   {
   }

   /*
    * Configure NFC-A modulation
    */
   void initialize(unsigned int sampleRate)
   {
      log->info("--------------------------------------------");
      log->info("initializing NFC-A decoder");
      log->info("--------------------------------------------");
      log->info("\tsignalSampleRate     {}", {decoder->sampleRate});
      log->info("\tpowerLevelThreshold  {}", {decoder->powerLevelThreshold});
      log->info("\tcorrelationThreshold {}", {correlationThreshold});
      log->info("\tmodulationThreshold  {} -> {}", {minimumModulationDeep, maximumModulationDeep});

      // clear last detected frame end
      lastFrameEnd = 0;

      // clear chained flags
      chainedFlags = 0;

      // clear detected symbol status
      symbolStatus = {};

      // clear bit stream status
      streamStatus = {};

      // clear frame processing status
      frameStatus = {};

      // compute symbol parameters for 106Kbps, 212Kbps and 424Kbps
      for (int rate = r106k; rate <= r424k; rate++)
      {
         // clear bitrate parameters
         bitrateParams[rate] = {};

         // clear modulation parameters
         modulationStatus[rate] = {};

         // configure bitrate parametes
         NfcBitrateParams *bitrate = bitrateParams + rate;

         // set tech type and rate
         bitrate->techType = FrameTech::NfcATech;
         bitrate->rateType = rate;

         // symbol timing parameters
         bitrate->symbolsPerSecond = static_cast<int>(std::round(NFC_FC / static_cast<float>(128 >> rate)));

         // number of samples per symbol
         bitrate->period0SymbolSamples = static_cast<int>(std::round(decoder->signalParams.sampleTimeUnit * (256 >> rate))); // double symbol samples
         bitrate->period1SymbolSamples = static_cast<int>(std::round(decoder->signalParams.sampleTimeUnit * (128 >> rate))); // full symbol samples
         bitrate->period2SymbolSamples = static_cast<int>(std::round(decoder->signalParams.sampleTimeUnit * (64 >> rate))); // half symbol samples
         bitrate->period4SymbolSamples = static_cast<int>(std::round(decoder->signalParams.sampleTimeUnit * (32 >> rate))); // quarter of symbol...
         bitrate->period8SymbolSamples = static_cast<int>(std::round(decoder->signalParams.sampleTimeUnit * (16 >> rate))); // and so on...

         // delay guard for each symbol rate
         bitrate->symbolDelayDetect = rate > r106k ? bitrateParams[rate - 1].symbolDelayDetect + bitrateParams[rate - 1].period1SymbolSamples : 0;

         // moving average offsets
         bitrate->offsetFutureIndex = BUFFER_SIZE;
         bitrate->offsetSignalIndex = BUFFER_SIZE - bitrate->symbolDelayDetect;
         bitrate->offsetDelay0Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period0SymbolSamples;
         bitrate->offsetDelay1Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period1SymbolSamples;
         bitrate->offsetDelay2Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period2SymbolSamples;
         bitrate->offsetDelay4Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period4SymbolSamples;
         bitrate->offsetDelay8Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period8SymbolSamples;

         log->info("{} kpbs parameters:", {round(bitrate->symbolsPerSecond / 1E3)});
         log->info("\tsymbolsPerSecond     {}", {bitrate->symbolsPerSecond});
         log->info("\tperiod1SymbolSamples {} ({} us)", {bitrate->period1SymbolSamples, 1E6 * bitrate->period1SymbolSamples / decoder->sampleRate});
         log->debug("\tperiod2SymbolSamples {} ({} us)", {bitrate->period2SymbolSamples, 1E6 * bitrate->period2SymbolSamples / decoder->sampleRate});
         log->debug("\tperiod4SymbolSamples {} ({} us)", {bitrate->period4SymbolSamples, 1E6 * bitrate->period4SymbolSamples / decoder->sampleRate});
         log->debug("\tperiod8SymbolSamples {} ({} us)", {bitrate->period8SymbolSamples, 1E6 * bitrate->period8SymbolSamples / decoder->sampleRate});
         log->debug("\tsymbolDelayDetect    {} ({} us)", {bitrate->symbolDelayDetect, 1E6 * bitrate->symbolDelayDetect / decoder->sampleRate});
         log->debug("\toffsetInsertIndex    {}", {bitrate->offsetFutureIndex});
         log->debug("\toffsetSignalIndex    {}", {bitrate->offsetSignalIndex});
         log->debug("\toffsetDelay8Index    {}", {bitrate->offsetDelay8Index});
         log->debug("\toffsetDelay4Index    {}", {bitrate->offsetDelay4Index});
         log->debug("\toffsetDelay2Index    {}", {bitrate->offsetDelay2Index});
         log->debug("\toffsetDelay1Index    {}", {bitrate->offsetDelay1Index});
         log->debug("\toffsetDelay0Index    {}", {bitrate->offsetDelay0Index});
      }

      // initialize default protocol parameters for start decoding
      protocolStatus.maxFrameSize = 256;
      protocolStatus.startUpGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_SFGT_DEF);
      protocolStatus.frameGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_FGT_DEF);
      protocolStatus.frameWaitingTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_FWT_DEF);
      protocolStatus.requestGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_RGT_DEF);

      // initialize frame parameters to default protocol parameters
      frameStatus.startUpGuardTime = protocolStatus.startUpGuardTime;
      frameStatus.frameWaitingTime = protocolStatus.frameWaitingTime;
      frameStatus.frameGuardTime = protocolStatus.frameGuardTime;
      frameStatus.requestGuardTime = protocolStatus.requestGuardTime;

      log->info("Startup parameters");
      log->info("\tmaxFrameSize {} bytes", {protocolStatus.maxFrameSize});
      log->info("\tframeGuardTime {} samples ({} us)", {protocolStatus.frameGuardTime, 1000000.0 * protocolStatus.frameGuardTime / decoder->sampleRate});
      log->info("\tframeWaitingTime {} samples ({} us)", {protocolStatus.frameWaitingTime, 1000000.0 * protocolStatus.frameWaitingTime / decoder->sampleRate});
      log->info("\trequestGuardTime {} samples ({} us)", {protocolStatus.requestGuardTime, 1000000.0 * protocolStatus.requestGuardTime / decoder->sampleRate});
   }

   /*
    * Detect NFC-A modulation
    */
   bool detectModulation()
   {
      // wait until has enough data in buffer
      if (decoder->signalClock < BUFFER_SIZE)
         return false;

      // ignore low power signals
      if (decoder->signalEnvelope < decoder->powerLevelThreshold)
         return false;

      // for NFC-A minimum correlation is required to filter-out higher bit-rates, only valid rate can reach the threshold
      float minimumCorrelationValue = decoder->signalEnvelope * correlationThreshold;

      for (int rate = r106k; rate <= r424k; rate++)
      {
         NfcBitrateParams *bitrate = bitrateParams + rate;
         NfcModulationStatus *modulation = modulationStatus + rate;

         //  signal pointers
         unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
         unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);
         unsigned int delay8Index = (bitrate->offsetDelay8Index + decoder->signalClock);

         // correlation pointers
         unsigned int filterPoint1 = (signalIndex % bitrate->period1SymbolSamples);
         unsigned int filterPoint2 = (signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
         unsigned int filterPoint3 = (signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += decoder->sample[signalIndex & (BUFFER_SIZE - 1)].samplingValue;
         modulation->filterIntegrate -= decoder->sample[delay2Index & (BUFFER_SIZE - 1)].samplingValue;

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation factors
         float correlatedS0 = modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2];
         float correlatedS1 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3];
         float correlatedSD = (correlatedS0 - correlatedS1) / static_cast<float>(bitrate->period2SymbolSamples);

         if (decoder->debug)
         {
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, modulation->filterIntegrate / static_cast<float>(bitrate->period2SymbolSamples));
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 1, correlatedSD);
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 2, modulation->searchValueThreshold);

            if (decoder->signalClock == modulation->searchSyncTime)
               decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 1, 0.75f);
         }

         // recover status from previous partial search
         if (modulation->correlatedPeakTime && decoder->signalClock > modulation->correlatedPeakTime + bitrate->period1SymbolSamples)
         {
            modulation->symbolStartTime = 0;
            modulation->symbolEndTime = 0;
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->searchSyncTime = 0;
            modulation->detectorPeakTime = 0;
            modulation->detectorPeakValue = 0;
            modulation->correlatedPeakTime = 0;
            modulation->correlatedPeakValue = 0;
         }

         // wait until correlation search start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         if (!modulation->symbolStartTime)
         {
            // get signal deep
            float signalDeep = decoder->sample[delay8Index & (BUFFER_SIZE - 1)].modulateDepth;

            // detect minimum correlation point
            if (correlatedSD < -minimumCorrelationValue)
            {
               if (correlatedSD < modulation->correlatedPeakValue)
               {
                  modulation->correlatedPeakValue = correlatedSD;
                  modulation->correlatedPeakTime = decoder->signalClock;
                  modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
               }

               if (signalDeep > modulation->detectorPeakValue)
               {
                  modulation->detectorPeakValue = signalDeep;
                  modulation->detectorPeakTime = decoder->signalClock;
               }
            }
         }
         else
         {
            if (correlatedSD > minimumCorrelationValue)
            {
               // detect maximum correlation point
               if (correlatedSD > modulation->correlatedPeakValue)
               {
                  modulation->correlatedPeakValue = correlatedSD;
                  modulation->correlatedPeakTime = decoder->signalClock;
               }
            }
         }

         // wait until search finished and consume all pulse to measure wide
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         if (!modulation->symbolStartTime)
         {
            // check modulation deep
            if (modulation->detectorPeakValue < minimumModulationDeep)
            {
               // reset modulation to continue search
               modulation->symbolStartTime = 0;
               modulation->symbolEndTime = 0;
               modulation->searchSyncTime = 0;
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;
               modulation->searchPulseWidth = 0;
               modulation->correlatedPeakTime = 0;
               modulation->correlatedPeakValue = 0;
               modulation->detectorPeakTime = 0;
               modulation->detectorPeakValue = 0;
               continue;
            }

            modulation->searchSyncTime = modulation->correlatedPeakTime + bitrate->period2SymbolSamples;
            modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
            modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
            modulation->symbolStartTime = modulation->correlatedPeakTime - bitrate->period2SymbolSamples;
            modulation->correlatedPeakTime = 0;
            modulation->correlatedPeakValue = 0;
            continue;
         }

         // pulse end time
         modulation->symbolEndTime = modulation->correlatedPeakTime;
         modulation->searchPulseWidth = modulation->symbolEndTime - modulation->symbolStartTime;

         // NFC-A pulse wide discriminator
         int minimumPulseWidth = bitrate->period1SymbolSamples - bitrate->period4SymbolSamples;
         int maximumPulseWidth = bitrate->period1SymbolSamples + bitrate->period4SymbolSamples;

         // check for valid NFC-A modulated pulse
         if (modulation->correlatedPeakTime == 0 || // no modulation found
            modulation->detectorPeakValue < minimumModulationDeep || // insufficient modulation deep
            modulation->searchPulseWidth < minimumPulseWidth || // pulse too short
            modulation->searchPulseWidth > maximumPulseWidth) // pulse too wide
         {
            // reset modulation to continue search
            modulation->symbolStartTime = 0;
            modulation->symbolEndTime = 0;
            modulation->searchSyncTime = 0;
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->searchPulseWidth = 0;
            modulation->correlatedPeakTime = 0;
            modulation->correlatedPeakValue = 0;
            modulation->detectorPeakTime = 0;
            modulation->detectorPeakValue = 0;
            continue;
         }

         // prepare next search window from synchronization point
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
         modulation->searchValueThreshold = modulation->correlatedPeakValue / 2;
         modulation->searchCorr0Value = 0;
         modulation->searchCorr1Value = 0;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // setup frame info
         frameStatus.frameType = NfcPollFrame;
         frameStatus.symbolRate = bitrate->symbolsPerSecond;
         frameStatus.frameStart = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         frameStatus.frameEnd = 0;

         // setup symbol info
         symbolStatus.value = 0;
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;
         symbolStatus.pattern = PatternZ;

         // modulation detected
         decoder->bitrate = bitrate;
         decoder->modulation = modulation;

         return true;
      }

      return false;
   }

   /*
    * Decode next poll or listen frame
    */
   void decodeFrame(hw::SignalBuffer &samples, std::list<RawFrame> &frames)
   {
      if (frameStatus.frameType == NfcPollFrame)
      {
         decodePollFrame(samples, frames);
      }

      if (frameStatus.frameType == NfcListenFrame)
      {
         decodeListenFrame(samples, frames);
      }
   }

   /*
    * Decode next poll frame
    */
   bool decodePollFrame(hw::SignalBuffer &buffer, std::list<RawFrame> &frames)
   {
      int pattern;
      bool frameEnd = false, truncateError = false;

      // read NFC-A request request
      while ((pattern = decodePollFrameSymbolAsk(buffer)) > NoPattern)
      {
         streamStatus.pattern = pattern;

         if (streamStatus.pattern == PatternY && (streamStatus.previous == PatternY || streamStatus.previous == PatternZ))
            frameEnd = true;

         else if (streamStatus.bytes == protocolStatus.maxFrameSize)
            truncateError = true;

         // detect end of request (Pattern-Y after Pattern-Z)
         if (frameEnd || truncateError)
         {
            // frames must contain at least one full byte or 7 bits for short frames
            if (streamStatus.bytes > 0 || streamStatus.bits == 7)
            {
               // add remaining byte to request
               if (streamStatus.bits >= 7)
                  streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;

               // build request frame
               RawFrame request = RawFrame(FrameTech::NfcATech, NfcPollFrame);

               request.setFrameRate(frameStatus.symbolRate);
               request.setSampleStart(frameStatus.frameStart);
               request.setSampleEnd(frameStatus.frameEnd);
               request.setSampleRate(decoder->sampleRate);
               request.setTimeStart(static_cast<double>(frameStatus.frameStart) / static_cast<double>(decoder->sampleRate));
               request.setTimeEnd(static_cast<double>(frameStatus.frameEnd) / static_cast<double>(decoder->sampleRate));
               request.setDateTime(decoder->streamTime + request.timeStart());

               if (streamStatus.flags & ParityError)
                  request.setFrameFlags(ParityError);

               if (truncateError)
                  request.setFrameFlags(Truncated);

               if (streamStatus.bytes == 1 && streamStatus.bits == 7)
                  request.setFrameFlags(ShortFrame);

               // add bytes to frame and flip to prepare read
               request.put(streamStatus.buffer, streamStatus.bytes).flip();

               // process frame
               process(request);

               // add to frame list
               frames.push_back(request);

               // clear stream status
               streamStatus = {};

               // clear modulation status for receiving card response
               if (decoder->modulation)
               {
                  decoder->modulation->symbolStartTime = 0;
                  decoder->modulation->symbolEndTime = 0;
                  decoder->modulation->filterIntegrate = 0;
                  decoder->modulation->detectIntegrate = 0;
                  decoder->modulation->phaseIntegrate = 0;
                  decoder->modulation->searchModeState = 0;
                  decoder->modulation->searchSyncTime = 0;
                  decoder->modulation->searchStartTime = 0;
                  decoder->modulation->searchEndTime = 0;
                  decoder->modulation->searchPulseWidth = 0;
                  decoder->modulation->searchLastValue = 0;
                  decoder->modulation->searchLastPhase = 0;
                  decoder->modulation->searchValueThreshold = 0;
                  decoder->modulation->searchPhaseThreshold = 0;
                  decoder->modulation->correlatedPeakValue = 0;

                  std::memset(decoder->modulation->integrationData, 0, sizeof(NfcModulationStatus::integrationData));
                  std::memset(decoder->modulation->correlationData, 0, sizeof(NfcModulationStatus::correlationData));
               }

               // return request frame data
               return true;
            }

            // reset modulation and restart frame detection
            resetModulation();

            // no valid frame found
            return false;
         }

         // update frame end
         if (symbolStatus.edge)
            frameStatus.frameEnd = symbolStatus.edge;

         if (streamStatus.previous)
         {
            int value = (streamStatus.previous == PatternX);

            // decode next bit
            if (streamStatus.bits < 8)
            {
               streamStatus.data = streamStatus.data | (value << streamStatus.bits++);
            }

            // store full byte in stream buffer and check parity
            else if (streamStatus.bytes < protocolStatus.maxFrameSize)
            {
               streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;
               streamStatus.flags |= !checkParity(streamStatus.data, value) ? ParityError : 0;
               streamStatus.data = streamStatus.bits = 0;
            }

            // too many bytes in frame, abort decoder
            else
            {
               // reset modulation status
               resetModulation();

               // no valid frame found
               return false;
            }
         }

         // update previous command state
         streamStatus.previous = streamStatus.pattern;
      }

      // no frame detected
      return false;
   }

   /*
    * Decode next listen frame
    */
   bool decodeListenFrame(hw::SignalBuffer &buffer, std::list<RawFrame> &frames)
   {
      int pattern;
      bool frameEnd = false, truncateError = false;

      // decode TAG ASK response
      if (decoder->bitrate->rateType == r106k)
      {
         if (!frameStatus.frameStart)
         {
            // search Start Of Frame pattern
            pattern = decodeListenFrameStartAsk(buffer);

            // Pattern-D found, mark frame start time
            if (pattern == PatternD)
            {
               frameStatus.frameStart = symbolStatus.start;
            }
            else
            {
               //  end of frame waiting time, restart modulation search
               if (pattern == NoPattern)
                  resetModulation();

               // no frame found
               return RawFrame::Nil;
            }
         }

         if (frameStatus.frameStart)
         {
            // decode remaining response
            while ((pattern = decodeListenFrameSymbolAsk(buffer)) > NoPattern)
            {
               if (pattern == PatternF)
                  frameEnd = true;

               else if (streamStatus.bytes == protocolStatus.maxFrameSize)
                  truncateError = true;

               // detect end of response for ASK
               if (frameEnd || truncateError)
               {
                  // a valid response must contains at least 4 bits of data
                  if (streamStatus.bytes > 0 || streamStatus.bits == 4)
                  {
                     // add remaining byte to request
                     if (streamStatus.bits == 4)
                        streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;

                     RawFrame response = RawFrame(FrameTech::NfcATech, NfcListenFrame);

                     response.setFrameRate(decoder->bitrate->symbolsPerSecond);
                     response.setSampleStart(frameStatus.frameStart);
                     response.setSampleEnd(frameStatus.frameEnd);
                     response.setSampleRate(decoder->sampleRate);
                     response.setTimeStart(static_cast<double>(frameStatus.frameStart) / static_cast<double>(decoder->sampleRate));
                     response.setTimeEnd(static_cast<double>(frameStatus.frameEnd) / static_cast<double>(decoder->sampleRate));
                     response.setDateTime(decoder->streamTime + response.timeStart());

                     if (streamStatus.flags & ParityError)
                        response.setFrameFlags(ParityError);

                     if (truncateError)
                        response.setFrameFlags(Truncated);

                     if (streamStatus.bytes == 1 && streamStatus.bits == 4)
                        response.setFrameFlags(ShortFrame);

                     // add bytes to frame and flip to prepare read
                     response.put(streamStatus.buffer, streamStatus.bytes).flip();

                     // process frame
                     process(response);

                     // add to frame list
                     frames.push_back(response);

                     // reset modulation status
                     resetModulation();

                     return true;
                  }

                  // only detect first pattern-D without anymore, so can be spurious pulse, we try to find SoF again
                  resetFrameSearch();

                  // no valid frame found
                  return false;
               }

               // update frame end
               if (symbolStatus.edge)
                  frameStatus.frameEnd = symbolStatus.edge;

               // decode next bit
               if (streamStatus.bits < 8)
               {
                  streamStatus.data |= (symbolStatus.value << streamStatus.bits++);
               }

               // store full byte in stream buffer and check parity
               else if (streamStatus.bytes < protocolStatus.maxFrameSize)
               {
                  streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;
                  streamStatus.flags |= !checkParity(streamStatus.data, symbolStatus.value) ? ParityError : 0;
                  streamStatus.data = streamStatus.bits = 0;
               }

               // too many bytes in frame, abort decoder
               else
               {
                  // reset modulation status
                  resetModulation();

                  // no valid frame found
                  return false;
               }
            }
         }
      }

      // decode TAG BPSK response
      else if (decoder->bitrate->rateType == r212k || decoder->bitrate->rateType == r424k)
      {
         if (!frameStatus.frameStart)
         {
            // detect first pattern
            pattern = decodeListenFrameStartBpsk(buffer);

            // Pattern-S found, mark frame start time
            if (pattern == PatternS)
            {
               frameStatus.frameStart = symbolStatus.start;
            }
            else
            {
               //  end of frame waiting time, restart modulation search
               if (pattern == NoPattern)
                  resetModulation();

               // no frame found
               return false;
            }
         }

         // frame SoF detected, decode frame stream...
         if (frameStatus.frameStart)
         {
            while ((pattern = decodeListenFrameSymbolBpsk(buffer)) > NoPattern)
            {
               if (pattern == PatternO)
                  frameEnd = true;

               else if (streamStatus.bytes == protocolStatus.maxFrameSize)
                  truncateError = true;

               // detect end of response for BPSK
               if (frameEnd || truncateError)
               {
                  if (streamStatus.bits == 9)
                  {
                     // store byte in stream buffer
                     streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;

                     // last byte has even parity
                     streamStatus.flags |= checkParity(streamStatus.data, streamStatus.parity) ? ParityError : 0;
                  }

                  // frames must contain at least one full byte
                  if (streamStatus.bytes > 0)
                  {
                     // mark frame end at star of EoF symbol
                     frameStatus.frameEnd = symbolStatus.end;

                     // build responde frame
                     RawFrame response = RawFrame(FrameTech::NfcATech, NfcListenFrame);

                     response.setFrameRate(decoder->bitrate->symbolsPerSecond);
                     response.setSampleStart(frameStatus.frameStart);
                     response.setSampleEnd(frameStatus.frameEnd);
                     response.setSampleRate(decoder->sampleRate);
                     response.setTimeStart(static_cast<double>(frameStatus.frameStart) / static_cast<double>(decoder->sampleRate));
                     response.setTimeEnd(static_cast<double>(frameStatus.frameEnd) / static_cast<double>(decoder->sampleRate));
                     response.setDateTime(decoder->streamTime + response.timeStart());

                     if (streamStatus.flags & ParityError)
                        response.setFrameFlags(ParityError);

                     if (truncateError)
                        response.setFrameFlags(Truncated);

                     // add bytes to frame and flip to prepare read
                     response.put(streamStatus.buffer, streamStatus.bytes).flip();

                     // process frame
                     process(response);

                     // add to frame list
                     frames.push_back(response);

                     // reset modulation status
                     resetModulation();

                     return true;
                  }

                  // reset modulation status
                  resetModulation();

                  // no valid frame found
                  return false;
               }

               // decode next data bit
               if (streamStatus.bits < 8)
                  streamStatus.data |= (symbolStatus.value << streamStatus.bits);

                  // decode parity bit
               else if (streamStatus.bits < 9)
                  streamStatus.parity = symbolStatus.value;

               // store full byte in stream buffer and check parity
               else
               {
                  // store byte in stream buffer and check parity
                  streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;
                  streamStatus.flags |= !checkParity(streamStatus.data, streamStatus.parity) ? ParityError : 0;
                  streamStatus.data = symbolStatus.value;
                  streamStatus.bits = 0;
               }

               streamStatus.bits++;
            }
         }
      }

      // end of stream...
      return false;
   }

   /*
    * Decode one ASK modulated poll frame symbol
    */
   int decodePollFrameSymbolAsk(hw::SignalBuffer &buffer)
   {
      NfcBitrateParams *bitrate = decoder->bitrate;
      NfcModulationStatus *modulation = decoder->modulation;

      // compute signal pointers
      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;
         ++delay2Index;

         // compute correlation pointers
         unsigned int filterPoint1 = (signalIndex % bitrate->period1SymbolSamples);
         unsigned int filterPoint2 = (signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
         unsigned int filterPoint3 = (signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += decoder->sample[signalIndex & (BUFFER_SIZE - 1)].samplingValue;
         modulation->filterIntegrate -= decoder->sample[delay2Index & (BUFFER_SIZE - 1)].samplingValue;

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation factors
         float correlatedS0 = modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2];
         float correlatedS1 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3];
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1) / static_cast<float>(bitrate->period2SymbolSamples);

         if (decoder->debug)
         {
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, modulation->filterIntegrate / static_cast<float>(bitrate->period2SymbolSamples));
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 1, correlatedS0 / static_cast<float>(bitrate->period4SymbolSamples));
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 2, modulation->searchValueThreshold);

            if (decoder->signalClock == modulation->searchSyncTime)
               decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 1, 0.50f);
         }

         // wait until correlation search start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         // detect maximum symbol correlation
         if (correlatedSD > modulation->correlatedPeakValue && correlatedSD > modulation->searchValueThreshold)
         {
            modulation->correlatedPeakValue = correlatedSD;
            modulation->correlatedPeakTime = decoder->signalClock;
         }

         // capture symbol correlation values at synchronization point
         if (decoder->signalClock == modulation->searchSyncTime)
         {
            modulation->searchCorrDValue = correlatedSD;
            modulation->searchCorr0Value = correlatedS0;
            modulation->searchCorr1Value = correlatedS1;
         }

         // wait until correlation search finish
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // detect Pattern-Y when no modulation occurs (below search detection threshold)
         if (modulation->searchCorrDValue < modulation->searchValueThreshold)
         {
            // estimate symbol timings from synchronization point (peak detection not valid due lack of modulation)
            modulation->symbolStartTime = modulation->symbolEndTime;
            modulation->symbolEndTime = modulation->searchSyncTime;
            modulation->symbolRiseTime = modulation->symbolStartTime;

            // setup symbol info
            symbolStatus.value = 1;
            symbolStatus.pattern = PatternY;
         }

         // detect Pattern-Z
         else if (modulation->searchCorr0Value > modulation->searchCorr1Value)
         {
            // re-sync symbol end from correlate peak detector
            modulation->symbolStartTime = modulation->symbolEndTime;
            modulation->symbolEndTime = modulation->correlatedPeakTime;
            modulation->symbolRiseTime = modulation->correlatedPeakTime - bitrate->period2SymbolSamples;

            // setup symbol info
            symbolStatus.value = 0;
            symbolStatus.pattern = PatternZ;
         }

         // detect Pattern-X
         else
         {
            // re-sync symbol end from correlate peak detector
            modulation->symbolStartTime = modulation->symbolEndTime;
            modulation->symbolEndTime = modulation->correlatedPeakTime;
            modulation->symbolRiseTime = modulation->correlatedPeakTime;

            // detect Pattern-X, setup symbol info
            symbolStatus.value = 1;
            symbolStatus.pattern = PatternX;
         }

         // sets next search window
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
         modulation->searchCorrDValue = 0;
         modulation->searchCorr0Value = 0;
         modulation->searchCorr1Value = 0;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.edge = modulation->symbolRiseTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;

         return symbolStatus.pattern;
      }

      return Invalid;
   }

   /*
    * Decode SOF for ASK modulated listen frame
    */
   int decodeListenFrameStartAsk(hw::SignalBuffer &buffer)
   {
      NfcBitrateParams *bitrate = decoder->bitrate;
      NfcModulationStatus *modulation = decoder->modulation;

      unsigned int futureIndex = (bitrate->offsetFutureIndex + decoder->signalClock);
      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;
         ++futureIndex;
         ++delay2Index;

         // compute correlation points
         unsigned int filterPoint1 = (signalIndex % bitrate->period1SymbolSamples);
         unsigned int filterPoint2 = (signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;

         // get signal samples
         float signalData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].filteredValue;
         float signalDeep = decoder->sample[futureIndex & (BUFFER_SIZE - 1)].modulateDepth;

         // store signal square in filter buffer
         modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)] = signalData * signalData * 10;

         // integrate symbol (moving average)
         modulation->filterIntegrate += modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->filterIntegrate -= modulation->integrationData[delay2Index & (BUFFER_SIZE - 1)]; // remove delayed value

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation results for each symbol and distance
         float correlatedS0 = modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2];

         if (decoder->debug)
         {
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]);
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 1, modulation->filterIntegrate);
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 2, correlatedS0);
         }

         // wait until frame guard time is reached to start response search
         if (decoder->signalClock < frameStatus.guardEnd)
            continue;

         // using minimum signal st.dev as lower level threshold
         if (decoder->signalClock == frameStatus.guardEnd)
            modulation->searchValueThreshold = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].meanDeviation * bitrate->period8SymbolSamples;

         // check for maximum response time
         if (decoder->signalClock > frameStatus.waitingEnd)
            return NoPattern;

         // poll frame modulation detected while waiting for response
         if (signalDeep > minimumModulationDeep)
            return NoPattern;

         if (decoder->debug)
         {
            if (decoder->signalClock < (frameStatus.guardEnd + 5))
               decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 2, modulation->searchValueThreshold);

            if (decoder->signalClock == modulation->searchSyncTime)
               decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 2, 0.75f);
         }

         // detect modulation peaks
         if (!modulation->symbolStartTime)
         {
            // detect maximum correlation point
            if (correlatedS0 > modulation->searchValueThreshold && correlatedS0 > modulation->correlatedPeakValue)
            {
               modulation->correlatedPeakValue = correlatedS0;
               modulation->correlatedPeakTime = decoder->signalClock;
               modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
            }
         }
         else
         {
            // detect maximum correlation point
            if (correlatedS0 < -modulation->searchValueThreshold && correlatedS0 < modulation->correlatedPeakValue)
            {
               modulation->correlatedPeakValue = correlatedS0;
               modulation->correlatedPeakTime = decoder->signalClock;
            }
         }

         // wait until search finished and consume all pulse to measure wide
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         if (!modulation->symbolStartTime)
         {
            modulation->searchSyncTime = modulation->correlatedPeakTime + bitrate->period2SymbolSamples;
            modulation->searchEndTime = modulation->searchEndTime + bitrate->period2SymbolSamples;
            modulation->symbolStartTime = modulation->correlatedPeakTime - bitrate->period2SymbolSamples;
            modulation->correlatedPeakTime = 0;
            modulation->correlatedPeakValue = 0;
            continue;
         }

         // pulse end time
         modulation->symbolEndTime = modulation->correlatedPeakTime;
         modulation->searchPulseWidth = modulation->symbolEndTime - modulation->symbolStartTime;

         // NFC-A pulse wide discriminator
         int minimumPulseWidth = bitrate->period1SymbolSamples - bitrate->period8SymbolSamples;
         int maximumPulseWidth = bitrate->period1SymbolSamples + bitrate->period8SymbolSamples;

         // check for valid NFC-A modulated pulse
         if (modulation->correlatedPeakTime == 0 || // no modulation found
            modulation->searchPulseWidth < minimumPulseWidth || // pulse too short
            modulation->searchPulseWidth > maximumPulseWidth) // pulse too wide
         {
            // reset modulation to continue search
            modulation->symbolStartTime = 0;
            modulation->symbolEndTime = 0;
            modulation->searchSyncTime = 0;
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->searchPulseWidth = 0;
            modulation->correlatedPeakTime = 0;
            modulation->correlatedPeakValue = 0;
            modulation->detectorPeakTime = 0;
            modulation->detectorPeakValue = 0;
            continue;
         }

         // prepare next search window from synchronization point
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
         modulation->searchValueThreshold = std::fabs(modulation->correlatedPeakValue * 0.25f);
         modulation->searchCorr0Value = 0;
         modulation->searchCorr1Value = 0;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // setup symbol info
         symbolStatus.value = 1;
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;
         symbolStatus.pattern = PatternD;

         return symbolStatus.pattern;
      }

      return Invalid;
   }

   /*
    * Decode one ASK modulated listen frame symbol
    */
   int decodeListenFrameSymbolAsk(hw::SignalBuffer &buffer)
   {
      NfcBitrateParams *bitrate = decoder->bitrate;
      NfcModulationStatus *modulation = decoder->modulation;

      // compute pointers
      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;
         ++delay2Index;

         // compute correlation points
         unsigned int filterPoint1 = (signalIndex % bitrate->period1SymbolSamples);
         unsigned int filterPoint2 = (signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
         unsigned int filterPoint3 = (signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;

         // get signal samples
         float signalData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].filteredValue;

         // store signal in filter buffer removing DC and rectified
         modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)] = signalData * signalData * 10;

         // integrate symbol (moving average)
         modulation->filterIntegrate += modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->filterIntegrate -= modulation->integrationData[delay2Index & (BUFFER_SIZE - 1)]; // remove delayed value

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation results for each symbol and distance
         float correlatedS0 = modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2];
         float correlatedS1 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3];
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1);

         if (decoder->debug)
         {
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]);
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 1, modulation->filterIntegrate);
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 2, correlatedS0);

            if (decoder->signalClock == modulation->searchSyncTime)
               decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 2, 0.50f);
         }

         // wait until correlation search start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         // detect maximum symbol correlation
         if (correlatedSD > modulation->correlatedPeakValue)
         {
            modulation->correlatedPeakValue = correlatedSD;
            modulation->correlatedPeakTime = decoder->signalClock;
         }

         // capture symbol correlation values at synchronization point
         if (decoder->signalClock == modulation->searchSyncTime)
         {
            modulation->searchCorrDValue = correlatedSD;
            modulation->searchCorr0Value = correlatedS0;
            modulation->searchCorr1Value = correlatedS1;
         }

         // wait until correlation search finish
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         if (modulation->searchCorrDValue > modulation->searchValueThreshold)
         {
            modulation->symbolStartTime = modulation->symbolEndTime;
            modulation->symbolEndTime = modulation->correlatedPeakTime;
            modulation->searchValueThreshold = modulation->correlatedPeakValue * 0.25f;

            if (modulation->searchCorr0Value > modulation->searchCorr1Value)
            {
               modulation->symbolRiseTime = modulation->searchSyncTime;

               symbolStatus.value = 0;
               symbolStatus.pattern = PatternE;
            }
            else
            {
               modulation->symbolRiseTime = modulation->searchSyncTime - bitrate->period2SymbolSamples;

               symbolStatus.value = 1;
               symbolStatus.pattern = PatternD;
            }
         }
         else
         {
            modulation->symbolStartTime = modulation->symbolEndTime;
            modulation->symbolEndTime = modulation->searchSyncTime;
            modulation->symbolRiseTime = 0;

            // no modulation (End Of Frame) EoF
            symbolStatus.pattern = PatternF;
         }

         // next timing search window
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // setup symbol info
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.edge = modulation->symbolRiseTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;

         return symbolStatus.pattern;
      }

      return
         Invalid;
   }

   /*
    * Decode SOF for BPSK modulated listen frame
    */

   int decodeListenFrameStartBpsk(hw::SignalBuffer &buffer)
   {
      NfcBitrateParams *bitrate = decoder->bitrate;
      NfcModulationStatus *modulation = decoder->modulation;

      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay1Index = (bitrate->offsetDelay1Index + decoder->signalClock);
      unsigned int delay4Index = (bitrate->offsetDelay4Index + decoder->signalClock);
      unsigned int futureIndex = (bitrate->offsetFutureIndex + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++futureIndex;
         ++signalIndex;
         ++delay1Index;
         ++delay4Index;

         // get signal samples
         float signalData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].filteredValue;
         float delay1Data = decoder->sample[delay1Index & (BUFFER_SIZE - 1)].filteredValue;
         float signalDeep = decoder->sample[futureIndex & (BUFFER_SIZE - 1)].modulateDepth;

         // multiply 1 symbol delayed signal with incoming signal, (magic number 10 must be signal dependent, but i don't how...)
         modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)] = signalData * delay1Data * 10;

         if (decoder->debug)
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]);

         // wait until frame guard time (TR0)
         if (decoder->signalClock < frameStatus.guardEnd)
            continue;

         // using minimum signal st.dev as lower level threshold scaled to 1/4 symbol to compensate integration
         if (decoder->signalClock == frameStatus.guardEnd)
            modulation->searchValueThreshold = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].meanDeviation;

         // check if frame waiting time exceeded without detect modulation
         if (decoder->signalClock > frameStatus.waitingEnd)
            return NoPattern;

         // check if poll frame modulation is detected while waiting for response
         if (signalDeep > minimumModulationDeep)
            return NoPattern;

         // compute phase integration after guard end
         modulation->phaseIntegrate += modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->phaseIntegrate -= modulation->integrationData[delay4Index & (BUFFER_SIZE - 1)]; // remove delayed value

         if (decoder->debug)
         {
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 1, modulation->phaseIntegrate);

            if (decoder->signalClock < (frameStatus.guardEnd + 100))
               decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 1, modulation->searchValueThreshold);
         }

         // detect first zero-cross
         if (modulation->phaseIntegrate > modulation->searchValueThreshold)
         {
            if (!modulation->symbolStartTime)
               modulation->symbolStartTime = decoder->signalClock;

            modulation->searchEndTime = decoder->signalClock + bitrate->period2SymbolSamples;
         }

         // detect preamble is received, 32 subcarrier clocks (4 ETU)
         if (!modulation->symbolEndTime && (modulation->phaseIntegrate < 0 || decoder->signalClock == modulation->searchEndTime))
         {
            int preambleSyncLength = decoder->signalClock - modulation->symbolStartTime;

            if (preambleSyncLength < decoder->signalParams.elementaryTimeUnit * 3 || // preamble too short
               preambleSyncLength > decoder->signalParams.elementaryTimeUnit * 4) // preamble too long
            {
               modulation->symbolStartTime = 0;
               modulation->symbolEndTime = 0;
               modulation->searchEndTime = 0;
               continue;
            }

            // set symbol end time
            modulation->symbolEndTime = modulation->searchEndTime + bitrate->period2SymbolSamples;
         }

         // wait until correlation search finish or detect zero cross
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         if (decoder->debug)
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 1, 0.75);

         // set next synchronization point
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period2SymbolSamples;
         modulation->searchLastPhase = modulation->phaseIntegrate;
         modulation->searchPhaseThreshold = std::fabs(modulation->phaseIntegrate * 0.25f);

         // clear edge transition detector
         modulation->detectorPeakTime = 0;

         // set symbol info
         symbolStatus.value = 0;
         symbolStatus.start = modulation->symbolStartTime - bitrate->period1SymbolSamples - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->period1SymbolSamples - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;
         symbolStatus.pattern = PatternS;

         return symbolStatus.pattern;
      }

      return Invalid;
   }

   /*
    * Decode one BPSK modulated listen frame symbol
    */
   int decodeListenFrameSymbolBpsk(hw::SignalBuffer &buffer)
   {
      NfcBitrateParams *bitrate = decoder->bitrate;
      NfcModulationStatus *modulation = decoder->modulation;

      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay1Index = (bitrate->offsetDelay1Index + decoder->signalClock);
      unsigned int delay4Index = (bitrate->offsetDelay4Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;
         ++delay1Index;
         ++delay4Index;

         // get signal samples
         float signalData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].filteredValue;
         float delay1Data = decoder->sample[delay1Index & (BUFFER_SIZE - 1)].filteredValue;

         // multiply 1 symbol delayed signal with incoming signal
         modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)] = signalData * delay1Data * 10;

         // integrate
         modulation->phaseIntegrate += modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->phaseIntegrate -= modulation->integrationData[delay4Index & (BUFFER_SIZE - 1)]; // remove delayed value

         if (decoder->debug)
         {
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]);
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 1, modulation->phaseIntegrate);
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 2, modulation->searchValueThreshold);
         }

         // zero-cross detector for re-synchronization, only one time for each symbol to avoid oscillations!
         if (!modulation->detectorPeakTime)
         {
            if ((modulation->phaseIntegrate > 0 && modulation->searchLastPhase < 0) || (modulation->phaseIntegrate < 0 && modulation->searchLastPhase > 0))
            {
               modulation->detectorPeakTime = decoder->signalClock;
               modulation->searchSyncTime = decoder->signalClock + bitrate->period2SymbolSamples;
               modulation->searchLastPhase = modulation->phaseIntegrate;
            }
         }

         // wait until synchronization point is reached
         if (decoder->signalClock != modulation->searchSyncTime)
            continue;

         if (decoder->debug)
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 1, 0.50f);

         // no modulation detected, generate End Of Frame
         if (std::abs(modulation->phaseIntegrate) < std::abs(modulation->searchPhaseThreshold))
            return PatternO;

         // set symbol timings
         modulation->symbolStartTime = modulation->symbolEndTime;
         modulation->symbolEndTime = modulation->searchSyncTime + bitrate->period2SymbolSamples;

         // next synchronization point
         modulation->searchSyncTime = modulation->searchSyncTime + bitrate->period1SymbolSamples;
         modulation->searchLastPhase = modulation->phaseIntegrate;

         // clear edge transition detector
         modulation->detectorPeakTime = 0;

         // symbol change, invert pattern and value
         if (modulation->phaseIntegrate < -modulation->searchPhaseThreshold)
         {
            symbolStatus.value = !symbolStatus.value;
            symbolStatus.pattern = (symbolStatus.pattern == PatternM) ? PatternN : PatternM;
         }
         else
         {
            // update threshold for next symbol
            modulation->searchPhaseThreshold = modulation->phaseIntegrate * 0.25f;
         }

         // setup symbol info
         symbolStatus.start = modulation->symbolStartTime - bitrate->period1SymbolSamples - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->period1SymbolSamples - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;

         return symbolStatus.pattern;
      }

      return Invalid;
   }

   /*
    * Reset frame search status
    */
   void resetFrameSearch()
   {
      // reset frame search status
      if (decoder->modulation)
      {
         decoder->modulation->symbolStartTime = 0;
         decoder->modulation->symbolEndTime = 0;
         decoder->modulation->symbolRiseTime = 0;
         decoder->modulation->searchSyncTime = 0;
         decoder->modulation->searchStartTime = 0;
         decoder->modulation->searchEndTime = 0;
         decoder->modulation->searchPulseWidth = 0;
         decoder->modulation->correlatedPeakTime = 0;
         decoder->modulation->correlatedPeakValue = 0;
         decoder->modulation->detectorPeakTime = 0;
         decoder->modulation->detectorPeakValue = 0;
      }

      // reset frame start time
      frameStatus.frameStart = 0;
   }

   /*
    * Reset modulation status
    */
   void resetModulation()
   {
      // reset modulation status for all rates
      for (int rate = r106k; rate <= r424k; rate++)
      {
         modulationStatus[rate] = {};
      }

      // clear stream status
      streamStatus = {};

      // clear stream status
      symbolStatus = {};

      // clear frame status
      frameStatus.frameType = 0;
      frameStatus.frameStart = 0;
      frameStatus.frameEnd = 0;

      // restore bitrate
      decoder->bitrate = nullptr;

      // restore modulation
      decoder->modulation = nullptr;
   }

   /*
    * Process request or response frame
    */
   void process(RawFrame &frame)
   {
      // for request frame set default response timings, must be overridden by subsequent process functions
      if (frame.frameType() == NfcPollFrame)
      {
         // initialize frame parameters to default protocol parameters
         frameStatus.startUpGuardTime = protocolStatus.startUpGuardTime;
         frameStatus.frameWaitingTime = protocolStatus.frameWaitingTime;
         frameStatus.frameGuardTime = protocolStatus.frameGuardTime;
         frameStatus.requestGuardTime = protocolStatus.requestGuardTime;
      }
      // for response frames only set frame guard time before receive next poll frame
      else
      {
         // initialize frame parameters to default protocol parameters
         frameStatus.frameGuardTime = protocolStatus.frameGuardTime;
      }

      do
      {
         if (processREQA(frame))
            break;

         if (processHLTA(frame))
            break;

         if (!(chainedFlags & Encrypted))
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
            // parity is calculated over decrypted data, so disable flag for encrypted frames
            frame.clearFrameFlags(ParityError);

            // set application frame phase
            frame.setFramePhase(NfcApplicationPhase);
         }
      }
      while (false);

      // set chained flags
      frame.setFrameFlags(chainedFlags);

      // for request frame set response timings
      if (frame.frameType() == NfcPollFrame)
      {
         // update frame timing parameters for receive PICC frame
         if (decoder->bitrate)
         {
            // response guard time TR0min (PICC must not modulate response within this period)
            frameStatus.guardEnd = frameStatus.frameEnd + frameStatus.frameGuardTime + decoder->bitrate->symbolDelayDetect;

            // response delay time WFT (PICC must reply to command before this period)
            frameStatus.waitingEnd = frameStatus.frameEnd + frameStatus.frameWaitingTime + decoder->bitrate->symbolDelayDetect;

            // next frame must be ListenFrame
            frameStatus.frameType = NfcListenFrame;
         }
      }
      else
      {
         // update frame timing parameters for receive next PCD frame
         if (decoder->bitrate)
         {
            // poll frame guard time (PCD must not modulate within this period)
            frameStatus.guardEnd = frameStatus.frameEnd + frameStatus.frameGuardTime + decoder->bitrate->symbolDelayDetect;
         }

         // switch to modulation search
         frameStatus.frameType = 0;

         // reset frame command
         frameStatus.lastCommand = 0;
      }

      // mark last processed frame
      lastFrameEnd = frameStatus.frameEnd;

      // reset frame start
      frameStatus.frameStart = 0;

      // reset frame end
      frameStatus.frameEnd = 0;
   }

   /*
    * Process REQA frame
    */
   bool processREQA(RawFrame &frame)
   {
      if (frame.frameType() == NfcPollFrame)
      {
         if ((frame[0] == NFCA_REQA || frame[0] == NFCA_WUPA) && frame.limit() == 1)
         {
            frame.setFramePhase(NfcSelectionPhase);

            frameStatus.lastCommand = frame[0];

            // This commands starts or wakeup card communication, so reset the protocol parameters to the default values
            protocolStatus.maxFrameSize = 256;
            protocolStatus.startUpGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_SFGT_DEF);
            protocolStatus.frameGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_FGT_DEF);
            protocolStatus.frameWaitingTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_FWT_DEF);
            protocolStatus.requestGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_RGT_DEF);

            // The REQ-A Response must start exactly at 128 * n, n=9, decoder search between n=7 and n=18
            frameStatus.frameGuardTime = decoder->signalParams.sampleTimeUnit * NFCA_FGT_DEF; // ATQ-A response guard
            frameStatus.frameWaitingTime = decoder->signalParams.sampleTimeUnit * NFCA_FWT_ATQA; // ATQ-A response timeout

            // clear chained flags
            chainedFlags = 0;

            return true;
         }
      }

      if (frame.frameType() == NfcListenFrame)
      {
         if (frameStatus.lastCommand == NFCA_REQA || frameStatus.lastCommand == NFCA_WUPA)
         {
            frame.setFramePhase(NfcSelectionPhase);

            return true;
         }
      }

      return false;
   }

   /*
    * Process HLTA frame
    */
   bool processHLTA(RawFrame &frame)
   {
      if (frame.frameType() == NfcPollFrame)
      {
         if (frame[0] == NFCA_HLTA && frame.limit() == 4 && !frame.hasFrameFlags(CrcError))
         {
            frame.setFramePhase(NfcSelectionPhase);
            frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);

            frameStatus.lastCommand = frame[0];

            // After this command the PICC will stop and will not respond, set the protocol parameters to the default values
            protocolStatus.maxFrameSize = 256;
            protocolStatus.startUpGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_SFGT_DEF);
            protocolStatus.frameGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_FGT_DEF);
            protocolStatus.frameWaitingTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_FWT_DEF);
            protocolStatus.requestGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_RGT_DEF);

            // clear chained flags
            chainedFlags = 0;

            // reset modulation status
            resetModulation();

            return true;
         }
      }

      return false;
   }

   /*
    * Process Selection frame
    */
   bool processSELn(RawFrame &frame)
   {
      if (frame.frameType() == NfcPollFrame)
      {
         if (frame[0] == NFCA_SEL1 || frame[0] == NFCA_SEL2 || frame[0] == NFCA_SEL3)
         {
            frame.setFramePhase(NfcSelectionPhase);

            frameStatus.lastCommand = frame[0];

            // The selection commands has same timings as REQ-A
            frameStatus.frameGuardTime = decoder->signalParams.sampleTimeUnit * NFCA_FGT_DEF;
            frameStatus.frameWaitingTime = decoder->signalParams.sampleTimeUnit * NFCA_FWT_ATQA;

            return true;
         }
      }

      if (frame.frameType() == NfcListenFrame)
      {
         if (frameStatus.lastCommand == NFCA_SEL1 || frameStatus.lastCommand == NFCA_SEL2 || frameStatus.lastCommand == NFCA_SEL3)
         {
            frame.setFramePhase(NfcSelectionPhase);

            return true;
         }
      }

      return false;
   }

   /*
    * Process RAST frame
    */
   bool processRATS(RawFrame &frame)
   {
      // capture parameters from ATS and reconfigure decoder timings
      if (frame.frameType() == NfcPollFrame)
      {
         if (frame[0] == NFCA_RATS)
         {
            int fsdi = (frame[1] >> 4) & 0x0F;

            frameStatus.lastCommand = frame[0];

            // sets maximum frame length requested by reader
            protocolStatus.maxFrameSize = NFC_FDS_TABLE[fsdi];

            // sets the activation frame waiting time for ATS response
            frameStatus.frameWaitingTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFC_FWT_ACTIVATION);

            log->debug("RATS frame parameters");
            log->debug("  maxFrameSize {} bytes", {protocolStatus.maxFrameSize});

            // set frame flags
            frame.setFramePhase(NfcSelectionPhase);
            frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);

            return true;
         }
      }

      if (frame.frameType() == NfcListenFrame)
      {
         if (frameStatus.lastCommand == NFCA_RATS)
         {
            int offset = 0;
            int tl = frame[offset++];

            if (tl > 0)
            {
               int t0 = frame[offset++];

               // if TA is transmitted, skip...
               if (t0 & 0x10)
                  offset++;

               // if TB is transmitted capture timing parameters
               if (t0 & 0x20)
               {
                  int tb = frame[offset++];

                  // get Start-up Frame Guard time Integer
                  int sfgi = (tb & 0x0f);

                  // get Frame Waiting Time Integer
                  int fwi = (tb >> 4) & 0x0f;

                  // A received value of SFGI = 15 MUST be treated by the NFC Forum Device as SFGI = 0.
                  if (sfgi == 15)
                     sfgi = 0;

                  // A received value of FWI = 15 MUST be treated by the NFC Forum Device as FWI = 4.
                  if (fwi == 15)
                     fwi = 4;

                  // calculate timing parameters
                  protocolStatus.startUpGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFC_SFGT_TABLE[sfgi]);
                  protocolStatus.frameWaitingTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFC_FWT_TABLE[fwi]);
               }
               else
               {
                  // if TB is not transmitted establish default timing parameters
                  protocolStatus.startUpGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_SFGT_DEF);
                  protocolStatus.frameWaitingTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCA_FWT_DEF);
               }

               log->debug("ATS protocol timing parameters");
               log->debug("  startUpGuardTime {} samples ({} us)", {protocolStatus.startUpGuardTime, 1000000.0 * protocolStatus.startUpGuardTime / decoder->sampleRate});
               log->debug("  frameWaitingTime {} samples ({} us)", {protocolStatus.frameWaitingTime, 1000000.0 * protocolStatus.frameWaitingTime / decoder->sampleRate});
            }

            frame.setFramePhase(NfcSelectionPhase);
            frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);

            return true;
         }
      }

      return false;
   }

   /*
    * Process PPS frame
    */
   bool processPPSr(RawFrame &frame)
   {
      if (frame.frameType() == NfcPollFrame)
      {
         if ((frame[0] & 0xF0) == NFCA_PPS)
         {
            frameStatus.lastCommand = frame[0] & 0xF0;

            frame.setFramePhase(NfcSelectionPhase);
            frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);

            return true;
         }
      }

      if (frame.frameType() == NfcListenFrame)
      {
         if (frameStatus.lastCommand == NFCA_PPS)
         {
            frame.setFramePhase(NfcSelectionPhase);
            frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);

            return true;
         }
      }

      return false;
   }

   /*
    * Process Mifare Classic AUTH frame
    */
   bool processAUTH(RawFrame &frame)
   {
      if (frame.frameType() == NfcPollFrame)
      {
         if (frame[0] == NFCA_AUTH1 || frame[0] == NFCA_AUTH2)
         {
            frameStatus.lastCommand = frame[0];
            //         frameStatus.frameWaitingTime = static_cast<int>(signalParams.sampleTimeUnit * 256 * 16 * (1 << 14);

            frame.setFramePhase(NfcApplicationPhase);
            frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);

            //      if (!frameStatus.lastCommand)
            //      {
            //         crypto1_init(&frameStatus.crypto1, 0x521284223154);
            //      }

            return true;
         }
      }

      if (frame.frameType() == NfcListenFrame)
      {
         if (frameStatus.lastCommand == NFCA_AUTH1 || frameStatus.lastCommand == NFCA_AUTH2)
         {
            //      unsigned int uid = 0x1e47fc35;
            //      unsigned int nc = frame[0] << 24 || frame[1] << 16 || frame[2] << 8 || frame[0];
            //
            //      crypto1_word(&frameStatus.crypto1, uid ^ nc, 0);

            // set chained flags

            chainedFlags = Encrypted;

            frame.setFramePhase(NfcApplicationPhase);

            return true;
         }
      }

      return false;
   }

   /*
    * Process ISO-14443 I-Block frame
    */
   bool processIBlock(RawFrame &frame)
   {
      if (frame.frameType() == NfcPollFrame)
      {
         if ((frame[0] & 0xE2) == NFCA_IBLOCK && frame.limit() > 4)
         {
            frameStatus.lastCommand = frame[0] & 0xE2;

            frame.setFramePhase(NfcApplicationPhase);
            frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);

            return true;
         }
      }

      if (frame.frameType() == NfcListenFrame)
      {
         if (frameStatus.lastCommand == NFCA_IBLOCK)
         {
            frame.setFramePhase(NfcApplicationPhase);
            frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);

            return true;
         }
      }

      return false;
   }

   /*
    * Process ISO-14443 R-Block frame
    */
   bool processRBlock(RawFrame &frame)
   {
      if (frame.frameType() == NfcPollFrame)
      {
         if ((frame[0] & 0xE6) == NFCA_RBLOCK && frame.limit() == 3)
         {
            frameStatus.lastCommand = frame[0] & 0xE6;

            frame.setFramePhase(NfcApplicationPhase);
            frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);

            return true;
         }
      }

      if (frame.frameType() == NfcListenFrame)
      {
         if (frameStatus.lastCommand == NFCA_RBLOCK)
         {
            frame.setFramePhase(NfcApplicationPhase);
            frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);

            return true;
         }
      }

      return false;
   }

   /*
    * Process ISO-14443 S-Block frame
    */
   bool processSBlock(RawFrame &frame)
   {
      if (frame.frameType() == NfcPollFrame)
      {
         if ((frame[0] & 0xC7) == NFCA_SBLOCK && frame.limit() == 4)
         {
            frameStatus.lastCommand = frame[0] & 0xC7;

            frame.setFramePhase(NfcApplicationPhase);
            frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);

            return true;
         }
      }

      if (frame.frameType() == NfcListenFrame)
      {
         if (frameStatus.lastCommand == NFCA_SBLOCK)
         {
            frame.setFramePhase(NfcApplicationPhase);
            frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);

            return true;
         }
      }

      return false;
   }

   /*
    * Process other frame types
    */
   void processOther(RawFrame &frame)
   {
      frame.setFramePhase(NfcApplicationPhase);
      frame.setFrameFlags(!checkCrc(frame) ? CrcError : 0);
   }

   /*
    * Check NFC-A crc NFC-A ITU-V.41
    */
   static bool checkCrc(RawFrame &frame)
   {
      unsigned int size = frame.limit();

      if (size < 2)
         return true;

      unsigned short crc = Crc::ccitt16(frame.data(), 0, size - 2, 0x6363, true);
      unsigned short res = ((unsigned int)frame[size - 2] & 0xff) | ((unsigned int)frame[size - 1] & 0xff) << 8;

      return res == crc;
   }

   /*
    * Check NFC-A byte parity
    */
   static bool checkParity(unsigned int value, unsigned int parity)
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
};

NfcA::NfcA(NfcDecoderStatus *decoder) : self(new Impl(decoder))
{
}

NfcA::~NfcA()
{
   delete self;
}

float NfcA::modulationThresholdMin() const
{
   return self->minimumModulationDeep;
}

float NfcA::modulationThresholdMax() const
{
   return self->maximumModulationDeep;
}

void NfcA::setModulationThreshold(float min, float max)
{
   if (!std::isnan(min))
      self->minimumModulationDeep = min;

   if (!std::isnan(max))
      self->maximumModulationDeep = max;
}

float NfcA::correlationThreshold() const
{
   return self->correlationThreshold;
}

void NfcA::setCorrelationThreshold(float value)
{
   if (!std::isnan(value))
      self->correlationThreshold = value;
}

/*
 * Configure NFC-A modulation
 */
void NfcA::initialize(unsigned int sampleRate)
{
   self->initialize(sampleRate);
}

/*
 * Detect NFC-A modulation
 */
bool NfcA::detect()
{
   return self->detectModulation();
}

/*
 * Decode next poll or listen frame
 */
void NfcA::decode(hw::SignalBuffer &samples, std::list<RawFrame> &frames)
{
   self->decodeFrame(samples, frames);
}

}

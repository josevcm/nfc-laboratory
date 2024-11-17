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

#include <cstring>

#include <rt/Logger.h>

#include <lab/nfc/Nfc.h>

#include <tech/NfcF.h>

#define SEARCH_MODE_OBSERVED 0
#define SEARCH_MODE_REVERSED 1

namespace lab {

enum PatternType
{
   Invalid = 0,
   NoPattern = 1,
   PatternL = 2,
   PatternH = 3,
   PatternS = 4,
   PatternE = 5
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

struct NfcF::Impl : NfcTech
{
   rt::Logger *log = rt::Logger::getLogger("decoder.NfcF");

   NfcDecoderStatus *decoder;

   // bitrate parameters
   NfcBitrateParams bitrateParams[4] {};

   // detected symbol status
   NfcSymbolStatus symbolStatus {};

   // bit stream status
   NfcStreamStatus streamStatus {};

   // frame processing status
   NfcFrameStatus frameStatus {};

   // protocol processing status
   NfcProtocolStatus protocolStatus {};

   // modulation status for each bitrate
   NfcModulationStatus modulationStatus[4] {};

   // minimum modulation threshold to detect valid signal for NFC-F (default 10%)
   float minimumModulationDeep = 0.10f;

   // minimum modulation threshold to detect valid signal for NFC-F (default 75%)
   float maximumModulationDeep = 0.90f;

   // minimum correlation threshold to detect valid NFC-F pulse (default 10%)
   float correlationThreshold = 0.50f;

   // last detected frame end
   unsigned int lastFrameEnd = 0;

   // chained frame flags
   unsigned int chainedFlags = 0;

   Impl(NfcDecoderStatus *decoder) : decoder(decoder)
   {
   }

   inline void initialize(unsigned int sampleRate)
   {
      log->info("--------------------------------------------");
      log->info("initializing NFC-F decoder");
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

      // compute symbol parameters for 212Kbps and 424Kbps
      for (int rate = r212k; rate <= r424k; rate++)
      {
         // clear bitrate parameters
         bitrateParams[rate] = {};

         // clear modulation parameters
         modulationStatus[rate] = {};

         // configure bitrate parametes
         NfcBitrateParams *bitrate = bitrateParams + rate;

         // set tech type and rate
         bitrate->techType = FrameTech::NfcFTech;
         bitrate->rateType = rate;

         // symbol timing parameters
         bitrate->symbolsPerSecond = int(std::round(NFC_FC / float(128 >> rate)));

         // number of samples per symbol
         bitrate->period0SymbolSamples = static_cast<int>(std::round(decoder->signalParams.sampleTimeUnit * (256 >> rate))); // double symbol samples
         bitrate->period1SymbolSamples = static_cast<int>(std::round(decoder->signalParams.sampleTimeUnit * (128 >> rate))); // full symbol samples
         bitrate->period2SymbolSamples = static_cast<int>(std::round(decoder->signalParams.sampleTimeUnit * (64 >> rate))); // half symbol samples
         bitrate->period4SymbolSamples = static_cast<int>(std::round(decoder->signalParams.sampleTimeUnit * (32 >> rate))); // quarter of symbol...
         bitrate->period8SymbolSamples = static_cast<int>(std::round(decoder->signalParams.sampleTimeUnit * (16 >> rate))); // and so on...
         bitrate->preamble1Samples = static_cast<int>(std::round(decoder->signalParams.sampleTimeUnit * (128 >> rate) * 48)); // preamble length, 48 symbol-0

         // delay guard for each symbol rate
         bitrate->symbolDelayDetect = 0;

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
      protocolStatus.startUpGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCF_SFGT_DEF);
      protocolStatus.frameGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCF_FGT_DEF);
      protocolStatus.frameWaitingTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCF_FWT_DEF);
      protocolStatus.requestGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCF_RGT_DEF);

      // initialize frame parameters to default protocol parameters
      frameStatus.startUpGuardTime = protocolStatus.startUpGuardTime;
      frameStatus.frameWaitingTime = protocolStatus.frameWaitingTime;
      frameStatus.frameGuardTime = protocolStatus.frameGuardTime;
      frameStatus.requestGuardTime = protocolStatus.requestGuardTime;

      log->debug("Startup parameters");
      log->debug("\tmaxFrameSize {} bytes", {protocolStatus.maxFrameSize});
      log->debug("\tframeGuardTime {} samples ({} us)", {protocolStatus.frameGuardTime, 1000000.0 * protocolStatus.frameGuardTime / decoder->sampleRate});
      log->debug("\tframeWaitingTime {} samples ({} us)", {protocolStatus.frameWaitingTime, 1000000.0 * protocolStatus.frameWaitingTime / decoder->sampleRate});
      log->debug("\trequestGuardTime {} samples ({} us)", {protocolStatus.requestGuardTime, 1000000.0 * protocolStatus.requestGuardTime / decoder->sampleRate});
   }

   inline bool detectModulation()
   {
      // wait until has enough data in buffer
      if (decoder->signalClock < BUFFER_SIZE)
         return false;

      // ignore low power signals
      if (decoder->signalEnvelope < decoder->powerLevelThreshold)
         return false;

      // minimum correlation value for valid NFC-F symbols
      float minimumCorrelationValue = decoder->signalEnvelope * correlationThreshold;

      // POLL frame ASK detector for 212Kbps and 424Kbps
      for (int rate = r212k; rate <= r424k; rate++)
      {
         NfcBitrateParams *bitrate = bitrateParams + rate;
         NfcModulationStatus *modulation = modulationStatus + rate;

         //  signal pointers
         unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
         unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

         // correlation pointers
         unsigned int filterPoint1 = (signalIndex % bitrate->period1SymbolSamples);
         unsigned int filterPoint2 = (signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
         unsigned int filterPoint3 = (signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;

         // get signal samples
         float signalData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].samplingValue;
         float delay2Data = decoder->sample[delay2Index & (BUFFER_SIZE - 1)].samplingValue;
         float signalDeep = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].modulateDepth;

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += signalData; // add new value
         modulation->filterIntegrate -= delay2Data; // remove delayed value

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation factors
         float correlatedS0 = (modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2]);
         float correlatedS1 = (modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3]);
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1) / float(bitrate->period2SymbolSamples);

         if (decoder->debug)
         {
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, correlatedS0 / float(bitrate->period4SymbolSamples));

            if (decoder->signalClock == modulation->searchSyncTime && modulation->searchPulseWidth % 8 == 0)
               decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, 0.50f);
         }

         // recover status from previous partial search or maximum modulation depth
         if (signalDeep > maximumModulationDeep || (modulation->correlatedPeakTime && decoder->signalClock > modulation->correlatedPeakTime + bitrate->period1SymbolSamples))
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

         // wait until search start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         if (correlatedSD > minimumCorrelationValue)
         {
            // detect modulation peaks
            if (correlatedSD > modulation->correlatedPeakValue)
            {
               modulation->correlatedPeakValue = correlatedSD;
               modulation->correlatedPeakTime = decoder->signalClock;

               // for first symbol using a moving window and set initial sync values
               if (!modulation->searchSyncTime)
               {
                  modulation->searchSyncValue = correlatedSD; // correlation value at peak
                  modulation->searchCorr0Value = correlatedS0; // symbol correlation reference for first pulse
                  modulation->searchEndTime = decoder->signalClock + bitrate->period8SymbolSamples;
               }
            }
         }

         // capture correlation value at synchronization points
         if (decoder->signalClock == modulation->searchSyncTime)
         {
            modulation->searchSyncValue = correlatedSD;
            modulation->searchLastValue = correlatedS0;
         }

         // wait until search finish
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // first at least 94 pulses for NFC-F preamble
         if (modulation->searchPulseWidth++ < 94)
         {
            // check for valid pulse
            if (modulation->correlatedPeakTime == 0 || // no modulation found
               //                      modulation->detectorPeakValue < minimumModulationDeep || // insufficient modulation deep
               //                      modulation->detectorPeakValue > maximumModulationDeep || // excessive modulation deep
               modulation->searchSyncValue < modulation->searchValueThreshold) // pulse too low
            {
               // reset modulation to continue search
               modulation->symbolStartTime = 0;
               modulation->symbolEndTime = 0;
               modulation->searchSyncTime = 0;
               modulation->searchSyncValue = 0;
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;
               modulation->searchPulseWidth = 0;
               modulation->searchValueThreshold = 0;
               modulation->correlatedPeakValue = 0;
               modulation->correlatedPeakTime = 0;
               continue;
            }
         }

         // wait until detect modulation change between preamble and synchronization bytes
         if (modulation->searchSyncValue > modulation->searchValueThreshold)
         {
            if (!modulation->symbolStartTime)
               modulation->symbolStartTime = modulation->correlatedPeakTime - bitrate->period2SymbolSamples;

            // update symbol end time
            modulation->symbolEndTime = modulation->correlatedPeakTime;

            // update search for next synchronization point
            modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period2SymbolSamples;
            modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
            modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
            modulation->searchValueThreshold = modulation->correlatedPeakValue / 2;
            modulation->searchLastPhase = modulation->searchLastValue;

            // reset correlation marks to continue search
            modulation->correlatedPeakTime = 0;
            modulation->correlatedPeakValue = 0;

            continue;
         }

         // detect polarity and compensate frame length
         if ((modulation->searchLastPhase < 0 && modulation->searchCorr0Value < 0) || (modulation->searchLastPhase > 0 && modulation->searchCorr0Value > 0))
            modulation->symbolStartTime -= bitrate->period2SymbolSamples;

         // now check preamble length with +-1/4 symbol tolerance
         int preambleLength = modulation->symbolEndTime - modulation->symbolStartTime;
         int preambleMinLength = bitrate->preamble1Samples - bitrate->period4SymbolSamples;
         int preambleMaxLength = bitrate->preamble1Samples + bitrate->period4SymbolSamples;

         if (preambleLength < preambleMinLength || // preamble to short
            preambleLength > preambleMaxLength) // preamble to long
         {
            // reset modulation to continue search
            modulation->symbolStartTime = 0;
            modulation->symbolEndTime = 0;
            modulation->searchSyncTime = 0;
            modulation->searchSyncValue = 0;
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->searchPulseWidth = 0;
            modulation->searchValueThreshold = 0;
            modulation->correlatedPeakValue = 0;
            modulation->correlatedPeakTime = 0;
            continue;
         }

         if (decoder->debug)
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, 0.75f);

         modulation->searchModeState = modulation->searchLastPhase > 0 ? SEARCH_MODE_OBSERVED : SEARCH_MODE_REVERSED;
         modulation->searchSyncTime = modulation->searchSyncTime + bitrate->period2SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period4SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period4SymbolSamples;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // setup symbol info
         symbolStatus.start = modulation->symbolStartTime;
         symbolStatus.end = modulation->symbolEndTime;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;
         symbolStatus.pattern = PatternType::PatternS;

         // setup frame info
         frameStatus.frameType = NfcPollFrame;
         frameStatus.symbolRate = bitrate->symbolsPerSecond;
         frameStatus.frameStart = symbolStatus.start;
         frameStatus.frameEnd = 0;

         decoder->bitrate = bitrate;
         decoder->modulation = modulation;

         return true;
      }

      return false;
   }

   inline

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
   inline bool decodePollFrame(hw::SignalBuffer &buffer, std::list<RawFrame> &frames)
   {
      int pattern;
      bool frameEnd = false, truncateError = false;

      // decode remaining request frame
      while ((pattern = decodePollFrameSymbolAsk(buffer)) > PatternType::NoPattern)
      {
         // frame ends if no found manchester transition (PatternE)
         if (pattern == PatternType::PatternE)
            frameEnd = true;

            // frame ends width truncate error max frame size is reached
         else if (streamStatus.bytes == protocolStatus.maxFrameSize)
            truncateError = true;

         // detect end of frame
         if (frameEnd || truncateError)
         {
            // a valid frame must contain at least two bytes of data
            if (streamStatus.bytes > 2)
            {
               // set last symbol timing
               frameStatus.frameEnd = symbolStatus.end;

               RawFrame request = RawFrame(FrameTech::NfcFTech, FrameType::NfcPollFrame);

               request.setFrameRate(decoder->bitrate->symbolsPerSecond);
               request.setSampleStart(frameStatus.frameStart);
               request.setSampleEnd(frameStatus.frameEnd);
               request.setSampleRate(decoder->sampleRate);
               request.setTimeStart(static_cast<double>(frameStatus.frameStart) / static_cast<double>(decoder->sampleRate));
               request.setTimeEnd(static_cast<double>(frameStatus.frameEnd) / static_cast<double>(decoder->sampleRate));
               request.setDateTime(decoder->streamTime + request.timeStart());

               if (truncateError)
                  request.setFrameFlags(FrameFlags::Truncated);

               // check synchronization bytes, must be 0xB24D
               if (streamStatus.buffer[0] != 0xB2 || streamStatus.buffer[1] != 0x4D)
                  request.setFrameFlags(FrameFlags::SyncError);

               // add bytes to frame and flip to prepare read
               request.put(streamStatus.buffer + 2, streamStatus.bytes - 2).flip();

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

               return true;
            }

            // reset modulation and restart frame detection
            resetModulation();

            // no valid frame found
            return false;
         }

         // decode next bit
         streamStatus.data = (streamStatus.data << 1) | symbolStatus.value;

         // store full byte in stream buffer
         if (++streamStatus.bits == 8)
         {
            streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;
            streamStatus.data = 0;
            streamStatus.bits = 0;
         }
      }

      // no frame detected
      return false;
   }

   /*
    * Decode next listen frame
    */
   inline bool decodeListenFrame(hw::SignalBuffer &buffer, std::list<RawFrame> &frames)
   {
      int pattern;
      bool frameEnd = false, truncateError = false;

      if (!frameStatus.frameStart)
      {
         // detect SOF pattern
         pattern = decodeListenFrameStartAsk(buffer);

         // Pattern-S found, mark frame start time
         if (pattern == PatternType::PatternS)
         {
            frameStatus.frameStart = symbolStatus.start;
         }
         else
         {
            //  end of frame waiting time, restart modulation search
            if (pattern == PatternType::NoPattern)
               resetModulation();

            // no frame found
            return false;
         }
      }

      // frame SoF detected, decode frame stream...
      if (frameStatus.frameStart)
      {
         while ((pattern = decodeListenFrameSymbolAsk(buffer)) > PatternType::NoPattern)
         {
            // frame ends if no found manchester transition (PatternE)
            if (pattern == PatternType::PatternE)
               frameEnd = true;

               // frame ends width truncate error max frame size is reached
            else if (streamStatus.bytes == protocolStatus.maxFrameSize)
               truncateError = true;

            // detect end of frame
            if (frameEnd || truncateError)
            {
               // a valid frame must contain at least one byte of data
               if (streamStatus.bytes > 2)
               {
                  // set last symbol timing
                  frameStatus.frameEnd = symbolStatus.end;

                  // build response frame
                  RawFrame response = RawFrame(FrameTech::NfcFTech, FrameType::NfcListenFrame);

                  response.setFrameRate(decoder->bitrate->symbolsPerSecond);
                  response.setSampleStart(frameStatus.frameStart);
                  response.setSampleEnd(frameStatus.frameEnd);
                  response.setSampleRate(decoder->sampleRate);
                  response.setTimeStart(static_cast<double>(frameStatus.frameStart) / static_cast<double>(decoder->sampleRate));
                  response.setTimeEnd(static_cast<double>(frameStatus.frameEnd) / static_cast<double>(decoder->sampleRate));
                  response.setDateTime(decoder->streamTime + response.timeStart());

                  if (truncateError)
                     response.setFrameFlags(FrameFlags::Truncated);

                  // check synchronization bytes, must be 0xB24D
                  if (streamStatus.buffer[0] != 0xB2 || streamStatus.buffer[1] != 0x4D)
                     response.setFrameFlags(FrameFlags::SyncError);

                  // add bytes to frame and flip to prepare read
                  response.put(streamStatus.buffer + 2, streamStatus.bytes - 2).flip();

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

            // decode next bit
            streamStatus.data = (streamStatus.data << 1) | symbolStatus.value;

            // store full byte in stream buffer
            if (++streamStatus.bits == 8)
            {
               streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;
               streamStatus.data = 0;
               streamStatus.bits = 0;
            }
         }
      }

      return false;
   }

   /*
    * Decode one ASK modulated poll frame symbol
    */
   inline int decodePollFrameSymbolAsk(hw::SignalBuffer &buffer)
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

         // get signal samples
         float currentData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].samplingValue;
         float delayedData = decoder->sample[delay2Index & (BUFFER_SIZE - 1)].samplingValue;

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += currentData; // add new value
         modulation->filterIntegrate -= delayedData; // remove delayed value

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation factors
         float correlatedS0 = modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2];
         float correlatedS1 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3];
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1) / float(bitrate->period2SymbolSamples);

         if (decoder->debug)
         {
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, correlatedS0 / float(bitrate->period4SymbolSamples));

            if (decoder->signalClock == modulation->searchSyncTime)
               decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, 0.50f);
         }

         // wait until correlation search start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         // detect maximum symbol correlation
         if (correlatedSD > modulation->searchValueThreshold && correlatedSD > modulation->correlatedPeakValue)
         {
            modulation->correlatedPeakValue = correlatedSD;
            modulation->correlatedPeakTime = decoder->signalClock;
         }

         // capture symbol correlation values at synchronization point
         if (decoder->signalClock == modulation->searchSyncTime)
         {
            modulation->searchCorr0Value = correlatedS0;
            modulation->searchCorr1Value = correlatedS1;
         }

         // wait until correlation search finish
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // no modulation (End Of Frame) EoF
         if (!modulation->correlatedPeakTime)
            return PatternType::PatternE;

         // synchronize symbol start and end time
         modulation->symbolStartTime = modulation->symbolEndTime;
         modulation->symbolEndTime = modulation->correlatedPeakTime;

         // sets next search window
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period4SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period4SymbolSamples;
         modulation->searchValueThreshold = modulation->correlatedPeakValue / 2;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // setup symbol info
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;

         // detect Pattern type
         if ((modulation->searchModeState == SEARCH_MODE_OBSERVED && modulation->searchCorr0Value > modulation->searchCorr1Value) ||
            (modulation->searchModeState == SEARCH_MODE_REVERSED && modulation->searchCorr0Value < modulation->searchCorr1Value))
         {
            symbolStatus.value = 0;
            symbolStatus.pattern = PatternType::PatternL;
         }
         else
         {
            symbolStatus.value = 1;
            symbolStatus.pattern = PatternType::PatternH;
         }

         return symbolStatus.pattern;
      }

      return PatternType::Invalid;
   }

   /*
   * Decode SOF ASK modulated listen frame symbol
   */
   inline int decodeListenFrameStartAsk(hw::SignalBuffer &buffer)
   {
      NfcBitrateParams *bitrate = decoder->bitrate;
      NfcModulationStatus *modulation = decoder->modulation;

      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;
         ++delay2Index;

         // get signal samples
         float signalData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].samplingValue;
         float delay2Data = decoder->sample[delay2Index & (BUFFER_SIZE - 1)].samplingValue;

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += signalData; // add new value
         modulation->filterIntegrate -= delay2Data; // remove delayed value

         // wait until frame guard time is reached
         if (decoder->signalClock < (frameStatus.guardEnd - bitrate->period1SymbolSamples))
            continue;

         // correlation pointers
         unsigned int filterPoint1 = (signalIndex % bitrate->period1SymbolSamples);
         unsigned int filterPoint2 = (signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
         unsigned int filterPoint3 = (signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation factors
         float correlatedS0 = modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2];
         float correlatedS1 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3];
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1) / static_cast<float>(bitrate->period2SymbolSamples);

         // get signal deep
         //         float signalDeep = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].deep;

         if (decoder->debug)
         {
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, correlatedS0 / static_cast<float>(bitrate->period4SymbolSamples));

            if (decoder->signalClock == modulation->searchSyncTime && modulation->searchPulseWidth % 8 == 0)
               decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, 0.50f);
         }

         // wait until frame guard time is reached
         if (decoder->signalClock < frameStatus.guardEnd)
            continue;

         // using signal st.dev as lower level threshold
         if (decoder->signalClock == frameStatus.guardEnd)
            modulation->searchValueThreshold = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].meanDeviation * 10;

         // check for maximum response time
         if (decoder->signalClock > frameStatus.waitingEnd)
            return PatternType::NoPattern;

         // wait until search start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         if (correlatedSD >= modulation->searchValueThreshold)
         {
            // detect modulation peaks
            if (correlatedSD > modulation->correlatedPeakValue)
            {
               modulation->correlatedPeakValue = correlatedSD;
               modulation->correlatedPeakTime = decoder->signalClock;

               // for first symbol using a moving window and set initial sync values
               if (!modulation->searchSyncTime)
               {
                  modulation->searchSyncValue = correlatedSD; // correlation value at peak
                  modulation->searchCorr0Value = correlatedS0; // symbol correlation reference for first pulse
                  modulation->searchEndTime = decoder->signalClock + bitrate->period8SymbolSamples;
               }
            }
         }

         // capture correlation value at synchronization points
         if (decoder->signalClock == modulation->searchSyncTime)
         {
            modulation->searchSyncValue = correlatedSD;
            modulation->searchLastValue = correlatedS0;
         }

         // wait until search finish
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // first at least 94 pulses for NFC-F preamble
         if (modulation->searchPulseWidth++ < 94)
         {
            // check for valid pulse
            if (modulation->correlatedPeakTime == 0 || // no modulation found
               //                      modulation->detectorPeakValue < minimumModulationDeep || // insufficient modulation deep
               //                      modulation->detectorPeakValue > maximumModulationDeep || // excessive modulation deep
               modulation->searchSyncValue < modulation->searchValueThreshold) // pulse too low
            {
               // reset modulation to continue search
               modulation->symbolStartTime = 0;
               modulation->symbolEndTime = 0;
               modulation->searchSyncTime = 0;
               modulation->searchSyncValue = 0;
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;
               modulation->searchPulseWidth = 0;
               modulation->searchValueThreshold = 0;
               modulation->correlatedPeakValue = 0;
               modulation->correlatedPeakTime = 0;
               continue;
            }
         }

         // wait until detect modulation change between preamble and synchronization bytes
         if (modulation->searchSyncValue > modulation->searchValueThreshold)
         {
            if (!modulation->symbolStartTime)
               modulation->symbolStartTime = modulation->correlatedPeakTime - bitrate->period2SymbolSamples;

            // update symbol end time
            modulation->symbolEndTime = modulation->correlatedPeakTime;

            // update search for next synchronization point
            modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period2SymbolSamples;
            modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
            modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
            modulation->searchValueThreshold = modulation->correlatedPeakValue / 2;
            modulation->searchLastPhase = modulation->searchLastValue;

            // reset correlation marks to continue search
            modulation->correlatedPeakTime = 0;
            modulation->correlatedPeakValue = 0;

            continue;
         }

         // detect polarity and compensate frame length
         if ((modulation->searchLastPhase < 0 && modulation->searchCorr0Value < 0) || (modulation->searchLastPhase > 0 && modulation->searchCorr0Value > 0))
            modulation->symbolStartTime -= bitrate->period2SymbolSamples;

         // now check preamble length with +-1/4 symbol tolerance
         int preambleLength = modulation->symbolEndTime - modulation->symbolStartTime;
         int preambleMinLength = bitrate->preamble1Samples - bitrate->period4SymbolSamples;
         int preambleMaxLength = bitrate->preamble1Samples + bitrate->period4SymbolSamples;

         if (preambleLength < preambleMinLength || // preamble to short
            preambleLength > preambleMaxLength) // preamble to long
         {
            // reset modulation to continue search
            modulation->symbolStartTime = 0;
            modulation->symbolEndTime = 0;
            modulation->searchSyncTime = 0;
            modulation->searchSyncValue = 0;
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->searchPulseWidth = 0;
            modulation->searchValueThreshold = 0;
            modulation->correlatedPeakValue = 0;
            modulation->correlatedPeakTime = 0;
            continue;
         }

         if (decoder->debug)
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, 0.75f);

         modulation->searchModeState = modulation->searchLastPhase > 0 ? SEARCH_MODE_OBSERVED : SEARCH_MODE_REVERSED;
         modulation->searchSyncTime = modulation->searchSyncTime + bitrate->period2SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period4SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period4SymbolSamples;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // set symbol info
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;
         symbolStatus.pattern = PatternType::PatternS;

         return symbolStatus.pattern;
      }

      return PatternType::Invalid;
   }

   /*
   * Decode one BPSK modulated listen frame symbol
   */
   int decodeListenFrameSymbolAsk(hw::SignalBuffer &buffer)
   {
      NfcBitrateParams *bitrate = decoder->bitrate;
      NfcModulationStatus *modulation = decoder->modulation;

      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;
         ++delay2Index;

         // get signal samples
         float signalData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].samplingValue;
         float delay2Data = decoder->sample[delay2Index & (BUFFER_SIZE - 1)].samplingValue;

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += signalData; // add new value
         modulation->filterIntegrate -= delay2Data; // remove delayed value

         // correlation pointers
         unsigned int filterPoint1 = (signalIndex % bitrate->period1SymbolSamples);
         unsigned int filterPoint2 = (signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
         unsigned int filterPoint3 = (signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation factors
         float correlatedS0 = modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2];
         float correlatedS1 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3];
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1) / static_cast<float>(bitrate->period2SymbolSamples);

         if (decoder->debug)
         {
            decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, correlatedS0 / static_cast<float>(bitrate->period4SymbolSamples));

            if (decoder->signalClock == modulation->searchSyncTime)
               decoder->debug->set(DEBUG_SIGNAL_DECODER_CHANNEL + 0, 0.50f);
         }

         // wait until correlation search start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         // detect maximum symbol correlation
         if (correlatedSD > modulation->searchValueThreshold && correlatedSD > modulation->correlatedPeakValue)
         {
            modulation->correlatedPeakValue = correlatedSD;
            modulation->correlatedPeakTime = decoder->signalClock;
         }

         // capture symbol correlation values at synchronization point
         if (decoder->signalClock == modulation->searchSyncTime)
         {
            modulation->searchCorr0Value = correlatedS0;
            modulation->searchCorr1Value = correlatedS1;
         }

         // wait until correlation search finish
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // no modulation (End Of Frame) EoF
         if (!modulation->correlatedPeakTime)
            return PatternType::PatternE;

         // synchronize symbol start and end time
         modulation->symbolStartTime = modulation->symbolEndTime;
         modulation->symbolEndTime = modulation->correlatedPeakTime;

         // sets next search window
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period4SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period4SymbolSamples;
         modulation->searchValueThreshold = modulation->correlatedPeakValue / 2;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // setup symbol info
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;

         if ((modulation->searchModeState == SEARCH_MODE_OBSERVED && modulation->searchCorr0Value > modulation->searchCorr1Value) ||
            (modulation->searchModeState == SEARCH_MODE_REVERSED && modulation->searchCorr0Value < modulation->searchCorr1Value))
         {
            symbolStatus.value = 0;
            symbolStatus.pattern = PatternType::PatternL;
         }
         else
         {
            symbolStatus.value = 1;
            symbolStatus.pattern = PatternType::PatternH;
         }

         return symbolStatus.pattern;
      }

      return PatternType::Invalid;
   }

   /*
    * Reset modulation status
    */
   void resetModulation()
   {
      // reset modulation detection for all rates
      for (int rate = r212k; rate <= r424k; rate++)
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
      if (frame.frameType() == FrameType::NfcPollFrame)
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
         if (processREQC(frame))
            break;

         processOther(frame);

      }
      while (false);

      // set chained flags
      frame.setFrameFlags(chainedFlags);

      // for request frame set response timings
      if (frame.frameType() == FrameType::NfcPollFrame)
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
    * Process REQB/WUPB frame
    */
   bool processREQC(RawFrame &frame)
   {
      if (frame.frameType() == FrameType::NfcPollFrame)
      {
         if (frame[1] == CommandType::NFCB_REQC)
         {
            frameStatus.lastCommand = frame[1];

            // read number TSN in the Polling Request
            int tsn = frame[5];

            // This commands starts or wakeup card communication, so reset the protocol parameters to the default values
            protocolStatus.maxFrameSize = 256;
            protocolStatus.startUpGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCF_SFGT_DEF);
            protocolStatus.frameGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCF_FGT_DEF);
            protocolStatus.frameWaitingTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCF_FWT_DEF);
            protocolStatus.requestGuardTime = static_cast<int>(decoder->signalParams.sampleTimeUnit * NFCF_RGT_DEF);

            // The REQ-F Response must start between this range
            frameStatus.frameGuardTime = decoder->signalParams.sampleTimeUnit * NFCF_FGT_DEF; // ATQ-C response guard
            frameStatus.frameWaitingTime = decoder->signalParams.sampleTimeUnit * (NFCF_FDT_ATQC + (tsn + 1) * NFCF_TSU_ATQC); // ATQ-C response timeout

            // clear chained flags
            chainedFlags = 0;

            // set frame flags
            frame.setFramePhase(FramePhase::NfcSelectionPhase);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      if (frame.frameType() == FrameType::NfcListenFrame)
      {
         if (frameStatus.lastCommand == CommandType::NFCB_REQC)
         {
            frame.setFramePhase(FramePhase::NfcSelectionPhase);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            //            log->debug("ATQC protocol timing parameters");
            //            log->debug("  maxFrameSize {} bytes", {protocolStatus.maxFrameSize});
            //            log->debug("  frameWaitingTime {} samples ({} us)", {protocolStatus.frameWaitingTime, 1E6 * protocolStatus.frameWaitingTime / decoder->sampleRate});

            return true;
         }
      }

      return false;
   }

   /*
    * Process other frames
    */
   void processOther(RawFrame &frame)
   {
      frame.setFramePhase(FramePhase::NfcApplicationPhase);
      frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);
   }

   /*
    * Check NFC-F crc
    */
   bool checkCrc(RawFrame &frame)
   {
      int size = frame.limit();

      if (size < 2)
         return false;

      unsigned short crc = crc16(frame, 0, size - 2, 0x0000, false);
      unsigned short res = (((unsigned int)frame[size - 2] & 0xff) << 8) | (unsigned int)frame[size - 1] & 0xff;

      return res == crc;
   }
};

NfcF::NfcF(NfcDecoderStatus *decoder) : self(new Impl(decoder))
{
}

NfcF::~NfcF()
{
   delete self;
}

float NfcF::modulationThresholdMin() const
{
   return self->minimumModulationDeep;
}

float NfcF::modulationThresholdMax() const
{
   return self->maximumModulationDeep;
}

void NfcF::setModulationThreshold(float min, float max)
{
   if (!std::isnan(min))
      self->minimumModulationDeep = min;

   if (!std::isnan(max))
      self->maximumModulationDeep = max;
}

float NfcF::correlationThreshold() const
{
   return self->correlationThreshold;
}

void NfcF::setCorrelationThreshold(float value)
{
   if (!std::isnan(value))
      self->correlationThreshold = value;
}

void NfcF::initialize(unsigned int sampleRate)
{
   self->initialize(sampleRate);
}

bool NfcF::detect()
{
   return self->detectModulation();
}

void NfcF::decode(hw::SignalBuffer &samples, std::list<RawFrame> &frames)
{
   self->decodeFrame(samples, frames);
}

}

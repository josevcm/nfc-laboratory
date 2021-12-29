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

#include <tech/NfcA.h>

#ifdef DEBUG_SIGNAL
#define DEBUG_ASK_CORR_CHANNEL 1
#define DEBUG_ASK_SYNC_CHANNEL 1
#define DEBUG_BPSK_PHASE_CHANNEL 1
#define DEBUG_BPSK_SYNC_CHANNEL 1
#define DEBUG_ASK_DEEP_CHANNEL 2
#endif

namespace nfc {

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

struct NfcA::Impl : NfcTech
{
   rt::Logger log {"NfcA"};

   // global signal status
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

   // minimum modulation threshold to detect valid signal for NFC-A (default 95%)
   float minimumModulationThreshold = 0.95f;

   // minimum modulation threshold to detect valid signal for NFC-A (default 100%)
   float maximumModulationThreshold = 1.00f;

   // last detected frame end
   unsigned int lastFrameEnd = 0;

   // chained frame flags
   unsigned int chainedFlags = 0;

   explicit Impl(DecoderStatus *decoder) : decoder(decoder)
   {
   }

   /*
    * Configure NFC-A modulation
    */
   inline void configure(long sampleRate)
   {
      log.info("--------------------------------------------");
      log.info("initializing NFC-A decoder");
      log.info("--------------------------------------------");
      log.info("\tsignalSampleRate     {}", {decoder->sampleRate});
      log.info("\tpowerLevelThreshold  {}", {decoder->powerLevelThreshold});
      log.info("\tmodulationThreshold  {} -> {}", {minimumModulationThreshold, maximumModulationThreshold});

      // clear last detected frame end
      lastFrameEnd = 0;

      // clear chained flags
      chainedFlags = 0;

      // clear detected symbol status
      symbolStatus = {0,};

      // clear bit stream status
      streamStatus = {0,};

      // clear frame processing status
      frameStatus = {0,};

      // compute symbol parameters for 106Kbps, 212Kbps and 424Kbps
      for (int rate = r106k; rate <= r424k; rate++)
      {
         // clear bitrate parameters
         bitrateParams[rate] = {0,};

         // clear modulation parameters
         modulationStatus[rate] = {0,};

         // configure bitrate parametes
         BitrateParams *bitrate = bitrateParams + rate;

         // set tech type and rate
         bitrate->techType = TechType::NfcA;
         bitrate->rateType = rate;

         // symbol timing parameters
         bitrate->symbolsPerSecond = int(std::round(NFC_FC / float(128 >> rate)));

         // number of samples per symbol
         bitrate->period0SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * (256 >> rate))); // double symbol samples
         bitrate->period1SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * (128 >> rate))); // full symbol samples
         bitrate->period2SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * (64 >> rate))); // half symbol samples
         bitrate->period4SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * (32 >> rate))); // quarter of symbol...
         bitrate->period8SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * (16 >> rate))); // and so on...

         // delay guard for each symbol rate
//         bitrate->symbolDelayDetect = rate == r106k ? int(std::round(decoder->signalParams.sampleTimeUnit * 3072)) : bitrateParams[rate - 1].symbolDelayDetect + bitrateParams[rate - 1].period1SymbolSamples;
         bitrate->symbolDelayDetect = rate == r106k ? 0 : bitrateParams[rate - 1].symbolDelayDetect + bitrateParams[rate - 1].period1SymbolSamples;

         // moving average offsets
         bitrate->offsetFutureIndex = BUFFER_SIZE;
         bitrate->offsetSignalIndex = BUFFER_SIZE - bitrate->symbolDelayDetect;
         bitrate->offsetDelay0Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period0SymbolSamples;
         bitrate->offsetDelay1Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period1SymbolSamples;
         bitrate->offsetDelay2Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period2SymbolSamples;
         bitrate->offsetDelay4Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period4SymbolSamples;
         bitrate->offsetDelay8Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period8SymbolSamples;

         // exponential symbol average
         bitrate->symbolAverageW0 = float(1 - 5.0 / bitrate->period1SymbolSamples);
         bitrate->symbolAverageW1 = float(1 - bitrate->symbolAverageW0);

         log.info("{} kpbs parameters:", {round(bitrate->symbolsPerSecond / 1E3)});
         log.info("\tsymbolsPerSecond     {}", {bitrate->symbolsPerSecond});
         log.info("\tperiod1SymbolSamples {} ({} us)", {bitrate->period1SymbolSamples, 1E6 * bitrate->period1SymbolSamples / decoder->sampleRate});
         log.info("\tperiod2SymbolSamples {} ({} us)", {bitrate->period2SymbolSamples, 1E6 * bitrate->period2SymbolSamples / decoder->sampleRate});
         log.info("\tperiod4SymbolSamples {} ({} us)", {bitrate->period4SymbolSamples, 1E6 * bitrate->period4SymbolSamples / decoder->sampleRate});
         log.info("\tperiod8SymbolSamples {} ({} us)", {bitrate->period8SymbolSamples, 1E6 * bitrate->period8SymbolSamples / decoder->sampleRate});
         log.info("\tsymbolDelayDetect    {} ({} us)", {bitrate->symbolDelayDetect, 1E6 * bitrate->symbolDelayDetect / decoder->sampleRate});
         log.info("\toffsetInsertIndex    {}", {bitrate->offsetFutureIndex});
         log.info("\toffsetSignalIndex    {}", {bitrate->offsetSignalIndex});
         log.info("\toffsetDelay8Index    {}", {bitrate->offsetDelay8Index});
         log.info("\toffsetDelay4Index    {}", {bitrate->offsetDelay4Index});
         log.info("\toffsetDelay2Index    {}", {bitrate->offsetDelay2Index});
         log.info("\toffsetDelay1Index    {}", {bitrate->offsetDelay1Index});
         log.info("\toffsetDelay0Index    {}", {bitrate->offsetDelay0Index});
      }

      // initialize default protocol parameters for start decoding
      protocolStatus.maxFrameSize = 256;
      protocolStatus.startUpGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_SFGT_DEF);
      protocolStatus.frameGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_FGT_DEF);
      protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFCA_FWT_DEF);
      protocolStatus.requestGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_RGT_DEF);
      protocolStatus.tr1MinimumTime = int(decoder->signalParams.sampleTimeUnit * 250);
      protocolStatus.tr1MaximumTime = int(decoder->signalParams.sampleTimeUnit * 500);

      // initialize frame parameters to default protocol parameters
      frameStatus.startUpGuardTime = protocolStatus.startUpGuardTime;
      frameStatus.frameWaitingTime = protocolStatus.frameWaitingTime;
      frameStatus.frameGuardTime = protocolStatus.frameGuardTime;
      frameStatus.requestGuardTime = protocolStatus.requestGuardTime;
      frameStatus.tr1MinimumTime = protocolStatus.tr1MinimumTime;
      frameStatus.tr1MaximumTime = protocolStatus.tr1MaximumTime;

      log.info("Startup parameters");
      log.info("\tmaxFrameSize {} bytes", {protocolStatus.maxFrameSize});
      log.info("\tframeGuardTime {} samples ({} us)", {protocolStatus.frameGuardTime, 1000000.0 * protocolStatus.frameGuardTime / decoder->sampleRate});
      log.info("\tframeWaitingTime {} samples ({} us)", {protocolStatus.frameWaitingTime, 1000000.0 * protocolStatus.frameWaitingTime / decoder->sampleRate});
      log.info("\trequestGuardTime {} samples ({} us)", {protocolStatus.requestGuardTime, 1000000.0 * protocolStatus.requestGuardTime / decoder->sampleRate});
      log.info("\ttr1MinimumTime {} samples ({} us)", {protocolStatus.tr1MinimumTime, 1000000.0 * protocolStatus.tr1MinimumTime / decoder->sampleRate});
      log.info("\ttr1MaximumTime {} samples ({} us)", {protocolStatus.tr1MaximumTime, 1000000.0 * protocolStatus.tr1MaximumTime / decoder->sampleRate});
   }

   /*
    * Detect NFC-A modulation
    */
   inline bool detectModulation()
   {
      // ignore low power signals
      if (decoder->signalStatus.signalAverg < decoder->powerLevelThreshold)
         return false;

      // minimum correlation value for valid NFC-A symbols
      float minimumCorrelation = decoder->signalStatus.signalAverg * minimumModulationThreshold;

      // POLL frame ASK detector for  106Kbps, 212Kbps and 424Kbps
      for (int rate = r106k; rate <= r424k; rate++)
      {
         BitrateParams *bitrate = bitrateParams + rate;
         ModulationStatus *modulation = modulationStatus + rate;

         //  signal pointers
         unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
         unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

         // correlation pointers
         unsigned int filterPoint1 = (signalIndex % bitrate->period1SymbolSamples);
         unsigned int filterPoint2 = (signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
         unsigned int filterPoint3 = (signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;

         // get signal samples
         float signalData = decoder->signalStatus.signalData[signalIndex & (BUFFER_SIZE - 1)];
         float delay2Data = decoder->signalStatus.signalData[delay2Index & (BUFFER_SIZE - 1)];
         float signalDeep = decoder->signalStatus.signalDeep[signalIndex & (BUFFER_SIZE - 1)];

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += signalData; // add new value
         modulation->filterIntegrate -= delay2Data; // remove delayed value

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation factors
         float correlatedS1 = (modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3]) / float(bitrate->period4SymbolSamples);

         // compute symbol average
         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + signalData * bitrate->symbolAverageW1;

#ifdef DEBUG_ASK_CORR_CHANNEL
         decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, correlatedS1);
#endif

         // detect modulation deep
         if (correlatedS1 > 0)
         {
            // reset previous detector peak values
            if (modulation->detectorPeakTime && modulation->detectorPeakTime < decoder->signalClock - bitrate->period1SymbolSamples)
            {
               modulation->detectorPeekValue = 0;
               modulation->detectorPeakTime = 0;
            }

            // maximum modulation deep
            if (signalDeep > modulation->detectorPeekValue)
            {
               modulation->detectorPeekValue = signalDeep;
               modulation->detectorPeakTime = decoder->signalClock;
            }
         }

         // detect modulation peak
         if (correlatedS1 >= minimumCorrelation)
         {
            modulation->searchPulseWidth++;

            // detect maximum correlation point
            if (correlatedS1 > modulation->correlatedPeekValue)
            {
               modulation->correlatedPeekValue = correlatedS1;
               modulation->correlatedPeakTime = decoder->signalClock;
               modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
            }
         }

         // wait until search finished and consume all pulse to measure wide
         if (modulation->searchEndTime == 0 || decoder->signalClock < modulation->searchEndTime || correlatedS1 > 0)
            continue;

         // NFC-A / NFC-V pulse wide discriminator
         int maximumPulseWidth = bitrate->period4SymbolSamples + bitrate->period8SymbolSamples;

         // check for valid NFC-A modulated pulse
         if (modulation->correlatedPeakTime == 0 || // no modulation found
             modulation->detectorPeekValue < minimumModulationThreshold || // insufficient modulation deep
             modulation->searchPulseWidth > maximumPulseWidth) // pulse too wide, possible NFC-V modulation
         {
            // reset modulation to continue search
            modulation->searchSyncTime = 0;
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->searchPulseWidth = 0;
            modulation->correlatedPeekValue = 0;
            modulation->detectorPeekValue = 0;
            continue;
         }

#ifdef DEBUG_ASK_SYNC_CHANNEL
         decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.75f);
#endif

         // sets symbol start and end time
         modulation->symbolStartTime = modulation->correlatedPeakTime - bitrate->period2SymbolSamples;
         modulation->symbolEndTime = modulation->correlatedPeakTime + bitrate->period2SymbolSamples;
         modulation->symbolCorr0 = 0;
         modulation->symbolCorr1 = 0;

         // prepare next search window from synchronization point
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeekValue = 0;

         modulation->signalValueThreshold = decoder->signalStatus.signalAverg * minimumModulationThreshold;

         // setup frame info
         frameStatus.frameType = PollFrame;
         frameStatus.symbolRate = bitrate->symbolsPerSecond;
         frameStatus.frameStart = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         frameStatus.frameEnd = 0;

         // setup symbol info
         symbolStatus.value = 0;
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;
         symbolStatus.pattern = PatternType::PatternZ;

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
   inline void decodeFrame(sdr::SignalBuffer &samples, std::list<NfcFrame> &frames)
   {
      if (frameStatus.frameType == FrameType::PollFrame)
      {
         decodePollFrame(samples, frames);
      }

      if (frameStatus.frameType == FrameType::ListenFrame)
      {
         decodeListenFrame(samples, frames);
      }
   }

   /*
    * Decode next poll frame
    */
   inline bool decodePollFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
   {
      int pattern;
      bool frameEnd = false, truncateError = false;

      // read NFC-A request request
      while ((pattern = decodePollFrameSymbolAsk(buffer)) > PatternType::NoPattern)
      {
         streamStatus.pattern = pattern;

         if (streamStatus.pattern == PatternType::PatternY && (streamStatus.previous == PatternType::PatternY || streamStatus.previous == PatternType::PatternZ))
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

               // set last symbol timing
               if (streamStatus.previous == PatternType::PatternZ)
                  frameStatus.frameEnd = symbolStatus.end - decoder->bitrate->period2SymbolSamples;
               else
                  frameStatus.frameEnd = symbolStatus.end - decoder->bitrate->period1SymbolSamples;

               // build request frame
               NfcFrame request = NfcFrame(TechType::NfcA, FrameType::PollFrame);

               request.setFrameRate(frameStatus.symbolRate);
               request.setSampleStart(frameStatus.frameStart);
               request.setSampleEnd(frameStatus.frameEnd);
               request.setTimeStart(double(frameStatus.frameStart) / double(decoder->sampleRate));
               request.setTimeEnd(double(frameStatus.frameEnd) / double(decoder->sampleRate));

               if (streamStatus.flags & FrameFlags::ParityError)
                  request.setFrameFlags(FrameFlags::ParityError);

               if (truncateError)
                  request.setFrameFlags(FrameFlags::Truncated);

               if (streamStatus.bytes == 1 && streamStatus.bits == 7)
                  request.setFrameFlags(FrameFlags::ShortFrame);

               // add bytes to frame and flip to prepare read
               request.put(streamStatus.buffer, streamStatus.bytes).flip();

               // clear modulation status for next frame search
               decoder->modulation->symbolStartTime = 0;
               decoder->modulation->symbolEndTime = 0;
               decoder->modulation->filterIntegrate = 0;
               decoder->modulation->detectIntegrate = 0;
               decoder->modulation->phaseIntegrate = 0;
               decoder->modulation->searchSyncTime = 0;
               decoder->modulation->searchStartTime = 0;
               decoder->modulation->searchEndTime = 0;
               decoder->modulation->searchPulseWidth = 0;

               // clear stream status
               streamStatus = {0,};

               // process frame
               process(request);

               // add to frame list
               frames.push_back(request);

               // return request frame data
               return true;
            }

            // reset modulation and restart frame detection
            resetModulation();

            // no valid frame found
            return false;
         }

         if (streamStatus.previous)
         {
            int value = (streamStatus.previous == PatternType::PatternX);

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
   inline bool decodeListenFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
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
            if (pattern == PatternType::PatternD)
            {
               frameStatus.frameStart = symbolStatus.start;
            }
            else
            {
               //  end of frame waiting time, restart modulation search
               if (pattern == PatternType::NoPattern)
                  resetModulation();

               // no frame found
               return NfcFrame::Nil;
            }
         }

         if (frameStatus.frameStart)
         {
            // decode remaining response
            while ((pattern = decodeListenFrameSymbolAsk(buffer)) > PatternType::NoPattern)
            {
               if (pattern == PatternType::PatternF)
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

                     frameStatus.frameEnd = symbolStatus.end;

                     NfcFrame response = NfcFrame(TechType::NfcA, FrameType::ListenFrame);

                     response.setFrameRate(decoder->bitrate->symbolsPerSecond);
                     response.setSampleStart(frameStatus.frameStart);
                     response.setSampleEnd(frameStatus.frameEnd);
                     response.setTimeStart(double(frameStatus.frameStart) / double(decoder->sampleRate));
                     response.setTimeEnd(double(frameStatus.frameEnd) / double(decoder->sampleRate));

                     if (streamStatus.flags & ParityError)
                        response.setFrameFlags(FrameFlags::ParityError);

                     if (truncateError)
                        response.setFrameFlags(FrameFlags::Truncated);

                     if (streamStatus.bytes == 1 && streamStatus.bits == 4)
                        response.setFrameFlags(FrameFlags::ShortFrame);

                     // add bytes to frame and flip to prepare read
                     response.put(streamStatus.buffer, streamStatus.bytes).flip();

                     // process frame
                     process(response);

                     // reset modulation status
                     resetModulation();

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

            // Pattern-M found, mark frame start time
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
            while ((pattern = decodeListenFrameSymbolBpsk(buffer)) > PatternType::NoPattern)
            {
               if (pattern == PatternType::PatternO)
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
                     NfcFrame response = NfcFrame(TechType::NfcA, FrameType::ListenFrame);

                     response.setFrameRate(decoder->bitrate->symbolsPerSecond);
                     response.setSampleStart(frameStatus.frameStart);
                     response.setSampleEnd(frameStatus.frameEnd);
                     response.setTimeStart(double(frameStatus.frameStart) / double(decoder->sampleRate));
                     response.setTimeEnd(double(frameStatus.frameEnd) / double(decoder->sampleRate));

                     if (streamStatus.flags & ParityError)
                        response.setFrameFlags(FrameFlags::ParityError);

                     if (truncateError)
                        response.setFrameFlags(FrameFlags::Truncated);

                     // add bytes to frame and flip to prepare read
                     response.put(streamStatus.buffer, streamStatus.bytes).flip();

                     // reset modulation status
                     resetModulation();

                     // process frame
                     process(response);

                     // add to frame list
                     frames.push_back(response);

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
                  // store byte in stream buffer
                  streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;

                  // frame bytes has odd parity
                  streamStatus.flags |= !checkParity(streamStatus.data, streamStatus.parity) ? ParityError : 0;

                  // initialize next value from current symbol
                  streamStatus.data = symbolStatus.value;

                  // reset bit counter
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
   inline int decodePollFrameSymbolAsk(sdr::SignalBuffer &buffer)
   {
      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

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
         float currentData = decoder->signalStatus.signalData[signalIndex & (BUFFER_SIZE - 1)];
         float delayedData = decoder->signalStatus.signalData[delay2Index & (BUFFER_SIZE - 1)];

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += currentData; // add new value
         modulation->filterIntegrate -= delayedData; // remove delayed value

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation factors
         float correlatedS0 = modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2];
         float correlatedS1 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3];
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1) / float(bitrate->period2SymbolSamples);

         // compute symbol average
         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + currentData * bitrate->symbolAverageW1;

#ifdef DEBUG_ASK_CORR_CHANNEL
         decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, correlatedSD);
#endif

#ifdef DEBUG_ASK_SYNC_CHANNEL
         if (decoder->signalClock == modulation->searchSyncTime)
            decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.50f);
#endif

         // wait until correlation search start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         // detect maximum symbol correlation
         if (correlatedSD > modulation->correlatedPeekValue)
         {
            modulation->correlatedPeekValue = correlatedSD;
            modulation->correlatedPeakTime = decoder->signalClock;
         }

         // capture symbol correlation values at synchronization point
         if (decoder->signalClock == modulation->searchSyncTime)
         {
            modulation->symbolCorr0 = correlatedS0;
            modulation->symbolCorr1 = correlatedS1;
         }

         // wait until correlation search finish
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // detect Pattern-Y when no modulation occurs (below search detection threshold)
         if (modulation->correlatedPeekValue < modulation->signalValueThreshold)
         {
            // estimate symbol timings from synchronization point (peak detection not valid due lack of modulation)
            modulation->symbolEndTime = modulation->searchSyncTime;

            // setup symbol info
            symbolStatus.value = 1;
            symbolStatus.pattern = PatternType::PatternY;
         }

            // detect Pattern-Z
         else if (modulation->symbolCorr0 > modulation->symbolCorr1)
         {
            // re-sync symbol end from correlate peak detector
            modulation->symbolEndTime = modulation->correlatedPeakTime;

            // setup symbol info
            symbolStatus.value = 0;
            symbolStatus.pattern = PatternType::PatternZ;
         }

            // detect Pattern-X
         else
         {
            // re-sync symbol end from correlate peak detector
            modulation->symbolEndTime = modulation->correlatedPeakTime;

            // detect Pattern-X, setup symbol info
            symbolStatus.value = 1;
            symbolStatus.pattern = PatternType::PatternX;
         }

         // sets symbol start parameters and next synchronization point
         modulation->symbolStartTime = modulation->symbolEndTime - bitrate->period1SymbolSamples;
         modulation->symbolCorr0 = 0;
         modulation->symbolCorr1 = 0;

         // sets next search window
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeekValue = 0;

         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;

         return symbolStatus.pattern;
      }

      return PatternType::Invalid;
   }

   /*
    * Decode SOF for ASK modulated listen frame
    */
   inline int decodeListenFrameStartAsk(sdr::SignalBuffer &buffer)
   {
      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

      unsigned int futureIndex = (bitrate->offsetFutureIndex + decoder->signalClock);
      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;
         ++futureIndex;
         ++delay2Index;

         // get signal samples
         float signalData = decoder->signalStatus.signalData[signalIndex & (BUFFER_SIZE - 1)];
         float signalDeep = decoder->signalStatus.signalDeep[futureIndex & (BUFFER_SIZE - 1)];

         // compute symbol average (signal offset)
         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + signalData * bitrate->symbolAverageW1;

         // remove DC offset from signal value
         signalData -= modulation->symbolAverage;

         // store signal square in filter buffer
         modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)] = signalData * signalData;

         // wait until frame guard time is reached
         if (decoder->signalClock < (frameStatus.guardEnd - bitrate->period1SymbolSamples))
            continue;

         // compute correlation points
         unsigned int filterPoint1 = (signalIndex % bitrate->period1SymbolSamples);
         unsigned int filterPoint2 = (signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
         unsigned int filterPoint3 = (signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;

         // integrate symbol (moving average)
         modulation->filterIntegrate += modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->filterIntegrate -= modulation->integrationData[delay2Index & (BUFFER_SIZE - 1)]; // remove delayed value

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation results for each symbol and distance
         float correlatedS0 = modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2];
         float correlatedS1 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3];
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1);

         // wait until frame guard time is reached
         if (decoder->signalClock < frameStatus.guardEnd)
            continue;

         // using signal st.dev as lower level threshold
         if (decoder->signalClock == frameStatus.guardEnd)
            modulation->signalValueThreshold = decoder->signalStatus.signalMdev[signalIndex & (BUFFER_SIZE - 1)];

         // check for maximum response time
         if (decoder->signalClock > frameStatus.waitingEnd)
            return PatternType::NoPattern;

         // poll frame modulation detected while waiting for response
         if (signalDeep > minimumModulationThreshold)
            return PatternType::NoPattern;

#ifdef DEBUG_ASK_CORR_CHANNEL
         decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, modulation->signalValueThreshold);
#endif

         // detect maximum correlation peak
         if (correlatedSD > modulation->signalValueThreshold)
         {
#ifdef DEBUG_ASK_CORR_CHANNEL
            decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, correlatedSD);
#endif
            // max correlation peak detector
            if (correlatedSD > modulation->correlatedPeekValue)
            {
               modulation->searchPulseWidth++;
               modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
               modulation->correlatedPeekValue = correlatedSD;
               modulation->correlatedPeakTime = decoder->signalClock;
            }
         }

         // wait until search finished
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

#ifdef DEBUG_ASK_SYNC_CHANNEL
         decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.75f);
#endif
         if (modulation->searchPulseWidth <= bitrate->period8SymbolSamples)
         {
            // if no valid modulation is found, we restart SOF search
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->correlatedPeekValue = 0;
            modulation->searchPulseWidth = 0;
            break;
         }

         // set pattern search window
         modulation->symbolStartTime = modulation->correlatedPeakTime - bitrate->period2SymbolSamples;
         modulation->symbolEndTime = modulation->correlatedPeakTime + bitrate->period2SymbolSamples;

         // timing search window
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeekValue = 0;

         // setup symbol info
         symbolStatus.value = 1;
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;
         symbolStatus.pattern = PatternType::PatternD;

         return symbolStatus.pattern;
      }

      return PatternType::Invalid;
   }

   /*
    * Decode one ASK modulated listen frame symbol
    */
   inline int decodeListenFrameSymbolAsk(sdr::SignalBuffer &buffer)
   {
      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

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
         float signalData = decoder->signalStatus.signalData[signalIndex & (BUFFER_SIZE - 1)];

         // compute symbol average (signal offset)
         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + signalData * bitrate->symbolAverageW1;

         // remove DC offset from signal value
         signalData -= modulation->symbolAverage;

         // store signal square in filter buffer
         modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)] = signalData * signalData;

         // integrate symbol (moving average)
         modulation->filterIntegrate += modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->filterIntegrate -= modulation->integrationData[delay2Index & (BUFFER_SIZE - 1)]; // remove delayed value

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation results for each symbol and distance
         float correlatedS0 = modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2];
         float correlatedS1 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3];
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1);

#ifdef DEBUG_ASK_CORR_CHANNEL
         decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, correlatedSD);
#endif

#ifdef DEBUG_ASK_SYNC_CHANNEL
         if (decoder->signalClock == modulation->searchSyncTime)
            decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.50f);
#endif

         // wait until correlation search start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         // detect maximum symbol correlation
         if (correlatedSD > modulation->correlatedPeekValue)
         {
            modulation->correlatedPeekValue = correlatedSD;
            modulation->correlatedPeakTime = decoder->signalClock;
         }

         // capture symbol correlation values at synchronization point
         if (decoder->signalClock == modulation->searchSyncTime)
         {
            modulation->symbolCorr0 = correlatedS0;
            modulation->symbolCorr1 = correlatedS1;
         }

         // wait until correlation search finish
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         if (modulation->correlatedPeekValue > modulation->signalValueThreshold)
         {
            modulation->symbolStartTime = modulation->symbolEndTime;
            modulation->symbolEndTime = modulation->correlatedPeakTime;

            if (modulation->symbolCorr0 > modulation->symbolCorr1)
            {
               symbolStatus.value = 0;
               symbolStatus.pattern = PatternType::PatternE;
            }
            else
            {
               symbolStatus.value = 1;
               symbolStatus.pattern = PatternType::PatternD;
            }
         }
         else
         {
            modulation->symbolStartTime = modulation->symbolEndTime;
            modulation->symbolEndTime = modulation->searchSyncTime;

            // no modulation (End Of Frame) EoF
            symbolStatus.pattern = PatternType::PatternF;
         }

         // next timing search window
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeekValue = 0;

         // setup symbol info
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;

         return symbolStatus.pattern;
      }

      return PatternType::Invalid;
   }

   /*
    * Decode SOF for BPSK modulated listen frame
    */
   inline int decodeListenFrameStartBpsk(sdr::SignalBuffer &buffer)
   {
      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

      unsigned int futureIndex = (bitrate->offsetFutureIndex + decoder->signalClock);
      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay1Index = (bitrate->offsetDelay1Index + decoder->signalClock);
      unsigned int delay4Index = (bitrate->offsetDelay4Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++futureIndex;
         ++signalIndex;
         ++delay1Index;
         ++delay4Index;

         // get signal samples
         float signalData = decoder->signalStatus.signalData[signalIndex & (BUFFER_SIZE - 1)];
         float delay1Data = decoder->signalStatus.signalData[delay1Index & (BUFFER_SIZE - 1)];
         float signalDeep = decoder->signalStatus.signalDeep[futureIndex & (BUFFER_SIZE - 1)];

         // compute symbol average
         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + signalData * bitrate->symbolAverageW1;

         // multiply 1 symbol delayed signal with incoming signal, (magic number 10 must be signal dependent...)
         float phase = (signalData - modulation->symbolAverage) * (delay1Data - modulation->symbolAverage) * 10;

         // store signal phase in filter buffer
         modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)] = phase;

         // integrate response from PICC after guard time (TR0)
         if (decoder->signalClock < (frameStatus.guardEnd - bitrate->period1SymbolSamples))
            continue;

         // compute phase integration
         modulation->phaseIntegrate += modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->phaseIntegrate -= modulation->integrationData[delay4Index & (BUFFER_SIZE - 1)]; // remove delayed value

         // integrate response from PICC after guard time (TR0)
         if (decoder->signalClock < frameStatus.guardEnd)
            continue;

#ifdef DEBUG_BPSK_PHASE_CHANNEL
         decoder->debug->set(DEBUG_BPSK_PHASE_CHANNEL, modulation->phaseIntegrate);
#endif

         // using signal st.dev as lower level threshold
         if (decoder->signalClock == frameStatus.guardEnd)
            modulation->signalValueThreshold = decoder->signalStatus.signalMdev[signalIndex & (BUFFER_SIZE - 1)];

         // check if frame waiting time exceeded without detect modulation
         if (decoder->signalClock >= frameStatus.waitingEnd)
            return PatternType::NoPattern;

         // check if poll frame modulation is detected while waiting for response
         if (signalDeep > minimumModulationThreshold)
            return PatternType::NoPattern;

         // detect first zero-cross
         if (modulation->phaseIntegrate > modulation->signalValueThreshold)
         {
            modulation->correlatedPeakTime = decoder->signalClock;
            modulation->searchEndTime = decoder->signalClock + bitrate->period2SymbolSamples;
            modulation->searchPulseWidth++;
         }

         // wait until correlation search finish
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         if (modulation->searchPulseWidth < frameStatus.tr1MinimumTime)
         {
            modulation->correlatedPeakTime = 0;
            modulation->searchEndTime = 0;
            modulation->searchPulseWidth = 0;
            continue;
         }

#ifdef DEBUG_BPSK_SYNC_CHANNEL
         decoder->debug->set(DEBUG_BPSK_SYNC_CHANNEL, 0.75);
#endif
         // set symbol window
         modulation->symbolStartTime = modulation->correlatedPeakTime - bitrate->period2SymbolSamples - 500 * decoder->signalParams.sampleTimeUnit;
         modulation->symbolEndTime = modulation->correlatedPeakTime + bitrate->period1SymbolSamples;

         // set next synchronization point
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period2SymbolSamples;
         modulation->searchLastPhase = modulation->phaseIntegrate;
         modulation->searchPulseWidth = 0;

         modulation->signalPhaseThreshold = std::fabs(modulation->phaseIntegrate / 3);
         modulation->correlatedPeekValue = 0;

         // set symbol info
         symbolStatus.value = 0;
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
   inline int decodeListenFrameSymbolBpsk(sdr::SignalBuffer &buffer)
   {
      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay1Index = (bitrate->offsetDelay1Index + decoder->signalClock);
      unsigned int delay4Index = (bitrate->offsetDelay4Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;
         ++delay1Index;
         ++delay4Index;

         // get signal samples
         float signalData = decoder->signalStatus.signalData[signalIndex & (BUFFER_SIZE - 1)];
         float delay1Data = decoder->signalStatus.signalData[delay1Index & (BUFFER_SIZE - 1)];

         // compute symbol average
         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + signalData * bitrate->symbolAverageW1;

         // multiply 1 symbol delayed signal with incoming signal
         float phase = (signalData - modulation->symbolAverage) * (delay1Data - modulation->symbolAverage);

         // store signal phase in filter buffer
         modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)] = phase * 10;

         modulation->phaseIntegrate += modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->phaseIntegrate -= modulation->integrationData[delay4Index & (BUFFER_SIZE - 1)]; // remove delayed value

#ifdef DEBUG_BPSK_PHASE_CHANNEL
         decoder->debug->set(DEBUG_BPSK_PHASE_CHANNEL, modulation->phaseIntegrate);
#endif
         // edge detector for re-synchronization
         if ((modulation->phaseIntegrate > 0 && modulation->searchLastPhase < 0) || (modulation->phaseIntegrate < 0 && modulation->searchLastPhase > 0))
         {
            modulation->searchSyncTime = decoder->signalClock + bitrate->period2SymbolSamples;
            modulation->searchLastPhase = modulation->phaseIntegrate;
         }

         // wait until synchronization point is reached
         if (decoder->signalClock != modulation->searchSyncTime)
            continue;

#ifdef DEBUG_BPSK_SYNC_CHANNEL
         decoder->debug->set(DEBUG_BPSK_SYNC_CHANNEL, 0.50f);
#endif

         // set symbol timings
         modulation->symbolStartTime = modulation->symbolEndTime;
         modulation->symbolEndTime = modulation->searchSyncTime + bitrate->period2SymbolSamples;

         // next synchronization point
         modulation->searchSyncTime = modulation->searchSyncTime + bitrate->period1SymbolSamples;
         modulation->searchLastPhase = modulation->phaseIntegrate;

         // no modulation detected, generate End Of Frame
         if (std::abs(modulation->phaseIntegrate) < std::abs(modulation->signalPhaseThreshold))
            return PatternType::PatternO;

         // symbol change, invert pattern and value
         if (modulation->phaseIntegrate < -modulation->signalPhaseThreshold)
         {
            symbolStatus.value = !symbolStatus.value;
            symbolStatus.pattern = (symbolStatus.pattern == PatternType::PatternM) ? PatternType::PatternN : PatternType::PatternM;
         }

         // setup symbol info
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;

         return symbolStatus.pattern;
      }

      return PatternType::Invalid;
   }

   /*
    * Reset frame search status
    */
   inline void resetFrameSearch()
   {
      // reset frame search status
      if (decoder->modulation)
      {
         decoder->modulation->symbolEndTime = 0;
         decoder->modulation->searchEndTime = 0;
         decoder->modulation->correlatedPeekValue = 0;
      }

      // reset frame start time
      frameStatus.frameStart = 0;
   }

   /*
    * Reset modulation status
    */
   inline void resetModulation()
   {
      // reset modulation detection for all rates
      for (int rate = r106k; rate <= r424k; rate++)
      {
         modulationStatus[rate].searchSyncTime = 0;
         modulationStatus[rate].searchStartTime = 0;
         modulationStatus[rate].searchEndTime = 0;
         modulationStatus[rate].searchPulseWidth = 0;
         modulationStatus[rate].searchLastPhase = NAN;
         modulationStatus[rate].correlatedPeekValue = 0;
         modulationStatus[rate].detectorPeekValue = 0;
         modulationStatus[rate].symbolAverage = 0;
      }

      // clear stream status
      streamStatus = {0,};

      // clear stream status
      symbolStatus = {0,};

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
   inline void process(NfcFrame &frame)
   {
      // for request frame set default response timings, must be overridden by subsequent process functions
      if (frame.isPollFrame())
      {
         // initialize frame parameters to default protocol parameters
         frameStatus.startUpGuardTime = protocolStatus.startUpGuardTime;
         frameStatus.frameWaitingTime = protocolStatus.frameWaitingTime;
         frameStatus.frameGuardTime = protocolStatus.frameGuardTime;
         frameStatus.requestGuardTime = protocolStatus.requestGuardTime;
         frameStatus.tr1MinimumTime = protocolStatus.tr1MinimumTime;
         frameStatus.tr1MaximumTime = protocolStatus.tr1MaximumTime;
      }

      do
      {
         if (processREQA(frame))
            break;

         if (processHLTA(frame))
            break;

         if (!(chainedFlags & FrameFlags::Encrypted))
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
      } while (false);

      // set chained flags
      frame.setFrameFlags(chainedFlags);

      // for request frame set response timings
      if (frame.isPollFrame())
      {
         // update frame timing parameters for receive PICC frame
         if (decoder->bitrate)
         {
            // response guard time TR0min (PICC must not modulate reponse within this period)
            frameStatus.guardEnd = frameStatus.frameEnd + frameStatus.frameGuardTime + decoder->bitrate->symbolDelayDetect;

            // response delay time WFT (PICC must reply to command before this period)
            frameStatus.waitingEnd = frameStatus.frameEnd + frameStatus.frameWaitingTime + decoder->bitrate->symbolDelayDetect;

            // next frame must be ListenFrame
            frameStatus.frameType = ListenFrame;
         }
      }
      else
      {
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
   inline bool processREQA(NfcFrame &frame)
   {
      if (frame.isPollFrame())
      {
         if ((frame[0] == CommandType::NFCA_REQA || frame[0] == CommandType::NFCA_WUPA) && frame.limit() == 1)
         {
            frame.setFramePhase(FramePhase::SelectionFrame);

            frameStatus.lastCommand = frame[0];

            // This commands starts or wakeup card communication, so reset the protocol parameters to the default values
            protocolStatus.maxFrameSize = 256;
            protocolStatus.startUpGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_SFGT_DEF);
            protocolStatus.frameGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_FGT_DEF);
            protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFCA_FWT_DEF);
            protocolStatus.requestGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_RGT_DEF);

            // The REQ-A Response must start exactly at 128 * n, n=9, decoder search between n=7 and n=18
            frameStatus.frameGuardTime = decoder->signalParams.sampleTimeUnit * NFC_TR0_MIN; // ATQ-A response guard
            frameStatus.frameWaitingTime = decoder->signalParams.sampleTimeUnit * NFCA_FWT_ATQA; // ATQ-A response timeout

            // clear chained flags
            chainedFlags = 0;

            return true;
         }
      }

      if (frame.isListenFrame())
      {
         if (frameStatus.lastCommand == CommandType::NFCA_REQA || frameStatus.lastCommand == CommandType::NFCA_WUPA)
         {
            frame.setFramePhase(FramePhase::SelectionFrame);

            return true;
         }
      }

      return false;
   }

   /*
    * Process HLTA frame
    */
   inline bool processHLTA(NfcFrame &frame)
   {
      if (frame.isPollFrame())
      {
         if (frame[0] == CommandType::NFCA_HLTA && frame.limit() == 4)
         {
            frame.setFramePhase(FramePhase::SelectionFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            frameStatus.lastCommand = frame[0];

            // After this command the PICC will stop and will not respond, set the protocol parameters to the default values
            protocolStatus.maxFrameSize = 256;
            protocolStatus.startUpGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_SFGT_DEF);
            protocolStatus.frameGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_FGT_DEF);
            protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFCA_FWT_DEF);
            protocolStatus.requestGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_RGT_DEF);

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
   inline bool processSELn(NfcFrame &frame)
   {
      if (frame.isPollFrame())
      {
         if (frame[0] == CommandType::NFCA_SEL1 || frame[0] == CommandType::NFCA_SEL2 || frame[0] == CommandType::NFCA_SEL3)
         {
            frame.setFramePhase(FramePhase::SelectionFrame);

            frameStatus.lastCommand = frame[0];

            // The selection commands has same timings as REQ-A
            frameStatus.frameGuardTime = decoder->signalParams.sampleTimeUnit * NFC_TR0_MIN;
            frameStatus.frameWaitingTime = decoder->signalParams.sampleTimeUnit * NFCA_FWT_ATQA;

            return true;
         }
      }

      if (frame.isListenFrame())
      {
         if (frameStatus.lastCommand == CommandType::NFCA_SEL1 || frameStatus.lastCommand == CommandType::NFCA_SEL2 || frameStatus.lastCommand == CommandType::NFCA_SEL3)
         {
            frame.setFramePhase(FramePhase::SelectionFrame);

            return true;
         }
      }

      return false;
   }

   /*
    * Process RAST frame
    */
   inline bool processRATS(NfcFrame &frame)
   {
      // capture parameters from ATS and reconfigure decoder timings
      if (frame.isPollFrame())
      {
         if (frame[0] == CommandType::NFCA_RATS)
         {
            int fsdi = (frame[1] >> 4) & 0x0F;

            frameStatus.lastCommand = frame[0];

            // sets maximum frame length requested by reader
            protocolStatus.maxFrameSize = NFC_FDS_TABLE[fsdi];

            // sets the activation frame waiting time for ATS response
            frameStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFC_FWT_ACTIVATION);

            log.info("RATS frame parameters");
            log.info("  maxFrameSize {} bytes", {protocolStatus.maxFrameSize});

            // set frame flags
            frame.setFramePhase(FramePhase::SelectionFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      if (frame.isListenFrame())
      {
         if (frameStatus.lastCommand == CommandType::NFCA_RATS)
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
                  protocolStatus.startUpGuardTime = int(decoder->signalParams.sampleTimeUnit * NFC_SFGT_TABLE[sfgi]);
                  protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFC_FWT_TABLE[fwi]);
               }
               else
               {
                  // if TB is not transmitted establish default timing parameters
                  protocolStatus.startUpGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_SFGT_DEF);
                  protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFCA_FWT_DEF);
               }

               log.info("ATS protocol timing parameters");
               log.info("  startUpGuardTime {} samples ({} us)", {protocolStatus.startUpGuardTime, 1000000.0 * protocolStatus.startUpGuardTime / decoder->sampleRate});
               log.info("  frameWaitingTime {} samples ({} us)", {protocolStatus.frameWaitingTime, 1000000.0 * protocolStatus.frameWaitingTime / decoder->sampleRate});
            }

            frame.setFramePhase(FramePhase::SelectionFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      return false;
   }

   /*
    * Process PPS frame
    */
   inline bool processPPSr(NfcFrame &frame)
   {
      if (frame.isPollFrame())
      {
         if ((frame[0] & 0xF0) == CommandType::NFCA_PPS)
         {
            frameStatus.lastCommand = frame[0] & 0xF0;

            frame.setFramePhase(FramePhase::SelectionFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      if (frame.isListenFrame())
      {
         if (frameStatus.lastCommand == CommandType::NFCA_PPS)
         {
            frame.setFramePhase(FramePhase::SelectionFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      return false;
   }

   /*
    * Process Mifare Classic AUTH frame
    */
   inline bool processAUTH(NfcFrame &frame)
   {
      if (frame.isPollFrame())
      {
         if (frame[0] == CommandType::NFCA_AUTH1 || frame[0] == CommandType::NFCA_AUTH2)
         {
            frameStatus.lastCommand = frame[0];
            //         frameStatus.frameWaitingTime = int(signalParams.sampleTimeUnit * 256 * 16 * (1 << 14);

            frame.setFramePhase(FramePhase::ApplicationFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            //      if (!frameStatus.lastCommand)
            //      {
            //         crypto1_init(&frameStatus.crypto1, 0x521284223154);
            //      }

            return true;
         }
      }

      if (frame.isListenFrame())
      {
         if (frameStatus.lastCommand == CommandType::NFCA_AUTH1 || frameStatus.lastCommand == CommandType::NFCA_AUTH2)
         {
            //      unsigned int uid = 0x1e47fc35;
            //      unsigned int nc = frame[0] << 24 || frame[1] << 16 || frame[2] << 8 || frame[0];
            //
            //      crypto1_word(&frameStatus.crypto1, uid ^ nc, 0);

            // set chained flags

            chainedFlags = FrameFlags::Encrypted;

            frame.setFramePhase(FramePhase::ApplicationFrame);

            return true;
         }
      }

      return false;
   }

   /*
    * Process ISO-14443 I-Block frame
    */
   inline bool processIBlock(NfcFrame &frame)
   {
      if (frame.isPollFrame())
      {
         if ((frame[0] & 0xE2) == CommandType::NFCA_IBLOCK)
         {
            frameStatus.lastCommand = frame[0] & 0xE2;

            frame.setFramePhase(FramePhase::ApplicationFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      if (frame.isListenFrame())
      {
         if (frameStatus.lastCommand == CommandType::NFCA_IBLOCK)
         {
            frame.setFramePhase(FramePhase::ApplicationFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      return false;
   }

   /*
    * Process ISO-14443 R-Block frame
    */
   inline bool processRBlock(NfcFrame &frame)
   {
      if (frame.isPollFrame())
      {
         if ((frame[0] & 0xE6) == CommandType::NFCA_RBLOCK)
         {
            frameStatus.lastCommand = frame[0] & 0xE6;

            frame.setFramePhase(FramePhase::ApplicationFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      if (frame.isListenFrame())
      {
         if (frameStatus.lastCommand == CommandType::NFCA_RBLOCK)
         {
            frame.setFramePhase(FramePhase::ApplicationFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      return false;
   }

   /*
    * Process ISO-14443 S-Block frame
    */
   inline bool processSBlock(NfcFrame &frame)
   {
      if (frame.isPollFrame())
      {
         if ((frame[0] & 0xC7) == CommandType::NFCA_SBLOCK)
         {
            frameStatus.lastCommand = frame[0] & 0xC7;

            frame.setFramePhase(FramePhase::ApplicationFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      if (frame.isListenFrame())
      {
         if (frameStatus.lastCommand == CommandType::NFCA_SBLOCK)
         {
            frame.setFramePhase(FramePhase::ApplicationFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      return false;
   }

   /*
    * Process other frame types
    */
   inline void processOther(NfcFrame &frame)
   {
      frame.setFramePhase(FramePhase::ApplicationFrame);
      frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);
   }

   /*
    * Check NFC-A crc NFC-A ITU-V.41
    */
   inline bool checkCrc(NfcFrame &frame)
   {
      int size = frame.limit();

      if (size < 3)
         return false;

      unsigned short crc = crc16(frame, 0, size - 2, 0x6363, true);
      unsigned short res = ((unsigned int) frame[size - 2] & 0xff) | ((unsigned int) frame[size - 1] & 0xff) << 8;

      return res == crc;
   }

   /*
    * Check NFC-A byte parity
    */
   inline static bool checkParity(unsigned int value, unsigned int parity)
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

NfcA::NfcA(DecoderStatus *decoder) : self(new Impl(decoder))
{
}

NfcA::~NfcA()
{
   delete self;
}

void NfcA::setModulationThreshold(float min, float max)
{
   if (!std::isnan(min))
      self->minimumModulationThreshold = min;
}

/*
 * Configure NFC-A modulation
 */
void NfcA::configure(long sampleRate)
{
   self->configure(sampleRate);
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
void NfcA::decode(sdr::SignalBuffer &samples, std::list<NfcFrame> &frames)
{
   self->decodeFrame(samples, frames);
}

}
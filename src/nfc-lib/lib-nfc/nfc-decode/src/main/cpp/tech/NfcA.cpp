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
#define DEBUG_ASK_SYNC_CHANNEL 2
#define DEBUG_BPSK_PHASE_CHANNEL 1
#define DEBUG_BPSK_SYNC_CHANNEL 2
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
   PatternO = 10
};

struct NfcA::Impl
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

   // minimum modulation threshold to detect valid signal for NFC-A (default 85%)
   float minimumModulationThreshold = 0.850f;

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
      log.info("\tmodulationThreshold  {}", {minimumModulationThreshold});

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

      // compute symbol parameters for 106Kbps, 212Kbps, 424Kbps and 848Kbps
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
         bitrate->symbolsPerSecond = NFC_FC / (128 >> rate);

         // number of samples per symbol
         bitrate->period1SymbolSamples = int(round(decoder->signalParams.sampleTimeUnit * (128 >> rate))); // full symbol samples
         bitrate->period2SymbolSamples = int(round(decoder->signalParams.sampleTimeUnit * (64 >> rate))); // half symbol samples
         bitrate->period4SymbolSamples = int(round(decoder->signalParams.sampleTimeUnit * (32 >> rate))); // quarter of symbol...
         bitrate->period8SymbolSamples = int(round(decoder->signalParams.sampleTimeUnit * (16 >> rate))); // and so on...

         // delay guard for each symbol rate
         bitrate->symbolDelayDetect = rate > r106k ? bitrateParams[rate - 1].symbolDelayDetect + bitrateParams[rate - 1].period1SymbolSamples : 0;

         // moving average offsets
         bitrate->offsetSignalIndex = BUFFER_SIZE - bitrate->symbolDelayDetect;
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
         log.info("\toffsetSignalIndex    {}", {bitrate->offsetSignalIndex});
         log.info("\toffsetDelay1Index    {}", {bitrate->offsetDelay1Index});
         log.info("\toffsetDelay2Index    {}", {bitrate->offsetDelay2Index});
         log.info("\toffsetDelay4Index    {}", {bitrate->offsetDelay4Index});
         log.info("\toffsetDelay8Index    {}", {bitrate->offsetDelay8Index});
      }

      // initialize default protocol parameters for start decoding
      protocolStatus.maxFrameSize = 256;
      protocolStatus.startUpGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_SFGT_DEF);
      protocolStatus.frameGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_FGT_DEF);
      protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFCA_FWT_DEF);
      protocolStatus.requestGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCA_RGT_DEF);

      // initialize frame parameters to default protocol parameters
      frameStatus.startUpGuardTime = protocolStatus.startUpGuardTime;
      frameStatus.frameWaitingTime = protocolStatus.frameWaitingTime;
      frameStatus.frameGuardTime = protocolStatus.frameGuardTime;
      frameStatus.requestGuardTime = protocolStatus.requestGuardTime;

      log.info("Startup parameters");
      log.info("\tmaxFrameSize {} bytes", {protocolStatus.maxFrameSize});
      log.info("\tframeGuardTime {} samples ({} us)", {protocolStatus.frameGuardTime, 1000000.0 * protocolStatus.frameGuardTime / decoder->sampleRate});
      log.info("\tframeWaitingTime {} samples ({} us)", {protocolStatus.frameWaitingTime, 1000000.0 * protocolStatus.frameWaitingTime / decoder->sampleRate});
      log.info("\trequestGuardTime {} samples ({} us)", {protocolStatus.requestGuardTime, 1000000.0 * protocolStatus.requestGuardTime / decoder->sampleRate});
   }

   /*
    * Detect NFC-A modulation
    */
   inline bool detectModulation()
   {
      // ignore low power signals
      if (decoder->signalStatus.powerAverage < decoder->powerLevelThreshold)
         return false;

      // POLL frame ASK detector for  106Kbps, 212Kbps and 424Kbps
      for (int rate = r106k; rate <= r424k; rate++)
      {
         BitrateParams *bitrate = bitrateParams + rate;
         ModulationStatus *modulation = modulationStatus + rate;

         // compute signal pointers
         modulation->signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
         modulation->delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

         // get signal samples
         float signalData = decoder->signalStatus.signalData[modulation->signalIndex & (BUFFER_SIZE - 1)];
         float delay2Data = decoder->signalStatus.signalData[modulation->delay2Index & (BUFFER_SIZE - 1)];

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += signalData; // add new value
         modulation->filterIntegrate -= delay2Data; // remove delayed value

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
         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + signalData * bitrate->symbolAverageW1;

#ifdef DEBUG_ASK_CORR_CHANNEL
         decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, modulation->correlatedSD);
#endif
#ifdef DEBUG_ASK_SYNC_CHANNEL
         decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.0f);
#endif
         // search for Pattern-Z in PCD to PICC request
         if (modulation->correlatedSD > decoder->signalStatus.powerAverage * minimumModulationThreshold)
         {
            // signal modulation deep value
            float deepValue = decoder->signalStatus.deepData[modulation->signalIndex & (BUFFER_SIZE - 1)];

            if (modulation->searchDeepValue < deepValue)
               modulation->searchDeepValue = deepValue;

            // max correlation peak detector
            if (modulation->correlatedSD > modulation->correlationPeek)
            {
               modulation->searchPulseWidth++;
               modulation->searchPeakTime = decoder->signalClock;
               modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
               modulation->correlationPeek = modulation->correlatedSD;
            }
         }

         // wait until search finished
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

#ifdef DEBUG_ASK_SYNC_CHANNEL
         decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.750f);
#endif
         // check minimum modulation deep
         if (modulation->searchDeepValue < minimumModulationThreshold)
         {
            // reset modulation to continue search
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->searchDeepValue = 0;
            modulation->correlationPeek = 0;

            continue;
         }

         // set lower threshold to detect valid response pattern
         modulation->searchThreshold = decoder->signalStatus.powerAverage * minimumModulationThreshold;

         // set pattern search window
         modulation->symbolStartTime = modulation->searchPeakTime - bitrate->period2SymbolSamples;
         modulation->symbolEndTime = modulation->searchPeakTime + bitrate->period2SymbolSamples;

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

         // reset modulation to continue search
         modulation->searchStartTime = 0;
         modulation->searchEndTime = 0;
         modulation->searchDeepValue = 0;
         modulation->correlationPeek = 0;

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
                  frameStatus.frameEnd = symbolStatus.start - decoder->bitrate->period2SymbolSamples;
               else
                  frameStatus.frameEnd = symbolStatus.start - decoder->bitrate->period1SymbolSamples;

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
               decoder->modulation->symbolSyncTime = 0;
               decoder->modulation->filterIntegrate = 0;
               decoder->modulation->detectIntegrate = 0;
               decoder->modulation->phaseIntegrate = 0;
               decoder->modulation->searchStartTime = 0;
               decoder->modulation->searchEndTime = 0;

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
            pattern = decodeListenFrameSymbolAsk(buffer);

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
            pattern = decodeListenFrameSymbolBpsk(buffer);

            // Pattern-M found, mark frame start time
            if (pattern == PatternType::PatternM)
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
                     frameStatus.frameEnd = symbolStatus.start;

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
      symbolStatus.pattern = PatternType::Invalid;

      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

      while (decoder->nextSample(buffer))
      {
         // compute pointers
         modulation->signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
         modulation->delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

         // get signal samples
         float currentData = decoder->signalStatus.signalData[modulation->signalIndex & (BUFFER_SIZE - 1)];
         float delayedData = decoder->signalStatus.signalData[modulation->delay2Index & (BUFFER_SIZE - 1)];

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += currentData; // add new value
         modulation->filterIntegrate -= delayedData; // remove delayed value

         // correlation pointers
         modulation->filterPoint1 = (modulation->signalIndex % bitrate->period1SymbolSamples);
         modulation->filterPoint2 = (modulation->signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
         modulation->filterPoint3 = (modulation->signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;

         // store integrated signal in correlation buffer
         modulation->correlationData[modulation->filterPoint1] = modulation->filterIntegrate;

         // compute correlation factors
         modulation->correlatedS0 = modulation->correlationData[modulation->filterPoint1] - modulation->correlationData[modulation->filterPoint2];
         modulation->correlatedS1 = modulation->correlationData[modulation->filterPoint2] - modulation->correlationData[modulation->filterPoint3];
         modulation->correlatedSD = std::fabs(modulation->correlatedS0 - modulation->correlatedS1) / float(bitrate->period2SymbolSamples);

#ifdef DEBUG_ASK_CORR_CHANNEL
         decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, modulation->correlatedSD);
#endif

#ifdef DEBUG_ASK_SYNC_CHANNEL
         decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.0f);
#endif
         // compute symbol average
         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + currentData * bitrate->symbolAverageW1;

         // set next search sync window from previous state
         if (!modulation->searchStartTime)
         {
            // estimated symbol start and end
            modulation->symbolStartTime = modulation->symbolEndTime;
            modulation->symbolEndTime = modulation->symbolStartTime + bitrate->period1SymbolSamples;

            // timing search window
            modulation->searchStartTime = modulation->symbolEndTime - bitrate->period8SymbolSamples;
            modulation->searchEndTime = modulation->symbolEndTime + bitrate->period8SymbolSamples;

            // reset symbol parameters
            modulation->symbolCorr0 = 0;
            modulation->symbolCorr1 = 0;
         }

         // search max correlation peak
         if (decoder->signalClock >= modulation->searchStartTime && decoder->signalClock <= modulation->searchEndTime)
         {
            if (modulation->correlatedSD > modulation->correlationPeek)
            {
               modulation->correlationPeek = modulation->correlatedSD;
               modulation->symbolCorr0 = modulation->correlatedS0;
               modulation->symbolCorr1 = modulation->correlatedS1;
               modulation->symbolEndTime = decoder->signalClock;
            }
         }

         // capture next symbol
         if (decoder->signalClock == modulation->searchEndTime)
         {
#ifdef DEBUG_ASK_SYNC_CHANNEL
            decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.50f);
#endif
            // detect Pattern-Y when no modulation occurs (below search detection threshold)
            if (modulation->correlationPeek < modulation->searchThreshold)
            {
               // estimate symbol end from start (peak detection not valid due lack of modulation)
               modulation->symbolEndTime = modulation->symbolStartTime + bitrate->period1SymbolSamples;

               // setup symbol info
               symbolStatus.value = 1;
               symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
               symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
               symbolStatus.length = symbolStatus.end - symbolStatus.start;
               symbolStatus.pattern = PatternType::PatternY;

               break;
            }

            // detect Pattern-Z
            if (modulation->symbolCorr0 > modulation->symbolCorr1)
            {
               // setup symbol info
               symbolStatus.value = 0;
               symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
               symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
               symbolStatus.length = symbolStatus.end - symbolStatus.start;
               symbolStatus.pattern = PatternType::PatternZ;

               break;
            }

            // detect Pattern-X, setup symbol info
            symbolStatus.value = 1;
            symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
            symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
            symbolStatus.length = symbolStatus.end - symbolStatus.start;
            symbolStatus.pattern = PatternType::PatternX;

            break;
         }
      }

      // reset search status if symbol has detected
      if (symbolStatus.pattern != PatternType::Invalid)
      {
         modulation->searchStartTime = 0;
         modulation->searchEndTime = 0;
         modulation->searchPulseWidth = 0;
         modulation->correlationPeek = 0;
         modulation->correlatedSD = 0;
      }

      return symbolStatus.pattern;
   }

   /*
    * Decode one ASK modulated listen frame symbol
    */
   inline int decodeListenFrameSymbolAsk(sdr::SignalBuffer &buffer)
   {
      int pattern = PatternType::Invalid;

      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

      while (decoder->nextSample(buffer))
      {
         // compute pointers
         modulation->signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
         modulation->delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

         // get signal samples
         float signalData = decoder->signalStatus.signalData[modulation->signalIndex & (BUFFER_SIZE - 1)];

         // compute symbol average (signal offset)
         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + signalData * bitrate->symbolAverageW1;

         // remove DC offset from signal value
         signalData -= modulation->symbolAverage;

         // store signal square in filter buffer
         modulation->integrationData[modulation->signalIndex & (BUFFER_SIZE - 1)] = signalData * signalData;

         // start correlation after frameGuardTime
         if (decoder->signalClock > (frameStatus.guardEnd - bitrate->period1SymbolSamples))
         {
            // compute correlation points
            modulation->filterPoint1 = (modulation->signalIndex % bitrate->period1SymbolSamples);
            modulation->filterPoint2 = (modulation->signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
            modulation->filterPoint3 = (modulation->signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;

            // integrate symbol (moving average)
            modulation->filterIntegrate += modulation->integrationData[modulation->signalIndex & (BUFFER_SIZE - 1)]; // add new value
            modulation->filterIntegrate -= modulation->integrationData[modulation->delay2Index & (BUFFER_SIZE - 1)]; // remove delayed value

            // store integrated signal in correlation buffer
            modulation->correlationData[modulation->filterPoint1] = modulation->filterIntegrate;

            // compute correlation results for each symbol and distance
            modulation->correlatedS0 = modulation->correlationData[modulation->filterPoint1] - modulation->correlationData[modulation->filterPoint2];
            modulation->correlatedS1 = modulation->correlationData[modulation->filterPoint2] - modulation->correlationData[modulation->filterPoint3];
            modulation->correlatedSD = std::fabs(modulation->correlatedS0 - modulation->correlatedS1);
         }

#ifdef DEBUG_ASK_CORR_CHANNEL
         decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, modulation->correlatedS0);
#endif

#ifdef DEBUG_ASK_SYNC_CHANNEL
         decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, modulation->filterIntegrate);
#endif
         // search for Start Of Frame pattern (SoF)
         if (!modulation->symbolEndTime)
         {
            // frame waiting time exceeded
            if (decoder->signalClock >= frameStatus.waitingEnd)
            {
               pattern = PatternType::NoPattern;
               break;
            }

            // wait until frame guard time is reached
            if (decoder->signalClock < frameStatus.guardEnd)
               continue;

            // capture signal variance as lower level threshold
            if (decoder->signalClock == frameStatus.guardEnd)
               modulation->searchThreshold = decoder->signalStatus.signalVariance;

            // detect maximum correlation peak
            if (modulation->correlatedSD > modulation->searchThreshold)
            {
               // max correlation peak detector
               if (modulation->correlatedSD > modulation->correlationPeek)
               {
                  modulation->searchPulseWidth++;
                  modulation->searchPeakTime = decoder->signalClock;
                  modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
                  modulation->correlationPeek = modulation->correlatedSD;
               }
            }

            // Check for SoF symbol
            if (decoder->signalClock != modulation->searchEndTime)
               continue;

#ifdef DEBUG_ASK_SYNC_CHANNEL
            decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.75f);
#endif
            if (modulation->searchPulseWidth > bitrate->period8SymbolSamples)
            {
               // set pattern search window
               modulation->symbolStartTime = modulation->searchPeakTime - bitrate->period2SymbolSamples;
               modulation->symbolEndTime = modulation->searchPeakTime + bitrate->period2SymbolSamples;

               // setup symbol info
               symbolStatus.value = 1;
               symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
               symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
               symbolStatus.length = symbolStatus.end - symbolStatus.start;

               pattern = PatternType::PatternD;
               break;
            }

            // if no valid modulation is found, we restart SOF search
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->correlationPeek = 0;
            modulation->searchPulseWidth = 0;
            modulation->correlatedSD = 0;
         }

            // search Response Bit Stream
         else
         {
            // set next search sync window from previous
            if (!modulation->searchStartTime)
            {
#ifdef DEBUG_ASK_SYNC_CHANNEL
               decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.50f);
#endif
               // estimated symbol start and end
               modulation->symbolStartTime = modulation->symbolEndTime;
               modulation->symbolEndTime = modulation->symbolStartTime + bitrate->period1SymbolSamples;

               // timing search window
               modulation->searchStartTime = modulation->symbolEndTime - bitrate->period8SymbolSamples;
               modulation->searchEndTime = modulation->symbolEndTime + bitrate->period8SymbolSamples;

               // reset symbol parameters
               modulation->symbolCorr0 = 0;
               modulation->symbolCorr1 = 0;
            }

            // search symbol timings
            if (decoder->signalClock >= modulation->searchStartTime && decoder->signalClock <= modulation->searchEndTime)
            {
               if (modulation->correlatedSD > modulation->correlationPeek)
               {
                  modulation->correlationPeek = modulation->correlatedSD;
                  modulation->symbolCorr0 = modulation->correlatedS0;
                  modulation->symbolCorr1 = modulation->correlatedS1;
                  modulation->symbolEndTime = decoder->signalClock;
               }
            }

            // wait until search is finished
            if (decoder->signalClock != modulation->searchEndTime)
               continue;

            if (modulation->correlationPeek > modulation->searchThreshold)
            {
               // setup symbol info
               symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
               symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
               symbolStatus.length = symbolStatus.end - symbolStatus.start;

               if (modulation->symbolCorr0 > modulation->symbolCorr1)
               {
                  symbolStatus.value = 0;
                  pattern = PatternType::PatternE;
                  break;
               }

               symbolStatus.value = 1;
               pattern = PatternType::PatternD;
               break;
            }

            // no modulation (End Of Frame) EoF
            pattern = PatternType::PatternF;
            break;
         }
      }

      // reset search status
      if (pattern != PatternType::Invalid)
      {
         symbolStatus.pattern = pattern;

         modulation->searchStartTime = 0;
         modulation->searchEndTime = 0;
         modulation->correlationPeek = 0;
         modulation->searchPulseWidth = 0;
         modulation->correlatedSD = 0;
      }

      return pattern;
   }

   /*
    * Decode one BPSK modulated listen frame symbol
    */
   inline int decodeListenFrameSymbolBpsk(sdr::SignalBuffer &buffer)
   {
      int pattern = PatternType::Invalid;

      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

      while (decoder->nextSample(buffer))
      {
         modulation->signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
         modulation->delay1Index = (bitrate->offsetDelay1Index + decoder->signalClock);
         modulation->delay4Index = (bitrate->offsetDelay4Index + decoder->signalClock);

         // get signal samples
         float signalData = decoder->signalStatus.signalData[modulation->signalIndex & (BUFFER_SIZE - 1)];
         float delay1Data = decoder->signalStatus.signalData[modulation->delay1Index & (BUFFER_SIZE - 1)];

         // compute symbol average
         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + signalData * bitrate->symbolAverageW1;

         // multiply 1 symbol delayed signal with incoming signal
         float phase = (signalData - modulation->symbolAverage) * (delay1Data - modulation->symbolAverage);

         // store signal phase in filter buffer
         modulation->integrationData[modulation->signalIndex & (BUFFER_SIZE - 1)] = phase * 10;

         // integrate response from PICC after guard time (TR0)
         if (decoder->signalClock > (frameStatus.guardEnd - bitrate->period1SymbolSamples))
         {
            modulation->phaseIntegrate += modulation->integrationData[modulation->signalIndex & (BUFFER_SIZE - 1)]; // add new value
            modulation->phaseIntegrate -= modulation->integrationData[modulation->delay4Index & (BUFFER_SIZE - 1)]; // remove delayed value
         }

#ifdef DEBUG_BPSK_PHASE_CHANNEL
         decoder->debug->set(DEBUG_BPSK_PHASE_CHANNEL, modulation->phaseIntegrate);
#endif

         // search for Start Of Frame pattern (SoF)
         if (!modulation->symbolEndTime)
         {
            // detect first zero-cross
            if (modulation->phaseIntegrate > 0.00025f)
            {
               modulation->searchPeakTime = decoder->signalClock;
               modulation->searchEndTime = decoder->signalClock + bitrate->period2SymbolSamples;
            }

               // frame waiting time exceeded without detect modulation
            else if (decoder->signalClock == frameStatus.waitingEnd)
            {
               pattern = PatternType::NoPattern;
               break;
            }

            if (decoder->signalClock == modulation->searchEndTime)
            {
#ifdef DEBUG_BPSK_SYNC_CHANNEL
               decoder->debug->set(DEBUG_BPSK_SYNC_CHANNEL, 0.75);
#endif
               // set symbol window
               modulation->symbolSyncTime = 0;
               modulation->symbolStartTime = modulation->searchPeakTime;
               modulation->symbolEndTime = modulation->searchPeakTime + bitrate->period1SymbolSamples;
               modulation->symbolPhase = modulation->phaseIntegrate;
               modulation->phaseThreshold = std::fabs(modulation->phaseIntegrate / 3);

               // set symbol info
               symbolStatus.value = 0;
               symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
               symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
               symbolStatus.length = symbolStatus.end - symbolStatus.start;

               pattern = PatternType::PatternM;
               break;
            }
         }

            // search Response Bit Stream
         else
         {
            // edge detector for re-synchronization
            if ((modulation->phaseIntegrate > 0 && modulation->symbolPhase < 0) || (modulation->phaseIntegrate < 0 && modulation->symbolPhase > 0))
            {
               modulation->searchPeakTime = decoder->signalClock;
               modulation->symbolStartTime = decoder->signalClock;
               modulation->symbolEndTime = decoder->signalClock + bitrate->period1SymbolSamples;
               modulation->symbolSyncTime = decoder->signalClock + bitrate->period2SymbolSamples;
               modulation->symbolPhase = modulation->phaseIntegrate;
            }

            // set next search sync window from previous
            if (!modulation->symbolSyncTime)
            {
               // estimated symbol start and end
               modulation->symbolStartTime = modulation->symbolEndTime;
               modulation->symbolEndTime = modulation->symbolStartTime + bitrate->period1SymbolSamples;
               modulation->symbolSyncTime = modulation->symbolStartTime + bitrate->period2SymbolSamples;
            }

               // search symbol timings
            else if (decoder->signalClock == modulation->symbolSyncTime)
            {
#ifdef DEBUG_BPSK_SYNC_CHANNEL
               decoder->debug->set(DEBUG_BPSK_SYNC_CHANNEL, 0.50f);
#endif
               modulation->symbolPhase = modulation->phaseIntegrate;

               // setup symbol info
               symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
               symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
               symbolStatus.length = symbolStatus.end - symbolStatus.start;

               // no symbol change, keep previous symbol pattern
               if (modulation->phaseIntegrate > modulation->phaseThreshold)
               {
                  pattern = symbolStatus.pattern;
                  break;
               }

               // symbol change, invert pattern and value
               if (modulation->phaseIntegrate < -modulation->phaseThreshold)
               {
                  symbolStatus.value = !symbolStatus.value;
                  pattern = (symbolStatus.pattern == PatternType::PatternM) ? PatternType::PatternN : PatternType::PatternM;
                  break;
               }

               // no modulation detected, generate End Of Frame symbol
               pattern = PatternType::PatternO;
               break;
            }
         }
      }

      // reset search status
      if (pattern != PatternType::Invalid)
      {
         symbolStatus.pattern = pattern;

         modulation->searchStartTime = 0;
         modulation->searchEndTime = 0;
         modulation->symbolSyncTime = 0;
         modulation->correlationPeek = 0;
         modulation->searchPulseWidth = 0;
         modulation->correlatedSD = 0;
      }

      return pattern;
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
         decoder->modulation->searchPeakTime = 0;
         decoder->modulation->searchEndTime = 0;
         decoder->modulation->correlationPeek = 0;
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
         modulationStatus[rate].searchStartTime = 0;
         modulationStatus[rate].searchEndTime = 0;
         modulationStatus[rate].correlationPeek = 0;
         modulationStatus[rate].searchPulseWidth = 0;
         modulationStatus[rate].searchDeepValue = 0;
         modulationStatus[rate].symbolAverage = 0;
         modulationStatus[rate].symbolPhase = NAN;
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
            auto fsdi = (frame[1] >> 4) & 0x0F;

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
    * Check NFC-A crc
    */
   inline static bool checkCrc(NfcFrame &frame)
   {
      unsigned short crc = 0x6363; // NFC-A ITU-V.41
      unsigned short res = 0;

      int length = frame.limit();

      if (length <= 2)
         return false;

      for (int i = 0; i < length - 2; i++)
      {
         auto d = (unsigned char) frame[i];

         d = (d ^ (unsigned int) (crc & 0xff));
         d = (d ^ (d << 4));

         crc = (crc >> 8) ^ ((unsigned short) (d << 8)) ^ ((unsigned short) (d << 3)) ^ ((unsigned short) (d >> 4));
      }

      res |= ((unsigned int) frame[length - 2] & 0xff);
      res |= ((unsigned int) frame[length - 1] & 0xff) << 8;

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

void NfcA::setModulationThreshold(float min)
{
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
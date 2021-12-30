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

#include <tech/NfcF.h>

#ifdef DEBUG_SIGNAL
#define DEBUG_ASK_CORR_CHANNEL 1
#define DEBUG_ASK_SYNC_CHANNEL 1
#endif

namespace nfc {

enum PatternType
{
   Invalid = 0,
   NoPattern = 1,
   PatternL = 2,
   PatternH = 3,
   PatternS = 4,
   PatternE = 5
};

struct NfcF::Impl : NfcTech
{
   rt::Logger log {"NfcF"};

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

   // minimum modulation threshold to detect valid signal for NFC-F (default 10%)
   float minimumModulationThreshold = 0.10f;

   // minimum modulation threshold to detect valid signal for NFC-F (default 75%)
   float maximumModulationThreshold = 0.75f;

   // minimum correlation threshold to detect valid NFC-V pulse (default 50%)
   float minimumCorrelationThreshold = 0.50f;

   // last detected frame end
   unsigned int lastFrameEnd = 0;

   // chained frame flags
   unsigned int chainedFlags = 0;

   Impl(DecoderStatus *decoder) : decoder(decoder)
   {
   }

   inline void configure(long sampleRate)
   {
      log.info("--------------------------------------------");
      log.info("initializing NFC-F decoder");
      log.info("--------------------------------------------");
      log.info("\tsignalSampleRate     {}", {decoder->sampleRate});
      log.info("\tpowerLevelThreshold  {}", {decoder->powerLevelThreshold});
      log.info("\tcorrelationThreshold {}", {minimumCorrelationThreshold});
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

      // compute symbol parameters for 212Kbps and 424Kbps
      for (int rate = r212k; rate <= r424k; rate++)
      {
         // clear bitrate parameters
         bitrateParams[rate] = {0,};

         // clear modulation parameters
         modulationStatus[rate] = {0,};

         // configure bitrate parametes
         BitrateParams *bitrate = bitrateParams + rate;

         // set tech type and rate
         bitrate->techType = TechType::NfcF;
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
         bitrate->symbolDelayDetect = 0;

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
      protocolStatus.startUpGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCF_SFGT_DEF);
      protocolStatus.frameGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCF_FGT_DEF);
      protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFCF_FWT_DEF);
      protocolStatus.requestGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCF_RGT_DEF);

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

   inline bool detectModulation()
   {
      // ignore low power signals
      if (decoder->signalStatus.signalAverg < decoder->powerLevelThreshold)
         return false;

      // minimum correlation value for valid NFC-F symbols
      float minimumCorrelation = decoder->signalStatus.signalAverg * minimumModulationThreshold;

      // POLL frame ASK detector for 212Kbps and 424Kbps
      for (int rate = r212k; rate <= r424k; rate++)
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
         float correlatedS0 = modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2];
         float correlatedS1 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3];
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1) / float(bitrate->period2SymbolSamples);

#ifdef DEBUG_ASK_CORR_CHANNEL
         decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, correlatedSD);
#endif

#ifdef DEBUG_ASK_SYNC_CHANNEL
         if (decoder->signalClock == modulation->searchSyncTime)
            decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.75f);
#endif

         // wait until search start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         // detect modulation deep
         if (correlatedS1 > 0)
         {
            // reset previous detector peak values
            if (modulation->detectorPeakTime && modulation->detectorPeakTime < decoder->signalClock - bitrate->period1SymbolSamples)
            {
               modulation->detectorPeakValue = 0;
               modulation->detectorPeakTime = 0;
            }

            // maximum modulation deep
            if (signalDeep > modulation->detectorPeakValue)
            {
               modulation->detectorPeakValue = signalDeep;
               modulation->detectorPeakTime = decoder->signalClock;
            }
         }

         // detect modulation peak
         if (correlatedSD >= minimumCorrelation)
         {
            // detect maximum correlation peak
            if (correlatedSD > modulation->correlatedPeakValue)
            {
               modulation->correlatedPeakValue = correlatedSD;
               modulation->correlatedPeakTime = decoder->signalClock;

               // update search end if no search window is defined from previous symbol
               if (!modulation->searchStartTime)
                  modulation->searchEndTime = decoder->signalClock + bitrate->period8SymbolSamples;
            }
         }

         // detect synchronization in preamble and count transitions
         if (decoder->signalClock == modulation->searchSyncTime)
         {
            // capture symbol correlation at synchronization point
            modulation->searchSyncValue = correlatedSD;

            // preamble sync code starts now! detect if manchester is inverted
            if (++modulation->searchPulseWidth == 95)
            {
               // check for inverted manchester mode
               modulation->searchModeState = correlatedSD < modulation->searchLastValue / 2;
            }
         }

         // wait until search finished
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // check for valid NFC-F modulated preamble
         if (modulation->searchPulseWidth < 95)
         {
            // check for valid NFC-F modulated pulse
            if (modulation->correlatedPeakTime == 0 || // no modulation found
                modulation->detectorPeakValue < minimumModulationThreshold || // insufficient modulation deep
                modulation->detectorPeakValue > maximumModulationThreshold || // excessive modulation deep
                modulation->searchSyncValue < modulation->searchLastValue / 2) // pulse too low
            {
               // reset modulation to continue search
               modulation->searchModeState = 0;
               modulation->searchSyncTime = 0;
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;
               modulation->searchPulseWidth = 0;
               modulation->searchSyncValue = 0;
               modulation->searchLastValue = 0;
               modulation->correlatedPeakValue = 0;
               modulation->symbolStartTime = 0;
               modulation->symbolEndTime = 0;
               continue;
            }
         }

         // set symbol timings, first symbol and rest
         if (!modulation->symbolStartTime)
            modulation->symbolStartTime = modulation->correlatedPeakTime - bitrate->period2SymbolSamples;
         else
            modulation->symbolEndTime = modulation->searchSyncTime;

         // set next synchronization point for current manchester mode
         if (modulation->searchModeState)
            modulation->searchSyncTime = modulation->searchSyncTime + bitrate->period2SymbolSamples;
         else
            modulation->searchSyncTime = modulation->correlatedPeakTime + bitrate->period2SymbolSamples;

         // set next search window parameters
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
         modulation->searchLastValue = modulation->correlatedPeakValue;

         // sets next search window
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // wait until preamble finished (48 bits)
         if (modulation->searchPulseWidth < 95)
            continue;

         // set signal threshold for modulation detector
         modulation->signalValueThreshold = minimumCorrelation;

         // setup symbol info
         symbolStatus.start = modulation->symbolStartTime;
         symbolStatus.end = modulation->symbolEndTime;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;
         symbolStatus.pattern = PatternType::PatternS;

         // setup frame info
         frameStatus.frameType = PollFrame;
         frameStatus.symbolRate = bitrate->symbolsPerSecond;
         frameStatus.frameStart = symbolStatus.start;
         frameStatus.frameEnd = 0;

         decoder->bitrate = bitrate;
         decoder->modulation = modulation;

         return true;
      }

      return false;
   }

   inline void decodeFrame(sdr::SignalBuffer &samples, std::list<NfcFrame> &frames)
   {
      if (frameStatus.frameType == PollFrame)
      {
         decodePollFrame(samples, frames);
      }

      if (frameStatus.frameType == ListenFrame)
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
            // a valid frame must contain at least one byte of data
            if (streamStatus.bytes > 0)
            {
               // set last symbol timing
               frameStatus.frameEnd = symbolStatus.end;

               NfcFrame request = NfcFrame(TechType::NfcF, FrameType::PollFrame);

               request.setFrameRate(decoder->bitrate->symbolsPerSecond);
               request.setSampleStart(frameStatus.frameStart);
               request.setSampleEnd(frameStatus.frameEnd);
               request.setTimeStart(double(frameStatus.frameStart) / double(decoder->sampleRate));
               request.setTimeEnd(double(frameStatus.frameEnd) / double(decoder->sampleRate));

               if (truncateError)
                  request.setFrameFlags(FrameFlags::Truncated);

               // add bytes to frame and flip to prepare read
               request.put(streamStatus.buffer, streamStatus.bytes).flip();

               // clear modulation status for next frame search
               decoder->modulation->searchModeState = 0;
               decoder->modulation->symbolStartTime = 0;
               decoder->modulation->symbolEndTime = 0;
               decoder->modulation->searchSyncTime = 0;
               decoder->modulation->searchStartTime = 0;
               decoder->modulation->searchEndTime = 0;
               decoder->modulation->searchSyncValue = 0;
               decoder->modulation->searchLastValue = 0;
               decoder->modulation->correlatedPeakValue = 0;
               decoder->modulation->searchPulseWidth = 0;

               // clear stream status
               streamStatus = {0,};

               // process frame
               process(request);

               // add to frame list
               frames.push_back(request);

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
   inline bool decodeListenFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
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
               if (streamStatus.bytes > 0)
               {
                  // set last symbol timing
                  frameStatus.frameEnd = symbolStatus.end;

                  // build response frame
                  NfcFrame response = NfcFrame(TechType::NfcF, FrameType::ListenFrame);

                  response.setFrameRate(decoder->bitrate->symbolsPerSecond);
                  response.setSampleStart(frameStatus.frameStart);
                  response.setSampleEnd(frameStatus.frameEnd);
                  response.setTimeStart(double(frameStatus.frameStart) / double(decoder->sampleRate));
                  response.setTimeEnd(double(frameStatus.frameEnd) / double(decoder->sampleRate));

                  if (truncateError)
                     response.setFrameFlags(FrameFlags::Truncated);

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
         if (correlatedSD > modulation->signalValueThreshold && correlatedSD > modulation->correlatedPeakValue)
         {
            modulation->correlatedPeakValue = correlatedSD;
            modulation->correlatedPeakTime = decoder->signalClock;
         }

         // capture symbol correlation values at synchronization point
         if (decoder->signalClock == modulation->searchSyncTime)
         {
            if (modulation->searchModeState)
            {
               // inverted manchester code
               modulation->symbolCorr0 = correlatedS1;
               modulation->symbolCorr1 = correlatedS0;
            }
            else
            {
               modulation->symbolCorr0 = correlatedS0;
               modulation->symbolCorr1 = correlatedS1;
            }
         }

         // wait until correlation search finish
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // no modulation (End Of Frame) EoF
         if (!modulation->correlatedPeakTime)
            return PatternType::PatternE;

         // detect Pattern type
         if (modulation->symbolCorr0 > modulation->symbolCorr1)
         {
            symbolStatus.value = 0;
            symbolStatus.pattern = PatternType::PatternL;
         }
         else
         {
            symbolStatus.value = 1;
            symbolStatus.pattern = PatternType::PatternH;
         }

         // synchronize symbol start and end time
         modulation->symbolStartTime = modulation->symbolEndTime;
         modulation->symbolEndTime = modulation->correlatedPeakTime;

         // sets next search window
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period4SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period4SymbolSamples;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // setup symbol info
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;

         return symbolStatus.pattern;
      }

      return PatternType::Invalid;
   }

   /*
   * Decode SOF ASK modulated listen frame symbol
   */
   inline int decodeListenFrameStartAsk(sdr::SignalBuffer &buffer)
   {
      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;
         ++delay2Index;

         if (decoder->signalClock == 1180000)
            log.info("");

         // get signal samples
         float signalData = decoder->signalStatus.signalData[signalIndex & (BUFFER_SIZE - 1)];
         float delay2Data = decoder->signalStatus.signalData[delay2Index & (BUFFER_SIZE - 1)];

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
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1) / float(bitrate->period2SymbolSamples);

         // get signal deep
         float signalDeep = decoder->signalStatus.signalDeep[signalIndex & (BUFFER_SIZE - 1)];

#ifdef DEBUG_ASK_CORR_CHANNEL
         decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, correlatedSD);
#endif

#ifdef DEBUG_ASK_SYNC_CHANNEL
         if (decoder->signalClock == modulation->searchSyncTime)
            decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.75f);
#endif

         // wait until frame guard time is reached
         if (decoder->signalClock < frameStatus.guardEnd)
            continue;

         // using signal st.dev as lower level threshold
         if (decoder->signalClock == frameStatus.guardEnd)
            modulation->signalValueThreshold = decoder->signalStatus.signalMdev[signalIndex & (BUFFER_SIZE - 1)] * 10;

         // check for maximum response time
         if (decoder->signalClock > frameStatus.waitingEnd)
            return PatternType::NoPattern;

         // wait until search start (or no search at all)
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         // detect modulation deep
         if (correlatedSD > 0)
         {
            // reset previous detector peak values
            if (modulation->detectorPeakTime && modulation->detectorPeakTime < decoder->signalClock - bitrate->period1SymbolSamples)
            {
               modulation->detectorPeakValue = 0;
               modulation->detectorPeakTime = 0;
            }

            // maximum modulation deep
            if (signalDeep > modulation->detectorPeakValue)
            {
               modulation->detectorPeakValue = signalDeep;
               modulation->detectorPeakTime = decoder->signalClock;
            }
         }

         // detect maximum correlation peak
         if (correlatedSD > modulation->signalValueThreshold)
         {
            if (correlatedSD > modulation->correlatedPeakValue)
            {
               modulation->correlatedPeakValue = correlatedSD;
               modulation->correlatedPeakTime = decoder->signalClock;

               // update search end if no search window is defined from previous symbol
               if (!modulation->searchStartTime)
                  modulation->searchEndTime = decoder->signalClock + bitrate->period8SymbolSamples;
            }
         }

         // detect synchronization in preamble and count transitions
         if (decoder->signalClock == modulation->searchSyncTime)
         {
            // capture symbol correlation at synchronization point
            modulation->searchSyncValue = correlatedSD;

            // preamble sync code starts now! detect if manchester is inverted
            if (++modulation->searchPulseWidth == 95)
            {
               // check for inverted manchester mode
               modulation->searchModeState = correlatedSD < modulation->searchLastValue / 2;
            }
         }

         // wait until search finished
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // check for valid NFC-F modulated preamble
         if (modulation->searchPulseWidth < 95)
         {
            // check for valid NFC-F modulated response
            if (modulation->correlatedPeakTime == 0 || // no modulation found
//                modulation->detectorPeekValue > minimumModulationThreshold || // excessive modulation deep, other modulation detected
                modulation->searchSyncValue < modulation->searchLastValue / 2) // pulse too low
            {
               // reset modulation to continue search
               modulation->searchModeState = 0;
               modulation->searchSyncTime = 0;
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;
               modulation->searchPulseWidth = 0;
               modulation->searchSyncValue = 0;
               modulation->searchLastValue = 0;
               modulation->correlatedPeakValue = 0;
               modulation->symbolStartTime = 0;
               modulation->symbolEndTime = 0;
               continue;
            }
         }

         // set symbol timings, first symbol and rest
         if (!modulation->symbolStartTime)
            modulation->symbolStartTime = modulation->correlatedPeakTime - bitrate->period2SymbolSamples;
         else
            modulation->symbolEndTime = modulation->searchSyncTime;

         // set next synchronization point for current manchester mode
         if (modulation->searchModeState)
            modulation->searchSyncTime = modulation->searchSyncTime + bitrate->period2SymbolSamples;
         else
            modulation->searchSyncTime = modulation->correlatedPeakTime + bitrate->period2SymbolSamples;

         // set next search window parameters
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period8SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period8SymbolSamples;
         modulation->searchLastValue = modulation->correlatedPeakValue;

         // sets next search window
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // wait until preamble finished (48 bits)
         if (modulation->searchPulseWidth < 95)
            continue;

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
   inline int decodeListenFrameSymbolAsk(sdr::SignalBuffer &buffer)
   {
      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;
         ++delay2Index;

         // get signal samples
         float signalData = decoder->signalStatus.signalData[signalIndex & (BUFFER_SIZE - 1)];
         float delay2Data = decoder->signalStatus.signalData[delay2Index & (BUFFER_SIZE - 1)];

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
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1) / float(bitrate->period2SymbolSamples);

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
         if (correlatedSD > modulation->signalValueThreshold && correlatedSD > modulation->correlatedPeakValue)
         {
            modulation->correlatedPeakValue = correlatedSD;
            modulation->correlatedPeakTime = decoder->signalClock;
         }

         // capture symbol correlation values at synchronization point
         if (decoder->signalClock == modulation->searchSyncTime)
         {
            if (modulation->searchModeState)
            {
               // inverted manchester code
               modulation->symbolCorr0 = correlatedS1;
               modulation->symbolCorr1 = correlatedS0;
            }
            else
            {
               modulation->symbolCorr0 = correlatedS0;
               modulation->symbolCorr1 = correlatedS1;
            }
         }

         // wait until correlation search finish
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // no modulation (End Of Frame) EoF
         if (!modulation->correlatedPeakTime)
            return PatternType::PatternE;

         // detect Pattern type
         if (modulation->symbolCorr0 > modulation->symbolCorr1)
         {
            symbolStatus.value = 0;
            symbolStatus.pattern = PatternType::PatternL;
         }
         else
         {
            symbolStatus.value = 1;
            symbolStatus.pattern = PatternType::PatternH;
         }

         // synchronize symbol start and end time
         modulation->symbolStartTime = modulation->symbolEndTime;
         modulation->symbolEndTime = modulation->correlatedPeakTime;

         // sets next search window
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;
         modulation->searchStartTime = modulation->searchSyncTime - bitrate->period4SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + bitrate->period4SymbolSamples;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // setup symbol info
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;

         return symbolStatus.pattern;
      }

      return PatternType::Invalid;
   }

   /*
    * Reset modulation status
    */
   inline void resetModulation()
   {
      // reset modulation detection for all rates
      for (int rate = r212k; rate <= r424k; rate++)
      {
         modulationStatus[rate].searchModeState = 0;
         modulationStatus[rate].searchStartTime = 0;
         modulationStatus[rate].searchEndTime = 0;
         modulationStatus[rate].searchSyncTime = 0;
         modulationStatus[rate].searchSyncValue = 0;
         modulationStatus[rate].searchLastValue = 0;
         modulationStatus[rate].searchPulseWidth = 0;
         modulationStatus[rate].correlatedPeakValue = 0;
         modulationStatus[rate].symbolStartTime = 0;
         modulationStatus[rate].symbolEndTime = 0;
         modulationStatus[rate].symbolAverage = 0;
         modulationStatus[rate].symbolCorr0 = 0;
         modulationStatus[rate].symbolCorr1 = 0;
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
         if (processREQC(frame))
            break;

         processOther(frame);

      } while (false);

      // set chained flags
      frame.setFrameFlags(chainedFlags);

      // for request frame set response timings
      if (frame.isPollFrame())
      {
         // update frame timing parameters for receive PICC frame
         if (decoder->bitrate)
         {
            // response guard time TR0min (PICC must not modulate response within this period)
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
    * Process REQB/WUPB frame
    */
   inline bool processREQC(NfcFrame &frame)
   {
      if (frame.isPollFrame())
      {
         if (frame[3] == CommandType::NFCB_REQC)
         {
            frameStatus.lastCommand = frame[3];

            // This commands starts or wakeup card communication, so reset the protocol parameters to the default values
            protocolStatus.maxFrameSize = 256;
            protocolStatus.startUpGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCF_SFGT_DEF);
            protocolStatus.frameGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCF_FGT_DEF);
            protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFCF_FWT_DEF);
            protocolStatus.requestGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCF_RGT_DEF);

            // The REQ-F Response must start between this range
            frameStatus.frameGuardTime = decoder->signalParams.sampleTimeUnit * NFC_TR0_MIN; // ATQ-B response guard
            frameStatus.frameWaitingTime = decoder->signalParams.sampleTimeUnit * NFCF_FWT_ATQC; // ATQ-C response timeout

            // clear chained flags
            chainedFlags = 0;

            // set frame flags
            frame.setFramePhase(FramePhase::SelectionFrame);
//            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      if (frame.isListenFrame())
      {
         if (frameStatus.lastCommand == CommandType::NFCB_REQC)
         {
//            int fdsi = (frame[10] >> 4) & 0x0f;
//            int fwi = (frame[11] >> 4) & 0x0f;
//
//            // This commands update protocol parameters
//            protocolStatus.maxFrameSize = NFC_FDS_TABLE[fdsi];
//            protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFC_FWT_TABLE[fwi]);

            frame.setFramePhase(FramePhase::SelectionFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            log.info("ATQC protocol timing parameters");
            log.info("  maxFrameSize {} bytes", {protocolStatus.maxFrameSize});
            log.info("  frameWaitingTime {} samples ({} us)", {protocolStatus.frameWaitingTime, 1E6 * protocolStatus.frameWaitingTime / decoder->sampleRate});

            return true;
         }
      }

      return false;
   }

   /*
    * Process other frames
    */
   inline void processOther(NfcFrame &frame)
   {
      frame.setFramePhase(FramePhase::ApplicationFrame);
      frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);
   }

   /*
    * Check NFC-F crc
    */
   inline bool checkCrc(NfcFrame &frame)
   {
      int size = frame.limit();

      if (size < 3)
         return false;

      unsigned short crc = crc16(frame, 2, size - 2, 0x0000, false);
      unsigned short res = (((unsigned int) frame[size - 2] & 0xff) << 8) | (unsigned int) frame[size - 1] & 0xff;

      return res == crc;
   }
};

NfcF::NfcF(DecoderStatus *decoder) : self(new Impl(decoder))
{
}

NfcF::~NfcF()
{
   delete self;
}

void NfcF::setModulationThreshold(float min, float max)
{
   if (!std::isnan(min))
      self->minimumModulationThreshold = min;

   if (!std::isnan(max))
      self->maximumModulationThreshold = max;
}

void NfcF::setCorrelationThreshold(float value)
{
   if (!std::isnan(value))
      self->minimumCorrelationThreshold = value;
}

void NfcF::configure(long sampleRate)
{
   self->configure(sampleRate);
}

bool NfcF::detect()
{
   return self->detectModulation();
}

void NfcF::decode(sdr::SignalBuffer &samples, std::list<NfcFrame> &frames)
{
   self->decodeFrame(samples, frames);
}

}

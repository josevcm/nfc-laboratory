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

#include <tech/NfcB.h>

#ifdef DEBUG_SIGNAL
#define DEBUG_ASK_EDGE_CHANNEL 2
#define DEBUG_ASK_SYNC_CHANNEL 2
#define DEBUG_BPSK_PHASE_CHANNEL 2
#define DEBUG_BPSK_SYNC_CHANNEL 2
#endif

#define SOF_BEGIN 0
#define SOF_IDLE 1
#define SOF_END 2

namespace nfc {

enum PatternType
{
   Invalid = 0,
   NoPattern = 1,
   PatternL = 2,
   PatternH = 3,
   PatternS = 4,
   PatternM = 5,
   PatternN = 6,
   PatternO = 7
};

struct NfcB::Impl
{
   rt::Logger log {"NfcB"};

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

   // minimum modulation threshold to detect valid signal for NFC-B (default 10%)
   float minimumModulationThreshold = 0.10f;

   // minimum modulation threshold to detect valid signal for NFC-B (default 75%)
   float maximumModulationThreshold = 0.75f;

   // last detected frame end
   unsigned int lastFrameEnd = 0;

   // chained frame flags
   unsigned int chainedFlags = 0;

   explicit Impl(DecoderStatus *decoder) : decoder(decoder)
   {
   }

   /*
    * Configure NFC-B modulation
    */
   inline void configure(long sampleRate)
   {
      log.info("--------------------------------------------");
      log.info("initializing NFC-B decoder");
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
         bitrate->techType = TechType::NfcB;
         bitrate->rateType = rate;

         // symbol timing parameters
         bitrate->symbolsPerSecond = int(std::round(NFC_FC / (128 >> rate)));

         // number of samples per symbol
         bitrate->period0SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * (256 >> rate))); // double symbol samples
         bitrate->period1SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * (128 >> rate))); // full symbol samples
         bitrate->period2SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * (64 >> rate))); // half symbol samples
         bitrate->period4SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * (32 >> rate))); // quarter of symbol...
         bitrate->period8SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * (16 >> rate))); // and so on...

         // delay guard for each symbol rate
         bitrate->symbolDelayDetect = rate > r106k ? bitrateParams[rate - 1].symbolDelayDetect + bitrateParams[rate - 1].period1SymbolSamples : bitrate->period0SymbolSamples;

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
      protocolStatus.startUpGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCB_SFGT_DEF);
      protocolStatus.frameGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCB_FGT_DEF);
      protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFCB_FWT_DEF);
      protocolStatus.requestGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCB_RGT_DEF);

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
    * Detect NFC-B modulation
    */
   inline bool detectModulation()
   {
      // ignore low power signals
      if (decoder->signalStatus.signalAverg < decoder->powerLevelThreshold)
         return false;

      // POLL frame ASK detector for  106Kbps, 212Kbps and 424Kbps
      for (int rate = r106k; rate <= r424k; rate++)
      {
         BitrateParams *bitrate = bitrateParams + rate;
         ModulationStatus *modulation = modulationStatus + rate;

         // compute signal pointer
         modulation->signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);

         // signal edge detector value
         float edgeValue = decoder->signalStatus.signalEdge[modulation->signalIndex & (BUFFER_SIZE - 1)];

         // signal modulation deep value
         float deepValue = decoder->signalStatus.signalDeep[modulation->signalIndex & (BUFFER_SIZE - 1)];

#ifdef DEBUG_ASK_EDGE_CHANNEL
         decoder->debug->set(DEBUG_ASK_EDGE_CHANNEL, edgeValue * 10);
#endif
         // reset modulation if exceed limits
         if (deepValue > maximumModulationThreshold)
         {
            modulation->searchStage = SOF_BEGIN;
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->detectorPeek = 0;

            return false;
         }

         // search for first falling edge
         switch (modulation->searchStage)
         {
            case SOF_BEGIN:

               // detect edge at maximum peak
               if (edgeValue < -0.001 && modulation->detectorPeek > edgeValue && deepValue > minimumModulationThreshold)
               {
                  modulation->detectorPeek = edgeValue;
                  modulation->searchPeakTime = decoder->signalClock;
                  modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
               }

               // wait until search finished
               if (decoder->signalClock != modulation->searchEndTime)
                  break;

               // sets SOF symbol frame start (also frame start)
               modulation->symbolStartTime = modulation->searchPeakTime - bitrate->period8SymbolSamples;

               // and triger next stage
               modulation->searchStage = SOF_IDLE;
               modulation->searchStartTime = modulation->searchPeakTime + (10 * bitrate->period1SymbolSamples) - bitrate->period2SymbolSamples; // search falling edge up to 11 etu
               modulation->searchEndTime = modulation->searchPeakTime + (11 * bitrate->period1SymbolSamples) + bitrate->period2SymbolSamples; // search falling edge up to 11 etu
               modulation->searchPeakTime = 0;
               modulation->detectorPeek = 0;

               break;

            case SOF_IDLE:

               // rising edge must be between 10 and 11 etus
               if (decoder->signalClock > modulation->searchStartTime)
               {
                  // detect edge at maximum peak
                  if (edgeValue > 0.001 && modulation->detectorPeek < edgeValue)
                  {
                     modulation->detectorPeek = edgeValue;
                     modulation->searchPeakTime = decoder->signalClock;
                     modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
                  }

                  // wait until search finished
                  if (decoder->signalClock != modulation->searchEndTime)
                     break;

                  // if no edge detected reset modulation search
                  if (!modulation->searchPeakTime)
                  {
                     // if no edge found, restart search
                     modulation->searchStage = SOF_BEGIN;
                     modulation->searchStartTime = 0;
                     modulation->searchEndTime = 0;
                     modulation->searchPeakTime = 0;
                     modulation->detectorPeek = 0;
                     modulation->symbolStartTime = 0;
                     modulation->symbolEndTime = 0;

                     break;
                  }

                  // trigger last search stage
                  modulation->searchStage = SOF_END;
                  modulation->searchStartTime = modulation->searchPeakTime + (2 * bitrate->period1SymbolSamples) - bitrate->period2SymbolSamples; // search falling edge up to 11 etu
                  modulation->searchEndTime = modulation->searchPeakTime + (3 * bitrate->period1SymbolSamples) + bitrate->period2SymbolSamples; // search falling edge up to 11 etu
                  modulation->searchPeakTime = 0;
                  modulation->detectorPeek = 0;
               }

               else if (edgeValue > 0.001)
               {
                  // during SOF there must not be modulation changes
                  modulation->searchStage = SOF_BEGIN;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->searchPeakTime = 0;
                  modulation->detectorPeek = 0;
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;
               }

               break;

            case SOF_END:

               // falling edge must be between 2 and 3 etus
               if (decoder->signalClock < modulation->searchStartTime)
                  break;

               // detect edge at maximum peak
               if (edgeValue < -0.001 && modulation->detectorPeek > edgeValue && deepValue > minimumModulationThreshold)
               {
                  modulation->detectorPeek = edgeValue;
                  modulation->searchPeakTime = decoder->signalClock;
                  modulation->searchEndTime = decoder->signalClock + bitrate->period8SymbolSamples;
               }

               // wait until search finished
               if (decoder->signalClock != modulation->searchEndTime)
                  break;

               // if no edge detected reset modulation search
               if (!modulation->searchPeakTime)
               {
                  modulation->searchStage = SOF_BEGIN;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->searchPeakTime = 0;
                  modulation->detectorPeek = 0;
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;

                  break;
               }

               // set SOF symbol parameters
               modulation->symbolEndTime = modulation->searchPeakTime - bitrate->period8SymbolSamples;
               modulation->symbolSyncTime = 0;

               // reset modulation for next search
               modulation->searchStage = SOF_BEGIN;
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;
               modulation->searchDeepValue = 0;
               modulation->detectorPeek = 0;

               // setup frame info
               frameStatus.frameType = PollFrame;
               frameStatus.symbolRate = bitrate->symbolsPerSecond;
               frameStatus.frameStart = modulation->symbolStartTime - bitrate->symbolDelayDetect;
               frameStatus.frameEnd = 0;

               // modulation detected
               decoder->bitrate = bitrate;
               decoder->modulation = modulation;

               return true;
         }
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
      bool frameEnd = false, truncateError = false, streamError = false;

      // decode remaining request frame
      while ((pattern = decodePollFrameSymbolAsk(buffer)) > PatternType::NoPattern)
      {
         // frame ends if found 10 ETU width Pattern-L (10 consecutive bits at value 0)
         if (streamStatus.bits == 9 && !streamStatus.data && pattern == PatternType::PatternL)
            frameEnd = true;

            // frame ends width stream error if start bit is PatternH or end bit is pattern L
         else if ((streamStatus.bits == 0 && pattern == PatternType::PatternH) || (streamStatus.bits == 9 && pattern == PatternType::PatternL))
            streamError = true;

            // frame ends width truncate error max frame size is reached
         else if (streamStatus.bytes == protocolStatus.maxFrameSize)
            truncateError = true;

         // detect end of frame
         if (frameEnd || streamError || truncateError)
         {
            // a valid frame must contain at least one byte of data
            if (streamStatus.bytes > 0)
            {
               frameStatus.frameEnd = symbolStatus.end;

               NfcFrame response = NfcFrame(TechType::NfcB, FrameType::PollFrame);

               response.setFrameRate(decoder->bitrate->symbolsPerSecond);
               response.setSampleStart(frameStatus.frameStart);
               response.setSampleEnd(frameStatus.frameEnd);
               response.setTimeStart(double(frameStatus.frameStart) / double(decoder->sampleRate));
               response.setTimeEnd(double(frameStatus.frameEnd) / double(decoder->sampleRate));

               if (truncateError || streamError)
                  response.setFrameFlags(FrameFlags::Truncated);

               // add bytes to frame and flip to prepare read
               response.put(streamStatus.buffer, streamStatus.bytes).flip();

               // clear modulation status for next frame search
               decoder->modulation->symbolStartTime = 0;
               decoder->modulation->symbolEndTime = 0;
               decoder->modulation->symbolSyncTime = 0;
               decoder->modulation->filterIntegrate = 0;
               decoder->modulation->detectIntegrate = 0;
               decoder->modulation->phaseIntegrate = 0;
               decoder->modulation->searchStartTime = 0;
               decoder->modulation->searchEndTime = 0;
               decoder->modulation->searchPulseWidth = 0;

               // clear stream status
               streamStatus = {0,};

               // process frame
               process(response);

               // add to frame list
               frames.push_back(response);

               return true;
            }

            // reset modulation and restart frame detection
            resetModulation();

            // no valid frame found
            return false;
         }

         // decode next bit
         if (streamStatus.bits < 9)
         {
            if (streamStatus.bits > 0)
               streamStatus.data |= (symbolStatus.value << (streamStatus.bits - 1));

            streamStatus.bits++;
         }
            // store full byte in stream buffer
         else
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
      bool frameEnd = false, truncateError = false, streamError = false;

      if (!frameStatus.frameStart)
      {
         // detect SOF pattern
         pattern = decodeListenFrameStartBpsk(buffer);

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
         while ((pattern = decodeListenFrameSymbolBpsk(buffer)) > PatternType::NoPattern)
         {
            // frame ends if found 10 ETU width Pattern-M (10 consecutive bits at value 0)
            if (streamStatus.bits == 9 && !streamStatus.data && pattern == PatternType::PatternM)
               frameEnd = true;

               // frame stream error if start bit is PatternN (1) or end bit is pattern M (0)
            else if ((streamStatus.bits == 0 && pattern == PatternType::PatternN) || (streamStatus.bits == 9 && pattern == PatternType::PatternM))
               streamError = true;

               // frame ends width truncate error max frame size is reached
            else if (streamStatus.bytes == protocolStatus.maxFrameSize)
               truncateError = true;

            // detect end of frame
            if (frameEnd || streamError || truncateError)
            {
               // frames must contain at least one full byte
               if (streamStatus.bytes > 0)
               {
                  frameStatus.frameEnd = symbolStatus.end;

                  // build response frame
                  NfcFrame response = NfcFrame(TechType::NfcB, FrameType::ListenFrame);

                  response.setFrameRate(decoder->bitrate->symbolsPerSecond);
                  response.setSampleStart(frameStatus.frameStart);
                  response.setSampleEnd(frameStatus.frameEnd);
                  response.setTimeStart(double(frameStatus.frameStart) / double(decoder->sampleRate));
                  response.setTimeEnd(double(frameStatus.frameEnd) / double(decoder->sampleRate));

                  if (truncateError || streamError)
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
            if (streamStatus.bits < 9)
            {
               if (streamStatus.bits > 0)
                  streamStatus.data |= (symbolStatus.value << (streamStatus.bits - 1));

               streamStatus.bits++;
            }
               // store full byte in stream buffer
            else
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
      symbolStatus.pattern = PatternType::Invalid;

      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

      while (decoder->nextSample(buffer))
      {
         // compute signal pointer
         modulation->signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);

         // signal edge detector value
         float edgeValue = decoder->signalStatus.signalEdge[modulation->signalIndex & (BUFFER_SIZE - 1)];

         // signal modulation deep value
         float deepValue = decoder->signalStatus.signalDeep[modulation->signalIndex & (BUFFER_SIZE - 1)];

#ifdef DEBUG_ASK_EDGE_CHANNEL
         decoder->debug->set(DEBUG_ASK_EDGE_CHANNEL, edgeValue * 10);
#endif

         // edge re-synchronization window
         if (decoder->signalClock > modulation->searchStartTime && decoder->signalClock < modulation->searchEndTime)
         {
            // ignore edge type
            edgeValue = std::abs(edgeValue);

            // detect edge at maximum peak
            if (edgeValue > 0.001 && modulation->detectorPeek < edgeValue && deepValue > minimumModulationThreshold)
            {
               modulation->detectorPeek = edgeValue;
               modulation->symbolEndTime = decoder->signalClock - bitrate->period8SymbolSamples;
               modulation->symbolSyncTime = 0;
            }
         }

         // estimate next symbol timings
         if (!modulation->symbolSyncTime)
         {
            // estimated symbol start and end
            modulation->symbolStartTime = modulation->symbolEndTime;
            modulation->symbolEndTime = modulation->symbolStartTime + bitrate->period1SymbolSamples;
            modulation->symbolSyncTime = modulation->symbolStartTime + bitrate->period2SymbolSamples;
         }

         // wait until sync time is reached
         if (decoder->signalClock != modulation->symbolSyncTime)
            continue;

#ifdef DEBUG_ASK_SYNC_CHANNEL
         decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.50f);
#endif
         // modulated signal, symbol L -> 0 value
         if (deepValue > minimumModulationThreshold)
         {
            // setup symbol info
            symbolStatus.value = 0;
            symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
            symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
            symbolStatus.length = symbolStatus.end - symbolStatus.start;
            symbolStatus.pattern = PatternType::PatternL;
         }

            // non modulated signal, symbol H -> 1 value
         else
         {
            // setup symbol info
            symbolStatus.value = 1;
            symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
            symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
            symbolStatus.length = symbolStatus.end - symbolStatus.start;
            symbolStatus.pattern = PatternType::PatternH;
         }

         // next edge re-synchronization window
         modulation->searchStartTime = modulation->symbolEndTime - bitrate->period4SymbolSamples;
         modulation->searchEndTime = modulation->symbolEndTime + bitrate->period4SymbolSamples;

         // reset status for next symbol
         modulation->symbolSyncTime = 0;
         modulation->detectorPeek = 0;

         break;
      }

      return symbolStatus.pattern;
   }

   /*
    * Decode SOF BPSK modulated listen frame symbol
    */
   inline int decodeListenFrameStartBpsk(sdr::SignalBuffer &buffer)
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
         float signalMDev = decoder->signalStatus.signalMdev[modulation->signalIndex & (BUFFER_SIZE - 1)];

         // compute symbol average
         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + signalData * bitrate->symbolAverageW1;

         // multiply 1 symbol delayed signal with incoming signal
         float phase = (signalData - modulation->symbolAverage) * (delay1Data - modulation->symbolAverage) * 10;

#ifdef DEBUG_BPSK_PHASE_CHANNEL
         decoder->debug->set(DEBUG_BPSK_PHASE_CHANNEL - 1, phase);
#endif
         // store signal phase in filter buffer
         modulation->integrationData[modulation->signalIndex & (BUFFER_SIZE - 1)] = phase;

#ifdef DEBUG_BPSK_PHASE_CHANNEL
         decoder->debug->set(DEBUG_BPSK_PHASE_CHANNEL, signalMDev);
#endif
         // integrate response from PICC after guard time (TR0)
         if (decoder->signalClock < (frameStatus.guardEnd - bitrate->period1SymbolSamples))
            continue;

         modulation->phaseIntegrate += modulation->integrationData[modulation->signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->phaseIntegrate -= modulation->integrationData[modulation->delay4Index & (BUFFER_SIZE - 1)]; // remove delayed value

         // wait until frame guard time is reached
         if (decoder->signalClock < frameStatus.guardEnd)
            continue;

         // using signal st.dev as lower level threshold
         if (decoder->signalClock == frameStatus.guardEnd)
            modulation->searchThreshold = signalMDev;

#ifdef DEBUG_BPSK_PHASE_CHANNEL
         decoder->debug->set(DEBUG_BPSK_PHASE_CHANNEL, modulation->searchThreshold);
#endif

         // search for Start Of Frame pattern (SoF)
         switch (modulation->searchStage)
         {
            case SOF_BEGIN:

               // detect first zero-cross
               if (modulation->phaseIntegrate > modulation->searchThreshold)
               {
#ifdef DEBUG_BPSK_PHASE_CHANNEL
                  decoder->debug->set(DEBUG_BPSK_PHASE_CHANNEL, modulation->phaseIntegrate);
#endif
                  modulation->searchPeakTime = decoder->signalClock;
                  modulation->searchEndTime = decoder->signalClock + bitrate->period2SymbolSamples;
                  modulation->searchPulseWidth++;
               }

               // frame waiting time exceeded without detect modulation
               if (decoder->signalClock >= frameStatus.waitingEnd)
               {
                  pattern = PatternType::NoPattern;
                  break;
               }

               // wait until search finished
               if (decoder->signalClock != modulation->searchEndTime)
                  break;

               // must start with 10etu un-modulated pulse
               if (modulation->searchPulseWidth >= bitrate->period1SymbolSamples * 10)
               {
#ifdef DEBUG_BPSK_SYNC_CHANNEL
                  decoder->debug->set(DEBUG_BPSK_SYNC_CHANNEL, 0.75);
#endif
                  // if edge found, set SOF symbol start
                  modulation->symbolStartTime = modulation->searchPeakTime;

                  // and trigger next stage
                  modulation->searchStage = SOF_IDLE;
                  modulation->searchStartTime = modulation->searchPeakTime + (10 * bitrate->period1SymbolSamples) - bitrate->period2SymbolSamples; // search falling edge up to 11 etu
                  modulation->searchEndTime = modulation->searchPeakTime + (11 * bitrate->period1SymbolSamples) + bitrate->period2SymbolSamples; // search falling edge up to 11 etu
                  modulation->searchPeakTime = 0;
               }
               else
               {
                  // if no valid edge is found, we restart SOF search
                  modulation->searchStage = SOF_BEGIN;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->searchPeakTime = 0;
                  modulation->searchPulseWidth = 0;
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;
               }

               break;

            case SOF_IDLE:

#ifdef DEBUG_BPSK_PHASE_CHANNEL
               decoder->debug->set(DEBUG_BPSK_PHASE_CHANNEL, modulation->phaseIntegrate);
#endif
               // wait until search start
               if (decoder->signalClock < modulation->searchStartTime)
                  break;

               // detect second zero-cross
               if (modulation->phaseIntegrate > 0.001f)
               {
                  modulation->searchPeakTime = decoder->signalClock;
                  modulation->searchEndTime = decoder->signalClock + bitrate->period2SymbolSamples;
               }

               // wait until search finished
               if (decoder->signalClock != modulation->searchEndTime)
                  break;

               // if no edge found, restart search
               if (!modulation->searchPeakTime)
               {
                  // if no edge is found, we restart SOF search
                  modulation->searchStage = SOF_BEGIN;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->searchPeakTime = 0;
                  modulation->searchPulseWidth = 0;
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;
                  break;
               }

#ifdef DEBUG_BPSK_SYNC_CHANNEL
               decoder->debug->set(DEBUG_BPSK_SYNC_CHANNEL, 0.75);
#endif
               // if edge found, synchronize symbol and check for end of SOF
               modulation->searchStage = SOF_END;
               modulation->searchStartTime = modulation->searchPeakTime + (2 * bitrate->period1SymbolSamples) - bitrate->period2SymbolSamples; // search falling edge up to 11 etu
               modulation->searchEndTime = modulation->searchPeakTime + (3 * bitrate->period1SymbolSamples) + bitrate->period2SymbolSamples; // search falling edge up to 11 etu
               modulation->searchPeakTime = 0;

               break;

            case SOF_END:

#ifdef DEBUG_BPSK_PHASE_CHANNEL
               decoder->debug->set(DEBUG_BPSK_PHASE_CHANNEL, modulation->phaseIntegrate);
#endif
               // wait until search start
               if (decoder->signalClock < modulation->searchStartTime)
                  break;

               // detect start bit zero-cross
               if (modulation->phaseIntegrate > 0.001f)
               {
                  modulation->searchPeakTime = decoder->signalClock;
                  modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
               }

               // wait until search finished
               if (decoder->signalClock != modulation->searchEndTime)
                  break;

               // if no modulation found, restart search
               if (!modulation->searchPeakTime)
               {
                  // if no edge is found, we restart SOF search
                  modulation->searchStage = SOF_BEGIN;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->searchPeakTime = 0;
                  modulation->searchPulseWidth = 0;
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;
                  break;
               }

               // if found, set SOF symbol end and reference phase
               modulation->symbolEndTime = modulation->searchPeakTime;
               modulation->symbolPhase = modulation->phaseIntegrate;
               modulation->phaseThreshold = std::fabs(modulation->phaseIntegrate / 3);

               // reset modulation to continue search
               modulation->searchStage = SOF_BEGIN;
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;

               // set reference symbol info
               symbolStatus.value = 1;
               symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
               symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
               symbolStatus.length = symbolStatus.end - symbolStatus.start;

               pattern = PatternType::PatternS;
         }

         if (pattern != PatternType::Invalid)
            break;
      }

      // reset search status
      if (pattern != PatternType::Invalid)
      {
         symbolStatus.pattern = pattern;

         modulation->searchStartTime = 0;
         modulation->searchEndTime = 0;
         modulation->symbolSyncTime = 0;
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
         float phase = (signalData - modulation->symbolAverage) * (delay1Data - modulation->symbolAverage) * 10;

         // store signal phase in filter buffer
         modulation->integrationData[modulation->signalIndex & (BUFFER_SIZE - 1)] = phase;

         modulation->phaseIntegrate += modulation->integrationData[modulation->signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->phaseIntegrate -= modulation->integrationData[modulation->delay4Index & (BUFFER_SIZE - 1)]; // remove delayed value

#ifdef DEBUG_BPSK_PHASE_CHANNEL
         decoder->debug->set(DEBUG_BPSK_PHASE_CHANNEL, modulation->phaseIntegrate);
#endif
         // edge detector for re-synchronization
         if ((modulation->phaseIntegrate > 0 && modulation->symbolPhase < 0) || (modulation->phaseIntegrate < 0 && modulation->symbolPhase > 0))
         {
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

         // wait until sync time is reached
         if (decoder->signalClock != modulation->symbolSyncTime)
            continue;

#ifdef DEBUG_BPSK_SYNC_CHANNEL
         decoder->debug->set(DEBUG_BPSK_SYNC_CHANNEL, 0.5);
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

      // reset search status
      if (pattern != PatternType::Invalid)
      {
         symbolStatus.pattern = pattern;

         modulation->searchStartTime = 0;
         modulation->searchEndTime = 0;
         modulation->symbolSyncTime = 0;
      }

      return pattern;
   }

   /*
    * Reset modulation status
    */
   inline void resetModulation()
   {
      // reset modulation detection for all rates
      for (int rate = r106k; rate <= r424k; rate++)
      {
         modulationStatus[rate].searchStage = 0;
         modulationStatus[rate].searchStartTime = 0;
         modulationStatus[rate].searchEndTime = 0;
         modulationStatus[rate].searchDeepValue = 0;
         modulationStatus[rate].symbolAverage = 0;
         modulationStatus[rate].symbolPhase = NAN;
         modulationStatus[rate].detectorPeek = 0;
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
         if (processREQB(frame))
            break;

         if (processATTRIB(frame))
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
   inline bool processREQB(NfcFrame &frame)
   {
      if (frame.isPollFrame())
      {
         if (frame[0] == CommandType::NFCB_REQB && frame.limit() == 5)
         {
            frameStatus.lastCommand = frame[0];

            // This commands starts or wakeup card communication, so reset the protocol parameters to the default values
            protocolStatus.maxFrameSize = 256;
            protocolStatus.startUpGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCB_SFGT_DEF);
            protocolStatus.frameGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCB_FGT_DEF);
            protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFCB_FWT_DEF);
            protocolStatus.requestGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCB_RGT_DEF);

            // The REQ-A Response must start between this range
            frameStatus.frameGuardTime = decoder->signalParams.sampleTimeUnit * NFC_TR0_MIN; // ATQ-B response guard
            frameStatus.frameWaitingTime = decoder->signalParams.sampleTimeUnit * NFCB_FWT_ATQB; // ATQ-B response timeout

            // clear chained flags
            chainedFlags = 0;

            // set frame flags
            frame.setFramePhase(FramePhase::SelectionFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      if (frame.isListenFrame())
      {
         if (frameStatus.lastCommand == CommandType::NFCB_REQB)
         {
            int fdsi = (frame[10] >> 4) & 0x0f;
            int fwi = (frame[11] >> 4) & 0x0f;

            // This commands update protocol parameters
            protocolStatus.maxFrameSize = NFC_FDS_TABLE[fdsi];
            protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFC_FWT_TABLE[fwi]);

            frame.setFramePhase(FramePhase::SelectionFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            log.info("ATQB protocol timing parameters");
            log.info("  maxFrameSize {} bytes", {protocolStatus.maxFrameSize});
            log.info("  frameWaitingTime {} samples ({} us)", {protocolStatus.frameWaitingTime, 1E6 * protocolStatus.frameWaitingTime / decoder->sampleRate});

            return true;
         }
      }

      return false;
   }

   /*
    * Process ATTRIB frame
    * The ATTRIB Command sent by the PCD shall include information required to select a single PICC
    */
   inline bool processATTRIB(NfcFrame &frame)
   {
      if (frame.isPollFrame())
      {
         if (frame[0] == CommandType::NFCB_ATTRIB && frame.limit() > 10)
         {
            frameStatus.lastCommand = frame[0];

            int param1 = frame[5];
            int param2 = frame[6];

            int tr0i = (param1 >> 6) & 0x3;
            int fdsi = param2 & 0xf;

            protocolStatus.maxFrameSize = NFC_FDS_TABLE[fdsi];

            if (!tr0i)
               protocolStatus.frameGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCB_FGT_DEF); // default value
            else
               protocolStatus.frameGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCB_TR0_MIN_TABLE[tr0i]);

            // sets the activation frame waiting time for ATTRIB response
            frameStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFC_FWT_ACTIVATION);

            // clear chained flags
            chainedFlags = 0;

            // set frame flags
            frame.setFramePhase(FramePhase::SelectionFrame);
            frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

            return true;
         }
      }

      if (frame.isListenFrame())
      {
         if (frameStatus.lastCommand == CommandType::NFCB_ATTRIB)
         {
            frame.setFramePhase(FramePhase::SelectionFrame);

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
    * Check NFC-B crc
    */
   static inline bool checkCrc(NfcFrame &frame)
   {
      unsigned short crc = 0xFFFF; // NFC-B ISO/IEC 13239
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

      crc = ~crc;

      res |= ((unsigned int) frame[length - 2] & 0xff);
      res |= ((unsigned int) frame[length - 1] & 0xff) << 8;

      return res == crc;
   }
};

NfcB::NfcB(DecoderStatus *decoder) : self(new Impl(decoder))
{
}

NfcB::~NfcB()
{
   delete self;
}

void NfcB::setModulationThreshold(float min, float max)
{
   if (!std::isnan(min))
      self->minimumModulationThreshold = min;

   if (!std::isnan(max))
      self->maximumModulationThreshold = max;
}

void NfcB::configure(long sampleRate)
{
   self->configure(sampleRate);
}

bool NfcB::detect()
{
   return self->detectModulation();
}

void NfcB::decode(sdr::SignalBuffer &samples, std::list<NfcFrame> &frames)
{
   self->decodeFrame(samples, frames);
}

}

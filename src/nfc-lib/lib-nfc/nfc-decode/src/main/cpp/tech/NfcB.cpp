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

#define LISTEN_MODE_TR1 0
#define LISTEN_MODE_SOS_S1 1
#define LISTEN_MODE_SOS_S2 2

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

/*
 * status for protocol
 */
struct ProtocolStatus
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

   // Synchronization time between the start of the PICC subcarrier generation and the start of the PICC subcarrier modulation
   unsigned int tr1MinimumTime;
   unsigned int tr1MaximumTime;

   // Start of Sequence first modulation
   unsigned int listenS1MinimumTime;
   unsigned int listenS1MaximumTime;

   // Start of Sequence first modulation
   unsigned int listenS2MinimumTime;
   unsigned int listenS2MaximumTime;
};


struct NfcB::Impl : NfcTech
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
   float minimumModulationDeep = 0.10f;

   // minimum modulation threshold to detect valid signal for NFC-B (default 75%)
   float maximumModulationDeep = 0.90f;

   // minimum correlation threshold to detect valid NFC-V pulse (default 50%)
   float minimumCorrelationThreshold = 0.50f;

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
      log.info("\tcorrelationThreshold {}", {minimumCorrelationThreshold});
      log.info("\tmodulationThreshold  {} -> {}", {minimumModulationDeep, maximumModulationDeep});

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
         bitrate->symbolDelayDetect = rate > r106k ? bitrateParams[rate - 1].symbolDelayDetect + bitrateParams[rate - 1].period1SymbolSamples : 0;

         // moving average offsets
         bitrate->offsetFutureIndex = BUFFER_SIZE;
         bitrate->offsetSignalIndex = BUFFER_SIZE - bitrate->symbolDelayDetect;
         bitrate->offsetDelay0Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period0SymbolSamples;
         bitrate->offsetDelay1Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period1SymbolSamples;
         bitrate->offsetDelay2Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period2SymbolSamples;
         bitrate->offsetDelay4Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period4SymbolSamples;
         bitrate->offsetDelay8Index = BUFFER_SIZE - bitrate->symbolDelayDetect - bitrate->period8SymbolSamples;

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

      // initialize NFC-B protocol specific parameters
      protocolStatus.maxFrameSize = 256;
      protocolStatus.startUpGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCB_SFGT_DEF);
      protocolStatus.frameGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCB_FGT_DEF);
      protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFCB_FWT_DEF);
      protocolStatus.requestGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCB_RGT_DEF);
      protocolStatus.tr1MinimumTime = int(decoder->signalParams.sampleTimeUnit * NFCB_TR1_MIN);
      protocolStatus.tr1MaximumTime = int(decoder->signalParams.sampleTimeUnit * NFCB_TR1_MAX);
      protocolStatus.listenS1MinimumTime = int(decoder->signalParams.sampleTimeUnit * NFCB_TLISTEN_S1_MIN);
      protocolStatus.listenS1MaximumTime = int(decoder->signalParams.sampleTimeUnit * NFCB_TLISTEN_S1_MAX);
      protocolStatus.listenS2MinimumTime = int(decoder->signalParams.sampleTimeUnit * NFCB_TLISTEN_S2_MIN);
      protocolStatus.listenS2MaximumTime = int(decoder->signalParams.sampleTimeUnit * NFCB_TLISTEN_S2_MAX);

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
      log.info("\ttr1MinimumTime {} samples ({} us)", {protocolStatus.tr1MinimumTime, 1000000.0 * protocolStatus.tr1MinimumTime / decoder->sampleRate});
      log.info("\ttr1MaximumTime {} samples ({} us)", {protocolStatus.tr1MaximumTime, 1000000.0 * protocolStatus.tr1MaximumTime / decoder->sampleRate});
      log.info("\tlistenS1MinimumTime {} samples ({} us)", {protocolStatus.listenS1MinimumTime, 1000000.0 * protocolStatus.listenS1MinimumTime / decoder->sampleRate});
      log.info("\tlistenS1MaximumTime {} samples ({} us)", {protocolStatus.listenS1MaximumTime, 1000000.0 * protocolStatus.listenS1MaximumTime / decoder->sampleRate});
      log.info("\tlistenS2MinimumTime {} samples ({} us)", {protocolStatus.listenS2MinimumTime, 1000000.0 * protocolStatus.listenS2MinimumTime / decoder->sampleRate});
      log.info("\tlistenS2MaximumTime {} samples ({} us)", {protocolStatus.listenS2MaximumTime, 1000000.0 * protocolStatus.listenS2MaximumTime / decoder->sampleRate});
   }

   /*
    * Detect NFC-B modulation
    */
   inline bool detectModulation()
   {
      // ignore low power signals
      if (decoder->signalAverage < decoder->powerLevelThreshold)
         return false;

      // POLL frame ASK detector for  106Kbps, 212Kbps and 424Kbps
      for (int rate = r106k; rate <= r424k; rate++)
      {
         BitrateParams *bitrate = bitrateParams + rate;
         ModulationStatus *modulation = modulationStatus + rate;

         // compute signal pointer
         unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);

         // get signal samples
         float signalEdge = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].filtered;
         float signalDeep = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].deep;

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL, signalEdge * 10);
#endif
         // reset modulation if exceed limits
         if (signalDeep > maximumModulationDeep)
         {
            modulation->symbolStartTime = 0;
            modulation->symbolEndTime = 0;
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->detectorPeakTime = 0;
            modulation->detectorPeakValue = 0;

            return false;
         }

         // no modulation detected, search SoF begin
         if (!modulation->symbolStartTime)
         {
            // first modulation must be over minimum modulation deep
            modulation->searchValueThreshold = decoder->signalAverage * minimumModulationDeep;

            // detect edge at maximum peak
            if (signalEdge < -modulation->searchValueThreshold && modulation->detectorPeakValue > signalEdge)
            {
               modulation->detectorPeakValue = signalEdge;
               modulation->detectorPeakTime = decoder->signalClock;
               modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
            }

            // wait until search finished
            if (decoder->signalClock != modulation->searchEndTime)
               continue;

            // sets SOF symbol frame start (also frame start)
            modulation->symbolStartTime = modulation->detectorPeakTime - bitrate->period8SymbolSamples;

            // set search window for next edge
            modulation->searchStartTime = modulation->symbolStartTime + (10 * bitrate->period1SymbolSamples) - bitrate->period2SymbolSamples; // search falling edge from 10 etu
            modulation->searchEndTime = modulation->symbolStartTime + (11 * bitrate->period1SymbolSamples) + bitrate->period2SymbolSamples; // search falling edge up to 11 etu
            modulation->searchValueThreshold = std::fabs(modulation->detectorPeakValue) / 2; // search peak threshold for next edge
            modulation->detectorPeakValue = 0;
            modulation->detectorPeakTime = 0;

            continue;
         }

         // first edge found, now wait for a sequence of 10 zeros
         if (!modulation->symbolEndTime)
         {
            // during 10-11 ETU there must not be modulation changes
            if (decoder->signalClock < modulation->searchStartTime)
            {
               if (signalEdge > modulation->searchValueThreshold)
               {
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->detectorPeakTime = 0;
                  modulation->detectorPeakValue = 0;
               }

               continue;
            }

            // detect edge at maximum peak
            if (signalEdge > modulation->searchValueThreshold && modulation->detectorPeakValue < signalEdge)
            {
               modulation->detectorPeakValue = signalEdge;
               modulation->detectorPeakTime = decoder->signalClock;
               modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
            }

            // wait until search finished
            if (decoder->signalClock != modulation->searchEndTime)
               continue;

            // if no edge detected reset SOF search
            if (!modulation->detectorPeakTime)
            {
               // if no edge found, restart search
               modulation->symbolStartTime = 0;
               modulation->symbolEndTime = 0;
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;
               modulation->detectorPeakValue = 0;
               continue;
            }

            // set symbol end
            modulation->symbolEndTime = modulation->detectorPeakTime;

            // trigger last search stage
            modulation->searchStartTime = modulation->detectorPeakTime + (2 * bitrate->period1SymbolSamples) - bitrate->period2SymbolSamples; // search falling edge from 2 etu
            modulation->searchEndTime = modulation->detectorPeakTime + (3 * bitrate->period1SymbolSamples) + bitrate->period2SymbolSamples; // search falling edge up to 3 etu
            modulation->searchValueThreshold = std::fabs(modulation->detectorPeakValue) / 2; // search peak threshold for next edge
            modulation->detectorPeakValue = 0;
            modulation->detectorPeakTime = 0;

            continue;
         }

         // during last 2-3 ETU there must not be modulation changes
         if (decoder->signalClock < modulation->searchStartTime)
         {
            if (signalEdge < -modulation->searchValueThreshold)
            {
               modulation->symbolStartTime = 0;
               modulation->symbolEndTime = 0;
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;
               modulation->detectorPeakTime = 0;
               modulation->detectorPeakValue = 0;
            }

            continue;
         }

         // detect edge at maximum peak
         if (signalEdge < -modulation->searchValueThreshold && modulation->detectorPeakValue > signalEdge)
         {
            modulation->detectorPeakValue = signalEdge;
            modulation->detectorPeakTime = decoder->signalClock;
            modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
         }

         // wait until search finished
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // if no edge detected reset modulation search
         if (!modulation->detectorPeakTime)
         {
            modulation->symbolStartTime = 0;
            modulation->symbolEndTime = 0;
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->detectorPeakTime = 0;
            modulation->detectorPeakValue = 0;

            break;
         }

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL, 0.75f);
#endif

         // update SOF symbol end time
         modulation->symbolEndTime = modulation->detectorPeakTime;

         // set synchronization to detect first bit, no need search!
         modulation->searchSyncTime = modulation->symbolEndTime + bitrate->period2SymbolSamples;
         modulation->searchStartTime = 0;
         modulation->searchEndTime = 0;
         modulation->searchValueThreshold = std::fabs(modulation->detectorPeakValue) / 2; // search peak threshold for next edge
         modulation->detectorPeakTime = 0;
         modulation->detectorPeakValue = 0;

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

               NfcFrame request = NfcFrame(TechType::NfcB, FrameType::PollFrame);

               request.setFrameRate(decoder->bitrate->symbolsPerSecond);
               request.setSampleStart(frameStatus.frameStart);
               request.setSampleEnd(frameStatus.frameEnd);
               request.setTimeStart(double(frameStatus.frameStart) / double(decoder->sampleRate));
               request.setTimeEnd(double(frameStatus.frameEnd) / double(decoder->sampleRate));

               if (truncateError || streamError)
                  request.setFrameFlags(FrameFlags::Truncated);

               // add bytes to frame and flip to prepare read
               request.put(streamStatus.buffer, streamStatus.bytes).flip();

               // process frame
               process(request);

               // add to frame list
               frames.push_back(request);

               // clear stream status
               streamStatus = {0,};

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
               }

               // clear stream status
               streamStatus = {0,};

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
                  // set frame end, we add 352 units to compensate EOS (no detected completely)
                  frameStatus.frameEnd = symbolStatus.end + int(decoder->signalParams.sampleTimeUnit * 352);

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
      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

      // compute signal pointer
      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;

         // get signal samples
         float signalEdge = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].filtered;
         float signalDeep = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].deep;

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL, signalEdge * 10);
#endif

         // edge re-synchronization window
         if (decoder->signalClock > modulation->searchStartTime && decoder->signalClock < modulation->searchEndTime)
         {
            // ignore edge type
            signalEdge = std::abs(signalEdge);

            // detect edge at maximum peak
            if (signalEdge > modulation->searchValueThreshold && modulation->detectorPeakValue < signalEdge)
            {
               modulation->detectorPeakValue = signalEdge;
               modulation->searchSyncTime = decoder->signalClock + bitrate->period2SymbolSamples;
            }
         }

         // wait until sync time is reached
         if (decoder->signalClock != modulation->searchSyncTime)
            continue;

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL, 0.50f);
#endif

         // set symbol timings
         modulation->symbolStartTime = modulation->symbolEndTime;
         modulation->symbolEndTime = modulation->searchSyncTime + bitrate->period2SymbolSamples;

         // update next search for re-synchronization
         modulation->searchStartTime = modulation->searchSyncTime + bitrate->period4SymbolSamples;
         modulation->searchEndTime = modulation->searchStartTime + bitrate->period2SymbolSamples;
         modulation->searchSyncTime = modulation->searchSyncTime + bitrate->period1SymbolSamples;

         // reset status for next symbol
         modulation->detectorPeakValue = 0;

         // modulated signal, symbol L -> 0 value
         if (signalDeep > minimumModulationDeep)
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

         return symbolStatus.pattern;
      }

      return PatternType::Invalid;
   }

   /*
    * Decode SOF BPSK modulated listen frame symbol
    */
   inline int decodeListenFrameStartBpsk(sdr::SignalBuffer &buffer)
   {
      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

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
         float signalData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].filtered;
         float delay1Data = decoder->sample[delay1Index & (BUFFER_SIZE - 1)].filtered;
         float signalDeep = decoder->sample[futureIndex & (BUFFER_SIZE - 1)].deep;

         // multiply 1 symbol delayed signal with incoming signal, (magic number 10 must be signal dependent, but i don't how...)
         modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)] = signalData * delay1Data * 10;

         // compute phase integration
         modulation->phaseIntegrate += modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->phaseIntegrate -= modulation->integrationData[delay4Index & (BUFFER_SIZE - 1)]; // remove delayed value

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 0, modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]);
#endif

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 1, modulation->phaseIntegrate);
#endif

#ifdef DEBUG_NFC_CHANNEL
         if (decoder->signalClock == frameStatus.waitingEnd)
            decoder->debug->set(DEBUG_NFC_CHANNEL + 1, -0.75f);
#endif

         // wait until frame guard time (TR0)
         if (decoder->signalClock < frameStatus.guardEnd)
            continue;

         // using signal st.dev as lower level threshold scaled to 1/8 symbol to compensate integration
         if (decoder->signalClock == frameStatus.guardEnd)
            modulation->searchValueThreshold = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].variance;

         // check if frame waiting time exceeded without detect modulation
         if (decoder->signalClock > frameStatus.waitingEnd)
            return PatternType::NoPattern;

         // check if poll frame modulation is detected while waiting for response
         if (signalDeep > maximumModulationDeep)
            return PatternType::NoPattern;

#ifdef DEBUG_NFC_CHANNEL
         if (decoder->signalClock < (frameStatus.guardEnd + 10))
            decoder->debug->set(DEBUG_NFC_CHANNEL + 1, modulation->searchValueThreshold);
#endif

#ifdef DEBUG_NFC_CHANNEL
         if (decoder->signalClock == frameStatus.guardEnd)
            decoder->debug->set(DEBUG_NFC_CHANNEL + 1, 0.50f);
#endif

         // wait util search start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         // search on positive phase
         if (modulation->phaseIntegrate > modulation->searchValueThreshold)
         {
            if (!modulation->symbolStartTime)
               modulation->symbolStartTime = decoder->signalClock;

            modulation->searchEndTime = decoder->signalClock + bitrate->period2SymbolSamples;
         }

         // wait util phase changes or search completed
         if (decoder->signalClock != modulation->searchEndTime && modulation->phaseIntegrate > 0)
            continue;

         // search for Start Of Frame pattern (SoF)
         switch (modulation->searchModeState)
         {
            case LISTEN_MODE_TR1:
            {
               // detect preamble in range NFCB_TR1_MIN to NFCB_TR1_MAX
               int preambleSyncLength = decoder->signalClock - modulation->symbolStartTime;

               if (preambleSyncLength < protocolStatus.tr1MinimumTime || // preamble too short
                   preambleSyncLength > protocolStatus.tr1MaximumTime) // preamble too long
               {
                  modulation->searchModeState = LISTEN_MODE_TR1;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;
                  continue;
               }

#ifdef DEBUG_NFC_CHANNEL
               decoder->debug->set(DEBUG_NFC_CHANNEL + 1, 0.75f);
#endif
               // set symbol end time
               modulation->symbolEndTime = decoder->signalClock;

               // continue searching T-LISTEN S1
               modulation->searchModeState = LISTEN_MODE_SOS_S1;
               modulation->searchStartTime = decoder->signalClock + bitrate->period1SymbolSamples + bitrate->period4SymbolSamples;
               modulation->searchEndTime = 0;

               continue;
            }

            case LISTEN_MODE_SOS_S1:
            {
               // detect T-LISTEN S1 period in range NFCB_TLISTEN_S1_MIN to NFCB_TLISTEN_S1_MAX
               int listenS1Length = decoder->signalClock - modulation->symbolEndTime;

               if (listenS1Length < protocolStatus.listenS1MinimumTime || // preamble too short
                   listenS1Length > protocolStatus.listenS1MaximumTime) // preamble too long
               {
                  modulation->searchModeState = LISTEN_MODE_TR1;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;
                  continue;
               }

#ifdef DEBUG_NFC_CHANNEL
               decoder->debug->set(DEBUG_NFC_CHANNEL + 1, 0.75f);
#endif

               // update symbol end time
               modulation->symbolEndTime = decoder->signalClock;

               // now search T-LISTEN S2
               modulation->searchModeState = LISTEN_MODE_SOS_S2;
               modulation->searchStartTime = decoder->signalClock + bitrate->period1SymbolSamples + bitrate->period4SymbolSamples;
               modulation->searchEndTime = 0;

               continue;
            }

            case LISTEN_MODE_SOS_S2:
            {
               // detect T-LISTEN S1 period in range NFCB_TLISTEN_SS_MIN to NFCB_TLISTEN_SS_MAX
               int listenS2Length = decoder->signalClock - modulation->symbolEndTime;

               if (listenS2Length < protocolStatus.listenS2MinimumTime || // preamble too short
                   listenS2Length > protocolStatus.listenS2MaximumTime) // preamble too long
               {
                  modulation->searchModeState = LISTEN_MODE_TR1;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;
                  continue;
               }

#ifdef DEBUG_NFC_CHANNEL
               decoder->debug->set(DEBUG_NFC_CHANNEL + 1, 0.75f);
#endif
               // update symbol end time
               modulation->symbolEndTime = decoder->signalClock;

               // set next synchronization point
               modulation->searchSyncTime = decoder->signalClock + bitrate->period2SymbolSamples;
               modulation->searchLastPhase = modulation->phaseIntegrate;
               modulation->searchPhaseThreshold = modulation->detectorPeakValue / 3;
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;

               // clear phase peak detector
               modulation->detectorPeakValue = 0;

               // set reference symbol info
               symbolStatus.value = 1;
               symbolStatus.start = modulation->symbolStartTime - bitrate->period1SymbolSamples - bitrate->symbolDelayDetect;
               symbolStatus.end = modulation->symbolEndTime - bitrate->period1SymbolSamples - bitrate->symbolDelayDetect;
               symbolStatus.length = symbolStatus.end - symbolStatus.start;
               symbolStatus.pattern = PatternType::PatternS;

               return symbolStatus.pattern;
            }
         }
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
         float signalData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].filtered;
         float delay1Data = decoder->sample[delay1Index & (BUFFER_SIZE - 1)].filtered;

         // multiply 1 symbol delayed signal with incoming signal, (magic number 10 must be signal dependent, but i don't how...)
         modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)] = signalData * delay1Data * 10;

         // compute phase integration
         modulation->phaseIntegrate += modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->phaseIntegrate -= modulation->integrationData[delay4Index & (BUFFER_SIZE - 1)]; // remove delayed value

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 0, modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]);
#endif

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 1, modulation->phaseIntegrate);
#endif

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 2, modulation->searchValueThreshold);
#endif

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

         // wait until sync time is reached
         if (decoder->signalClock != modulation->searchSyncTime)
            continue;

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 1, 0.50f);
#endif

         // no modulation detected, generate End Of Frame
         if (std::abs(modulation->phaseIntegrate) < std::abs(modulation->searchPhaseThreshold))
            return PatternType::PatternO;

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
            symbolStatus.pattern = (symbolStatus.pattern == PatternType::PatternM) ? PatternType::PatternN : PatternType::PatternM;
         }
            // update threshold for next symbol
         else
         {
            modulation->searchPhaseThreshold = modulation->phaseIntegrate / 3;
         }

         // setup symbol info
         symbolStatus.start = modulation->symbolStartTime - bitrate->period1SymbolSamples - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->period1SymbolSamples - bitrate->symbolDelayDetect;
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
      for (int rate = r106k; rate <= r424k; rate++)
      {
         modulationStatus[rate] = {0,};
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
//         frameStatus.tr1MinimumTime = protocolStatus.tr1MinimumTime;
//         frameStatus.tr1MaximumTime = protocolStatus.tr1MaximumTime;
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
            frameStatus.frameGuardTime = decoder->signalParams.sampleTimeUnit * NFCB_TR0_MIN; // ATQ-B response guard
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
    * Check NFC-B crc NFC-B ISO/IEC 13239
    */
   inline bool checkCrc(NfcFrame &frame)
   {
      int size = frame.limit();

      if (size < 3)
         return false;

      unsigned short crc = ~crc16(frame, 0, size - 2, 0xFFFF, true);
      unsigned short res = ((unsigned int) frame[size - 2] & 0xff) | ((unsigned int) frame[size - 1] & 0xff) << 8;

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
      self->minimumModulationDeep = min;

   if (!std::isnan(max))
      self->maximumModulationDeep = max;
}

void NfcB::setCorrelationThreshold(float value)
{
   if (!std::isnan(value))
      self->minimumCorrelationThreshold = value;
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

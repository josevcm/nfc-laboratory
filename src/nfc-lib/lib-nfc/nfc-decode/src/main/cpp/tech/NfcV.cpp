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

#include <cstring>

#include <tech/NfcV.h>

#define LISTEN_MODE_PREAMBLE1 0
#define LISTEN_MODE_PREAMBLE2 1

namespace nfc {

enum PatternType
{
   Invalid = 0,
   NoPattern = 1,
   Pattern0 = 2, // data 0
   Pattern1 = 3, // data 1
   Pattern2 = 4, // pulse pattern for 2 bit code
   Pattern8 = 5, // pulse pattern for 8 bit code
   PatternS = 6, // frame start / end pattern
   PatternE = 7, // frame error pattern
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

   // Length of first SOF preamble subcarrier burst
   unsigned int sofS1MinimumTime;
   unsigned int sofS1MaximumTime;

   // Length of second SOF preamble subcarrier burst
   unsigned int sofS2MinimumTime;
   unsigned int sofS2MaximumTime;
};

struct NfcV::Impl : NfcTech
{
   rt::Logger log {"NfcV"};

   DecoderStatus *decoder;

   // pulse position parameters
   PulseParams pulseParams[2] {0,};

   // bitrate parameters
   BitrateParams bitrateParams {0,};

   // detected symbol status
   SymbolStatus symbolStatus {0,};

   // bit stream status
   StreamStatus streamStatus {0,};

   // frame processing status
   FrameStatus frameStatus {0,};

   // protocol processing status
   ProtocolStatus protocolStatus {0,};

   // modulation status for each bitrate
   ModulationStatus modulationStatus {0,};

   // minimum modulation threshold to detect valid signal for NFC-V (default 95%)
   float minimumModulationDeep = 0.90f;

   // minimum modulation threshold to detect valid signal for NFC-V (default 100%)
   float maximumModulationDeep = 1.00f;

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
      log.info("initializing NFC-V decoder");
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

      // clear modulation parameters
      modulationStatus = {0,};

      // clear bitrate parameters
      bitrateParams = {0,};

      // set tech type and rate
      bitrateParams.techType = TechType::NfcV;

      // NFC-V has constant symbol rate
      bitrateParams.symbolsPerSecond = int(std::round(NFC_FC / 256));

      // number of samples per symbol
      bitrateParams.period0SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * 512)); // double full symbol samples
      bitrateParams.period1SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * 256)); // full symbol samples
      bitrateParams.period2SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * 128)); // half symbol samples
      bitrateParams.period4SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * 64)); // quarter of symbol...
      bitrateParams.period8SymbolSamples = int(std::round(decoder->signalParams.sampleTimeUnit * 32)); // and so on...

      // delay guard for each symbol rate
      bitrateParams.symbolDelayDetect = bitrateParams.period0SymbolSamples;

      // moving average offsets
      bitrateParams.offsetFutureIndex = BUFFER_SIZE;
      bitrateParams.offsetSignalIndex = BUFFER_SIZE - bitrateParams.symbolDelayDetect;
      bitrateParams.offsetDelay0Index = BUFFER_SIZE - bitrateParams.symbolDelayDetect - bitrateParams.period0SymbolSamples;
      bitrateParams.offsetDelay1Index = BUFFER_SIZE - bitrateParams.symbolDelayDetect - bitrateParams.period1SymbolSamples;
      bitrateParams.offsetDelay2Index = BUFFER_SIZE - bitrateParams.symbolDelayDetect - bitrateParams.period2SymbolSamples;
      bitrateParams.offsetDelay4Index = BUFFER_SIZE - bitrateParams.symbolDelayDetect - bitrateParams.period4SymbolSamples;
      bitrateParams.offsetDelay8Index = BUFFER_SIZE - bitrateParams.symbolDelayDetect - bitrateParams.period8SymbolSamples;

      log.info("{} kpbs parameters:", {round(bitrateParams.symbolsPerSecond / 1E3)});
      log.info("\tsymbolsPerSecond     {}", {bitrateParams.symbolsPerSecond});
      log.info("\tperiod0SymbolSamples {} ({} us)", {bitrateParams.period0SymbolSamples, 1E6 * bitrateParams.period0SymbolSamples / decoder->sampleRate});
      log.info("\tperiod1SymbolSamples {} ({} us)", {bitrateParams.period1SymbolSamples, 1E6 * bitrateParams.period1SymbolSamples / decoder->sampleRate});
      log.info("\tperiod2SymbolSamples {} ({} us)", {bitrateParams.period2SymbolSamples, 1E6 * bitrateParams.period2SymbolSamples / decoder->sampleRate});
      log.info("\tperiod4SymbolSamples {} ({} us)", {bitrateParams.period4SymbolSamples, 1E6 * bitrateParams.period4SymbolSamples / decoder->sampleRate});
      log.info("\tperiod8SymbolSamples {} ({} us)", {bitrateParams.period8SymbolSamples, 1E6 * bitrateParams.period8SymbolSamples / decoder->sampleRate});
      log.info("\toffsetInsertIndex    {}", {bitrateParams.offsetFutureIndex});
      log.info("\toffsetSignalIndex    {}", {bitrateParams.offsetSignalIndex});
      log.info("\toffsetDelay8Index    {}", {bitrateParams.offsetDelay8Index});
      log.info("\toffsetDelay4Index    {}", {bitrateParams.offsetDelay4Index});
      log.info("\toffsetDelay2Index    {}", {bitrateParams.offsetDelay2Index});
      log.info("\toffsetDelay1Index    {}", {bitrateParams.offsetDelay1Index});
      log.info("\toffsetDelay0Index    {}", {bitrateParams.offsetDelay0Index});

      // initialize pulse parameters for 1 of 4 code
      configurePulse(pulseParams + 0, 2);

      // initialize pulse parameters for 1 of 256 code
      configurePulse(pulseParams + 1, 8);

      // initialize default protocol parameters for start decoding
      protocolStatus.maxFrameSize = 256;
      protocolStatus.startUpGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCV_SFGT_DEF);
      protocolStatus.frameGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCV_FGT_DEF);
      protocolStatus.frameWaitingTime = int(decoder->signalParams.sampleTimeUnit * NFCV_FWT_DEF);
      protocolStatus.requestGuardTime = int(decoder->signalParams.sampleTimeUnit * NFCV_RGT_DEF);
      protocolStatus.sofS1MinimumTime = int(decoder->signalParams.sampleTimeUnit * (NFCV_TLISTEN_S1 - 32)); // 24 pulses of fc/32
      protocolStatus.sofS1MaximumTime = int(decoder->signalParams.sampleTimeUnit * (NFCV_TLISTEN_S1 + 32)); // 24 pulses of fc/32
      protocolStatus.sofS2MinimumTime = int(decoder->signalParams.sampleTimeUnit * (NFCV_TLISTEN_S2 - 32)); // 8 pulses of fc/32
      protocolStatus.sofS2MaximumTime = int(decoder->signalParams.sampleTimeUnit * (NFCV_TLISTEN_S2 + 32)); // 8 pulses of fc/32

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

   inline void configurePulse(PulseParams *pulse, int bits)
   {
      pulse->bits = bits;
      pulse->periods = 1 << bits;
      pulse->length = int(round(pulse->periods * decoder->signalParams.sampleTimeUnit * 256));

      for (int i = 0; i < pulse->periods; i++)
      {
         pulse->slots[i] = {
               .start = int(round(i * decoder->signalParams.sampleTimeUnit * 256)),
               .end = int(round((i + 1) * decoder->signalParams.sampleTimeUnit * 256)),
               .value = i};
      }
   }

   inline bool detectModulation()
   {
      // wait until has enough data in buffer
      if (decoder->signalClock < BUFFER_SIZE)
         return false;

      // ignore low power signals
      if (decoder->signalAverage < decoder->powerLevelThreshold)
         return false;

      BitrateParams *bitrate = &bitrateParams;
      ModulationStatus *modulation = &modulationStatus;

      // minimum correlation value for start detecting NFC-V symbols
      float minimumCorrelationValue = decoder->signalAverage * minimumCorrelationThreshold;

      // compute signal pointers
      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);
      unsigned int delay8Index = (bitrate->offsetDelay8Index + decoder->signalClock);

      // correlation points
      unsigned int filterPoint1 = (signalIndex % bitrate->period1SymbolSamples);
      unsigned int filterPoint2 = (signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;

      // get signal samples
      float signalData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].samplingValue;
      float delay2Data = decoder->sample[delay2Index & (BUFFER_SIZE - 1)].samplingValue;
      float signalDeep = decoder->sample[delay8Index & (BUFFER_SIZE - 1)].modulateDepth;

      // integrate signal data over 1/2 symbol
      modulation->filterIntegrate += signalData; // add new value
      modulation->filterIntegrate -= delay2Data; // remove delayed value

      // store integrated signal in correlation buffer
      modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

      // compute correlation factor
      float correlatedS0 = (modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint1]) / float(bitrate->period2SymbolSamples);

#ifdef DEBUG_NFC_CHANNEL
      decoder->debug->set(DEBUG_NFC_CHANNEL + 0, modulation->filterIntegrate / float(bitrate->period2SymbolSamples));
#endif

#ifdef DEBUG_NFC_CHANNEL
      decoder->debug->set(DEBUG_NFC_CHANNEL + 1, correlatedS0);
#endif

#ifdef DEBUG_NFC_CHANNEL
      if (decoder->signalClock == modulation->searchSyncTime)
         decoder->debug->set(DEBUG_NFC_CHANNEL + 1, 0.75f);
#endif

      // recover status from previous partial search
      if (modulation->correlatedPeakTime && decoder->signalClock > modulation->correlatedPeakTime + bitrate->period0SymbolSamples)
      {
         modulation->symbolStartTime = 0;
         modulation->symbolEndTime = 0;
         modulation->searchStartTime = 0;
         modulation->searchEndTime = 0;
         modulation->detectorPeakTime = 0;
         modulation->detectorPeakValue = 0;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;
      }

      // wait until search start
      if (decoder->signalClock < modulation->searchStartTime)
         return false;

      // max correlation detector
      if (correlatedS0 > minimumCorrelationValue)
      {
         // detect maximum correlation point
         if (correlatedS0 > modulation->correlatedPeakValue)
         {
            modulation->correlatedPeakValue = correlatedS0;
            modulation->correlatedPeakTime = decoder->signalClock;
            modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
         }

         // detect maximum modulation deep
         if (signalDeep > modulation->detectorPeakValue)
         {
            modulation->detectorPeakValue = signalDeep;
            modulation->detectorPeakTime = decoder->signalClock;
         }
      }

      // wait until search finished
      if (decoder->signalClock != modulation->searchEndTime)
         return false;

      // check for valid NFC-V modulated pulse
      if (signalData < minimumCorrelationValue || // pulse must be ended (in high state)
          modulation->correlatedPeakTime == 0 || // no modulation found
          modulation->detectorPeakValue < minimumModulationDeep) // insufficient modulation deep
      {
         // reset modulation to continue search
         modulation->symbolStartTime = 0;
         modulation->symbolEndTime = 0;
         modulation->searchStartTime = 0;
         modulation->searchEndTime = 0;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;
         modulation->detectorPeakTime = 0;
         modulation->detectorPeakValue = 0;
         return false;
      }

      // first pulse marks symbol begin
      if (!modulation->symbolStartTime)
      {
         // sets SOF symbol frame start (also frame start)
         modulation->symbolStartTime = modulation->correlatedPeakTime - bitrate->period2SymbolSamples;

         // and trigger next stage
         modulation->searchStartTime = modulation->symbolStartTime + (2 * bitrate->period1SymbolSamples);
         modulation->searchEndTime = modulation->symbolStartTime + (4 * bitrate->period1SymbolSamples);

         // reset correlation status
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;
         modulation->detectorPeakTime = 0;
         modulation->detectorPeakValue = 0;
      }

         // second pulse marks modulation encoding
      else
      {
         // check for 1 of 4 code
         if (modulation->correlatedPeakTime > (modulation->symbolStartTime + 3 * bitrate->period1SymbolSamples - bitrate->period8SymbolSamples) &&
             modulation->correlatedPeakTime < (modulation->symbolStartTime + 3 * bitrate->period1SymbolSamples + bitrate->period8SymbolSamples))
         {
            // set SOF symbol parameters
            modulation->symbolEndTime = modulation->correlatedPeakTime + bitrate->period1SymbolSamples;

            // timing search window
            modulation->searchSyncTime = modulation->symbolEndTime;
            modulation->searchStartTime = modulation->searchSyncTime;
            modulation->searchEndTime = modulation->searchSyncTime + pulseParams[0].length;

            // setup bitrate frame info
            frameStatus.symbolRate = bitrate->symbolsPerSecond / 2;

            // modulation detected
            decoder->pulse = pulseParams + 0;
         }

            // check for 1 of 256 code
         else if (modulation->correlatedPeakTime > (modulation->symbolStartTime + 4 * bitrate->period1SymbolSamples - bitrate->period8SymbolSamples) &&
                  modulation->correlatedPeakTime < (modulation->symbolStartTime + 4 * bitrate->period1SymbolSamples + bitrate->period8SymbolSamples))
         {
            // set SOF symbol parameters
            modulation->symbolEndTime = modulation->correlatedPeakTime;

            // timing search window
            modulation->searchSyncTime = modulation->symbolEndTime;
            modulation->searchStartTime = modulation->searchSyncTime;
            modulation->searchEndTime = modulation->searchSyncTime + pulseParams[1].length;

            // setup bitrate frame info
            frameStatus.symbolRate = bitrate->symbolsPerSecond / 32;

            // modulation detected
            decoder->pulse = pulseParams + 1;
         }

            // invalid code detected, reset symbol status
         else
         {
            modulation->symbolStartTime = 0;
            modulation->symbolEndTime = 0;
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->correlatedPeakTime = 0;
            modulation->correlatedPeakValue = 0;
            modulation->detectorPeakTime = 0;
            modulation->detectorPeakValue = 0;
            return false;
         }

         // setup frame info
         frameStatus.frameType = PollFrame;
         frameStatus.frameStart = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         frameStatus.frameEnd = 0;

         // reset modulation for next search
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // set lower threshold to detect valid response pattern
         modulation->searchValueThreshold = minimumCorrelationValue;

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

   inline bool decodePollFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
   {
      int pattern;
      bool frameEnd = false, truncateError = false, streamError = false;

      // decode remaining request frame
      while ((pattern = decodePollFrameSymbolPpm(buffer)) > PatternType::NoPattern)
      {
         // frame ends width pattern S
         if (pattern == PatternType::PatternS)
            frameEnd = true;

            // frame ends if detected stream error
         else if (pattern == PatternType::PatternE)
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
               // add remaining byte to request
               if (streamStatus.bits == 8)
                  streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;

               // set last symbol timing
               frameStatus.frameEnd = symbolStatus.end;

               NfcFrame request = NfcFrame(TechType::NfcV, FrameType::PollFrame);

               request.setFrameRate(frameStatus.symbolRate);
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

                  std::memset(decoder->modulation->integrationData, 0, sizeof(ModulationStatus::integrationData));
                  std::memset(decoder->modulation->correlationData, 0, sizeof(ModulationStatus::correlationData));
               }

               return true;
            }

            // reset modulation and restart frame detection
            resetModulation();

            // no valid frame found
            return false;
         }

         // store full byte in stream buffer
         if (streamStatus.bits == 8)
         {
            streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;
            streamStatus.data = 0;
            streamStatus.bits = 0;
         }

         // decode next bit
         streamStatus.data |= (symbolStatus.value << streamStatus.bits);
         streamStatus.bits += decoder->pulse->bits;
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
            // frame ends with Pattern-S
            if (pattern == PatternType::PatternS)
               frameEnd = true;

               // frame stream error
            else if (pattern == PatternType::PatternE)
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
                  // add remaining byte to response
                  if (streamStatus.bits == 8)
                     streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;

                  frameStatus.frameEnd = symbolStatus.end;

                  // build response frame
                  NfcFrame response = NfcFrame(TechType::NfcV, FrameType::ListenFrame);

                  response.setFrameRate(frameStatus.symbolRate);
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

            // store full byte in stream buffer
            if (streamStatus.bits == 8)
            {
               streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;
               streamStatus.data = 0;
               streamStatus.bits = 0;
            }

            // decode next bit
            streamStatus.data |= (symbolStatus.value << streamStatus.bits);
            streamStatus.bits++;
         }
      }

      return false;
   }

   /*
    * Decode one PPM modulated poll frame symbol
    */
   inline int decodePollFrameSymbolPpm(sdr::SignalBuffer &buffer)
   {
      PulseParams *pulse = decoder->pulse;
      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

      // compute signal pointers
      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      unsigned int delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;
         ++delay2Index;

         // correlation points
         unsigned int filterPoint1 = (signalIndex % bitrate->period1SymbolSamples);
         unsigned int filterPoint2 = (signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;

         // get signal samples
         float currentData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].samplingValue;
         float delayedData = decoder->sample[delay2Index & (BUFFER_SIZE - 1)].samplingValue;

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += currentData; // add new value
         modulation->filterIntegrate -= delayedData; // remove delayed value

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation factor
         float correlatedS0 = (modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint1]) / float(bitrate->period2SymbolSamples);

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 0, modulation->filterIntegrate / float(bitrate->period2SymbolSamples));
#endif

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 1, correlatedS0);
#endif

#ifdef DEBUG_NFC_CHANNEL
         if (decoder->signalClock == modulation->searchSyncTime)
            decoder->debug->set(DEBUG_NFC_CHANNEL + 1, 0.50f);
#endif

         // wait until search finished
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         // max correlation peak detector
         if (correlatedS0 > modulation->searchValueThreshold)
         {
            if (correlatedS0 > modulation->correlatedPeakValue)
            {
               modulation->correlatedPeakValue = correlatedS0;
               modulation->correlatedPeakTime = decoder->signalClock;
               modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
            }
         }

         // wait until search finished
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // detect EOF when modulation occurs in the first part of the second slot
         if (modulation->correlatedPeakTime > (modulation->searchStartTime + 1 * bitrate->period1SymbolSamples + bitrate->period4SymbolSamples) &&
             modulation->correlatedPeakTime < (modulation->searchStartTime + 2 * bitrate->period1SymbolSamples - bitrate->period4SymbolSamples))
         {
#ifdef DEBUG_ASK_SYNC_CHANNEL
            decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.75f);
#endif
            // estimate symbol end from start (peak detection not valid due lack of modulation)
            modulation->symbolEndTime = modulation->correlatedPeakTime + decoder->bitrate->period2SymbolSamples;

            // setup symbol info
            symbolStatus.value = 0;
            symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
            symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
            symbolStatus.length = symbolStatus.end - symbolStatus.start;
            symbolStatus.pattern = PatternType::PatternS;

            return symbolStatus.pattern;
         }

         // by default assume pulse error
         symbolStatus.value = 0;
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;
         symbolStatus.pattern = PatternType::PatternE;

         // search pulse code
         for (int i = 0; i < pulse->periods; i++)
         {
            PulseSlot *slot = pulse->slots + i;

            // pulse position must be in second half, otherwise is protocol error
            if (modulation->correlatedPeakTime > (modulation->searchStartTime + slot->end - bitrate->period4SymbolSamples) &&
                modulation->correlatedPeakTime < (modulation->searchStartTime + slot->end + bitrate->period4SymbolSamples))
            {
               // re-synchronize
               modulation->symbolStartTime = modulation->correlatedPeakTime - slot->end;
               modulation->symbolEndTime = modulation->symbolStartTime + pulse->length;

               // next search
               modulation->searchSyncTime = modulation->symbolEndTime;
               modulation->searchStartTime = modulation->searchSyncTime;
               modulation->searchEndTime = modulation->searchSyncTime + pulse->length;
               modulation->correlatedPeakTime = 0;
               modulation->correlatedPeakValue = 0;

               // setup symbol info
               symbolStatus.value = slot->value;
               symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
               symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
               symbolStatus.length = symbolStatus.end - symbolStatus.start;
               symbolStatus.pattern = pulse->bits == 2 ? PatternType::Pattern2 : PatternType::Pattern8;

               return symbolStatus.pattern;
            }
         }

         return PatternType::PatternE;
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

      // compute pointers
      unsigned int futureIndex = (bitrate->offsetFutureIndex + decoder->signalClock); // index for future signal samples (1 period advance)
      unsigned int signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock); // index for current signal sample
      unsigned int delay1Index = (bitrate->offsetDelay1Index + decoder->signalClock); // index for delayed signal (1/1 period delay)

      while (decoder->nextSample(buffer))
      {
         ++futureIndex;
         ++signalIndex;
         ++delay1Index;

         // compute correlation points
         unsigned int filterPoint1 = (signalIndex % bitrate->period0SymbolSamples);
         unsigned int filterPoint2 = (signalIndex + bitrate->period1SymbolSamples) % bitrate->period0SymbolSamples;

         // get signal samples
         float signalData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].filteredValue;
         float signalDeep = decoder->sample[futureIndex & (BUFFER_SIZE - 1)].modulateDepth;

         // store signal square in filter buffer
         modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)] = signalData * signalData * 10;

         // integrate symbol (moving average)
         modulation->filterIntegrate += modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->filterIntegrate -= modulation->integrationData[delay1Index & (BUFFER_SIZE - 1)]; // remove delayed value

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation results for each symbol and distance
         float correlatedS0 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint1];

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 0, modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]);
#endif

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 1, modulation->filterIntegrate);
#endif

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 2, correlatedS0);
#endif

         // start correlation after frameGuardTime
         if (decoder->signalClock < frameStatus.guardEnd)
            continue;

         // using signal variance at guard end as lower level threshold
         if (decoder->signalClock == frameStatus.guardEnd)
            modulation->searchValueThreshold = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].meanDeviation;

         // check if frame waiting time exceeded without detect modulation
         if (decoder->signalClock > frameStatus.waitingEnd)
            return PatternType::NoPattern;

         // check if poll frame modulation is detected while waiting for response
         if (signalDeep > maximumModulationDeep)
            return PatternType::NoPattern;

#ifdef DEBUG_NFC_CHANNEL
         if (decoder->signalClock < (frameStatus.guardEnd + 5))
            decoder->debug->set(DEBUG_NFC_CHANNEL + 2, modulation->searchValueThreshold);
#endif

#ifdef DEBUG_NFC_CHANNEL
         if (decoder->signalClock == modulation->searchSyncTime)
            decoder->debug->set(DEBUG_NFC_CHANNEL + 2, 0.75f);
#endif

         // wait until search start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         // search negative peak correlation
         if (correlatedS0 < -modulation->searchValueThreshold && correlatedS0 < modulation->correlatedPeakValue)
         {
            modulation->correlatedPeakValue = correlatedS0;
            modulation->correlatedPeakTime = decoder->signalClock;
            modulation->searchEndTime = decoder->signalClock + bitrate->period8SymbolSamples;
         }

         // search positive peak correlation
         if (correlatedS0 > modulation->searchValueThreshold && correlatedS0 > modulation->correlatedPeakValue)
         {
            modulation->correlatedPeakValue = correlatedS0;
            modulation->correlatedPeakTime = decoder->signalClock;
            modulation->searchEndTime = decoder->signalClock + bitrate->period8SymbolSamples;
         }

         // wait until search finished
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         switch (modulation->searchModeState)
         {
            case LISTEN_MODE_PREAMBLE1:
            {
               // preamble symbol start detected!
               if (!modulation->symbolStartTime)
               {
                  // now search preamble end
                  modulation->symbolStartTime = modulation->correlatedPeakTime - bitrate->period1SymbolSamples;
                  modulation->searchStartTime = modulation->correlatedPeakTime + bitrate->period0SymbolSamples;
                  modulation->searchEndTime = modulation->searchStartTime + bitrate->period1SymbolSamples;
                  modulation->correlatedPeakValue = 0;
                  modulation->correlatedPeakTime = 0;
                  continue;
               }

               // set preamble symbol ends
               modulation->symbolEndTime = modulation->correlatedPeakTime;

               // detect if preamble length is valid
               int preambleS1Length = modulation->symbolEndTime - modulation->symbolStartTime - bitrate->period1SymbolSamples;

               if (modulation->correlatedPeakTime == 0 || // invalid preamble
                   preambleS1Length < protocolStatus.sofS1MinimumTime || // preamble too short
                   preambleS1Length > protocolStatus.sofS1MaximumTime) // preamble too long
               {
                  modulation->searchModeState = LISTEN_MODE_PREAMBLE1;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;
                  continue;
               }

               // and trigger next stage
               modulation->searchModeState = LISTEN_MODE_PREAMBLE2;
               modulation->searchStartTime = modulation->correlatedPeakTime + bitrate->period1SymbolSamples - bitrate->period2SymbolSamples;
               modulation->searchEndTime = modulation->searchStartTime + bitrate->period1SymbolSamples;
               modulation->correlatedPeakValue = 0;
               modulation->correlatedPeakTime = 0;

               continue;
            }

            case LISTEN_MODE_PREAMBLE2:
            {
               // detect if preamble length is valid
               int preambleS2Length = modulation->correlatedPeakTime - modulation->symbolEndTime;

               if (modulation->correlatedPeakTime == 0 || // invalid preamble
                   preambleS2Length < protocolStatus.sofS2MinimumTime || // preamble too short
                   preambleS2Length > protocolStatus.sofS2MaximumTime) // preamble too long
               {
                  modulation->searchModeState = LISTEN_MODE_PREAMBLE1;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;
                  continue;
               }

               // if found, set SOF symbol end and next sync point
               modulation->symbolEndTime = modulation->correlatedPeakTime;

               // next search window timing
               modulation->searchSyncTime = modulation->symbolEndTime + decoder->bitrate->period0SymbolSamples;
               modulation->searchStartTime = modulation->searchSyncTime - decoder->bitrate->period4SymbolSamples;
               modulation->searchEndTime = modulation->searchSyncTime + decoder->bitrate->period4SymbolSamples;
               modulation->searchValueThreshold = modulation->correlatedPeakValue * 0.25;
               modulation->searchCorr0Value = 0;
               modulation->searchCorr1Value = 0;
               modulation->correlatedPeakTime = 0;
               modulation->correlatedPeakValue = 0;

               // set reference symbol info
               symbolStatus.value = 0;
               symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
               symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
               symbolStatus.length = symbolStatus.end - symbolStatus.start;
               symbolStatus.pattern = PatternType::PatternS;

               return symbolStatus.pattern;
            }
         }
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
      unsigned int delay1Index = (bitrate->offsetDelay1Index + decoder->signalClock);

      while (decoder->nextSample(buffer))
      {
         ++signalIndex;
         ++delay1Index;

         // compute correlation points
         unsigned int filterPoint1 = (signalIndex % bitrate->period0SymbolSamples);
         unsigned int filterPoint2 = (signalIndex + bitrate->period1SymbolSamples) % bitrate->period0SymbolSamples;

         // get signal samples
         float signalData = decoder->sample[signalIndex & (BUFFER_SIZE - 1)].filteredValue;

         // store signal square in filter buffer
         modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)] = signalData * signalData * 10;

         // integrate symbol (moving average)
         modulation->filterIntegrate += modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]; // add new value
         modulation->filterIntegrate -= modulation->integrationData[delay1Index & (BUFFER_SIZE - 1)]; // remove delayed value

         // store integrated signal in correlation buffer
         modulation->correlationData[filterPoint1] = modulation->filterIntegrate;

         // compute correlation results for each symbol and distance
         float correlatedS0 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint1];
         float correlatedSD = std::fabs(correlatedS0);

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 0, modulation->integrationData[signalIndex & (BUFFER_SIZE - 1)]);
#endif

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 1, modulation->filterIntegrate);
#endif

#ifdef DEBUG_NFC_CHANNEL
         decoder->debug->set(DEBUG_NFC_CHANNEL + 2, correlatedS0);
#endif

#ifdef DEBUG_NFC_CHANNEL
         if (decoder->signalClock == modulation->searchSyncTime)
            decoder->debug->set(DEBUG_NFC_CHANNEL + 2, 0.50f);
#endif

         // wait until search window start
         if (decoder->signalClock < modulation->searchStartTime)
            continue;

         // detect max correlation peak
         if (correlatedSD > modulation->searchValueThreshold && correlatedSD > modulation->correlatedPeakValue)
         {
            modulation->searchCorr0Value = correlatedS0;
            modulation->searchCorr1Value = -correlatedS0;
            modulation->correlatedPeakValue = correlatedSD;
            modulation->symbolEndTime = decoder->signalClock;
         }

         // wait until search window ends
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // no modulation found(End Of Frame)
         if (modulation->correlatedPeakValue < modulation->searchValueThreshold)
            return PatternType::PatternS;

         // estimated symbol start and end
         modulation->symbolStartTime = modulation->symbolEndTime;
         modulation->symbolEndTime = modulation->symbolStartTime + decoder->bitrate->period0SymbolSamples;

         // timing search window
         modulation->searchSyncTime = modulation->symbolEndTime;
         modulation->searchStartTime = modulation->searchSyncTime - decoder->bitrate->period4SymbolSamples;
         modulation->searchEndTime = modulation->searchSyncTime + decoder->bitrate->period4SymbolSamples;
         modulation->searchValueThreshold = modulation->correlatedPeakValue * 0.25;
         modulation->correlatedPeakTime = 0;
         modulation->correlatedPeakValue = 0;

         // setup symbol info
         symbolStatus.value = modulation->searchCorr0Value > modulation->searchCorr1Value ? 0 : 1;
         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;
         symbolStatus.pattern = symbolStatus.value ? PatternType::Pattern1 : PatternType::Pattern0;

         return symbolStatus.pattern;
      }

      return PatternType::Invalid;
   }

   /*
   * Reset modulation status
   */
   inline void resetModulation()
   {
      // clear stream status
      streamStatus = {0,};

      // clear stream status
      symbolStatus = {0,};

      // clear modulation status
      modulationStatus = {0,};

      // clear frame status
      frameStatus.frameType = 0;
      frameStatus.frameStart = 0;
      frameStatus.frameEnd = 0;

      // restore pulse code
      decoder->pulse = nullptr;

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
         frameStatus.frameGuardTime = protocolStatus.frameGuardTime;
         frameStatus.frameWaitingTime = protocolStatus.frameWaitingTime;
      }
         // for response frames only set frame guard time before receive next poll frame
      else
      {
         // initialize frame parameters to default protocol parameters
         frameStatus.frameGuardTime = protocolStatus.frameGuardTime;
      }

      do
      {
//         if (processREQA(frame))
//            break;
//
//         if (processHLTA(frame))
//            break;

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
            // response guard time TR0min (PICC must not modulate reponse within this period)
            frameStatus.guardEnd = frameStatus.frameEnd + frameStatus.frameGuardTime - decoder->bitrate->symbolDelayDetect;

            // response delay time WFT (PICC must reply to command before this period)
            frameStatus.waitingEnd = frameStatus.frameEnd + frameStatus.frameWaitingTime - decoder->bitrate->symbolDelayDetect;

            // next frame must be ListenFrame
            frameStatus.frameType = ListenFrame;
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
    * Process other frames
    */
   inline void processOther(NfcFrame &frame)
   {
      frame.setFramePhase(FramePhase::ApplicationFrame);
      frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);
   }

   /*
    * Check NFC-V crc
    */
   inline bool checkCrc(NfcFrame &frame)
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

NfcV::NfcV(DecoderStatus *decoder) : self(new Impl(decoder))
{
}

NfcV::~NfcV()
{
   delete self;
}

void NfcV::setModulationThreshold(float min, float max)
{
   if (!std::isnan(min))
      self->minimumModulationDeep = min;

   if (!std::isnan(max))
      self->maximumModulationDeep = max;
}

void NfcV::setCorrelationThreshold(float value)
{
   if (!std::isnan(value))
      self->minimumCorrelationThreshold = value;
}

void NfcV::configure(long sampleRate)
{
   self->configure(sampleRate);
}

bool NfcV::detect()
{
   return self->detectModulation();
}

void NfcV::decode(sdr::SignalBuffer &samples, std::list<NfcFrame> &frames)
{
   self->decodeFrame(samples, frames);
}

}

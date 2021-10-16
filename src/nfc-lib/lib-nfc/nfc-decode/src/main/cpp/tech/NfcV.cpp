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

#include <tech/NfcV.h>

#ifdef DEBUG_SIGNAL
#define DEBUG_ASK_CORR_CHANNEL 1
#define DEBUG_ASK_SYNC_CHANNEL 2
#endif

#define SOF_BEGIN 0
#define SOF_MODE 1

namespace nfc {

enum PatternType
{
   Invalid = 0,
   NoPattern = 1,
   Pattern2 = 2, // data pulse pattern for 2 bit code
   Pattern8 = 3, // data pulse pattern for 8 bit code
   PatternS = 4, // frame end pattern
   PatternE = 5, // frame error pattern
};

struct NfcV::Impl
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

   // minimum modulation threshold to detect valid signal for NFC-V (default 85%)
   float minimumModulationThreshold = 0.850f;

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

      // clear modulation parameters
      modulationStatus = {0,};

      // clear bitrate parameters
      bitrateParams = {0,};

      // set tech type and rate
      bitrateParams.techType = TechType::NfcV;

      // NFC-V has constant symbol rate
      bitrateParams.symbolsPerSecond = NFC_FC / 256;

      // number of samples per symbol
      bitrateParams.period1SymbolSamples = int(round(decoder->signalParams.sampleTimeUnit * 256)); // full symbol samples
      bitrateParams.period2SymbolSamples = int(round(decoder->signalParams.sampleTimeUnit * 128)); // half symbol samples
      bitrateParams.period4SymbolSamples = int(round(decoder->signalParams.sampleTimeUnit * 64)); // quarter of symbol...
      bitrateParams.period8SymbolSamples = int(round(decoder->signalParams.sampleTimeUnit * 32)); // and so on...

      // moving average offsets
      bitrateParams.offsetSignalIndex = BUFFER_SIZE;
      bitrateParams.offsetDelay1Index = BUFFER_SIZE - bitrateParams.period1SymbolSamples;
      bitrateParams.offsetDelay2Index = BUFFER_SIZE - bitrateParams.period2SymbolSamples;
      bitrateParams.offsetDelay4Index = BUFFER_SIZE - bitrateParams.period4SymbolSamples;
      bitrateParams.offsetDelay8Index = BUFFER_SIZE - bitrateParams.period8SymbolSamples;

      // exponential symbol average
      bitrateParams.symbolAverageW0 = float(1 - 5.0 / bitrateParams.period1SymbolSamples);
      bitrateParams.symbolAverageW1 = float(1 - bitrateParams.symbolAverageW0);

      log.info("{} kpbs parameters:", {round(bitrateParams.symbolsPerSecond / 1E3)});
      log.info("\tsymbolsPerSecond     {}", {bitrateParams.symbolsPerSecond});
      log.info("\tperiod1SymbolSamples {} ({} us)", {bitrateParams.period1SymbolSamples, 1E6 * bitrateParams.period1SymbolSamples / decoder->sampleRate});
      log.info("\tperiod2SymbolSamples {} ({} us)", {bitrateParams.period2SymbolSamples, 1E6 * bitrateParams.period2SymbolSamples / decoder->sampleRate});
      log.info("\tperiod4SymbolSamples {} ({} us)", {bitrateParams.period4SymbolSamples, 1E6 * bitrateParams.period4SymbolSamples / decoder->sampleRate});
      log.info("\tperiod8SymbolSamples {} ({} us)", {bitrateParams.period8SymbolSamples, 1E6 * bitrateParams.period8SymbolSamples / decoder->sampleRate});
      log.info("\toffsetSignalIndex    {}", {bitrateParams.offsetSignalIndex});
      log.info("\toffsetDelay1Index    {}", {bitrateParams.offsetDelay1Index});
      log.info("\toffsetDelay2Index    {}", {bitrateParams.offsetDelay2Index});
      log.info("\toffsetDelay4Index    {}", {bitrateParams.offsetDelay4Index});
      log.info("\toffsetDelay8Index    {}", {bitrateParams.offsetDelay8Index});

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
      pulse->length = pulse->periods * 256 / NFC_FC;

      for (int i = 0; i < pulse->periods; i++)
      {
         pulse->slots[i] = {.start = i * (256 / NFC_FC), .end = (i + 1) * (256 / NFC_FC), .value = i};
      }
   }

   inline bool detectModulation()
   {
      // ignore low power signals
      if (decoder->signalStatus.powerAverage < decoder->powerLevelThreshold)
         return false;

      BitrateParams *bitrate = &bitrateParams;
      ModulationStatus *modulation = &modulationStatus;

      // compute signal pointers
      modulation->signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
      modulation->delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

      // get signal samples
      float currentData = decoder->signalStatus.signalData[modulation->signalIndex & (BUFFER_SIZE - 1)];
      float delayedData = decoder->signalStatus.signalData[modulation->delay2Index & (BUFFER_SIZE - 1)];

      // integrate signal data over 1/2 symbol
      modulation->filterIntegrate += currentData; // add new value
      modulation->filterIntegrate -= delayedData; // remove delayed value

      // correlation points
      modulation->filterPoint1 = (modulation->signalIndex % bitrate->period1SymbolSamples);
      modulation->filterPoint2 = (modulation->signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;

      // store integrated signal in correlation buffer
      modulation->correlationData[modulation->filterPoint1] = modulation->filterIntegrate;

      // compute correlation factor
      modulation->correlatedS0 = (modulation->correlationData[modulation->filterPoint2] - modulation->correlationData[modulation->filterPoint1]) / float(bitrate->period2SymbolSamples);

      // compute symbol average
      modulation->symbolAverage = (modulation->symbolAverage * bitrate->symbolAverageW0) + (currentData * bitrate->symbolAverageW1);

#ifdef DEBUG_ASK_CORR_CHANNEL
      decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, modulation->correlatedS0);
#endif
#ifdef DEBUG_ASK_SYNC_CHANNEL
      decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.0f);
#endif
      // search for first falling edge
      switch (modulation->searchStage)
      {
         case SOF_BEGIN:

            // max correlation peak detector
            if (modulation->correlatedS0 > decoder->signalStatus.powerAverage * minimumModulationThreshold && modulation->correlatedS0 > modulation->correlationPeek)
            {
               modulation->searchPeakTime = decoder->signalClock;
               modulation->searchEndTime = decoder->signalClock + bitrate->period2SymbolSamples;
               modulation->correlationPeek = modulation->correlatedS0;
            }

            // wait until search finished
            if (decoder->signalClock != modulation->searchEndTime)
               break;

#ifdef DEBUG_ASK_SYNC_CHANNEL
            decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.75f);
#endif
            // sets SOF symbol frame start (also frame start)
            modulation->symbolStartTime = modulation->searchPeakTime - bitrate->period2SymbolSamples;

            // and triger next stage
            modulation->searchStage = SOF_MODE;
            modulation->searchStartTime = modulation->symbolStartTime + (2 * bitrate->period1SymbolSamples);
            modulation->searchEndTime = modulation->symbolStartTime + (4 * bitrate->period1SymbolSamples);
            modulation->searchPeakTime = 0;
            modulation->correlationPeek = 0;

            break;

         case SOF_MODE:

            // detect SOF modulation pulse code 1 / 4 or 1 / 256
            if (decoder->signalClock > modulation->searchStartTime && decoder->signalClock <= modulation->searchEndTime)
            {
               // max correlation peak detector
               if (modulation->correlatedS0 > decoder->signalStatus.powerAverage * minimumModulationThreshold && modulation->correlatedS0 > modulation->correlationPeek)
               {
                  modulation->searchPeakTime = decoder->signalClock;
                  modulation->searchEndTime = decoder->signalClock + bitrate->period2SymbolSamples;
                  modulation->correlationPeek = modulation->correlatedS0;
               }

               // wait until search finished
               if (decoder->signalClock != modulation->searchEndTime)
                  break;

#ifdef DEBUG_ASK_SYNC_CHANNEL
               decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.75f);
#endif
               // if no pulse detected reset modulation search
               if (!modulation->searchPeakTime)
               {
                  // if no edge found, restart search
                  modulation->searchStage = SOF_BEGIN;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->searchPeakTime = 0;
                  modulation->correlationPeek = 0;
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;

                  break;
               }

               // reset modulation for next search
               modulation->searchStage = SOF_BEGIN;
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;
               modulation->correlationPeek = 0;

               // check for 1 of 4 code
               if (modulation->searchPeakTime > (modulation->symbolStartTime + 3 * bitrate->period1SymbolSamples - bitrate->period8SymbolSamples) &&
                   modulation->searchPeakTime < (modulation->symbolStartTime + 3 * bitrate->period1SymbolSamples + bitrate->period8SymbolSamples))
               {
                  // set SOF symbol parameters
                  modulation->symbolEndTime = modulation->searchPeakTime + bitrate->period1SymbolSamples;

                  // setup frame info
                  frameStatus.frameType = PollFrame;
                  frameStatus.symbolRate = bitrate->symbolsPerSecond;
                  frameStatus.frameStart = modulation->symbolStartTime;
                  frameStatus.frameEnd = 0;

                  // modulation detected
                  decoder->pulse = pulseParams + 0;
                  decoder->bitrate = bitrate;
                  decoder->modulation = modulation;

                  return true;
               }

               // check for 1 of 256 code
               if (modulation->searchPeakTime > (modulation->symbolStartTime + 4 * bitrate->period1SymbolSamples - bitrate->period8SymbolSamples) &&
                   modulation->searchPeakTime < (modulation->symbolStartTime + 4 * bitrate->period1SymbolSamples + bitrate->period8SymbolSamples))
               {
                  // set SOF symbol parameters
                  modulation->symbolEndTime = modulation->searchPeakTime;

                  // setup frame info
                  frameStatus.frameType = PollFrame;
                  frameStatus.symbolRate = bitrate->symbolsPerSecond;
                  frameStatus.frameStart = modulation->symbolStartTime;
                  frameStatus.frameEnd = 0;

                  // modulation detected
                  decoder->pulse = pulseParams + 1;
                  decoder->bitrate = bitrate;
                  decoder->modulation = modulation;

                  return true;
               }

               // invalid code detected, reset symbol status
               modulation->symbolStartTime = 0;
               modulation->symbolEndTime = 0;
            }

            break;
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
               frameStatus.frameEnd = symbolStatus.end;

               NfcFrame response = NfcFrame(TechType::NfcV, FrameType::PollFrame);

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

//         // decode next bit
//         if (streamStatus.bits < 9)
//         {
//            if (streamStatus.bits > 0)
//               streamStatus.data |= (symbolStatus.value << (streamStatus.bits - 1));
//
//            streamStatus.bits++;
//         }
//            // store full byte in stream buffer
//         else
//         {
//            streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;
//            streamStatus.data = 0;
//            streamStatus.bits = 0;
//         }
      }

      // no frame detected
      return false;
   }

   inline bool decodeListenFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
   {
      resetModulation();

      return false;
   }

   inline int decodePollFrameSymbolPpm(sdr::SignalBuffer &buffer)
   {
      symbolStatus.pattern = PatternType::Invalid;

      PulseParams *pulse = decoder->pulse;
      BitrateParams *bitrate = decoder->bitrate;
      ModulationStatus *modulation = decoder->modulation;

      while (decoder->nextSample(buffer))
      {
         // compute signal pointers
         modulation->signalIndex = (bitrate->offsetSignalIndex + decoder->signalClock);
         modulation->delay2Index = (bitrate->offsetDelay2Index + decoder->signalClock);

         // get signal samples
         float currentData = decoder->signalStatus.signalData[modulation->signalIndex & (BUFFER_SIZE - 1)];
         float delayedData = decoder->signalStatus.signalData[modulation->delay2Index & (BUFFER_SIZE - 1)];

         // integrate signal data over 1/2 symbol
         modulation->filterIntegrate += currentData; // add new value
         modulation->filterIntegrate -= delayedData; // remove delayed value

         // correlation points
         modulation->filterPoint1 = (modulation->signalIndex % bitrate->period1SymbolSamples);
         modulation->filterPoint2 = (modulation->signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;

         // store integrated signal in correlation buffer
         modulation->correlationData[modulation->filterPoint1] = modulation->filterIntegrate;

         // compute correlation factor
         modulation->correlatedS0 = (modulation->correlationData[modulation->filterPoint2] - modulation->correlationData[modulation->filterPoint1]) / float(bitrate->period2SymbolSamples);

#ifdef DEBUG_ASK_CORR_CHANNEL
         decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, modulation->correlatedS0);
#endif

#ifdef DEBUG_ASK_SYNC_CHANNEL
         decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.0f);
#endif
         // set next search sync window from previous state
         if (!modulation->searchStartTime)
         {
            // estimated symbol start and end
            modulation->symbolStartTime = modulation->symbolEndTime;
            modulation->symbolEndTime = modulation->symbolStartTime + 4 * bitrate->period1SymbolSamples;

            // timing search window
            modulation->searchStartTime = modulation->symbolStartTime;
            modulation->searchEndTime = modulation->symbolEndTime;
         }

#ifdef DEBUG_ASK_SYNC_CHANNEL
         if (decoder->signalClock == modulation->searchStartTime)
            decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.50f);
#endif

         // search max correlation peak
         if (decoder->signalClock >= modulation->searchStartTime && decoder->signalClock <= modulation->searchEndTime)
         {
            if (modulation->correlatedS0 > modulation->correlationPeek)
            {
               modulation->correlationPeek = modulation->correlatedS0;
               modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
               modulation->searchPeakTime = decoder->signalClock;
            }
         }

         // wait until search finished
         if (decoder->signalClock != modulation->searchEndTime)
            continue;

         // detect Pattern-S when modulation occurs in the first part of the second slot
         if (modulation->searchPeakTime > (modulation->symbolStartTime + 1 * bitrate->period1SymbolSamples + bitrate->period4SymbolSamples) &&
             modulation->searchPeakTime < (modulation->symbolStartTime + 2 * bitrate->period1SymbolSamples - bitrate->period4SymbolSamples))
         {
#ifdef DEBUG_ASK_SYNC_CHANNEL
            decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.75f);
#endif
            // estimate symbol end from start (peak detection not valid due lack of modulation)
            modulation->symbolEndTime = modulation->searchPeakTime + decoder->bitrate->period2SymbolSamples;

            // setup symbol info
            symbolStatus.value = 0;
            symbolStatus.start = modulation->symbolStartTime;
            symbolStatus.end = modulation->symbolEndTime;
            symbolStatus.length = symbolStatus.end - symbolStatus.start;
            symbolStatus.pattern = PatternType::PatternS;

            break;
         }

         // by default assume pulse error
         symbolStatus.value = 0;
         symbolStatus.start = modulation->symbolStartTime;
         symbolStatus.end = modulation->symbolEndTime;
         symbolStatus.length = symbolStatus.end - symbolStatus.start;
         symbolStatus.pattern = PatternType::PatternE;

         // search pulse code
         for (int i = 0; i < pulse->periods; i++)
         {
            PulseSlot *slot = pulse->slots + i;

            // pulse position must be in second half, otherwise is protocol error
            if (modulation->searchPeakTime > (modulation->symbolStartTime + slot->end - bitrate->period4SymbolSamples) &&
                modulation->searchPeakTime < (modulation->symbolStartTime + slot->end + bitrate->period4SymbolSamples))
            {
               // setup symbol info
               symbolStatus.value = slot->value;
               symbolStatus.start = modulation->symbolStartTime;
               symbolStatus.end = modulation->symbolEndTime;
               symbolStatus.length = symbolStatus.end - symbolStatus.start;
               symbolStatus.pattern = pulse->bits == 2 ? PatternType::Pattern2 : PatternType::Pattern8;

               break;
            }
         }

         break;
      }

      // reset search status if symbol has detected
      if (symbolStatus.pattern != PatternType::Invalid)
      {
         modulation->searchStartTime = 0;
         modulation->searchEndTime = 0;
         modulation->correlationPeek = 0;
      }

      return symbolStatus.pattern;
   }

   inline int decodeListenFrameSymbolBpsk(sdr::SignalBuffer &buffer)
   {
      return 0;
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
      modulationStatus.searchStage = 0;
      modulationStatus.searchStartTime = 0;
      modulationStatus.searchEndTime = 0;
      modulationStatus.searchDeepValue = 0;
      modulationStatus.symbolAverage = 0;
      modulationStatus.correlationPeek = 0;

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
   }
};

NfcV::NfcV(DecoderStatus *decoder) : self(new Impl(decoder))
{
}

NfcV::~NfcV()
{
   delete self;
}

void NfcV::setModulationThreshold(float min)
{
   self->minimumModulationThreshold = min;
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

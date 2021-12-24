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

#define SOF_SEARCH 0
#define SOF_PREAMBLE 1
#define SOF_SYNCCODE 2

namespace nfc {

struct NfcF::Impl
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
         bitrate->symbolDelayDetect = 0; //rate == r106k ? int(std::round(decoder->signalParams.sampleTimeUnit * 2048)) : bitrateParams[rate - 1].symbolDelayDetect + bitrateParams[rate - 1].period1SymbolSamples;

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

      // POLL frame ASK detector for 212Kbps and 424Kbps
      for (int rate = r212k; rate <= r212k; rate++)
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
         float correlatedS1 = modulation->correlationData[filterPoint1] - modulation->correlationData[filterPoint2];
         float correlatedS0 = modulation->correlationData[filterPoint2] - modulation->correlationData[filterPoint3];
         float correlatedSD = std::fabs(correlatedS0 - correlatedS1) / float(bitrate->period2SymbolSamples);

         // compute symbol average
//         modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + signalData * bitrate->symbolAverageW1;

#ifdef DEBUG_ASK_CORR_CHANNEL
         decoder->debug->set(DEBUG_ASK_CORR_CHANNEL, correlatedS0 / float(bitrate->period2SymbolSamples));
#endif

         // reset modulation if exceed limits
         if (signalDeep > maximumModulationThreshold)
         {
            modulation->searchStage = SOF_SEARCH;
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->correlationPeek = 0;

            return false;
         }

#ifdef DEBUG_ASK_SYNC_CHANNEL
         if (decoder->signalClock == modulation->symbolSyncTime)
            decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.50f);
#endif

         // search for preamble and sync code
         switch (modulation->searchStage)
         {
            case SOF_SEARCH:
            {
               // detect maximum correlation peak
               if (correlatedSD > decoder->signalStatus.signalAverg * minimumModulationThreshold && correlatedSD > modulation->correlationPeek)
               {
                  modulation->searchPeakTime = decoder->signalClock;
                  modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
                  modulation->correlationPeek = correlatedSD;
               }

               // wait until search finished
               if (decoder->signalClock != modulation->searchEndTime)
                  break;

#ifdef DEBUG_ASK_SYNC_CHANNEL
               decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.750f);
#endif
               // check minimum modulation deep
               if (!modulation->searchPeakTime)
               {
                  // reset modulation to continue search
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->correlationPeek = 0;
                  break;
               }

               // sets first symbol start and end time (also frame start)
               modulation->symbolStartTime = modulation->searchPeakTime - bitrate->period1SymbolSamples;
               modulation->symbolEndTime = modulation->searchPeakTime;
               modulation->symbolSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;

               // and trigger next stage
               modulation->searchStage = SOF_PREAMBLE;
               modulation->searchPulseCount = 1;
               modulation->searchStartTime = modulation->symbolSyncTime - bitrate->period4SymbolSamples;
               modulation->searchEndTime = modulation->symbolSyncTime + bitrate->period4SymbolSamples;
               modulation->searchThreshold = decoder->signalStatus.signalAverg * minimumModulationThreshold;
               modulation->searchPeakTime = 0;
               modulation->correlationPeek = 0;

               break;
            }

            case SOF_PREAMBLE:
            {
               // wait until search start
               if (decoder->signalClock < modulation->searchStartTime)
                  break;

               // detect maximum correlation peak for re-synchronization
               if (correlatedSD > modulation->searchThreshold && correlatedSD > modulation->correlationPeek)
               {
                  modulation->searchPeakTime = decoder->signalClock;
                  modulation->searchEndTime = decoder->signalClock + bitrate->period4SymbolSamples;
                  modulation->correlationPeek = correlatedSD;
               }

               // capture symbol correlation values at synchronization point
               if (decoder->signalClock == modulation->symbolSyncTime)
               {
                  modulation->symbolCorr0 = correlatedS0;
                  modulation->symbolCorr1 = correlatedS1;
               }

               // wait until search finished
               if (decoder->signalClock != modulation->searchEndTime)
                  continue;

               // no valid modulation found, reset search
               if (!modulation->searchPeakTime || modulation->symbolCorr0 < modulation->symbolCorr1)
               {
                  modulation->searchStage = SOF_SEARCH;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->searchPeakTime = 0;
                  modulation->symbolStartTime = 0;
                  modulation->symbolEndTime = 0;
                  modulation->correlationPeek = 0;
                  break;
               }

               // re-sync and sets symbol start and end time
               modulation->symbolStartTime = modulation->symbolEndTime;
               modulation->symbolEndTime = modulation->searchPeakTime;
               modulation->symbolSyncTime = modulation->symbolEndTime + bitrate->period1SymbolSamples;

               modulation->searchPulseCount++;
               modulation->searchStartTime = modulation->symbolSyncTime - bitrate->period4SymbolSamples;
               modulation->searchEndTime = modulation->symbolSyncTime + bitrate->period4SymbolSamples;
               modulation->searchPeakTime = 0;
               modulation->correlationPeek = 0;

               if (modulation->searchPulseCount == 6 * 8)
               {
#ifdef DEBUG_ASK_SYNC_CHANNEL
                  decoder->debug->set(DEBUG_ASK_SYNC_CHANNEL, 0.750f);
#endif
                  // setup frame info
                  frameStatus.frameType = PollFrame;
                  frameStatus.symbolRate = bitrate->symbolsPerSecond;
                  frameStatus.frameStart = modulation->symbolStartTime - bitrate->symbolDelayDetect;
                  frameStatus.frameEnd = 0;

                  decoder->bitrate = bitrate;
                  decoder->modulation = modulation;

                  return true;
               }

               break;
            }
         }

         // set lower threshold to detect valid response pattern
//         modulation->searchThreshold = decoder->signalStatus.signalAverg * minimumModulationThreshold;
//
//         // set pattern search window
//         modulation->symbolStartTime = modulation->searchPeakTime - bitrate->period2SymbolSamples;
//         modulation->symbolEndTime = modulation->searchPeakTime + bitrate->period2SymbolSamples;

         // setup frame info
//         frameStatus.frameType = PollFrame;
//         frameStatus.symbolRate = bitrate->symbolsPerSecond;
//         frameStatus.frameStart = modulation->symbolStartTime - bitrate->symbolDelayDetect;
//         frameStatus.frameEnd = 0;

//         // setup symbol info
//         symbolStatus.value = 0;
//         symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
//         symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
//         symbolStatus.length = symbolStatus.end - symbolStatus.start;
//         symbolStatus.pattern = PatternType::PatternZ;
//
//         // reset modulation to continue search
//         modulation->searchStartTime = 0;
//         modulation->searchEndTime = 0;
//         modulation->correlationPeek = 0;
//
//         // modulation detected
//         decoder->bitrate = bitrate;
//         decoder->modulation = modulation;
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
      resetModulation();

      return false;
   }

   inline bool decodeListenFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
   {
      resetModulation();

      return false;
   }

   /*
    * Reset modulation status
    */
   inline void resetModulation()
   {
      // reset modulation detection for all rates
      for (int rate = r212k; rate <= r424k; rate++)
      {
         modulationStatus[rate].searchStartTime = 0;
         modulationStatus[rate].searchEndTime = 0;
         modulationStatus[rate].correlationPeek = 0;
         modulationStatus[rate].searchPulseWidth = 0;
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

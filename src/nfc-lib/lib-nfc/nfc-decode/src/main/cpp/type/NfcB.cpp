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

#include "NfcB.h"

#ifdef DEBUG_SIGNAL
#define DEBUG_ASK_INTEGRATION_CHANNEL 1
#define DEBUG_ASK_DETECTION_CHANNEL 2
#define DEBUG_ASK_SYNCHRONIZATION_CHANNEL 3
#endif

#define SOF_START_EDGE 0
#define SOF_MIDDLE_EDGE 1
#define SOF_END_EDGE 2

namespace nfc {

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

   // minimum modulation threshold to detect valid signal for NFC-B (default 25%)
   float maximumModulationThreshold = 0.50f;

   // last detected frame end
   unsigned int lastFrameEnd = 0;

   // chained frame flags
   unsigned int chainedFlags = 0;

   Impl(DecoderStatus *decoder) : decoder(decoder)
   {
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
   self->minimumModulationThreshold = min;
   self->maximumModulationThreshold = max;
}

void NfcB::configure(long sampleRate)
{
   self->log.info("--------------------------------------------");
   self->log.info("initializing NFC-B decoder");
   self->log.info("--------------------------------------------");
   self->log.info("\tsignalSampleRate     {}", {self->decoder->sampleRate});
   self->log.info("\tpowerLevelThreshold  {}", {self->decoder->powerLevelThreshold});
   self->log.info("\tmodulationThreshold  {} -> {}", {self->minimumModulationThreshold, self->maximumModulationThreshold});

   // clear detected symbol status
   self->symbolStatus = {0,};

   // clear bit stream status
   self->streamStatus = {0,};

   // clear frame processing status
   self->frameStatus = {0,};

   // clear last detected frame end
   self->lastFrameEnd = 0;

   // clear chained flags
   self->chainedFlags = 0;

   // compute symbol parameters for 106Kbps, 212Kbps, 424Kbps and 848Kbps
   for (int rate = r106k; rate <= r424k; rate++)
   {
      // clear bitrate parameters
      self->bitrateParams[rate] = {0,};

      // clear modulation parameters
      self->modulationStatus[rate] = {0,};

      // configure bitrate parametes
      BitrateParams *bitrate = self->bitrateParams + rate;

      // set tech type and rate
      bitrate->techType = TechType::NfcB;
      bitrate->rateType = rate;

      // symbol timing parameters
      bitrate->symbolsPerSecond = BaseFrequency / (128 >> rate);

      // number of samples per symbol
      bitrate->period1SymbolSamples = int(round(self->decoder->signalParams.sampleTimeUnit * (128 >> rate))); // full symbol samples
      bitrate->period2SymbolSamples = int(round(self->decoder->signalParams.sampleTimeUnit * (64 >> rate))); // half symbol samples
      bitrate->period4SymbolSamples = int(round(self->decoder->signalParams.sampleTimeUnit * (32 >> rate))); // quarter of symbol...
      bitrate->period8SymbolSamples = int(round(self->decoder->signalParams.sampleTimeUnit * (16 >> rate))); // and so on...

      // delay guard for each symbol rate
      bitrate->symbolDelayDetect = rate > r106k ? self->bitrateParams[rate - 1].symbolDelayDetect + self->bitrateParams[rate - 1].period1SymbolSamples : 0;

      // moving average offsets
      bitrate->offsetSignalIndex = SignalBufferLength - bitrate->symbolDelayDetect;
      bitrate->offsetSymbolIndex = SignalBufferLength - bitrate->symbolDelayDetect - bitrate->period1SymbolSamples;
      bitrate->offsetFilterIndex = SignalBufferLength - bitrate->symbolDelayDetect - bitrate->period4SymbolSamples;
      bitrate->offsetDetectIndex = SignalBufferLength - bitrate->symbolDelayDetect - bitrate->period8SymbolSamples;

      // exponential symbol average
      bitrate->symbolAverageW0 = float(1 - 5.0 / bitrate->period1SymbolSamples);
      bitrate->symbolAverageW1 = float(1 - bitrate->symbolAverageW0);

      self->log.info("{} kpbs parameters:", {round(bitrate->symbolsPerSecond / 1E3)});
      self->log.info("\tsymbolsPerSecond     {}", {bitrate->symbolsPerSecond});
      self->log.info("\tperiod1SymbolSamples {} ({} us)", {bitrate->period1SymbolSamples, 1E6 * bitrate->period1SymbolSamples / self->decoder->sampleRate});
      self->log.info("\tperiod2SymbolSamples {} ({} us)", {bitrate->period2SymbolSamples, 1E6 * bitrate->period2SymbolSamples / self->decoder->sampleRate});
      self->log.info("\tperiod4SymbolSamples {} ({} us)", {bitrate->period4SymbolSamples, 1E6 * bitrate->period4SymbolSamples / self->decoder->sampleRate});
      self->log.info("\tperiod8SymbolSamples {} ({} us)", {bitrate->period8SymbolSamples, 1E6 * bitrate->period8SymbolSamples / self->decoder->sampleRate});
      self->log.info("\tsymbolDelayDetect    {} ({} us)", {bitrate->symbolDelayDetect, 1E6 * bitrate->symbolDelayDetect / self->decoder->sampleRate});
      self->log.info("\toffsetSignalIndex    {}", {bitrate->offsetSignalIndex});
      self->log.info("\toffsetSymbolIndex    {}", {bitrate->offsetSymbolIndex});
      self->log.info("\toffsetFilterIndex    {}", {bitrate->offsetFilterIndex});
      self->log.info("\toffsetDetectIndex    {}", {bitrate->offsetDetectIndex});
   }

   // initialize default protocol parameters for start decoding
   self->protocolStatus.maxFrameSize = 256;
   self->protocolStatus.startUpGuardTime = int(self->decoder->signalParams.sampleTimeUnit * 256 * 16 * (1 << 0));
   self->protocolStatus.frameWaitingTime = int(self->decoder->signalParams.sampleTimeUnit * 256 * 16 * (1 << 4));
   self->protocolStatus.frameGuardTime = int(self->decoder->signalParams.sampleTimeUnit * 128 * 7);
   self->protocolStatus.requestGuardTime = int(self->decoder->signalParams.sampleTimeUnit * 7000);

   // initialize frame parameters to default protocol parameters
   self->frameStatus.startUpGuardTime = self->protocolStatus.startUpGuardTime;
   self->frameStatus.frameWaitingTime = self->protocolStatus.frameWaitingTime;
   self->frameStatus.frameGuardTime = self->protocolStatus.frameGuardTime;
   self->frameStatus.requestGuardTime = self->protocolStatus.requestGuardTime;

   // initialize exponential average factors for power value
   self->decoder->signalParams.powerAverageW0 = float(1 - 1E3 / self->decoder->sampleRate);
   self->decoder->signalParams.powerAverageW1 = float(1 - self->decoder->signalParams.powerAverageW0);

   // initialize exponential average factors for signal average
   self->decoder->signalParams.signalAverageW0 = float(1 - 1E5 / self->decoder->sampleRate);
   self->decoder->signalParams.signalAverageW1 = float(1 - self->decoder->signalParams.signalAverageW0);

   // initialize exponential average factors for signal variance
   self->decoder->signalParams.signalVarianceW0 = float(1 - 1E5 / self->decoder->sampleRate);
   self->decoder->signalParams.signalVarianceW1 = float(1 - self->decoder->signalParams.signalVarianceW0);

   self->log.info("Startup parameters");
   self->log.info("\tmaxFrameSize {} bytes", {self->protocolStatus.maxFrameSize});
   self->log.info("\tframeGuardTime {} samples ({} us)", {self->protocolStatus.frameGuardTime, 1000000.0 * self->protocolStatus.frameGuardTime / self->decoder->sampleRate});
   self->log.info("\tframeWaitingTime {} samples ({} us)", {self->protocolStatus.frameWaitingTime, 1000000.0 * self->protocolStatus.frameWaitingTime / self->decoder->sampleRate});
   self->log.info("\trequestGuardTime {} samples ({} us)", {self->protocolStatus.requestGuardTime, 1000000.0 * self->protocolStatus.requestGuardTime / self->decoder->sampleRate});
}

bool NfcB::detectModulation()
{
   if (self->decoder->signalClock == 12046578)
      self->log.info("12046578");

   // ignore low power signals
   if (self->decoder->signalStatus.powerAverage > self->decoder->powerLevelThreshold)
   {
      // POLL frame ASK detector for  106Kbps, 212Kbps and 424Kbps
      for (int rate = r106k; rate <= r106k; rate++)
      {
         BitrateParams *bitrate = self->bitrateParams + rate;
         ModulationStatus *modulation = self->modulationStatus + rate;

         // compute signal pointers for edge detector, current index, slow average, and fast average
         modulation->signalIndex = (bitrate->offsetSignalIndex + self->decoder->signalClock);
         modulation->filterIndex = (bitrate->offsetFilterIndex + self->decoder->signalClock); // 1/4 symbol delay
         modulation->detectIndex = (bitrate->offsetDetectIndex + self->decoder->signalClock); // 1/4 symbol delay

         // get signal samples
         float signalData = self->decoder->signalStatus.signalData[modulation->signalIndex & (SignalBufferLength - 1)];
         float filterData = self->decoder->signalStatus.signalData[modulation->filterIndex & (SignalBufferLength - 1)];
         float detectData = self->decoder->signalStatus.signalData[modulation->detectIndex & (SignalBufferLength - 1)];

         // integrate signal data over 1/4 symbol (slow average)
         modulation->filterIntegrate += signalData; // add new value
         modulation->filterIntegrate -= filterData; // remove delayed value

         // integrate signal data over 1/8 symbol (fast average)
         modulation->detectIntegrate += signalData; // add new value
         modulation->detectIntegrate -= detectData; // remove delayed value

         // signal edge detector
         float edgeDetector = (modulation->filterIntegrate / bitrate->period4SymbolSamples) - (modulation->detectIntegrate / bitrate->period8SymbolSamples);

         // signal modulation deep
         float modulationDeep = (self->decoder->signalStatus.powerAverage - signalData) / self->decoder->signalStatus.powerAverage;

#ifdef DEBUG_ASK_INTEGRATION_CHANNEL
         self->decoder->debug->set(DEBUG_ASK_INTEGRATION_CHANNEL, modulation->filterIntegrate / bitrate->period4SymbolSamples);
#endif

#ifdef DEBUG_ASK_DETECTION_CHANNEL
         self->decoder->debug->set(DEBUG_ASK_DETECTION_CHANNEL, modulationDeep);
#endif

#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
         self->decoder->debug->set(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, edgeDetector);
#endif

         // reset modulation if exceed limits or no detect second falling edge
         if (modulationDeep > self->maximumModulationThreshold)
         {
            modulation->searchStage = SOF_START_EDGE;
            modulation->searchStartTime = 0;
            modulation->searchEndTime = 0;
            modulation->detectorPeek = 0;

            return false;
         }

         // search for first falling edge
         switch (modulation->searchStage)
         {
            case SOF_START_EDGE:

               if (modulationDeep > self->minimumModulationThreshold)
               {
                  // detect edge at maximum peak
                  if (modulation->detectorPeek < edgeDetector && edgeDetector > 0.001)
                  {
                     modulation->detectorPeek = edgeDetector;
                     modulation->searchPeakTime = self->decoder->signalClock;
                     modulation->searchEndTime = self->decoder->signalClock + bitrate->period2SymbolSamples;
                  }

                  // first edge detect finished
                  if (self->decoder->signalClock == modulation->searchEndTime)
                  {
                     // if no edge found, reset modulation
                     if (modulation->searchPeakTime)
                     {
                        modulation->searchStage = SOF_MIDDLE_EDGE;
                        modulation->searchStartTime = modulation->searchPeakTime + (10 * bitrate->period1SymbolSamples) - bitrate->period2SymbolSamples; // search falling edge up to 11 etu
                        modulation->searchEndTime = modulation->searchPeakTime + (11 * bitrate->period1SymbolSamples) + bitrate->period2SymbolSamples; // search falling edge up to 11 etu
                        modulation->detectorPeek = 0;
                     }
                     else
                     {
                        modulation->searchStartTime = 0;
                        modulation->searchEndTime = 0;
                     }
                  }
               }

               break;

            case SOF_MIDDLE_EDGE:

               // rising edge must be between 10 and 11 etus
               if (self->decoder->signalClock > modulation->searchStartTime && self->decoder->signalClock <= modulation->searchEndTime)
               {
                  // detect edge at maximum peak
                  if (modulation->detectorPeek > edgeDetector && edgeDetector < -0.001)
                  {
                     modulation->detectorPeek = edgeDetector;
                     modulation->searchPeakTime = self->decoder->signalClock;
                     modulation->searchEndTime = self->decoder->signalClock + bitrate->period2SymbolSamples;
                  }

                  // first edge detect finished
                  if (self->decoder->signalClock == modulation->searchEndTime)
                  {
                     // if no edge found, reset modulation
                     if (modulation->searchPeakTime)
                     {
                        modulation->searchStage = SOF_END_EDGE;
                        modulation->searchStartTime = modulation->searchPeakTime + (2 * bitrate->period1SymbolSamples) - bitrate->period2SymbolSamples; // search falling edge up to 11 etu
                        modulation->searchEndTime = modulation->searchPeakTime + (3 * bitrate->period1SymbolSamples) + bitrate->period2SymbolSamples; // search falling edge up to 11 etu
                        modulation->detectorPeek = 0;
                     }
                     else
                     {
                        modulation->searchStage = SOF_START_EDGE;
                        modulation->detectorPeek = 0;
                        modulation->searchStartTime = 0;
                        modulation->searchEndTime = 0;
                        modulation->searchPeakTime = 0;
                     }
                  }
               }

                  // during SOF there must not be modulation changes
               else if (fabs(edgeDetector) > 0.001)
               {
                  modulation->detectorPeek = 0;
                  modulation->searchStage = 0;
                  modulation->searchStartTime = 0;
                  modulation->searchEndTime = 0;
                  modulation->searchPeakTime = 0;

                  return false;
               }

               break;

            case SOF_END_EDGE:

               // fallig edge must be between 2 and 3 etus
               if (self->decoder->signalClock > modulation->searchStartTime && self->decoder->signalClock <= modulation->searchEndTime)
               {
                  // detect edge at maximum peak
                  if (modulation->detectorPeek < edgeDetector && edgeDetector > 0.001)
                  {
                     modulation->detectorPeek = edgeDetector;
                     modulation->searchPeakTime = self->decoder->signalClock;
                     modulation->searchEndTime = self->decoder->signalClock + bitrate->period2SymbolSamples;
                  }

                  // first edge detect finished
                  if (self->decoder->signalClock == modulation->searchEndTime)
                  {
                     // no edge found! reset modulation
                     if (modulation->searchPeakTime)
                     {
                        // set pattern search window
                        modulation->symbolStartTime = modulation->searchPeakTime;
                        modulation->symbolEndTime = modulation->searchPeakTime + bitrate->period1SymbolSamples;

                        // setup frame info
                        self->frameStatus.frameType = PollFrame;
                        self->frameStatus.symbolRate = bitrate->symbolsPerSecond;
                        self->frameStatus.frameStart = modulation->symbolStartTime - bitrate->symbolDelayDetect;
                        self->frameStatus.frameEnd = 0;

                        // setup symbol info
                        self->symbolStatus.value = 0;
                        self->symbolStatus.start = modulation->symbolStartTime - bitrate->symbolDelayDetect;
                        self->symbolStatus.end = modulation->symbolEndTime - bitrate->symbolDelayDetect;
                        self->symbolStatus.length = self->symbolStatus.end - self->symbolStatus.start;
                        self->symbolStatus.pattern = PatternType::PatternS;

                        // reset modulation to continue search
                        modulation->searchStage = SOF_START_EDGE;
                        modulation->searchStartTime = 0;
                        modulation->searchEndTime = 0;
                        modulation->searchDeepValue = 0;
                        modulation->detectorPeek = 0;

                        // modulation detected
                        self->decoder->bitrate = bitrate;
                        self->decoder->modulation = modulation;

                        return true;
                     }
                     else
                     {
                        modulation->searchStage = SOF_START_EDGE;
                        modulation->detectorPeek = 0;
                        modulation->searchStartTime = 0;
                        modulation->searchEndTime = 0;
                        modulation->searchPeakTime = 0;
                     }
                  }
               }
         }
      }
   }

   return false;
}

void NfcB::decodeFrame(sdr::SignalBuffer &samples, std::list<NfcFrame> &frames)
{
   if (self->frameStatus.frameType == PollFrame)
   {
      decodePollFrame(samples, frames);
   }

   if (self->frameStatus.frameType == ListenFrame)
   {
      decodeListenFrame(samples, frames);
   }
}

bool NfcB::decodePollFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   self->decoder->bitrate = nullptr;
   self->decoder->modulation = nullptr;

   return false;
}

bool NfcB::decodeListenFrame(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   self->decoder->bitrate = nullptr;
   self->decoder->modulation = nullptr;

   return false;
}

int NfcB::decodePollFrameSymbolAsk(sdr::SignalBuffer &buffer)
{
   return 0;
}

int NfcB::decodeListenFrameSymbolBpsk(sdr::SignalBuffer &buffer)
{
   return 0;
}

/*
 * Reset modulation status
 */
void NfcB::resetModulation()
{
   // reset modulation detection for all rates
   for (int rate = r106k; rate <= r424k; rate++)
   {
      self->modulationStatus[rate].searchStartTime = 0;
      self->modulationStatus[rate].searchEndTime = 0;
      self->modulationStatus[rate].correlationPeek = 0;
      self->modulationStatus[rate].searchPulseWidth = 0;
      self->modulationStatus[rate].searchDeepValue = 0;
      self->modulationStatus[rate].symbolAverage = 0;
      self->modulationStatus[rate].symbolPhase = NAN;
   }

   // clear stream status
   self->streamStatus = {0,};

   // clear stream status
   self->symbolStatus = {0,};

   // clear frame status
   self->frameStatus.frameType = 0;
   self->frameStatus.frameStart = 0;
   self->frameStatus.frameEnd = 0;

   // restore bitrate
   self->decoder->bitrate = nullptr;

   // restore modulation
   self->decoder->modulation = nullptr;
}

}

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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <cmath>
#include <chrono>
#include <functional>

#include <rt/Logger.h>
#include <sdr/RecordDevice.h>

#include <nfc/Nfc.h>
#include <nfc/NfcA.h>
#include <nfc/NfcB.h>

#include <nfc/NfcDecoder.h>

#include "NfcSignal.h"

//#define DEBUG_SIGNAL

#ifdef DEBUG_SIGNAL
#define DEBUG_CHANNELS 8

#define DEBUG_SIGNAL_VALUE_CHANNEL 0
#define DEBUG_SIGNAL_POWER_CHANNEL 1
#define DEBUG_SIGNAL_AVERAGE_CHANNEL 2
#define DEBUG_SIGNAL_VARIANCE_CHANNEL 3
#define DEBUG_SIGNAL_EDGE_CHANNEL 4

#define DEBUG_ASK_CORRELATION_CHANNEL 5
#define DEBUG_ASK_INTEGRATION_CHANNEL 6
#define DEBUG_ASK_SYNCHRONIZATION_CHANNEL 7

#define DEBUG_BPSK_PHASE_INTEGRATION_CHANNEL 5
#define DEBUG_BPSK_PHASE_DEMODULATION_CHANNEL 4
#define DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL 7
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

enum FrameCommand
{
   NFCA_REQA = 0x26,
   NFCA_HLTA = 0x50,
   NFCA_WUPA = 0x52,
   NFCA_AUTH1 = 0x60,
   NFCA_AUTH2 = 0x61,
   NFCA_SEL1 = 0x93,
   NFCA_SEL2 = 0x95,
   NFCA_SEL3 = 0x97,
   NFCA_RATS = 0xE0,
   NFCA_PPS = 0xD0,
   NFCA_IBLOCK = 0x02,
   NFCA_RBLOCK = 0xA2,
   NFCA_SBLOCK = 0xC2
};

// FSDI to FSD conversion (frame size)
static const int TABLE_FDS[] = {16, 24, 32, 40, 48, 64, 96, 128, 256, 0, 0, 0, 0, 0, 0, 256};



/*
 * Signal debugger
 */
#ifdef DEBUG_SIGNAL
struct DecoderDebug
{
   unsigned int channels;
   unsigned int clock;

   sdr::RecordDevice *recorder;
   sdr::SignalBuffer buffer;

   float values[10] {0,};

   DecoderDebug(unsigned int channels, unsigned int decoder.sampleRate) : channels(channels), clock(0)
   {
      char file[128];
      struct tm timeinfo {};

      std::time_t rawTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      localtime_s(&timeinfo, &rawTime);
      strftime(file, sizeof(file), "decoder-%Y%m%d%H%M%S.wav", &timeinfo);

      recorder = new sdr::RecordDevice(file);
      recorder->setChannelCount(channels);
      recorder->setSampleRate(decoder.sampleRate);
      recorder->open(sdr::RecordDevice::Write);
   }

   ~DecoderDebug()
   {
      delete recorder;
   }

   void block(unsigned int time)
   {
      if (clock != time)
      {
         // store sample buffer
         buffer.put(values, recorder->channelCount());

         // clear sample buffer
         for (auto &f : values)
         {
            f = 0;
         }

         clock = time;
      }
   }

   void value(int channel, float value)
   {
      if (channel >= 0 && channel < recorder->channelCount())
      {
         values[channel] = value;
      }
   }

   void begin(int sampleCount)
   {
      buffer = sdr::SignalBuffer(sampleCount * recorder->channelCount(), recorder->channelCount(), recorder->decoder.sampleRate());
   }

   void commit()
   {
      buffer.flip();

      recorder->write(buffer);
   }
};
#endif

struct NfcDecoder::Impl
{
   // frame handler
   typedef std::function<bool(nfc::NfcFrame &frame)> FrameHandler;

   rt::Logger log {"NfcDecoder"};

   // NFC-A Decoder
   struct NfcA nfca;

   // NFC-B Decoder
   struct NfcB nfcb;

   // global decoder status
   struct DecoderStatus decoder;

   // decoder signal debugging
#ifdef DEBUG_SIGNAL
   std::shared_ptr<DecoderDebug> decoderDebug;
#endif

   // frame handlers
//   std::vector<FrameHandler> frameHandlers;

   Impl();

   inline void configure(long sampleRate);

   inline std::list<NfcFrame> nextFrames(sdr::SignalBuffer &samples);

   inline bool detectModulation(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames);

   inline bool decodeFrameDevNfcA(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames);

   inline bool decodeFrameTagNfcA(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames);

   inline int decodeSymbolDevAskNfcA(sdr::SignalBuffer &buffer);

   inline int decodeSymbolTagAskNfcA(sdr::SignalBuffer &buffer);

   inline int decodeSymbolTagBpskNfcA(sdr::SignalBuffer &buffer);

   inline bool decodeFrameDevNfcB(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames);

   inline bool decodeFrameTagNfcB(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames);

   inline int decodeSymbolTagAskNfcB(sdr::SignalBuffer &buffer);

   inline int decodeSymbolTagBpskNfcB(sdr::SignalBuffer &buffer);

   inline void resetFrameSearch();

   inline void resetModulation();

   inline bool nextSample(sdr::SignalBuffer &buffer);

   inline void process(NfcFrame frame);

   inline bool processREQA(NfcFrame &frame);

   inline bool processHLTA(NfcFrame &frame);

   inline bool processSELn(NfcFrame &frame);

   inline bool processRATS(NfcFrame &frame);

   inline bool processPPSr(NfcFrame &frame);

   inline bool processAUTH(NfcFrame &frame);

   inline bool processIBlock(NfcFrame &frame);

   inline bool processRBlock(NfcFrame &frame);

   inline bool processSBlock(NfcFrame &frame);

   inline void processOther(NfcFrame &frame);

   inline static bool checkCrc(NfcFrame &frame);

   inline static bool checkParity(unsigned int value, unsigned int parity);
};

NfcDecoder::NfcDecoder() : impl(std::make_shared<Impl>())
{
}

std::list<NfcFrame> NfcDecoder::nextFrames(sdr::SignalBuffer samples)
{
   return impl->nextFrames(samples);
}

void NfcDecoder::setSampleRate(long sampleRate)
{
   impl->configure(sampleRate);
}

void NfcDecoder::setPowerLevelThreshold(float value)
{
   impl->decoder.powerLevelThreshold = value;
}

float NfcDecoder::powerLevelThreshold() const
{
   return impl->decoder.powerLevelThreshold;
}

void NfcDecoder::setModulationThreshold(float value)
{
   impl->decoder.modulationThreshold = value;
}

float NfcDecoder::modulationThreshold() const
{
   return impl->decoder.modulationThreshold;
}

float NfcDecoder::signalStrength() const
{
   return impl->decoder.signalStatus.powerAverage;
}

NfcDecoder::Impl::Impl()
{
}

/**
 * Configure samplerate
 */
void NfcDecoder::Impl::configure(long newSampleRate)
{
   // clear signal parameters
   decoder.signalParams = {0,};

   // clear signal processing status
   decoder.signalStatus = {0,};

   // clear detected symbol status
   decoder.symbolStatus = {0,};

   // clear bit stream status
   decoder.streamStatus = {0,};

   // clear frame processing status
   decoder.frameStatus = {0,};

   // set decoder samplerate
   decoder.sampleRate = newSampleRate;

   // clear signal master clock
   decoder.signalClock = 0;

   // clear last detected frame end
   decoder.lastFrameEnd = 0;

   // clear chained flags
   decoder.chainedFlags = 0;

   if (decoder.sampleRate > 0)
   {
      // calculate sample time unit, (equivalent to 1/fc in ISO/IEC 14443-3 specifications)
      decoder.signalParams.sampleTimeUnit = double(decoder.sampleRate) / double(BaseFrequency);

      log.info("--------------------------------------------");
      log.info("initializing NFC decoder");
      log.info("--------------------------------------------");
      log.info("\tsignalSampleRate     {}", {decoder.sampleRate});
      log.info("\tdecoder.powerLevelThreshold  {}", {decoder.powerLevelThreshold});
      log.info("\tdecoder.modulationThreshold  {}", {decoder.modulationThreshold});

      // compute symbol parameters for 106Kbps, 212Kbps, 424Kbps and 848Kbps
      for (int rate = r106k; rate <= r424k; rate++)
      {
         // clear decoder.bitrate parameters
         decoder.bitrateParams[rate] = {0,};

         // clear decoder.modulation parameters
         decoder.modulationStatus[rate] = {0,};

         // configuracion to current decoder.bitrate
         decoder.bitrate = decoder.bitrateParams + rate;

         // set tech type and rate
         decoder.bitrate->techType = TechType::NfcA;
         decoder.bitrate->rateType = rate;

         // symbol timing parameters
         decoder.bitrate->symbolsPerSecond = BaseFrequency / (128 >> rate);

         // number of samples per symbol
         decoder.bitrate->period1SymbolSamples = int(round(decoder.signalParams.sampleTimeUnit * (128 >> rate))); // full symbol samples
         decoder.bitrate->period2SymbolSamples = int(round(decoder.signalParams.sampleTimeUnit * (64 >> rate))); // half symbol samples
         decoder.bitrate->period4SymbolSamples = int(round(decoder.signalParams.sampleTimeUnit * (32 >> rate))); // quarter of symbol...
         decoder.bitrate->period8SymbolSamples = int(round(decoder.signalParams.sampleTimeUnit * (16 >> rate))); // and so on...

         // delay guard for each symbol rate
         decoder.bitrate->symbolDelayDetect = rate > r106k ? decoder.bitrateParams[rate - 1].symbolDelayDetect + decoder.bitrateParams[rate - 1].period1SymbolSamples : 0;

         // moving average offsets
         decoder.bitrate->offsetSignalIndex = SignalBufferLength - decoder.bitrate->symbolDelayDetect;
         decoder.bitrate->offsetFilterIndex = SignalBufferLength - decoder.bitrate->symbolDelayDetect - decoder.bitrate->period2SymbolSamples;
         decoder.bitrate->offsetSymbolIndex = SignalBufferLength - decoder.bitrate->symbolDelayDetect - decoder.bitrate->period1SymbolSamples;
         decoder.bitrate->offsetDetectIndex = SignalBufferLength - decoder.bitrate->symbolDelayDetect - decoder.bitrate->period4SymbolSamples;

         // exponential symbol average
         decoder.bitrate->symbolAverageW0 = float(1 - 5.0 / decoder.bitrate->period1SymbolSamples);
         decoder.bitrate->symbolAverageW1 = float(1 - decoder.bitrate->symbolAverageW0);

         log.info("{} kpbs parameters:", {round(decoder.bitrate->symbolsPerSecond / 1E3)});
         log.info("\tsymbolsPerSecond     {}", {decoder.bitrate->symbolsPerSecond});
         log.info("\tperiod1SymbolSamples {} ({} us)", {decoder.bitrate->period1SymbolSamples, 1E6 * decoder.bitrate->period1SymbolSamples / decoder.sampleRate});
         log.info("\tperiod2SymbolSamples {} ({} us)", {decoder.bitrate->period2SymbolSamples, 1E6 * decoder.bitrate->period2SymbolSamples / decoder.sampleRate});
         log.info("\tperiod4SymbolSamples {} ({} us)", {decoder.bitrate->period4SymbolSamples, 1E6 * decoder.bitrate->period4SymbolSamples / decoder.sampleRate});
         log.info("\tperiod8SymbolSamples {} ({} us)", {decoder.bitrate->period8SymbolSamples, 1E6 * decoder.bitrate->period8SymbolSamples / decoder.sampleRate});
         log.info("\tsymbolDelayDetect    {} ({} us)", {decoder.bitrate->symbolDelayDetect, 1E6 * decoder.bitrate->symbolDelayDetect / decoder.sampleRate});
         log.info("\toffsetSignalIndex    {}", {decoder.bitrate->offsetSignalIndex});
         log.info("\toffsetFilterIndex    {}", {decoder.bitrate->offsetFilterIndex});
         log.info("\toffsetSymbolIndex    {}", {decoder.bitrate->offsetSymbolIndex});
         log.info("\toffsetDetectIndex    {}", {decoder.bitrate->offsetDetectIndex});
      }

      // initialize default protocol parameters for start decoding
      decoder.protocolStatus.maxFrameSize = 256;
      decoder.protocolStatus.startUpGuardTime = int(decoder.signalParams.sampleTimeUnit * 256 * 16 * (1 << 0));
      decoder.protocolStatus.frameWaitingTime = int(decoder.signalParams.sampleTimeUnit * 256 * 16 * (1 << 4));
      decoder.protocolStatus.frameGuardTime = int(decoder.signalParams.sampleTimeUnit * 128 * 7);
      decoder.protocolStatus.requestGuardTime = int(decoder.signalParams.sampleTimeUnit * 7000);

      // initialize frame parameters to default protocol parameters
      decoder.frameStatus.startUpGuardTime = decoder.protocolStatus.startUpGuardTime;
      decoder.frameStatus.frameWaitingTime = decoder.protocolStatus.frameWaitingTime;
      decoder.frameStatus.frameGuardTime = decoder.protocolStatus.frameGuardTime;
      decoder.frameStatus.requestGuardTime = decoder.protocolStatus.requestGuardTime;

      // initialize exponential average factors for power value
      decoder.signalParams.powerAverageW0 = float(1 - 1E3 / decoder.sampleRate);
      decoder.signalParams.powerAverageW1 = float(1 - decoder.signalParams.powerAverageW0);

      // initialize exponential average factors for signal average
      decoder.signalParams.signalAverageW0 = float(1 - 1E5 / decoder.sampleRate);
      decoder.signalParams.signalAverageW1 = float(1 - decoder.signalParams.signalAverageW0);

      // initialize exponential average factors for signal variance
      decoder.signalParams.signalVarianceW0 = float(1 - 1E5 / decoder.sampleRate);
      decoder.signalParams.signalVarianceW1 = float(1 - decoder.signalParams.signalVarianceW0);

      // starts width no decoder.modulation
      decoder.modulation = nullptr;

      log.info("Startup parameters");
      log.info("\tmaxFrameSize {} bytes", {decoder.protocolStatus.maxFrameSize});
      log.info("\tframeGuardTime {} samples ({} us)", {decoder.protocolStatus.frameGuardTime, 1000000.0 * decoder.protocolStatus.frameGuardTime / decoder.sampleRate});
      log.info("\tframeWaitingTime {} samples ({} us)", {decoder.protocolStatus.frameWaitingTime, 1000000.0 * decoder.protocolStatus.frameWaitingTime / decoder.sampleRate});
      log.info("\trequestGuardTime {} samples ({} us)", {decoder.protocolStatus.requestGuardTime, 1000000.0 * decoder.protocolStatus.requestGuardTime / decoder.sampleRate});
   }

#ifdef DEBUG_SIGNAL
   log.warn("DECODER DEBUGGER ENABLED!, performance may be impacted");
   decoderDebug = std::make_shared<DecoderDebug>(DEBUG_CHANNELS, decoder.sampleRate);
#endif
}

/**
 * Extract next frames
 */
std::list<NfcFrame> NfcDecoder::Impl::nextFrames(sdr::SignalBuffer &samples)
{
   // detected frames
   std::list<NfcFrame> frames;

   // only process valid sample buffer
   if (samples.isValid())
   {
      // re-configure decoder parameters on sample rate changes
      if (decoder.sampleRate != samples.sampleRate())
      {
         configure(samples.sampleRate());
      }

#ifdef DEBUG_SIGNAL
      decoderDebug->begin(samples.elements());
#endif

      while (!samples.isEmpty())
      {
         if (!decoder.modulation)
         {
            if (!detectModulation(samples, frames))
            {
               break;
            }
         }

         if (decoder.bitrate->techType == TechType::NfcA)
         {
            if (decoder.frameStatus.frameType == PollFrame)
            {
               decodeFrameDevNfcA(samples, frames);
            }

            if (decoder.frameStatus.frameType == ListenFrame)
            {
               decodeFrameTagNfcA(samples, frames);
            }
         }
         else if (decoder.bitrate->techType == TechType::NfcB)
         {
            if (decoder.frameStatus.frameType == PollFrame)
            {
               decodeFrameDevNfcB(samples, frames);
            }

            if (decoder.frameStatus.frameType == ListenFrame)
            {
               decodeFrameTagNfcB(samples, frames);
            }
         }
      }

#ifdef DEBUG_SIGNAL
      decoderDebug->commit();
#endif
   }

      // if sample buffer is not valid only process remain carrier detector
   else
   {
      if (decoder.signalStatus.carrierOff)
      {
//         log.debug("detected carrier lost from {} to {}", {decoder.signalStatus.carrierOff, decoder.signalClock});

         NfcFrame silence = NfcFrame(TechType::None, FrameType::NoCarrier);

         silence.setFramePhase(FramePhase::CarrierFrame);
         silence.setSampleStart(decoder.signalStatus.carrierOff);
         silence.setSampleEnd(decoder.signalClock);
         silence.setTimeStart(double(decoder.signalStatus.carrierOff) / double(decoder.sampleRate));
         silence.setTimeEnd(double(decoder.signalClock) / double(decoder.sampleRate));

         frames.push_back(silence);
      }

      else if (decoder.signalStatus.carrierOn)
      {
//         log.debug("detected carrier present from {} to {}", {decoder.signalStatus.carrierOn, decoder.signalClock});

         NfcFrame carrier = NfcFrame(TechType::None, FrameType::EmptyFrame);

         carrier.setFramePhase(FramePhase::CarrierFrame);
         carrier.setSampleStart(decoder.signalStatus.carrierOn);
         carrier.setSampleEnd(decoder.signalClock);
         carrier.setTimeStart(double(decoder.signalStatus.carrierOn) / double(decoder.sampleRate));
         carrier.setTimeEnd(double(decoder.signalClock) / double(decoder.sampleRate));

         frames.push_back(carrier);
      }
   }

   // return frame list
   return frames;
}

/*
 * Search for NFC-A modulated signal
 */
bool NfcDecoder::Impl::detectModulation(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   decoder.symbolStatus.pattern = PatternType::Invalid;

   while (nextSample(buffer) && decoder.symbolStatus.pattern == PatternType::Invalid)
   {
      // ignore low power signals
      if (decoder.signalStatus.powerAverage > decoder.powerLevelThreshold)
      {
         // POLL frame ASK detector for  106Kbps, 212Kbps and 424Kbps
         for (int rate = r106k; rate <= r424k; rate++)
         {
            decoder.bitrate = decoder.bitrateParams + rate;
            decoder.modulation = decoder.modulationStatus + rate;

            // compute signal pointers
            decoder.modulation->signalIndex = (decoder.bitrate->offsetSignalIndex + decoder.signalClock);
            decoder.modulation->filterIndex = (decoder.bitrate->offsetFilterIndex + decoder.signalClock);

            // get signal samples
            float currentData = decoder.signalStatus.signalData[decoder.modulation->signalIndex & (SignalBufferLength - 1)];
            float delayedData = decoder.signalStatus.signalData[decoder.modulation->filterIndex & (SignalBufferLength - 1)];

            // integrate signal data over 1/2 symbol
            decoder.modulation->filterIntegrate += currentData; // add new value
            decoder.modulation->filterIntegrate -= delayedData; // remove delayed value

            // correlation points
            decoder.modulation->filterPoint1 = (decoder.modulation->signalIndex % decoder.bitrate->period1SymbolSamples);
            decoder.modulation->filterPoint2 = (decoder.modulation->signalIndex + decoder.bitrate->period2SymbolSamples) % decoder.bitrate->period1SymbolSamples;
            decoder.modulation->filterPoint3 = (decoder.modulation->signalIndex + decoder.bitrate->period1SymbolSamples - 1) % decoder.bitrate->period1SymbolSamples;

            // store integrated signal in correlation buffer
            decoder.modulation->correlationData[decoder.modulation->filterPoint1] = decoder.modulation->filterIntegrate;

            // compute correlation factors
            decoder.modulation->correlatedS0 = decoder.modulation->correlationData[decoder.modulation->filterPoint1] - decoder.modulation->correlationData[decoder.modulation->filterPoint2];
            decoder.modulation->correlatedS1 = decoder.modulation->correlationData[decoder.modulation->filterPoint2] - decoder.modulation->correlationData[decoder.modulation->filterPoint3];
            decoder.modulation->correlatedSD = std::fabs(decoder.modulation->correlatedS0 - decoder.modulation->correlatedS1) / float(decoder.bitrate->period2SymbolSamples);

            // compute symbol average
            decoder.modulation->symbolAverage = decoder.modulation->symbolAverage * decoder.bitrate->symbolAverageW0 + currentData * decoder.bitrate->symbolAverageW1;

#ifdef DEBUG_ASK_CORRELATION_CHANNEL
            decoderDebug->value(DEBUG_ASK_CORRELATION_CHANNEL, decoder.modulation->correlatedSD);
#endif
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
            decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.0f);
#endif
            // search for Pattern-Z in PCD to PICC request
            if (decoder.modulation->correlatedSD > decoder.signalStatus.powerAverage * decoder.modulationThreshold)
            {
               // calculate symbol decoder.modulation deep
               float modulationDeep = (decoder.signalStatus.powerAverage - currentData) / decoder.signalStatus.powerAverage;

               if (decoder.modulation->searchDeepValue < modulationDeep)
                  decoder.modulation->searchDeepValue = modulationDeep;

               // max correlation peak detector
               if (decoder.modulation->correlatedSD > decoder.modulation->correlationPeek)
               {
                  decoder.modulation->searchPulseWidth++;
                  decoder.modulation->searchPeakTime = decoder.signalClock;
                  decoder.modulation->searchEndTime = decoder.signalClock + decoder.bitrate->period4SymbolSamples;
                  decoder.modulation->correlationPeek = decoder.modulation->correlatedSD;
               }
            }

            // Check for SoF symbol
            if (decoder.signalClock == decoder.modulation->searchEndTime)
            {
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
               decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.75f);
#endif
               // check decoder.modulation deep and Pattern-Z, signaling Start Of Frame (PCD->PICC)
               if (decoder.modulation->searchDeepValue > decoder.modulationThreshold)
               {
                  // set lower threshold to detect valid response pattern
                  decoder.modulation->searchThreshold = decoder.signalStatus.powerAverage * decoder.modulationThreshold;

                  // set pattern search window
                  decoder.modulation->symbolStartTime = decoder.modulation->searchPeakTime - decoder.bitrate->period2SymbolSamples;
                  decoder.modulation->symbolEndTime = decoder.modulation->searchPeakTime + decoder.bitrate->period2SymbolSamples;

                  // setup frame info
                  decoder.frameStatus.frameType = PollFrame;
                  decoder.frameStatus.symbolRate = decoder.bitrate->symbolsPerSecond;
                  decoder.frameStatus.frameStart = decoder.modulation->symbolStartTime - decoder.bitrate->symbolDelayDetect;
                  decoder.frameStatus.frameEnd = 0;

                  // setup symbol info
                  decoder.symbolStatus.value = 0;
                  decoder.symbolStatus.start = decoder.modulation->symbolStartTime - decoder.bitrate->symbolDelayDetect;
                  decoder.symbolStatus.end = decoder.modulation->symbolEndTime - decoder.bitrate->symbolDelayDetect;
                  decoder.symbolStatus.length = decoder.symbolStatus.end - decoder.symbolStatus.start;
                  decoder.symbolStatus.pattern = PatternType::PatternZ;

                  break;
               }

               // reset decoder.modulation to continue search
               decoder.modulation->searchStartTime = 0;
               decoder.modulation->searchEndTime = 0;
               decoder.modulation->searchDeepValue = 0;
               decoder.modulation->correlationPeek = 0;
            }
         }
      }

      // carrier edge detector
      float edge = std::fabs(decoder.signalStatus.signalAverage - decoder.signalStatus.powerAverage);

      // positive edge
      if (decoder.signalStatus.signalAverage > edge && decoder.signalStatus.powerAverage > decoder.powerLevelThreshold)
      {
         if (!decoder.signalStatus.carrierOn)
         {
            decoder.signalStatus.carrierOn = decoder.signalClock;

            if (decoder.signalStatus.carrierOff)
            {
               NfcFrame silence = NfcFrame(TechType::None, FrameType::NoCarrier);

               silence.setFramePhase(FramePhase::CarrierFrame);
               silence.setSampleStart(decoder.signalStatus.carrierOff);
               silence.setSampleEnd(decoder.signalStatus.carrierOn);
               silence.setTimeStart(double(decoder.signalStatus.carrierOff) / double(decoder.sampleRate));
               silence.setTimeEnd(double(decoder.signalStatus.carrierOn) / double(decoder.sampleRate));

               frames.push_back(silence);
            }

            decoder.signalStatus.carrierOff = 0;
         }
      }

         // negative edge
      else if (decoder.signalStatus.signalAverage < edge || decoder.signalStatus.powerAverage < decoder.powerLevelThreshold)
      {
         if (!decoder.signalStatus.carrierOff)
         {
            decoder.signalStatus.carrierOff = decoder.signalClock;

            if (decoder.signalStatus.carrierOn)
            {
               NfcFrame carrier = NfcFrame(TechType::None, FrameType::EmptyFrame);

               carrier.setFramePhase(FramePhase::CarrierFrame);
               carrier.setSampleStart(decoder.signalStatus.carrierOn);
               carrier.setSampleEnd(decoder.signalStatus.carrierOff);
               carrier.setTimeStart(double(decoder.signalStatus.carrierOn) / double(decoder.sampleRate));
               carrier.setTimeEnd(double(decoder.signalStatus.carrierOff) / double(decoder.sampleRate));

               frames.push_back(carrier);
            }

            decoder.signalStatus.carrierOn = 0;
         }
      }
   }

   if (decoder.symbolStatus.pattern != PatternType::Invalid)
   {
      // reset decoder.modulation to continue search
      decoder.modulation->searchStartTime = 0;
      decoder.modulation->searchEndTime = 0;
      decoder.modulation->searchDeepValue = 0;
      decoder.modulation->correlationPeek = 0;

      // decoder.modulation detected
      return true;
   }

   // no decoder.bitrate detected
   decoder.bitrate = nullptr;

   // no decoder.modulation detected
   decoder.modulation = nullptr;

   return false;
}

bool NfcDecoder::Impl::decodeFrameDevNfcA(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   int pattern;

   // read NFC-A request request
   while ((pattern = decodeSymbolDevAskNfcA(buffer)) > PatternType::NoPattern)
   {
      decoder.streamStatus.pattern = pattern;

      // detect end of request (Pattern-Y after Pattern-Z)
      if ((decoder.streamStatus.pattern == PatternType::PatternY && (decoder.streamStatus.previous == PatternType::PatternY || decoder.streamStatus.previous == PatternType::PatternZ)) || decoder.streamStatus.bytes == decoder.protocolStatus.maxFrameSize)
      {
         // frames must contains at least one full byte or 7 bits for short frames
         if (decoder.streamStatus.bytes > 0 || decoder.streamStatus.bits == 7)
         {
            // add remaining byte to request
            if (decoder.streamStatus.bits >= 7)
               decoder.streamStatus.buffer[decoder.streamStatus.bytes++] = decoder.streamStatus.data;

            // set last symbol timing
            if (decoder.streamStatus.previous == PatternType::PatternZ)
               decoder.frameStatus.frameEnd = decoder.symbolStatus.start - decoder.bitrate->period2SymbolSamples;
            else
               decoder.frameStatus.frameEnd = decoder.symbolStatus.start - decoder.bitrate->period1SymbolSamples;

            // build request frame
            NfcFrame request = NfcFrame(TechType::NfcA, FrameType::PollFrame);

            request.setFrameRate(decoder.frameStatus.symbolRate);
            request.setSampleStart(decoder.frameStatus.frameStart);
            request.setSampleEnd(decoder.frameStatus.frameEnd);
            request.setTimeStart(double(decoder.frameStatus.frameStart) / double(decoder.sampleRate));
            request.setTimeEnd(double(decoder.frameStatus.frameEnd) / double(decoder.sampleRate));

            if (decoder.streamStatus.flags & ParityError)
               request.setFrameFlags(FrameFlags::ParityError);

            if (decoder.streamStatus.bytes == decoder.protocolStatus.maxFrameSize)
               request.setFrameFlags(FrameFlags::Truncated);

            if (decoder.streamStatus.bytes == 1 && decoder.streamStatus.bits == 7)
               request.setFrameFlags(FrameFlags::ShortFrame);

            // add bytes to frame and flip to prepare read
            request.put(decoder.streamStatus.buffer, decoder.streamStatus.bytes).flip();

            // clear decoder.modulation status for next frame search
            decoder.modulation->symbolStartTime = 0;
            decoder.modulation->symbolEndTime = 0;
            decoder.modulation->filterIntegrate = 0;
            decoder.modulation->phaseIntegrate = 0;

            // clear stream status
            decoder.streamStatus = {0,};

            // process frame
            process(request);

            // add to frame list
            frames.push_back(request);

            // return request frame data
            return true;
         }

         // reset decoder.modulation and restart frame detection
         resetModulation();

         // no valid frame found
         return false;
      }

      if (decoder.streamStatus.previous)
      {
         int value = (decoder.streamStatus.previous == PatternType::PatternX);

         // decode next bit
         if (decoder.streamStatus.bits < 8)
         {
            decoder.streamStatus.data = decoder.streamStatus.data | (value << decoder.streamStatus.bits++);
         }

            // store full byte in stream buffer and check parity
         else if (decoder.streamStatus.bytes < decoder.protocolStatus.maxFrameSize)
         {
            decoder.streamStatus.buffer[decoder.streamStatus.bytes++] = decoder.streamStatus.data;
            decoder.streamStatus.flags |= !checkParity(decoder.streamStatus.data, value) ? ParityError : 0;
            decoder.streamStatus.data = decoder.streamStatus.bits = 0;
         }

            // too many bytes in frame, abort decoder
         else
         {
            // reset decoder.modulation status
            resetModulation();

            // no valid frame found
            return false;
         }
      }

      // update previous command state
      decoder.streamStatus.previous = decoder.streamStatus.pattern;
   }

   // no frame detected
   return false;
}

bool NfcDecoder::Impl::decodeFrameTagNfcA(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   int pattern;

   // decode TAG ASK response
   if (decoder.bitrate->rateType == r106k)
   {
      if (!decoder.frameStatus.frameStart)
      {
         // search Start Of Frame pattern
         pattern = decodeSymbolTagAskNfcA(buffer);

         // Pattern-D found, mark frame start time
         if (pattern == PatternType::PatternD)
         {
            decoder.frameStatus.frameStart = decoder.symbolStatus.start;
         }
         else
         {
            //  end of frame waiting time, restart decoder.modulation search
            if (pattern == PatternType::NoPattern)
               resetModulation();

            // no frame found
            return NfcFrame::Nil;
         }
      }

      if (decoder.frameStatus.frameStart)
      {
         // decode remaining response
         while ((pattern = decodeSymbolTagAskNfcA(buffer)) > PatternType::NoPattern)
         {
            // detect end of response for ASK
            if (pattern == PatternType::PatternF || decoder.streamStatus.bytes == decoder.protocolStatus.maxFrameSize)
            {
               // a valid response must contains at least 4 bits of data
               if (decoder.streamStatus.bytes > 0 || decoder.streamStatus.bits == 4)
               {
                  // add remaining byte to request
                  if (decoder.streamStatus.bits == 4)
                     decoder.streamStatus.buffer[decoder.streamStatus.bytes++] = decoder.streamStatus.data;

                  decoder.frameStatus.frameEnd = decoder.symbolStatus.end;

                  NfcFrame response = NfcFrame(TechType::NfcA, FrameType::ListenFrame);

                  response.setFrameRate(decoder.bitrate->symbolsPerSecond);
                  response.setSampleStart(decoder.frameStatus.frameStart);
                  response.setSampleEnd(decoder.frameStatus.frameEnd);
                  response.setTimeStart(double(decoder.frameStatus.frameStart) / double(decoder.sampleRate));
                  response.setTimeEnd(double(decoder.frameStatus.frameEnd) / double(decoder.sampleRate));

                  if (decoder.streamStatus.flags & ParityError)
                     response.setFrameFlags(FrameFlags::ParityError);

                  if (decoder.streamStatus.bytes == decoder.protocolStatus.maxFrameSize)
                     response.setFrameFlags(FrameFlags::Truncated);

                  if (decoder.streamStatus.bytes == 1 && decoder.streamStatus.bits == 4)
                     response.setFrameFlags(FrameFlags::ShortFrame);

                  // add bytes to frame and flip to prepare read
                  response.put(decoder.streamStatus.buffer, decoder.streamStatus.bytes).flip();

                  // reset decoder.modulation status
                  resetModulation();

                  // process frame
                  process(response);

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
            if (decoder.streamStatus.bits < 8)
            {
               decoder.streamStatus.data |= (decoder.symbolStatus.value << decoder.streamStatus.bits++);
            }

               // store full byte in stream buffer and check parity
            else if (decoder.streamStatus.bytes < decoder.protocolStatus.maxFrameSize)
            {
               decoder.streamStatus.buffer[decoder.streamStatus.bytes++] = decoder.streamStatus.data;
               decoder.streamStatus.flags |= !checkParity(decoder.streamStatus.data, decoder.symbolStatus.value) ? ParityError : 0;
               decoder.streamStatus.data = decoder.streamStatus.bits = 0;
            }

               // too many bytes in frame, abort decoder
            else
            {
               // reset decoder.modulation status
               resetModulation();

               // no valid frame found
               return false;
            }
         }
      }
   }

      // decode TAG BPSK response
   else if (decoder.bitrate->rateType == r212k || decoder.bitrate->rateType == r424k)
   {
      if (!decoder.frameStatus.frameStart)
      {
         // detect first pattern
         pattern = decodeSymbolTagBpskNfcA(buffer);

         // Pattern-M found, mark frame start time
         if (pattern == PatternType::PatternM)
         {
            decoder.frameStatus.frameStart = decoder.symbolStatus.start;
         }
         else
         {
            //  end of frame waiting time, restart decoder.modulation search
            if (pattern == PatternType::NoPattern)
               resetModulation();

            // no frame found
            return false;
         }
      }

      // frame SoF detected, decode frame stream...
      if (decoder.frameStatus.frameStart)
      {
         while ((pattern = decodeSymbolTagBpskNfcA(buffer)) > PatternType::NoPattern)
         {
            // detect end of response for BPSK
            if (pattern == PatternType::PatternO)
            {
               if (decoder.streamStatus.bits == 9)
               {
                  // store byte in stream buffer
                  decoder.streamStatus.buffer[decoder.streamStatus.bytes++] = decoder.streamStatus.data;

                  // last byte has even parity
                  decoder.streamStatus.flags |= checkParity(decoder.streamStatus.data, decoder.streamStatus.parity) ? ParityError : 0;
               }

               // frames must contains at least one full byte
               if (decoder.streamStatus.bytes > 0)
               {
                  // mark frame end at star of EoF symbol
                  decoder.frameStatus.frameEnd = decoder.symbolStatus.start;

                  // build responde frame
                  NfcFrame response = NfcFrame(TechType::NfcA, FrameType::ListenFrame);

                  response.setFrameRate(decoder.bitrate->symbolsPerSecond);
                  response.setSampleStart(decoder.frameStatus.frameStart);
                  response.setSampleEnd(decoder.frameStatus.frameEnd);
                  response.setTimeStart(double(decoder.frameStatus.frameStart) / double(decoder.sampleRate));
                  response.setTimeEnd(double(decoder.frameStatus.frameEnd) / double(decoder.sampleRate));

                  if (decoder.streamStatus.flags & ParityError)
                     response.setFrameFlags(FrameFlags::ParityError);

                  if (decoder.streamStatus.bytes == decoder.protocolStatus.maxFrameSize)
                     response.setFrameFlags(FrameFlags::Truncated);

                  // add bytes to frame and flip to prepare read
                  response.put(decoder.streamStatus.buffer, decoder.streamStatus.bytes).flip();

                  // reset decoder.modulation status
                  resetModulation();

                  // process frame
                  process(response);

                  // add to frame list
                  frames.push_back(response);

                  return true;
               }

               // reset decoder.modulation status
               resetModulation();

               // no valid frame found
               return false;
            }

            // decode next data bit
            if (decoder.streamStatus.bits < 8)
            {
               decoder.streamStatus.data |= (decoder.symbolStatus.value << decoder.streamStatus.bits);
            }

               // decode parity bit
            else if (decoder.streamStatus.bits < 9)
            {
               decoder.streamStatus.parity = decoder.symbolStatus.value;
            }

               // store full byte in stream buffer and check parity
            else if (decoder.streamStatus.bytes < decoder.protocolStatus.maxFrameSize)
            {
               // store byte in stream buffer
               decoder.streamStatus.buffer[decoder.streamStatus.bytes++] = decoder.streamStatus.data;

               // frame bytes has odd parity
               decoder.streamStatus.flags |= !checkParity(decoder.streamStatus.data, decoder.streamStatus.parity) ? ParityError : 0;

               // initialize next value from current symbol
               decoder.streamStatus.data = decoder.symbolStatus.value;

               // reset bit counter
               decoder.streamStatus.bits = 0;
            }

               // too many bytes in frame, abort decoder
            else
            {
               // reset decoder.modulation status
               resetModulation();

               // no valid frame found
               return false;
            }

            decoder.streamStatus.bits++;
         }
      }
   }

   // end of stream...
   return false;
}

int NfcDecoder::Impl::decodeSymbolDevAskNfcA(sdr::SignalBuffer &buffer)
{
   decoder.symbolStatus.pattern = PatternType::Invalid;

   while (nextSample(buffer))
   {
      // compute pointers
      decoder.modulation->signalIndex = (decoder.bitrate->offsetSignalIndex + decoder.signalClock);
      decoder.modulation->filterIndex = (decoder.bitrate->offsetFilterIndex + decoder.signalClock);

      // get signal samples
      float currentData = decoder.signalStatus.signalData[decoder.modulation->signalIndex & (SignalBufferLength - 1)];
      float delayedData = decoder.signalStatus.signalData[decoder.modulation->filterIndex & (SignalBufferLength - 1)];

      // integrate signal data over 1/2 symbol
      decoder.modulation->filterIntegrate += currentData; // add new value
      decoder.modulation->filterIntegrate -= delayedData; // remove delayed value

      // correlation pointers
      decoder.modulation->filterPoint1 = (decoder.modulation->signalIndex % decoder.bitrate->period1SymbolSamples);
      decoder.modulation->filterPoint2 = (decoder.modulation->signalIndex + decoder.bitrate->period2SymbolSamples) % decoder.bitrate->period1SymbolSamples;
      decoder.modulation->filterPoint3 = (decoder.modulation->signalIndex + decoder.bitrate->period1SymbolSamples - 1) % decoder.bitrate->period1SymbolSamples;

      // store integrated signal in correlation buffer
      decoder.modulation->correlationData[decoder.modulation->filterPoint1] = decoder.modulation->filterIntegrate;

      // compute correlation factors
      decoder.modulation->correlatedS0 = decoder.modulation->correlationData[decoder.modulation->filterPoint1] - decoder.modulation->correlationData[decoder.modulation->filterPoint2];
      decoder.modulation->correlatedS1 = decoder.modulation->correlationData[decoder.modulation->filterPoint2] - decoder.modulation->correlationData[decoder.modulation->filterPoint3];
      decoder.modulation->correlatedSD = std::fabs(decoder.modulation->correlatedS0 - decoder.modulation->correlatedS1) / float(decoder.bitrate->period2SymbolSamples);

#ifdef DEBUG_ASK_CORRELATION_CHANNEL
      decoderDebug->value(DEBUG_ASK_CORRELATION_CHANNEL, decoder.modulation->correlatedSD);
#endif

#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
      decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.0f);
#endif
      // compute symbol average
      decoder.modulation->symbolAverage = decoder.modulation->symbolAverage * decoder.bitrate->symbolAverageW0 + currentData * decoder.bitrate->symbolAverageW1;

      // set next search sync window from previous state
      if (!decoder.modulation->searchStartTime)
      {
         // estimated symbol start and end
         decoder.modulation->symbolStartTime = decoder.modulation->symbolEndTime;
         decoder.modulation->symbolEndTime = decoder.modulation->symbolStartTime + decoder.bitrate->period1SymbolSamples;

         // timig search window
         decoder.modulation->searchStartTime = decoder.modulation->symbolEndTime - decoder.bitrate->period8SymbolSamples;
         decoder.modulation->searchEndTime = decoder.modulation->symbolEndTime + decoder.bitrate->period8SymbolSamples;

         // reset symbol parameters
         decoder.modulation->symbolCorr0 = 0;
         decoder.modulation->symbolCorr1 = 0;
      }

      // search max correlation peak
      if (decoder.signalClock >= decoder.modulation->searchStartTime && decoder.signalClock <= decoder.modulation->searchEndTime)
      {
         if (decoder.modulation->correlatedSD > decoder.modulation->correlationPeek)
         {
            decoder.modulation->correlationPeek = decoder.modulation->correlatedSD;
            decoder.modulation->symbolCorr0 = decoder.modulation->correlatedS0;
            decoder.modulation->symbolCorr1 = decoder.modulation->correlatedS1;
            decoder.modulation->symbolEndTime = decoder.signalClock;
         }
      }

      // capture next symbol
      if (decoder.signalClock == decoder.modulation->searchEndTime)
      {
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
         decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.50f);
#endif
         // detect Pattern-Y when no decoder.modulation occurs (below search detection threshold)
         if (decoder.modulation->correlationPeek < decoder.modulation->searchThreshold)
         {
            // estimate symbol end from start (peak detection not valid due lack of decoder.modulation)
            decoder.modulation->symbolEndTime = decoder.modulation->symbolStartTime + decoder.bitrate->period1SymbolSamples;

            // setup symbol info
            decoder.symbolStatus.value = 1;
            decoder.symbolStatus.start = decoder.modulation->symbolStartTime - decoder.bitrate->symbolDelayDetect;
            decoder.symbolStatus.end = decoder.modulation->symbolEndTime - decoder.bitrate->symbolDelayDetect;
            decoder.symbolStatus.length = decoder.symbolStatus.end - decoder.symbolStatus.start;
            decoder.symbolStatus.pattern = PatternType::PatternY;

            break;
         }

         // detect Pattern-Z
         if (decoder.modulation->symbolCorr0 > decoder.modulation->symbolCorr1)
         {
            // setup symbol info
            decoder.symbolStatus.value = 0;
            decoder.symbolStatus.start = decoder.modulation->symbolStartTime - decoder.bitrate->symbolDelayDetect;
            decoder.symbolStatus.end = decoder.modulation->symbolEndTime - decoder.bitrate->symbolDelayDetect;
            decoder.symbolStatus.length = decoder.symbolStatus.end - decoder.symbolStatus.start;
            decoder.symbolStatus.pattern = PatternType::PatternZ;

            break;
         }

         // detect Pattern-X, setup symbol info
         decoder.symbolStatus.value = 1;
         decoder.symbolStatus.start = decoder.modulation->symbolStartTime - decoder.bitrate->symbolDelayDetect;
         decoder.symbolStatus.end = decoder.modulation->symbolEndTime - decoder.bitrate->symbolDelayDetect;
         decoder.symbolStatus.length = decoder.symbolStatus.end - decoder.symbolStatus.start;
         decoder.symbolStatus.pattern = PatternType::PatternX;

         break;
      }
   }

   // reset search status if symbol has detected
   if (decoder.symbolStatus.pattern != PatternType::Invalid)
   {
      decoder.modulation->searchStartTime = 0;
      decoder.modulation->searchEndTime = 0;
      decoder.modulation->searchPulseWidth = 0;
      decoder.modulation->correlationPeek = 0;
      decoder.modulation->correlatedSD = 0;
   }

   return decoder.symbolStatus.pattern;
}

int NfcDecoder::Impl::decodeSymbolTagAskNfcA(sdr::SignalBuffer &buffer)
{
   int pattern = PatternType::Invalid;

   while (nextSample(buffer))
   {
      // compute pointers
      decoder.modulation->signalIndex = (decoder.bitrate->offsetSignalIndex + decoder.signalClock);
      decoder.modulation->detectIndex = (decoder.bitrate->offsetDetectIndex + decoder.signalClock);

      // get signal samples
      float currentData = decoder.signalStatus.signalData[decoder.modulation->signalIndex & (SignalBufferLength - 1)];

      // compute symbol average (signal offset)
      decoder.modulation->symbolAverage = decoder.modulation->symbolAverage * decoder.bitrate->symbolAverageW0 + currentData * decoder.bitrate->symbolAverageW1;

      // signal value
      currentData -= decoder.modulation->symbolAverage;

      // store signal square in filter buffer
      decoder.modulation->integrationData[decoder.modulation->signalIndex & (SignalBufferLength - 1)] = currentData * currentData;

      // start correlation after frameGuardTime
      if (decoder.signalClock > (decoder.frameStatus.guardEnd - decoder.bitrate->period1SymbolSamples))
      {
         // compute correlation points
         decoder.modulation->filterPoint1 = (decoder.modulation->signalIndex % decoder.bitrate->period1SymbolSamples);
         decoder.modulation->filterPoint2 = (decoder.modulation->signalIndex + decoder.bitrate->period2SymbolSamples) % decoder.bitrate->period1SymbolSamples;
         decoder.modulation->filterPoint3 = (decoder.modulation->signalIndex + decoder.bitrate->period1SymbolSamples - 1) % decoder.bitrate->period1SymbolSamples;

         // integrate symbol (moving average)
         decoder.modulation->filterIntegrate += decoder.modulation->integrationData[decoder.modulation->signalIndex & (SignalBufferLength - 1)]; // add new value
         decoder.modulation->filterIntegrate -= decoder.modulation->integrationData[decoder.modulation->detectIndex & (SignalBufferLength - 1)]; // remove delayed value

         // store integrated signal in correlation buffer
         decoder.modulation->correlationData[decoder.modulation->filterPoint1] = decoder.modulation->filterIntegrate;

         // compute correlation results for each symbol and distance
         decoder.modulation->correlatedS0 = decoder.modulation->correlationData[decoder.modulation->filterPoint1] - decoder.modulation->correlationData[decoder.modulation->filterPoint2];
         decoder.modulation->correlatedS1 = decoder.modulation->correlationData[decoder.modulation->filterPoint2] - decoder.modulation->correlationData[decoder.modulation->filterPoint3];
         decoder.modulation->correlatedSD = std::fabs(decoder.modulation->correlatedS0 - decoder.modulation->correlatedS1);
      }

#ifdef DEBUG_ASK_CORRELATION_CHANNEL
      decoderDebug->value(DEBUG_ASK_CORRELATION_CHANNEL, decoder.modulation->correlatedSD);
#endif

#ifdef DEBUG_ASK_INTEGRATION_CHANNEL
      decoderDebug->value(DEBUG_ASK_INTEGRATION_CHANNEL, decoder.modulation->filterIntegrate);
#endif

#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
      decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.0f);
#endif

      // search for Start Of Frame pattern (SoF)
      if (!decoder.modulation->symbolEndTime)
      {
         if (decoder.signalClock > decoder.frameStatus.guardEnd)
         {
            if (decoder.modulation->correlatedSD > decoder.modulation->searchThreshold)
            {
               // max correlation peak detector
               if (decoder.modulation->correlatedSD > decoder.modulation->correlationPeek)
               {
                  decoder.modulation->searchPulseWidth++;
                  decoder.modulation->searchPeakTime = decoder.signalClock;
                  decoder.modulation->searchEndTime = decoder.signalClock + decoder.bitrate->period4SymbolSamples;
                  decoder.modulation->correlationPeek = decoder.modulation->correlatedSD;
               }
            }

            // Check for SoF symbol
            if (decoder.signalClock == decoder.modulation->searchEndTime)
            {
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
               decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.75f);
#endif
               if (decoder.modulation->searchPulseWidth > decoder.bitrate->period8SymbolSamples)
               {
                  // set pattern search window
                  decoder.modulation->symbolStartTime = decoder.modulation->searchPeakTime - decoder.bitrate->period2SymbolSamples;
                  decoder.modulation->symbolEndTime = decoder.modulation->searchPeakTime + decoder.bitrate->period2SymbolSamples;

                  // setup symbol info
                  decoder.symbolStatus.value = 1;
                  decoder.symbolStatus.start = decoder.modulation->symbolStartTime - decoder.bitrate->symbolDelayDetect;
                  decoder.symbolStatus.end = decoder.modulation->symbolEndTime - decoder.bitrate->symbolDelayDetect;
                  decoder.symbolStatus.length = decoder.symbolStatus.end - decoder.symbolStatus.start;

                  pattern = PatternType::PatternD;
                  break;
               }

               // reset search status
               decoder.modulation->searchStartTime = 0;
               decoder.modulation->searchEndTime = 0;
               decoder.modulation->correlationPeek = 0;
               decoder.modulation->searchPulseWidth = 0;
               decoder.modulation->correlatedSD = 0;
            }
         }

         // capture signal variance as lower level threshold
         if (decoder.signalClock == decoder.frameStatus.guardEnd)
            decoder.modulation->searchThreshold = decoder.signalStatus.signalVariance;

         // frame waiting time exceeded
         if (decoder.signalClock == decoder.frameStatus.waitingEnd)
         {
            pattern = PatternType::NoPattern;
            break;
         }
      }

         // search Response Bit Stream
      else
      {
         // set next search sync window from previous
         if (!decoder.modulation->searchStartTime)
         {
            // estimated symbol start and end
            decoder.modulation->symbolStartTime = decoder.modulation->symbolEndTime;
            decoder.modulation->symbolEndTime = decoder.modulation->symbolStartTime + decoder.bitrate->period1SymbolSamples;

            // timig search window
            decoder.modulation->searchStartTime = decoder.modulation->symbolEndTime - decoder.bitrate->period8SymbolSamples;
            decoder.modulation->searchEndTime = decoder.modulation->symbolEndTime + decoder.bitrate->period8SymbolSamples;

            // reset symbol parameters
            decoder.modulation->symbolCorr0 = 0;
            decoder.modulation->symbolCorr1 = 0;
         }

         // search symbol timings
         if (decoder.signalClock >= decoder.modulation->searchStartTime && decoder.signalClock <= decoder.modulation->searchEndTime)
         {
            if (decoder.modulation->correlatedSD > decoder.modulation->correlationPeek)
            {
               decoder.modulation->correlationPeek = decoder.modulation->correlatedSD;
               decoder.modulation->symbolCorr0 = decoder.modulation->correlatedS0;
               decoder.modulation->symbolCorr1 = decoder.modulation->correlatedS1;
               decoder.modulation->symbolEndTime = decoder.signalClock;
            }
         }

         // capture next symbol
         if (decoder.signalClock == decoder.modulation->searchEndTime)
         {
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
            decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.50f);
#endif
            if (decoder.modulation->correlationPeek > decoder.modulation->searchThreshold)
            {
               // setup symbol info
               decoder.symbolStatus.start = decoder.modulation->symbolStartTime - decoder.bitrate->symbolDelayDetect;
               decoder.symbolStatus.end = decoder.modulation->symbolEndTime - decoder.bitrate->symbolDelayDetect;
               decoder.symbolStatus.length = decoder.symbolStatus.end - decoder.symbolStatus.start;

               if (decoder.modulation->symbolCorr0 > decoder.modulation->symbolCorr1)
               {
                  decoder.symbolStatus.value = 0;
                  pattern = PatternType::PatternE;
                  break;
               }

               decoder.symbolStatus.value = 1;
               pattern = PatternType::PatternD;
               break;
            }

            // no decoder.modulation (End Of Frame) EoF
            pattern = PatternType::PatternF;
            break;
         }
      }
   }

   // reset search status
   if (pattern != PatternType::Invalid)
   {
      decoder.symbolStatus.pattern = pattern;

      decoder.modulation->searchStartTime = 0;
      decoder.modulation->searchEndTime = 0;
      decoder.modulation->correlationPeek = 0;
      decoder.modulation->searchPulseWidth = 0;
      decoder.modulation->correlatedSD = 0;
   }

   return pattern;
}

int NfcDecoder::Impl::decodeSymbolTagBpskNfcA(sdr::SignalBuffer &buffer)
{
   int pattern = PatternType::Invalid;

   while (nextSample(buffer))
   {
      decoder.modulation->signalIndex = (decoder.bitrate->offsetSignalIndex + decoder.signalClock);
      decoder.modulation->symbolIndex = (decoder.bitrate->offsetSymbolIndex + decoder.signalClock);
      decoder.modulation->detectIndex = (decoder.bitrate->offsetDetectIndex + decoder.signalClock);

      // get signal samples
      float currentSample = decoder.signalStatus.signalData[decoder.modulation->signalIndex & (SignalBufferLength - 1)];
      float delayedSample = decoder.signalStatus.signalData[decoder.modulation->symbolIndex & (SignalBufferLength - 1)];

      // compute symbol average
      decoder.modulation->symbolAverage = decoder.modulation->symbolAverage * decoder.bitrate->symbolAverageW0 + currentSample * decoder.bitrate->symbolAverageW1;

      // multiply 1 symbol delayed signal with incoming signal
      float phase = (currentSample - decoder.modulation->symbolAverage) * (delayedSample - decoder.modulation->symbolAverage);

      // store signal phase in filter buffer
      decoder.modulation->integrationData[decoder.modulation->signalIndex & (SignalBufferLength - 1)] = phase * 10;

      // integrate response from PICC after guard time (TR0)
      if (decoder.signalClock > (decoder.frameStatus.guardEnd - decoder.bitrate->period1SymbolSamples))
      {
         decoder.modulation->phaseIntegrate += decoder.modulation->integrationData[decoder.modulation->signalIndex & (SignalBufferLength - 1)]; // add new value
         decoder.modulation->phaseIntegrate -= decoder.modulation->integrationData[decoder.modulation->detectIndex & (SignalBufferLength - 1)]; // remove delayed value
      }

#ifdef DEBUG_BPSK_PHASE_INTEGRATION_CHANNEL
      decoderDebug->value(DEBUG_BPSK_PHASE_INTEGRATION_CHANNEL, decoder.modulation->phaseIntegrate);
#endif

#ifdef DEBUG_BPSK_PHASE_DEMODULATION_CHANNEL
      decoderDebug->value(DEBUG_BPSK_PHASE_DEMODULATION_CHANNEL, phase * 10);
#endif
      // search for Start Of Frame pattern (SoF)
      if (!decoder.modulation->symbolEndTime)
      {
         // detect first zero-cross
         if (decoder.modulation->phaseIntegrate > 0.00025f)
         {
            decoder.modulation->searchPeakTime = decoder.signalClock;
            decoder.modulation->searchEndTime = decoder.signalClock + decoder.bitrate->period2SymbolSamples;
         }

         if (decoder.signalClock == decoder.modulation->searchEndTime)
         {
#ifdef DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL
            decoderDebug->value(DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL, 0.75);
#endif
            // set symbol window
            decoder.modulation->symbolStartTime = decoder.modulation->searchPeakTime;
            decoder.modulation->symbolEndTime = decoder.modulation->searchPeakTime + decoder.bitrate->period1SymbolSamples;
            decoder.modulation->symbolPhase = decoder.modulation->phaseIntegrate;
            decoder.modulation->phaseThreshold = std::fabs(decoder.modulation->phaseIntegrate / 3);

            // set symbol info
            decoder.symbolStatus.value = 0;
            decoder.symbolStatus.start = decoder.modulation->symbolStartTime - decoder.bitrate->symbolDelayDetect;
            decoder.symbolStatus.end = decoder.modulation->symbolEndTime - decoder.bitrate->symbolDelayDetect;
            decoder.symbolStatus.length = decoder.symbolStatus.end - decoder.symbolStatus.start;

            pattern = PatternType::PatternM;
            break;
         }

            // frame waiting time exceeded
         else if (decoder.signalClock == decoder.frameStatus.waitingEnd)
         {
            pattern = PatternType::NoPattern;
            break;
         }
      }

         // search Response Bit Stream
      else
      {
         // edge detector for re-synchronization
         if ((decoder.modulation->phaseIntegrate > 0 && decoder.modulation->symbolPhase < 0) || (decoder.modulation->phaseIntegrate < 0 && decoder.modulation->symbolPhase > 0))
         {
            decoder.modulation->searchPeakTime = decoder.signalClock;
            decoder.modulation->searchEndTime = decoder.signalClock + decoder.bitrate->period2SymbolSamples;
            decoder.modulation->symbolStartTime = decoder.signalClock;
            decoder.modulation->symbolEndTime = decoder.signalClock + decoder.bitrate->period1SymbolSamples;
            decoder.modulation->symbolPhase = decoder.modulation->phaseIntegrate;
         }

         // set next search sync window from previous
         if (!decoder.modulation->searchEndTime)
         {
            // estimated symbol start and end
            decoder.modulation->symbolStartTime = decoder.modulation->symbolEndTime;
            decoder.modulation->symbolEndTime = decoder.modulation->symbolStartTime + decoder.bitrate->period1SymbolSamples;

            // timig next symbol
            decoder.modulation->searchEndTime = decoder.modulation->symbolStartTime + decoder.bitrate->period2SymbolSamples;
         }

            // search symbol timings
         else if (decoder.signalClock == decoder.modulation->searchEndTime)
         {
#ifdef DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL
            decoderDebug->value(DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL, 0.5);
#endif
            decoder.modulation->symbolPhase = decoder.modulation->phaseIntegrate;

            // setup symbol info
            decoder.symbolStatus.start = decoder.modulation->symbolStartTime - decoder.bitrate->symbolDelayDetect;
            decoder.symbolStatus.end = decoder.modulation->symbolEndTime - decoder.bitrate->symbolDelayDetect;
            decoder.symbolStatus.length = decoder.symbolStatus.end - decoder.symbolStatus.start;

            // no symbol change, keep previous symbol pattern
            if (decoder.modulation->phaseIntegrate > decoder.modulation->phaseThreshold)
            {
               pattern = decoder.symbolStatus.pattern;
               break;
            }

            // symbol change, invert pattern and value
            if (decoder.modulation->phaseIntegrate < -decoder.modulation->phaseThreshold)
            {
               decoder.symbolStatus.value = !decoder.symbolStatus.value;
               pattern = (decoder.symbolStatus.pattern == PatternType::PatternM) ? PatternType::PatternN : PatternType::PatternM;
               break;
            }

            // no decoder.modulation detected, generate End Of Frame symbol
            pattern = PatternType::PatternO;
            break;
         }
      }
   }

   // reset search status
   if (pattern != PatternType::Invalid)
   {
      decoder.symbolStatus.pattern = pattern;

      decoder.modulation->searchStartTime = 0;
      decoder.modulation->searchEndTime = 0;
      decoder.modulation->correlationPeek = 0;
      decoder.modulation->searchPulseWidth = 0;
      decoder.modulation->correlatedSD = 0;
   }

   return pattern;
}

bool NfcDecoder::Impl::decodeFrameDevNfcB(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   return false;
}

bool NfcDecoder::Impl::decodeFrameTagNfcB(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   return false;
}

int NfcDecoder::Impl::decodeSymbolTagAskNfcB(sdr::SignalBuffer &buffer)
{
   return 0;
}

int NfcDecoder::Impl::decodeSymbolTagBpskNfcB(sdr::SignalBuffer &buffer)
{
   return 0;
}

void NfcDecoder::Impl::resetFrameSearch()
{
   // reset frame search status
   if (decoder.modulation)
   {
      decoder.modulation->symbolEndTime = 0;
      decoder.modulation->searchPeakTime = 0;
      decoder.modulation->searchEndTime = 0;
      decoder.modulation->correlationPeek = 0;
   }

   // reset frame start time
   decoder.frameStatus.frameStart = 0;
}

void NfcDecoder::Impl::resetModulation()
{
   // reset decoder.modulation detection for all rates
   for (int rate = r106k; rate <= r424k; rate++)
   {
      decoder.modulationStatus[rate].searchStartTime = 0;
      decoder.modulationStatus[rate].searchEndTime = 0;
      decoder.modulationStatus[rate].correlationPeek = 0;
      decoder.modulationStatus[rate].searchPulseWidth = 0;
      decoder.modulationStatus[rate].searchDeepValue = 0;
      decoder.modulationStatus[rate].symbolAverage = 0;
      decoder.modulationStatus[rate].symbolPhase = NAN;
   }

   // clear stream status
   decoder.streamStatus = {0,};

   // clear stream status
   decoder.symbolStatus = {0,};

   // clear frame status
   decoder.frameStatus.frameType = 0;
   decoder.frameStatus.frameStart = 0;
   decoder.frameStatus.frameEnd = 0;

   // restore decoder.bitrate
   decoder.bitrate = nullptr;

   // restore decoder.modulation
   decoder.modulation = nullptr;
}

bool NfcDecoder::Impl::nextSample(sdr::SignalBuffer &buffer)
{
   if (buffer.available() == 0)
      return false;

   // real-value signal
   if (buffer.stride() == 1)
   {
      // read next sample data
      buffer.get(decoder.signalStatus.signalValue);
   }

      // IQ channel signal
   else
   {
      // read next sample data
      buffer.get(decoder.signalStatus.sampleData, 2);

      // compute magnitude from IQ channels
      auto i = double(decoder.signalStatus.sampleData[0]);
      auto q = double(decoder.signalStatus.sampleData[1]);

      decoder.signalStatus.signalValue = sqrtf(i * i + q * q);
   }

   // update signal clock
   decoder.signalClock++;

   // compute power average (exponential average)
   decoder.signalStatus.powerAverage = decoder.signalStatus.powerAverage * decoder.signalParams.powerAverageW0 + decoder.signalStatus.signalValue * decoder.signalParams.powerAverageW1;

   // compute signal average (exponential average)
   decoder.signalStatus.signalAverage = decoder.signalStatus.signalAverage * decoder.signalParams.signalAverageW0 + decoder.signalStatus.signalValue * decoder.signalParams.signalAverageW1;

   // compute signal variance (exponential variance)
   decoder.signalStatus.signalVariance = decoder.signalStatus.signalVariance * decoder.signalParams.signalVarianceW0 + std::abs(decoder.signalStatus.signalValue - decoder.signalStatus.signalAverage) * decoder.signalParams.signalVarianceW1;

   // store next signal value in sample buffer
   decoder.signalStatus.signalData[decoder.signalClock & (SignalBufferLength - 1)] = decoder.signalStatus.signalValue;

#ifdef DEBUG_SIGNAL
   decoderDebug->block(decoder.signalClock);
#endif

#ifdef DEBUG_SIGNAL_VALUE_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_VALUE_CHANNEL, decoder.signalStatus.signalValue);
#endif

#ifdef DEBUG_SIGNAL_POWER_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_POWER_CHANNEL, decoder.signalStatus.powerAverage);
#endif

#ifdef DEBUG_SIGNAL_AVERAGE_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_AVERAGE_CHANNEL, decoder.signalStatus.signalAverage);
#endif

#ifdef DEBUG_SIGNAL_VARIANCE_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_VARIANCE_CHANNEL, decoder.signalStatus.signalVariance);
#endif

#ifdef DEBUG_SIGNAL_EDGE_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_EDGE_CHANNEL, decoder.signalStatus.signalAverage - decoder.signalStatus.powerAverage);
#endif

   return true;
}

void NfcDecoder::Impl::process(NfcFrame frame)
{
   // for request frame set default response timings, must be overridden by subsequent process functions
   if (frame.isPollFrame())
   {
      decoder.frameStatus.frameWaitingTime = decoder.protocolStatus.frameWaitingTime;
      decoder.frameStatus.frameWaitingTime = decoder.protocolStatus.frameWaitingTime;
   }

   while (true)
   {
      if (processREQA(frame))
         break;

      if (processHLTA(frame))
         break;

      if (!(decoder.chainedFlags & FrameFlags::Encrypted))
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

      break;
   }

   // set chained flags
   frame.setFrameFlags(decoder.chainedFlags);

   // for request frame set response timings
   if (frame.isPollFrame())
   {
      // update frame timing parameters for receive PICC frame
      if (decoder.bitrate)
      {
         // response guard time TR0min (PICC must not modulate reponse within this period)
         decoder.frameStatus.guardEnd = decoder.frameStatus.frameEnd + decoder.frameStatus.frameGuardTime + decoder.bitrate->symbolDelayDetect;

         // response delay time WFT (PICC must reply to command before this period)
         decoder.frameStatus.waitingEnd = decoder.frameStatus.frameEnd + decoder.frameStatus.frameWaitingTime + decoder.bitrate->symbolDelayDetect;

         // next frame must be ListenFrame
         decoder.frameStatus.frameType = ListenFrame;
      }
   }
   else
   {
      // switch to decoder.modulation search
      decoder.frameStatus.frameType = 0;

      // reset frame command
      decoder.frameStatus.lastCommand = 0;
   }

   // mark last processed frame
   decoder.lastFrameEnd = decoder.frameStatus.frameEnd;

   // reset farme start
   decoder.frameStatus.frameStart = 0;

   // reset farme end
   decoder.frameStatus.frameEnd = 0;
}

bool NfcDecoder::Impl::processREQA(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if ((frame[0] == NFCA_REQA || frame[0] == NFCA_WUPA) && frame.limit() == 1)
      {
         frame.setFramePhase(FramePhase::SelectionFrame);

         decoder.frameStatus.lastCommand = frame[0];

         // This commands starts or wakeup card communication, so reset the protocol parameters to the default values
         decoder.protocolStatus.maxFrameSize = 256;
         decoder.protocolStatus.frameGuardTime = int(decoder.signalParams.sampleTimeUnit * 128 * 7);
         decoder.protocolStatus.frameWaitingTime = int(decoder.signalParams.sampleTimeUnit * 256 * 16 * (1 << 4));

         // The REQ-A Response must start exactly at 128 * n, n=9, decoder search between n=7 and n=18
         decoder.frameStatus.frameGuardTime = decoder.signalParams.sampleTimeUnit * 128 * 7; // REQ-A response guard
         decoder.frameStatus.frameWaitingTime = decoder.signalParams.sampleTimeUnit * 128 * 18; // REQ-A response timeout

         // clear chained flags
         decoder.chainedFlags = 0;

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (decoder.frameStatus.lastCommand == NFCA_REQA || decoder.frameStatus.lastCommand == NFCA_WUPA)
      {
         frame.setFramePhase(FramePhase::SelectionFrame);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processHLTA(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if (frame[0] == NFCA_HLTA && frame.limit() == 4)
      {
         frame.setFramePhase(FramePhase::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         decoder.frameStatus.lastCommand = frame[0];

         // After this command the PICC will stop and will not respond, set the protocol parameters to the default values
         decoder.protocolStatus.maxFrameSize = 256;
         decoder.protocolStatus.frameGuardTime = int(decoder.signalParams.sampleTimeUnit * 128 * 7);
         decoder.protocolStatus.frameWaitingTime = int(decoder.signalParams.sampleTimeUnit * 256 * 16 * (1 << 4));

         // clear chained flags
         decoder.chainedFlags = 0;

         // reset decoder.modulation status
         resetModulation();

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processSELn(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if (frame[0] == NFCA_SEL1 || frame[0] == NFCA_SEL2 || frame[0] == NFCA_SEL3)
      {
         frame.setFramePhase(FramePhase::SelectionFrame);

         decoder.frameStatus.lastCommand = frame[0];

         // The selection commands has same timings as REQ-A
         decoder.frameStatus.frameGuardTime = decoder.signalParams.sampleTimeUnit * 128 * 7;
         decoder.frameStatus.frameWaitingTime = decoder.signalParams.sampleTimeUnit * 128 * 18;

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (decoder.frameStatus.lastCommand == NFCA_SEL1 || decoder.frameStatus.lastCommand == NFCA_SEL2 || decoder.frameStatus.lastCommand == NFCA_SEL3)
      {
         frame.setFramePhase(FramePhase::SelectionFrame);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processRATS(NfcFrame &frame)
{
   // capture parameters from ATS and reconfigure decoder timings
   if (frame.isPollFrame())
   {
      if (frame[0] == NFCA_RATS)
      {
         auto fsdi = (frame[1] >> 4) & 0x0F;

         decoder.frameStatus.lastCommand = frame[0];

         // sets maximum frame length requested by reader
         decoder.protocolStatus.maxFrameSize = TABLE_FDS[fsdi];

         // sets the activation frame waiting time for ATS response, ISO/IEC 14443-4 defined a value of 65536/fc (~4833 μs).
         decoder.frameStatus.frameWaitingTime = int(decoder.signalParams.sampleTimeUnit * 65536);

         log.info("RATS frame parameters");
         log.info("  maxFrameSize {} bytes", {decoder.protocolStatus.maxFrameSize});

         frame.setFramePhase(FramePhase::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (decoder.frameStatus.lastCommand == NFCA_RATS)
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
               decoder.protocolStatus.startUpGuardTime = int(decoder.signalParams.sampleTimeUnit * 256 * 16 * (1 << sfgi));
               decoder.protocolStatus.frameWaitingTime = int(decoder.signalParams.sampleTimeUnit * 256 * 16 * (1 << fwi));
            }
            else
            {
               // if TB is not transmitted establish default timing parameters
               decoder.protocolStatus.startUpGuardTime = int(decoder.signalParams.sampleTimeUnit * 256 * 16 * (1 << 0));
               decoder.protocolStatus.frameWaitingTime = int(decoder.signalParams.sampleTimeUnit * 256 * 16 * (1 << 4));
            }

            log.info("ATS protocol timing parameters");
            log.info("  startUpGuardTime {} samples ({} us)", {decoder.frameStatus.startUpGuardTime, 1000000.0 * decoder.frameStatus.startUpGuardTime / decoder.sampleRate});
            log.info("  frameWaitingTime {} samples ({} us)", {decoder.frameStatus.frameWaitingTime, 1000000.0 * decoder.frameStatus.frameWaitingTime / decoder.sampleRate});
         }

         frame.setFramePhase(FramePhase::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processPPSr(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if ((frame[0] & 0xF0) == NFCA_PPS)
      {
         decoder.frameStatus.lastCommand = frame[0] & 0xF0;

         // Set PPS response waiting time to protocol default
         decoder.frameStatus.frameWaitingTime = decoder.protocolStatus.frameWaitingTime;

         frame.setFramePhase(FramePhase::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (decoder.frameStatus.lastCommand == NFCA_PPS)
      {
         frame.setFramePhase(FramePhase::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processAUTH(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if (frame[0] == NFCA_AUTH1 || frame[0] == NFCA_AUTH2)
      {
         decoder.frameStatus.lastCommand = frame[0];
//         decoder.frameStatus.frameWaitingTime = int(signalParams.sampleTimeUnit * 256 * 16 * (1 << 14);

         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

//      if (!decoder.frameStatus.lastCommand)
//      {
//         crypto1_init(&decoder.frameStatus.crypto1, 0x521284223154);
//      }

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (decoder.frameStatus.lastCommand == NFCA_AUTH1 || decoder.frameStatus.lastCommand == NFCA_AUTH2)
      {
         //      unsigned int uid = 0x1e47fc35;
//      unsigned int nc = frame[0] << 24 || frame[1] << 16 || frame[2] << 8 || frame[0];
//
//      crypto1_word(&decoder.frameStatus.crypto1, uid ^ nc, 0);

         // set chained flags

         decoder.chainedFlags = FrameFlags::Encrypted;

         frame.setFramePhase(FramePhase::ApplicationFrame);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processIBlock(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if ((frame[0] & 0xE2) == NFCA_IBLOCK)
      {
         decoder.frameStatus.lastCommand = frame[0] & 0xE2;

         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (decoder.frameStatus.lastCommand == NFCA_IBLOCK)
      {
         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processRBlock(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if ((frame[0] & 0xE6) == NFCA_RBLOCK)
      {
         decoder.frameStatus.lastCommand = frame[0] & 0xE6;

         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (decoder.frameStatus.lastCommand == NFCA_RBLOCK)
      {
         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processSBlock(NfcFrame &frame)
{
   if (frame.isPollFrame())
   {
      if ((frame[0] & 0xC7) == NFCA_SBLOCK)
      {
         decoder.frameStatus.lastCommand = frame[0] & 0xC7;

         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   if (frame.isListenFrame())
   {
      if (decoder.frameStatus.lastCommand == NFCA_SBLOCK)
      {
         frame.setFramePhase(FramePhase::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);

         return true;
      }
   }

   return false;
}

void NfcDecoder::Impl::processOther(NfcFrame &frame)
{
   frame.setFramePhase(FramePhase::ApplicationFrame);
   frame.setFrameFlags(!checkCrc(frame) ? FrameFlags::CrcError : 0);
}

bool NfcDecoder::Impl::checkCrc(NfcFrame &frame)
{
   unsigned short crc = 0, res = 0;

   int length = frame.limit();

   if (length <= 2)
      return false;

   if (frame.isNfcA())
      crc = 0x6363; // NFC-A ITU-V.41
   else if (frame.isNfcB())
      crc = 0xFFFF; // NFC-B ISO/IEC 13239

   for (int i = 0; i < length - 2; i++)
   {
      auto d = (unsigned char) frame[i];

      d = (d ^ (unsigned int) (crc & 0xff));
      d = (d ^ (d << 4));

      crc = (crc >> 8) ^ ((unsigned short) (d << 8)) ^ ((unsigned short) (d << 3)) ^ ((unsigned short) (d >> 4));
   }

   if (frame.isNfcB())
      crc = ~crc;

   res |= ((unsigned int) frame[length - 2] & 0xff);
   res |= ((unsigned int) frame[length - 1] & 0xff) << 8;

   return res == crc;
}

bool NfcDecoder::Impl::checkParity(unsigned int value, unsigned int parity)
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

}
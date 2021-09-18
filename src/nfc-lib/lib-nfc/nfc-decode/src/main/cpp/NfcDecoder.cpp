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
#include <nfc/NfcDecoder.h>

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

// carrier frequency (13.56 Mhz)
constexpr static const unsigned int BaseFrequency = 13.56E6;

// buffer for signal integration, must be power of 2^n
constexpr static const unsigned int SignalBufferLength = 512;

enum TechType
{
   NfcA = 1,
   NfcB = 2
};

enum FrameType
{
   PollFrame = 1,
   ListenFrame = 2
};

enum FrameFlags
{
   ShortFrame = NfcFrame::ShortFrame,
   Encrypted = NfcFrame::Encrypted,
   ParityError = NfcFrame::ParityError,
   CrcError = NfcFrame::CrcError,
   Truncated = NfcFrame::Truncated
};

enum RateType
{
   r106k = 0,
   r212k = 1,
   r424k = 2,
   r848k = 3
};

enum PatternType
{
   Invalid = 0,
   NoCarrier = 1,
   NoPattern = 2,
   PatternX = 3,
   PatternY = 4,
   PatternZ = 5,
   PatternD = 6,
   PatternE = 7,
   PatternF = 8,
   PatternM = 9,
   PatternN = 10,
   PatternO = 11
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

struct SignalParams
{
   // exponential factors for power integrator
   float powerAverageW0;
   float powerAverageW1;

   // exponential factors for average integrator
   float signalAverageW0;
   float signalAverageW1;

   // exponential factors for variance integrator
   float signalVarianceW0;
   float signalVarianceW1;

   // default decoder parameters
   double sampleTimeUnit;
};

struct BitrateParams
{
   int rateType;
   int techType;

   float symbolAverageW0;
   float symbolAverageW1;

   // symbol parameters
   unsigned int symbolsPerSecond;
   unsigned int period1SymbolSamples;
   unsigned int period2SymbolSamples;
   unsigned int period4SymbolSamples;
   unsigned int period8SymbolSamples;

   // modulation parameters
   unsigned int symbolDelayDetect;
   unsigned int offsetSignalIndex;
   unsigned int offsetFilterIndex;
   unsigned int offsetSymbolIndex;
   unsigned int offsetDetectIndex;
};

struct SignalStatus
{
   // raw IQ sample data
   float sampleData[2];

   // signal normalized value
   float signalValue;

   // exponential power integrator
   float powerAverage;

   // exponential average integrator
   float signalAverage;

   // exponential variance integrator
   float signalVariance;

   // signal data buffer
   float signalData[SignalBufferLength];

   // silence start (no modulation detected)
   unsigned int carrierOff;

   // silence end (modulation detected)
   unsigned int carrierOn;
};

/*
 * Modulation parameters and status for one symbol rate
 */
struct ModulationStatus
{
   // symbol search status
   unsigned int searchStartTime;   // sample start of symbol search window
   unsigned int searchEndTime;     // sample end of symbol search window
   unsigned int searchPeakTime;    // sample time for for maximum correlation peak
   unsigned int searchPulseWidth;         // detected signal pulse width
   float searchDeepValue;          // signal modulation deep during search
   float searchThreshold;          // signal threshold

   // symbol parameters
   unsigned int symbolStartTime;
   unsigned int symbolEndTime;
   float symbolCorr0;
   float symbolCorr1;
   float symbolPhase;
   float symbolAverage;

   // integrator processor
   float filterIntegrate;
   float phaseIntegrate;
   float phaseThreshold;

   // integration indexes
   unsigned int signalIndex;
   unsigned int filterIndex;
   unsigned int symbolIndex;
   unsigned int detectIndex;

   // correlation indexes
   unsigned int filterPoint1;
   unsigned int filterPoint2;
   unsigned int filterPoint3;

   // correlation processor
   float correlatedS0;
   float correlatedS1;
   float correlatedSD;
   float correlationPeek;

   float integrationData[SignalBufferLength];
   float correlationData[SignalBufferLength];
};

/*
 * Status for one demodulated symbol
 */
struct SymbolStatus
{
   unsigned int pattern; // symbol pattern
   unsigned int value; // symbol value (0 / 1)
   unsigned long start;    // sample clocks for start of last decoded symbol
   unsigned long end; // sample clocks for end of last decoded symbol
   unsigned int length; // samples for next symbol synchronization
   unsigned int rate; // symbol rate
};

/*
 * Status for demodulate bit stream
 */
struct StreamStatus
{
   unsigned int previous;
   unsigned int pattern;
   unsigned int bits;
   unsigned int data;
   unsigned int flags;
   unsigned int parity;
   unsigned int bytes;
   unsigned char buffer[512];
};

/*
 * Status for demodulate frame
 */
struct FrameStatus
{
   unsigned int lastCommand; // last command
   unsigned int frameType; // frame type
   unsigned int symbolRate; // frame bit rate
   unsigned int frameStart;    // sample clocks for start of last decoded symbol
   unsigned int frameEnd; // sample clocks for end of last decoded symbol
   unsigned int guardEnd; // frame guard end time
   unsigned int waitingEnd; // frame waiting end time

   unsigned int frameGuardTime;
   unsigned int frameWaitingTime;
   unsigned int startUpGuardTime;
   unsigned int maxFrameSize;
};

struct DecoderDebug
{
   unsigned int channels;
   unsigned int clock;

   sdr::RecordDevice *recorder;
   sdr::SignalBuffer buffer;

   float values[10] {0,};

   DecoderDebug(unsigned int channels, unsigned int sampleRate) : channels(channels), clock(0)
   {
      char file[128];
      struct tm timeinfo {};

      std::time_t rawTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      localtime_s(&timeinfo, &rawTime);
      strftime(file, sizeof(file), "decoder-%Y%m%d%H%M%S.wav", &timeinfo);

      recorder = new sdr::RecordDevice(file);
      recorder->setChannelCount(channels);
      recorder->setSampleRate(sampleRate);
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
      buffer = sdr::SignalBuffer(sampleCount * recorder->channelCount(), recorder->channelCount(), recorder->sampleRate());
   }

   void commit()
   {
      buffer.flip();

      recorder->write(buffer);
   }
};

struct NfcDecoder::Impl
{
   // frame handler
   typedef std::function<bool(nfc::NfcFrame &frame)> FrameHandler;

   rt::Logger log {"NfcDecoder"};

   // signal parameters
   SignalParams signalParams {0,};

   // bitrate parameters
   BitrateParams bitrateParams[4] {0,};

   // signal processing status
   SignalStatus signalStatus {0,};

   // detected symbol status
   SymbolStatus symbolStatus {0,};

   // bit stream status
   StreamStatus streamStatus {0,};

   // frame processing status
   FrameStatus frameStatus {0,};

   // modulation status for each bitrate
   ModulationStatus modulationStatus[4] {0,};

   // current detected bitrate
   BitrateParams *bitrate = nullptr;

   // current detected modulation
   ModulationStatus *modulation = nullptr;

   // decoder debugging recorder
   std::shared_ptr<DecoderDebug> decoderDebug;

   // signal sample rate
   unsigned int sampleRate = 0;

   // signal master clock
   unsigned int signalClock = 0;

   // last detected frame end
   unsigned int lastFrameEnd = 0;

   // chained frame flags
   unsigned int chainedFlags = 0;

   // minimun signal level
   float powerLevelThreshold = 0.010f;

   // minimun modulation threshold to detect valid signal (default 5%)
   float modulationThreshold = 0.850f;

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
   impl->powerLevelThreshold = value;
}

float NfcDecoder::powerLevelThreshold() const
{
   return impl->powerLevelThreshold;
}

void NfcDecoder::setModulationThreshold(float value)
{
   impl->modulationThreshold = value;
}

float NfcDecoder::modulationThreshold() const
{
   return impl->modulationThreshold;
}

float NfcDecoder::signalStrength() const
{
   return impl->signalStatus.powerAverage;
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
   signalParams = {0,};

   // clear signal processing status
   signalStatus = {0,};

   // clear detected symbol status
   symbolStatus = {0,};

   // clear bit stream status
   streamStatus = {0,};

   // clear frame processing status
   frameStatus = {0,};

   // set decoder samplerate
   sampleRate = newSampleRate;

   // clear signal master clock
   signalClock = 0;

   // clear last detected frame end
   lastFrameEnd = 0;

   // clear chained flags
   chainedFlags = 0;

   if (sampleRate > 0)
   {
      // calculate sample time unit
      signalParams.sampleTimeUnit = double(sampleRate) / double(BaseFrequency);

      log.info("--------------------------------------------");
      log.info("initializing NFC decoder");
      log.info("--------------------------------------------");
      log.info("\tsignalSampleRate     {}", {sampleRate});
      log.info("\tpowerLevelThreshold  {}", {powerLevelThreshold});
      log.info("\tmodulationThreshold  {}", {modulationThreshold});

      // compute symbol parameters for 106Kbps, 212Kbps, 424Kbps and 848Kbps
      for (int rate = r106k; rate <= r424k; rate++)
      {
         // clear bitrate parameters
         bitrateParams[rate] = {0,};

         // clear modulation parameters
         modulationStatus[rate] = {0,};

         // configuracion to current bitrate
         bitrate = bitrateParams + rate;

         // set tech type and rate
         bitrate->techType = NfcA;
         bitrate->rateType = rate;

         // symbol timming parameters
         bitrate->symbolsPerSecond = BaseFrequency / (128 >> rate);

         // correlator timming parameters
         bitrate->period1SymbolSamples = int(round(signalParams.sampleTimeUnit * (128 >> rate)));
         bitrate->period2SymbolSamples = int(round(signalParams.sampleTimeUnit * (64 >> rate)));
         bitrate->period4SymbolSamples = int(round(signalParams.sampleTimeUnit * (32 >> rate)));
         bitrate->period8SymbolSamples = int(round(signalParams.sampleTimeUnit * (16 >> rate)));

         // delay guard from lower rate
         bitrate->symbolDelayDetect = rate > r106k ? bitrateParams[rate - 1].symbolDelayDetect + bitrateParams[rate - 1].period1SymbolSamples : 0;

         // moving average offsets
         bitrate->offsetSignalIndex = SignalBufferLength - bitrate->symbolDelayDetect;
         bitrate->offsetFilterIndex = SignalBufferLength - bitrate->symbolDelayDetect - bitrate->period2SymbolSamples;
         bitrate->offsetSymbolIndex = SignalBufferLength - bitrate->symbolDelayDetect - bitrate->period1SymbolSamples;
         bitrate->offsetDetectIndex = SignalBufferLength - bitrate->symbolDelayDetect - bitrate->period4SymbolSamples;

         // exponential symbol average
         bitrate->symbolAverageW0 = float(1 - 5.0 / bitrate->period1SymbolSamples);
         bitrate->symbolAverageW1 = float(1 - bitrate->symbolAverageW0);

         log.info("{} kpbs parameters:", {round(bitrate->symbolsPerSecond / 1E3)});
         log.info("\tsymbolsPerSecond     {}", {bitrate->symbolsPerSecond});
         log.info("\tperiod1SymbolSamples {} ({} us)", {bitrate->period1SymbolSamples, 1E6 * bitrate->period1SymbolSamples / sampleRate});
         log.info("\tperiod2SymbolSamples {} ({} us)", {bitrate->period2SymbolSamples, 1E6 * bitrate->period2SymbolSamples / sampleRate});
         log.info("\tperiod4SymbolSamples {} ({} us)", {bitrate->period4SymbolSamples, 1E6 * bitrate->period4SymbolSamples / sampleRate});
         log.info("\tperiod8SymbolSamples {} ({} us)", {bitrate->period8SymbolSamples, 1E6 * bitrate->period8SymbolSamples / sampleRate});
         log.info("\tsymbolDelayDetect    {} ({} us)", {bitrate->symbolDelayDetect, 1E6 * bitrate->symbolDelayDetect / sampleRate});
         log.info("\toffsetSignalIndex    {}", {bitrate->offsetSignalIndex});
         log.info("\toffsetFilterIndex    {}", {bitrate->offsetFilterIndex});
         log.info("\toffsetSymbolIndex    {}", {bitrate->offsetSymbolIndex});
         log.info("\toffsetDetectIndex    {}", {bitrate->offsetDetectIndex});
      }

      // initialize default frame parameters
      frameStatus.maxFrameSize = 256;
      frameStatus.frameGuardTime = signalParams.sampleTimeUnit * 128 * 7;
      frameStatus.frameWaitingTime = signalParams.sampleTimeUnit * 128 * 18;

      // initialize exponential average factors for power value
      signalParams.powerAverageW0 = float(1 - 1E3 / sampleRate);
      signalParams.powerAverageW1 = float(1 - signalParams.powerAverageW0);

      // initialize exponential average factors for signal average
      signalParams.signalAverageW0 = float(1 - 1E5 / sampleRate);
      signalParams.signalAverageW1 = float(1 - signalParams.signalAverageW0);

      // initialize exponential average factors for signal variance
      signalParams.signalVarianceW0 = float(1 - 1E5 / sampleRate);
      signalParams.signalVarianceW1 = float(1 - signalParams.signalVarianceW0);

      // starts width no modulation
      modulation = nullptr;

   }

#ifdef DEBUG_SIGNAL
   log.warn("DECODER DEBUGGER ENABLED!, performance may be impacted");
   decoderDebug = std::make_shared<DecoderDebug>(DEBUG_CHANNELS, sampleRate);
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
      if (sampleRate != samples.sampleRate())
      {
         configure(samples.sampleRate());
      }

#ifdef DEBUG_SIGNAL
      decoderDebug->begin(samples.elements());
#endif

      while (!samples.isEmpty())
      {
         if (!modulation)
         {
            if (!detectModulation(samples, frames))
            {
               break;
            }
         }

         if (bitrate->techType == NfcA)
         {
            if (frameStatus.frameType == PollFrame)
            {
               decodeFrameDevNfcA(samples, frames);
            }

            if (frameStatus.frameType == ListenFrame)
            {
               decodeFrameTagNfcA(samples, frames);
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
      if (signalStatus.carrierOff)
      {
//         log.debug("detected carrier lost from {} to {}", {signalStatus.carrierOff, signalClock});

         NfcFrame silence = NfcFrame(NfcFrame::None, NfcFrame::NoCarrier);

         silence.setFramePhase(NfcFrame::CarrierFrame);
         silence.setSampleStart(signalStatus.carrierOff);
         silence.setSampleEnd(signalClock);
         silence.setTimeStart(double(signalStatus.carrierOff) / double(sampleRate));
         silence.setTimeEnd(double(signalClock) / double(sampleRate));

         frames.push_back(silence);
      }

      else if (signalStatus.carrierOn)
      {
//         log.debug("detected carrier present from {} to {}", {signalStatus.carrierOn, signalClock});

         NfcFrame carrier = NfcFrame(NfcFrame::None, NfcFrame::EmptyFrame);

         carrier.setFramePhase(NfcFrame::CarrierFrame);
         carrier.setSampleStart(signalStatus.carrierOn);
         carrier.setSampleEnd(signalClock);
         carrier.setTimeStart(double(signalStatus.carrierOn) / double(sampleRate));
         carrier.setTimeEnd(double(signalClock) / double(sampleRate));

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
   symbolStatus.pattern = PatternType::Invalid;

   while (nextSample(buffer) && symbolStatus.pattern == PatternType::Invalid)
   {
      // ignore low power signals
      if (signalStatus.powerAverage > powerLevelThreshold)
      {
         // POLL frame ASK detector for  106Kbps, 212Kbps and 424Kbps
         for (int rate = r106k; rate <= r424k; rate++)
         {
            bitrate = bitrateParams + rate;
            modulation = modulationStatus + rate;

            // compute signal pointers
            modulation->signalIndex = (bitrate->offsetSignalIndex + signalClock);
            modulation->filterIndex = (bitrate->offsetFilterIndex + signalClock);

            // get signal samples
            float currentData = signalStatus.signalData[modulation->signalIndex & (SignalBufferLength - 1)];
            float delayedData = signalStatus.signalData[modulation->filterIndex & (SignalBufferLength - 1)];

            // integrate signal data over 1/2 symbol
            modulation->filterIntegrate += currentData; // add new value
            modulation->filterIntegrate -= delayedData; // remove delayed value

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
            modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + currentData * bitrate->symbolAverageW1;

#ifdef DEBUG_ASK_CORRELATION_CHANNEL
            decoderDebug->value(DEBUG_ASK_CORRELATION_CHANNEL, modulation->correlatedSD);
#endif
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
            decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.0f);
#endif
            // search for Pattern-Z in PCD to PICC request
            if (modulation->correlatedSD > signalStatus.powerAverage * modulationThreshold)
            {
               // calculate symbol modulation deep
               float modulationDeep = (signalStatus.powerAverage - currentData) / signalStatus.powerAverage;

               if (modulation->searchDeepValue < modulationDeep)
                  modulation->searchDeepValue = modulationDeep;

               // max correlation peak detector
               if (modulation->correlatedSD > modulation->correlationPeek)
               {
                  modulation->searchPulseWidth++;
                  modulation->searchPeakTime = signalClock;
                  modulation->searchEndTime = signalClock + bitrate->period4SymbolSamples;
                  modulation->correlationPeek = modulation->correlatedSD;
               }
            }

            // Check for SoF symbol
            if (signalClock == modulation->searchEndTime)
            {
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
               decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.75f);
#endif
               // check modulation deep and Pattern-Z, signaling Start Of Frame (PCD->PICC)
               if (modulation->searchDeepValue > 0.85 && modulation->correlationPeek > signalStatus.powerAverage * modulationThreshold)
               {
                  // set lower threshold to detect valid response pattern
                  modulation->searchThreshold = signalStatus.powerAverage * modulationThreshold;

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

                  break;
               }

               // reset modulation to continue search
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;
               modulation->searchDeepValue = 0;
               modulation->correlationPeek = 0;
            }
         }
      }

      // carrier edge detector
      float edge = std::fabs(signalStatus.signalAverage - signalStatus.powerAverage);

      // positive edge
      if (signalStatus.signalAverage > edge && signalStatus.powerAverage > powerLevelThreshold)
      {
         if (!signalStatus.carrierOn)
         {
            signalStatus.carrierOn = signalClock;

            if (signalStatus.carrierOff)
            {
//               log.debug("detected carrier lost from {} to {}", {signalStatus.carrierOff, signalStatus.carrierOn});

               NfcFrame silence = NfcFrame(NfcFrame::None, NfcFrame::NoCarrier);

               silence.setFramePhase(NfcFrame::CarrierFrame);
               silence.setSampleStart(signalStatus.carrierOff);
               silence.setSampleEnd(signalStatus.carrierOn);
               silence.setTimeStart(double(signalStatus.carrierOff) / double(sampleRate));
               silence.setTimeEnd(double(signalStatus.carrierOn) / double(sampleRate));

               frames.push_back(silence);
            }

            signalStatus.carrierOff = 0;
         }
      }

         // negative edge
      else if (signalStatus.signalAverage < edge || signalStatus.powerAverage < powerLevelThreshold)
      {
         if (!signalStatus.carrierOff)
         {
            signalStatus.carrierOff = signalClock;

            if (signalStatus.carrierOn)
            {
//               log.debug("detected carrier present from {} to {}", {signalStatus.carrierOn, signalStatus.carrierOff});

               NfcFrame carrier = NfcFrame(NfcFrame::None, NfcFrame::EmptyFrame);

               carrier.setFramePhase(NfcFrame::CarrierFrame);
               carrier.setSampleStart(signalStatus.carrierOn);
               carrier.setSampleEnd(signalStatus.carrierOff);
               carrier.setTimeStart(double(signalStatus.carrierOn) / double(sampleRate));
               carrier.setTimeEnd(double(signalStatus.carrierOff) / double(sampleRate));

               frames.push_back(carrier);
            }

            signalStatus.carrierOn = 0;
         }
      }
   }

   if (symbolStatus.pattern != PatternType::Invalid)
   {
      // reset modulation to continue search
      modulation->searchStartTime = 0;
      modulation->searchEndTime = 0;
      modulation->searchDeepValue = 0;
      modulation->correlationPeek = 0;

      // modulation detected
      return true;
   }

   // no bitrate detected
   bitrate = nullptr;

   // no modulation detected
   modulation = nullptr;

   return false;
}

bool NfcDecoder::Impl::decodeFrameDevNfcA(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   int pattern;

   // read NFC-A request request
   while ((pattern = decodeSymbolDevAskNfcA(buffer)) > PatternType::NoPattern)
   {
      streamStatus.pattern = pattern;

      // detect end of request (Pattern-Y after Pattern-Z)
      if ((streamStatus.pattern == PatternType::PatternY && (streamStatus.previous == PatternType::PatternY || streamStatus.previous == PatternType::PatternZ)) || streamStatus.bytes == frameStatus.maxFrameSize)
      {
         // frames must contains at least one full byte or 7 bits for short frames
         if (streamStatus.bytes > 0 || streamStatus.bits == 7)
         {
            // add remaining byte to request
            if (streamStatus.bits >= 7)
               streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;

            // set last symbol timming
            if (streamStatus.previous == PatternType::PatternZ)
               frameStatus.frameEnd = symbolStatus.start - bitrate->period2SymbolSamples;
            else
               frameStatus.frameEnd = symbolStatus.start - bitrate->period1SymbolSamples;

            // build request frame
            NfcFrame request = NfcFrame(NfcFrame::NfcA, NfcFrame::RequestFrame);

            request.setFrameRate(frameStatus.symbolRate);
            request.setSampleStart(frameStatus.frameStart);
            request.setSampleEnd(frameStatus.frameEnd);
            request.setTimeStart(double(frameStatus.frameStart) / double(sampleRate));
            request.setTimeEnd(double(frameStatus.frameEnd) / double(sampleRate));

            if (streamStatus.flags & ParityError)
               request.setFrameFlags(NfcFrame::ParityError);

            if (streamStatus.bytes == frameStatus.maxFrameSize)
               request.setFrameFlags(NfcFrame::Truncated);

            if (streamStatus.bytes == 1 && streamStatus.bits == 7)
               request.setFrameFlags(NfcFrame::ShortFrame);

            // add bytes to frame and flip to prepare read
            request.put(streamStatus.buffer, streamStatus.bytes).flip();

            // clear modulation status for next frame search
            modulation->symbolStartTime = 0;
            modulation->symbolEndTime = 0;
            modulation->filterIntegrate = 0;
            modulation->phaseIntegrate = 0;

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
         else if (streamStatus.bytes < frameStatus.maxFrameSize)
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

bool NfcDecoder::Impl::decodeFrameTagNfcA(sdr::SignalBuffer &buffer, std::list<NfcFrame> &frames)
{
   int pattern;

   // decode TAG ASK response
   if (bitrate->rateType == r106k)
   {
      if (!frameStatus.frameStart)
      {
         // search Start Of Frame pattern
         pattern = decodeSymbolTagAskNfcA(buffer);

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
         while ((pattern = decodeSymbolTagAskNfcA(buffer)) > PatternType::NoPattern)
         {
            // detect end of response for ASK
            if (pattern == PatternType::PatternF || streamStatus.bytes == frameStatus.maxFrameSize)
            {
               // a valid response must contains at least 4 bits of data
               if (streamStatus.bytes > 0 || streamStatus.bits == 4)
               {
                  // add remaining byte to request
                  if (streamStatus.bits == 4)
                     streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;

                  frameStatus.frameEnd = symbolStatus.end;

                  NfcFrame response = NfcFrame(NfcFrame::NfcA, NfcFrame::ResponseFrame);

                  response.setFrameRate(bitrate->symbolsPerSecond);
                  response.setSampleStart(frameStatus.frameStart);
                  response.setSampleEnd(frameStatus.frameEnd);
                  response.setTimeStart(double(frameStatus.frameStart) / double(sampleRate));
                  response.setTimeEnd(double(frameStatus.frameEnd) / double(sampleRate));

                  if (streamStatus.flags & ParityError)
                     response.setFrameFlags(NfcFrame::ParityError);

                  if (streamStatus.bytes == frameStatus.maxFrameSize)
                     response.setFrameFlags(NfcFrame::Truncated);

                  if (streamStatus.bytes == 1 && streamStatus.bits == 4)
                     response.setFrameFlags(NfcFrame::ShortFrame);

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
            else if (streamStatus.bytes < frameStatus.maxFrameSize)
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
   else if (bitrate->rateType == r212k || bitrate->rateType == r424k)
   {
      if (!frameStatus.frameStart)
      {
         // detect first pattern
         pattern = decodeSymbolTagBpskNfcA(buffer);

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
         while ((pattern = decodeSymbolTagBpskNfcA(buffer)) > PatternType::NoPattern)
         {
            // detect end of response for BPSK
            if (pattern == PatternType::PatternO)
            {
               if (streamStatus.bits == 9)
               {
                  // store byte in stream buffer
                  streamStatus.buffer[streamStatus.bytes++] = streamStatus.data;

                  // last byte has even parity
                  streamStatus.flags |= checkParity(streamStatus.data, streamStatus.parity) ? ParityError : 0;
               }

               // frames must contains at least one full byte
               if (streamStatus.bytes > 0)
               {
                  // mark frame end at star of EoF symbol
                  frameStatus.frameEnd = symbolStatus.start;

                  // build responde frame
                  NfcFrame response = NfcFrame(NfcFrame::NfcA, NfcFrame::ResponseFrame);

                  response.setFrameRate(bitrate->symbolsPerSecond);
                  response.setSampleStart(frameStatus.frameStart);
                  response.setSampleEnd(frameStatus.frameEnd);
                  response.setTimeStart(double(frameStatus.frameStart) / double(sampleRate));
                  response.setTimeEnd(double(frameStatus.frameEnd) / double(sampleRate));

                  if (streamStatus.flags & ParityError)
                     response.setFrameFlags(NfcFrame::ParityError);

                  if (streamStatus.bytes == frameStatus.maxFrameSize)
                     response.setFrameFlags(NfcFrame::Truncated);

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
            {
               streamStatus.data |= (symbolStatus.value << streamStatus.bits);
            }

               // decode parity bit
            else if (streamStatus.bits < 9)
            {
               streamStatus.parity = symbolStatus.value;
            }

               // store full byte in stream buffer and check parity
            else if (streamStatus.bytes < frameStatus.maxFrameSize)
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

               // too many bytes in frame, abort decoder
            else
            {
               // reset modulation status
               resetModulation();

               // no valid frame found
               return false;
            }

            streamStatus.bits++;
         }
      }
   }

   // end of stream...
   return false;
}

int NfcDecoder::Impl::decodeSymbolDevAskNfcA(sdr::SignalBuffer &buffer)
{
   symbolStatus.pattern = PatternType::Invalid;

   while (nextSample(buffer))
   {
      // compute pointers
      modulation->signalIndex = (bitrate->offsetSignalIndex + signalClock);
      modulation->filterIndex = (bitrate->offsetFilterIndex + signalClock);

      // get signal samples
      float currentData = signalStatus.signalData[modulation->signalIndex & (SignalBufferLength - 1)];
      float delayedData = signalStatus.signalData[modulation->filterIndex & (SignalBufferLength - 1)];

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

#ifdef DEBUG_ASK_CORRELATION_CHANNEL
      decoderDebug->value(DEBUG_ASK_CORRELATION_CHANNEL, modulation->correlatedSD);
#endif

#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
      decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.0f);
#endif
      // compute symbol average
      modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + currentData * bitrate->symbolAverageW1;

      // set next search sync window from previous state
      if (!modulation->searchStartTime)
      {
         // estimated symbol start and end
         modulation->symbolStartTime = modulation->symbolEndTime;
         modulation->symbolEndTime = modulation->symbolStartTime + bitrate->period1SymbolSamples;

         // timig search window
         modulation->searchStartTime = modulation->symbolEndTime - bitrate->period8SymbolSamples;
         modulation->searchEndTime = modulation->symbolEndTime + bitrate->period8SymbolSamples;

         // reset symbol parameters
         modulation->symbolCorr0 = 0;
         modulation->symbolCorr1 = 0;
      }

      // search max correlation peak
      if (signalClock >= modulation->searchStartTime && signalClock <= modulation->searchEndTime)
      {
         if (modulation->correlatedSD > modulation->correlationPeek)
         {
            modulation->correlationPeek = modulation->correlatedSD;
            modulation->symbolCorr0 = modulation->correlatedS0;
            modulation->symbolCorr1 = modulation->correlatedS1;
            modulation->symbolEndTime = signalClock;
         }
      }

      // capture next symbol
      if (signalClock == modulation->searchEndTime)
      {
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
         decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.50f);
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

int NfcDecoder::Impl::decodeSymbolTagAskNfcA(sdr::SignalBuffer &buffer)
{
   int pattern = PatternType::Invalid;

   while (nextSample(buffer))
   {
      // compute pointers
      modulation->signalIndex = (bitrate->offsetSignalIndex + signalClock);
      modulation->detectIndex = (bitrate->offsetDetectIndex + signalClock);

      // get signal samples
      float currentData = signalStatus.signalData[modulation->signalIndex & (SignalBufferLength - 1)];

      // compute symbol average (signal offset)
      modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + currentData * bitrate->symbolAverageW1;

      // signal value
      currentData -= modulation->symbolAverage;

      // store signal square in filter buffer
      modulation->integrationData[modulation->signalIndex & (SignalBufferLength - 1)] = currentData * currentData;

      // start correlation after frameGuardTime
      if (signalClock > (frameStatus.guardEnd - bitrate->period1SymbolSamples))
      {
         // compute correlation points
         modulation->filterPoint1 = (modulation->signalIndex % bitrate->period1SymbolSamples);
         modulation->filterPoint2 = (modulation->signalIndex + bitrate->period2SymbolSamples) % bitrate->period1SymbolSamples;
         modulation->filterPoint3 = (modulation->signalIndex + bitrate->period1SymbolSamples - 1) % bitrate->period1SymbolSamples;

         // integrate symbol (moving average)
         modulation->filterIntegrate += modulation->integrationData[modulation->signalIndex & (SignalBufferLength - 1)]; // add new value
         modulation->filterIntegrate -= modulation->integrationData[modulation->detectIndex & (SignalBufferLength - 1)]; // remove delayed value

         // store integrated signal in correlation buffer
         modulation->correlationData[modulation->filterPoint1] = modulation->filterIntegrate;

         // compute correlation results for each symbol and distance
         modulation->correlatedS0 = modulation->correlationData[modulation->filterPoint1] - modulation->correlationData[modulation->filterPoint2];
         modulation->correlatedS1 = modulation->correlationData[modulation->filterPoint2] - modulation->correlationData[modulation->filterPoint3];
         modulation->correlatedSD = std::fabs(modulation->correlatedS0 - modulation->correlatedS1);
      }

#ifdef DEBUG_ASK_CORRELATION_CHANNEL
      decoderDebug->value(DEBUG_ASK_CORRELATION_CHANNEL, modulation->correlatedSD);
#endif

#ifdef DEBUG_ASK_INTEGRATION_CHANNEL
      decoderDebug->value(DEBUG_ASK_INTEGRATION_CHANNEL, modulation->filterIntegrate);
#endif

#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
      decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.0f);
#endif

      // search for Start Of Frame pattern (SoF)
      if (!modulation->symbolEndTime)
      {
         if (signalClock > frameStatus.guardEnd)
         {
            if (modulation->correlatedSD > modulation->searchThreshold)
            {
               // max correlation peak detector
               if (modulation->correlatedSD > modulation->correlationPeek)
               {
                  modulation->searchPulseWidth++;
                  modulation->searchPeakTime = signalClock;
                  modulation->searchEndTime = signalClock + bitrate->period4SymbolSamples;
                  modulation->correlationPeek = modulation->correlatedSD;
               }
            }

            // Check for SoF symbol
            if (signalClock == modulation->searchEndTime)
            {
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
               decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.75f);
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

               // reset search status
               modulation->searchStartTime = 0;
               modulation->searchEndTime = 0;
               modulation->correlationPeek = 0;
               modulation->searchPulseWidth = 0;
               modulation->correlatedSD = 0;
            }
         }

         // capture signal variance as lower level threshold
         if (signalClock == frameStatus.guardEnd)
            modulation->searchThreshold = signalStatus.signalVariance;

         // frame waiting time exceeded
         if (signalClock == frameStatus.waitingEnd)
         {
            pattern = PatternType::NoPattern;
            break;
         }
      }

         // search Response Bit Stream
      else
      {
         // set next search sync window from previous
         if (!modulation->searchStartTime)
         {
            // estimated symbol start and end
            modulation->symbolStartTime = modulation->symbolEndTime;
            modulation->symbolEndTime = modulation->symbolStartTime + bitrate->period1SymbolSamples;

            // timig search window
            modulation->searchStartTime = modulation->symbolEndTime - bitrate->period8SymbolSamples;
            modulation->searchEndTime = modulation->symbolEndTime + bitrate->period8SymbolSamples;

            // reset symbol parameters
            modulation->symbolCorr0 = 0;
            modulation->symbolCorr1 = 0;
         }

         // search symbol timmings
         if (signalClock >= modulation->searchStartTime && signalClock <= modulation->searchEndTime)
         {
            if (modulation->correlatedSD > modulation->correlationPeek)
            {
               modulation->correlationPeek = modulation->correlatedSD;
               modulation->symbolCorr0 = modulation->correlatedS0;
               modulation->symbolCorr1 = modulation->correlatedS1;
               modulation->symbolEndTime = signalClock;
            }
         }

         // capture next symbol
         if (signalClock == modulation->searchEndTime)
         {
#ifdef DEBUG_ASK_SYNCHRONIZATION_CHANNEL
            decoderDebug->value(DEBUG_ASK_SYNCHRONIZATION_CHANNEL, 0.50f);
#endif
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

int NfcDecoder::Impl::decodeSymbolTagBpskNfcA(sdr::SignalBuffer &buffer)
{
   int pattern = PatternType::Invalid;

   while (nextSample(buffer))
   {
      modulation->signalIndex = (bitrate->offsetSignalIndex + signalClock);
      modulation->symbolIndex = (bitrate->offsetSymbolIndex + signalClock);
      modulation->detectIndex = (bitrate->offsetDetectIndex + signalClock);

      // get signal samples
      float currentSample = signalStatus.signalData[modulation->signalIndex & (SignalBufferLength - 1)];
      float delayedSample = signalStatus.signalData[modulation->symbolIndex & (SignalBufferLength - 1)];

      // compute symbol average
      modulation->symbolAverage = modulation->symbolAverage * bitrate->symbolAverageW0 + currentSample * bitrate->symbolAverageW1;

      // multiply 1 symbol delayed signal with incoming signal
      float phase = (currentSample - modulation->symbolAverage) * (delayedSample - modulation->symbolAverage);

      // store signal phase in filter buffer
      modulation->integrationData[modulation->signalIndex & (SignalBufferLength - 1)] = phase * 10;

      // integrate response from PICC after guard time (TR0)
      if (signalClock > (frameStatus.guardEnd - bitrate->period1SymbolSamples))
      {
         modulation->phaseIntegrate += modulation->integrationData[modulation->signalIndex & (SignalBufferLength - 1)]; // add new value
         modulation->phaseIntegrate -= modulation->integrationData[modulation->detectIndex & (SignalBufferLength - 1)]; // remove delayed value
      }

#ifdef DEBUG_BPSK_PHASE_INTEGRATION_CHANNEL
      decoderDebug->value(DEBUG_BPSK_PHASE_INTEGRATION_CHANNEL, modulation->phaseIntegrate);
#endif

#ifdef DEBUG_BPSK_PHASE_DEMODULATION_CHANNEL
      decoderDebug->value(DEBUG_BPSK_PHASE_DEMODULATION_CHANNEL, phase * 10);
#endif
      // search for Start Of Frame pattern (SoF)
      if (!modulation->symbolEndTime)
      {
         // detect first zero-cross
         if (modulation->phaseIntegrate > 0.00025f)
         {
            modulation->searchPeakTime = signalClock;
            modulation->searchEndTime = signalClock + bitrate->period2SymbolSamples;
         }

         if (signalClock == modulation->searchEndTime)
         {
#ifdef DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL
            decoderDebug->value(DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL, 0.75);
#endif
            // set symbol window
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

            // frame waiting time exceeded
         else if (signalClock == frameStatus.waitingEnd)
         {
            pattern = PatternType::NoPattern;
            break;
         }
      }

         // search Response Bit Stream
      else
      {
         // edge detector for re-synchronization
         if ((modulation->phaseIntegrate > 0 && modulation->symbolPhase < 0) || (modulation->phaseIntegrate < 0 && modulation->symbolPhase > 0))
         {
            modulation->searchPeakTime = signalClock;
            modulation->searchEndTime = signalClock + bitrate->period2SymbolSamples;
            modulation->symbolStartTime = signalClock;
            modulation->symbolEndTime = signalClock + bitrate->period1SymbolSamples;
            modulation->symbolPhase = modulation->phaseIntegrate;
         }

         // set next search sync window from previous
         if (!modulation->searchEndTime)
         {
            // estimated symbol start and end
            modulation->symbolStartTime = modulation->symbolEndTime;
            modulation->symbolEndTime = modulation->symbolStartTime + bitrate->period1SymbolSamples;

            // timig next symbol
            modulation->searchEndTime = modulation->symbolStartTime + bitrate->period2SymbolSamples;
         }

            // search symbol timmings
         else if (signalClock == modulation->searchEndTime)
         {
#ifdef DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL
            decoderDebug->value(DEBUG_BPSK_PHASE_SYNCHRONIZATION_CHANNEL, 0.5);
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
      modulation->correlationPeek = 0;
      modulation->searchPulseWidth = 0;
      modulation->correlatedSD = 0;
   }

   return pattern;
}

void NfcDecoder::Impl::resetFrameSearch()
{
   // reset frame search status
   if (modulation)
   {
      modulation->symbolEndTime = 0;
      modulation->searchPeakTime = 0;
      modulation->searchEndTime = 0;
      modulation->correlationPeek = 0;
   }

   // reset frame start time
   frameStatus.frameStart = 0;
}

void NfcDecoder::Impl::resetModulation()
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
   bitrate = nullptr;

   // restore modulation
   modulation = nullptr;
}

bool NfcDecoder::Impl::nextSample(sdr::SignalBuffer &buffer)
{
   if (buffer.available() == 0)
      return false;

   // real-value signal
   if (buffer.stride() == 1)
   {
      // read next sample data
      buffer.get(signalStatus.signalValue);
   }

      // IQ channel signal
   else
   {
      // read next sample data
      buffer.get(signalStatus.sampleData, 2);

      // compute magnitude from IQ channels
      auto i = double(signalStatus.sampleData[0]);
      auto q = double(signalStatus.sampleData[1]);

      signalStatus.signalValue = sqrtf(i * i + q * q);
   }

   // update signal clock
   signalClock++;

   // compute power average (exponential average)
   signalStatus.powerAverage = signalStatus.powerAverage * signalParams.powerAverageW0 + signalStatus.signalValue * signalParams.powerAverageW1;

   // compute signal average (exponential average)
   signalStatus.signalAverage = signalStatus.signalAverage * signalParams.signalAverageW0 + signalStatus.signalValue * signalParams.signalAverageW1;

   // compute signal variance (exponential variance)
   signalStatus.signalVariance = signalStatus.signalVariance * signalParams.signalVarianceW0 + std::abs(signalStatus.signalValue - signalStatus.signalAverage) * signalParams.signalVarianceW1;

   // store next signal value in sample buffer
   signalStatus.signalData[signalClock & (SignalBufferLength - 1)] = signalStatus.signalValue;

#ifdef DEBUG_SIGNAL
   decoderDebug->block(signalClock);
#endif

#ifdef DEBUG_SIGNAL_VALUE_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_VALUE_CHANNEL, signalStatus.signalValue);
#endif

#ifdef DEBUG_SIGNAL_POWER_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_POWER_CHANNEL, signalStatus.powerAverage);
#endif

#ifdef DEBUG_SIGNAL_AVERAGE_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_AVERAGE_CHANNEL, signalStatus.signalAverage);
#endif

#ifdef DEBUG_SIGNAL_VARIANCE_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_VARIANCE_CHANNEL, signalStatus.signalVariance);
#endif

#ifdef DEBUG_SIGNAL_EDGE_CHANNEL
   decoderDebug->value(DEBUG_SIGNAL_EDGE_CHANNEL, signalStatus.signalAverage - signalStatus.powerAverage);
#endif

   return true;
}

void NfcDecoder::Impl::process(NfcFrame frame)
{
   while (true)
   {
      if (processREQA(frame))
         break;

      if (processHLTA(frame))
         break;

      if (!(chainedFlags & NfcFrame::Encrypted))
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
         frame.setFramePhase(NfcFrame::ApplicationFrame);
      }

      break;
   }

   // set chained flags
   frame.setFrameFlags(chainedFlags);

   // for request frame set response timings
   if (frame.isRequestFrame())
   {
      // response guard time TR0min (PICC must not modulate reponse within this period)
      frameStatus.guardEnd = frameStatus.frameEnd + frameStatus.frameGuardTime + bitrate->symbolDelayDetect;

      // response delay time WFT (PICC must reply to command before this period)
      frameStatus.waitingEnd = frameStatus.frameEnd + frameStatus.frameWaitingTime + bitrate->symbolDelayDetect;

      // next frame must be ListenFrame
      frameStatus.frameType = ListenFrame;
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

   // reset farme start
   frameStatus.frameStart = 0;

   // reset farme end
   frameStatus.frameEnd = 0;
}

bool NfcDecoder::Impl::processREQA(NfcFrame &frame)
{
   if (frame.isRequestFrame())
   {
      if ((frame[0] == NFCA_REQA || frame[0] == NFCA_WUPA) && frame.limit() == 1)
      {
         frame.setFramePhase(NfcFrame::SelectionFrame);

         frameStatus.lastCommand = frame[0];
         frameStatus.frameGuardTime = signalParams.sampleTimeUnit * 128 * 7;
         frameStatus.frameWaitingTime = signalParams.sampleTimeUnit * 128 * 18;
         frameStatus.maxFrameSize = 256;

         // clear chained flags
         chainedFlags = 0;

         return true;
      }
   }

   if (frame.isResponseFrame())
   {
      if (frameStatus.lastCommand == NFCA_REQA || frameStatus.lastCommand == NFCA_WUPA)
      {
         frame.setFramePhase(NfcFrame::SelectionFrame);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processHLTA(NfcFrame &frame)
{
   if (frame.isRequestFrame())
   {
      if (frame[0] == NFCA_HLTA && frame.limit() == 4)
      {
         frame.setFramePhase(NfcFrame::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);

         frameStatus.lastCommand = frame[0];
         frameStatus.frameGuardTime = signalParams.sampleTimeUnit * 128 * 7;
         frameStatus.frameWaitingTime = signalParams.sampleTimeUnit * 128 * 18;
         frameStatus.maxFrameSize = 256;

         // clear chained flags
         chainedFlags = 0;

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processSELn(NfcFrame &frame)
{
   if (frame.isRequestFrame())
   {
      if (frame[0] == NFCA_SEL1 || frame[0] == NFCA_SEL2 || frame[0] == NFCA_SEL3)
      {
         frame.setFramePhase(NfcFrame::SelectionFrame);

         frameStatus.lastCommand = frame[0];
         frameStatus.frameGuardTime = signalParams.sampleTimeUnit * 128 * 7;
         frameStatus.frameWaitingTime = signalParams.sampleTimeUnit * 128 * 18;
         frameStatus.maxFrameSize = 256;

         return true;
      }
   }

   if (frame.isResponseFrame())
   {
      if (frameStatus.lastCommand == NFCA_SEL1 || frameStatus.lastCommand == NFCA_SEL2 || frameStatus.lastCommand == NFCA_SEL3)
      {
         frame.setFramePhase(NfcFrame::SelectionFrame);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processRATS(NfcFrame &frame)
{
   // capture parameters from ATS and reconfigure decoder timmings
   if (frame.isRequestFrame())
   {
      if (frame[0] == NFCA_RATS)
      {
         auto fsdi = (frame[1] >> 4) & 0x0F;

         frameStatus.lastCommand = frame[0];
         frameStatus.maxFrameSize = TABLE_FDS[fsdi];
         frameStatus.frameWaitingTime = signalParams.sampleTimeUnit * 256 * 16 * (1 << 14);

         log.info("RATS frame parameters");
         log.info("  maxFrameSize {} bytes", {frameStatus.maxFrameSize});

         frame.setFramePhase(NfcFrame::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);

         return true;
      }
   }

   if (frame.isResponseFrame())
   {
      if (frameStatus.lastCommand == NFCA_RATS)
      {
         auto offset = 0;
         auto tl = frame[offset++];

         if (tl > 0)
         {
            auto t0 = frame[offset++];

            // if TA is transmitted, skip..
            if (t0 & 0x10)
               offset++;

            // if TB is transmitted capture timming parameters
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

               // calculate timming parameters
               frameStatus.startUpGuardTime = int(256 * 16 * signalParams.sampleTimeUnit * (1 << sfgi));
               frameStatus.frameWaitingTime = int(256 * 16 * signalParams.sampleTimeUnit * (1 << fwi));

               log.info("ATS timming parameters");
               log.info("  startUpGuardTime {} samples ({} us)", {frameStatus.startUpGuardTime, 1000000.0 * frameStatus.startUpGuardTime / sampleRate});
               log.info("  frameWaitingTime {} samples ({} us)", {frameStatus.frameWaitingTime, 1000000.0 * frameStatus.frameWaitingTime / sampleRate});
            }
         }

         frame.setFramePhase(NfcFrame::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processPPSr(NfcFrame &frame)
{
   if (frame.isRequestFrame())
   {
      if ((frame[0] & 0xF0) == NFCA_PPS)
      {
         frameStatus.lastCommand = frame[0] & 0xF0;
         frameStatus.frameWaitingTime = signalParams.sampleTimeUnit * 256 * 16 * (1 << 14);

         frame.setFramePhase(NfcFrame::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);

         return true;
      }
   }

   if (frame.isResponseFrame())
   {
      if (frameStatus.lastCommand == NFCA_PPS)
      {
         frame.setFramePhase(NfcFrame::SelectionFrame);
         frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processAUTH(NfcFrame &frame)
{
   if (frame.isRequestFrame())
   {
      if (frame[0] == NFCA_AUTH1 || frame[0] == NFCA_AUTH2)
      {
         frameStatus.lastCommand = frame[0];
         frameStatus.frameWaitingTime = signalParams.sampleTimeUnit * 256 * 16 * (1 << 14);

         frame.setFramePhase(NfcFrame::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);

//      if (!frameStatus.lastCommand)
//      {
//         crypto1_init(&frameStatus.crypto1, 0x521284223154);
//      }

         return true;
      }
   }

   if (frame.isResponseFrame())
   {
      if (frameStatus.lastCommand == NFCA_AUTH1 || frameStatus.lastCommand == NFCA_AUTH2)
      {
         //      unsigned int uid = 0x1e47fc35;
//      unsigned int nc = frame[0] << 24 || frame[1] << 16 || frame[2] << 8 || frame[0];
//
//      crypto1_word(&frameStatus.crypto1, uid ^ nc, 0);

         // set chained flags

         chainedFlags = NfcFrame::Encrypted;

         frame.setFramePhase(NfcFrame::ApplicationFrame);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processIBlock(NfcFrame &frame)
{
   if (frame.isRequestFrame())
   {
      if ((frame[0] & 0xE2) == NFCA_IBLOCK)
      {
         frameStatus.lastCommand = frame[0] & 0xE2;
         frameStatus.frameWaitingTime = signalParams.sampleTimeUnit * 256 * 16 * (1 << 14);

         frame.setFramePhase(NfcFrame::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);

         return true;
      }
   }

   if (frame.isResponseFrame())
   {
      if (frameStatus.lastCommand == NFCA_IBLOCK)
      {
         frame.setFramePhase(NfcFrame::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processRBlock(NfcFrame &frame)
{
   if (frame.isRequestFrame())
   {
      if ((frame[0] & 0xE6) == NFCA_RBLOCK)
      {
         frameStatus.lastCommand = frame[0] & 0xE6;
         frameStatus.frameWaitingTime = signalParams.sampleTimeUnit * 256 * 16 * (1 << 14);

         frame.setFramePhase(NfcFrame::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);

         return true;
      }
   }

   if (frame.isResponseFrame())
   {
      if (frameStatus.lastCommand == NFCA_RBLOCK)
      {
         frame.setFramePhase(NfcFrame::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);

         return true;
      }
   }

   return false;
}

bool NfcDecoder::Impl::processSBlock(NfcFrame &frame)
{
   if (frame.isRequestFrame())
   {
      if ((frame[0] & 0xC7) == NFCA_SBLOCK)
      {
         frameStatus.lastCommand = frame[0] & 0xC7;
         frameStatus.frameWaitingTime = signalParams.sampleTimeUnit * 256 * 16 * (1 << 14);

         frame.setFramePhase(NfcFrame::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);

         return true;
      }
   }

   if (frame.isResponseFrame())
   {
      if (frameStatus.lastCommand == NFCA_SBLOCK)
      {
         frame.setFramePhase(NfcFrame::ApplicationFrame);
         frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);

         return true;
      }
   }

   return false;
}

void NfcDecoder::Impl::processOther(NfcFrame &frame)
{
   if (frame.isRequestFrame())
   {
      frameStatus.frameWaitingTime = signalParams.sampleTimeUnit * 256 * 16 * (1 << 14);
   }

   frame.setFramePhase(NfcFrame::ApplicationFrame);
   frame.setFrameFlags(!checkCrc(frame) ? NfcFrame::CrcError : 0);
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
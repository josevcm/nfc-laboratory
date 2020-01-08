/*

  Copyright (c) 2020 Jose Vicente Campos Martinez - <josevcm@gmail.com>

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

#include <math.h>

#include <QtCore>

#include <devices/SignalDevice.h>
#include <devices/RecordDevice.h>

#include "NfcDecoder.h"

#define RECORD_SIGNAL 0
#define RECORD_S0 1
#define RECORD_S1 2
#define RECORD_SD 3
#define RECORD_POWER 4
#define RECORD_AVERAGE 5
#define RECORD_PHASE 1
#define RECORD_BPSK 2

// https://www.allaboutcircuits.com/textbook/radio-frequency-analysis-design/

NfcDecoder::NfcDecoder(SignalDevice *device, RecordDevice *record, QObject *parent) :
   QObject(parent),
   mSignalDevice(device),
   mRecordDevice(record),
   mSampleBuffer(SampleBuffer<int>::IQ, SampleBufferLength, 2),
   mSampleSize(0),
   mSampleRate(0),
   mSampleCount(0),
   mSignalClock(0),
   mRecorder(nullptr)
{
   qInfo() << "create frame decoder";

   // get device sampling parameters
   mSampleSize = device->sampleSize();
   mSampleRate = device->sampleRate();

   // setup signal detection parameters
   mPowerLevelThreshold = 0.050f;
   mModulationThreshold = 0.850f;
   mMaximunFrameLength = 256;

   // invalidate buffer to prepare first read
   mSampleBuffer.flip();

   // clear frame status
   memset(&mFrameSearch, 0, sizeof(struct FrameInfo));

   // clear signal status
   memset(&mSignalInfo, 0, sizeof(struct SignalInfo));

   // clear symbol status
   memset(&mSymbolInfo, 0, sizeof(struct SymbolInfo));

   // calculate sample time unit
   mSignalInfo.sampleTimeUnit = double(mSampleRate) / double(BaseFrequency);

   // calculate default timming parameters
   mSignalInfo.defaultFrameGuardTime = int(round(mSignalInfo.sampleTimeUnit * (1 << 10))); // TR0min (1024 in specification)
   mSignalInfo.defaultFrameWaitingTime = int(round(mSignalInfo.sampleTimeUnit * (1 << 16))); // WFT (65536 in specification)

   // initialize timming parameters to default values
   mSignalInfo.frameGuardTime = mSignalInfo.defaultFrameGuardTime;
   mSignalInfo.frameWaitingTime = mSignalInfo.defaultFrameWaitingTime;

   qInfo().noquote() << "";
   qInfo().noquote() << QString("default decoder parameters");
   qInfo().noquote() << QString("  sampleRate           %1").arg(mSampleRate);
   qInfo().noquote() << QString("  sampleSize           %1 bits").arg(mSampleSize);
   qInfo().noquote() << QString("  powerLevelThreshold  %1").arg(double(mPowerLevelThreshold), -1, 'f', 3);
   qInfo().noquote() << QString("  modulationThreshold  %1").arg(double(mModulationThreshold), -1, 'f', 3);
   qInfo().noquote() << QString("  maximunFrameLength   %1 bytes").arg(mMaximunFrameLength);
   qInfo().noquote() << QString("  frameGuardTime       %1 samples (%2 us)").arg(mSignalInfo.frameGuardTime).arg(1000000.0 * mSignalInfo.frameGuardTime / mSampleRate, -1, 'f', 0);
   qInfo().noquote() << QString("  frameWaitingTime     %1 samples (%2 us)").arg(mSignalInfo.frameWaitingTime).arg(1000000.0 * mSignalInfo.frameWaitingTime / mSampleRate, -1, 'f', 0);
   qInfo().noquote() << "";

   // compute symbol parameters for 106Kbps, 212Kbps, 424Kbps and 848Kbps
   for (int rate = r106k; rate <= r848k; rate++)
   {
      struct DecodeInfo *decode = mDecodeInfo + rate;

      memset(decode, 0, sizeof(struct DecodeInfo));

      // set rate type
      decode->rateType = rate;

      // symbol timming parameters
      decode->symbolsPerSecond = BaseFrequency / (128 >> rate);

      // correlator timming parameters
      decode->period1SymbolSamples = int(round(mSignalInfo.sampleTimeUnit * (128 >> rate)));
      decode->period2SymbolSamples = int(round(mSignalInfo.sampleTimeUnit * (64 >> rate)));
      decode->period4SymbolSamples = int(round(mSignalInfo.sampleTimeUnit * (32 >> rate)));
      decode->period8SymbolSamples = int(round(mSignalInfo.sampleTimeUnit * (16 >> rate)));

      decode->symbolDelayDetect = rate > r106k ? mDecodeInfo[rate - 1].symbolDelayDetect + mDecodeInfo[rate - 1].period1SymbolSamples: 0;

      // moving average offsets
      decode->offsetSignalIndex = SignalBufferLength - decode->symbolDelayDetect;
      decode->offsetFilterIndex = SignalBufferLength - decode->symbolDelayDetect - decode->period2SymbolSamples;
      decode->offsetSymbolIndex = SignalBufferLength - decode->symbolDelayDetect - decode->period1SymbolSamples;
      decode->offsetDetectIndex = SignalBufferLength - decode->symbolDelayDetect - decode->period4SymbolSamples;

      qInfo().noquote() << QString("%1 kpbs parameters:").arg(decode->symbolsPerSecond / 1000.0, 3, 'f', 0);
      qInfo().noquote() << QString("  symbolsPerSecond     %1").arg(decode->symbolsPerSecond);
      qInfo().noquote() << QString("  period1SymbolSamples %1 (%2 us)").arg(decode->period1SymbolSamples).arg(1000000.0 * decode->period1SymbolSamples / mSampleRate, -1, 'f', 0);
      qInfo().noquote() << QString("  period2SymbolSamples %1 (%2 us)").arg(decode->period2SymbolSamples).arg(1000000.0 * decode->period2SymbolSamples / mSampleRate, -1, 'f', 0);
      qInfo().noquote() << QString("  period4SymbolSamples %1 (%2 us)").arg(decode->period4SymbolSamples).arg(1000000.0 * decode->period4SymbolSamples / mSampleRate, -1, 'f', 0);
      qInfo().noquote() << QString("  period8SymbolSamples %1 (%2 us)").arg(decode->period8SymbolSamples).arg(1000000.0 * decode->period8SymbolSamples / mSampleRate, -1, 'f', 0);
      qInfo().noquote() << QString("  symbolDelayDetect    %1 (%2 us)").arg(decode->symbolDelayDetect).arg(1000000.0 * decode->symbolDelayDetect / mSampleRate, -1, 'f', 0);
      qInfo().noquote() << QString("  offsetSignalIndex    %1").arg(decode->offsetSignalIndex);
      qInfo().noquote() << QString("  offsetFilterIndex    %1").arg(decode->offsetFilterIndex);
      qInfo().noquote() << QString("  offsetSymbolIndex    %1").arg(decode->offsetSymbolIndex);
      qInfo().noquote() << QString("  offsetDetectIndex    %1").arg(decode->offsetDetectIndex);
      qInfo().noquote() << "";
   }

   // initialize exponential avegare factors for power value
   mSignalInfo.powerAverageW0 = float(1 - 1000.0 / mSampleRate);
   mSignalInfo.powerAverageW1 = float(1 - mSignalInfo.powerAverageW0);

   // initialize exponential avegare factors for signal average
   mSignalInfo.signalAverageW0 = float(1 - 100000.0 / mSampleRate);
   mSignalInfo.signalAverageW1 = float(1 - mSignalInfo.signalAverageW0);

   // initialize exponential avegare factors for signal variance
   mSignalInfo.signalVarianceW0 = float(1 - 100000.0 / mSampleRate);
   mSignalInfo.signalVarianceW1 = float(1 - mSignalInfo.signalVarianceW0);

   // set recorder
   if (mRecordDevice)
   {
      mRecorder = QSharedPointer<NfcDecoder::Registry>(new NfcDecoder::Registry(record));
   }
}

NfcDecoder::~NfcDecoder()
{
   qInfo() << "finish frame decoder";
}

/**
 * Extract next frame
 */
NfcFrame NfcDecoder::nextFrame(long timeout)
{
   QTime timer;

   NfcFrame frame = NfcFrame::Nil;

   // current pattern
   int pattern;

   // previous pattern
   int previous = PatternType::Invalid;

   // receiver data processing
   int value = 0, data = 0, bits = 0, stream = 0, parity = 1;

   // reset frame search mode
   int mode = DecodeMode::FrameBegin;

   // capture start time
   double timeStart = double(mSignalClock) / double(mSampleRate);

   // start timeout
   timer.start();

   // find next frame
   while ((pattern = nextPattern(mode, timeout)))
   {
      // no pattern... restart search
      if (pattern == PatternType::NoPattern || pattern == PatternType::NoCarrier)
      {
         data = 0;
         bits = 0;
         stream = 0;
         parity = 1;

         frame = NfcFrame::Nil;
         previous = PatternType::Invalid;
         mode = DecodeMode::FrameBegin;

         if (timer.elapsed() > timeout)
            break;

         continue;
      }

      // initialize frame on first symbol
      if (!frame)
      {
         // first frame symbol for ASK request must be Pattern Z, logic "0" (SoF)
         if (pattern == PatternType::PatternZ)
         {
            frame = NfcFrame(NfcFrame::NfcA, NfcFrame::RequestFrame);
            frame.setSampleStart(mSymbolInfo.start);
            frame.setTimeStart(double(mSymbolInfo.start) / double(mSampleRate));
            frame.setFrameRate(mFrameSearch.decode->symbolsPerSecond);

            // search for first bit (must not be Pattern-Y)
            mode = DecodeMode::BitStart;
         }
         // first frame symbol for Manchester responses must be Pattern D, logic "1" (SoF)
         else if (pattern == PatternType::PatternD)
         {
            frame = NfcFrame(NfcFrame::NfcA, NfcFrame::ResponseFrame);
            frame.setSampleStart(mSymbolInfo.start);
            frame.setTimeStart(double(mSymbolInfo.start) / double(mSampleRate));
            frame.setFrameRate(mFrameSearch.decode->symbolsPerSecond);

            // search for first bit
            mode = DecodeMode::BitStart;
         }
         // first frame symbol for BPSK responses must be Pattern M, logic "0" (SoF)
         else if (pattern == PatternType::PatternM)
         {
            frame = NfcFrame(NfcFrame::NfcA, NfcFrame::ResponseFrame);
            frame.setSampleStart(mSymbolInfo.start);
            frame.setTimeStart(double(mSymbolInfo.start) / double(mSampleRate));
            frame.setFrameRate(mFrameSearch.decode->symbolsPerSecond);

            // search for first bit
            mode = DecodeMode::BitStream;
         }

         data = 0;
         bits = 0;
         stream = 0;
         parity = 1;

         previous = PatternType::Invalid;

         continue;
      }

      if (previous != PatternType::Invalid)
      {
         // detect end of frame (Pattern-Y after Pattern-Z or Pattern-Y or Pattern-O)
         if (((previous == PatternType::PatternY || previous == PatternType::PatternZ) && pattern == PatternType::PatternY) || (pattern == PatternType::PatternF) || (pattern == PatternType::PatternO))
         {
            // add remaining byte to frame
            if (bits >= 7)
               frame.put(data);

            // frames must contains at least one full byte or 7 bits for short frames
            if (!frame.isEmpty())
            {
               frame.setSampleEnd(mSymbolInfo.end);
               frame.setTimeEnd(double(mSymbolInfo.end) / double(mSampleRate));

               if(pattern == PatternType::PatternY)
               {
                  // detect short frame
                  if (frame.length() == 1 && bits == 7)
                     frame.setFrameFlags(NfcFrame::ShortFrame);

                  // set next frame type to search
                  mFrameSearch.type = FrameType::ResponseFrame;

                  // exit search loop
                  break;
               }

               if(pattern == PatternType::PatternF)
               {
                  // set next frame type to search
                  mFrameSearch.type = FrameType::RequestFrame;

                  // exit search loop
                  break;
               }

               if(pattern == PatternType::PatternO)
               {
                  // set next frame type to search
                  mFrameSearch.type = FrameType::RequestFrame;

                  // exit search loop
                  break;
               }
            }

            // reset receive data
            data = 0;
            bits = 0;
            stream = 0;
            parity = 1;

            frame = NfcFrame::Nil;
            previous = PatternType::Invalid;
            mode = DecodeMode::FrameBegin;

            mFrameSearch.decode = nullptr;
            mFrameSearch.type = FrameType::RequestFrame;

            continue;
         }
         else
         {
            value = 0;

            value = value || (previous == PatternType::PatternX);
            value = value || (previous == PatternType::PatternD);
            value = value || (previous == PatternType::PatternN);

            if (bits < 8)
            {
               parity = parity ^ value;
               data = data | (value << bits++);
               stream++;
            }
            // check parity bit
            else
            {
               frame.put(data);

               if (value != parity)
                  frame.setFrameFlags(NfcFrame::ParityError);

               if (frame.length() == mMaximunFrameLength)
                  break;

               data = 0;
               bits = 0;
               parity = 1;
            }
         }
      }

      previous = pattern;

      mode = DecodeMode::BitStream;
   }

   // capture start time
   double timeEnd = double(mSignalClock) / double(mSampleRate);

   // end of file...
   if (pattern == PatternType::Invalid)
      return NfcFrame::Nil;

   // timeout width no signal...
   if (pattern == PatternType::NoCarrier)
      return NfcFrame(NfcFrame::TechType::None, NfcFrame::FrameType::NoSignal, timeStart, timeEnd);

   // timeout width no frame...
   if (pattern == PatternType::NoPattern)
      return NfcFrame(NfcFrame::TechType::None, NfcFrame::FrameType::NoFrame, timeStart, timeEnd);

   // process frame to search communication parameters
   process(frame);

   // and return frame
   return frame;
}

/**
 * Detect next frame pattern
 */
int NfcDecoder::nextPattern(int searchMode, long timeout)
{
//   QElapsedTimer timer;

   int pattern = PatternType::Invalid;

   for (int rate = r106k; rate <= r424k; rate++)
   {
      mDecodeInfo[rate].searchStartTime = 0;
      mDecodeInfo[rate].searchEndTime = 0;
      mDecodeInfo[rate].searchCeil = 0;

      if (searchMode == FrameBegin)
      {
         mDecodeInfo[rate].searchFrameType = 0;
         mDecodeInfo[rate].symbolPhase = NAN;
      }
   }

   // start timeout timer
//   timer.start();
   long deadline = mSignalClock + (timeout * mSampleRate / 1000);

   do
   {
      // update signal clock
      mSignalClock++;

      // fetch next block
      if (mSampleBuffer.isEmpty())
      {
         if (mSignalDevice->waitForReadyRead(50))
         {
            mSignalDevice->read(mSampleBuffer.reset());
            mSampleCount += mSampleBuffer.available();
         }
      }

      // if no more samples, exit
      if (mSampleBuffer.isEmpty())
         return PatternType::Invalid;

      // read next sample data
      mSampleBuffer.get(mSignalInfo.sampleData);

      // normalize sample between 0 and 1 (16 bit)
      double i = double(mSignalInfo.sampleData[0]);
      double q = double(mSignalInfo.sampleData[1]);

      // compute magnitude from IQ channels
      mSignalInfo.signalSample = sqrt(i * i + q * q);

      // compute power average (exponential average)
      mSignalInfo.powerAverage = mSignalInfo.powerAverage * mSignalInfo.powerAverageW0 + mSignalInfo.signalSample * mSignalInfo.powerAverageW1;

      // compute signal average (exponential average)
      mSignalInfo.signalAverage = mSignalInfo.signalAverage * mSignalInfo.signalAverageW0 + mSignalInfo.signalSample * mSignalInfo.signalAverageW1;

      // compute signal variance (exponential variance)
      mSignalInfo.signalVariance = mSignalInfo.signalVariance * mSignalInfo.signalVarianceW0 + abs(mSignalInfo.signalSample - mSignalInfo.signalAverage) * mSignalInfo.signalVarianceW1;

      // store next signal value in sample buffer
      mSignalInfo.signalData[mSignalClock & (SignalBufferLength - 1)] = mSignalInfo.signalSample;

      // record signal channel
      if (mRecorder)
         mRecorder->set(RECORD_SIGNAL, mSignalInfo.signalSample);

      // calculate integrations for 106Kbps, 212Kbps and 424Kbps
      for (int rate = r106k; rate <= r424k; rate++)
      {
         struct DecodeInfo *decode = mDecodeInfo + rate;

         // compute moving average signal offsets
         decode->signalIndex = (decode->offsetSignalIndex + mSignalClock);
         decode->filterIndex = (decode->offsetFilterIndex + mSignalClock);
         decode->symbolIndex = (decode->offsetSymbolIndex + mSignalClock);
         decode->detectIndex = (decode->offsetDetectIndex + mSignalClock);

         // integrate filter data (moving average)
         decode->filterIntegrate += mSignalInfo.signalData[decode->signalIndex & (SignalBufferLength - 1)]; // add new value
         decode->filterIntegrate -= mSignalInfo.signalData[decode->filterIndex & (SignalBufferLength - 1)]; // remove delayed value

         // integrate filter data (moving average)
         decode->symbolIntegrate += mSignalInfo.signalData[decode->signalIndex & (SignalBufferLength - 1)]; // add new value
         decode->symbolIntegrate -= mSignalInfo.signalData[decode->symbolIndex & (SignalBufferLength - 1)]; // remove delayed value

         // compute correlation points
         decode->filterPoint1 = (decode->signalIndex % decode->period1SymbolSamples);
         decode->filterPoint2 = (decode->signalIndex + decode->period2SymbolSamples) % decode->period1SymbolSamples;
         decode->filterPoint3 = (decode->signalIndex + decode->period1SymbolSamples - 1) % decode->period1SymbolSamples;

         // store integrated signal in correlation buffer
         decode->filterData[decode->filterPoint1] = decode->filterIntegrate;

         // compute correlation results for each symbol and distance
         decode->correlatedS0 = decode->filterData[decode->filterPoint1] - decode->filterData[decode->filterPoint2];
         decode->correlatedS1 = decode->filterData[decode->filterPoint2] - decode->filterData[decode->filterPoint3];
         decode->correlatedSD = fabs(decode->correlatedS0 - decode->correlatedS1) / decode->period2SymbolSamples;
      }

      // ignore low power signals
      if (mSignalInfo.powerAverage > mPowerLevelThreshold)
      {
         // search request frames
         if (mFrameSearch.type == FrameType::RequestFrame)
         {
            // ASK detector for symbol rate and SoF
            if (!mFrameSearch.decode)
            {
               // detect symbol for 106Kbps, 212Kbps and 424Kbps
               for (int rate = r106k; rate <= r424k && !pattern; rate++)
               {
                  struct DecodeInfo *decode = mDecodeInfo + rate;

                  if (mRecorder)
                  {
                     mRecorder->set(RECORD_S0, decode->correlatedS0 / decode->period2SymbolSamples);
                     mRecorder->set(RECORD_S1, decode->correlatedS1 / decode->period2SymbolSamples);
                     mRecorder->set(RECORD_SD, decode->correlatedSD);
                     mRecorder->set(RECORD_POWER, mSignalInfo.powerAverage);
                     mRecorder->set(RECORD_AVERAGE, mSignalInfo.signalAverage);
                  }

                  // search for Pattern-Z in PCD to PICC request
                  if (decode->correlatedSD > mSignalInfo.powerAverage * mModulationThreshold)
                  {
                     // max correlation peak detector
                     if (decode->correlatedSD > decode->searchCeil)
                     {
                        decode->searchCeil = decode->correlatedSD;
                        decode->searchPeakTime = mSignalClock;
                        decode->searchEndTime = mSignalClock + decode->period4SymbolSamples;
                     }
                  }

                  // Check for SoF symbol
                  if (mSignalClock == decode->searchEndTime)
                  {
                     // if Pattern-Z is detected, signaling Start Of Frame (PCD->PICC)
                     if (decode->searchCeil > mSignalInfo.powerAverage * mModulationThreshold)
                     {
                        // set frame type for rest of detection
                        decode->searchFrameType = FrameType::RequestFrame;

                        // set lower threshold to detect valid response pattern
                        decode->searchThreshold =  mSignalInfo.powerAverage * mModulationThreshold;

                        // set pattern search window
                        decode->symbolStartTime = decode->searchPeakTime - decode->period2SymbolSamples;
                        decode->symbolEndTime = decode->searchPeakTime + decode->period2SymbolSamples;

                        // lock symbol parameters
                        mFrameSearch.decode = decode;

                        // setup symbol info
                        mSymbolInfo.value = 0;
                        mSymbolInfo.start = decode->symbolStartTime - decode->symbolDelayDetect;
                        mSymbolInfo.end = decode->symbolEndTime - decode->symbolDelayDetect;
                        mSymbolInfo.length = mSymbolInfo.end - mSymbolInfo.start;

                        // return symbol start of frame
                        pattern = PatternType::PatternZ;
                     }
                     else
                     {
                        // invalid start of frame
                        pattern = PatternType::NoPattern;
                     }
                  }
               }
            }

            // ASK detector for frame bits
            else
            {
               struct DecodeInfo *decode = mFrameSearch.decode;

               if (mRecorder)
               {
                  mRecorder->set(RECORD_S0, decode->correlatedS0 / decode->period2SymbolSamples);
                  mRecorder->set(RECORD_S1, decode->correlatedS1 / decode->period2SymbolSamples);
                  mRecorder->set(RECORD_SD, decode->correlatedSD);
                  mRecorder->set(RECORD_POWER, mSignalInfo.powerAverage);
                  mRecorder->set(RECORD_AVERAGE, mSignalInfo.signalAverage);
               }

               // set next search sync window from previous state
               if (!decode->searchStartTime)
               {
                  // estimated symbol start and end
                  decode->symbolStartTime = decode->symbolEndTime;
                  decode->symbolEndTime = decode->symbolStartTime + decode->period1SymbolSamples;

                  // timig search window
                  decode->searchStartTime = decode->symbolEndTime - decode->period8SymbolSamples;
                  decode->searchEndTime = decode->symbolEndTime + decode->period8SymbolSamples;

                  // reset symbol parameters
                  decode->symbolCorr0 = 0;
                  decode->symbolCorr1 = 0;
               }

               // search symbol timmings
               if (mSignalClock >= decode->searchStartTime && mSignalClock <= decode->searchEndTime)
               {
                  if (decode->correlatedSD > decode->searchCeil)
                  {
                     decode->searchCeil = decode->correlatedSD;
                     decode->symbolCorr0 = decode->correlatedS0;
                     decode->symbolCorr1 = decode->correlatedS1;
                     decode->symbolEndTime = mSignalClock;
                  }
               }

               // capture next symbol
               if (mSignalClock == decode->searchEndTime)
               {
                  // Pattern-Y
                  if (decode->searchCeil < decode->searchThreshold)
                  {
                     // first bit could not be Pattern-Y
                     if (searchMode == BitStream)
                     {
                        // estimate symbol end (peak detection not valid due lack of modulation)
                        decode->symbolEndTime = decode->symbolStartTime + decode->period1SymbolSamples;

                        // response guard time TR0min (PICC must not modulate reponse within this period)
                        decode->responseGuard = decode->symbolStartTime + mSignalInfo.frameGuardTime;

                        // response delay time WFT (PICC must reply to command before this period)
                        decode->responseTimeout = decode->symbolStartTime + mSignalInfo.frameWaitingTime;

                        // setup symbol info
                        mSymbolInfo.value = 0;
                        mSymbolInfo.start = decode->symbolStartTime - decode->symbolDelayDetect;
                        mSymbolInfo.end = decode->symbolEndTime - decode->symbolDelayDetect;
                        mSymbolInfo.length = mSymbolInfo.end - mSymbolInfo.start;

                        pattern = PatternType::PatternY;
                     }
                     else
                     {
                        // first bit could not be Pattern Y...
                        pattern = PatternType::NoPattern;
                     }
                  }
                  else
                  {
                     // Pattern-Z
                     if (decode->symbolCorr0 > decode->symbolCorr1)
                     {
                        // setup symbol info
                        mSymbolInfo.value = 0;
                        mSymbolInfo.start = decode->symbolStartTime - decode->symbolDelayDetect;
                        mSymbolInfo.end = decode->symbolEndTime - decode->symbolDelayDetect;
                        mSymbolInfo.length = mSymbolInfo.end - mSymbolInfo.start;

                        // return symbol for bit "0"
                        pattern = PatternType::PatternZ;
                     }
                     // Pattern-X
                     else
                     {
                        // setup symbol info
                        mSymbolInfo.value = 1;
                        mSymbolInfo.start = decode->symbolStartTime - decode->symbolDelayDetect;
                        mSymbolInfo.end = decode->symbolEndTime - decode->symbolDelayDetect;
                        mSymbolInfo.length = mSymbolInfo.end - mSymbolInfo.start;

                        // return symbol for bit "1"
                        pattern = PatternType::PatternX;
                     }
                  }
               }
            }
         }

         // search response frames
         else
         {
            struct DecodeInfo *decode = mFrameSearch.decode;

            // ASK detector
            if (decode->rateType == RateType::r106k)
            {
               if (mRecorder)
               {
                  mRecorder->set(RECORD_S0, decode->correlatedS0 / decode->period2SymbolSamples);
                  mRecorder->set(RECORD_S1, decode->correlatedS1 / decode->period2SymbolSamples);
                  mRecorder->set(RECORD_SD, decode->correlatedSD);
                  mRecorder->set(RECORD_POWER, mSignalInfo.powerAverage);
                  mRecorder->set(RECORD_AVERAGE, mSignalInfo.signalAverage);
               }

               // Search Response Start Of Frame pattern (SoF)
               if (searchMode == FrameBegin)
               {
                  // search for Pattern-D in PICC to PCD after TR0 time
                  if (mSignalClock > decode->responseGuard)
                  {
                     if (decode->correlatedSD > decode->searchThreshold)
                     {
                        // max correlation peak detector
                        if (decode->correlatedSD > decode->searchCeil)
                        {
                           decode->searchPeakTime = mSignalClock;
                           decode->searchEndTime = mSignalClock + decode->period4SymbolSamples;
                           decode->searchCeil = decode->correlatedSD;
                           decode->searchPhase = decode->correlatedS1;
                        }
                     }
                  }
                  else if (mSignalClock == decode->responseGuard)
                  {
                     decode->searchThreshold = mSignalInfo.signalVariance * 5;
                  }

                  // Check for SoF symbol
                  if (mSignalClock == decode->searchEndTime)
                  {
                     // set frame type for rest of detection
                     decode->searchFrameType = FrameType::ResponseFrame;

                     // set pattern search window
                     decode->symbolStartTime = decode->searchPeakTime - decode->period2SymbolSamples;
                     decode->symbolEndTime = decode->searchPeakTime + decode->period2SymbolSamples;
                     decode->symbolPhase = decode->searchPhase;

                     // setup symbol info
                     mSymbolInfo.value = 1;
                     mSymbolInfo.start = decode->symbolStartTime - decode->symbolDelayDetect;
                     mSymbolInfo.end = decode->symbolEndTime - decode->symbolDelayDetect;
                     mSymbolInfo.length = mSymbolInfo.end - mSymbolInfo.start;

                     // return symbol start of frame
                     pattern = PatternType::PatternD;
                  }

                  // if detect ASK modulation switch to request frame detector
                  else if (mSignalInfo.signalSample < (mSignalInfo.powerAverage * (1 - mModulationThreshold)))
                  {
                     mFrameSearch.decode = nullptr;
                     mFrameSearch.type = RequestFrame;

                     decode->searchStartTime = 0;
                     decode->searchEndTime = 0;
                     decode->searchCeil = 0;
                  }

                  // if no symbol detected and timeout... exit
                  else if (mSignalClock > decode->responseTimeout)
                  {
                     // no valid signal found...
                     pattern = PatternType::NoPattern;
                  }
               }

               // Search Response Bit Stream
               else
               {
                  // set next search sync window from previous
                  if (!decode->searchStartTime)
                  {
                     // estimated symbol start and end
                     decode->symbolStartTime = decode->symbolEndTime;
                     decode->symbolEndTime = decode->symbolStartTime + decode->period1SymbolSamples;

                     // timig search window
                     decode->searchStartTime = decode->symbolEndTime - decode->period8SymbolSamples;
                     decode->searchEndTime = decode->symbolEndTime + decode->period8SymbolSamples;

                     // reset symbol parameters
                     decode->symbolCorr0 = 0;
                     decode->symbolCorr1 = 0;
                  }

                  // search symbol timmings
                  if (mSignalClock >= decode->searchStartTime && mSignalClock <= decode->searchEndTime)
                  {
                     if (decode->correlatedSD > decode->searchCeil)
                     {
                        decode->searchCeil = decode->correlatedSD;
                        decode->symbolCorr0 = decode->correlatedS0;
                        decode->symbolCorr1 = decode->correlatedS1;
                        decode->symbolEndTime = mSignalClock;
                     }
                  }

                  // capture next symbol
                  if (mSignalClock == decode->searchEndTime)
                  {
                     //                     if (decode->searchCeil > mSignalInfo.powerAverageValue * mResponseFrameThreshold)
                     if (decode->searchCeil > decode->searchThreshold)
                     {
                        // setup symbol info
                        mSymbolInfo.start = decode->symbolStartTime - decode->symbolDelayDetect;
                        mSymbolInfo.end = decode->symbolEndTime - decode->symbolDelayDetect;
                        mSymbolInfo.length = mSymbolInfo.end - mSymbolInfo.start;

                        if (decode->symbolCorr0 > decode->symbolCorr1)
                        {
                           // check detected symbol phase to set symbol type
                           if (decode->symbolPhase < 0)
                           {
                              mSymbolInfo.value = 0;
                              pattern = PatternType::PatternE;
                           }
                           else
                           {
                              mSymbolInfo.value = 1;
                              pattern = PatternType::PatternD;
                           }
                        }
                        else
                        {
                           // check detected symbol phase to set symbol type
                           if (decode->symbolPhase < 0)
                           {
                              mSymbolInfo.value = 1;
                              pattern = PatternType::PatternD;
                           }
                           else
                           {
                              mSymbolInfo.value = 0;
                              pattern = PatternType::PatternE;
                           }
                        }
                     }
                     else
                     {
                        // no valid signal found...
                        pattern = PatternType::PatternF;
                     }
                  }
               }
            }

            // BPSK detector
            else if (decode->rateType == RateType::r212k || decode->rateType == RateType::r424k)
            {
               // calculate offset to remove DC component
               float offset = decode->symbolIntegrate / decode->period1SymbolSamples;

               // multiply 1 symbol delayed signal with incoming signal
               float phase = (mSignalInfo.signalData[decode->signalIndex & (SignalBufferLength - 1)] - offset) * (mSignalInfo.signalData[decode->symbolIndex & (SignalBufferLength - 1)] - offset);

               // store signal phase in detect buffer
               mSignalInfo.detectData[decode->signalIndex & (SignalBufferLength - 1)] = phase;

               // integrate response from PICC after guard time (TR0)
               if (mSignalClock > decode->responseGuard)
               {
                  // integrate symbol phase (moving average)
                  decode->phaseIntegrate += mSignalInfo.detectData[decode->signalIndex & (SignalBufferLength - 1)]; // add new value
                  decode->phaseIntegrate -= mSignalInfo.detectData[decode->detectIndex & (SignalBufferLength - 1)]; // remove delayed value
               }
               else
               {
                  decode->phaseIntegrate = 0;
               }

               if (mRecorder)
               {
                  mRecorder->set(RECORD_PHASE, phase);
                  mRecorder->set(RECORD_BPSK, decode->phaseIntegrate);
                  mRecorder->set(RECORD_SD, 0);
               }

               // Search Response Start Of Frame pattern (SoF)
               if (searchMode == FrameBegin)
               {
                  // detect first zero-cross
                  if (decode->phaseIntegrate > 0.001f)
                  {
                     decode->searchPeakTime = mSignalClock;
                     decode->searchEndTime = mSignalClock + decode->period2SymbolSamples;
                  }

                  if (mSignalClock == decode->searchEndTime)
                  {
                     // set pattern search window
                     decode->symbolStartTime = decode->searchPeakTime;
                     decode->symbolEndTime = decode->searchPeakTime + decode->period1SymbolSamples;
                     decode->symbolPhase = decode->phaseIntegrate;

                     // setup symbol info
                     mSymbolInfo.value = 0;
                     mSymbolInfo.start = decode->symbolStartTime - decode->symbolDelayDetect;
                     mSymbolInfo.end = decode->symbolEndTime - decode->symbolDelayDetect;
                     mSymbolInfo.length = mSymbolInfo.end - mSymbolInfo.start;

                     pattern = PatternType::PatternM;
                  }

                  // if detect ASK modulation switch to request frame detector
                  else if (mSignalInfo.signalSample < (mSignalInfo.powerAverage * (1 - mModulationThreshold)))
                  {
                     mFrameSearch.decode = nullptr;
                     mFrameSearch.type = RequestFrame;

                     decode->searchStartTime = 0;
                     decode->searchEndTime = 0;
                     decode->searchCeil = 0;
                  }

                  // if no modulation detected... exit
                  else if (mSignalClock > decode->responseTimeout)
                  {
                     // no valid signal found...
                     pattern = PatternType::NoPattern;
                  }
               }
               else
               {
                  // edge detector for re-sincronization
                  if ((decode->phaseIntegrate > 0 && decode->symbolPhase < 0) || (decode->phaseIntegrate < 0 && decode->symbolPhase > 0))
                  {
                     decode->searchPeakTime = mSignalClock;
                     decode->searchEndTime = mSignalClock + decode->period2SymbolSamples;
                     decode->symbolStartTime = mSignalClock;
                     decode->symbolEndTime = mSignalClock + decode->period1SymbolSamples;
                     decode->symbolPhase = decode->phaseIntegrate;
                  }

                  // set next search sync window from previous
                  if (!decode->searchEndTime)
                  {
                     // estimated symbol start and end
                     decode->symbolStartTime = decode->symbolEndTime;
                     decode->symbolEndTime = decode->symbolStartTime + decode->period1SymbolSamples;

                     // timig next symbol
                     decode->searchEndTime = decode->symbolStartTime + decode->period2SymbolSamples;
                  }

                  // search symbol timmings
                  else if (mSignalClock == decode->searchEndTime)
                  {
                     decode->symbolPhase = decode->phaseIntegrate;

                     // setup symbol info
                     mSymbolInfo.start = decode->symbolStartTime - decode->symbolDelayDetect;
                     mSymbolInfo.end = decode->symbolEndTime - decode->symbolDelayDetect;
                     mSymbolInfo.length = mSymbolInfo.end - mSymbolInfo.start;

                     if (decode->phaseIntegrate > 0.001f)
                     {
                        pattern = decode->symbolPattern;
                     }
                     else if (decode->phaseIntegrate < -0.001f)
                     {
                        mSymbolInfo.value = !mSymbolInfo.value;

                        pattern = decode->symbolPattern == PatternType::PatternM ? PatternType::PatternN : PatternType::PatternM;
                     }
                     else
                     {
                        pattern = PatternType::PatternO;
                     }
                  }
               }
            }
            else
            {
               pattern = PatternType::NoPattern;
            }
         }
      }

      if (!pattern && mSignalClock > deadline)
      {
         pattern = PatternType::NoPattern;
      }

      if (mRecorder)
         mRecorder->commit();
   }
   while (!pattern);

   if (pattern ==  PatternType::NoCarrier || pattern ==  PatternType::NoPattern)
   {
//      if (mSignalInfo.signalAverage > mPowerLevelThreshold)
//         pattern = PatternType::NoPattern;
//      else
//         pattern = PatternType::NoCarrier;

      mFrameSearch.decode = nullptr;
      mFrameSearch.type = RequestFrame;
   }
   else if (mFrameSearch.decode)
   {
      mFrameSearch.decode->symbolPattern = pattern;
   }

   return pattern;
}

void NfcDecoder::setPowerLevelThreshold(float value)
{
   mPowerLevelThreshold = value;
}

float NfcDecoder::powerLevelThreshold() const
{
   return mPowerLevelThreshold;
}

void NfcDecoder::setModulationThreshold(float value)
{
   mModulationThreshold = value;
}

float NfcDecoder::modulationThreshold() const
{
   return mModulationThreshold;
}

void NfcDecoder::setMaximunFrameLength(int value)
{
   mMaximunFrameLength = value;
}

float NfcDecoder::maximunFrameLength() const
{
   return mMaximunFrameLength;
}

void NfcDecoder::process(NfcFrame frame)
{
   if (frame.isRequestFrame())
   {
      int cmd = frame[0];

      // Request Command, Type A
      if (cmd == 0x26)
         processREQA(frame);

      // HALT Command, Type A
      else if (cmd == 0x50)
         processHLTA(frame);

      // Wake-UP Command, Type A
      else if (cmd == 0x52)
         processWUPA(frame);

      // Mifare AUTH
      else if (cmd == 0x60 || cmd == 0x61)
         processAUTH(frame);

      // Select Command, Type A
      else if (cmd == 0x93 || cmd == 0x95 || cmd == 0x97)
         processSELn(frame);

      // Request for Answer to Select
      else if (cmd == 0xE0)
         processRATS(frame);

      // Protocol Parameter Selection
      else if ((cmd & 0xF0) == 0xD0)
         processPPSr(frame);

      // ISO-DEP protocol I-Block
      else if ((cmd & 0xE2) == 0x02)
         processIBlock(frame);

      // ISO-DEP protocol R-Block
      else if ((cmd & 0xE6) == 0xA2)
         processRBlock(frame);

      // ISO-DEP protocol S-Block
      else if ((cmd & 0xC7) == 0xC2)
         processSBlock(frame);

      // Other request frames...
      else
         processOther(frame);

      mFrameSearch.last = cmd;
   }
   else
   {
      int cmd = mFrameSearch.last;

      // Request Command, Type A
      if (cmd == 0x26)
         processREQA(frame);

      // HALT Command, Type A
      else if (cmd == 0x50)
         processHLTA(frame);

      // Wake-UP Command, Type A
      else if (cmd == 0x52)
         processWUPA(frame);

      // Mifare AUTH
      else if (cmd == 0x60 || cmd == 0x61)
         processAUTH(frame);

      // Select Command, Type A
      else if (cmd == 0x93 || cmd == 0x95 || cmd == 0x97)
         processSELn(frame);

      // Request for Answer to Select
      else if (cmd == 0xE0)
         processRATS(frame);

      // Protocol Parameter Selection
      else if ((cmd & 0xF0) == 0xD0)
         processPPSr(frame);

      // ISO-DEP protocol I-Block
      else if ((cmd & 0xE2) == 0x02)
         processIBlock(frame);

      // ISO-DEP protocol R-Block
      else if ((cmd & 0xE6) == 0xA2)
         processRBlock(frame);

      // ISO-DEP protocol S-Block
      else if ((cmd & 0xC7) == 0xC2)
         processSBlock(frame);

      // Other response frames...
      else
         processOther(frame);

      mFrameSearch.last = 0;
   }
}

void NfcDecoder::processREQA(NfcFrame frame)
{
   frame.setFramePhase(NfcFrame::SenseFrame);

   if ((mSignalInfo.frameGuardTime != mSignalInfo.defaultFrameGuardTime) || (mSignalInfo.frameWaitingTime != mSignalInfo.defaultFrameWaitingTime))
   {
      //      mSignalInfo.frameGuardTime = mSignalInfo.defaultFrameGuardTime;
      mSignalInfo.frameWaitingTime = mSignalInfo.defaultFrameWaitingTime;

      qInfo().noquote() << QString("restore timming parameters");
      qInfo().noquote() << QString("  frameGuardTime   %1 samples (%2 us)").arg(mSignalInfo.frameGuardTime).arg(1000000.0 * mSignalInfo.frameGuardTime / mSampleRate, -1, 'f', 0);
      qInfo().noquote() << QString("  frameWaitingTime %1 samples (%2 us)").arg(mSignalInfo.frameWaitingTime).arg(1000000.0 * mSignalInfo.frameWaitingTime / mSampleRate, -1, 'f', 0);
   }
}

void NfcDecoder::processHLTA(NfcFrame frame)
{
   frame.setFramePhase(NfcFrame::SenseFrame);
}

void NfcDecoder::processWUPA(NfcFrame frame)
{
   frame.setFramePhase(NfcFrame::SenseFrame);
}

void NfcDecoder::processSELn(NfcFrame frame)
{
   frame.setFramePhase(NfcFrame::SelectionFrame);
}

void NfcDecoder::processRATS(NfcFrame frame)
{
   frame.setFramePhase(NfcFrame::SelectionFrame);

   // capture parameters from ATS and reconfigure decoder timmings
   if (frame.isResponseFrame())
   {
      int offset = 0;
      int tl = frame[offset++];

      if (tl > 0)
      {
         int t0 = frame[offset++];

         // if TA is transmitted, skip..
         if (t0 & 0x10)
            offset++;

         // if TB is transmitted capture timming parameters
         if (t0 & 0x20)
         {
            int tb = frame[offset++];

            int sfgi = (tb & 0x0f);
            int fwi = (tb >> 4) & 0x0f;

            //         mSignalInfo.frameGuardTime = int(256 * 16 * mSignalInfo.sampleTimeUnit * (1 << sfgi));
            mSignalInfo.frameWaitingTime = int(256 * 16 * mSignalInfo.sampleTimeUnit * (1 << fwi));

            qInfo().noquote() << QString("ATS timming parameters");
            qInfo().noquote() << QString("  frameGuardTime   %1 samples (%2 us)").arg(mSignalInfo.frameGuardTime).arg(1000000.0 * mSignalInfo.frameGuardTime / mSampleRate, -1, 'f', 0);
            qInfo().noquote() << QString("  frameWaitingTime %1 samples (%2 us)").arg(mSignalInfo.frameWaitingTime).arg(1000000.0 * mSignalInfo.frameWaitingTime / mSampleRate, -1, 'f', 0);
         }
      }
   }
}

void NfcDecoder::processPPSr(NfcFrame frame)
{
   frame.setFramePhase(NfcFrame::SelectionFrame);
}

void NfcDecoder::processAUTH(NfcFrame frame)
{
   frame.setFramePhase(NfcFrame::InformationFrame);
}

void NfcDecoder::processIBlock(NfcFrame frame)
{
   frame.setFramePhase(NfcFrame::InformationFrame);
}

void NfcDecoder::processRBlock(NfcFrame frame)
{
   frame.setFramePhase(NfcFrame::InformationFrame);
}

void NfcDecoder::processSBlock(NfcFrame frame)
{
   frame.setFramePhase(NfcFrame::InformationFrame);
}

void NfcDecoder::processOther(NfcFrame frame)
{
   frame.setFramePhase(NfcFrame::InformationFrame);
}

bool NfcDecoder::checkCrc(NfcFrame frame)
{
   unsigned short crc = 0, res = 0;

   int length = frame.length();

   if (length <= 2)
      return false;

   if (frame.isNfcA())
      crc = 0x6363; // NFC-A ITU-V.41
   else if (frame.isNfcB())
      crc = 0xFFFF; // NFC-B ISO/IEC 13239

   for (int i = 0; i < length - 2; i++)
   {
      unsigned char d = (unsigned char)frame[i];

      d = (d ^ (unsigned int)(crc & 0xff));
      d = (d ^ (d << 4));

      crc = (crc >> 8) ^ ((unsigned short)(d << 8)) ^ ((unsigned short)(d << 3)) ^ ((unsigned short)(d >> 4));
   }

   if (frame.isNfcB())
      crc = ~crc;

   res |= ((unsigned int)frame[length - 2] & 0xff);
   res |= ((unsigned int)frame[length - 1] & 0xff) << 8;

   return res == crc;
}

/**
 * Return total sample count
 */
long NfcDecoder::sampleCount() const
{
   return mSampleCount;
}

/**
 * Return signal strength
 */
float NfcDecoder::signalStrength() const
{
   return float(mSignalInfo.powerAverage);
}

NfcDecoder::Registry::Registry(RecordDevice *recorder) :
   sample(0), offset(0), pending(false), recorder(recorder)
{
   if (recorder)
   {
      sample = recorder->sampleSize();
      channels = recorder->channelCount();
      offset = 1 << (sample - 1);
      memset(buffer, 0, sizeof(buffer));
   }
}

NfcDecoder::Registry::~Registry()
{
   commit();
}

void NfcDecoder::Registry::set(int channel, int value)
{
   void *p = buffer + channel * sample / 8;

   if (sample == 8)
      *((qint8*) (p)) = qToLittleEndian<quint8>(128 + value);
   else if (sample == 16)
      *((qint16*) (p)) = qToLittleEndian<quint16>(value);
   else if (sample == 32)
      *((qint32*) (p)) = qToLittleEndian<quint32>(value);

   pending = true;
}

void NfcDecoder::Registry::set(int channel, float value)
{
   this->set(channel, int(value * offset));
}

void NfcDecoder::Registry::set(int channel, double value)
{
   this->set(channel, int(value * offset));
}

void NfcDecoder::Registry::commit()
{
   if (recorder && pending)
   {
      recorder->write((char*) buffer, channels * sample / 8);

      memset(buffer, 0, sizeof(buffer));

      pending = false;
   }
}


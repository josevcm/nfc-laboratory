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

#ifndef NFCDECODER_H
#define NFCDECODER_H

#include <QObject>
#include <QPointer>

#include <devices/SampleBuffer.h>

#include "NfcFrame.h"

class SignalDevice;
class RecordDevice;
class NfcSymbol;

class NfcDecoder: public QObject
{
   public:

      // carrier frequency (13.56 Mhz)
      constexpr static const int BaseFrequency = 13.56E6;

      // buffer for signal receiver
      constexpr static const int SampleBufferLength = 4096;

      // buffer for signal integration, must be power of 2^n
      constexpr static const int SignalBufferLength = 256;

   private:

      enum RateType
      {
         None = -1, r106k = 0, r212k = 1, r424k = 2, r848k = 3
      };

      enum PatternType
      {
         Invalid = 0, NoCarrier = 1, NoPattern = 2, PatternX = 3, PatternY = 4, PatternZ = 5, PatternD = 6, PatternE = 7, PatternF = 8, PatternM = 9, PatternN = 10, PatternO = 11
      };

      enum FrameType
      {
         RequestFrame = 0, ResponseFrame = 1
      };

      enum DecodeMode
      {
         FrameBegin = 0, BitStart = 1, BitStream = 2
      };

      enum FrameCommand
      {
         NFCA_REQA = 0x26,
         NFCA_WUPA = 0x52,
         NFCA_SEL1 = 0x93,
         NFCA_SEL2 = 0x95,
         NFCA_SEL3 = 0x97,
         NFCA_RATS = 0xE0
      };

      struct SignalInfo
      {
            // raw sample data
            float sampleData[2];

            // signal normalized value
            float signalSample;

            // exponential power integrator
            float powerAverage;
            float powerAverageW0;
            float powerAverageW1;

            // exponential average integrator
            float signalAverage;
            float signalAverageW0;
            float signalAverageW1;

            // exponential variance integrator
            float signalVariance;
            float signalVarianceW0;
            float signalVarianceW1;

            // default decoder parameters
            double sampleTimeUnit;
            int defaultFrameGuardTime;
            int defaultFrameWaitingTime;

            // decoder parameters
            int frameGuardTime;
            int frameWaitingTime;

            // signal data buffers
            float signalData[SignalBufferLength];
            float detectData[SignalBufferLength];
      };

      struct DecodeInfo
      {
            int rateType;

            // symbol parameters
            int symbolsPerSecond;
            int period1SymbolSamples;
            int period2SymbolSamples;
            int period4SymbolSamples;
            int period8SymbolSamples;
            int symbolDelayDetect;
            int offsetSignalIndex;
            int offsetFilterIndex;
            int offsetSymbolIndex;
            int offsetDetectIndex;

            // symbol search status
            int searchFrameType;   // Request / Response
            int searchPeakTime;    // Peak for maximun correlation
            int searchStartTime;   // Start of symbol search window
            int searchEndTime;     // End of symbol search window
            float searchPhase; // Detected symbol phase
            float searchCeil;
            float searchThreshold;
            int responseTimeout;
            int responseGuard;

            // integration indexes
            int signalIndex;
            int filterIndex;
            int symbolIndex;
            int detectIndex;

            // correlator indexes
            int filterPoint1;
            int filterPoint2;
            int filterPoint3;

            // integrator processor
            float symbolIntegrate;
            float filterIntegrate;
            float phaseIntegrate;

            // correlation processor
            float correlatedS0;
            float correlatedS1;
            float correlatedSD;

            // symbol parameters
            int symbolStartTime;
            int symbolEndTime;
            int symbolPattern;
            float symbolCorr0;
            float symbolCorr1;
            float symbolPhase;

            int padding;

            float filterData[SignalBufferLength];
      };

      struct SymbolInfo
      {
            int value; // symbol value (0 / 1)
            long start;	// sample clocks for start of last decoded symbol
            long end; // sample clocks for end of last decoded symbol
            int length; // samples for next symbol syncronization
      };

      struct FrameInfo
      {
            int last; // last command
            int type; // frame type
            long start;	// sample clocks for start of last decoded symbol
            long end; // sample clocks for end of last decoded symbol
            int length; // samples for next symbol syncronization

            DecodeInfo *decode;
      };

   private:

      // raw signal source
      QSharedPointer<SignalDevice> mSignalDevice;

      // raw signal recorder
      QSharedPointer<RecordDevice> mRecordDevice;

      // sample data buffer
      SampleBuffer<float> mSampleBuffer;

      // frame processing status
      struct FrameInfo mFrameSearch;

      // signal processing status
      struct SignalInfo mSignalInfo;

      // symbol detected
      struct SymbolInfo mSymbolInfo;

      // symbol decoder status
      struct DecodeInfo mDecodeInfo[4];

      // signal sample bits
      int mSampleSize;

      // signal sample rate
      long mSampleRate;

      // total processed samples
      long mSampleCount;

      // signal master clock
      long mSignalClock;

      // minimun signal level
      float mPowerLevelThreshold;

      // minimun modulation threshold to detect valid signal (default 5%)
      float mModulationThreshold;

      // maximun frame length bytes (default 64)
      int mMaximunFrameLength;

   private:

      int nextPattern(int searchMode, long timeout);

      void record(int channel, float value);

      void process(NfcFrame frame);

      void processREQA(NfcFrame frame);
      void processHLTA(NfcFrame frame);
      void processWUPA(NfcFrame frame);
      void processSELn(NfcFrame frame);
      void processRATS(NfcFrame frame);
      void processPPSr(NfcFrame frame);
      void processAUTH(NfcFrame frame);
      void processIBlock(NfcFrame frame);
      void processRBlock(NfcFrame frame);
      void processSBlock(NfcFrame frame);
      void processOther(NfcFrame frame);

      bool checkCrc(NfcFrame frame);

   public:

      NfcDecoder(SignalDevice *device, RecordDevice *record = nullptr, QObject *parent = nullptr);

      virtual ~NfcDecoder();

      NfcFrame nextFrame(long timeout);

      long sampleCount() const;
      float signalStrength() const;

      void setPowerLevelThreshold(float value);
      float powerLevelThreshold() const;

      void setModulationThreshold(float value);
      float modulationThreshold() const;

//      void setRequestFrameThreshold(float value);
//      float requestFrameThreshold() const;

//      void setResponseFrameThreshold(float value);
//      float responseFrameThreshold() const;

      void setMaximunFrameLength(int value);
      float maximunFrameLength() const;

   private:

      class Registry
      {
         private:

            int sample;
            int channels;
            int offset;
            bool pending;
            char buffer[1024];
            RecordDevice *recorder;

         public:

            Registry(RecordDevice *recorder);
            virtual ~Registry();

            void set(int channel, int value);
            void set(int channel, float value);
            void set(int channel, double value);
            void commit();
      };

      QSharedPointer<NfcDecoder::Registry> mRecorder;
};

#endif /* NFCDECODER_H */

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

#ifndef STREAMSTATUSEVENT_H
#define STREAMSTATUSEVENT_H

#include <QEvent>
#include <QVector>
#include <QStringList>

class StreamStatusEvent: public QEvent
{
   public:

      static int Type;

      enum Status
      {
         Stopped = 0,
         Streaming = 1,
         Recording = 2
      };

      enum Info
      {
         None = 0,
         Status = (1 << 0),
         Source = (1 << 1),
         Frequency = (1 << 2),
         TunerGain = (1 << 3),
         SampleRate = (1 << 4),
         SampleCount = (1 << 5),
         SignalPower = (1 << 6),
         SourceList = (1 << 7),
         FrequencyList = (1 << 8),
         TunerGainList = (1 << 9),
         ReceivedSamples = (1 << 10),
         StreamProgress = (1 << 11)
      };

   public:

      StreamStatusEvent();
      StreamStatusEvent(int status);
      virtual ~StreamStatusEvent();

      bool hasStatus();
      int status() const;
      StreamStatusEvent *setStatus(int status);

      bool hasSource();
      const QString &source() const;
      StreamStatusEvent *setSource(const QString &source);

      bool hasFrequency();
      int frequency() const;
      StreamStatusEvent *setFrequency(long frequecy);

      bool hasSampleRate();
      int sampleRate() const;
      StreamStatusEvent *setSampleRate(long sampleRate);

      bool hasSampleCount();
      long sampleCount() const;
      StreamStatusEvent *setSampleCount(long sampleCount);

      bool hasTunerGain();
      float tunerGain() const;
      StreamStatusEvent *setTunerGain(float tunerGain);

      bool hasSignalPower();
      float signalPower() const;
      StreamStatusEvent *setSignalPower(float signalPower);

      bool hasStreamProgress();
      float streamProgress() const;
      StreamStatusEvent *setStreamProgress(float streamProgress);

      bool hasSourceList();
      const QStringList &sourceList() const;
      StreamStatusEvent *setSourceList(const QStringList &sourceList);

      bool hasFrequencyList();
      const QVector<long> &frequencyList() const;
      StreamStatusEvent *setFrequencyList(const QVector<long> &frequencyList);

      bool hasTunerGainList();
      const QVector<float> &tunerGainList() const;
      StreamStatusEvent *setTunerGainList(const QVector<float> &tunerGainList);

      static StreamStatusEvent *create();
      static StreamStatusEvent *create(int status);

   private:

      int mInfo;
      int mStatus;
      QString mSource;
      long mFrequency;
      long mSampleRate;
      long mSampleCount;
      float mTunerGain;
      float mSignalPower;
      float mStreamProgress;

      QStringList mSourceList;
      QVector<long> mFrequencyList;
      QVector<float> mTunerGainList;
};

#endif /* STREAMSTATUSEVENT_H_ */

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

#include "StreamStatusEvent.h"

int StreamStatusEvent::Type = QEvent::registerEventType();

StreamStatusEvent::StreamStatusEvent() :
      QEvent(QEvent::Type(Type)), mInfo(0), mStatus(0), mFrequency(0), mSampleRate(0), mSampleCount(0), mTunerGain(0), mSignalPower(0)
{
}

StreamStatusEvent::StreamStatusEvent(int status) :
      QEvent(QEvent::Type(Type)), mInfo(Status), mStatus(status), mFrequency(0), mSampleRate(0), mSampleCount(0), mTunerGain(0), mSignalPower(0)
{
}

StreamStatusEvent::~StreamStatusEvent()
{
}

bool StreamStatusEvent::hasStatus()
{
	return mInfo & Status;
}

int StreamStatusEvent::status() const
{
	return mStatus;
}

StreamStatusEvent *StreamStatusEvent::setStatus(int status)
{
   mInfo |= Status;
   mStatus = status;
   return this;
}

bool StreamStatusEvent::hasSource()
{
   return mInfo & Source;
}

const QString &StreamStatusEvent::source() const
{
   return mSource;
}

StreamStatusEvent *StreamStatusEvent::setSource(const QString &source)
{
   mInfo |= Source;
   mSource = source;
   return this;
}

bool StreamStatusEvent::hasFrequency()
{
	return mInfo & Frequency;
}

int StreamStatusEvent::frequency() const
{
	return mFrequency;
}

StreamStatusEvent *StreamStatusEvent::setFrequency(long frequecy)
{
   mInfo |= Frequency;
   mFrequency = frequecy;
   return this;
}

bool StreamStatusEvent::hasSampleRate()
{
	return mInfo & SampleRate;
}

int StreamStatusEvent::sampleRate() const
{
	return mSampleRate;
}

StreamStatusEvent *StreamStatusEvent::setSampleRate(long sampleRate)
{
   mInfo |= SampleRate;
   mSampleRate = sampleRate;
   return this;
}

bool StreamStatusEvent::hasSampleCount()
{
   return mInfo & SampleCount;
}

long StreamStatusEvent::sampleCount() const
{
   return mSampleCount;
}

StreamStatusEvent *StreamStatusEvent::setSampleCount(long sampleCount)
{
   mInfo |= SampleCount;
   mSampleCount = sampleCount;
   return this;
}

bool StreamStatusEvent::hasTunerGain()
{
	return mInfo & TunerGain;
}

float StreamStatusEvent::tunerGain() const
{
	return mTunerGain;
}

StreamStatusEvent *StreamStatusEvent::setTunerGain(float tunerGain)
{
   mInfo |= TunerGain;
   mTunerGain = tunerGain;
   return this;
}

bool StreamStatusEvent::hasSignalPower()
{
   return mInfo & SignalPower;
}

float StreamStatusEvent::signalPower() const
{
   return mSignalPower;
}

StreamStatusEvent *StreamStatusEvent::setSignalPower(float signalPower)
{
   mInfo |= SignalPower;
   mSignalPower = signalPower;
   return this;
}

bool StreamStatusEvent::hasStreamProgress()
{
   return mInfo & StreamProgress;
}

float StreamStatusEvent::streamProgress() const
{
   return mStreamProgress;
}

StreamStatusEvent *StreamStatusEvent::setStreamProgress(float streamProgress)
{
   mInfo |= StreamProgress;
   mStreamProgress = streamProgress;
   return this;
}

bool StreamStatusEvent::hasSourceList()
{
   return mInfo & SourceList;
}

const QStringList &StreamStatusEvent::sourceList() const
{
   return mSourceList;
}

StreamStatusEvent *StreamStatusEvent::setSourceList(const QStringList &sourceList)
{
   mInfo |= SourceList;
   mSourceList = sourceList;
   return this;
}

bool StreamStatusEvent::hasFrequencyList()
{
	return mInfo & FrequencyList;
}

const QVector<long> &StreamStatusEvent::frequencyList() const
{
	return mFrequencyList;
}

StreamStatusEvent *StreamStatusEvent::setFrequencyList(const QVector<long> &frequencyList)
{
   mInfo |= FrequencyList;
   mFrequencyList = frequencyList;
   return this;
}

bool StreamStatusEvent::hasTunerGainList()
{
   return mInfo & TunerGainList;
}

const QVector<float> &StreamStatusEvent::tunerGainList() const
{
   return mTunerGainList;
}

StreamStatusEvent *StreamStatusEvent::setTunerGainList(const QVector<float> &tunerGainList)
{
   mInfo |= TunerGainList;
   mTunerGainList = tunerGainList;
   return this;
}

StreamStatusEvent *StreamStatusEvent::create()
{
   return new StreamStatusEvent();
}

StreamStatusEvent *StreamStatusEvent::create(int status)
{
   return new StreamStatusEvent(status);
}


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

#include "NfcFrame.h"

const NfcFrame NfcFrame::Nil;

NfcFrame::NfcFrame() :
   mImpl(nullptr)
{
}

NfcFrame::NfcFrame(int tech, int type)
{
   mImpl = new Impl(tech, type, 0);

   mImpl->increment();
}

NfcFrame::NfcFrame(int tech, int type, double timeStart, double timeEnd)
{
   mImpl = new Impl(tech, type, 0, timeStart, timeEnd);

   mImpl->increment();
}

NfcFrame::NfcFrame(int tech, int type, SampleBuffer<float> samples)
{
   mImpl = new Impl(tech,type, 0, samples);

   mImpl->increment();
}

NfcFrame::NfcFrame(int tech, int type, int flags, SampleBuffer<float> samples)
{
   mImpl = new Impl(tech, type, flags, samples);

   mImpl->increment();
}

NfcFrame::NfcFrame(const NfcFrame &other)
{
   mImpl = other.mImpl;

   if (mImpl)
      mImpl->increment();
}

NfcFrame::~NfcFrame()
{
   if (mImpl && mImpl->decrement() == 0)
      delete mImpl;
}

NfcFrame &NfcFrame::operator=(const NfcFrame &other)
{
   if (mImpl && mImpl->decrement() == 0)
      delete mImpl;

   mImpl = other.mImpl;

   if (mImpl)
      mImpl->increment();

   return *this;
}

bool NfcFrame::isNil() const
{
   return !mImpl;
}

bool NfcFrame::isValid() const
{
   return mImpl;
}

bool NfcFrame::isEmpty() const
{
   return mImpl ? mImpl->mLength == 0 : true;
}

bool NfcFrame::isNfcA() const
{
   return mImpl ? mImpl->mTechType == TechType::NfcA : false;
}

bool NfcFrame::isNfcB() const
{
   return mImpl ? mImpl->mTechType == TechType::NfcB : false;
}

bool NfcFrame::isNfcF() const
{
   return mImpl ? mImpl->mTechType == TechType::NfcF : false;
}

bool NfcFrame::isNoSignal() const
{
   return mImpl ? mImpl->mFrameType == FrameType::NoSignal : false;
}

bool NfcFrame::isNoFrame() const
{
   return mImpl ? mImpl->mFrameType == FrameType::NoFrame : false;
}

bool NfcFrame::isRequestFrame() const
{
   return mImpl ? mImpl->mFrameType == FrameType::RequestFrame : false;
}

bool NfcFrame::isResponseFrame() const
{
   return mImpl ? mImpl->mFrameType == FrameType::ResponseFrame : false;
}

bool NfcFrame::isShortFrame() const
{
   return mImpl ? mImpl->mFrameFlags & FrameFlags::ShortFrame : false;
}

bool NfcFrame::isParityError() const
{
   return mImpl ? mImpl->mFrameFlags & FrameFlags::ParityError : false;
}

bool NfcFrame::isTruncated() const
{
   return mImpl ? mImpl->mFrameFlags & FrameFlags::Truncated : false;
}

int NfcFrame::techType() const
{
   return mImpl ? mImpl->mTechType : -1;
}

void NfcFrame::setTechType(int tech)
{
   if (mImpl)
      mImpl->mTechType = tech;
}

int NfcFrame::frameType() const
{
   return mImpl ? mImpl->mFrameType : -1;
}

void NfcFrame::setFrameType(int type)
{
   if (mImpl)
      mImpl->mFrameType = type;
}

int NfcFrame::framePhase() const
{
   return mImpl ? mImpl->mFramePhase : -1;
}

void NfcFrame::setFramePhase(int phase)
{
   if (mImpl)
      mImpl->mFramePhase = phase;
}

int NfcFrame::frameFlags() const
{
   return mImpl ? mImpl->mFrameFlags : -1;
}

void NfcFrame::setFrameFlags(int flags)
{
   if (mImpl)
      mImpl->mFrameFlags |= flags;
}

bool NfcFrame::hasFrameFlags(int flags)
{
   return  mImpl ? mImpl->mFrameFlags & flags : false;
}

int NfcFrame::frameRate() const
{
   return mImpl ? mImpl->mFrameRate : -1;
}

void NfcFrame::setFrameRate(int rate)
{
   if (mImpl)
      mImpl->mFrameRate = rate;
}

double NfcFrame::timeStart() const
{
   return mImpl ? mImpl->mTimeStart : -1;
}

void NfcFrame::setTimeStart(double start)
{
   if (mImpl)
      mImpl->mTimeStart = start;
}

double NfcFrame::timeEnd() const
{
   return mImpl ? mImpl->mTimeEnd : -1;
}

void NfcFrame::setTimeEnd(double end)
{
   if (mImpl)
      mImpl->mTimeEnd = end;
}

long NfcFrame::sampleStart() const
{
   return mImpl ? mImpl->mSampleStart : -1;
}

void NfcFrame::setSampleStart(long start)
{
   if (mImpl)
      mImpl->mSampleStart = start;
}

long NfcFrame::sampleEnd() const
{
   return mImpl ? mImpl->mSampleEnd : -1;
}

void NfcFrame::setSampleEnd(long end)
{
   if (mImpl)
      mImpl->mSampleEnd = end;
}

SampleBuffer<float> NfcFrame::samples() const
{
   return mImpl ? mImpl->mSamples : SampleBuffer<float>();
}

void NfcFrame::setSamples(SampleBuffer<float> samples)
{
   if (mImpl)
      mImpl->mSamples = samples;
}


int NfcFrame::length() const
{
   return mImpl ? mImpl->mLength : 0;
}

void NfcFrame::resize(int newLength)
{
   if (mImpl)
      mImpl->resize(newLength);
}

int NfcFrame::get(int &data)
{
   if (mImpl)
   {
      if (mImpl->mOffset < mImpl->mLength)
      {
         data = mImpl->mData[mImpl->mOffset++];

         return 1;
      }

      return 0;
   }

   return -1;
}

int NfcFrame::get(QByteArray &data)
{
   if (mImpl)
   {
      int position = mImpl->mOffset;

      while (mImpl->mOffset < mImpl->mLength)
      {
         data.append((char)mImpl->mData[mImpl->mOffset++]);
      }

      return mImpl->mOffset - position;
   }

   return -1;
}

int NfcFrame::put(int data)
{
   if (mImpl)
   {
      // offset reach length increase length
      if (mImpl->mOffset == mImpl->mLength)
         mImpl->mLength++;

      // offset reach  capacity resize buffer
      if (mImpl->mLength == mImpl->mCapacity)
         resize(mImpl->mCapacity + 256);

      // store data
      mImpl->mData[mImpl->mOffset++] = data;

      return 1;
   }

   return -1;
}

int NfcFrame::put(const QByteArray &data)
{
   if (mImpl)
   {
      for (int i = 0; i < data.length(); i++)
      {
         // offset reach length increase length
         if (mImpl->mOffset == mImpl->mLength)
            mImpl->mLength++;

         // offset reach  capacity resize buffer
         if (mImpl->mLength == mImpl->mCapacity)
            resize(mImpl->mCapacity + 256);

         // store data
         mImpl->mData[mImpl->mOffset++] = data.at(i) & 0xff;
      }

      return data.length();
   }

   return -1;
}

int NfcFrame::operator[](int index) const
{
   if (mImpl)
   {
      if (index >= 0 && index < mImpl->mLength)
      {
         return mImpl->mData[index];
      }
   }

   return -1;
}

NfcFrame::operator bool() const
{
   return mImpl;
}

QByteArray NfcFrame::toByteArray(int from, int length) const
{
   QByteArray data;

   if (mImpl)
   {
      if (length > mImpl->mLength)
         length = mImpl->mLength;

      if (from >= 0)
      {
         if (from < mImpl->mLength)
         {
            for (int i = from; i < mImpl->mLength && length > 0; i++, length--)
            {
               data.append((char)mImpl->mData[i]);
            }
         }
      }
      else
      {
         if (from >= -mImpl->mLength)
         {
            for (int i = mImpl->mLength + from; i < mImpl->mLength; i++, length--)
            {
               data.append((char)mImpl->mData[i]);
            }
         }
      }
   }

   return data;
}

NfcFrame::Impl::Impl(int tech, int type, int flags) :
   mRefs(0), mTechType(tech), mFrameType(type), mFrameFlags(flags), mFramePhase(0), mFrameRate(0), mTimeStart(0), mTimeEnd(0), mSampleStart(0), mSampleEnd(0), mLength(0), mOffset(0), mCapacity(256), mData(0)
{
   mData = new int[mCapacity];
}

NfcFrame::Impl::Impl(int tech, int type, int flags, double timeStart, double timeEnd) :
   mRefs(0), mTechType(tech), mFrameType(type), mFrameFlags(flags), mFramePhase(0), mFrameRate(0), mTimeStart(timeStart), mTimeEnd(timeEnd), mSampleStart(0), mSampleEnd(0), mLength(0), mOffset(0), mCapacity(256), mData(0)
{
   mData = new int[mCapacity];
}

NfcFrame::Impl::Impl(int tech, int type, int flags, SampleBuffer<float> samples) :
   mRefs(0), mTechType(tech), mFrameType(type), mFrameFlags(flags), mFramePhase(0), mFrameRate(0), mTimeStart(0), mTimeEnd(0), mSampleStart(0), mSampleEnd(0), mLength(0), mOffset(0), mCapacity(256), mData(0), mSamples(samples)
{
   mData = new int[mCapacity];
}

NfcFrame::Impl::~Impl()
{
   delete[] mData;
}

int NfcFrame::Impl::increment()
{
   return ++mRefs;
}

int NfcFrame::Impl::decrement()
{
   return --mRefs;
}

void NfcFrame::Impl::resize(int newLength)
{
   int *alloc = new int[newLength];

   for (int i = 0; i < newLength && i < mLength; i++)
   {
      alloc[i] = mData[1];
   }

   delete[] mData;

   mCapacity = newLength;

   mData = alloc;
}


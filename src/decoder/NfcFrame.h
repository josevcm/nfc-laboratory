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

#ifndef NFCFRAME_H
#define NFCFRAME_H

#include <QByteArray>

#include <devices/SampleBuffer.h>

class NfcFrame
{
   public:

      enum TechType
      {
         None = 0,
         NfcA = 1,
         NfcB = 2,
         NfcF = 3
      };

      enum FrameType
      {
         NoSignal = 0,
         NoFrame = 1,
         RequestFrame = 2,
         ResponseFrame = 3
      };

      enum FramePhase
      {
         SenseFrame = 1,
         SelectionFrame = 2,
         InformationFrame = 3
      };

      enum FrameFlags
      {
         ShortFrame = 1,
         ParityError = 2,
         Truncated = 4
      };

      static const NfcFrame Nil;

   private:

      class Impl
      {
            int mRefs;
            int mTechType;
            int mFrameType;
            int mFrameFlags;
            int mFramePhase;
            int mFrameRate;
            double mTimeStart;
            double mTimeEnd;
            long mSampleStart;
            long mSampleEnd;
            int mLength;
            int mOffset;
            int mCapacity;
            int *mData;

            SampleBuffer<float> mSamples;

            Impl(int tech, int type, int flags);
            Impl(int tech, int type, int flags, double timeStart, double timeEnd);
            Impl(int tech, int type, int flags, SampleBuffer<float> samples);
            ~Impl();

            int increment();
            int decrement();

            void resize(int newLength);

            friend class NfcFrame;
      };

   private:

      Impl *mImpl;

   public:

      NfcFrame();
      NfcFrame(int tech, int type);
      NfcFrame(int tech, int type, double timeStart, double timeEnd);
      NfcFrame(int tech, int type, SampleBuffer<float> samples);
      NfcFrame(int tech, int type, int flags, SampleBuffer<float> samples = SampleBuffer<float>());
      NfcFrame(const NfcFrame &other);
      virtual ~NfcFrame();

      NfcFrame &operator=(const NfcFrame &other);

      bool isNil() const;
      bool isValid() const;
      bool isEmpty() const;

      bool isNfcA() const;
      bool isNfcB() const;
      bool isNfcF() const;

      bool isNoSignal() const;
      bool isNoFrame() const;
      bool isRequestFrame() const;
      bool isResponseFrame() const;

      bool isShortFrame() const;
      bool isParityError() const;
      bool isTruncated() const;

      int techType() const;
      void setTechType(int tech);

      int frameType() const;
      void setFrameType(int type);

      int framePhase() const;
      void setFramePhase(int phase);

      int frameFlags() const;
      void setFrameFlags(int flags);
      bool hasFrameFlags(int flags);

      int frameRate() const;
      void setFrameRate(int rate);

      double timeStart() const;
      void setTimeStart(double start);

      double timeEnd() const;
      void setTimeEnd(double end);

      long sampleStart() const;
      void setSampleStart(long start);

      long sampleEnd() const;
      void setSampleEnd(long end);

      SampleBuffer<float> samples() const;
      void setSamples(SampleBuffer<float> samples);

      int length() const;

      void resize(int length);

      int get(int &data);
      int get(QByteArray &data);

      int put(int data);
      int put(const QByteArray &data);

      int operator[](int index) const;

      operator bool() const;

      QByteArray toByteArray(int from = 0, int length = INT32_MAX) const;
};

#endif /* NFCFRAME_H_ */

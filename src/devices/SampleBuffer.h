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

#ifndef SAMPLEBUFFER_H
#define SAMPLEBUFFER_H

template<class T>
class SampleBuffer
{
   public:

      enum Type
      {
         Raw, IQ, Real
      };

   private:

      class Impl
      {
            T *mData; // signal sample data
            int mType; // type of signal
            int mRefs; // signal reference counter
            int mPosition; // current signal position
            int mLimit; // total number of samples
            int mCapacity; // total capacity of samples
            int mStride; // signal channel stride
            long mClock; // signal clock

            Impl(int type, int size, int stride, int clock) :
               mData(0), mType(type), mRefs(0), mPosition(0), mLimit(size), mCapacity(size), mStride(stride), mClock(clock)
            {
               mData = new T[size * stride];
            }

            ~Impl()
            {
               delete[] mData;
            }

            int increment()
            {
               return ++mRefs;
            }

            int decrement()
            {
               return --mRefs;
            }

            friend class SampleBuffer;
      };

   private:

      Impl *mImpl;

   public:

      SampleBuffer() :
         mImpl(0)
      {
      }

      SampleBuffer(const SampleBuffer &other)
      {
         mImpl = other.mImpl;

         if (mImpl)
            mImpl->increment();
      }

      SampleBuffer(int type, int size, int stride = 1, long clock = 0)
      {
         mImpl = new Impl(type, size, stride, clock);

         mImpl->increment();
      }

      virtual ~SampleBuffer()
      {
         if (mImpl && mImpl->decrement() == 0)
            delete mImpl;
      }

      SampleBuffer &operator=(const SampleBuffer &other)
      {
         if (mImpl && mImpl->decrement() == 0)
            delete mImpl;

         mImpl = other.mImpl;

         if (mImpl)
            mImpl->increment();

         return *this;
      }

      inline bool isValid() const
      {
         return mImpl;
      }

      inline bool isEmpty() const
      {
         return mImpl && mImpl->mPosition == mImpl->mLimit;
      }

      inline Type type() const
      {
         return mImpl ? mImpl->mType : -1;
      }

      inline int position() const
      {
         return mImpl ? mImpl->mPosition : -1;
      }

      inline int limit() const
      {
         return mImpl ? mImpl->mLimit : -1;
      }

      inline int capacity() const
      {
         return mImpl ? mImpl->mCapacity : -1;
      }

      inline int available() const
      {
         return mImpl ? mImpl->mLimit - mImpl->mPosition : -1;
      }

      inline int stride() const
      {
         return mImpl ? mImpl->mStride : -1;
      }

      inline long clock() const
      {
         return mImpl ? mImpl->mClock : -1;
      }

      inline T *data() const
      {
         return mImpl ? mImpl->mData : 0;
      }

      inline SampleBuffer<T> &resize(int newCapacity)
      {
         if (mImpl)
         {
            T* data = new T[newCapacity * mImpl->mStride];

            for (int i = 0; i < newCapacity * mImpl->mStride; i++)
            {
               if (i < mImpl->mLimit * mImpl->mStride)
               {
                  data[i] = mImpl->mData[i];
               }
            }

            delete[] mImpl->mData;

            mImpl->mData = data;
            mImpl->mLimit = newCapacity;
            mImpl->mCapacity = newCapacity;
         }

         return *this;
      }

      inline SampleBuffer<T> &reset()
      {
         if (mImpl)
         {
            mImpl->mPosition = 0;
            mImpl->mLimit = mImpl->mCapacity;
         }

         return *this;
      }

      inline SampleBuffer<T> &wrap(int length)
      {
         if (mImpl)
         {
            mImpl->mPosition = mImpl->mPosition + length;
         }

         return *this;
      }

      inline SampleBuffer<T> &flip()
      {
         if (mImpl)
         {
            mImpl->mLimit = mImpl->mPosition;
            mImpl->mPosition = 0;
         }

         return *this;
      }

      inline SampleBuffer<T> &get(T *value)
      {
         if (mImpl)
         {
            if (mImpl->mPosition < mImpl->mLimit)
            {
               for (int n = 0; n < mImpl->mStride; n++)
               {
                  value[n] = mImpl->mData[mImpl->mPosition * mImpl->mStride + n];
               }

               mImpl->mPosition++;
            }
         }

         return *this;
      }

      inline SampleBuffer<T> &put(T *value)
      {
         if (mImpl)
         {
            if (mImpl->mPosition < mImpl->mLimit)
            {
               for (int n = 0; n < mImpl->mStride; n++)
               {
                  mImpl->mData[mImpl->mPosition * mImpl->mStride + n] = value[n];
               }

               mImpl->mPosition++;
            }
         }

         return *this;
      }

      inline SampleBuffer<T> &get(int index, T *values)
      {
         if (mImpl)
         {
            if (index >= 0 && index < mImpl->mLimit)
            {
               for (int n = 0; n < mImpl->mStride; n++)
               {
                  values[n] = mImpl->mData[index * mImpl->mStride + n];
               }
            }
         }

         return *this;
      }

      inline SampleBuffer<T> &set(int index, T *values)
      {
         if (mImpl)
         {
            if (index >= 0 && index < mImpl->mLimit)
            {
               for (int n = 0; n < mImpl->mStride; n++)
               {
                  mImpl->mData[index * mImpl->mStride + n] = values[n];
               }
            }
         }

         return *this;
      }

      virtual T *operator[](int index)
      {
         if (mImpl && index >= 0 && index < mImpl->mLimit)
         {
            return mImpl->mData + mImpl->mStride * index;
         }

         return 0;
      }

      inline operator bool() const
      {
         return mImpl;
      }
};

#endif /* SAMPLEBUFFER_H */

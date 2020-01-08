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

#ifndef NFCITERATOR_H
#define NFCITERATOR_H

#include <QList>

#include "NfcFrame.h"
#include "NfcStream.h"

class NfcIterator
{
   public:

      inline NfcIterator(const NfcStream &container) : mCurrent(0), mContainer(&container)
      {
      }

      inline NfcIterator(const NfcStream *container = nullptr) : mCurrent(0), mContainer(container)
      {
      }

      inline bool hasNext() const
      {
         return mContainer && mContainer->length() > mCurrent;
      }

      inline NfcFrame next()
      {
         return mContainer->at(mCurrent++);
      }

      inline void reset()
      {
         mCurrent = 0;
      }

   private:

      int mCurrent;

      const NfcStream *mContainer;
};

#endif // NFCITERATOR_H

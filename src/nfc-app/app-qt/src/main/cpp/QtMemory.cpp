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

#include <QDebug>

#include <sdr/SignalBuffer.h>

#include "QtMemory.h"

struct QtMemory::Impl
{
   // configuration
   QSettings &settings;

   // buffer vector
   QVector<sdr::SignalBuffer> signalBufferCache;

   // total currentMemoryUsage
   long currentBufferSamples;

   // maximum allowed memory usage
   long maximumBufferSamples;

   explicit Impl(QSettings &settings) : settings(settings)
   {
      maximumBufferSamples = 512 * 1024 * 1024 >> 2;
   };
};

QtMemory::QtMemory(QSettings &settings) : impl(new Impl(settings))
{
}

void QtMemory::append(const sdr::SignalBuffer &buffer)
{
   impl->signalBufferCache.append(buffer);
   impl->currentBufferSamples += buffer.elements();

   while (impl->currentBufferSamples > impl->maximumBufferSamples)
   {
      impl->currentBufferSamples -= impl->signalBufferCache.takeFirst().elements();
   }
}

void QtMemory::clear()
{
   impl->currentBufferSamples = 0;
   impl->signalBufferCache.clear();
}

long QtMemory::length() const
{
   return impl->signalBufferCache.length();
}

long QtMemory::samples() const
{
   return impl->currentBufferSamples;
}

long QtMemory::size() const
{
   return impl->currentBufferSamples * sizeof(float);
}

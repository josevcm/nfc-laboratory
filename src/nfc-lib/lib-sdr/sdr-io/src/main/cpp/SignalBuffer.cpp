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

#include <sdr/SignalBuffer.h>

namespace sdr {

struct SignalBuffer::Impl
{
   long samplerate;
   long decimation;

   explicit Impl(long samplerate, long decimation) : samplerate(samplerate), decimation(decimation)
   {
   }
};

SignalBuffer::SignalBuffer() : impl(std::make_shared<Impl>(0, 0))
{
}

SignalBuffer::SignalBuffer(unsigned int length, unsigned int stride, unsigned int samplerate, unsigned int decimation, int type, void *context) : Buffer<float>(length, type, stride, context), impl(std::make_shared<Impl>(samplerate, decimation))
{
}

SignalBuffer::SignalBuffer(float *data, unsigned int length, unsigned int stride, unsigned int samplerate, unsigned int decimation, int type, void *context) : Buffer<float>(data, length, type, stride, context), impl(std::make_shared<Impl>(samplerate, decimation))
{
}

SignalBuffer::SignalBuffer(const SignalBuffer &other) : Buffer(other), impl(other.impl)
{
}

SignalBuffer &SignalBuffer::operator=(const SignalBuffer &other)
{
   if (this == &other)
      return *this;

   rt::Buffer<float>::operator=(other);

   impl = other.impl;

   return *this;
}

unsigned int SignalBuffer::decimation() const
{
   return impl->decimation;
}

unsigned int SignalBuffer::sampleRate() const
{
   return impl->samplerate;
}

}

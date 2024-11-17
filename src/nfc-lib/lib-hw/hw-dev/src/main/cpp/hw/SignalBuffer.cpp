/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2024 Jose Vicente Campos Martinez, <josevcm@gmail.com>

  NFC-LABORATORY is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NFC-LABORATORY is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NFC-LABORATORY. If not, see <http://www.gnu.org/licenses/>.

*/

#include <hw/SignalBuffer.h>

namespace hw {

struct SignalBuffer::Impl
{
   unsigned int id;
   unsigned int samplerate;
   unsigned int decimation;
   unsigned long long offset;

   explicit Impl(unsigned int id, unsigned long long offset, unsigned int samplerate, unsigned int decimation) : id(id), offset(offset), samplerate(samplerate), decimation(decimation)
   {
   }
};

SignalBuffer::SignalBuffer() : impl(std::make_shared<Impl>(0, 0, 0, 0))
{
}

SignalBuffer::SignalBuffer(unsigned int length, unsigned int stride, unsigned int interleave, unsigned int samplerate, unsigned long long offset, unsigned int decimation, unsigned int type, unsigned int id, void *context) : Buffer(length, type, stride, interleave, context), impl(std::make_shared<Impl>(id, offset, samplerate, decimation))
{
}

SignalBuffer::SignalBuffer(float *data, unsigned int length, unsigned int stride, unsigned int interleave, unsigned int samplerate, unsigned long long offset, unsigned int decimation, unsigned int type, unsigned int id, void *context) : Buffer(data, length, type, stride, interleave, context), impl(std::make_shared<Impl>(id, offset, samplerate, decimation))
{
}

SignalBuffer::SignalBuffer(const SignalBuffer &other) : Buffer(other), impl(other.impl)
{
}

SignalBuffer &SignalBuffer::operator=(const SignalBuffer &other)
{
   if (this == &other)
      return *this;

   Buffer::operator=(other);

   impl = other.impl;

   return *this;
}

unsigned int SignalBuffer::id() const
{
   return impl->id;
}

unsigned int SignalBuffer::sampleRate() const
{
   return impl->samplerate;
}

unsigned int SignalBuffer::decimation() const
{
   return impl->decimation;
}

unsigned long long SignalBuffer::offset() const
{
   return impl->offset;
}

}

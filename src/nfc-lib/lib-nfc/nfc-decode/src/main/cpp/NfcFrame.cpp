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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <nfc/NfcFrame.h>

namespace nfc {

struct NfcFrame::Impl
{
   unsigned int techType = 0;
   unsigned int frameType = 0;
   unsigned int frameFlags = 0;
   unsigned int framePhase = 0;
   unsigned int frameRate = 0;
   unsigned long sampleStart = 0;
   unsigned long sampleEnd = 0;
   double timeStart = 0;
   double timeEnd = 0;
};

const NfcFrame NfcFrame::Nil;

NfcFrame::NfcFrame() : rt::ByteBuffer(256), impl(std::make_shared<Impl>())
{
}

NfcFrame::NfcFrame(int techType, int frameType) : NfcFrame()
{
   impl->techType = techType;
   impl->frameType = frameType;
}

NfcFrame::NfcFrame(int techType, int frameType, double timeStart, double timeEnd) : NfcFrame()
{
   impl->techType = techType;
   impl->frameType = frameType;
   impl->timeStart = timeStart;
   impl->timeEnd = timeEnd;
}

NfcFrame::NfcFrame(const NfcFrame &other) : rt::ByteBuffer(other)
{
   impl = other.impl;
}

NfcFrame &NfcFrame::operator=(const NfcFrame &other)
{
   if (this == &other)
      return *this;

   rt::ByteBuffer::operator=(other);

   impl = other.impl;

   return *this;
}

NfcFrame::operator bool() const
{
   return Buffer::operator bool();
}

bool NfcFrame::isNfcA() const
{
   return impl->techType == TechType::NfcA;
}

bool NfcFrame::isNfcB() const
{
   return impl->techType == TechType::NfcB;
}

bool NfcFrame::isNfcF() const
{
   return impl->techType == TechType::NfcF;
}

bool NfcFrame::isNoCarrier() const
{
   return impl->frameType == FrameType::NoCarrier;
}

bool NfcFrame::isEmptyFrame() const
{
   return impl->frameType == FrameType::EmptyFrame;
}

bool NfcFrame::isRequestFrame() const
{
   return impl->frameType == FrameType::RequestFrame;
}

bool NfcFrame::isResponseFrame() const
{
   return impl->frameType == FrameType::ResponseFrame;
}

bool NfcFrame::isShortFrame() const
{
   return impl->frameFlags & FrameFlags::ShortFrame;
}

bool NfcFrame::isEncrypted() const
{
   return impl->frameFlags & FrameFlags::Encrypted;
}

bool NfcFrame::isTruncated() const
{
   return impl->frameFlags & FrameFlags::Truncated;
}

bool NfcFrame::hasParityError() const
{
   return impl->frameFlags & FrameFlags::ParityError;
}

bool NfcFrame::hasCrcError() const
{
   return impl->frameFlags & FrameFlags::CrcError;
}

unsigned int NfcFrame::techType() const
{
   return impl->techType;
}

void NfcFrame::setTechType(unsigned int tech)
{
   impl->techType = tech;
}

unsigned int NfcFrame::frameType() const
{
   return impl->frameType;
}

void NfcFrame::setFrameType(unsigned int type)
{
   impl->frameType = type;
}

unsigned int NfcFrame::framePhase() const
{
   return impl->framePhase;
}

void NfcFrame::setFramePhase(unsigned int phase)
{
   impl->framePhase = phase;
}

unsigned int NfcFrame::frameFlags() const
{
   return impl->frameFlags;
}

void NfcFrame::setFrameFlags(unsigned int flags)
{
   impl->frameFlags |= flags;
}

void NfcFrame::clearFrameFlags(unsigned int flags)
{
   impl->frameFlags &= ~flags;
}

bool NfcFrame::hasFrameFlags(unsigned int flags)
{
   return impl->frameFlags & flags;
}

unsigned int NfcFrame::frameRate() const
{
   return impl->frameRate;
}

void NfcFrame::setFrameRate(unsigned int rate)
{
   impl->frameRate = rate;
}

double NfcFrame::timeStart() const
{
   return impl->timeStart;
}

void NfcFrame::setTimeStart(double start)
{
   impl->timeStart = start;
}

double NfcFrame::timeEnd() const
{
   return impl->timeEnd;
}

void NfcFrame::setTimeEnd(double end)
{
   impl->timeEnd = end;
}

unsigned long NfcFrame::sampleStart() const
{
   return impl->sampleStart;
}

void NfcFrame::setSampleStart(unsigned long start)
{
   impl->sampleStart = start;
}

unsigned long NfcFrame::sampleEnd() const
{
   return impl->sampleEnd;
}

void NfcFrame::setSampleEnd(unsigned long end)
{
   impl->sampleEnd = end;
}

}
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

#include <lab/data/RawFrame.h>

namespace lab {

struct RawFrame::Impl
{
   unsigned int techType = 0;
   unsigned int frameType = 0;
   unsigned int frameFlags = 0;
   unsigned int framePhase = 0;
   unsigned int frameRate = 0;
   unsigned long sampleStart = 0;
   unsigned long sampleEnd = 0;
   unsigned long sampleRate = 0;
   double timeStart = 0;
   double timeEnd = 0;
   double dateTime = 0;
};

const RawFrame RawFrame::Nil;

RawFrame::RawFrame() : ByteBuffer(), impl(std::make_shared<Impl>())
{
}

RawFrame::RawFrame(unsigned int size) : ByteBuffer(size), impl(std::make_shared<Impl>())
{
}

RawFrame::RawFrame(unsigned int techType, unsigned int frameType) : RawFrame(256)
{
   impl->techType = techType;
   impl->frameType = frameType;
}

RawFrame::RawFrame(unsigned int techType, unsigned int frameType, double timeStart, double timeEnd) : RawFrame(256)
{
   impl->techType = techType;
   impl->frameType = frameType;
   impl->timeStart = timeStart;
   impl->timeEnd = timeEnd;
}

RawFrame::RawFrame(const RawFrame &other) : ByteBuffer(other)
{
   impl = other.impl;
}

RawFrame &RawFrame::operator=(const RawFrame &other)
{
   if (this == &other)
      return *this;

   ByteBuffer::operator=(other);

   impl = other.impl;

   return *this;
}

bool RawFrame::operator==(const RawFrame &other) const
{
   if (this == &other)
      return true;

   if (impl->techType != other.impl->techType ||
      impl->frameType != other.impl->frameType ||
      impl->frameFlags != other.impl->frameFlags ||
      impl->framePhase != other.impl->framePhase ||
      impl->frameRate != other.impl->frameRate ||
      impl->sampleStart != other.impl->sampleStart ||
      impl->sampleEnd != other.impl->sampleEnd ||
      impl->sampleRate != other.impl->sampleRate)
      return false;

   return ByteBuffer::operator==(other);
}

bool RawFrame::operator!=(const RawFrame &other) const
{
   return !operator==(other);
}

bool RawFrame::operator<(const RawFrame &other) const
{
   return impl->timeStart < other.impl->timeStart;
}

bool RawFrame::operator>(const RawFrame &other) const
{
   return impl->timeStart > other.impl->timeStart;
}

RawFrame::operator bool() const
{
   return Buffer::operator bool();
}

unsigned int RawFrame::techType() const
{
   return impl->techType;
}

void RawFrame::setTechType(unsigned int techType)
{
   impl->techType = techType;
}

unsigned int RawFrame::frameType() const
{
   return impl->frameType;
}

void RawFrame::setFrameType(unsigned int frameType)
{
   impl->frameType = frameType;
}

unsigned int RawFrame::framePhase() const
{
   return impl->framePhase;
}

void RawFrame::setFramePhase(unsigned int framePhase)
{
   impl->framePhase = framePhase;
}

unsigned int RawFrame::frameFlags() const
{
   return impl->frameFlags;
}

void RawFrame::setFrameFlags(unsigned int frameFlags)
{
   impl->frameFlags |= frameFlags;
}

void RawFrame::clearFrameFlags(unsigned int frameFlags)
{
   impl->frameFlags &= ~frameFlags;
}

bool RawFrame::hasFrameFlags(unsigned int frameFlags) const
{
   return impl->frameFlags & frameFlags;
}

unsigned int RawFrame::frameRate() const
{
   return impl->frameRate;
}

void RawFrame::setFrameRate(unsigned int rate)
{
   impl->frameRate = rate;
}

double RawFrame::timeStart() const
{
   return impl->timeStart;
}

void RawFrame::setTimeStart(double timeStart)
{
   impl->timeStart = timeStart;
}

double RawFrame::timeEnd() const
{
   return impl->timeEnd;
}

void RawFrame::setTimeEnd(double timeEnd)
{
   impl->timeEnd = timeEnd;
}

double RawFrame::dateTime() const
{
   return impl->dateTime;
}

void RawFrame::setDateTime(double dateTime)
{
   impl->dateTime = dateTime;
}

unsigned long RawFrame::sampleStart() const
{
   return impl->sampleStart;
}

void RawFrame::setSampleStart(unsigned long sampleStart)
{
   impl->sampleStart = sampleStart;
}

unsigned long RawFrame::sampleEnd() const
{
   return impl->sampleEnd;
}

void RawFrame::setSampleEnd(unsigned long sampleEnd)
{
   impl->sampleEnd = sampleEnd;
}

unsigned long RawFrame::sampleRate() const
{
   return impl->sampleRate;
}

void RawFrame::setSampleRate(unsigned long sampleRate)
{
   impl->sampleRate = sampleRate;
}

}

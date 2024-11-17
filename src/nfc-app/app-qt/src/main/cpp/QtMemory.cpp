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

#include <QDebug>

#include <lab/data/RawFrame.h>

#include <hw/SignalBuffer.h>

#include "QtMemory.h"

struct QtMemory::Impl
{
   // configuration
   QSettings settings;

   // frame vector
   QVector<lab::RawFrame> frameCache;

   // buffer vector
   QVector<hw::SignalBuffer> signalCache;

   // total currentMemoryUsage
   long signalSamples;

   // maximum allowed memory usage
   long maximumSamples;

   explicit Impl() : signalSamples(0), maximumSamples(512 * 1024 * 1024 >> 2)
   {
   };
};

QtMemory::QtMemory() : impl(new Impl())
{
}

void QtMemory::append(const lab::RawFrame &frame)
{
   impl->frameCache.append(frame);
}

void QtMemory::append(const hw::SignalBuffer &buffer)
{
   impl->signalCache.append(buffer);
   impl->signalSamples += buffer.elements();

   while (impl->signalSamples > impl->maximumSamples)
   {
      impl->signalSamples -= impl->signalCache.takeFirst().elements();
   }

   int timeMean = 0;
   int timeLast = 0;

   int valueMean = 0;
   int valueLast = 0;

   // sample scale to float
   float scale = 1 << (8 * 2 - 1);

   for (int i = 0; i < buffer.limit(); i += buffer.stride())
   {
      int time = int(buffer[i + 1]);
      int value = int(scale * buffer[i]);

      valueMean += value - valueLast;
      valueLast = value;

      timeMean += time - timeLast;
      timeLast = time;
   }

   qInfo() << "time mean interval: " << timeMean / buffer.elements() << "value mean interval: " << valueMean / buffer.elements();
}

void QtMemory::clear()
{
   qInfo() << "clearing memory cache:";
   qInfo() << "\t" << impl->signalSamples << "samples in the cache";
   qInfo() << "\t" << impl->frameCache.size() << "frames in the cache";
   qInfo() << "\t" << impl->signalCache.size() << "buffers in the cache";

   impl->frameCache.clear();
   impl->signalCache.clear();
   impl->signalSamples = 0;
}

long QtMemory::frames() const
{
   return impl->frameCache.size();
}

long QtMemory::samples() const
{
   return impl->signalSamples;
}

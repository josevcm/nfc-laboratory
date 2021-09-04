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

#include <cmath>
#include <mutex>

#include <rt/Buffer.h>

#include <gl/engine/Text.h>

#include <nfc/DefaultShader.h>
#include <nfc/SignalSmoother.h>
#include <nfc/EnvelopeShader.h>
#include <nfc/HeatmapShader.h>

#include <nfc/FrequencyData.h>

namespace nfc {

struct FrequencyData::Impl
{
   int length;
   int decimation;

   // default parameters
   nfc::SmoothParameters params;

   // shader buffers
   gl::Buffer dataValue;
   gl::Buffer dataRange;
   gl::Buffer dataBlock;

   // signal buffer update mutex
   std::mutex signalMutex;

   // last received buffer
   sdr::SignalBuffer signalBuffer;

   Impl(int length) : length(length)
   {
      decimation = 2;

      // initialize GL shader buffers
      dataRange = gl::Buffer::createArrayBuffer(length * sizeof(float));
      dataValue = gl::Buffer::createArrayBuffer(length * sizeof(float) * 2);
      dataBlock = gl::Buffer::createStorageBuffer(1 << 18).bind(0);

      // default smooth parameters
      params = {(float) length, 150, 1, 0.25, 0.15};
   }
};

FrequencyData::FrequencyData(int length) : self(std::make_shared<Impl>(length))
{
}

void FrequencyData::setCenterFreq(long value)
{
}

void FrequencyData::setSampleRate(long value)
{
}

void FrequencyData::refresh(const sdr::SignalBuffer &buffer)
{
   if (self->signalMutex.try_lock())
   {
      self->signalBuffer = buffer;
      self->signalMutex.unlock();
   }
}

gl::Widget *FrequencyData::resize(int width, int height)
{
   gl::Widget::resize(width, height);

   const auto &rect = bounds();

   float step = (rect.xmax - rect.xmin) / (float) self->length;

   float range[self->length];

   #pragma GCC ivdep
   for (int i = 0; i < self->length; i++)
      range[i] = rect.xmin + step * (float) i;

   self->dataRange.update(range);

   return this;
}

void FrequencyData::update(float time, float delta)
{
   std::lock_guard<std::mutex> lock(self->signalMutex);

   if (self->signalBuffer.isValid())
   {
      self->dataValue.update(self->signalBuffer.data(), 0, self->signalBuffer.available() * sizeof(float));
   }
}

void FrequencyData::draw(gl::Device *device, gl::Program *shader) const
{
   if (auto *smoothShader = dynamic_cast<SignalSmoother *>(shader))
   {
      smoothShader->process(self->dataRange, self->dataValue, self->params, self->length);
   }
   else if (auto heatmapShader = dynamic_cast<HeatmapShader *>(shader))
   {
      heatmapShader->setMatrixBlock(*this);
      heatmapShader->setDataRange(self->dataRange);
      heatmapShader->drawLineStrip(self->length);
   }
   else if (auto envelopeShader = dynamic_cast<EnvelopeShader *>(shader))
   {
      envelopeShader->setMatrixBlock(*this);
      envelopeShader->setDataRange(self->dataRange);
      envelopeShader->drawLineStrip(self->length);
   }

   gl::Widget::draw(device, shader);
}

}

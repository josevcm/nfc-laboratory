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

#include <mutex>

#include <gl/engine/Buffer.h>
#include <gl/engine/Geometry.h>
#include <gl/typeface/FreeType.h>

#include <sdr/SignalBuffer.h>

#include <nfc/QuadratureData.h>
#include <nfc/QuadratureShader.h>

namespace nfc {

struct QuadratureData::Impl
{
   unsigned int samples;

   // signal data
   gl::Buffer dataValue;

   // last received buffer
   sdr::SignalBuffer signalBuffer;

   // signal buffer update mutex
   std::mutex signalMutex;
};

QuadratureData::QuadratureData(int samples) : self(std::make_shared<Impl>())
{
   self->samples = samples;

   // IQ signal buffer
   self->dataValue = gl::Buffer::createArrayBuffer(2 * samples * sizeof(float));
}

void QuadratureData::setCenterFreq(long value)
{
}

void QuadratureData::setSampleRate(long value)
{
}

void QuadratureData::refresh(const sdr::SignalBuffer &buffer)
{
   if (self->signalMutex.try_lock())
   {
      self->signalBuffer = buffer;
      self->signalMutex.unlock();
   }
}

gl::Widget *QuadratureData::resize(int width, int height)
{
   gl::Widget::resize(width, height);

   reset()->scale(2, 2, 1);

   return this;
}

void QuadratureData::update(float time, float delta)
{
   std::lock_guard<std::mutex> lock(self->signalMutex);

   if (self->signalBuffer.isValid() && self->signalBuffer.stride() == 2)
   {
      unsigned int points = std::min(self->signalBuffer.elements(), self->samples);

      self->dataValue.update(self->signalBuffer.data(), 0, points * 2 * sizeof(float));
   }
   else
   {
      self->dataValue.update(nullptr, 0, 0);
   }
}

void QuadratureData::draw(gl::Device *device, gl::Program *shader) const
{
   if (auto *qualityShader = dynamic_cast<QuadratureShader *>(shader))
   {
      qualityShader->setMatrixBlock(*this);
      qualityShader->setLineThickness(1.0f);
      qualityShader->setDataValue(self->dataValue);
      qualityShader->drawLineStrip(self->samples);
   }

   gl::Widget::draw(device, shader);
}

}
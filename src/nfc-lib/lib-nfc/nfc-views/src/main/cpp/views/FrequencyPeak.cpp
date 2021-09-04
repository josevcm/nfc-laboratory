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
#include <cmath>

#include <rt/Buffer.h>
#include <rt/Format.h>

#include <gl/engine/Text.h>
#include <gl/typeface/FreeType.h>

#include <sdr/SignalBuffer.h>

#include <nfc/PeakShader.h>
#include <nfc/FrequencyPeak.h>

namespace nfc {

struct FrequencyPeak::Impl
{
   const gl::Color peakColor {0.95f, 0.85f, 0.05f, 1.0f};

   unsigned int centerFreq;
   unsigned int sampleRate;
   unsigned int decimation;

   // signal buffer update mutex
   std::mutex signalMutex;

   // last received buffer
   sdr::SignalBuffer signalBuffer;

   // draw buffer
   gl::Buffer peakMarks;

   //
   gl::Text *frequencyCarrierLabel = nullptr;
   gl::Text *frequencyMinimumLabel = nullptr;
   gl::Text *frequencyMaximumLabel = nullptr;
};

FrequencyPeak::FrequencyPeak(int length) : self(std::make_shared<Impl>())
{
   self->peakMarks = gl::Buffer::createArrayBuffer(10 * sizeof(int));

   // create caption
   add(self->frequencyCarrierLabel = gl::FreeType::text("courbd", 11));
//   add(self->frequencyMinimumLabel = gl::FreeType::text("courbd", 12));
//   add(self->frequencyMaximumLabel = gl::FreeType::text("courbd", 12));
}

void FrequencyPeak::setCenterFreq(long value)
{
   self->centerFreq = value;
}

void FrequencyPeak::setSampleRate(long value)
{
   self->sampleRate = value;
}

void FrequencyPeak::refresh(const sdr::SignalBuffer &buffer)
{
   if (self->signalMutex.try_lock())
   {
      self->signalBuffer = buffer;
      self->signalMutex.unlock();
   }
}

gl::Widget *FrequencyPeak::resize(int width, int height)
{
   gl::Widget::resize(width, height);

   self->frequencyCarrierLabel->move(5, height - 40);

   return this;
}

void FrequencyPeak::update(float time, float delta)
{
   std::lock_guard<std::mutex> lock(self->signalMutex);

   if (self->signalBuffer.isValid())
   {
      const auto data = self->signalBuffer.data();
      const auto length = self->signalBuffer.capacity();

      // detect peaks
      int peaks[1] = {};
      float value[1] = {};
      float average = 0;
      float variance = 0;

      // compute signal average
      for (int i = 0; i < length; i++)
         average += data[i];

      average = average / (float) length;

      // compute signal variance
      for (int i = 0; i < length; i++)
         variance += (data[i] - average) * (data[i] - average);

      variance = variance / (float) length;

      // detect signal peaks
      for (int i = 0; i < length; i++)
      {
         float deviation = (data[i] - average) * (data[i] - average);

         if (value[0] < data[i] && (deviation > variance * 15 * 15))
         {
            value[0] = data[i];
            peaks[0] = i;
         }
      }

      self->peakMarks.update(peaks, 0, sizeof(peaks));

      if (peaks[0])
      {
         // get signal parameters
         float sampleRate = self->signalBuffer.sampleRate();
         float decimation = self->signalBuffer.decimation() > 0 ? self->signalBuffer.decimation() : 1.0f;

         // calculate frequency bin size
         float binSize = (sampleRate / decimation) / (float) self->signalBuffer.elements();

         // update carrier label
         float peakFreq = self->centerFreq - (sampleRate / (decimation * 2)) + binSize * peaks[0];

         // format carrier label
         self->frequencyCarrierLabel->setText(rt::Format::format("{.6} MHz", {peakFreq / 1E6}));
         self->frequencyCarrierLabel->setVisible(true);
      }

//      // format minimum label
//      self->frequencyMinimumLabel->setText(rt::Format::format("{.2}MHz", {(centerFreq - (sampleRate / (decimation * 2))) / 1E6}));
//      self->frequencyMinimumLabel->setVisible(true);
//      self->frequencyMinimumLabel->move(5, 4);
//
//      // format maximum label
//      self->frequencyMaximumLabel->setText(rt::Format::format("{.2}MHz", {(centerFreq + (sampleRate / (decimation * 2))) / 1E6}));
//      self->frequencyMaximumLabel->setVisible(true);
//      self->frequencyMaximumLabel->move(width() - self->frequencyMaximumLabel->width() - 15, 4);
   }
}

void FrequencyPeak::draw(gl::Device *device, gl::Program *shader) const
{
   if (auto peakShader = dynamic_cast<PeakShader *>(shader))
   {
      peakShader->setMatrixBlock(*this);
      peakShader->setObjectColor(self->peakColor);
      peakShader->setPeakMarks(self->peakMarks);
      peakShader->drawPoints(1);
   }

   gl::Widget::draw(device, shader);
}

}

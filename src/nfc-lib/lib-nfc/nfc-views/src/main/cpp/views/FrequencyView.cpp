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

#include <gl/engine/Buffer.h>
#include <gl/engine/Geometry.h>
#include <gl/typeface/FreeType.h>

#include <nfc/DefaultShader.h>
#include <nfc/FrequencyView.h>
#include <nfc/FrequencyGrid.h>
#include <nfc/FrequencyData.h>
#include <nfc/FrequencyPeak.h>

namespace nfc {

struct FrequencyView::Impl
{
   FrequencyGrid *gridView = nullptr;
   FrequencyData *dataView = nullptr;
   FrequencyPeak *peakView = nullptr;
};

FrequencyView::FrequencyView(int samples) : self(std::make_shared<Impl>())
{
   gl::Widget::add(self->gridView = new FrequencyGrid(samples));
   gl::Widget::add(self->dataView = new FrequencyData(samples));
   gl::Widget::add(self->peakView = new FrequencyPeak(samples));
}

void FrequencyView::setCenterFreq(long value)
{
   self->gridView->setCenterFreq(value);
   self->dataView->setCenterFreq(value);
   self->peakView->setCenterFreq(value);
}

void FrequencyView::setSampleRate(long value)
{
   self->gridView->setSampleRate(value);
   self->dataView->setSampleRate(value);
   self->peakView->setSampleRate(value);
}

void FrequencyView::refresh(const sdr::SignalBuffer &buffer)
{
   self->dataView->refresh(buffer);
   self->peakView->refresh(buffer);
}

gl::Widget *FrequencyView::resize(int width, int height)
{
   self->gridView->resize(width, height);
   self->dataView->resize(width, height);
   self->peakView->resize(width, height);

   return gl::Widget::resize(width, height);
}

}
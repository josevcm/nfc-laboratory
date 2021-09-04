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
#include <nfc/QuadratureView.h>
#include <nfc/QuadratureGrid.h>
#include <nfc/QuadratureData.h>

namespace nfc {

struct QuadratureView::Impl
{
   QuadratureGrid *gridView;
   QuadratureData *dataView;
};

QuadratureView::QuadratureView(int samples) : self(std::make_shared<Impl>())
{
   gl::Widget::add(self->gridView = new QuadratureGrid(samples));
   gl::Widget::add(self->dataView = new QuadratureData(samples));
}

void QuadratureView::setCenterFreq(long value)
{
}

void QuadratureView::setSampleRate(long value)
{
}

void QuadratureView::refresh(const sdr::SignalBuffer &buffer)
{
   self->dataView->refresh(buffer);
}

gl::Widget *QuadratureView::resize(int width, int height)
{
   self->gridView->resize(width, height);
   self->dataView->resize(width, height);

   return gl::Widget::resize(width, height);
}

}
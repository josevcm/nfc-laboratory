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

#ifndef NFC_LAB_QUADRATUREGRID_H
#define NFC_LAB_QUADRATUREGRID_H

#include <memory>

#include <gl/engine/Widget.h>

#include <nfc/SignalView.h>

namespace nfc {

class QuadratureGrid : public gl::Widget, public SignalView
{
      struct Impl;

   public:

      explicit QuadratureGrid(int samples = 65536);

      void setCenterFreq(long value);

      void setSampleRate(long value);

      gl::Widget *resize(int width, int height) override;

      void draw(gl::Device *device, gl::Program *shader) const override;

   private:

      std::shared_ptr<Impl> self;
};

}
#endif //NFC_LAB_QUADRATUREGRID_H

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

#include <nfc/QuadratureShader.h>

namespace nfc {

struct QuadratureShader::Impl
{
   int dataValueId = -1;
};

QuadratureShader::QuadratureShader(const gl::Assets *assets) : gl::ObjectShader(assets), self(std::make_shared<Impl>())
{
   load("QuadratureShader");
}

bool QuadratureShader::load(const std::string &name)
{
   if (gl::ObjectShader::load(name))
   {
      self->dataValueId = attribLocation("dataValue");

      return true;
   }

   return false;
}

void QuadratureShader::useProgram() const
{
   ObjectShader::useProgram();

   enableAttribArray(self->dataValueId);
}

void QuadratureShader::endProgram() const
{
   disableAttribArray(self->dataValueId);

   ObjectShader::endProgram();
}

void QuadratureShader::setDataValue(const gl::Buffer &buffer) const
{
   setVertexFloatArray(self->dataValueId, buffer, 2);
}

}
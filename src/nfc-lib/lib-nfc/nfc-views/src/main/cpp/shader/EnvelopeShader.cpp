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

#include <nfc/EnvelopeShader.h>

namespace nfc {

struct EnvelopeShader::Impl
{
   int dataRangeId = -1;
};

EnvelopeShader::EnvelopeShader(const gl::Assets *assets) : gl::GeometryShader(assets), self(std::make_shared<Impl>())
{
   load("EnvelopeShader");
}

bool EnvelopeShader::load(const std::string &name)
{
   if (gl::GeometryShader::load(name))
   {
      self->dataRangeId = attribLocation("dataRange");

      return true;
   }

   return false;
}

void EnvelopeShader::useProgram() const
{
   GeometryShader::useProgram();

   enableAttribArray(self->dataRangeId);
}

void EnvelopeShader::endProgram() const
{
   disableAttribArray(self->dataRangeId);

   GeometryShader::endProgram();
}

void EnvelopeShader::setDataRange(const gl::Buffer &buffer) const
{
   setVertexFloatArray(self->dataRangeId, buffer, 1);
}

}
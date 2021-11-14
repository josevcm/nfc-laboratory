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

#include <nfc/SignalSmoother.h>

namespace nfc {

struct SignalSmoother::Impl
{
   // data block counter
   int dataBlock = 0;

   // input locations
   int dataRangeId = -1;
   int dataValueId = -1;
   int dataBlockId = -1;

   // config buffer
   gl::Buffer config;
};

SignalSmoother::SignalSmoother(const gl::Assets *assets) : gl::ObjectShader(assets), self(std::make_shared<Impl>())
{
   load("SignalSmoother");
}

bool SignalSmoother::load(const std::string &name)
{
   if (gl::ObjectShader::load(name))
   {
      self->dataRangeId = attribLocation("dataRange");
      self->dataValueId = attribLocation("dataValue");
      self->dataBlockId = uniformLocation("dataBlock");

      self->config = gl::Buffer::createUniformBuffer(sizeof(SmoothParameters));

      return true;
   }

   return false;
}

void SignalSmoother::useProgram() const
{
   gl::ObjectShader::useProgram();

   enableAttribArray(self->dataRangeId);
   enableAttribArray(self->dataValueId);
}

void SignalSmoother::endProgram() const
{
   disableAttribArray(self->dataRangeId);
   disableAttribArray(self->dataValueId);

   gl::ObjectShader::endProgram();
}

void SignalSmoother::process(const gl::Buffer &range, const gl::Buffer &value, const SmoothParameters &params, int elements)
{
   // prepare program parameters and bind to location 0
   self->config.update(&params, 0, sizeof(params)).bind(0);

   // bind buffer to shader
   setVertexFloatArray(self->dataRangeId, range, 1);
   setVertexFloatArray(self->dataValueId, value, 1);

   // set data block counter
   setUniformInteger(self->dataBlockId, {self->dataBlock++});

   // run shader program
   drawPoints(elements);
}

}
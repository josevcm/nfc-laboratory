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

#include <opengl/GL.h>

#include <rt/Logger.h>

#include <gl/engine/Model.h>
#include <gl/engine/Buffer.h>
#include <gl/engine/Program.h>

#include <gl/shader/AbstractShader.h>

namespace gl {

struct AbstractShader::Impl
{
   rt::Logger log {"AbstractShader"};

   long programId = -1;

   int sampler0Id = -1;
   int sampler1Id = -1;
   int sampler2Id = -1;
   int sampler3Id = -1;

   int matrixBlockId = -1;
   int modelMatrixOffset = -1;
   int worldMatrixOffset = -1;
   int normalMatrixOffset = -1;
   int mvProjMatrixOffset = -1;

   Buffer matrixBlock;

   Impl()
   {
      matrixBlock = Buffer::createUniformBuffer(4 * 4 * 4 * sizeof(float));
   }
};

AbstractShader::AbstractShader() : self(new Impl)
{
}

AbstractShader::~AbstractShader()
{
   delete self;
}

void AbstractShader::useProgram() const
{
   Program::useProgram();

   if (self->sampler0Id != -1)
      glUniform1i(self->sampler0Id, 0);

   if (self->sampler1Id != -1)
      glUniform1i(self->sampler1Id, 1);

   if (self->sampler2Id != -1)
      glUniform1i(self->sampler2Id, 2);

   if (self->sampler3Id != -1)
      glUniform1i(self->sampler3Id, 3);

   self->matrixBlock.bind(0);
}

void AbstractShader::endProgram() const
{
   Program::endProgram();
}

void AbstractShader::drawPoints(unsigned int count) const
{
   glDrawArrays(GL_POINTS, 0, count);
}

void AbstractShader::drawLines(unsigned int count) const
{
   glDrawArrays(GL_LINES, 0, count);
}

void AbstractShader::drawLineStrip(unsigned int count) const
{
   glDrawArrays(GL_LINE_STRIP, 0, count);
}

void AbstractShader::drawLineStrip(const int *list, int unsigned count) const
{
   glDrawElements(GL_LINE_STRIP, count, GL_UNSIGNED_INT, list);
}

void AbstractShader::drawLineStrip(const Buffer &buffer, unsigned int count) const
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id());
   glDrawElements(GL_LINE_STRIP, count > 0 ? count : buffer.elements(), GL_UNSIGNED_INT, nullptr);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void AbstractShader::drawLineLoop(unsigned int count) const
{
   glDrawArrays(GL_LINE_LOOP, 0, count);
}

void AbstractShader::drawLineLoop(const int *list, int unsigned count) const
{
   glDrawElements(GL_LINE_LOOP, count, GL_UNSIGNED_INT, list);
}

void AbstractShader::drawLineLoop(const Buffer &buffer, unsigned int count) const
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id());
   glDrawElements(GL_LINE_LOOP, count > 0 ? count : buffer.elements(), GL_UNSIGNED_INT, nullptr);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void AbstractShader::drawTriangles(const int *list, int unsigned count) const
{
   glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, list);
}

void AbstractShader::drawTriangles(const Buffer &buffer, unsigned int count) const
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id());
   glDrawElements(GL_TRIANGLES, count > 0 ? count : buffer.elements(), GL_UNSIGNED_INT, nullptr);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void AbstractShader::setLineThickness(float thickness) const
{
   glLineWidth(thickness);
}

void AbstractShader::setMatrixBlock(const Model &model) const
{
   if (self->modelMatrixOffset != -1)
      self->matrixBlock.update(model.modelMatrix().matrix, self->modelMatrixOffset, 4 * 4 * sizeof(float));

   if (self->worldMatrixOffset != -1)
      self->matrixBlock.update(model.worldMatrix().matrix, self->worldMatrixOffset, 4 * 4 * sizeof(float));

   if (self->normalMatrixOffset != -1)
      self->matrixBlock.update(model.normalMatrix().matrix, self->normalMatrixOffset, 4 * 4 * sizeof(float));

   if (self->mvProjMatrixOffset != -1)
      self->matrixBlock.update(model.projMatrix().matrix, self->mvProjMatrixOffset, 4 * 4 * sizeof(float));
}

void AbstractShader::setVertexByteArray(int location, const Buffer &buffer, int components, int offset, int stride) const
{
   if (location != -1 && buffer.valid())
   {
      glBindBuffer(GL_ARRAY_BUFFER, buffer.id());
      glVertexAttribIPointer(location, components, GL_BYTE, stride ? stride : buffer.stride(), reinterpret_cast<const void *>(offset));
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }
}

void AbstractShader::setVertexShortArray(int location, const Buffer &buffer, int components, int offset, int stride) const
{
   if (location != -1 && buffer.valid())
   {
      glBindBuffer(GL_ARRAY_BUFFER, buffer.id());
      glVertexAttribIPointer(location, components, GL_SHORT, stride ? stride : buffer.stride(), reinterpret_cast<const void *>(offset));
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }
}

void AbstractShader::setVertexIntegerArray(int location, const Buffer &buffer, int components, int offset, int stride) const
{
   if (location != -1 && buffer.valid())
   {
      glBindBuffer(GL_ARRAY_BUFFER, buffer.id());
      glVertexAttribIPointer(location, components, GL_INT, stride ? stride : buffer.stride(), reinterpret_cast<const void *>(offset));
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }
}

void AbstractShader::setVertexFloatArray(int location, const Buffer &buffer, int components, int offset, int stride) const
{
   if (location != -1 && buffer.valid())
   {
      glBindBuffer(GL_ARRAY_BUFFER, buffer.id());
      glVertexAttribPointer(location, components, GL_FLOAT, false, stride ? stride : buffer.stride(), reinterpret_cast<const void *>(offset));
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }
}

void AbstractShader::setUniformInteger(int location, const std::vector<int> &values) const
{
   if (location != -1)
   {
      switch (values.size())
      {
         case 1:
            glUniform1i(location, values[0]);
            break;
         case 2:
            glUniform2i(location, values[0], values[1]);
            break;
         case 3:
            glUniform3i(location, values[0], values[1], values[2]);
            break;
         case 4:
            glUniform4i(location, values[0], values[1], values[2], values[3]);
            break;
      }
   }
}

void AbstractShader::setUniformFloat(int location, const std::vector<float> &values) const
{
   if (location != -1)
   {
      switch (values.size())
      {
         case 1:
            glUniform1f(location, values[0]);
            break;
         case 2:
            glUniform2f(location, values[0], values[1]);
            break;
         case 3:
            glUniform3f(location, values[0], values[1], values[2]);
            break;
         case 4:
            glUniform4f(location, values[0], values[1], values[2], values[3]);
            break;
      }
   }
}

void AbstractShader::enableAttribArray(int location) const
{
   if (location != -1)
      glEnableVertexAttribArray(location);
}

void AbstractShader::disableAttribArray(int location) const
{
   if (location != -1)
      glDisableVertexAttribArray(location);
}

bool AbstractShader::linkProgram()
{
   if (Program::linkProgram())
   {
      // setup samplers uniform locations
      self->sampler0Id = uniformLocation("uSampler0");
      self->sampler1Id = uniformLocation("uSampler1");
      self->sampler2Id = uniformLocation("uSampler2");
      self->sampler3Id = uniformLocation("uSampler3");

      // setup uniform matrix Alloc
      self->matrixBlockId = uniformBlock("MatrixBlock");
      self->modelMatrixOffset = uniformBlockOffset(self->matrixBlockId, "MatrixBlock.modelMatrix");
      self->worldMatrixOffset = uniformBlockOffset(self->matrixBlockId, "MatrixBlock.worldMatrix");
      self->normalMatrixOffset = uniformBlockOffset(self->matrixBlockId, "MatrixBlock.normalMatrix");
      self->mvProjMatrixOffset = uniformBlockOffset(self->matrixBlockId, "MatrixBlock.mvProjMatrix");

      return true;
   }

   return false;
}

}
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

#ifndef UI_GL_ABSTRACTSHADER_H
#define UI_GL_ABSTRACTSHADER_H

#include <vector>
#include <string>

#include <gl/engine/Program.h>

namespace gl {

class Buffer;

class AbstractShader : public Program
{
      struct Impl;

   public:

      AbstractShader();

      ~AbstractShader() override;

      virtual bool load(const std::string &name) = 0;

      void useProgram() const override;

      void endProgram() const override;

      void drawPoints(unsigned int count) const;

      void drawLines(unsigned int count) const;

      void drawLineStrip(unsigned int count) const;

      void drawLineStrip(const int *list, unsigned int count) const;

      void drawLineStrip(const Buffer &buffer, unsigned int count) const;

      void drawLineLoop(unsigned int count) const;

      void drawLineLoop(const int *list, unsigned int count) const;

      void drawLineLoop(const Buffer &buffer, unsigned int count) const;

      void drawTriangles(const int *list, unsigned int count) const;

      void drawTriangles(const Buffer &buffer, unsigned int count) const;

      void setLineThickness(float thickness) const;

      void setMatrixBlock(const Model &model) const;

      void setVertexByteArray(int location, const Buffer &buffer, int components, int offset = 0, int stride = 0) const;

      void setVertexShortArray(int location, const Buffer &buffer, int components, int offset = 0, int stride = 0) const;

      void setVertexIntegerArray(int location, const Buffer &buffer, int components, int offset = 0, int stride = 0) const;

      void setVertexFloatArray(int location, const Buffer &buffer, int components, int offset = 0, int stride = 0) const;

      void setUniformInteger(int location, const std::vector<int> &values) const;

      void setUniformFloat(int location, const std::vector<float> &values) const;

   protected:

      void enableAttribArray(int id) const;

      void disableAttribArray(int id) const;

      bool linkProgram() override;

   private:

      Impl *self;
};

}

#endif //NFC_VIEW_ABSTRACTSHADER_H

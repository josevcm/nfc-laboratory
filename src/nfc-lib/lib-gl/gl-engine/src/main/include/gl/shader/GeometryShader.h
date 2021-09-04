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

#ifndef UI_GL_GEOMETRYSHADER_H
#define UI_GL_GEOMETRYSHADER_H

#include <gl/shader/AbstractShader.h>

namespace gl {

class Assets;
class Buffer;
class Point;
class Color;
class Ligthing;
class Geometry;

class GeometryShader : public AbstractShader
{
      struct Impl;

   public:

      explicit GeometryShader(const Assets *asset, const std::string &name = {});

      ~GeometryShader() override;

      bool load(const std::string &name) override;

      void useProgram() const override;

      void endProgram() const override;

      void setLightingModel(const Ligthing &ligthing) const;

      void setObjectColor(const Color &color) const;

      void setAmbientLigthColor(const Color &color) const;

      void setDiffuseLigthColor(const Color &color) const;

      void setDiffuseLigthVector(const Point &vector) const;

      void setVertexColors(const Buffer &buffer, int components = 4, int offset = 0, int stride = 0) const;

      void setVertexNormals(const Buffer &buffer, int components = 4, int offset = 0, int stride = 0) const;

      void setVertexPoints(const Buffer &buffer, int components = 3, int offset = 0, int stride = 0) const;

      void setVertexTexels(const Buffer &buffer, int components = 2, int offset = 0, int stride = 0) const;

      void drawGeometry(const Geometry &geometry, int elements = 0) const;

   protected:

      const Assets *assets();

   private:

      Impl *self;
};

}
#endif //GLE_SHADER_DEFAULTSHADER_H

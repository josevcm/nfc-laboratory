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

#include <gl/engine/Colors.h>
#include <gl/engine/Assets.h>
#include <gl/shader/FontShader.h>

namespace gl {

struct FontShader::Impl
{
   rt::Logger log {"FontShader"};

   int uFontColorId = -1;
   int uFontSmoothId = -1;
   int uStrokeColorId = -1;
   int uStrokeWidthId = -1;
   int uShadowColorId = -1;
   int uShadowSmoothId = -1;
   int uShadowOffsetId = -1;
};

FontShader::FontShader(const Assets *assets) : GeometryShader(assets), self(new Impl)
{
   if (!load("FontShader"))
   {
      self->log.error("program load error, font shader not available!");
   }
}

FontShader::~FontShader()
{
   delete self;
}

bool FontShader::load(const std::string &name)
{
   if (GeometryShader::load(name))
   {
      // setup font render properties
      self->uFontColorId = uniformLocation("uFontColor");
      self->uFontSmoothId = uniformLocation("uFontSmooth");
      self->uStrokeColorId = uniformLocation("uStrokeColor");
      self->uStrokeWidthId = uniformLocation("uStrokeWidth");
      self->uShadowColorId = uniformLocation("uShadowColor");
      self->uShadowSmoothId = uniformLocation("uShadowSmooth");
      self->uShadowOffsetId = uniformLocation("uShadowOffset");

      return true;
   }

   return false;
}

void FontShader::setFontColor(const Color &color) const
{
   if (self->uFontColorId != -1)
      glUniform4f(self->uFontColorId, color.r, color.g, color.b, color.a);
}

void FontShader::setFontSmooth(float factor) const
{
   if (self->uFontSmoothId != -1)
      glUniform1f(self->uFontSmoothId, factor);
}

void FontShader::setStrokeColor(const Color &color) const
{
   if (self->uStrokeColorId != -1)
      glUniform4f(self->uStrokeColorId, color.r, color.g, color.b, color.a);
}

void FontShader::setStrokeWidth(float width) const
{
   if (self->uStrokeWidthId != -1)
      glUniform1f(self->uStrokeWidthId, width);
}

void FontShader::setShadowColor(const Color &color) const
{
   if (self->uShadowColorId != -1)
      glUniform4f(self->uShadowColorId, color.r, color.g, color.b, color.a);
}

void FontShader::setShadowSmooth(float factor) const
{
   if (self->uShadowSmoothId != -1)
      glUniform1f(self->uShadowSmoothId, factor);
}

void FontShader::setShadowOffset(const Vector &offset) const
{
   if (self->uShadowOffsetId != -1)
      glUniform2f(self->uShadowOffsetId, offset.x, offset.y);
}

}
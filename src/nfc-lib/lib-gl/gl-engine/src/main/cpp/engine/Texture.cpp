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

#include <gl/engine/Texture.h>

namespace gl {

struct Texture::Impl
{
   unsigned int id;
   unsigned int type;
   unsigned int size;
   unsigned int width;
   unsigned int height;

   Impl(int type, const void *buffer, unsigned int size, unsigned int width, unsigned int height) : id(0), type(type), size(size), width(width), height(height)
   {
      glGenTextures(1, &id);
      glBindTexture(GL_TEXTURE_2D, id);
      glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, buffer);
      glBindTexture(GL_TEXTURE_2D, 0);
   }

   ~Impl()
   {
      glDeleteTextures(1, &id);
   }

   void activate(int unit) const
   {
      // activamos textura
      glActiveTexture(GL_TEXTURE0 + unit);

      // asociamos textura
      glBindTexture(GL_TEXTURE_2D, id);

      // ajustamos opciones de filtrado
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   }

//   void update(const void *data, unsigned int offset, unsigned int size, unsigned int elements, unsigned int stride)
//   {
//      this->size = size ? size : this->size;
//      this->elements = elements ? elements : this->elements;
//      this->stride = stride ? stride : this->stride;
//
//      glBindTexture(GL_TEXTURE_2D, id);
//      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mTextureWidth, mTextureHeight, GL_RGBA, GL_UNSIGNED_BYTE, mPixels);
//      glBindTexture(GL_TEXTURE_2D, 0);
//   }
};

Texture::Texture(Impl *shared) : self(shared)
{
}

Texture &Texture::operator=(const Texture &other)
{
   if (this != &other)
      self = other.self;

   return *this;
}

unsigned int Texture::id() const
{
   return self ? self->id : 0;
}

void Texture::bind(int unit) const
{
   if (self)
      self->activate(unit);
}

Texture Texture::createTexture(int type, const void *buffer, unsigned int size, unsigned int width, unsigned int height)
{
   return Texture {new Impl(type, buffer, size, width, height)};
}

}

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

#include <map>

#include <gl/engine/Font.h>

namespace gl {

struct Font::Impl
{
   int size;

   std::map<int, Quad> quads;

   Impl(int size, const std::vector<Quad> &chars) : size(size)
   {
      for (const Quad &quad : chars)
      {
         quads[quad.ch] = quad;
      }
   }
};

Font::Font(int size, const std::vector<Quad> &quads, const Texture &texture) : Texture(texture), self(new Impl(size, quads))
{
}

Font &Font::operator=(const Font &other)
{
   if (this == &other)
      return *this;

   Texture::operator=(other);

   self = other.self;

   return *this;
}

Font::operator bool() const
{
   return self.operator bool();
}

const Quad &Font::getQuad(int ch) const
{
   return self->quads[ch];
}

int Font::size() const
{
   return self->size;
}

}

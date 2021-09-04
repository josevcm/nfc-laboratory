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

#ifndef UI_GL_FONT_H
#define UI_GL_FONT_H

#include <vector>
#include <memory>

#include <gl/engine/Texture.h>

namespace gl {

struct Quad
{
   int ch = 0;

   // texture coordinates
   float texelLeft = 0;
   float texelRight = 0;
   float texelTop = 0;
   float texelBottom = 0;

   // glyp origin coordinates
   float glyphLeft = 0;
   float glyphRight = 0;
   float glyphTop = 0;
   float glyphBottom = 0;

   // character properties
   int charWidth = 0;
   int charHeight = 0;
   int charAdvance = 0;

   explicit operator bool() const
   {
      return ch;
   }
};

class Font : public Texture
{
      struct Impl;

   public:

      Font() = default;

      Font(int size, const std::vector<Quad> &quads, const Texture &texture);

      Font &operator=(const Font &other);

      explicit operator bool() const;

      int size() const;

      const Quad &getQuad(int ch) const;

   private:

      std::shared_ptr<Impl> self;
};

}

#endif //GLE_ENGINE_FONT_H

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

#ifndef UI_GL_TEXTWIDGET_H
#define UI_GL_TEXTWIDGET_H

#include <gl/engine/Widget.h>

namespace gl {

class Font;

class TextWidget : public Widget
{
      struct Impl;

   public:

      explicit TextWidget(const Font &font, const std::string &text, float scale = 1.0);

      ~TextWidget() override;

      void setText(const std::string &text);

      void setFontSmooth(float fontSmooth);

      void setFontColor(float r, float g, float b, float a);

      void setShadowSmooth(float shadowSmooth);

      void setShadowOffset(float x, float y);

      void setShadowColor(float r, float g, float b, float a);

      void setStrokeColor(float r, float g, float b, float a);

      void setStrokeWidth(float strokeWidth);

      void draw(Device *device, Program *shader) const override;

   private:

      Impl *widget;

};

}

#endif //NFC_VIEW_FIELDFONT_H

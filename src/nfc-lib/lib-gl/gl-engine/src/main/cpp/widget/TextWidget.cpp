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

#include <rt/Logger.h>

#include <gl/engine/Buffer.h>
#include <gl/engine/Colors.h>
#include <gl/engine/Font.h>
#include <gl/shader/FontShader.h>
#include <gl/widget/TextWidget.h>

namespace gl {

struct TextWidget::Impl
{
   rt::Logger log {"TextWidget"};

   Font font;
   std::string text;

   float aspect = 2;
   float scale = 1;
   float width = 0;
   float height = 0;

   float fontSmooth = 0.05f;
   Color fontColor = Colors::WHITE;

   float shadowSmooth = 0.5f;
   Color shadowColor = Colors::BLACK;;
   Vector shadowOffset = {0, 0, 0};

   Color strokeColor = Colors::RED;
   float strokeWidth = 0.0f;

   Buffer quadVertex;
   Buffer quadTexels;
   Buffer quadIndex;
};

TextWidget::TextWidget(const Font &font, const std::string &text, float scale) : widget(new Impl)
{
   widget->font = font;
   widget->scale = scale;

   setText(text);
}

TextWidget::~TextWidget()
{
   delete widget;
}

void TextWidget::setFontSmooth(float fontSmooth)
{
   widget->fontSmooth = fontSmooth;
}

void TextWidget::setFontColor(float r, float g, float b, float a)
{
   widget->fontColor = {r, g, b, a};
}

void TextWidget::setShadowSmooth(float shadowSmooth)
{
   widget->shadowSmooth = shadowSmooth;
}

void TextWidget::setShadowColor(float r, float g, float b, float a)
{
   widget->shadowColor = {r, g, b, a};
}

void TextWidget::setShadowOffset(float x, float y)
{
   widget->shadowOffset = {x, y, 0};
}

void TextWidget::setStrokeColor(float r, float g, float b, float a)
{
   widget->strokeColor = {r, g, b, a};
}

void TextWidget::setStrokeWidth(float strokeWidth)
{
   widget->strokeWidth = strokeWidth;
}

void TextWidget::setText(const std::string &text)
{
//   float x = 0;
//   float y = 0;
//
//   widget->text = text;
//   widget->width = 0;
//   widget->height = 0;
//
//   // primera pasada para calcular dimensiones y centrar el texto
//   for (char ch : text)
//   {
//      Quad quad = widget->font.getQuad(ch & 0xff);
//
//      float ratio = 2 * widget->aspect * widget->scale / quad.spacing;
//      float advance = ratio * quad.advance / widget->aspect;
//      float spacing = ratio * quad.spacing / 2;
//
//      widget->width += advance;
//
//      if (spacing > widget->height)
//         widget->height = spacing;
//   }
//
//   x = -widget->width / 2;
//
//   float vertex[12 * widget->text.size()];
//   float texels[8 * widget->text.size()];
//   int index[6 * widget->text.size()];
//
//   int i = 0;
//
//   // segunda pasda, calculamos coordenadas de cada caracter
//   for (char ch : text)
//   {
//      Quad quad = widget->font.getQuad(ch & 0xff);
//
//      if (quad)
//      {
//         float ratio = 2 * widget->aspect * widget->scale / quad.spacing;
//         float advance = ratio * quad.advance / widget->aspect;
//         float spacing = ratio * quad.spacing / 2;
//
//         float x0 = x;
//         float y0 = y + spacing;
//         float x1 = x + advance;
//         float y1 = y - spacing;
//
//         vertex[12 * i + 0] = x0;
//         vertex[12 * i + 1] = y0;
//         vertex[12 * i + 2] = 0;
//
//         vertex[12 * i + 3] = x0;
//         vertex[12 * i + 4] = y1;
//         vertex[12 * i + 5] = 0;
//
//         vertex[12 * i + 6] = x1;
//         vertex[12 * i + 7] = y1;
//         vertex[12 * i + 8] = 0;
//
//         vertex[12 * i + 9] = x1;
//         vertex[12 * i + 10] = y0;
//         vertex[12 * i + 11] = 0;
//
//         texels[8 * i + 0] = quad.left;
//         texels[8 * i + 1] = quad.top;
//
//         texels[8 * i + 2] = quad.left;
//         texels[8 * i + 3] = quad.bottom;
//
//         texels[8 * i + 4] = quad.right;
//         texels[8 * i + 5] = quad.bottom;
//
//         texels[8 * i + 6] = quad.right;
//         texels[8 * i + 7] = quad.top;
//
//         index[6 * i + 0] = i * 4 + 0;
//         index[6 * i + 1] = i * 4 + 1;
//         index[6 * i + 2] = i * 4 + 2;
//         index[6 * i + 3] = i * 4 + 0;
//         index[6 * i + 4] = i * 4 + 2;
//         index[6 * i + 5] = i * 4 + 3;
//
//         x += advance;
//      }
//
//      i++;
//   }
//
//   widget->quadVertex = Buffer::createArrayBuffer(sizeof(vertex), vertex, sizeof(vertex) / (3 * sizeof(float)));
//   widget->quadTexels = Buffer::createArrayBuffer(sizeof(texels), texels, sizeof(texels) / (2 * sizeof(float)));
//   widget->quadIndex = Buffer::createElementBuffer(sizeof(index), index, sizeof(index) / sizeof(unsigned int));
}

void TextWidget::draw(Device *device, Program *shader) const
{
   if (auto fontShader = dynamic_cast<FontShader *>(shader))
   {
      widget->font.bind(0);

      fontShader->setMatrixBlock(*this);

      fontShader->setFontColor(widget->fontColor);
      fontShader->setFontSmooth(widget->fontSmooth);

      fontShader->setShadowColor(widget->shadowColor);
      fontShader->setShadowOffset(widget->shadowOffset);
      fontShader->setShadowSmooth(widget->shadowSmooth);

      fontShader->setStrokeColor(widget->strokeColor);
      fontShader->setStrokeWidth(widget->strokeWidth);

      fontShader->setVertexPoints(widget->quadVertex);
      fontShader->setVertexTexels(widget->quadTexels);

      fontShader->drawTriangles(widget->quadIndex, widget->quadIndex.elements());
   }
}

}
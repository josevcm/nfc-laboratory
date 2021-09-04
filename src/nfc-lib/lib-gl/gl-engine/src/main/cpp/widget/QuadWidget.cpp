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
#include <gl/engine/Texture.h>
#include <gl/engine/Geometry.h>
#include <gl/shader/TextureShader.h>
#include <gl/widget/QuadWidget.h>

namespace gl {

constexpr static Vertex vertex[] = {{{-1, -1, 0}, {}, {0, 0}},
                                    {{1,  -1, 0}, {}, {1, 0}},
                                    {{1,  1,  0}, {}, {1, 1}},
                                    {{-1, 1,  0}, {}, {0, 1}}};

constexpr static unsigned int index[] {
      0, 1, 3,
      1, 2, 3
};

struct QuadWidget::Impl
{
   rt::Logger log {"QuadWidget"};

   Texture texture;
   Buffer vertex;
   Buffer index;
};

QuadWidget::QuadWidget(const Texture &texture) : widget(new Impl)
{
   widget->texture = texture;
   widget->vertex = Buffer::createArrayBuffer(sizeof(vertex), vertex, sizeof(vertex) / sizeof(Vertex), sizeof(Vertex));
   widget->index = Buffer::createElementBuffer(sizeof(index), index, sizeof(index) / sizeof(unsigned int));
}

QuadWidget::~QuadWidget()
{
   delete widget;
}

void QuadWidget::draw(Device *device, Program *shader) const
{
   if (auto textureShader = dynamic_cast<TextureShader *>(shader))
   {
      widget->texture.bind(0);

      textureShader->setMatrixBlock(*this);
      textureShader->setObjectColor({1.0, 1.0, 1.0, 1.0});
      textureShader->setVertexPoints(widget->vertex, offsetof(Vertex, point));
      textureShader->setVertexTexels(widget->vertex, offsetof(Vertex, texel));
      textureShader->drawTriangles(widget->index, widget->index.elements());
   }
}

}
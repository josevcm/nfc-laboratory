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
#include <gl/engine/Geometry.h>
#include <gl/shader/ObjectShader.h>
#include <gl/widget/PanelWidget.h>

namespace gl {

struct PanelWidget::Impl
{
   rt::Logger log {"PanelWidget"};

   Buffer vertex;
   Buffer index;
};

PanelWidget::PanelWidget(float width, float height, Color color) : widget(new Impl)
{
   Vertex vertex[] = {{{-width / 2, -height / 2, 0}, color},
                      {{-width / 2, +height / 2, 0}, color},
                      {{+width / 2, -height / 2, 0}, color},
                      {{+width / 2, +height / 2, 0}, color}};

   unsigned int index[] = {0, 2, 1};

   widget->vertex = Buffer::createArrayBuffer(sizeof(vertex), vertex, sizeof(vertex) / sizeof(Vertex), sizeof(Vertex));
   widget->index = Buffer::createElementBuffer(sizeof(index), index, sizeof(index) / sizeof(unsigned int), sizeof(unsigned int));
}

PanelWidget::~PanelWidget()
{
   delete widget;
}

void PanelWidget::draw(Device *device, Program *shader) const
{
   if (auto geometryShader = dynamic_cast<ObjectShader *>(shader))
   {
      geometryShader->setMatrixBlock(*this);
      geometryShader->setVertexPoints(widget->vertex, offsetof(Vertex, point));
      geometryShader->setVertexColors(widget->vertex, offsetof(Vertex, color));
      geometryShader->drawTriangles(widget->index, widget->index.elements());
   }
}

}
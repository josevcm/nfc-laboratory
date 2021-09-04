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
#include <gl/shader/GeometryShader.h>
#include <gl/widget/AxisWidget.h>

namespace gl {

struct AxisWidget::Impl
{
   rt::Logger log {"AxisWidget"};

   Buffer vertex;
};

AxisWidget::AxisWidget() : widget(new Impl)
{
   widget->log.debug("create AxisWidget");

   float c = 0.60;

   Vertex vertex[] = {{{-1, 0,  0},  {c, c, c, 1}},
                      {{1,  0,  0},  {c, c, c, 1}},
                      {{0,  -1, 0},  {c, c, c, 1}},
                      {{0,  1,  0},  {c, c, c, 1}},
                      {{0,  0,  -1}, {c, c, c, 1}},
                      {{0,  0,  1},  {c, c, c, 1}}};

   widget->vertex = Buffer::createArrayBuffer(sizeof(vertex), vertex, sizeof(vertex) / sizeof(Vertex), sizeof(Vertex));
}

AxisWidget::~AxisWidget()
{
   delete widget;
}

void AxisWidget::draw(Device *device, Program *shader) const
{
   if (auto geometryShader = dynamic_cast<GeometryShader *>(shader))
   {
      geometryShader->setMatrixBlock(*this);
      geometryShader->setLineThickness(1.0f);
      geometryShader->setVertexPoints(widget->vertex, offsetof(Vertex, point));
      geometryShader->setVertexColors(widget->vertex, offsetof(Vertex, color));
      geometryShader->drawLines(widget->vertex.elements());
   }
}

}
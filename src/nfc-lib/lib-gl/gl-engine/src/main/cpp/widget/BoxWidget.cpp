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
#include <gl/widget/BoxWidget.h>

namespace gl {


struct BoxWidget::Impl
{
   rt::Logger log {"BoxWidget"};

   Buffer vertex;
   Buffer index;
};

BoxWidget::BoxWidget(Color color) : widget(new Impl)
{
   widget->log.debug("create AxisWidget");

   Vertex vertex[] {
         {{-1, -1, -1}, color},
         {{1,  -1, -1}, color},
         {{1,  1,  -1}, color},
         {{-1, 1,  -1}, color},
         {{-1, -1, 1},  color},
         {{1,  -1, 1},  color},
         {{1,  1,  1},  color},
         {{-1, 1,  1},  color}};

   unsigned int index[] {
         5, 4, 0,
         1, 5, 0,
         6, 5, 1,
         2, 6, 1,
         7, 6, 2,
         3, 7, 2,
         4, 7, 3,
         0, 4, 3,
         6, 7, 4,
         5, 6, 4,
         1, 0, 3,
         2, 1, 3
   };

   widget->vertex = Buffer::createArrayBuffer(sizeof(vertex), vertex, sizeof(vertex) / sizeof(Vertex), sizeof(Vertex));
   widget->index = Buffer::createElementBuffer(sizeof(index), index, sizeof(index) / sizeof(unsigned int), sizeof(unsigned int));
}

BoxWidget::~BoxWidget()
{
   delete widget;
}

void BoxWidget::draw(Device *device, Program *shader) const
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
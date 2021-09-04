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
#include <gl/widget/GridWidget.h>

namespace gl {

struct GridWidget::Impl
{
   rt::Logger log {"GridWidget"};

   // recuadro exterior
   Buffer borderCoords;
   Buffer borderColors;

   // divisiones regilla
   Buffer gridCoords;
   Buffer gridColors;
};

constexpr static float colors[]
      {
            0.75f, 0.75f, 0.75f, 1.0f,
            0.75f, 0.75f, 0.75f, 1.0f,
            0.75f, 0.75f, 0.75f, 1.0f,
            0.75f, 0.75f, 0.75f, 1.0f,
      };

GridWidget::GridWidget(float z, float width, float height, int vdiv, int hdiv) : widget(new Impl)
{
   float gridLines[(vdiv - 1) * 2 * 3 + (hdiv - 1) * 2 * 3];
   float gridColor[(vdiv - 1) * 2 * 4 + (hdiv - 1) * 2 * 4];

   auto l = gridLines;
   auto c = gridColor;
   auto wd = width / vdiv;
   auto w2 = width / 2;
   auto hd = height / hdiv;
   auto h2 = height / 2;

   // fill vertical grid
   for (int i = 1; i < vdiv; i++)
   {
      *l++ = i * wd - w2; // x
      *l++ = -h2;                   // y
      *l++ = z;                    // z

      *l++ = i * wd - w2; // x
      *l++ = h2;                    // y
      *l++ = z;                    // z

      *c++ = colors[0];
      *c++ = colors[1];
      *c++ = colors[2];
      *c++ = colors[3];

      *c++ = colors[4];
      *c++ = colors[5];
      *c++ = colors[6];
      *c++ = colors[7];
   }

   // fill horizontal grid
   for (int i = 1; i < hdiv; i++)
   {
      *l++ = -w2;         // x
      *l++ = i * hd - h2; // y
      *l++ = z;           // z

      *l++ = w2;          // x
      *l++ = i * hd - h2; // y
      *l++ = z;           // z

      *c++ = colors[0];
      *c++ = colors[1];
      *c++ = colors[2];
      *c++ = colors[3];

      *c++ = colors[4];
      *c++ = colors[5];
      *c++ = colors[6];
      *c++ = colors[7];
   }

   float outline[] {-w2, +h2, z, w2, h2, z, w2, -h2, z, -w2, -h2, z,};

   widget->borderCoords = Buffer::createArrayBuffer(sizeof(outline), outline, sizeof(outline) / (3 * sizeof(float)));
   widget->borderColors = Buffer::createArrayBuffer(sizeof(colors), colors, sizeof(colors) / (4 * sizeof(float)));
   widget->gridCoords = Buffer::createArrayBuffer(sizeof(gridLines), gridLines, sizeof(gridLines) / (3 * sizeof(float)));
   widget->gridColors = Buffer::createArrayBuffer(sizeof(gridColor), gridColor, sizeof(gridColor) / (4 * sizeof(float)));
}

GridWidget::~GridWidget()
{
   delete widget;
}

void GridWidget::draw(Device *device, Program *shader) const
{
   if (auto geometryShader = dynamic_cast<GeometryShader *>(shader))
   {
      geometryShader->setMatrixBlock(*this);

      // divisiones
      geometryShader->setLineThickness(1.0f);
      geometryShader->setVertexPoints(widget->gridCoords);
      geometryShader->setVertexColors(widget->gridColors);
      geometryShader->drawLines(widget->gridCoords.elements());

      // marco
      geometryShader->setLineThickness(2.0f);
      geometryShader->setVertexPoints(widget->borderCoords);
      geometryShader->setVertexColors(widget->borderColors);
      geometryShader->drawLineLoop(widget->borderCoords.elements());
   }
}

}
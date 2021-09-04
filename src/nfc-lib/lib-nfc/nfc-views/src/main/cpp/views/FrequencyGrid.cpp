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

#include <gl/engine/Text.h>

#include <gl/typeface/FreeType.h>

#include <nfc/DefaultShader.h>

#include <nfc/FrequencyGrid.h>

namespace nfc {

struct FrequencyGrid::Impl
{
   const gl::Color gridColor {0.75f, 0.75f, 0.75f, 1.0f};
   const gl::Color markColor {0.25f, 0.25f, 0.25f, 1.0f};

   gl::Buffer gridBuffer;

   gl::Text *viewCaptionLabel = nullptr;
};

FrequencyGrid::FrequencyGrid(int length) : self(std::make_shared<Impl>())
{
   self->gridBuffer = gl::Buffer::createArrayBuffer(8 * sizeof(gl::Vertex), nullptr, 8, sizeof(gl::Vertex));

   // create caption
   add(self->viewCaptionLabel = gl::FreeType::text("calibriz", 16, "NFC Frequency"));
}

void FrequencyGrid::setCenterFreq(long value)
{
}

void FrequencyGrid::setSampleRate(long value)
{
}

gl::Widget *FrequencyGrid::resize(int width, int height)
{
   gl::Widget::resize(width, height);

   // get widget bounds
   auto &rect = bounds();

   // model grid
   gl::Vertex grid[8] = {
         // outline loop
         {{rect.xmin + 0.005f, rect.ymin + 0.005f, 0.0f}, self->gridColor},
         {{rect.xmin + 0.005f, rect.ymax - 0.005f, 0.0f}, self->gridColor},
         {{rect.xmax - 0.005f, rect.ymax - 0.005f, 0.0f}, self->gridColor},
         {{rect.xmax - 0.005f, rect.ymin + 0.005f, 0.0f}, self->gridColor},
         // central line
         {{0.0f,               rect.ymin + 0.005f, 0.1f}, self->markColor},
         {{0.0f,               rect.ymax - 0.005f, 0.1f}, self->markColor},
   };

   self->gridBuffer.update(grid, 0, sizeof(grid));

   self->viewCaptionLabel->move(4, height - 20);

   return this;
}

void FrequencyGrid::draw(gl::Device *device, gl::Program *shader) const
{
   if (auto defaultShader = dynamic_cast<DefaultShader *>(shader))
   {
      defaultShader->setMatrixBlock(*this);
      defaultShader->setLineThickness(1.0f);

      // grid and marks
      defaultShader->setVertexPoints(self->gridBuffer, 3, 4 * sizeof(gl::Vertex) + offsetof(gl::Vertex, point));
      defaultShader->setVertexColors(self->gridBuffer, 4, 4 * sizeof(gl::Vertex) + offsetof(gl::Vertex, color));
      defaultShader->drawLines(2);

      // outline loop
      defaultShader->setVertexPoints(self->gridBuffer, 3, offsetof(gl::Vertex, point));
      defaultShader->setVertexColors(self->gridBuffer, 4, offsetof(gl::Vertex, color));
      defaultShader->drawLineLoop(4);
   }

   gl::Widget::draw(device, shader);
}

}

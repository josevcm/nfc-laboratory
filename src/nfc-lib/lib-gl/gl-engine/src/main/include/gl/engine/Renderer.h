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

#ifndef UI_GL_RENDERER_H
#define UI_GL_RENDERER_H

#include <vector>

#include <gl/engine/Device.h>
#include <gl/engine/Program.h>

namespace gl {

class Model;

class Renderer : public Device
{
      struct Impl;

   public:

      enum RenderState
      {
         NONE, PICK, DRAW
      };

   public:

      Renderer();

      virtual ~Renderer();

      Renderer *begin();

      Renderer *end();

      Renderer *draw(Model *model);

      Renderer *dispose();

      Renderer *addShader(Program *shader);

      Renderer *setEnableBlending(bool value);

      Renderer *setEnableCullFace(bool value);

      Renderer *setEnableDeepTest(bool value);

      Renderer *setEnableStencilTest(bool value);

      Renderer *setEnableScissorTest(bool value);

      Renderer *setEnableRasterizer(bool value);

      Renderer *setBlendEquation(int mode);

      Renderer *setBlendFunction(int sfactor, int dfactor);

      Renderer *setClearColor(float red, float green, float blue, float alpha = 1.0);

      Renderer *setViewport(int x, int y, int width, int height);

      Renderer *setScissor(int x, int y, int width, int height);

   private:

      Impl *self;
};

}
#endif //ANDROID_RADIO_RENDERER_H

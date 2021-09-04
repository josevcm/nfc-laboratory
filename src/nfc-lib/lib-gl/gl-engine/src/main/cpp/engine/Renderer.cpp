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

#include <opengl/GL.h>

#include <gl/engine/Model.h>
#include <gl/engine/Renderer.h>

namespace gl {

struct Renderer::Impl
{
   Renderer::RenderState renderState;

   std::vector<Program *> shaderList;

   Impl() : renderState(Renderer::NONE)
   {
   }

   ~Impl()
   {
      for (auto shader : shaderList)
      {
         delete shader;
      }
   }
};

Renderer::Renderer() : self(new Impl)
{
}


Renderer::~Renderer()
{
   delete self;
}

Renderer *Renderer::begin()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   self->renderState = DRAW;

   return this;
}

Renderer *Renderer::end()
{
   self->renderState = NONE;

   return this;
}

Renderer *Renderer::draw(Model *model)
{
   for (auto shader : self->shaderList)
   {
      shader->useProgram();
      model->draw(this, shader);
      shader->endProgram();
   }

   return this;
}

Renderer *Renderer::dispose()
{
   for (auto shader : self->shaderList)
   {
      delete shader;
   }

   self->shaderList.clear();

   return this;
}

Renderer *Renderer::addShader(Program *shader)
{
   self->shaderList.push_back(shader);

   return this;
}

Renderer *Renderer::setEnableBlending(bool value)
{
   if (value)
      glEnable(GL_BLEND);
   else
      glDisable(GL_BLEND);

   return this;
}

Renderer *Renderer::setEnableCullFace(bool value)
{
   if (value)
      glEnable(GL_CULL_FACE);
   else
      glDisable(GL_CULL_FACE);

   return this;
}

Renderer *Renderer::setEnableDeepTest(bool value)
{
   if (value)
      glEnable(GL_DEPTH_TEST);
   else
      glDisable(GL_DEPTH_TEST);

   return this;
}

Renderer *Renderer::setEnableStencilTest(bool value)
{
   if (value)
      glEnable(GL_STENCIL_TEST);
   else
      glDisable(GL_STENCIL_TEST);

   return this;
}

Renderer *Renderer::setEnableScissorTest(bool value)
{
   if (value)
      glEnable(GL_SCISSOR_TEST);
   else
      glDisable(GL_SCISSOR_TEST);

   return this;
}

Renderer *Renderer::setEnableRasterizer(bool value)
{
   if (value)
      glDisable(GL_RASTERIZER_DISCARD);
   else
      glEnable(GL_RASTERIZER_DISCARD);

   return this;
}

Renderer *Renderer::setBlendEquation(int mode)
{
   glBlendEquation(mode);

   return this;
}

Renderer *Renderer::setBlendFunction(int sfactor, int dfactor)
{
   glBlendFunc(sfactor, dfactor);

   return this;
}

Renderer *Renderer::setClearColor(float red, float green, float blue, float alpha)
{
   glClearColor(red, green, blue, alpha);

   return this;
}

Renderer *Renderer::setViewport(int x, int y, int width, int height)
{
   glViewport(x, y, width, height);

   return this;
}

Renderer *Renderer::setScissor(int x, int y, int width, int height)
{
   glScissor(x, y, width, height);

   return this;
}

}


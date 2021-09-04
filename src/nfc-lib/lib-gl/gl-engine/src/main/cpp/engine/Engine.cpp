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

#include <cmath>

#include <GL/glew.h>

#include <rt/Logger.h>

#include <gl/engine/Scene.h>
#include <gl/engine/Widget.h>
#include <gl/engine/Viewer.h>
#include <gl/engine/Metrics.h>
#include <gl/engine/Renderer.h>
#include <gl/engine/Engine.h>

namespace gl {

struct Engine::Impl
{
   rt::Logger log {"Engine"};
};

Engine::Engine() : impl(new Impl),
                   renderer(new Renderer()),
                   objects(new Scene()),
                   widgets(new Widget()),
                   camera(new Viewer()),
                   screen(new Viewer()),
                   metrics(new Metrics())
{
}

Engine::~Engine()
{
   delete impl;
   delete metrics;
   delete screen;
   delete widgets;
   delete camera;
   delete objects;
   delete renderer;
}

bool Engine::begin()
{
   int result;

   impl->log.info("GLEW version: {}", {(char *) glewGetString(GLEW_VERSION)});

   if ((result = glewInit()) != GLEW_OK)
      impl->log.error("GLEW initializacion error: {}", {(char *) glewGetErrorString(result)});

   return result == GLEW_OK;
}

void Engine::resize(int width, int height)
{
   // update metrics
   metrics->resize(width, height);

   // adjust the viewport based on geometry changes, such as screen rotation
   renderer->setViewport(0, 0, width, height);

   // ajustamos relacion de aspecto de camara
   camera->setCamera((float) (30.0f * M_PI / 360), metrics->aspect, 1.0f, 1000000.0f);

   // ajustamos relacion de aspecto de pantalla
   screen->setOrtho(-metrics->aspect, +metrics->aspect, -1, 1, 1, -1);
}

void Engine::update(float time, float delta)
{
   // update metrics
   metrics->update(time, delta);

   // update model
   objects->update(time, delta);
   widgets->update(time, delta);

   // compute model
   objects->compute(camera);
   widgets->compute(screen);

   // draw all
   renderer->begin();
   renderer->draw(objects);
   renderer->draw(widgets);
   renderer->end();

   // clear flags
   camera->clearDirty();
   screen->clearDirty();
}

void Engine::dispose()
{
   objects->dispose();
   widgets->dispose();
   renderer->dispose();
}

}
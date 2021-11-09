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

#include <QDebug>
#include <QElapsedTimer>
#include <QResizeEvent>
#include <QMutex>

#include <gl/engine/Engine.h>

#include <gl/shader/GeometryShader.h>
#include <gl/shader/TypeFaceShader.h>

#include <nfc/QuadratureView.h>
#include <nfc/QuadratureShader.h>
#include <nfc/DefaultShader.h>

#include <QtResources.h>

#include "QuadratureWidget.h"

struct QuadratureWidget::Impl : public gl::Engine
{
   // application resources
   QtResources *resources = nullptr;

   // signal quadrature view
   nfc::QuadratureView *quadratureView = nullptr;

   // frame clock
   QElapsedTimer frameTimer;

   // last frame time
   float lastFrame;

   Impl() : lastFrame(0), resources(new QtResources())
   {
   }

   bool begin() override
   {
      if (gl::Engine::begin())
      {
         renderer->setEnableCullFace(true);
         renderer->setEnableDeepTest(true);
         renderer->setEnableBlending(true);
         renderer->setBlendFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         renderer->setClearColor(0.098, 0.137, 0.176, 1.0);

         // setup render shaders
         renderer->addShader(new nfc::QuadratureShader(resources));
         renderer->addShader(new nfc::DefaultShader(resources));
         renderer->addShader(new gl::TypeFaceShader(resources));

         // create signal quality display
         widgets->add(quadratureView = new nfc::QuadratureView());

         // start frame timer
         frameTimer.start();

         return true;
      }

      return false;
   }

   void resize(int width, int height)
   {
      gl::Engine::resize(width, height);

      if (quadratureView)
         quadratureView->resize(width, height);
   }

   void paint()
   {
      float elapsed = (float) frameTimer.elapsed() / 1E3;

      gl::Engine::update(elapsed, lastFrame - elapsed);

      lastFrame = elapsed;
   }

   void refresh(const sdr::SignalBuffer &buffer) const
   {
      if (quadratureView)
         quadratureView->refresh(buffer);
   }
};

QuadratureWidget::QuadratureWidget(QWidget *parent) : QOpenGLWidget(parent), impl(new Impl())
{
}

void QuadratureWidget::refresh(const sdr::SignalBuffer &buffer)
{
   impl->refresh(buffer);
}

void QuadratureWidget::initializeGL()
{
   initializeOpenGLFunctions();

   qInfo().noquote() << "OpenGL vendor  :" << QString((const char *) glGetString(GL_VENDOR));
   qInfo().noquote() << "OpenGL version :" << QString((const char *) glGetString(GL_VERSION));
   qInfo().noquote() << "OpenGL renderer:" << QString((const char *) glGetString(GL_RENDERER));
   qInfo().noquote() << "OpenGL vGLSL   :" << QString((const char *) glGetString(GL_SHADING_LANGUAGE_VERSION));

   impl->begin();
}

void QuadratureWidget::setCenterFreq(long value)
{
}

void QuadratureWidget::setSampleRate(long value)
{
}

void QuadratureWidget::resizeGL(int w, int h)
{
   // force event size to optimize 2D font draw
   impl->resize(w & -2, h & -2);

   //

}

void QuadratureWidget::paintGL()
{
   impl->paint();

   update();
}

void QuadratureWidget::resizeEvent(QResizeEvent *event)
{
   if (width() != height())
   {
      setMinimumWidth(height());
   }

   QOpenGLWidget::resizeEvent(event);
}


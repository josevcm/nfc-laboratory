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

#include <QDebug>
#include <QMutex>
#include <QElapsedTimer>

#include <gl/engine/Engine.h>

#include <gl/shader/GeometryShader.h>
#include <gl/shader/TypeFaceShader.h>

#include <nfc/DefaultShader.h>
#include <nfc/SignalSmoother.h>
#include <nfc/EnvelopeShader.h>
#include <nfc/HeatmapShader.h>
#include <nfc/PeakShader.h>
#include <nfc/FrequencyView.h>

#include <QtResources.h>

#include "FrequencyWidget.h"

struct FrequencyWidget::Impl : public gl::Engine
{
   // application resources
   QtResources *resources = nullptr;

   // signal spectrum view
   nfc::FrequencyView *frequencyView = nullptr;

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
         renderer->addShader(new nfc::SignalSmoother(resources));
         renderer->addShader(new nfc::EnvelopeShader(resources));
         renderer->addShader(new nfc::HeatmapShader(resources));
         renderer->addShader(new nfc::PeakShader(resources));
         renderer->addShader(new nfc::DefaultShader(resources));
         renderer->addShader(new gl::TypeFaceShader(resources));

         // create frequency display
         widgets->add(frequencyView = new nfc::FrequencyView());

         // start frame timer
         frameTimer.start();

         return true;
      }

      return false;
   }

   void resize(int width, int height) override
   {
      gl::Engine::resize(width, height);

      frequencyView->resize(width, height);

      qDebug() << width << "x" << height;
   }

   void paint()
   {
      float elapsed = (float) (frameTimer.elapsed() / 1E3);

      gl::Engine::update(elapsed, lastFrame - elapsed);

      lastFrame = elapsed;
   }

   void refresh(const sdr::SignalBuffer &buffer) const
   {
      if (frequencyView)
      {
         frequencyView->refresh(buffer);
      }
   }

   void setCenterFreq(long value)
   {
      frequencyView->setCenterFreq(value);
   }

   void setSampleRate(long value)
   {
      frequencyView->setSampleRate(value);
   }
};

FrequencyWidget::FrequencyWidget(QWidget *parent) : QOpenGLWidget(parent), impl(new Impl())
{
}


void FrequencyWidget::setCenterFreq(long value)
{
   impl->setCenterFreq(value);
}

void FrequencyWidget::setSampleRate(long value)
{
   impl->setSampleRate(value);
}

void FrequencyWidget::refresh(const sdr::SignalBuffer &buffer)
{
   impl->refresh(buffer);
}

void FrequencyWidget::initializeGL()
{
   initializeOpenGLFunctions();

   qInfo().noquote() << "OpenGL vendor  :" << QString((const char *) glGetString(GL_VENDOR));
   qInfo().noquote() << "OpenGL version :" << QString((const char *) glGetString(GL_VERSION));
   qInfo().noquote() << "OpenGL renderer:" << QString((const char *) glGetString(GL_RENDERER));
   qInfo().noquote() << "OpenGL vGLSL   :" << QString((const char *) glGetString(GL_SHADING_LANGUAGE_VERSION));

   impl->begin();
}

void FrequencyWidget::resizeGL(int w, int h)
{
   // force event size to optimize 2D font draw
   impl->resize(w & -2, h & -2);
}

void FrequencyWidget::paintGL()
{
   impl->paint();

   update();
}

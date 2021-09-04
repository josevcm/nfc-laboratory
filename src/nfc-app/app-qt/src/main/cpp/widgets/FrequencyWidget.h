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

#ifndef APP_FREQUENCYWIDGET_H
#define APP_FREQUENCYWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>

#include <sdr/SignalBuffer.h>

class FrequencyWidget : public QOpenGLWidget, public QOpenGLExtraFunctions
{
   Q_OBJECT

      struct Impl;

   public:

      explicit FrequencyWidget(QWidget *parent = nullptr);

      void setCenterFreq(long value);

      void setSampleRate(long value);

      void refresh(const sdr::SignalBuffer &buffer);

   protected:

      void initializeGL() override;

      void resizeGL(int w, int h) override;

      void paintGL() override;

   private:

      Impl *impl;
};


#endif // NFC_LAB_FREQUENCYWIDGET_H

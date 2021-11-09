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

#ifndef NFC_LAB_SIGNALWIDGET_H
#define NFC_LAB_SIGNALWIDGET_H

#include <QWidget>

namespace sdr {
class SignalBuffer;
}

class SignalWidget : public QWidget
{
   Q_OBJECT

      struct Impl;

   public:

      explicit SignalWidget(QWidget *parent = nullptr);

      void setCenterFreq(long value);

      void setSampleRate(long value);

      void setRange(float lower, float upper);

      void setCenter(float value);

      void append(const sdr::SignalBuffer &buffer);

      void select(float from, float to);

      void refresh();

      void clear();

      float minimumRange() const;

      float maximumRange() const;

      float minimumScale() const;

      float maximumScale() const;

   protected:

      void enterEvent(QEvent *event) override;

      void leaveEvent(QEvent *event) override;

   public:

      Q_SIGNAL void rangeChanged(float from, float to);

      Q_SIGNAL void scaleChanged(float from, float to);

      Q_SIGNAL void selectionChanged(float from, float to);

   private:

      QSharedPointer<Impl> impl;

};


#endif //NFC_LAB_SIGNALWIDGET_H

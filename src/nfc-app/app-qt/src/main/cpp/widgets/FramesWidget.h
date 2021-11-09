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

#ifndef NFC_LAB_FRAMESWIDGET_H
#define NFC_LAB_FRAMESWIDGET_H

#include <QWidget>
#include <QSharedPointer>

namespace nfc {
class NfcFrame;
}

class FramesWidget : public QWidget
{
   Q_OBJECT

      struct Impl;

   public:

      explicit FramesWidget(QWidget *parent = nullptr);

      void append(const nfc::NfcFrame &frame);

      void select(double from, double to);

      void clear();

      void refresh();

   protected:

      void enterEvent(QEvent *event) override;
      void leaveEvent(QEvent *event) override;

   public:

      Q_SIGNAL void selectionChanged(float from, float to);

   private:

      QSharedPointer<Impl> impl;
};


#endif //NFC_LAB_FRAMESWIDGET_H

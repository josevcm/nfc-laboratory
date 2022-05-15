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

#include "QCPAxisCursorMarker.h"

struct QCPAxisCursorMarker::Impl
{
   QCPItemTracer *cursorTracer = nullptr;
   QCPItemText *cursorLabel = nullptr;
   QCPItemLine *cursorLine = nullptr;

   explicit Impl(QCPAxis *axis) : cursorTracer(new QCPItemTracer(axis->parentPlot())),
                                  cursorLine(new QCPItemLine(axis->parentPlot())),
                                  cursorLabel(new QCPItemText(axis->parentPlot()))
   {
      cursorTracer->setVisible(false);
      cursorTracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
      cursorTracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      cursorTracer->position->setAxisRect(axis->axisRect());
      cursorTracer->position->setAxes(axis, nullptr);
      cursorTracer->position->setCoords(0, 0);

      cursorLabel->setVisible(false);
      cursorLabel->setLayer("overlay");
      cursorLabel->setPen(QPen(Qt::gray));
      cursorLabel->setBrush(QBrush(Qt::white));
      cursorLabel->setClipToAxisRect(false);
      cursorLabel->setPadding(QMargins(2, 2, 4, 3));
      cursorLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
      cursorLabel->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      cursorLabel->position->setParentAnchorX(cursorTracer->position);
      cursorLabel->position->setCoords(0, 1);

      cursorLine->setVisible(false);
      cursorLine->setLayer("overlay");
      cursorLine->setPen(QPen(Qt::gray, 0, Qt::SolidLine));
      cursorLine->setClipToAxisRect(true);
      cursorLine->setHead(QCPLineEnding::esFlatArrow);
      cursorLine->setTail(QCPLineEnding::esBar);
      cursorLine->start->setTypeY(QCPItemPosition::ptAxisRectRatio);
      cursorLine->start->setParentAnchorX(cursorTracer->position);
      cursorLine->start->setCoords(0, 0);
      cursorLine->end->setTypeY(QCPItemPosition::ptAxisRectRatio);
      cursorLine->end->setParentAnchorX(cursorTracer->position);
      cursorLine->end->setCoords(0, 1);
   }

   ~Impl()
   {
      delete cursorLabel;
      delete cursorTracer;
      delete cursorLine;
   }

   void show() const
   {
      cursorLine->setVisible(true);
      cursorLabel->setVisible(true);
   }

   void hide() const
   {
      cursorLine->setVisible(false);
      cursorLabel->setVisible(false);
   }

   void setPosition(double value, const QString &text) const
   {
      cursorTracer->position->setCoords(value, 0);
      cursorLabel->setText(text);
   }
};

QCPAxisCursorMarker::QCPAxisCursorMarker(QCPAxis *axis) : impl(new Impl(axis))
{
}

void QCPAxisCursorMarker::setPosition(double value, const QString &text)
{
   impl->setPosition(value, text);
}

void QCPAxisCursorMarker::setVisible(bool visible)
{
   if (visible)
      impl->show();
   else
      impl->hide();
}


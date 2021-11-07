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

#include "CursorMarker.h"

CursorMarker::CursorMarker(QCPAxis *axis) : axis(axis)
      {
            setup();
      }

CursorMarker::~CursorMarker()
{
   delete label;
   delete tracer;
}

void CursorMarker::setup()
{
   tracer = new QCPItemTracer(axis->parentPlot());
   tracer->setVisible(false);
   tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
   tracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
   tracer->position->setAxisRect(axis->axisRect());
   tracer->position->setAxes(axis, nullptr);
   tracer->position->setCoords(0, 0);

   label = new QCPItemText(axis->parentPlot());
   label->setPen(QPen(Qt::darkGray));
   label->setBrush(QBrush(Qt::white));
   label->setLayer("overlay");
   label->setVisible(false);
   label->setClipToAxisRect(false);
   label->setPadding(QMargins(2, 1, 4, 3));
   label->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
   label->position->setParentAnchor(tracer->position);
}

void CursorMarker::show()
{
   label->setVisible(true);
}

void CursorMarker::hide()
{
   label->setVisible(false);
}

void CursorMarker::update(double from, const QString &text)
{
   label->setText(text);
   tracer->position->setCoords(from, 1);
}

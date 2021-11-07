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

#include "RangeMarker.h"

RangeMarker::RangeMarker(QCPAxis *axis) : axis(axis)
      {
            setup();
      }

RangeMarker::~RangeMarker()
{
   delete label;
   delete arrow;
   delete end;
   delete tracer;
   delete start;
}

void RangeMarker::setup()
{
   tracer = new QCPItemTracer(axis->parentPlot());
   tracer->setVisible(false);
   tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
   tracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
   tracer->position->setAxisRect(axis->axisRect());
   tracer->position->setAxes(axis, nullptr);
   tracer->position->setCoords(0, 1);

   start = new QCPItemTracer(axis->parentPlot());
   start->setVisible(false);
   start->setPen(QPen(Qt::white));
   start->position->setTypeX(QCPItemPosition::ptPlotCoords);
   start->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
   start->position->setAxisRect(axis->axisRect());
   start->position->setAxes(axis, nullptr);
   start->position->setCoords(0, 1);

   end = new QCPItemTracer(axis->parentPlot());
   end->setVisible(false);
   end->setPen(QPen(Qt::white));
   end->position->setTypeX(QCPItemPosition::ptPlotCoords);
   end->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
   end->position->setAxisRect(axis->axisRect());
   end->position->setAxes(axis, nullptr);
   end->position->setCoords(0, 1);

   arrow = new QCPItemLine(axis->parentPlot());
   arrow->setPen(QPen(Qt::gray));
   arrow->setLayer("overlay");
   arrow->setClipToAxisRect(false);
   arrow->setHead(QCPLineEnding::esSpikeArrow);
   arrow->setTail(QCPLineEnding::esSpikeArrow);
   arrow->start->setParentAnchor(start->position);
   arrow->end->setParentAnchor(end->position);

   label = new QCPItemText(axis->parentPlot());
   label->setPen(QPen(Qt::gray));
   label->setBrush(QBrush(Qt::white));
   label->setLayer("overlay");
   label->setVisible(false);
   label->setClipToAxisRect(false);
   label->setPadding(QMargins(5, 0, 4, 2));
   label->setPositionAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
   label->position->setParentAnchor(tracer->position);
}

void RangeMarker::show(double from, double to, const QString &text)
{
   label->setText(text);
   tracer->position->setCoords((from + to) / 2, 0);
   start->position->setCoords(from, 0);
   end->position->setCoords(to, 0);

   label->setVisible(true);
   arrow->setVisible(true);
   start->setVisible(true);
   end->setVisible(true);
}

void RangeMarker::hide()
{
   label->setVisible(false);
   arrow->setVisible(false);
   start->setVisible(false);
   end->setVisible(false);
}

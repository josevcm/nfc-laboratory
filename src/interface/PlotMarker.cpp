/*

  Copyright (c) 2020 Jose Vicente Campos Martinez - <josevcm@gmail.com>

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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include "PlotMarker.h"

PlotMarker::PlotMarker(QCPAxis *parentAxis) :
   QObject(parentAxis), mAxis(parentAxis)
{
   mStart = new QCPItemTracer(mAxis->parentPlot());
   mStart->setVisible(false);
   mStart->position->setTypeX(QCPItemPosition::ptPlotCoords);
   mStart->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
   mStart->position->setAxisRect(mAxis->axisRect());
   mStart->position->setAxes(mAxis, nullptr);
   mStart->position->setCoords(0, 1);
   mStart->setPen(QPen(Qt::white));

   mMiddle = new QCPItemTracer(mAxis->parentPlot());
   mMiddle->setVisible(false);
   mMiddle->position->setTypeX(QCPItemPosition::ptPlotCoords);
   mMiddle->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
   mMiddle->position->setAxisRect(mAxis->axisRect());
   mMiddle->position->setAxes(mAxis, nullptr);
   mMiddle->position->setCoords(0, 1);

   mEnd = new QCPItemTracer(mAxis->parentPlot());
   mEnd->setVisible(false);
   mEnd->position->setTypeX(QCPItemPosition::ptPlotCoords);
   mEnd->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
   mEnd->position->setAxisRect(mAxis->axisRect());
   mEnd->position->setAxes(mAxis, nullptr);
   mEnd->position->setCoords(0, 1);
   mEnd->setPen(QPen(Qt::white));

   mArrow = new QCPItemLine(mAxis->parentPlot());
   mArrow->setLayer("overlay");
   mArrow->setClipToAxisRect(false);
   mArrow->setHead(QCPLineEnding::esSpikeArrow);
   mArrow->start->setParentAnchor(mStart->position);
   mArrow->end->setParentAnchor(mEnd->position);

   mLabel = new QCPItemText(mAxis->parentPlot());
   mLabel->setLayer("overlay");
   mLabel->setVisible(false);
   mLabel->setClipToAxisRect(false);
   mLabel->setPadding(QMargins(3, 0, 4, 2));
   mLabel->setBrush(QBrush(Qt::white));
   mLabel->setPen(QPen(Qt::white));
   mLabel->setPositionAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
   mLabel->position->setParentAnchor(mMiddle->position);
}

PlotMarker::~PlotMarker()
{
   delete mLabel;
   delete mArrow;
   delete mEnd;
   delete mMiddle;
   delete mStart;
}

QPen PlotMarker::pen() const
{
   return mLabel->pen();
}

void PlotMarker::setPen(const QPen &pen)
{
   mArrow->setPen(pen);
   mLabel->setPen(pen);
}

QBrush PlotMarker::brush() const
{
   return mLabel->brush();
}

void PlotMarker::setBrush(const QBrush &brush)
{
   mLabel->setBrush(brush);
}

QString PlotMarker::text() const
{
   return mLabel->text();
}

void PlotMarker::setText(const QString &text)
{
   mLabel->setText(text);
   mLabel->setVisible(!text.isEmpty());
}

void PlotMarker::setRange(double start, double end)
{
   mStart->position->setCoords(start, 0);
   mMiddle->position->setCoords((start + end) / 2, 0);
   mEnd->position->setCoords(end, 0);

   mStart->setVisible(start > 0);
   mEnd->setVisible(end > 0);
}

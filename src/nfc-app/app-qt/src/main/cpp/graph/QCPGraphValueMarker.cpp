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

#include "QCPGraphValueMarker.h"

QCPGraphValueMarker::QCPGraphValueMarker(QCPGraph *graph, const QColor &color) : tracer(new QCPItemTracer(graph->parentPlot())), label(new QCPItemText(graph->parentPlot()))
{
   tracer->setVisible(false);
   tracer->setGraph(graph);
   tracer->setGraphKey(0);
   tracer->setInterpolating(true);
   tracer->setStyle(QCPItemTracer::tsSquare);
   tracer->setPen(QPen(color, 2.5f));
   tracer->setSize(10);
   tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
   tracer->position->setTypeY(QCPItemPosition::ptPlotCoords);

   label->setVisible(false);
   label->setColor(Qt::white);
   label->setLayer("overlay");
   label->setClipToAxisRect(false);
   label->setPadding(QMargins(0, 0, 0, 15));
   label->setPositionAlignment(Qt::AlignBottom | Qt::AlignHCenter);
   label->position->setParentAnchor(tracer->position);
}

QCPGraphValueMarker::~QCPGraphValueMarker()
{
   delete label;
   delete tracer;
}

void QCPGraphValueMarker::show()
{
   label->setVisible(true);
   tracer->setVisible(true);
}

void QCPGraphValueMarker::hide()
{
   label->setVisible(false);
   tracer->setVisible(false);
}

void QCPGraphValueMarker::update(double key, const QString &text)
{
   label->setText(text);
   tracer->setGraphKey(key);
}
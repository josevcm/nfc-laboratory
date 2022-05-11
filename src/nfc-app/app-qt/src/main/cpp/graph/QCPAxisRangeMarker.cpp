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

#include <QString>

#include "QCPAxisRangeMarker.h"

struct QCPAxisRangeMarker::Impl
{
   int deep = 0;
   double scale = 0.10;

   QCPItemTracer *labelTracer = nullptr;
   QCPItemTracer *startTracer = nullptr;
   QCPItemTracer *endTracer = nullptr;

   QCPItemText *rangeLabel = nullptr;
   QCPItemLine *arrowLine = nullptr;
   QCPItemLine *startLine = nullptr;
   QCPItemLine *endLine = nullptr;

   QColor selectColor {0, 200, 255, 255};
   QColor selectText {255, 255, 0, 255};
   QFont selectFont;

   explicit Impl(QCPAxis *axis) : labelTracer(new QCPItemTracer(axis->parentPlot())),
                                  startTracer(new QCPItemTracer(axis->parentPlot())),
                                  endTracer(new QCPItemTracer(axis->parentPlot())),
                                  arrowLine(new QCPItemLine(axis->parentPlot())),
                                  startLine(new QCPItemLine(axis->parentPlot())),
                                  endLine(new QCPItemLine(axis->parentPlot())),
                                  rangeLabel(new QCPItemText(axis->parentPlot()))
   {
      selectFont.setBold(true);

      labelTracer->setVisible(false);
      labelTracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
      labelTracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      labelTracer->position->setAxisRect(axis->axisRect());
      labelTracer->position->setAxes(axis, nullptr);
      labelTracer->position->setCoords(0, 0);

      startTracer->setVisible(false);
      startTracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
      startTracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      startTracer->position->setAxisRect(axis->axisRect());
      startTracer->position->setAxes(axis, nullptr);
      startTracer->position->setCoords(0, 0);

      endTracer->setVisible(false);
      endTracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
      endTracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      endTracer->position->setAxisRect(axis->axisRect());
      endTracer->position->setAxes(axis, nullptr);
      endTracer->position->setCoords(0, 0);

      arrowLine->setVisible(false);
      arrowLine->setLayer("overlay");
      arrowLine->setPen(QPen(Qt::gray, 0, Qt::SolidLine));
      arrowLine->setClipToAxisRect(false);
      arrowLine->setHead(QCPLineEnding::esSpikeArrow);
      arrowLine->setTail(QCPLineEnding::esSpikeArrow);
      arrowLine->setSelectable(true);
      arrowLine->setSelectedPen(QPen(selectColor));
      arrowLine->start->setTypeY(QCPItemPosition::ptAxisRectRatio);
      arrowLine->start->setParentAnchorX(startTracer->position);
      arrowLine->start->setCoords(0, 0);
      arrowLine->end->setTypeY(QCPItemPosition::ptAxisRectRatio);
      arrowLine->end->setParentAnchorX(endTracer->position);
      arrowLine->end->setCoords(0, 0);

      startLine->setVisible(false);
      startLine->setLayer("overlay");
      startLine->setPen(QPen(Qt::gray, 0, Qt::DashLine));
      startLine->setClipToAxisRect(true);
      startLine->setHead(QCPLineEnding::esFlatArrow);
      startLine->setSelectable(true);
      startLine->setSelectedPen(QPen(selectColor));
      startLine->start->setTypeY(QCPItemPosition::ptAxisRectRatio);
      startLine->start->setParentAnchorX(startTracer->position);
      startLine->start->setCoords(0, 0);
      startLine->end->setTypeY(QCPItemPosition::ptAxisRectRatio);
      startLine->end->setParentAnchorX(startTracer->position);
      startLine->end->setCoords(0, 1);

      endLine->setVisible(false);
      endLine->setLayer("overlay");
      endLine->setPen(QPen(Qt::gray, 0, Qt::DashLine));
      endLine->setClipToAxisRect(true);
      endLine->setHead(QCPLineEnding::esFlatArrow);
      endLine->setSelectable(true);
      endLine->setSelectedPen(QPen(selectColor));
      endLine->start->setTypeY(QCPItemPosition::ptAxisRectRatio);
      endLine->start->setParentAnchorX(endTracer->position);
      endLine->start->setCoords(0, 0);
      endLine->end->setTypeY(QCPItemPosition::ptAxisRectRatio);
      endLine->end->setParentAnchorX(endTracer->position);
      endLine->end->setCoords(0, 1);

      rangeLabel->setVisible(false);
      rangeLabel->setLayer("overlay");
      rangeLabel->setPen(QPen(Qt::gray));
      rangeLabel->setBrush(QBrush(Qt::white));
      rangeLabel->setClipToAxisRect(false);
      rangeLabel->setPadding(QMargins(5, 0, 4, 2));
      rangeLabel->setPositionAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
      rangeLabel->setSelectable(true);
      rangeLabel->setSelectedFont(selectFont);
      rangeLabel->setSelectedColor(selectText);
      rangeLabel->setSelectedBrush(QBrush(selectColor));
      rangeLabel->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      rangeLabel->position->setParentAnchorX(labelTracer->position);
      rangeLabel->position->setCoords(0, 0);

      QObject::connect(rangeLabel, &QCPAbstractItem::selectionChanged, [=](bool selected) {
         arrowLine->setSelected(selected);
      });

      QObject::connect(arrowLine, &QCPAbstractItem::selectionChanged, [=](bool selected) {
         rangeLabel->setSelected(selected);
      });
   }

   ~Impl()
   {
      delete startTracer;
      delete endTracer;
      delete labelTracer;
      delete rangeLabel;
      delete arrowLine;
      delete startLine;
      delete endLine;
   }

   void show() const
   {
      rangeLabel->setVisible(true);
      arrowLine->setVisible(true);
      startLine->setVisible(true);
      endLine->setVisible(true);
   }

   void hide() const
   {
      rangeLabel->setVisible(false);
      arrowLine->setVisible(false);
      startLine->setVisible(false);
      endLine->setVisible(false);
   }

   void select(bool selected) const
   {
      arrowLine->setSelected(selected);
      rangeLabel->setSelected(selected);
   }

   void setPositionStart(double value) const
   {
      // update marker startTracer
      startTracer->position->setCoords(value, 0);

      // update rangeLabel
      range();
   }

   void setPositionEnd(double value) const
   {
      // update marker endTracer
      endTracer->position->setCoords(value, 0);

      // update rangeLabel
      range();
   }

   void setDeep(int deep)
   {
      this->deep = deep;
      this->scale = 0.10;

      rangeLabel->position->setCoords(0, deep * this->scale);
      startLine->start->setCoords(0, deep * this->scale);
      endLine->start->setCoords(0, deep * this->scale);
      arrowLine->start->setCoords(0, deep * this->scale);
      arrowLine->end->setCoords(0, deep * this->scale);
   }

   void range() const
   {
      // update rangeLabel position
      labelTracer->position->setCoords((startTracer->position->key() + endTracer->position->key()) / 2, 0);

      // get range time
      double value = fabs(endTracer->position->key() - startTracer->position->key());

      QString text;

      if (value < 1E-6)
         text = QString("%1 ns").arg(value * 1E9, 3, 'f', 0);
      if (value < 1E-3)
         text = QString("%1 us").arg(value * 1E6, 3, 'f', 0);
      else if (value < 1)
         text = QString("%1 ms").arg(value * 1E3, 7, 'f', 3);
      else if (value < 1E3)
         text = QString("%1 s").arg(value, 7, 'f', 5);
      else if (value < 1E6)
         text = QString("%1 Ks").arg(value / 1E3, 7, 'f', 5);
      else if (value < 1E9)
         text = QString("%1 Ms").arg(value / 1E6, 7, 'f', 5);
      else if (value < 1E12)
         text = QString("%1 Gs").arg(value / 1E12, 7, 'f', 5);

      rangeLabel->setText(text);
   }
};

QCPAxisRangeMarker::QCPAxisRangeMarker(QCPAxis *axis) : impl(new Impl(axis))
{
}

double QCPAxisRangeMarker::positionStart() const
{
   return impl->startTracer->position->key();
}


void QCPAxisRangeMarker::setPositionStart(double value)
{
   impl->setPositionStart(value);
}

double QCPAxisRangeMarker::positionEnd() const
{
   return impl->endTracer->position->key();
}

void QCPAxisRangeMarker::setPositionEnd(double value)
{
   impl->setPositionEnd(value);
}

bool QCPAxisRangeMarker::visible() const
{
   return impl->rangeLabel->visible();
}

void QCPAxisRangeMarker::setVisible(bool visible)
{
   if (visible)
      impl->show();
   else
      impl->hide();
}

bool QCPAxisRangeMarker::selected() const
{
   return impl->rangeLabel->selected();
}

void QCPAxisRangeMarker::setSelected(bool selected)
{
   impl->select(selected);
}

int QCPAxisRangeMarker::deep() const
{
   return impl->deep;
}

void QCPAxisRangeMarker::setDeep(int deep)
{
   impl->setDeep(deep);
}

double QCPAxisRangeMarker::width() const
{
   return fabs(impl->endTracer->position->key() - impl->startTracer->position->key());
}

/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2024 Jose Vicente Campos Martinez, <josevcm@gmail.com>

  NFC-LABORATORY is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NFC-LABORATORY is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NFC-LABORATORY. If not, see <http://www.gnu.org/licenses/>.

*/

#include <styles/Theme.h>

#include "MarkerCursor.h"

struct MarkerCursor::Impl
{
   QCustomPlot *plot;
   QCPItemTracer *cursorTracer;
   QCPItemTracer *startTracer;
   QCPItemTracer *endTracer;
   QCPItemText *cursorLabel;
   QCPItemLine *cursorLine;

   std::function<QString(double)> formatter;

   explicit Impl(QCustomPlot *plot) : plot(plot),
                                      cursorTracer(new QCPItemTracer(plot)),
                                      startTracer(new QCPItemTracer(plot)),
                                      endTracer(new QCPItemTracer(plot)),
                                      cursorLine(new QCPItemLine(plot)),
                                      cursorLabel(new QCPItemText(plot)),
                                      formatter([](double value) { return QString::number(value); })
   {
      cursorTracer->setVisible(false);
      cursorTracer->setSelectable(false);
      cursorTracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
      cursorTracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      cursorTracer->position->setAxisRect(plot->xAxis->axisRect());
      cursorTracer->position->setAxes(plot->xAxis, nullptr);
      cursorTracer->position->setCoords(0, 0);

      startTracer->setVisible(false);
      startTracer->setSelectable(false);
      startTracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      startTracer->position->setAxisRect(plot->xAxis->axisRect());
      startTracer->position->setAxes(plot->xAxis, nullptr);
      startTracer->position->setParentAnchorX(cursorTracer->position);
      startTracer->position->setCoords(0, 0);

      endTracer->setVisible(false);
      endTracer->setSelectable(false);
      endTracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      endTracer->position->setAxisRect(plot->xAxis->axisRect());
      endTracer->position->setAxes(plot->xAxis, nullptr);
      endTracer->position->setParentAnchorX(cursorTracer->position);
      endTracer->position->setCoords(0, 1);

      cursorLine->setVisible(false);
      cursorLine->setSelectable(false);
      cursorLine->setLayer("overlay");
      cursorLine->setPen(QPen(Qt::darkGray, 0, Qt::DashDotLine));
      cursorLine->setClipToAxisRect(true);
      cursorLine->setHead(QCPLineEnding::esSpikeArrow);
      cursorLine->start->setParentAnchor(startTracer->position);
      cursorLine->end->setParentAnchor(endTracer->position);
      cursorLine->end->setCoords(0, -2);

      cursorLabel->setVisible(false);
      cursorLabel->setSelectable(false);
      cursorLabel->setLayer("overlay");
      cursorLabel->setPen(QPen(Qt::transparent));
      cursorLabel->setBrush(QBrush(Qt::white));
      cursorLabel->setClipToAxisRect(false);
      cursorLabel->setPadding(QMargins(4, 0, 4, 2));
      cursorLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
      cursorLabel->position->setParentAnchor(endTracer->position);
      cursorLabel->position->setCoords(0, -1);
   }

   ~Impl()
   {
      plot->removeItem(cursorLabel);
      plot->removeItem(cursorLine);
      plot->removeItem(startTracer);
      plot->removeItem(endTracer);
      plot->removeItem(cursorTracer);
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

   void setPosition(double value) const
   {
      cursorTracer->position->setCoords(value, 0);
      cursorLabel->setText(formatter(value));
   }
};

MarkerCursor::MarkerCursor(QCustomPlot *plot) : impl(new Impl(plot))
{
}

double MarkerCursor::position() const
{
   return impl->cursorTracer->position->key();
}

void MarkerCursor::setPosition(double value)
{
   impl->setPosition(value);
}

bool MarkerCursor::visible() const
{
   return impl->cursorLine->visible();
}

void MarkerCursor::setVisible(bool visible)
{
   if (visible)
      impl->show();
   else
      impl->hide();
}

void MarkerCursor::setFormatter(const std::function<QString(double)> &formatter)
{
   impl->formatter = formatter;
}





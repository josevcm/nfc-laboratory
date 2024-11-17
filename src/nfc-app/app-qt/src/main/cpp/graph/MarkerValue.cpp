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

#include "MarkerValue.h"

struct MarkerValue::Impl
{
   QCustomPlot *plot;
   QCPItemTracer *tracer = nullptr;
   QCPItemText *label = nullptr;

   explicit Impl(QCPGraph *graph) : plot(graph->parentPlot()),
                                    tracer(new QCPItemTracer(plot)),
                                    label(new QCPItemText(plot))
   {
      tracer->setVisible(false);
      tracer->setGraph(graph);
      tracer->setGraphKey(0);
      tracer->setInterpolating(true);
      tracer->setStyle(QCPItemTracer::tsSquare);
      tracer->setPen(Theme::defaultMarkerPen);
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

   ~Impl()
   {
      plot->removeItem(label);
      plot->removeItem(tracer);
   }

   void hide()
   {
      label->setVisible(false);
      tracer->setVisible(false);
   }

   void show()
   {
      label->setVisible(true);
      tracer->setVisible(true);
   }

   void setPosition(double value, const QString &text)
   {
      label->setText(text);
      tracer->setGraphKey(value);
   }
};

MarkerValue::MarkerValue(QCPGraph *graph) : impl(new Impl(graph))
{
}

void MarkerValue::setPosition(double value, const QString &text)
{
   impl->setPosition(value, text);
}

void MarkerValue::setVisible(bool visible)
{
   if (visible)
      impl->show();
   else
      impl->hide();
}
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

#include <format/DataFormat.h>

#include "MarkerZoom.h"

struct MarkerZoom::Impl
{
   static const QColor defaultLabelColor;
   static const QPen defaultLabelPen;
   static const QBrush defaultLabelBrush;
   static const QFont defaultLabelFont;

   QCustomPlot *plot;
   QCPItemTracer *tracer = nullptr;
   QCPItemText *label = nullptr;
   QCPRange total {0, 0};

   QMetaObject::Connection rangeChangedConnection;

   explicit Impl(QCustomPlot *plot) : plot(plot),
                             tracer(new QCPItemTracer(plot)),
                             label(new QCPItemText(plot))
   {
      tracer->setVisible(false);
      tracer->setSelectable(false);
      tracer->position->setTypeX(QCPItemPosition::ptAxisRectRatio);
      tracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      tracer->position->setAxisRect(plot->xAxis->axisRect());
      tracer->position->setCoords(1, 0);

      label->setVisible(true);
      label->setSelectable(false);
      label->setText("");
      label->setColor(Qt::white);
      label->setLayer(plot->legend->layer());
      label->setClipToAxisRect(false);
      label->setFont(defaultLabelFont);
      label->setColor(defaultLabelColor);
      label->setPen(defaultLabelPen);
      label->setBrush(defaultLabelBrush);
      label->setPadding(QMargins(0, 0, 0, 0));
      label->setPositionAlignment(Qt::AlignRight | Qt::AlignTop);
      label->position->setParentAnchor(tracer->position);
      label->position->setCoords(0, 0);

      rangeChangedConnection = QObject::connect(plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &newRange) {
         rangeChanged(newRange);
      });
   }

   ~Impl()
   {
      QObject::disconnect(rangeChangedConnection);

      plot->removeItem(label);
      plot->removeItem(tracer);
   }

   void rangeChanged(const QCPRange &newRange)
   {
      QCPRange range = plot->xAxis->range();

      if (total.size() > 0)
         label->setText(QObject::tr("Zoom:%1").arg(DataFormat::percentage(total.size() / range.size())));
      else
         label->setText(QObject::tr("N/A"));
   }

   void hide()
   {
      label->setVisible(false);
   }

   void show()
   {
      label->setVisible(true);
   }
};

const QColor MarkerZoom::Impl::defaultLabelColor({0xF0, 0xF0, 0xF0, 0xFF});
const QPen MarkerZoom::Impl::defaultLabelPen({0x2B, 0x2B, 0x2B, 0x70});
const QBrush MarkerZoom::Impl::defaultLabelBrush(Qt::transparent);
const QFont MarkerZoom::Impl::defaultLabelFont("Roboto", 14, QFont::Normal);

MarkerZoom::MarkerZoom(QCustomPlot *plot) : impl(new Impl(plot))
{
}

void MarkerZoom::setTotalRange(double lower, double upper)
{
   impl->total.lower = lower;
   impl->total.upper = upper;
}

void MarkerZoom::setVisible(bool visible)
{
   if (visible)
      impl->show();
   else
      impl->hide();
}
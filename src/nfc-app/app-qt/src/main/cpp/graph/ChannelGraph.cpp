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

#include "ChannelGraph.h"

struct ChannelGraph::Impl
{
   ChannelGraph *graph;

   double offset;

   ChannelStyle style;

   explicit Impl(ChannelGraph *graph) : graph(graph), offset(0)
   {
   }

   void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
   {
      painter->save();

      QRectF bounds(rect.left(), rect.top(), rect.width(), rect.height() - 2);

      QPolygonF polygon {
            QPointF(bounds.left(), bounds.center().y()),
            QPointF(bounds.left() + 5, bounds.top()),
            QPointF(bounds.right() - 5, bounds.top()),
            QPointF(bounds.right(), bounds.center().y()),
            QPointF(bounds.right() - 5, bounds.bottom()),
            QPointF(bounds.left() + 5, bounds.bottom())
      };

      // draw shape
      painter->setPen(style.shapePen);
      painter->setBrush(style.shapeBrush);
      painter->drawPolygon(polygon);

      // draw text
      painter->setPen(style.labelPen);
      painter->setFont(style.labelFont);
      painter->drawText(bounds.adjusted(5, 0, -5, 0), Qt::AlignCenter, style.text);

      painter->restore();
   }
};

ChannelGraph::ChannelGraph(QCPAxis *keyAxis, QCPAxis *valueAxis) : QCPGraph(keyAxis, valueAxis), impl(new Impl(this))
{
}

void ChannelGraph::setStyle(const ChannelStyle &style)
{
   impl->style = style;
}

const ChannelStyle &ChannelGraph::style()
{
   return impl->style;
}

void ChannelGraph::setOffset(double offset)
{
   impl->offset = offset;
}

double ChannelGraph::offset()
{
   return impl->offset;
}

void ChannelGraph::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{
   impl->drawLegendIcon(painter, rect);
}

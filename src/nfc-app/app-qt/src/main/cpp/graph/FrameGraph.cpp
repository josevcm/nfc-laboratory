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

#include "FrameGraph.h"

bool qIsNaN(const QByteArray &value)
{
   return value.isNull();
}

struct DigitalLegend
{
   int key;
   int style;
   QString text;
   // QRectF bounds;
   // ChannelStyle style;
};

struct FrameGraph::Impl
{
   FrameGraph *graph;

   double offset;

   QSharedPointer<QCPDigitalDataContainer> dataContainer;

   QMap<int, ChannelStyle> styleMap;

   QMap<int, DigitalLegend> legendMap;

   StyleMapper styleMapper;

   ValueMapper valueMapper;

   QMetaObject::Connection legendClickConnection;

   explicit Impl(FrameGraph *graph) : graph(graph), offset(0), dataContainer(new QCPDigitalDataContainer)
   {
      legendClickConnection = connect(graph->parentPlot(), &QCustomPlot::legendClick, [=](QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event) {
         legendClick(legend, item, event);
      });
   }

   ~Impl()
   {
      disconnect(legendClickConnection);
   }

   void legendClick(QCPLegend *legend, QCPAbstractLegendItem *legendItem, QMouseEvent *event) const
   {
      QCPPlottableLegendItem *legendGraph = legend->itemWithPlottable(graph);

      if (legendItem != legendGraph)
         return;

      QRectF legendBounds = legendItem->rect();

      ChannelStyle defaultStyle {graph->mPen, graph->mPen, graph->mBrush, graph->mPen, graph->mParentPlot->font()};

      for (auto it = legendMap.constBegin(); it != legendMap.constEnd(); ++it)
      {
         ChannelStyle style = styleMapper ? styleMapper(it->style) : styleMap.value(it->style, defaultStyle);

         const QRectF bounds = getLegendRect(it->text, style);

         legendBounds.setWidth(bounds.width());
         legendBounds.setHeight(bounds.height());

         if (legendBounds.contains(event->pos()))
         {
            graph->legendClicked(it->key);
            return;
         }

         legendBounds = legendBounds.adjusted(bounds.width() + 5, 0, 0, 0);
      }
   }

   double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
   {
      if ((onlySelectable && graph->mSelectable == QCP::stNone) || dataContainer->isEmpty())
         return -1;

      if (!graph->mKeyAxis || !graph->mValueAxis)
         return -1;

      if (graph->mKeyAxis.data()->axisRect()->rect().contains(pos.toPoint()) || graph->mParentPlot->interactions().testFlag(QCP::iSelectPlottablesBeyondAxisRect))
      {
         // get visible data range:
         QCPDigitalDataContainer::const_iterator visibleBegin, visibleEnd;

         getVisibleDataBounds(visibleBegin, visibleEnd);

         for (QCPDigitalDataContainer::const_iterator it = visibleBegin; it != visibleEnd; ++it)
         {
            if (getFrameRect(*it).contains(pos))
            {
               if (details)
               {
                  int pointIndex = int(it - dataContainer->constBegin());

                  details->setValue(QCPDataSelection(QCPDataRange(pointIndex, pointIndex + 1)));
               }

               return graph->mParentPlot->selectionTolerance() * 0.99;
            }
         }
      }

      return -1;
   }

   void getDataSegments(QList<QCPDataRange> &selectedSegments, QList<QCPDataRange> &unselectedSegments) const
   {
      selectedSegments.clear();
      unselectedSegments.clear();

      if (graph->mSelectable == QCP::stWhole) // stWhole selection type draws the entire plottable with selected style if mSelection isn't empty
      {
         if (graph->selected())
            selectedSegments << QCPDataRange(0, graph->dataCount());
         else
            unselectedSegments << QCPDataRange(0, graph->dataCount());
      }
      else
      {
         QCPDataSelection sel(graph->selection());
         sel.simplify();
         selectedSegments = sel.dataRanges();
         unselectedSegments = sel.inverse(QCPDataRange(0, graph->dataCount())).dataRanges();
      }
   }

   void getVisibleDataBounds(QCPDigitalDataContainer::const_iterator &begin, QCPDigitalDataContainer::const_iterator &end) const
   {
      if (!graph->mKeyAxis)
      {
         begin = dataContainer->constEnd();
         end = dataContainer->constEnd();
         return;
      }

      if (dataContainer->isEmpty())
      {
         begin = dataContainer->constEnd();
         end = dataContainer->constEnd();
         return;
      }

      // get visible data range as QMap iterators
      begin = dataContainer->findBegin(graph->mKeyAxis.data()->range().lower);
      end = dataContainer->findEnd(graph->mKeyAxis.data()->range().upper, true);

      double lowerPixelBound = graph->mKeyAxis.data()->coordToPixel(graph->mKeyAxis.data()->range().lower);
      double upperPixelBound = graph->mKeyAxis.data()->coordToPixel(graph->mKeyAxis.data()->range().upper);

      bool isVisible;

      // walk left from begin to find lower bar that actually is completely outside visible pixel range:
      for (QCPDigitalDataContainer::const_iterator it = begin; it != dataContainer->constBegin(); --it)
      {
         const QRectF digitalRect = getFrameRect(*it);

         if (!graph->mKeyAxis.data()->rangeReversed())
            isVisible = digitalRect.right() >= lowerPixelBound;
         else
            isVisible = digitalRect.left() <= lowerPixelBound;

         if (!isVisible)
            break;

         begin = it;
      }

      for (QCPDigitalDataContainer::const_iterator it = end; it != dataContainer->constEnd(); ++it)
      {
         const QRectF digitalRect = getFrameRect(*it);

         if (!graph->mKeyAxis.data()->rangeReversed())
            isVisible = digitalRect.left() <= upperPixelBound;
         else
            isVisible = digitalRect.right() >= upperPixelBound;

         if (!isVisible)
            break;

         end = it;
      }
   }

   QRectF getFrameRect(const FrameData &data) const
   {
      QCPAxis *keyAxis = graph->mKeyAxis.data();
      QCPAxis *valueAxis = graph->mValueAxis.data();

      if (!keyAxis || !valueAxis)
         return {};

      double position = valueAxis->coordToPixel(offset);
      double left = keyAxis->coordToPixel(data.start);
      double right = keyAxis->coordToPixel(data.end);
      double top = position - data.height / 2;
      double bottom = position + data.height / 2;

      return {QPointF(left, top), QPointF(right, bottom)};
   }

   QRectF getLegendRect(const QString &text, const ChannelStyle &style) const
   {
      QFontMetrics fontMetrics(style.labelFont);

      QSize fontSize = fontMetrics.size(0, text);

      return {0, 0, static_cast<qreal>(fontSize.width() + 20), static_cast<qreal>(fontSize.height() + 2)};
   }

   void draw(QCPPainter *painter) const
   {
      QCPDigitalDataContainer::const_iterator visibleBegin, visibleEnd;
      getVisibleDataBounds(visibleBegin, visibleEnd);

      // loop over and draw segments of unselected/selected data:
      QList<QCPDataRange> selectedSegments, unselectedSegments, allSegments;
      getDataSegments(selectedSegments, unselectedSegments);

      allSegments << unselectedSegments << selectedSegments;

      ChannelStyle defaultStyle {graph->mPen, graph->mPen, graph->mBrush, graph->mPen, graph->mParentPlot->font()};

      for (int i = 0; i < allSegments.size(); i++)
      {
         const QCPDataRange &segment = allSegments.at(i);

         QCPDigitalDataContainer::const_iterator begin = visibleBegin;
         QCPDigitalDataContainer::const_iterator end = visibleEnd;

         dataContainer->limitIteratorsToDataRange(begin, end, segment);

         if (begin == end)
            continue;

         for (QCPDigitalDataContainer::const_iterator it = begin; it != end; ++it)
         {
            // get reference to segment data
            const FrameData &data = *it;

            ChannelStyle style = styleMapper ? styleMapper(data.style) : styleMap.value(data.style, defaultStyle);

            // build segment bounds
            QRectF bounds = getFrameRect(data);

            // apply antialiasing hint
            graph->applyDefaultAntialiasingHint(painter);

            // draw shape
            drawShape(painter, bounds, style, data, i >= unselectedSegments.size());

            // draw text
            drawText(painter, bounds, style, data, i >= unselectedSegments.size());
         }
      }

      // draw other selection decoration that isn't just line/scatter pens and brushes:
      if (graph->mSelectionDecorator)
         graph->mSelectionDecorator->drawDecoration(painter, graph->selection());
   }

   void drawShape(QCPPainter *painter, const QRectF &bounds, const ChannelStyle &style, const FrameData &data, bool selected) const
   {
      double ramp = bounds.width() > 10 ? 5 : 0;

      QPolygonF polygon {
         QPointF(bounds.left(), bounds.center().y()),
         QPointF(bounds.left() + ramp, bounds.top()),
         QPointF(bounds.right() - ramp, bounds.top()),
         QPointF(bounds.right(), bounds.center().y()),
         QPointF(bounds.right() - ramp, bounds.bottom()),
         QPointF(bounds.left() + ramp, bounds.bottom())
      };

      if (selected && graph->mSelectionDecorator)
      {
         graph->mSelectionDecorator->applyPen(painter);
         graph->mSelectionDecorator->applyBrush(painter);
      }
      else
      {
         painter->setPen(style.shapePen);
         painter->setBrush(style.shapeBrush);
      }

      painter->drawPolygon(polygon);
   }

   void drawText(QCPPainter *painter, const QRectF &bounds, const ChannelStyle &style, const FrameData &data, bool selected) const
   {
      if (!valueMapper || bounds.width() < 20)
         return;

      QString value = valueMapper(data);

      QFontMetrics fontMetrics(style.labelFont);

      QRectF textBounds = bounds.adjusted(5, -1, -5, -1);

      QSize textSize = fontMetrics.size(0, value);

      bool trimmed = false;

      while (textSize.width() > textBounds.width())
      {
         int trimToSpace = value.lastIndexOf(" ");

         if (trimToSpace > -1)
            value = value.left(trimToSpace);
         else
            value = value.left(value.length() - 1);

         textSize = fontMetrics.size(0, value + "..");

         trimmed = true;
      }

      if (trimmed)
         value += "..";

      painter->setPen(style.labelPen);
      painter->setFont(style.labelFont);
      painter->drawText(textBounds, Qt::AlignCenter, value);
   }

   void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
   {
      painter->save();

      QRectF legendBounds(rect.left(), rect.top(), 60, rect.height() - 2);

      ChannelStyle defaultStyle {graph->mPen, graph->mPen, graph->mBrush, graph->mPen, graph->mParentPlot->font()};

      for (auto it = legendMap.constBegin(); it != legendMap.constEnd(); it++)
      {
         const DigitalLegend &legend = it.value();

         const ChannelStyle style = styleMapper ? styleMapper(legend.style) : styleMap.value(legend.style, defaultStyle);
         const QRectF bounds = getLegendRect(legend.text, style);

         legendBounds.setWidth(bounds.width());
         legendBounds.setHeight(bounds.height());

         QPolygonF polygon;

         polygon.append(QPointF(legendBounds.left(), legendBounds.center().y()));
         polygon.append(QPointF(legendBounds.left() + 5, legendBounds.top()));
         polygon.append(QPointF(legendBounds.right() - 5, legendBounds.top()));
         polygon.append(QPointF(legendBounds.right(), legendBounds.center().y()));
         polygon.append(QPointF(legendBounds.right() - 5, legendBounds.bottom()));
         polygon.append(QPointF(legendBounds.left() + 5, legendBounds.bottom()));

         painter->setPen(style.shapePen);
         painter->setBrush(style.shapeBrush);
         painter->drawPolygon(polygon);

         QRectF textBounds = legendBounds.adjusted(5, 0, -5, 0);

         painter->setPen(style.labelPen);
         painter->setFont(style.labelFont);
         painter->drawText(textBounds, Qt::AlignCenter, it->text);

         legendBounds = legendBounds.adjusted(bounds.width() + 10, 0, 0, 0);
      }

      painter->restore();
   }
};

FrameGraph::FrameGraph(QCPAxis *keyAxis, QCPAxis *valueAxis) : QCPAbstractPlottable(keyAxis, valueAxis), impl(new Impl(this))
{
}

void FrameGraph::setStyle(int key, const ChannelStyle &style)
{
   impl->styleMap[key] = style;
}

void FrameGraph::setLegend(int key, const QString &text, int style)
{
   impl->legendMap[key] = {key, style, text};
}

QSharedPointer<QCPDigitalDataContainer> FrameGraph::data() const
{
   return impl->dataContainer;
}

int FrameGraph::dataCount() const
{
   return impl->dataContainer->size();
}

void FrameGraph::setMapper(ValueMapper value, StyleMapper style) const
{
   impl->valueMapper = value;
   impl->styleMapper = style;
}

void FrameGraph::setOffset(double offset)
{
   impl->offset = offset;
}

double FrameGraph::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
   return impl->selectTest(pos, onlySelectable, details);
}

QCPRange FrameGraph::getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain) const
{
   return impl->dataContainer->keyRange(foundRange, inSignDomain);
}

QCPRange FrameGraph::getValueRange(bool &foundRange, QCP::SignDomain inSignDomain, const QCPRange &inKeyRange) const
{
   return impl->dataContainer->valueRange(foundRange, inSignDomain, inKeyRange);
}

void FrameGraph::draw(QCPPainter *painter)
{
   impl->draw(painter);
}

void FrameGraph::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{
   impl->drawLegendIcon(painter, rect);
}

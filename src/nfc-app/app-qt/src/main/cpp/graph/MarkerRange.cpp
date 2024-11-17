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

#include <QString>

#include <format/DataFormat.h>

#include <styles/Theme.h>

#include "MarkerRange.h"

struct MarkerRange::Impl
{
   static const QPen defaultMarkerPen;
   static const QBrush defaultMarkerBrush;
   static const QPen defaultSelectedPen;
   static const QBrush defaultSelectedBrush;
   static const QPen defaultLinePen;

   QCustomPlot *plot;

   QCPItemRect *markerRect;
   QCPItemTracer *lowerTracer;
   QCPItemTracer *upperTracer;
   QCPItemTracer *centerTracer;
   QCPItemLine *leftLine;
   QCPItemLine *rightLine;
   QCPItemLine *leftArrow;
   QCPItemLine *rightArrow;
   QCPItemLine *spanArrow;
   QCPItemText *lowerLabel;
   QCPItemText *upperLabel;
   QCPItemText *spanLabel;
   QFontMetrics labelFontMetrics;

   double startValue = 0;
   double endValue = 0;
   bool completed = true;
   bool spanVisible = true;

   std::function<QString(double)> cursorFormatter = DataFormat::number;
   std::function<QString(double, double)> rangeFormatter = DataFormat::range;

   QMetaObject::Connection selectionStartedConnection;
   QMetaObject::Connection selectionChangedConnection;
   QMetaObject::Connection selectionAcceptedConnection;
   QMetaObject::Connection selectionCancelledConnection;
   QMetaObject::Connection rangeChangedConnection;

   explicit Impl(QCustomPlot *plot) : plot(plot),
                                      markerRect(new QCPItemRect(plot)),
                                      lowerTracer(new QCPItemTracer(plot)),
                                      upperTracer(new QCPItemTracer(plot)),
                                      centerTracer(new QCPItemTracer(plot)),
                                      leftLine(new QCPItemLine(plot)),
                                      rightLine(new QCPItemLine(plot)),
                                      leftArrow(new QCPItemLine(plot)),
                                      rightArrow(new QCPItemLine(plot)),
                                      spanArrow(new QCPItemLine(plot)),
                                      lowerLabel(new QCPItemText(plot)),
                                      upperLabel(new QCPItemText(plot)),
                                      spanLabel(new QCPItemText(plot)),
                                      labelFontMetrics(Theme::defaultLabelFont)
   {
      lowerTracer->setVisible(false);
      lowerTracer->setSelectable(false);
      lowerTracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
      lowerTracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      lowerTracer->position->setAxisRect(plot->xAxis->axisRect());
      lowerTracer->position->setAxes(plot->xAxis, nullptr);
      lowerTracer->position->setCoords(0, 0);

      upperTracer->setVisible(false);
      upperTracer->setSelectable(false);
      upperTracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
      upperTracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      upperTracer->position->setAxisRect(plot->xAxis->axisRect());
      upperTracer->position->setAxes(plot->xAxis, nullptr);
      upperTracer->position->setCoords(0, 0);

      centerTracer->setVisible(false);
      centerTracer->setSelectable(false);
      centerTracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
      centerTracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      centerTracer->position->setAxisRect(plot->xAxis->axisRect());
      centerTracer->position->setAxes(plot->xAxis, nullptr);
      centerTracer->position->setCoords(0, 0);

      leftArrow->setVisible(false);
      leftArrow->setSelectable(false);
      leftArrow->setPen(defaultLinePen);
      leftArrow->setLayer("overlay");
      leftArrow->setClipToAxisRect(true);
      leftArrow->setHead(QCPLineEnding::esFlatArrow);
      leftArrow->end->setParentAnchor(lowerTracer->position);
      leftArrow->end->setCoords(0, 12);
      leftArrow->start->setParentAnchor(leftArrow->end);
      leftArrow->start->setCoords(-10, 0);

      rightArrow->setVisible(false);
      rightArrow->setSelectable(false);
      rightArrow->setLayer("overlay");
      rightArrow->setPen(defaultLinePen);
      rightArrow->setClipToAxisRect(true);
      rightArrow->setHead(QCPLineEnding::esFlatArrow);
      rightArrow->end->setParentAnchor(upperTracer->position);
      rightArrow->end->setCoords(0, 12);
      rightArrow->start->setParentAnchor(rightArrow->end);
      rightArrow->start->setCoords(10, 0);

      spanArrow->setVisible(false);
      spanArrow->setSelectable(false);
      spanArrow->setLayer("overlay");
      spanArrow->setPen(defaultLinePen);
      spanArrow->setClipToAxisRect(true);
      spanArrow->setHead(QCPLineEnding::esFlatArrow);
      spanArrow->setTail(QCPLineEnding::esFlatArrow);
      spanArrow->start->setParentAnchor(leftArrow->end);
      spanArrow->end->setParentAnchor(rightArrow->end);

      lowerLabel->setVisible(false);
      lowerLabel->setSelectable(false);
      lowerLabel->setLayer("overlay");
      lowerLabel->setFont(Theme::defaultLabelFont);
      lowerLabel->setColor(Theme::defaultLabelColor);
      lowerLabel->setPen(Theme::defaultLabelPen);
      lowerLabel->setBrush(Theme::defaultLabelBrush);
      lowerLabel->setClipToAxisRect(true);
      lowerLabel->setPadding(QMargins(6, 2, 6, 4));
      lowerLabel->setPositionAlignment(Qt::AlignRight | Qt::AlignVCenter);
      lowerLabel->position->setParentAnchor(leftArrow->start);
      lowerLabel->position->setCoords(0, 0);

      upperLabel->setVisible(false);
      upperLabel->setSelectable(false);
      upperLabel->setLayer("overlay");
      upperLabel->setFont(Theme::defaultLabelFont);
      upperLabel->setColor(Theme::defaultLabelColor);
      upperLabel->setPen(Theme::defaultLabelPen);
      upperLabel->setBrush(Theme::defaultLabelBrush);
      upperLabel->setClipToAxisRect(true);
      upperLabel->setPadding(QMargins(6, 2, 6, 4));
      upperLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      upperLabel->position->setParentAnchor(rightArrow->start);
      upperLabel->position->setCoords(0, 0);

      spanLabel->setVisible(false);
      spanLabel->setSelectable(false);
      spanLabel->setLayer("overlay");
      spanLabel->setFont(Theme::defaultLabelFont);
      spanLabel->setColor(Theme::defaultLabelColor);
      spanLabel->setPen(Theme::defaultLabelPen);
      spanLabel->setBrush(Theme::defaultLabelBrush);
      spanLabel->setClipToAxisRect(true);
      spanLabel->setPadding(QMargins(6, 2, 6, 4));
      spanLabel->setPositionAlignment(Qt::AlignCenter | Qt::AlignVCenter);
      spanLabel->position->setParentAnchor(centerTracer->position);
      spanLabel->position->setCoords(0, 10);

      markerRect->setVisible(false);
      markerRect->setSelectable(false);
      markerRect->setLayer("overlay");
      markerRect->setPen(defaultMarkerPen);
      markerRect->setBrush(defaultMarkerBrush);
      markerRect->setSelectedPen(defaultSelectedPen);
      markerRect->setSelectedBrush(defaultSelectedBrush);
      markerRect->setClipToAxisRect(true);
      markerRect->topLeft->setTypeY(QCPItemPosition::ptAxisRectRatio);
      markerRect->topLeft->setParentAnchorX(lowerTracer->position);
      markerRect->topLeft->setCoords(0, 1);
      markerRect->bottomRight->setParentAnchorX(upperTracer->position);
      markerRect->bottomRight->setParentAnchorY(centerTracer->position);
      markerRect->bottomRight->setCoords(0, 40);

      leftLine->setVisible(false);
      leftLine->setSelectable(false);
      leftLine->setPen(defaultLinePen);
      leftLine->setLayer("overlay");
      leftLine->setClipToAxisRect(true);
      leftLine->start->setParentAnchorX(lowerTracer->position);
      leftLine->start->setParentAnchorY(markerRect->bottomRight);
      leftLine->start->setCoords(0, -4);
      leftLine->end->setParentAnchor(lowerTracer->position);
      leftLine->end->setCoords(0, 5);

      rightLine->setVisible(false);
      rightLine->setSelectable(false);
      rightLine->setPen(defaultLinePen);
      rightLine->setLayer("overlay");
      rightLine->setClipToAxisRect(true);
      rightLine->start->setParentAnchorX(upperTracer->position);
      rightLine->start->setParentAnchorY(markerRect->bottomRight);
      rightLine->start->setCoords(0, -4);
      rightLine->end->setParentAnchor(upperTracer->position);
      rightLine->end->setCoords(0, 5);

      selectionStartedConnection = QObject::connect(plot->selectionRect(), &QCPSelectionRect::started, [=](QMouseEvent *event) {
         start(event);
      });

      selectionChangedConnection = QObject::connect(plot->selectionRect(), &QCPSelectionRect::changed, [=](const QRect &rect, QMouseEvent *event) {
         update(rect.normalized(), event);
      });

      selectionAcceptedConnection = QObject::connect(plot->selectionRect(), &QCPSelectionRect::accepted, [=](const QRect &rect, QMouseEvent *event) {
         accept(rect.normalized(), event);
      });

      selectionCancelledConnection = QObject::connect(plot->selectionRect(), &QCPSelectionRect::canceled, [=](const QRect &rect, QInputEvent *event) {
         cancel(event);
      });

      rangeChangedConnection = QObject::connect(plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &newRange) {
         rangeChanged(newRange);
      });
   }

   ~Impl()
   {
      QObject::disconnect(selectionStartedConnection);
      QObject::disconnect(selectionChangedConnection);
      QObject::disconnect(selectionAcceptedConnection);
      QObject::disconnect(selectionCancelledConnection);
      QObject::disconnect(rangeChangedConnection);

      plot->removeItem(rightLine);
      plot->removeItem(leftLine);
      plot->removeItem(markerRect);
      plot->removeItem(spanLabel);
      plot->removeItem(upperLabel);
      plot->removeItem(lowerLabel);
      plot->removeItem(spanArrow);
      plot->removeItem(rightArrow);
      plot->removeItem(leftArrow);
      plot->removeItem(centerTracer);
      plot->removeItem(upperTracer);
      plot->removeItem(lowerTracer);
   }

   void start(QMouseEvent *event)
   {
      // de-bounce show to avoid flickering
      completed = false;
      QTimer::singleShot(250, [&]() {
         if (!completed && !markerRect->visible())
            show();
      });
   }

   void update(const QRect &rect, QMouseEvent *event)
   {
      startValue = plot->xAxis->pixelToCoord(rect.left());
      endValue = plot->xAxis->pixelToCoord(rect.right());

      if (!markerRect->visible())
         show();

      update();
   }

   void accept(const QRect &rect, QMouseEvent *event)
   {
      completed = true;
      update(rect, event);
   }

   void cancel(QInputEvent *event)
   {
      completed = true;
      hide();
   }

   void show()
   {
      markerRect->setVisible(true);
      leftLine->setVisible(true);
      rightLine->setVisible(true);
      leftArrow->setVisible(true);
      rightArrow->setVisible(true);
      spanArrow->setVisible(true);
      lowerLabel->setVisible(true);
      upperLabel->setVisible(true);
      spanLabel->setVisible(spanVisible);

      update();
   }

   void hide()
   {
      markerRect->setVisible(false);
      leftLine->setVisible(false);
      rightLine->setVisible(false);
      leftArrow->setVisible(false);
      rightArrow->setVisible(false);
      spanArrow->setVisible(false);
      lowerLabel->setVisible(false);
      upperLabel->setVisible(false);
      spanLabel->setVisible(false);
   }

   void update()
   {
      double start = startValue < endValue ? startValue : endValue;
      double end = endValue > startValue ? endValue : startValue;
      double range = end - start;
      double center = start + range / 2;

      spanArrow->setVisible(false);
      spanLabel->setVisible(spanVisible && markerRect->visible());

      if (range > 0)
      {
         double markerPixelStart = plot->xAxis->coordToPixel(start);
         double markerPixelEnd = plot->xAxis->coordToPixel(end);
         double markerPixelWidth = markerPixelEnd - markerPixelStart;

         QString timeValue = rangeFormatter(start, end);

         spanLabel->setText(timeValue);

         QSize spanLabelWidth = labelFontMetrics.size(0, timeValue);

         if ((markerPixelWidth > spanLabelWidth.width() + 16))
         {
            spanArrow->setVisible(markerRect->visible());

            markerRect->bottomRight->setCoords(0, spanLabelWidth.height() + 8);
            spanLabel->setPositionAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            spanLabel->position->setParentAnchor(centerTracer->position);
            spanLabel->position->setCoords(0, 10);
         }
         else
         {
            markerRect->bottomRight->setCoords(0, 12);
            spanLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            spanLabel->position->setParentAnchor(upperLabel->position);
            spanLabel->position->setCoords(0, 18);
         }
      }

      // update labels
      lowerLabel->setText(cursorFormatter(start));
      upperLabel->setText(cursorFormatter(end));

      // update marker startTracer
      lowerTracer->position->setCoords(start, 0);
      upperTracer->position->setCoords(end, 0);
      centerTracer->position->setCoords(center, 0);
   }

   double selectTest(const QPointF &pos)
   {
      return markerRect->selectTest(pos, false, nullptr);
   }

   void rangeChanged(const QCPRange &newRange)
   {
      update();
   }
};

// initialize default colors
const QPen MarkerRange::Impl::defaultMarkerPen({0x00, 0x80, 0xFF, 0x50});
const QBrush MarkerRange::Impl::defaultMarkerBrush({0x00, 0x80, 0xFF, 0x50});
const QPen MarkerRange::Impl::defaultSelectedPen({0x00, 0xFF, 0x80, 0x50});
const QBrush MarkerRange::Impl::defaultSelectedBrush({0x00, 0xFF, 0x80, 0x50});
const QPen MarkerRange::Impl::defaultLinePen({0x55, 0x55, 0x55, 0xFF});

MarkerRange::MarkerRange(QCustomPlot *plot) : impl(new Impl(plot))
{
}

void MarkerRange::setFormatter(const std::function<QString(double)> &formatter)
{
   impl->cursorFormatter = formatter;
}

void MarkerRange::setRangeFormatter(const std::function<QString(double, double)> &formatter)
{
   impl->rangeFormatter = formatter;
}

void MarkerRange::setRangeVisible(bool enable)
{
   impl->spanVisible = enable;
}


bool MarkerRange::isRangeVisible() const
{
   return impl->spanVisible;
}

QPen MarkerRange::markerPen()
{
   return impl->markerRect->pen();
}

void MarkerRange::setMarkerPen(const QPen &pen)
{
   impl->markerRect->setPen(pen);
}

QBrush MarkerRange::markerBrush()
{
   return impl->markerRect->brush();
}

void MarkerRange::setMarkerBrush(const QBrush &brush)
{
   impl->markerRect->setBrush(brush);
}

QPen MarkerRange::selectedPen()
{
   return impl->markerRect->selectedPen();
}

void MarkerRange::setSelectedPen(const QPen &pen)
{
   impl->markerRect->setSelectedPen(pen);
}

QBrush MarkerRange::selectedBrush()
{
   return impl->markerRect->selectedBrush();
}

void MarkerRange::setSelectedBrush(const QBrush &brush)
{
   impl->markerRect->setSelectedBrush(brush);
}

QPen MarkerRange::linePen()
{
   return impl->spanArrow->pen();
}

void MarkerRange::setLinePen(const QPen &pen)
{
   impl->leftArrow->setPen(pen);
   impl->rightArrow->setPen(pen);
   impl->spanArrow->setPen(pen);
}

QFont MarkerRange::labelFont()
{
   return impl->spanLabel->font();
}

void MarkerRange::setLabelFont(const QFont &font)
{
   impl->lowerLabel->setFont(font);
   impl->upperLabel->setFont(font);
   impl->spanLabel->setFont(font);
   impl->labelFontMetrics = QFontMetrics(font);
}

QColor MarkerRange::labelColor()
{
   return impl->spanLabel->color();
}

void MarkerRange::setLabelColor(const QColor &color)
{
   impl->lowerLabel->setColor(color);
   impl->upperLabel->setColor(color);
   impl->spanLabel->setColor(color);
}

QPen MarkerRange::labelPen()
{
   return impl->spanLabel->pen();
}

void MarkerRange::setLabelPen(const QPen &pen)
{
   impl->lowerLabel->setPen(pen);
   impl->upperLabel->setPen(pen);
   impl->spanLabel->setPen(pen);
}

QBrush MarkerRange::labelBrush()
{
   return impl->spanLabel->brush();
}

void MarkerRange::setLabelBrush(const QBrush &brush)
{
   impl->lowerLabel->setBrush(brush);
   impl->upperLabel->setBrush(brush);
   impl->spanLabel->setBrush(brush);
}

double MarkerRange::start() const
{
   return impl->lowerTracer->position->key();
}

double MarkerRange::end() const
{
   return impl->upperTracer->position->key();
}

void MarkerRange::setRange(double start, double end)
{
   impl->startValue = start;
   impl->endValue = end;
   impl->update();
}

bool MarkerRange::visible() const
{
   return impl->markerRect->visible();
}

void MarkerRange::setVisible(bool visible)
{
   if (visible)
      impl->show();
   else
      impl->hide();
}

double MarkerRange::selectTest(const QPointF &pos)
{
   return impl->selectTest(pos);
}

double MarkerRange::center() const
{
   return start() + width() / 2;
}

double MarkerRange::width() const
{
   return end() - start();
}

void MarkerRange::clear()
{
   setVisible(false);
}

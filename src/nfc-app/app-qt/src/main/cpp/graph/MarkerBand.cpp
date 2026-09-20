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

#include <QList>

#include "MarkerBand.h"

// margin in pixels left around a label before it is considered not to fit
#define LABEL_MARGIN 6

struct BandItem
{
   QCPItemRect *rect = nullptr;
   QCPItemText *label = nullptr;

   double start = 0;
   double end = 0;
   double level = 0;
};

struct MarkerBand::Impl
{
   static const QColor defaultLabelColor;
   static const QFont defaultLabelFont;

   QCustomPlot *plot;

   QFont labelFont;
   QColor labelColor;
   QFontMetrics labelFontMetrics;

   double lower = 0;
   double upper = 1;

   QList<BandItem> elements;

   QMetaObject::Connection rangeChangedConnection;

   explicit Impl(QCustomPlot *plot) : plot(plot),
                                      labelFont(defaultLabelFont),
                                      labelColor(defaultLabelColor),
                                      labelFontMetrics(defaultLabelFont)
   {
      rangeChangedConnection = QObject::connect(plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &) {
         update();
      });
   }

   ~Impl()
   {
      QObject::disconnect(rangeChangedConnection);

      clear();
   }

   void setHeight(double newLower, double newUpper)
   {
      lower = newLower;
      upper = newUpper;

      for (const BandItem &item: elements)
      {
         applyGeometry(item);
      }
   }

   void addRange(double start, double end, double level, const QString &label, const QPen &pen, const QBrush &brush)
   {
      BandItem item;

      item.start = start;
      item.end = end;
      item.level = level;

      item.rect = new QCPItemRect(plot);
      item.rect->setSelectable(false);
      item.rect->setClipToAxisRect(true);
      item.rect->setPen(pen);
      item.rect->setBrush(level > 0 ? brush : QBrush(Qt::NoBrush));
      item.rect->topLeft->setAxes(plot->xAxis, plot->yAxis);
      item.rect->bottomRight->setAxes(plot->xAxis, plot->yAxis);

      if (!label.isEmpty())
      {
         item.label = new QCPItemText(plot);
         item.label->setSelectable(false);
         item.label->setClipToAxisRect(true);
         item.label->setFont(labelFont);
         item.label->setColor(labelColor);
         item.label->setPen(Qt::NoPen);
         item.label->setBrush(Qt::NoBrush);
         item.label->setText(label);
         item.label->setPositionAlignment(Qt::AlignCenter);
         item.label->position->setAxes(plot->xAxis, plot->yAxis);
      }

      elements.append(item);

      applyGeometry(item);
      applyLabel(item);
   }

   void applyGeometry(const BandItem &item) const
   {
      // a level of zero collapses the block onto the lower edge, which reads as the flat line of an idle channel
      const double top = lower + (upper - lower) * item.level;

      item.rect->topLeft->setCoords(item.start, top);
      item.rect->bottomRight->setCoords(item.end, lower);
   }

   /*
    * Park the label on the middle of whatever part of the block is on screen, and hide it when that part is too
    * narrow to hold the text
    */
   void applyLabel(const BandItem &item) const
   {
      if (!item.label)
         return;

      const QCPRange range = plot->xAxis->range();

      const double visibleStart = std::max(item.start, range.lower);
      const double visibleEnd = std::min(item.end, range.upper);

      if (visibleStart >= visibleEnd)
      {
         item.label->setVisible(false);
         return;
      }

      const double width = plot->xAxis->coordToPixel(visibleEnd) - plot->xAxis->coordToPixel(visibleStart);

      if (width < labelFontMetrics.size(0, item.label->text()).width() + LABEL_MARGIN)
      {
         item.label->setVisible(false);
         return;
      }

      item.label->setVisible(true);
      item.label->position->setCoords((visibleStart + visibleEnd) / 2, (lower + upper) / 2);
   }

   void update() const
   {
      for (const BandItem &item: elements)
      {
         applyLabel(item);
      }
   }

   void setUpperBound(double end)
   {
      if (elements.isEmpty())
         return;

      BandItem &item = elements.last();

      if (end <= item.start)
         return;

      item.end = end;

      applyGeometry(item);
      applyLabel(item);
   }

   void clear()
   {
      for (const BandItem &item: elements)
      {
         if (item.label)
            plot->removeItem(item.label);

         plot->removeItem(item.rect);
      }

      elements.clear();
   }
};

const QColor MarkerBand::Impl::defaultLabelColor({0xF0, 0xF0, 0xF0, 0xFF});
const QFont MarkerBand::Impl::defaultLabelFont("Roboto", 9, QFont::Bold);

MarkerBand::MarkerBand(QCustomPlot *plot) : impl(new Impl(plot))
{
}

const QFont &MarkerBand::labelFont() const
{
   return impl->labelFont;
}

void MarkerBand::setLabelFont(const QFont &font)
{
   impl->labelFont = font;
   impl->labelFontMetrics = QFontMetrics(font);
}

const QColor &MarkerBand::labelColor() const
{
   return impl->labelColor;
}

void MarkerBand::setLabelColor(const QColor &color)
{
   impl->labelColor = color;
}

void MarkerBand::setHeight(double lower, double upper)
{
   impl->setHeight(lower, upper);
}

void MarkerBand::addRange(double start, double end, double level, const QString &label, const QPen &pen, const QBrush &brush)
{
   impl->addRange(start, end, level, label, pen, brush);
}

void MarkerBand::setUpperBound(double end)
{
   impl->setUpperBound(end);
}

double MarkerBand::upperBound() const
{
   return impl->elements.isEmpty() ? 0 : impl->elements.last().end;
}

bool MarkerBand::isEmpty() const
{
   return impl->elements.isEmpty();
}

void MarkerBand::clear()
{
   impl->clear();
}

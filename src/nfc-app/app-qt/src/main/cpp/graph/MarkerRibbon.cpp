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

#include <QDebug>

#include "MarkerRibbon.h"

#include <utility>

struct RibbonItem
{
   QCPItemTracer *leftTracer = nullptr;
   QCPItemTracer *rightTracer = nullptr;
   QCPItemRect *rectMarker = nullptr;
   QCPItemText *textLabel = nullptr;
};

struct MarkerRibbon::Impl
{
   static const QColor defaultLabelColor;
   static const QPen defaultLabelPen;
   static const QBrush defaultLabelBrush;
   static const QFont defaultLabelFont;

   QCustomPlot *plot;

   QFont labelFont;
   QColor labelColor;
   QFontMetrics labelFontMetrics;
   QList<RibbonItem> elements;

   QMetaObject::Connection rangeChangedConnection;

   explicit Impl(QCustomPlot *plot) : plot(plot),
                                      labelFont(defaultLabelFont),
                                      labelColor(defaultLabelColor),
                                      labelFontMetrics(defaultLabelFont)
   {
      rangeChangedConnection = QObject::connect(plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &newRange) {
         rangeChanged(newRange);
      });
   }

   ~Impl()
   {
      QObject::disconnect(rangeChangedConnection);

      for (const RibbonItem &item: elements)
      {
         plot->removeItem(item.leftTracer);
         plot->removeItem(item.rightTracer);
         plot->removeItem(item.rectMarker);
         plot->removeItem(item.textLabel);
      }
   }

   void addRange(double start, double end, const QString &label, const QPen &pen, const QBrush &brush)
   {
      RibbonItem item;

      item.leftTracer = new QCPItemTracer(plot);
      item.leftTracer->setVisible(false);
      item.leftTracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      item.leftTracer->position->setAxisRect(plot->xAxis->axisRect());
      item.leftTracer->position->setAxes(plot->xAxis, nullptr);
      item.leftTracer->position->setCoords(start, 1);

      item.rightTracer = new QCPItemTracer(plot);
      item.rightTracer->setVisible(false);
      item.rightTracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      item.rightTracer->position->setAxisRect(plot->xAxis->axisRect());
      item.rightTracer->position->setAxes(plot->xAxis, nullptr);
      item.rightTracer->position->setCoords(end, 1);

      item.rectMarker = new QCPItemRect(plot);
      item.rectMarker->setSelectable(false);
      item.rectMarker->setPen(pen);
      item.rectMarker->setBrush(brush);
      item.rectMarker->setClipToAxisRect(true);
      item.rectMarker->topLeft->setParentAnchor(item.leftTracer->position);
      item.rectMarker->topLeft->setCoords(-3, -2);
      item.rectMarker->bottomRight->setParentAnchor(item.rightTracer->position);
      item.rectMarker->bottomRight->setCoords(3, -2 - labelFontMetrics.height());

      item.textLabel = new QCPItemText(plot);
      item.textLabel->setSelectable(false);
      item.textLabel->setVisible(false);
      item.textLabel->setFont(defaultLabelFont);
      item.textLabel->setColor(defaultLabelColor);
      item.textLabel->setPen(defaultLabelPen);
      item.textLabel->setBrush(defaultLabelBrush);
      item.textLabel->setSelectable(false);
      item.textLabel->setClipToAxisRect(true);
      item.textLabel->setText(label);
      item.textLabel->setPadding({4, 0, 0, 2});
      item.textLabel->setPositionAlignment(Qt::AlignBottom | Qt::AlignLeft);
      item.textLabel->position->setParentAnchor(item.rectMarker->topLeft);
      item.textLabel->position->setCoords(0, 0);

      elements.append(item);
   }

   void clear()
   {
      for (const RibbonItem &item: elements)
      {
         plot->removeItem(item.textLabel);
         plot->removeItem(item.rectMarker);
         plot->removeItem(item.leftTracer);
         plot->removeItem(item.rightTracer);
      }

      elements.clear();
   }

   void rangeChanged(const QCPRange &newRange)
   {
      for (const RibbonItem &item: elements)
      {
         double markerWidth = item.rectMarker->right->pixelPosition().x() - item.rectMarker->left->pixelPosition().x();
         QSize labelWidth = labelFontMetrics.size(0, item.textLabel->text());
         item.textLabel->setVisible(markerWidth > labelWidth.width());
      }
   }
};

const QColor MarkerRibbon::Impl::defaultLabelColor({0xF0, 0xF0, 0xF0, 0xFF});
const QPen MarkerRibbon::Impl::defaultLabelPen(Qt::NoPen);
const QBrush MarkerRibbon::Impl::defaultLabelBrush(Qt::NoBrush);
const QFont MarkerRibbon::Impl::defaultLabelFont("Roboto", 9, QFont::Bold);

MarkerRibbon::MarkerRibbon(QCustomPlot *plot) : impl(new Impl(plot))
{
}

const QFont &MarkerRibbon::labelFont()
{
   return impl->labelFont;
}

void MarkerRibbon::setLabelFont(const QFont &font)
{
   impl->labelFont = font;
   impl->labelFontMetrics = QFontMetrics(font);
}

void MarkerRibbon::addRange(double start, double end, const QString &label, const QPen &pen, const QBrush &brush)
{
   impl->addRange(start, end, label, pen, brush);
}

void MarkerRibbon::clear()
{
   impl->clear();
}

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

#include "MarkerBracket.h"

struct MarkerBracket::Impl
{
   QCustomPlot *plot;

   QCPItemTracer *leftTracer;
   QCPItemTracer *rightTracer;
   QCPItemBracket *bracketItem;
   QCPItemText *bracketLabel;
   QFontMetrics labelFontMetrics;

   QMetaObject::Connection rangeChangedConnection;

   Impl(QCustomPlot *plot) : plot(plot),
                             leftTracer(new QCPItemTracer(plot)),
                             rightTracer(new QCPItemTracer(plot)),
                             bracketItem(new QCPItemBracket(plot)),
                             bracketLabel(new QCPItemText(plot)),
                             labelFontMetrics(Theme::defaultBracketLabelFont)
   {
      leftTracer->setVisible(false);
      rightTracer->setVisible(false);

      bracketItem->setLength(10);
      bracketItem->setSelectable(false);
      bracketItem->setClipToAxisRect(true);
      bracketItem->setPen(Theme::defaultBracketPen);
      bracketItem->setLayer("overlay");
      bracketItem->left->setParentAnchor(leftTracer->position);
      bracketItem->left->setCoords(0, -5);
      bracketItem->right->setParentAnchor(rightTracer->position);
      bracketItem->right->setCoords(0, -5);

      bracketLabel->setFont(Theme::defaultBracketLabelFont);
      bracketLabel->setColor(Theme::defaultBracketLabelColor);
      bracketLabel->setPen(Theme::defaultBracketLabelPen);
      bracketLabel->setBrush(Theme::defaultBracketLabelBrush);
      bracketLabel->setSelectable(false);
      bracketLabel->setClipToAxisRect(true);
      bracketLabel->setLayer("overlay");
      bracketLabel->setPositionAlignment(Qt::AlignBottom | Qt::AlignHCenter);
      bracketLabel->position->setParentAnchor(bracketItem->center);
      bracketLabel->position->setCoords(0, -5); // move 5 pixels to the top from bracket center anchor

      rangeChangedConnection = QObject::connect(plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &newRange) {
         rangeChanged(newRange);
      });
   }

   ~Impl()
   {
      QObject::disconnect(rangeChangedConnection);

      plot->removeItem(bracketLabel);
      plot->removeItem(bracketItem);
      plot->removeItem(leftTracer);
      plot->removeItem(rightTracer);
   }

   void update()
   {
      double bracketPixelStart = plot->xAxis->coordToPixel(leftTracer->position->key());
      double bracketPixelEnd = plot->xAxis->coordToPixel(rightTracer->position->key());
      double bracketPixelWidth = bracketPixelEnd - bracketPixelStart;

      QSize labelSize = labelFontMetrics.size(0, bracketLabel->text());

      // show bracket and label only if there is enough space
      if (bracketPixelWidth > labelSize.width())
      {
         bracketItem->setVisible(true);
         bracketLabel->setVisible(true);
         bracketLabel->setRotation(0);
         bracketLabel->setPositionAlignment(Qt::AlignBottom | Qt::AlignHCenter);
         bracketLabel->position->setCoords(0, -labelSize.height() / 2);
      }

         // show label only if there is enough space
      else if (bracketPixelWidth > labelSize.height())
      {
         bracketItem->setVisible(false);
         bracketLabel->setVisible(true);
         bracketLabel->setRotation(-90);
         bracketLabel->setPositionAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
         bracketLabel->position->setCoords(0, 5 - labelSize.width() / 2);
      }

         // hide bracket and label
      else
      {
         bracketItem->setVisible(false);
         bracketLabel->setVisible(false);
      }
   }

   void rangeChanged(const QCPRange &newRange)
   {
      if (newRange.lower > rightTracer->position->key() || newRange.upper < leftTracer->position->key())
         return;

      update();
   }
};

MarkerBracket::MarkerBracket(QCustomPlot *plot) : impl(new Impl(plot))
{
}

const QPen MarkerBracket::bracketPen()
{
   return impl->bracketItem->pen();
}

void MarkerBracket::setBracketPen(const QPen &pen)
{
   impl->bracketItem->setPen(pen);
}

const QFont MarkerBracket::labelFont()
{
   return impl->bracketLabel->font();
}

void MarkerBracket::setLabelFont(const QFont &font)
{
   impl->bracketLabel->setFont(font);
   impl->labelFontMetrics = QFontMetrics(font);
}

const QColor MarkerBracket::labelColor()
{
   return impl->bracketLabel->color();
}

void MarkerBracket::setLabelColor(const QColor &color)
{
   impl->bracketLabel->setColor(color);
}

const QPen MarkerBracket::labelPen()
{
   return impl->bracketLabel->pen();
}

void MarkerBracket::setLabelPen(const QPen &pen)
{
   impl->bracketLabel->setPen(pen);
}

const QBrush MarkerBracket::labelBrush()
{
   return impl->bracketLabel->brush();
}

void MarkerBracket::setLabelBrush(const QBrush &brush)
{
   impl->bracketLabel->setBrush(brush);
}

QPointF MarkerBracket::left() const
{
   return impl->leftTracer->position->coords();
}

void MarkerBracket::setLeft(const QPointF &point)
{
   impl->leftTracer->position->setCoords(point);
   impl->update();
}

QPointF MarkerBracket::right() const
{
   return impl->rightTracer->position->coords();
}

void MarkerBracket::setRight(const QPointF &point)
{
   impl->rightTracer->position->setCoords(point);
   impl->update();
}

QString MarkerBracket::text() const
{
   return impl->bracketLabel->text();
}

void MarkerBracket::setText(const QString &text)
{
   impl->bracketLabel->setText(text);
   impl->update();
}
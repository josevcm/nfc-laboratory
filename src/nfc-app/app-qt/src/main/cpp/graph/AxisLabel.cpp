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

#include "AxisLabel.h"

struct AxisLabel::Impl
{
   QCPAxis *axis;
   QCustomPlot *plot;
   QCPItemTracer *tracer;
   QCPItemText *label;
   QFontMetrics labelFontMetrics;

   Impl(QCPAxis *axis) : axis(axis),
                         plot(axis->parentPlot()),
                         tracer(new QCPItemTracer(plot)),
                         label(new QCPItemText(plot)),
                         labelFontMetrics(Theme::defaultLabelFont)
   {
      tracer->setVisible(false);
      tracer->setSelectable(false);
      tracer->position->setTypeX(QCPItemPosition::ptAxisRectRatio);
      tracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      tracer->position->setAxisRect(axis->axisRect());
      tracer->position->setAxes(nullptr, axis);

      label->setVisible(false);
      label->setSelectable(false);
      label->setClipToAxisRect(false);
      label->setPositionAlignment(Qt::AlignRight | Qt::AlignVCenter);
      label->setTextAlignment(Qt::AlignRight);
      label->setFont(Theme::defaultLabelFont);
      label->setColor(Theme::defaultLabelColor);
      label->setPadding(QMargins(5, 5, 5, 5));
      label->position->setParentAnchor(tracer->position);
      label->position->setCoords(0, 0);
   }

   void setText(const QString &text, Qt::Corner corner, Qt::Orientation orientation)
   {
      QSize textSize = labelFontMetrics.size(0, text);

      QSize rectSize(textSize.width() + label->padding().left() + label->padding().right(),
                     textSize.height() + label->padding().top() + label->padding().bottom());

      QMargins margins = axis->axisRect()->minimumMargins();

      switch (corner)
      {
         case Qt::TopLeftCorner:
         {
            if (margins.top() < rectSize.height())
               margins.setTop(rectSize.height());

            if (margins.left() < rectSize.width())
               margins.setLeft(rectSize.width());

            label->setPositionAlignment(Qt::AlignRight | Qt::AlignVCenter);
            label->setTextAlignment(Qt::AlignRight);
            label->position->setCoords(0, -textSize.height());
            tracer->position->setCoords(0, 0);
            break;
         }

         case Qt::TopRightCorner:
         {
            if (margins.top() < rectSize.height())
               margins.setTop(rectSize.height());

            if (margins.right() < rectSize.width())
               margins.setRight(rectSize.width());

            label->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            label->setTextAlignment(Qt::AlignLeft);
            label->position->setCoords(0, -textSize.height());
            tracer->position->setCoords(1, 0);
            break;
         }

         case Qt::BottomLeftCorner:
         {
            if (margins.bottom() < rectSize.height())
               margins.setBottom(rectSize.height());

            if (margins.left() < rectSize.width())
               margins.setLeft(rectSize.width());

            label->setPositionAlignment(Qt::AlignRight | Qt::AlignVCenter);
            label->setTextAlignment(Qt::AlignRight);
            label->position->setCoords(0, 5);
            tracer->position->setCoords(0, 1);
            break;
         }

         case Qt::BottomRightCorner:
         {
            if (margins.bottom() < rectSize.height())
               margins.setBottom(rectSize.height());

            if (margins.right() < rectSize.width())
               margins.setRight(rectSize.width());

            label->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            label->setTextAlignment(Qt::AlignLeft);
            label->position->setCoords(0, 5);
            tracer->position->setCoords(1, 1);
            break;
         }
      }

      // update label text
      label->setText(text);

      // update rect margins if necessary
      if (axis->axisRect()->minimumMargins() != margins)
         axis->axisRect()->setMinimumMargins(margins);
   }
};

AxisLabel::AxisLabel(QCPAxis *axis) : impl(new Impl(axis))
{
}

void AxisLabel::setText(const QString &text, Qt::Corner corner, Qt::Orientation orientation)
{
   impl->setText(text, corner, orientation);
}

void AxisLabel::setVisible(bool visible)
{
   impl->label->setVisible(visible);
}

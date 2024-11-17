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

#ifndef NFC_LAB_MARKERBRACKET_H
#define NFC_LAB_MARKERBRACKET_H

#include <QSharedPointer>

#include <3party/customplot/QCustomPlot.h>

class MarkerBracket
{
      struct Impl;

   public:

      MarkerBracket(QCustomPlot *plot);

      const QPen bracketPen();

      void setBracketPen(const QPen &pen);

      const QFont labelFont();

      void setLabelFont(const QFont &font);

      const QColor labelColor();

      void setLabelColor(const QColor &color);

      const QPen labelPen();

      void setLabelPen(const QPen &pen);

      const QBrush labelBrush();

      void setLabelBrush(const QBrush &brush);

      QPointF left() const;

      void setLeft(const QPointF &point);

      QPointF right() const;

      void setRight(const QPointF &point);

      QString text() const;

      void setText(const QString &text);

   private:

      QSharedPointer<Impl> impl;
};


#endif //NFC_LAB_MARKERBRACKET_H

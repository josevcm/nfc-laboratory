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

#ifndef NFC_LAB_MARKERRANGE_H
#define NFC_LAB_MARKERRANGE_H

#include <QSharedPointer>

#include <3party/customplot/QCustomPlot.h>

class MarkerRange
{
      struct Impl;

   public:

      explicit MarkerRange(QCustomPlot *plot);

      void setFormatter(const std::function<QString(double)> &formatter);

      void setRangeFormatter(const std::function<QString(double, double)> &formatter);

      void setRangeVisible(bool enable);

      bool isRangeVisible() const;

      QPen markerPen();

      void setMarkerPen(const QPen &pen);

      QBrush markerBrush();

      void setMarkerBrush(const QBrush &brush);

      QPen selectedPen();

      void setSelectedPen(const QPen &pen);

      QBrush selectedBrush();

      void setSelectedBrush(const QBrush &brush);

      QPen linePen();

      void setLinePen(const QPen &pen);

      QFont labelFont();

      void setLabelFont(const QFont &font);

      QColor labelColor();

      void setLabelColor(const QColor &color);

      QPen labelPen();

      void setLabelPen(const QPen &pen);

      QBrush labelBrush();

      void setLabelBrush(const QBrush &brush);

      double start() const;

      double end() const;

      bool visible() const;

      void setVisible(bool visible);

      void setRange(double start, double end);

      double selectTest(const QPointF &pos);

      double center() const;

      double width() const;

      void clear();

   private:

      QSharedPointer<Impl> impl;
};


#endif //NFC_LAB_MARKERRANGE_H

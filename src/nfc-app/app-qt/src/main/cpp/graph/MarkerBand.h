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

#ifndef NFC_LAB_MARKERBAND_H
#define NFC_LAB_MARKERBAND_H

#include <QSharedPointer>

#include <3party/customplot/QCustomPlot.h>

/*
 * Row of labelled blocks drawn in plot coordinates, used where a channel is summarized rather than plotted.
 *
 * Unlike MarkerRibbon, which pins a strip to the bottom of the axis rect, a band sits on the channel row it belongs
 * to. Each block keeps its label on the visible part of its span, so a block wider than the viewport, which is the
 * normal case for a clock that runs steadily through a whole capture, stays readable at any zoom.
 */
class MarkerBand
{
      struct Impl;

   public:

      explicit MarkerBand(QCustomPlot *plot);

      const QFont &labelFont() const;

      void setLabelFont(const QFont &font);

      const QColor &labelColor() const;

      void setLabelColor(const QColor &color);

      /*
       * Vertical extent of the band, in plot coordinates
       */
      void setHeight(double lower, double upper);

      /*
       * Add a block spanning [start, end], filling `level` of the band height: 1 draws a full block, 0 flattens it
       * into a line along the lower edge. An empty label leaves the block unlabelled.
       */
      void addRange(double start, double end, double level, const QString &label, const QPen &pen, const QBrush &brush);

      /*
       * Extend the last block to `end`, so a block that is still open can follow a capture as it grows
       */
      void setUpperBound(double end);

      /*
       * Right edge of the last block, or zero when the band is empty
       */
      double upperBound() const;

      bool isEmpty() const;

      void clear();

   private:

      QSharedPointer<Impl> impl;
};

#endif //NFC_LAB_MARKERBAND_H

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

#ifndef NFC_LAB_FRAMEDATA_H
#define NFC_LAB_FRAMEDATA_H

#include <3party/customplot/QCustomPlot.h>

class FrameData
{
   public:

      FrameData(int type = -1, int style = -1, double startTime = 0, double endTime = 0, double height = 24, const QByteArray &data = {});

      double sortKey() const
      {
         return start;
      }

      static FrameData fromSortKey(double sortKey)
      {
         return {-1, -1, sortKey, sortKey, {}};
      }

      static bool sortKeyIsMainKey()
      {
         return true;
      }

      double mainKey() const
      {
         return start;
      }

      QByteArray mainValue() const
      {
         return data;
      }

      QCPRange keyRange() const
      {
         return QCPRange(start, end);
      }

      QCPRange valueRange() const
      {
         return QCPRange(1, 1);
      }

      int type;
      int style;
      double start;
      double end;
      double height;
      QByteArray data;
};

#endif

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

#ifndef NFC_LAB_DATAFORMAT_H
#define NFC_LAB_DATAFORMAT_H

#include <QString>

class DataFormat
{
   public:

      static QString number(double v);

      static QString range(double a, double b);

      static QString time(double v);

      static QString timeRange(double a, double b);

      static QString frequency(double v);

      static QString frequencyRange(double a, double b);

      static QString percentage(double v);
};

#endif //NFC_LAB_DATAFORMAT_H

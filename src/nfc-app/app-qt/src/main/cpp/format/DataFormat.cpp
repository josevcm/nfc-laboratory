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

#include "DataFormat.h"

QString DataFormat::number(double v)
{
   return QString("%1").arg(v, 0, 'f', 3);
}

QString DataFormat::range(double a, double b)
{
   return QString("%1").arg(a - b, 0, 'f', 3);
}

QString DataFormat::time(double v)
{
   if (v < 1E-6)
      return QString("%1 ns").arg(v * 1E9, 3, 'f', 3);

   if (v < 1E-3)
      return QString("%1 µs").arg(v * 1E6, 3, 'f', 3);

   if (v < 1E0)
      return QString("%1 ms").arg(v * 1E3, 7, 'f', 3);

   return QString("%1 s").arg(v, 7, 'f', 5);
}

QString DataFormat::timeRange(double a, double b)
{
   return time(std::abs(a - b));
}

QString DataFormat::frequency(double v)
{
   if (v > 1E9)
      return QString("%1 GHz").arg(v / 1E9, 3, 'f', 3);

   if (v > 1E6)
      return QString("%1 MHz").arg(v / 1E6, 3, 'f', 3);

   if (v > 1E3)
      return QString("%1 kHz").arg(v / 1E3, 3, 'f', 3);

   return QString("%1 Hz").arg(v, 7, 'f', 3);
}

QString DataFormat::frequencyRange(double a, double b)
{
   return frequency(std::abs(a - b));
}

QString DataFormat::percentage(double v)
{
   return QString("%1 %").arg(v * 100, 7, 'f', 2);
}


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
#include <QStyleOption>

#include "IconStyle.h"

QPixmap IconStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const
{
   switch (iconMode)
   {
      case QIcon::Disabled:
      {
         // override disabled icon style, with something which works better across different light/dark themes.
         // the default Qt style here only works nicely for light themes.
         QImage image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);

         for (int i = 0; i < image.width(); i++)
         {
            for (int j = 0; j < image.height(); j++)
            {
               QColor color = image.pixelColor(i, j);

               int h = color.hue();
               int s = std::min(static_cast< int >(color.saturation() * 0.5), 255);
               int v = color.value();
               int a = std::min(static_cast< int >(color.alpha() * 0.3), 255);

               color.setHsv(h, s, v, a);

               image.setPixelColor(i, j, color);
            }
         }

         return QPixmap::fromImage(image);
      }

      case QIcon::Normal:
      case QIcon::Active:
      case QIcon::Selected:
         break;
   }

   return QProxyStyle::generatedIconPixmap(iconMode, pixmap, opt);
}

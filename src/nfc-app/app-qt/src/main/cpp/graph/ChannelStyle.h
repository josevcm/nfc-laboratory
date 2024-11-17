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

#ifndef NFC_SPY_CHANNELSTYLE_H
#define NFC_SPY_CHANNELSTYLE_H

#include <QPen>
#include <QBrush>
#include <QFont>
#include <QString>

struct ChannelStyle
{
   QPen linePen;
   QPen shapePen;
   QBrush shapeBrush;
   QPen labelPen;
   QFont labelFont;
   QString text;
};

#endif

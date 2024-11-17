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

#ifndef NFC_LAB_PARSER_H
#define NFC_LAB_PARSER_H

#include <lab/data/RawFrame.h>

#include <protocol/ProtocolFrame.h>

struct Parser
{
  virtual ~Parser() = default;

  ProtocolFrame *buildRootInfo(const QString &name, const lab::RawFrame &frame, int flags);

  ProtocolFrame *buildChildInfo(const QVariant &info);

  ProtocolFrame *buildChildInfo(const QString &name, const QVariant &info);

  ProtocolFrame *buildChildInfo(const QString &name, const QVariant &info, int start, int length);

  ProtocolFrame *buildChildInfo(const QString &name, const lab::RawFrame &frame, int start, int length);

  ProtocolFrame *buildChildInfo(const QString &name, const QVariant &info, int flags, int start, int length);

  QByteArray toByteArray(const lab::RawFrame &frame, int from = 0, int length = INT32_MAX);

  QString toString(const QByteArray &array);
};

#endif //PARSER_H

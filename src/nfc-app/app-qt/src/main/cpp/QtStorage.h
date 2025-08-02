/*

This file is part of NFC-LABORATORY.

  Copyright (C) 2025 Jose Vicente Campos Martinez, <josevcm@gmail.com>

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

#ifndef APP_QTSTORAGE_H
#define APP_QTSTORAGE_H

#include <QObject>
#include <QSettings>
#include <QSharedPointer>

namespace hw {
class SignalBuffer;
}

namespace lab {
class RawFrame;
}

class QtStorage : public QObject
{
      Q_OBJECT

      struct Impl;

   public:

      explicit QtStorage();

   public slots:

      void append(const lab::RawFrame &frame);

      void append(const hw::SignalBuffer &buffer);

      void clear();

      long frames() const;

      long samples() const;

   private:

      QSharedPointer<Impl> impl;
};

#endif //QTSTORAGE_H

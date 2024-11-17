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

#ifndef NFC_LAB_FREQUENCYWIDGET_H
#define NFC_LAB_FREQUENCYWIDGET_H

#include "AbstractPlotWidget.h"

namespace hw {
class SignalBuffer;
}

class FrequencyWidget : public AbstractPlotWidget
{
   Q_OBJECT

      struct Impl;

   public:

      explicit FrequencyWidget(QWidget *parent = nullptr);

      void setCenterFreq(long value);

      void setSampleRate(long value);

      void update(const hw::SignalBuffer &buffer);

      void start() override;

      void stop() override;

      void clear() override;

   protected:

      QCPRange selectByUser() override;

      QCPRange selectByRect(const QRect &rect) override;

      QCPRange rangeFilter(const QCPRange &newRange) override;

      QCPRange scaleFilter(const QCPRange &newScale) override;

   private:

      QSharedPointer<Impl> impl;
};


#endif //NFC_LAB_FREQUENCYWIDGET_H

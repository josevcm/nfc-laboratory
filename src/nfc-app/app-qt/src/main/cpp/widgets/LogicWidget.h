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

#ifndef APP_LOGICWIDGET_H
#define APP_LOGICWIDGET_H

#include <graph/ChannelStyle.h>

#include "AbstractPlotWidget.h"

namespace hw {
class SignalBuffer;
}

class StreamModel;

class LogicWidget : public AbstractPlotWidget
{
      Q_OBJECT

      struct Impl;

   public:

   public:

      explicit LogicWidget(QWidget *parent = nullptr);

      void addChannel(int id, const ChannelStyle &style);

      void setModel(StreamModel *streamModel);

      bool hasData() const override;

      void append(const hw::SignalBuffer &buffer);

      void refresh() override;

      void clear() override;

      void start() override;

      void stop() override;

   protected:

      QCPRange selectByUser() override;

      QCPRange selectByRect(const QRect &rect) override;

      QCPRange rangeFilter(const QCPRange &newRange) override;

      QCPRange scaleFilter(const QCPRange &newScale) override;

   signals:

      void toggleChannel(int channel, bool enabled);

   private:

      QSharedPointer<Impl> impl;
};

#endif

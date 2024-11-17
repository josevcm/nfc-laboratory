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

#ifndef NFC_LAB_FRAMEGRAPH_H
#define NFC_LAB_FRAMEGRAPH_H

#include <QByteArray>

#include <lab/data/RawFrame.h>

bool qIsNaN(const QByteArray &value);

#include <3party/customplot/QCustomPlot.h>

#include "FrameData.h"
#include "ChannelStyle.h"

typedef QCPDataContainer<FrameData> QCPDigitalDataContainer;

class FrameGraph : public QCPAbstractPlottable
{
      Q_OBJECT

      struct Impl;

   public:

      typedef std::function<ChannelStyle(int key)> StyleMapper;
      typedef std::function<QString(const FrameData &data)> ValueMapper;

   public:

      explicit FrameGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);

      void setStyle(int key, const ChannelStyle &style);

      void setLegend(int key, const QString &text, int styleKey);

      void setMapper(ValueMapper value, StyleMapper style) const;

      void setOffset(double offset);

      QSharedPointer<QCPDigitalDataContainer> data() const;

      int dataCount() const;

      virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details = nullptr) const;

      virtual QCPRange getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain = QCP::sdBoth) const;

      virtual QCPRange getValueRange(bool &foundRange, QCP::SignDomain inSignDomain = QCP::sdBoth, const QCPRange &inKeyRange = QCPRange()) const;

      virtual void draw(QCPPainter *painter);

      virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const;

   signals:

      void legendClicked(int key);

   private:

      QSharedPointer<Impl> impl;

};

#endif

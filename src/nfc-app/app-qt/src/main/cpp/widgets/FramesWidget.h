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

#ifndef NFC_LAB_FRAMESWIDGET_H
#define NFC_LAB_FRAMESWIDGET_H

#include "AbstractPlotWidget.h"

namespace lab {
class RawFrame;
}

class StreamModel;

class FramesWidget : public AbstractPlotWidget
{
   Q_OBJECT

      struct Impl;

   public:

      enum Channel
      {
         Nfc,
         Iso
      };

      enum Protocol
      {
         NfcA,
         NfcB,
         NfcF,
         NfcV,
         ISO7816
      };

      enum Type
      {
         // NFC frame types
         NfcSilence,
         NfcCarrier,
         NfcARequest,
         NfcAResponse,
         NfcBRequest,
         NfcBResponse,
         NfcFRequest,
         NfcFResponse,
         NfcVRequest,
         NfcVResponse,

         // ISO frame types
         IsoSilence,
         IsoVccOff,
         IsoResetOn,
         IsoStartup,
         IsoRequest,
         IsoResponse,
         IsoExchange
      };

   public:

      explicit FramesWidget(QWidget *parent = nullptr);

      void setModel(StreamModel *streamModel);

      void refresh() override;

      void clear() override;

      void setChannel(Channel channel, bool enabled);

      void setProtocol(Protocol proto, bool enabled);

      bool hasProtocol(Protocol proto);

   signals:

      void toggleChannel(Channel channel, bool enabled);

      void toggleProtocol(Protocol proto, bool enabled);

   protected:

      QCPRange selectByUser() override;

      QCPRange selectByRect(const QRect &rect) override;

      QCPRange rangeFilter(const QCPRange &newRange) override;

      QCPRange scaleFilter(const QCPRange &newScale) override;

   private:

      QSharedPointer<Impl> impl;

};


#endif

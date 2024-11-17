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

#include <3party/customplot/QCustomPlot.h>

#include <lab/nfc/Nfc.h>
#include <lab/data/RawFrame.h>

#include <model/StreamModel.h>

#include <graph/FrameData.h>
#include <graph/FrameGraph.h>

#include <styles/Theme.h>

#include <format/DataFormat.h>

#include "FramesWidget.h"

struct FramesWidget::Impl
{
   FramesWidget *widget = nullptr;

   QCustomPlot *plot = nullptr;

   // FrameGraph *frameGraph = nullptr;
   // QSharedPointer<QCPDigitalDataContainer> frameData;

   QMap<unsigned int, FrameGraph *> channels;

   QSharedPointer<QCPAxisTickerText> frameTicker;

   StreamModel *streamModel = nullptr;

   QMap<int, bool> enabledChannels;
   QMap<int, bool> enabledProtocols;

   QMetaObject::Connection rowsInsertedConnection;
   QMetaObject::Connection modelResetConnection;
   QMetaObject::Connection nfcLegendClickConnection;
   QMetaObject::Connection isoLegendClickConnection;

   explicit Impl(FramesWidget *parent) : widget(parent), plot(widget->plot()), frameTicker(new QCPAxisTickerText)
   {
      // set ticker for y-axis
      frameTicker->addTick(1, "NFC");
      frameTicker->addTick(2, "ICC");

      // set cursor formatter
      widget->setCursorFormatter(DataFormat::time);
      widget->setRangeFormatter(DataFormat::timeRange);

      // override default axis labels
      widget->plot()->xAxis->grid()->setSubGridVisible(true);
      widget->plot()->yAxis->grid()->setPen(Qt::NoPen);
      widget->plot()->yAxis->setTicker(frameTicker);

      // set space for legend
      widget->plot()->legend->setIconSize(500, 20);

      // setup NFC channel
      channels[Nfc] = new FrameGraph(plot->xAxis, plot->yAxis);
      channels[Nfc]->setOffset(1);
      channels[Nfc]->setSelectable(QCP::stDataRange);
      channels[Nfc]->setSelectionDecorator(nullptr);
      channels[Nfc]->setLegend(NfcA, "NFC-A", NfcARequest);
      channels[Nfc]->setLegend(NfcB, "NFC-B", NfcBRequest);
      channels[Nfc]->setLegend(NfcF, "NFC-F", NfcFRequest);
      channels[Nfc]->setLegend(NfcV, "NFC-V", NfcVRequest);
      channels[Nfc]->setMapper([=](const FrameData &data) { return nfcValue(data); }, [=](int key) { return nfcStyle(key); });
      channels[Nfc]->setOffset(static_cast<double>(channels.size()));

      // set channel ticker
      frameTicker->addTick(static_cast<double>(channels.size()), "NFC");

      // setup ISO channel
      channels[Iso] = new FrameGraph(plot->xAxis, plot->yAxis);
      channels[Iso]->setSelectable(QCP::stDataRange);
      channels[Iso]->setSelectionDecorator(nullptr);
      channels[Iso]->setLegend(ISO7816, "ISO-7816", IsoRequest);
      channels[Iso]->setMapper([=](const FrameData &data) { return isoValue(data); }, [=](int key) { return isoStyle(key); });
      channels[Iso]->setOffset(static_cast<double>(channels.size()));

      // set channel ticker
      frameTicker->addTick(static_cast<double>(channels.size()), "ICC");

      // connect NFC legend click
      nfcLegendClickConnection = connect(channels[Nfc], &FrameGraph::legendClicked, [=](int key) {
         toggleProtocol(key);
      });

      // connect ISO legend click
      isoLegendClickConnection = connect(channels[Iso], &FrameGraph::legendClicked, [=](int key) {
         toggleProtocol(key);
      });

      // update scale
      widget->setDataScale(0, static_cast<double>(channels.size()) + 1);
      widget->setViewScale(0, static_cast<double>(channels.size()) + 1);
   }

   ~Impl()
   {
      disconnect(rowsInsertedConnection);
      disconnect(modelResetConnection);
      disconnect(nfcLegendClickConnection);
      disconnect(isoLegendClickConnection);
   }

   QString nfcValue(const FrameData &data) const
   {
      switch (data.type)
      {
         case NfcSilence:
            return {};
         case NfcCarrier:
            return tr("Carrier");
         default:
            return data.data.toHex(' ').toUpper();
      }
   }

   ChannelStyle nfcStyle(int key) const
   {
      // set channel styles
      switch (key)
      {
         case NfcSilence:
            return {Theme::defaultCarrierPen, Theme::defaultCarrierPen, Theme::defaultCarrierBrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case NfcCarrier:
            return {Theme::defaultCarrierPen, Theme::defaultCarrierPen, Theme::defaultCarrierBrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case NfcARequest:
            return {Theme::defaultNfcAPen, Theme::defaultNfcAPen, Theme::defaultNfcABrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case NfcAResponse:
            return {Theme::defaultNfcAPen, Theme::defaultNfcAPen, Theme::responseNfcABrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case NfcBRequest:
            return {Theme::defaultNfcBPen, Theme::defaultNfcBPen, Theme::defaultNfcBBrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case NfcBResponse:
            return {Theme::defaultNfcBPen, Theme::defaultNfcBPen, Theme::responseNfcBBrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case NfcFRequest:
            return {Theme::defaultNfcFPen, Theme::defaultNfcFPen, Theme::defaultNfcFBrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case NfcFResponse:
            return {Theme::defaultNfcFPen, Theme::defaultNfcFPen, Theme::responseNfcFBrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case NfcVRequest:
            return {Theme::defaultNfcVPen, Theme::defaultNfcVPen, Theme::defaultNfcVBrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case NfcVResponse:
            return {Theme::defaultNfcVPen, Theme::defaultNfcVPen, Theme::responseNfcVBrush, Theme::defaultTextPen, Theme::monospaceTextFont};
      }

      return {};
   }

   QString isoValue(const FrameData &data) const
   {
      switch (data.type)
      {
         default:
            return data.data.toHex(' ').toUpper();
      }
   }

   ChannelStyle isoStyle(int key) const
   {
      // set channel styles
      switch (key)
      {
         case IsoSilence:
            return {Theme::defaultCarrierPen, Theme::defaultCarrierPen, Theme::defaultCarrierBrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case IsoVccOff:
            return {Theme::defaultCarrierPen, Theme::defaultCarrierPen, Theme::defaultCarrierBrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case IsoResetOn:
            return {Theme::defaultCarrierPen, Theme::defaultCarrierPen, Theme::defaultCarrierBrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case IsoStartup:
            return {Theme::defaultNfcFPen, Theme::defaultNfcFPen, Theme::defaultNfcFBrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case IsoRequest:
            return {Theme::defaultNfcAPen, Theme::defaultNfcAPen, Theme::defaultNfcABrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case IsoResponse:
            return {Theme::defaultNfcAPen, Theme::defaultNfcAPen, Theme::responseNfcABrush, Theme::defaultTextPen, Theme::monospaceTextFont};

         case IsoExchange:
            return {Theme::defaultNfcAPen, Theme::defaultNfcAPen, Theme::defaultNfcABrush, Theme::defaultTextPen, Theme::monospaceTextFont};
      }

      return {};
   }

   void toggleChannel(int key)
   {
      setChannel(static_cast<Channel>(key), !enabledChannels[key]);

      widget->toggleChannel(static_cast<Channel>(key), enabledChannels[key]);
   }

   void toggleProtocol(int key)
   {
      setProtocol(static_cast<Protocol>(key), !enabledProtocols[key]);

      widget->toggleProtocol(static_cast<Protocol>(key), enabledProtocols[key]);
   }

   void changeModel(StreamModel *model)
   {
      streamModel = model;

      if (streamModel)
      {
         disconnect(rowsInsertedConnection);
         disconnect(modelResetConnection);

         rowsInsertedConnection = connect(streamModel, &QAbstractItemModel::rowsInserted, [=](const QModelIndex &parent, int first, int last) {
            rowsInserted(parent, first, last);
         });

         modelResetConnection = connect(streamModel, &QAbstractItemModel::modelReset, [=]() {
            modelReset();
         });
      }
   }

   void clear()
   {
      for (auto channels: channels)
      {
         channels->data()->clear();
         channels->selection().clear();
      }
   }

   void refresh()
   {
   }

   void rowsInserted(const QModelIndex &parent, int first, int last)
   {
      for (int row = first; row <= last; row++)
      {
         QModelIndex index = streamModel->index(row, 0, parent);

         if (const lab::RawFrame *frame = streamModel->frame(index))
         {
            switch (frame->techType())
            {
               case lab::NfcAnyTech:
               case lab::NfcATech:
               case lab::NfcBTech:
               case lab::NfcFTech:
               case lab::NfcVTech:
                  addNfcFrame(frame);
                  break;

               case lab::IsoAnyTech:
               case lab::Iso7816Tech:
                  addIsoFrame(frame);
                  break;
            }
         }
      }

      // update graph
      double rangeStart = qInf();
      double rangeEnd = qInf();

      for (auto channel: channels)
      {
         if (channel->data()->size() > 0)
         {
            if (rangeStart == qInf() || rangeStart > channel->data()->at(0)->start)
               rangeStart = channel->data()->at(0)->start;

            if (rangeEnd == qInf() || rangeEnd < channel->data()->at(channel->data()->size() - 1)->end)
               rangeEnd = channel->data()->at(channel->data()->size() - 1)->end;
         }
      }

      if (rangeStart != qInf() && rangeEnd != qInf())
         widget->setDataRange(rangeStart, rangeEnd);
   }

   void addNfcFrame(const lab::RawFrame *frame)
   {
      QSharedPointer<QCPDigitalDataContainer> frameData = channels[Nfc]->data();

      QCPDigitalDataContainer::iterator previous = frameData->end();

      if (previous != frameData->begin())
         --previous;

      // add carrier / silence segment
      if (frame->frameType() == lab::FrameType::NfcCarrierOn || frame->frameType() == lab::FrameType::NfcCarrierOff)
      {
         if (frame->frameType() == lab::FrameType::NfcCarrierOn)
         {
            // initialize first carrier segment
            if (previous == frameData->end())
            {
               frameData->add(FrameData(NfcCarrier, NfcCarrier, frame->timeStart(), frame->timeEnd(), 20));
            }

            // add previous carrier segment
            else if (previous->type >= NfcARequest)
            {
               frameData->add(FrameData(NfcCarrier, NfcCarrier, previous->end, frame->timeStart(), 20));
            }

            // update previous carrier segment
            else
            {
               previous->end = frame->timeStart();

               // mark new carrier segment
               frameData->add(FrameData(NfcCarrier, NfcCarrier, frame->timeStart(), frame->timeEnd(), 20));
            }
         }
         else
         {
            // add first event as silence
            if (previous == frameData->end())
            {
               frameData->add(FrameData(NfcSilence, NfcSilence, frame->timeStart(), frame->timeEnd(), 0));
            }

            // add previous carrier segment
            else if (previous->type >= NfcARequest)
            {
               frameData->add(FrameData(NfcCarrier, NfcCarrier, previous->end, frame->timeStart(), 20));

               // mark new silence segment
               frameData->add(FrameData(NfcSilence, NfcSilence, frame->timeStart(), frame->timeEnd(), 0));
            }

            // update previous silence / carrier segment
            else
            {
               previous->end = frame->timeStart();

               // mark new silence segment
               frameData->add(FrameData(NfcSilence, NfcSilence, frame->timeStart(), frame->timeEnd(), 0));
            }
         }
      }

      // add NFC frame
      else if (frame->frameType() == lab::FrameType::NfcPollFrame || frame->frameType() == lab::FrameType::NfcListenFrame)
      {
         int type = -1;

         switch (frame->techType())
         {
            case lab::FrameTech::NfcATech:
               type = frame->frameType() == lab::FrameType::NfcPollFrame ? NfcARequest : NfcAResponse;
               break;
            case lab::FrameTech::NfcBTech:
               type = frame->frameType() == lab::FrameType::NfcPollFrame ? NfcBRequest : NfcBResponse;
               break;
            case lab::FrameTech::NfcFTech:
               type = frame->frameType() == lab::FrameType::NfcPollFrame ? NfcFRequest : NfcFResponse;
               break;
            case lab::FrameTech::NfcVTech:
               type = frame->frameType() == lab::FrameType::NfcPollFrame ? NfcVRequest : NfcVResponse;
               break;
         }

         // update previous carrier segment
         if (previous != frameData->end())
         {
            if (previous->type >= NfcARequest)
            {
               frameData->add(FrameData(NfcCarrier, NfcCarrier, previous->end, frame->timeStart(), 20));
            }
            else
            {
               previous->end = frame->timeStart();
            }
         }

         // add new frame
         frameData->add(FrameData(type, type, frame->timeStart(), frame->timeEnd(), 24, toByteArray(*frame)));
      }
   }

   void addIsoFrame(const lab::RawFrame *frame)
   {
      QSharedPointer<QCPDigitalDataContainer> frameData = channels[Iso]->data();

      // add first event as silence
      if (frameData->isEmpty())
         frameData->add(FrameData(IsoSilence, IsoSilence, 0, 0, 0));

      QCPDigitalDataContainer::iterator previous = frameData->end();

      // get previous segment
      if (previous != frameData->begin())
         --previous;

      // if previous segment is not silence, add new silence segment between frames
      if (previous->type >= IsoStartup)
         frameData->add(FrameData(IsoSilence, IsoSilence, previous->end, frame->timeStart(), 0));

         // or increase previous silence segment
      else
         previous->end = frame->timeStart();

      switch (frame->frameType())
      {
         case lab::FrameType::IsoATRFrame:
            frameData->add(FrameData(IsoStartup, IsoStartup, frame->timeStart(), frame->timeEnd(), 24, toByteArray(*frame)));
            break;

         case lab::FrameType::IsoRequestFrame:
            frameData->add(FrameData(IsoRequest, IsoRequest, frame->timeStart(), frame->timeEnd(), 24, toByteArray(*frame)));
            break;

         case lab::FrameType::IsoResponseFrame:
            frameData->add(FrameData(IsoResponse, IsoResponse, frame->timeStart(), frame->timeEnd(), 24, toByteArray(*frame)));
            break;

         case lab::FrameType::IsoExchangeFrame:
            frameData->add(FrameData(IsoExchange, IsoExchange, frame->timeStart(), frame->timeEnd(), 24, toByteArray(*frame)));
            break;
      }

      // add new frame
   }

   void modelReset() const
   {
      widget->clear();
   }

   QCPRange selectByUser() const
   {
      // get current selection on any channel
      for (auto channel: channels)
      {
         QCPDataSelection selection = channel->selection();

         // for empty selection no further action
         if (selection.isEmpty())
            continue;

         double startTime = channel->data()->at(selection.span().begin())->start;
         double endTime = channel->data()->at(selection.span().end() - 1)->end;

         return {startTime, endTime};
      }

      return {};
   }

   QCPRange selectByRect(const QRect &rect) const
   {
      // get current selection pn any channel
      for (auto channel: channels)
      {
         // clear current selection
         channel->setSelection(QCPDataSelection());

         if (rect.isEmpty())
            continue;

         // transport rect start / end to start / end in plot coordinates
         double rectStart = widget->plot()->xAxis->pixelToCoord(rect.left());
         double rectEnd = widget->plot()->xAxis->pixelToCoord(rect.right());

         // get start / end index of data to be selected
         int startIndex = static_cast<int>(channel->data()->findBegin(rectStart, false) - channel->data()->constBegin());
         int endIndex = static_cast<int>(channel->data()->findEnd(rectEnd, false) - channel->data()->constBegin() - 1);

         // only select events fully contained inside rect selection
         if (startIndex >= endIndex)
            return {};

         // finally get start / end for final selection
         double startTime = channel->data()->at(startIndex)->start;
         double endTime = channel->data()->at(endIndex - 1)->end;

         // select data in graph
         channel->setSelection(QCPDataSelection({startIndex, endIndex}));

         return {startTime, endTime};
      }

      return {};
   }

   QCPRange rangeFilter(const QCPRange &newRange) const
   {
      return newRange;
   }

   QCPRange scaleFilter(const QCPRange &newScale) const
   {
      // return {0, static_cast<double>(channels.size()) + 1};
      return {0, 3};
   }

   static QByteArray toByteArray(const lab::RawFrame &frame)
   {
      QByteArray buffer;

      for (int i = 0; i < frame.limit(); i++)
      {
         buffer.append(frame[i]);
      }

      return buffer;
   }

   void setChannel(Channel channel, bool enabled)
   {
      enabledChannels[channel] = enabled;

      switch (channel)
      {
         case Nfc:
            channels[Nfc]->setLegend(NfcA, "NFC-A", enabled ? NfcARequest : NfcSilence);
            channels[Nfc]->setLegend(NfcB, "NFC-B", enabled ? NfcBRequest : NfcSilence);
            channels[Nfc]->setLegend(NfcF, "NFC-F", enabled ? NfcFRequest : NfcSilence);
            channels[Nfc]->setLegend(NfcV, "NFC-V", enabled ? NfcVRequest : NfcSilence);
            break;
      }
   }

   void setProtocol(Protocol proto, bool enabled)
   {
      enabledProtocols[proto] = enabled;

      switch (proto)
      {
         case NfcA:
            channels[Nfc]->setLegend(NfcA, "NFC-A", enabled ? NfcARequest : NfcSilence);
            break;
         case NfcB:
            channels[Nfc]->setLegend(NfcB, "NFC-B", enabled ? NfcBRequest : NfcSilence);
            break;
         case NfcF:
            channels[Nfc]->setLegend(NfcF, "NFC-F", enabled ? NfcFRequest : NfcSilence);
            break;
         case NfcV:
            channels[Nfc]->setLegend(NfcV, "NFC-V", enabled ? NfcVRequest : NfcSilence);
            break;
         case ISO7816:
            channels[Iso]->setLegend(ISO7816, "ISO-7816", enabled ? IsoRequest : IsoSilence);
         break;
      }

      widget->plot()->replot();
   }
};

FramesWidget::FramesWidget(QWidget *parent) : AbstractPlotWidget(parent), impl(new Impl(this))
{
}

void FramesWidget::setModel(StreamModel *model)
{
   impl->changeModel(model);
}

void FramesWidget::clear()
{
   impl->clear();

   AbstractPlotWidget::clear();
}

void FramesWidget::refresh()
{
   impl->refresh();

   AbstractPlotWidget::refresh();
}

QCPRange FramesWidget::selectByUser()
{
   return impl->selectByUser();
}

QCPRange FramesWidget::selectByRect(const QRect &rect)
{
   return impl->selectByRect(rect);
}

QCPRange FramesWidget::rangeFilter(const QCPRange &newRange)
{
   return AbstractPlotWidget::rangeFilter(newRange);
}

QCPRange FramesWidget::scaleFilter(const QCPRange &newScale)
{
   return impl->scaleFilter(newScale);
}

void FramesWidget::setChannel(Channel channel, bool enabled)
{
   impl->setChannel(channel, enabled);
}

void FramesWidget::setProtocol(Protocol proto, bool enabled)
{
   impl->setProtocol(proto, enabled);
}

bool FramesWidget::hasProtocol(Protocol proto)
{
   return impl->enabledProtocols[proto];
}

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

#include <QMap>
#include <QList>
#include <QVBoxLayout>

#include <3party/customplot/QCustomPlot.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <lab/data/RawFrame.h>

#include <graph/AxisLabel.h>
#include <graph/ChannelGraph.h>
#include <graph/MarkerRibbon.h>
#include <graph/MarkerBracket.h>

#include <styles/Theme.h>

#include <format/DataFormat.h>

#include <model/StreamModel.h>

#include "LogicWidget.h"

#define MAX_SIGNAL_BUFFER (512 * 1024 * 1024 / sizeof(QCPGraphData))

struct LogicWidget::Impl
{
   LogicWidget *widget = nullptr;

   QCustomPlot *plot = nullptr;

   StreamModel *streamModel = nullptr;

   QMap<unsigned int, ChannelGraph *> channels;
   QMap<unsigned int, ChannelStyle> channelStyle;

   QSharedPointer<AxisLabel> scaleLabel;
   QSharedPointer<MarkerRibbon> ribbonMarker;
   QSharedPointer<QCPAxisTickerText> logicTicker;

   QList<QSharedPointer<MarkerBracket>> bracketList;

   double height;
   double threshold;

   int maximumEntries = MAX_SIGNAL_BUFFER;

   QMetaObject::Connection rowsInsertedConnection;
   QMetaObject::Connection modelResetConnection;

   explicit Impl(LogicWidget *parent) : widget(parent),
                                        plot(widget->plot()),
                                        scaleLabel(new AxisLabel(plot->yAxis)),
                                        ribbonMarker(new MarkerRibbon(plot)),
                                        logicTicker(new QCPAxisTickerText),
                                        height(0.70),
                                        threshold(0.5)
   {
      // set cursor formatter
      widget->setCursorFormatter(DataFormat::time);
      widget->setRangeFormatter(DataFormat::timeRange);

      // initialize axis properties
      plot->xAxis->grid()->setSubGridVisible(true);

      plot->yAxis->setTicker(logicTicker);
      plot->yAxis->grid()->setPen(Qt::NoPen);
      plot->yAxis->grid()->setSubGridVisible(true);

      // initialize axis label
      scaleLabel->setText("CH", Qt::TopLeftCorner);
      scaleLabel->setVisible(true);

      // initialize legend
      plot->legend->setIconSize(60, 20);
   }

   ~Impl()
   {
      disconnect(rowsInsertedConnection);
      disconnect(modelResetConnection);
   }

   /**
    * @brief Change model
    * @param model Stream model
    */
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

         modelResetConnection = connect(streamModel, &QAbstractItemModel::modelReset, [=] {
            resetModel();
         });
      }
   }

   /**
    * @brief Handle model reset to clear all data
    */
   void resetModel() const
   {
      widget->clear();
   }

   /**
    * @brief Add channel to widget
    * @param id Channel id
    * @param style Channel style
    */
   void addChannel(int id, const ChannelStyle &style)
   {
      if (channels.contains(id))
         return;

      // add channel to map and set parameters
      channels[id] = new ChannelGraph(plot->xAxis, plot->yAxis);
      channels[id]->setStyle(style);
      channels[id]->setPen(style.linePen);
      channels[id]->setLineStyle(QCPGraph::lsStepLeft);
      channels[id]->setSelectable(QCP::stDataRange);
      channels[id]->setSelectionDecorator(nullptr);
      channels[id]->setOffset(static_cast<double>(channels.size()));

      // set channel ticker
      logicTicker->addTick(static_cast<double>(channels.size()), style.text);

      // update scale
      widget->setDataScale(0, static_cast<double>(channels.size()) + 1);
      widget->setViewScale(0, static_cast<double>(channels.size()) + 1);

      // rebuild legend
      plot->legend->clear();

      for (const auto &ch: channels)
      {
         ch->addToLegend();
      }

      // add final legend space
      plot->legend->addElement(new QCPLayoutInset());
      plot->legend->setColumnStretchFactor(plot->legend->itemCount() - 1, 1000);
   }

   /**
    * @brief Check if widget has data
    * @return True if widget has data
    */
   bool hasData() const
   {
      for (const auto &channel: channels)
      {
         if (channel->data()->size() > 0)
            return true;
      }

      return false;
   }

   /**
    * @brief Append signal buffer to graph
    * @param buffer Signal buffer
    */
   void append(const hw::SignalBuffer &buffer)
   {
      if (!buffer.isValid() || !channels.contains(buffer.id()))
         return;

      ChannelGraph *ch = channels[buffer.id()];

      double offset = ch->offset();
      double sampleRate = buffer.sampleRate();
      double sampleStep = 1 / sampleRate;
      double startTime = static_cast<double>(buffer.offset()) / sampleRate;

      switch (buffer.type())
      {
         case hw::SignalType::SIGNAL_TYPE_RAW_LOGIC:
         {
            for (int i = 0; i < buffer.elements(); i++)
            {
               double time = std::fma(sampleStep, i, startTime);
               double value = buffer[i];

               ch->data()->add({time, offset + (value < threshold ? -height / 2 : +height / 2)});
            }

            break;
         }

         case hw::SignalType::SIGNAL_TYPE_ADV_LOGIC:
         {
            for (int i = 0; i < buffer.limit(); i += 2)
            {
               double time = std::fma(sampleStep, buffer[i + 1], startTime);
               double value = buffer[i + 0];

               ch->data()->add({time, offset + (value < threshold ? -height / 2 : +height / 2)});
            }

            break;
         }

         default:
            return;
      }

      // remove old data when maximum memory threshold is reached
      if (ch->data()->size() > maximumEntries)
         ch->data()->removeBefore(ch->data()->at(ch->data()->size() - maximumEntries)->key);

      // update graph
      widget->setDataRange(ch->data()->at(0)->key, ch->data()->at(ch->data()->size() - 1)->key);
   }

   /**
    * @brief Clear all data
    */
   void clear()
   {
      // clear all markers
      bracketList.clear();
      ribbonMarker->clear();

      // clear graph data
      for (const auto &channel: channels)
      {
         channel->data()->clear();
         channel->selection().clear();
      }

      widget->setDataRange(0, 1E-6);

      plot->replot();
   }

   /**
    * @brief Refresh widget
    */
   void refresh() const
   {
      //      double signalLowerRange = widget->dataLowerRange();
      //      double signalUpperRange = widget->dataUpperRange();
      //
      //      // statistics
      //      qDebug().noquote() << "total samples" << signalData->size() << "adaptive compression ratio" << QString("%1%").arg(float(signalData->size() / ((signalUpperRange - signalLowerRange) * 10E4)), 0, 'f', 2);
   }

   void rowsInserted(const QModelIndex &parent, int first, int last)
   {
      for (int row = first; row <= last; row++)
      {
         QModelIndex index = streamModel->index(row, StreamModel::Event, parent);

         if (const lab::RawFrame *frame = streamModel->frame(index))
         {
            // only process logic frames
            if (frame->techType() != lab::FrameTech::Iso7816Tech)
               continue;

            QString eventName = streamModel->data(index, Qt::DisplayRole).toString();

            // add bracket marker
            if (!eventName.isEmpty())
            {
               for (auto channel: channels)
               {
                  if (channel->style().text != "IO")
                     continue;

                  double maxValue = 0;

                  // detect maximum frame value
                  for (auto it = channel->data()->findBegin(frame->timeStart()); it != channel->data()->findEnd(frame->timeEnd()); ++it)
                  {
                     if (it->value > maxValue)
                        maxValue = it->value;
                  }

                  QSharedPointer<MarkerBracket> bracketMarker(new MarkerBracket(widget->plot()));

                  bracketMarker->setLeft(QPointF(frame->timeStart(), maxValue));
                  bracketMarker->setRight(QPointF(frame->timeEnd(), maxValue));
                  bracketMarker->setText(eventName);

                  bracketList.append(bracketMarker);
               }
            }

            // add frame tech to ribbon marker
            QString techName;
            QColor techColor;

            switch (frame->techType())
            {
               case lab::FrameTech::Iso7816Tech:
                  techName = "ISO 7816";
                  techColor = Theme::defaultLogicIOColor;
                  break;
            }

            if (!techName.isEmpty())
            {
               techColor.setAlpha(0xE0);
               ribbonMarker->addRange(frame->timeStart(), frame->timeEnd(), techName, QPen(techColor), QBrush(techColor));
            }
         }
      }
   }

   /**
    * @brief Detect selected data by user and adjust to frames
    * @return Selected range
    */
   QCPRange selectByUser() const
   {
      // get current selection on any channel
      for (auto channel: channels)
      {
         QCPDataSelection selection = channel->selection();

         // for empty selection no further action
         if (selection.isEmpty())
            continue;

         // get selection size and start / end
         double selectStart = channel->data()->at(selection.span().begin() + 1)->key;
         double selectEnd = channel->data()->at(selection.span().end() - 1)->key;

         // begin with full data
         double rangeStart = channel->data()->at(0)->key;
         double rangeEnd = channel->data()->at(channel->data()->size() - 1)->key;

         // adjust to frames
         for (QModelIndex modelIndex: streamModel->modelRange(rangeStart, rangeEnd))
         {
            if (const lab::RawFrame *frame = streamModel->frame(modelIndex))
            {
               // only process logic frames
               if (frame->techType() != lab::FrameTech::Iso7816Tech)
                  continue;

               if (frame->timeStart() <= selectStart && frame->timeEnd() >= selectStart)
               {
                  if (frame->timeStart() > rangeStart)
                     rangeStart = frame->timeStart();
               }
               else if (frame->timeEnd() < selectStart)
               {
                  if (frame->timeEnd() > rangeStart)
                     rangeStart = frame->timeEnd();
               }

               if (frame->timeStart() <= selectEnd && frame->timeEnd() >= selectEnd)
               {
                  if (frame->timeEnd() < rangeEnd)
                     rangeEnd = frame->timeEnd();
               }
               else if (frame->timeStart() > selectEnd)
               {
                  if (frame->timeStart() < rangeEnd)
                     rangeEnd = frame->timeStart();
               }
            }
         }

         // no frame selected
         if (rangeStart > rangeEnd)
            continue;

         // select full data frames
         int startIndex = int(channel->data()->findBegin(rangeStart, false) - channel->data()->constBegin() + 1);
         int endIndex = int(channel->data()->findEnd(rangeEnd, false) - channel->data()->constBegin() - 1);
         channel->setSelection(QCPDataSelection({startIndex, endIndex}));

         return {rangeStart, rangeEnd};
      }

      return {};
   }

   /**
 * @brief Detect selected data by rect and adjust to frames
 * @param rect Selection rect
 * @return Selected range
 */
   QCPRange selectByRect(const QRect &rect) const
   {
      // get current selection pn any channel
      for (auto channel: channels)
      {
         // clear current selection
         channel->setSelection(QCPDataSelection());

         // transport rect start / end to start / end in plot coordinates
         double rectStart = widget->plot()->xAxis->pixelToCoord(rect.left());
         double rectEnd = widget->plot()->xAxis->pixelToCoord(rect.right());

         // get start / end index of data to be selected
         int startIndex = static_cast<int>(channel->data()->findBegin(rectStart, false) - channel->data()->constBegin());
         int endIndex = static_cast<int>(channel->data()->findEnd(rectEnd, false) - channel->data()->constBegin() - 1);

         // only select events fully contained inside rect selection
         if (startIndex >= endIndex)
            continue;

         // finally get start / end for final selection
         double startTime = channel->data()->at(startIndex)->key;
         double endTime = channel->data()->at(endIndex)->key;

         // select data in graph
         channel->setSelection(QCPDataSelection({startIndex, endIndex}));

         return {startTime, endTime};
      }

      return {};
   }

   /**
    * @param Apply limits to new scale
    * @return
    */
   QCPRange scaleFilter(const QCPRange &newScale) const
   {
      return {0, static_cast<double>(channels.size() + 1)};
   }

   void dump() const
   {
      for (const auto &ch: channels)
      {
         qInfo() << "logic channel" << ch->style().text << "samples" << ch->data()->size();
      }
   }
};

LogicWidget::LogicWidget(QWidget *parent) : AbstractPlotWidget(parent), impl(new Impl(this))
{
}

void LogicWidget::addChannel(int id, const ChannelStyle &style)
{
   impl->addChannel(id, style);
}

void LogicWidget::setModel(StreamModel *model)
{
   impl->changeModel(model);
}

bool LogicWidget::hasData() const
{
   return impl->hasData();
}

void LogicWidget::append(const hw::SignalBuffer &buffer)
{
   impl->append(buffer);
}

void LogicWidget::start()
{
}

void LogicWidget::stop()
{
   impl->dump();
}

void LogicWidget::clear()
{
   impl->clear();

   AbstractPlotWidget::clear();
}

void LogicWidget::refresh()
{
   impl->refresh();

   AbstractPlotWidget::refresh();
}

QCPRange LogicWidget::selectByUser()
{
   return impl->selectByUser();
}

QCPRange LogicWidget::selectByRect(const QRect &rect)
{
   return impl->selectByRect(rect);
}

QCPRange LogicWidget::rangeFilter(const QCPRange &newRange)
{
   return AbstractPlotWidget::rangeFilter(newRange);
}

QCPRange LogicWidget::scaleFilter(const QCPRange &newScale)
{
   return impl->scaleFilter(newScale);
}

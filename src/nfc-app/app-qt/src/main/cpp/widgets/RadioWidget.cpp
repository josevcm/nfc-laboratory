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

#include <QList>
#include <QVBoxLayout>

#include <3party/customplot/QCustomPlot.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <lab/data/RawFrame.h>

#include <graph/AxisLabel.h>
#include <graph/MarkerRibbon.h>
#include <graph/MarkerBracket.h>
#include <graph/ChannelGraph.h>

#include <styles/Theme.h>

#include <format/DataFormat.h>

#include <model/StreamModel.h>

#include "RadioWidget.h"

#define MAX_SIGNAL_BUFFER (512 * 1024 * 1024 / sizeof(QCPGraphData))

struct RadioWidget::Impl
{
   RadioWidget *widget = nullptr;

   QCustomPlot *plot = nullptr;
   ChannelGraph *radioGraph = nullptr;
   QSharedPointer<QCPGraphDataContainer> signalData;

   StreamModel *streamModel = nullptr;

   QSharedPointer<AxisLabel> scaleLabel;
   QSharedPointer<MarkerRibbon> ribbonMarker;

   QList<QSharedPointer<MarkerBracket>> bracketList;

   int maximumEntries = MAX_SIGNAL_BUFFER;

   QMetaObject::Connection rowsInsertedConnection;
   QMetaObject::Connection modelResetConnection;

   explicit Impl(RadioWidget *parent) : widget(parent),
                                        plot(widget->plot()),
                                        radioGraph(new ChannelGraph(plot->xAxis, plot->yAxis)),
                                        ribbonMarker(new MarkerRibbon(plot))
   {
      // set cursor formatter
      widget->setCursorFormatter(DataFormat::time);
      widget->setRangeFormatter(DataFormat::timeRange);

      // set plot properties
      plot->xAxis->grid()->setSubGridVisible(true);

      // initialize axis label
      scaleLabel.reset(new AxisLabel(plot->yAxis));
      scaleLabel->setText("RMS", Qt::TopLeftCorner);
      scaleLabel->setVisible(true);

      // initialize signal graph
      radioGraph->setPen(Theme::defaultSignalPen);
      radioGraph->setSelectable(QCP::stDataRange);
      radioGraph->setSelectionDecorator(nullptr);
      radioGraph->setStyle({Theme::defaultSignalPen, Theme::defaultRadioNFCPen, Theme::defaultRadioNFCBrush, Theme::defaultTextPen, Theme::defaultLabelFont, "NFC"});

      // initialize legend
      plot->legend->setIconSize(60, 20);

      // add spacer to legend and stretch last item to fill the remaining space
      plot->legend->addElement(new QCPLayoutInset());
      plot->legend->setColumnStretchFactor(plot->legend->itemCount() - 1, 1000);

      // get signal storage backend
      signalData = radioGraph->data();
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

         modelResetConnection = connect(streamModel, &QAbstractItemModel::modelReset, [=]() {
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
    * @brief Check if signal data is available
    * @return True if signal data is available
    */
   bool hasData() const
   {
      return !signalData->isEmpty();
   }

   /**
    * @brief Append signal buffer to graph
    * @param buffer Signal buffer
    */
   void append(const hw::SignalBuffer &buffer) const
   {
      if (!buffer.isValid())
         return;

      double sampleRate = buffer.sampleRate();
      double sampleStep = 1 / sampleRate;
      double startTime = buffer.offset() / sampleRate;

      switch (buffer.type())
      {
         case hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES:
         {
            signalData->removeAfter(startTime);

            for (int i = 0; i < buffer.elements(); i++)
            {
               double value = buffer[i] * 2;
               double time = std::fma(sampleStep, i, startTime); // range = sampleStep * i + startTime

               signalData->add({time, value});
            }

            break;
         }

         case hw::SignalType::SIGNAL_TYPE_RADIO_SIGNAL:
         {
            signalData->removeAfter(startTime);

            for (int i = 0; i < buffer.limit(); i += 2)
            {
               double value = buffer[i + 0] * 2;
               double time = std::fma(sampleStep, buffer[i + 1], startTime); // time = sampleStep * buffer[i + 1] + startTime

               signalData->add({time, value});
            }

            break;
         }
      }

      // remove old data when maximum memory threshold is reached
      if (signalData->size() > maximumEntries)
         signalData->removeBefore(signalData->at(signalData->size() - maximumEntries)->key);

      // update graph
      widget->setDataRange(signalData->at(0)->key, signalData->at(signalData->size() - 1)->key);
      widget->setDataScale(0, 1);
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
      signalData->clear();
      radioGraph->selection().clear();

      // restore data range
      widget->setDataRange(0, 1E-6);
   }

   /**
    * @brief Refresh widget
    */
   void refresh() const
   {
      double signalLowerRange = widget->dataLowerRange();
      double signalUpperRange = widget->dataUpperRange();

      // statistics
      qDebug().noquote() << "total samples" << signalData->size() << "adaptive compression ratio" << QString("%1%").arg(float(signalData->size() / ((signalUpperRange - signalLowerRange) * 10E4)), 0, 'f', 2);
   }

   /**
    * @brief Handle new rows inserted to add bracket markers
    * @param parent Parent index
    * @param first First row
    * @param last Last row
    */
   void rowsInserted(const QModelIndex &parent, int first, int last)
   {
      for (int row = first; row <= last; row++)
      {
         QModelIndex index = streamModel->index(row, StreamModel::Event, parent);

         if (const lab::RawFrame *frame = streamModel->frame(index))
         {
            // only NFC frames
            if (frame->techType() != lab::FrameTech::NfcATech && frame->techType() != lab::FrameTech::NfcBTech && frame->techType() != lab::FrameTech::NfcFTech && frame->techType() != lab::FrameTech::NfcVTech)
               continue;

            // ignore carrier frames
            if (frame->frameType() == lab::FrameType::NfcCarrierOn || frame->frameType() == lab::FrameType::NfcCarrierOff)
               continue;

            QString eventName = streamModel->data(index, Qt::DisplayRole).toString();

            // add bracket marker
            if (!eventName.isEmpty())
            {
               double maxValue = 0;

               // detect maximum frame value
               for (auto it = signalData->findBegin(frame->timeStart()); it != signalData->findEnd(frame->timeEnd()); ++it)
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

            // add frame tech to ribbon marker
            QString techName;
            QColor techColor;

            switch (frame->techType())
            {
               case lab::FrameTech::NfcATech:
                  techName = "ISO 14443-A";
                  techColor = Theme::defaultNfcAColor;
                  break;

               case lab::FrameTech::NfcBTech:
                  techName = "ISO 14443-B";
                  techColor = Theme::defaultNfcBColor;
                  break;

               case lab::FrameTech::NfcFTech:
                  techName = "ISO 18092";
                  techColor = Theme::defaultNfcFColor;
                  break;

               case lab::FrameTech::NfcVTech:
                  techName = "ISO 15693";
                  techColor = Theme::defaultNfcVColor;
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
      // get current selection
      QCPDataSelection selection = radioGraph->selection();

      // for empty selection no further action
      if (selection.isEmpty())
         return {};

      // get selection size and start / end
      double selectStart = signalData->at(selection.span().begin() + 1)->key;
      double selectEnd = signalData->at(selection.span().end() - 1)->key;

      // begin with full data
      double rangeStart = signalData->at(0)->key;
      double rangeEnd = signalData->at(signalData->size() - 1)->key;

      // adjust to frames
      for (QModelIndex modelIndex: streamModel->modelRange(rangeStart, rangeEnd))
      {
         if (const lab::RawFrame *frame = streamModel->frame(modelIndex))
         {
            // only NFC frames
            if (frame->techType() != lab::FrameTech::NfcATech && frame->techType() != lab::FrameTech::NfcBTech && frame->techType() != lab::FrameTech::NfcFTech && frame->techType() != lab::FrameTech::NfcVTech)
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
         return {};

      // select full data frames
      int startIndex = int(signalData->findBegin(rangeStart, false) - signalData->constBegin() + 1);
      int endIndex = int(signalData->findEnd(rangeEnd, false) - signalData->constBegin() - 1);
      radioGraph->setSelection(QCPDataSelection({startIndex, endIndex}));

      return {rangeStart, rangeEnd};
   }

   /**
    * @brief Detect selected data by rect and adjust to frames
    * @param rect Selection rect
    * @return Selected range
    */
   QCPRange selectByRect(const QRect &rect) const
   {
      // clear current selection
      radioGraph->setSelection(QCPDataSelection());

      // transport rect start / end to start / end in plot coordinates
      double rectStart = widget->plot()->xAxis->pixelToCoord(rect.left());
      double rectEnd = widget->plot()->xAxis->pixelToCoord(rect.right());

      // get start / end index of data to be selected
      int startIndex = int(signalData->findBegin(rectStart, false) - signalData->constBegin());
      int endIndex = int(signalData->findEnd(rectEnd, false) - signalData->constBegin() - 1);

      // only select events fully contained inside rect selection
      if (startIndex >= endIndex)
         return {};

      // finally get start / end for final selection
      double startTime = signalData->at(startIndex)->key;
      double endTime = signalData->at(endIndex)->key;

      // select data in graph
      radioGraph->setSelection(QCPDataSelection({startIndex, endIndex}));

      return {startTime, endTime};
   }

   /**
    * @param Apply limits to new scale
    * @return
    */
   QCPRange scaleFilter(const QCPRange &newScale) const
   {
      QCPRange fixedScale(0, newScale.upper);

      // limit upper scale to signal range
      if (fixedScale.upper > widget->dataUpperScale() || fixedScale.upper < widget->dataLowerScale())
         fixedScale.upper = widget->dataUpperScale();

      return fixedScale;
   }

   void dump() const
   {
      qInfo() << "radio channel" << radioGraph->style().text << "samples" << radioGraph->data()->size();
   }
};

RadioWidget::RadioWidget(QWidget *parent) : AbstractPlotWidget(parent), impl(new Impl(this))
{
}

void RadioWidget::setModel(StreamModel *model)
{
   impl->changeModel(model);
}

bool RadioWidget::hasData() const
{
   return impl->hasData();
}

void RadioWidget::append(const hw::SignalBuffer &buffer)
{
   impl->append(buffer);
}

void RadioWidget::start()
{
}

void RadioWidget::stop()
{
   impl->dump();
}

void RadioWidget::clear()
{
   impl->clear();

   AbstractPlotWidget::clear();
}

void RadioWidget::refresh()
{
   impl->refresh();

   AbstractPlotWidget::refresh();
}

QCPRange RadioWidget::selectByUser()
{
   return impl->selectByUser();
}

QCPRange RadioWidget::selectByRect(const QRect &rect)
{
   return impl->selectByRect(rect);
}

QCPRange RadioWidget::rangeFilter(const QCPRange &newRange)
{
   return AbstractPlotWidget::rangeFilter(newRange);
}

QCPRange RadioWidget::scaleFilter(const QCPRange &newScale)
{
   return impl->scaleFilter(newScale);
}

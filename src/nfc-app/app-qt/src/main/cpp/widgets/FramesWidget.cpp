/*

  Copyright (c) 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <QDebug>
#include <QMouseEvent>
#include <QVBoxLayout>

#include <nfc/Nfc.h>
#include <nfc/NfcFrame.h>

#include <support/QCustomPlot.h>

#include <graph/RangeMarker.h>
#include <graph/CursorMarker.h>

#include "FramesWidget.h"


struct FramesWidget::Impl
{
   FramesWidget *widget = nullptr;

   QCustomPlot *plot = nullptr;

   QSharedPointer<RangeMarker> marker;
   QSharedPointer<CursorMarker> cursor;

   double lowerSignalRange = +INT32_MAX;
   double upperSignalRange = -INT32_MAX;

   explicit Impl(FramesWidget *parent) : widget(parent), plot(new QCustomPlot(parent))
   {
      setup();
      clear();
   }

   void setup()
   {
      // selection pend and brush
      QPen selectPen(QColor(0, 128, 255, 255));
      QBrush selectBrush(QColor(0, 128, 255, 128));

      // create pen an brush
      QPen channelPen[3] = {
            {QColor(220, 220, 32, 255)},  // RF signal pen
            {QColor(0, 200, 128, 255)},   // SELECT frames pen
            {QColor(200, 200, 200, 255)}  // DATA frames pen
      };

      QBrush channelBrush[3] = {
            {QColor(128, 128, 16, 64)},   // RF signal brush
            {QColor(0, 200, 128, 64)},    // SELECT frames brush
            {QColor(128, 128, 128, 64)}   // DATA frames brush
      };

      // enable opengl
//      plot->setOpenGl(true);

      // data label for Y-axis
      QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);

      textTicker->addTick(1, "RF");
      textTicker->addTick(2, "SEL");
      textTicker->addTick(3, "APP");

      // disable aliasing to increase performance
      plot->setNoAntialiasingOnDrag(true);

      // configure plot
      plot->setMouseTracking(true);
      plot->setBackground(Qt::NoBrush);
      plot->setInteraction(QCP::iRangeDrag, true); // enable graph drag
      plot->setInteraction(QCP::iRangeZoom, true); // enable graph zoom
      plot->setInteraction(QCP::iSelectPlottables, true); // enable graph selection
      plot->setInteraction(QCP::iMultiSelect, true); // enable graph multiple selection
      plot->axisRect()->setRangeDrag(Qt::Horizontal); // only drag horizontal axis
      plot->axisRect()->setRangeZoom(Qt::Horizontal); // only zoom horizontal axis

      // setup time axis
      plot->xAxis->setBasePen(QPen(Qt::white));
      plot->xAxis->setTickPen(QPen(Qt::white));
      plot->xAxis->setTickLabelColor(Qt::white);
      plot->xAxis->setSubTickPen(QPen(Qt::white));
      plot->xAxis->setSubTicks(true);
      plot->xAxis->setRange(0, 1);

      // setup Y axis
      plot->yAxis->setBasePen(QPen(Qt::white));
      plot->yAxis->setTickPen(QPen(Qt::white));
      plot->yAxis->setTickLabelColor(Qt::white);
      plot->yAxis->setSubTickPen(QPen(Qt::white));
      plot->yAxis->setTicker(textTicker);
      plot->yAxis->setRange(0, 4);

      // create channels
      for (int i = 0; i < 3; i++)
      {
         // create upper and lower data graph
         auto upper = plot->addGraph();
         auto lower = plot->addGraph();

         upper->setPen(channelPen[i]);
         upper->setBrush(channelBrush[i]);
         upper->setSelectable(QCP::stDataRange);
         upper->selectionDecorator()->setPen(selectPen);
         upper->selectionDecorator()->setBrush(selectBrush);
         upper->setChannelFillGraph(lower);

         lower->setPen(channelPen[i]);
         lower->setSelectable(QCP::stDataRange);
         lower->selectionDecorator()->setPen(selectPen);
      }

      // create range marker
      marker.reset(new RangeMarker(plot->graph(0)->keyAxis()));

      // create cursor marker
      cursor.reset(new CursorMarker(plot->graph(0)->keyAxis()));

      // prepare layout
      auto *layout = new QVBoxLayout(widget);

      layout->setSpacing(0);
      layout->setContentsMargins(0, 0, 0, 0);
      layout->addWidget(plot);

      // connect graph signals
      QObject::connect(plot, &QCustomPlot::mouseMove, [=](QMouseEvent *event) {
         mouseMove(event);
      });

      QObject::connect(plot, &QCustomPlot::mousePress, [=](QMouseEvent *event) {
         mousePress(event);
      });

      QObject::connect(plot, &QCustomPlot::selectionChangedByUser, [=]() {
         selectionChanged();
      });

      QObject::connect(plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &newRange) {
         rangeChanged(newRange);
      });
   }

   void append(const nfc::NfcFrame &frame)
   {
      // update signal ranges
      if (lowerSignalRange > frame.timeStart())
         lowerSignalRange = frame.timeStart();

      if (upperSignalRange < frame.timeEnd())
         upperSignalRange = frame.timeEnd();

      // update view range
      plot->xAxis->setRange(lowerSignalRange, upperSignalRange);

      int graphChannel;
      int graphValue;
      float graphHeigth;

      switch (frame.framePhase())
      {
         case nfc::FramePhase::CarrierFrame:
         {
            graphChannel = 0;
            graphValue = 1;
            graphHeigth = frame.isEmptyFrame() ? 0.25 : 0.0;
            break;
         }
         case nfc::FramePhase::SelectionFrame:
         {
            graphChannel = 1;
            graphValue = 2;
            graphHeigth = frame.isPollFrame() ? 0.25 : 0.15;
            break;
         }
         default:
         {
            graphChannel = 2;
            graphValue = 3;
            graphHeigth = frame.isPollFrame() ? 0.25 : 0.15;
         }
      }

      int upperGraph = graphChannel * 2 + 0;
      int lowerGraph = graphChannel * 2 + 1;

      // draw upper shape
      plot->graph(upperGraph)->addData(frame.timeStart(), graphValue);
      plot->graph(upperGraph)->addData(frame.timeStart() + 2.5E-6, graphValue + graphHeigth);
      plot->graph(upperGraph)->addData(frame.timeEnd() - 2.5E-6, graphValue + graphHeigth);
      plot->graph(upperGraph)->addData(frame.timeEnd(), graphValue);

      // draw lower shape
      plot->graph(lowerGraph)->addData(frame.timeStart(), graphValue);
      plot->graph(lowerGraph)->addData(frame.timeStart() + 2.5E-6, graphValue - graphHeigth);
      plot->graph(lowerGraph)->addData(frame.timeEnd() - 2.5E-6, graphValue - graphHeigth);
      plot->graph(lowerGraph)->addData(frame.timeEnd(), graphValue);
   }

   void select(double from, double to)
   {
      for (int i = 0; i < plot->graphCount(); i++)
      {
         QCPDataSelection selection;

         QCPGraph *graph = plot->graph(i);

         int begin = graph->findBegin(from, false);
         int end = graph->findEnd(to, false);

         selection.addDataRange(QCPDataRange(begin, end));

         graph->setSelection(selection);
      }

      selectionChanged();
   }

   void clear()
   {
      lowerSignalRange = +INT32_MAX;
      upperSignalRange = -INT32_MAX;

      plot->xAxis->setRange(0, 1);

      for (int i = 0; i < plot->graphCount(); i++)
      {
         plot->graph(i)->data()->clear();
         plot->graph(i)->setSelection(QCPDataSelection());
      }

      cursor->hide();
      marker->hide();

      plot->replot();
   }

   void refresh() const
   {
      // fix range if current value is out
      rangeChanged(plot->xAxis->range());

      // refresh graph
      plot->replot();
   }

   void mouseEnter() const
   {
      cursor->show();
      plot->replot();
   }

   void mouseLeave() const
   {
      cursor->hide();
      plot->replot();
   }

   void mouseMove(QMouseEvent *event) const
   {
      double time = plot->xAxis->pixelToCoord(event->pos().x());
      cursor->update(time, QString("%1 s").arg(time, 10, 'f', 6));
      plot->replot();
   }

   void mousePress(QMouseEvent *event) const
   {
      Qt::KeyboardModifiers keyModifiers = QGuiApplication::queryKeyboardModifiers();

      if (keyModifiers & Qt::ControlModifier)
      {
         plot->setSelectionRectMode(QCP::srmSelect);
      }
      else
      {
         plot->setSelectionRectMode(QCP::srmNone);
      }
   }

   void selectionChanged() const
   {
      QList<QCPGraph *> selectedGraphs = plot->selectedGraphs();

      double startTime = 0;
      double endTime = 0;

      if (!selectedGraphs.empty())
      {
         QList<QCPGraph *>::Iterator itGraph = selectedGraphs.begin();

         while (itGraph != selectedGraphs.end())
         {
            QCPGraph *graph = *itGraph++;

            QCPDataSelection selection = graph->selection();

            for (int i = 0; i < selection.dataRangeCount(); i++)
            {
               QCPDataRange range = selection.dataRange(i);

               QCPGraphDataContainer::const_iterator data = graph->data()->at(range.begin());
               QCPGraphDataContainer::const_iterator end = graph->data()->at(range.end());

               while (data != end)
               {
                  double timestamp = data->key;

                  if (startTime == 0 || timestamp < startTime)
                     startTime = timestamp;

                  if (endTime == 0 || timestamp > endTime)
                     endTime = timestamp;

                  data++;
               }
            }
         }

         if (startTime > 0 && startTime < endTime)
         {
            QString text;

            double elapsed = endTime - startTime;

            if (elapsed < 1E-3)
               text = QString("%1 us").arg(elapsed * 1000000, 3, 'f', 0);
            else if (elapsed < 1)
               text = QString("%1 ms").arg(elapsed * 1000, 7, 'f', 3);
            else
               text = QString("%1 s").arg(elapsed, 7, 'f', 5);

            // show timing marker
            marker->show(startTime, endTime, text);
         }
         else
         {
            startTime = 0;
            endTime = 0;
            marker->hide();
         }
      }
      else
      {
         marker->hide();
      }

      // refresh graph
      plot->replot();

      // trigger selection changed signal
      widget->selectionChanged(startTime, endTime);
   }

   void rangeChanged(const QCPRange &newRange) const
   {
      QCPRange fixRange = newRange;

      if (newRange.lower < lowerSignalRange || newRange.lower > upperSignalRange)
         fixRange.lower = lowerSignalRange < +INT32_MAX ? lowerSignalRange : 0;

      if (newRange.upper > upperSignalRange || newRange.upper < lowerSignalRange)
         fixRange.upper = upperSignalRange > -INT32_MAX ? upperSignalRange : 1;

      if (fixRange != newRange)
         plot->xAxis->setRange(fixRange);
   }
};

FramesWidget::FramesWidget(QWidget *parent) : QWidget(parent), impl(new Impl(this))
{
}

void FramesWidget::append(const nfc::NfcFrame &frame)
{
   impl->append(frame);
}

void FramesWidget::select(double from, double to)
{
   impl->select(from, to);
}

void FramesWidget::clear()
{
   impl->clear();
}

void FramesWidget::refresh()
{
   impl->refresh();
}

void FramesWidget::enterEvent(QEvent *event)
{
   impl->mouseEnter();
}

void FramesWidget::leaveEvent(QEvent *event)
{
   impl->mouseLeave();
}


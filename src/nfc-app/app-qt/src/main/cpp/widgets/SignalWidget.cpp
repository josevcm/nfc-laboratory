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

#include <QMouseEvent>
#include <QVBoxLayout>

#include <support/QCustomPlot.h>

#include <sdr/SignalBuffer.h>

#include <graph/RangeMarker.h>
#include <graph/CursorMarker.h>

#include "SignalWidget.h"

struct SignalWidget::Impl
{
   SignalWidget *widget = nullptr;

   QCustomPlot *plot = nullptr;

   QSharedPointer<RangeMarker> marker;
   QSharedPointer<CursorMarker> cursor;
   QSharedPointer<QCPGraphDataContainer> data;

   float lowerSignalRange = INT32_MAX;
   float upperSignalRange = -INT32_MAX;

   float lowerSignalScale = INT32_MAX;
   float upperSignalScale = -INT32_MAX;

   float maximumRangeSpan = 5;

   explicit Impl(SignalWidget *parent) : widget(parent), plot(new QCustomPlot(parent))
   {
      setup();
      clear();
   }

   void setup()
   {
      QPen signalPen(QColor(100, 255, 140, 255));
      QPen selectPen(QColor(0, 200, 255, 255));

      // create data container
      data.reset(new QCPGraphDataContainer());

      // disable aliasing to increase performance
      plot->setNoAntialiasingOnDrag(true);

      // configure plot
      plot->setMouseTracking(true);
      plot->setBackground(Qt::NoBrush);
      plot->setInteraction(QCP::iRangeDrag, true); // enable graph drag
      plot->setInteraction(QCP::iRangeZoom, true); // enable graph zoom
      plot->setInteraction(QCP::iSelectPlottables, true); // enable graph selection
      plot->setInteraction(QCP::iMultiSelect, true); // enable graph multiple selection

      plot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical); // drag horizontal and vertical axes
      plot->axisRect()->setRangeZoom(Qt::Horizontal); // only zoom horizontal axis
      plot->axisRect()->setRangeZoomFactor(0.65, 0.75);

      // setup time axis
      plot->xAxis->setBasePen(QPen(Qt::darkGray));
      plot->xAxis->setTickPen(QPen(Qt::white));
      plot->xAxis->setTickLabelColor(Qt::white);
      plot->xAxis->setSubTickPen(QPen(Qt::darkGray));
      plot->xAxis->setSubTicks(true);
      plot->xAxis->setRange(0, 1);

      // setup Y axis
      plot->yAxis->setBasePen(QPen(Qt::darkGray));
      plot->yAxis->setTickPen(QPen(Qt::white));
      plot->yAxis->setTickLabelColor(Qt::white);
      plot->yAxis->setSubTickPen(QPen(Qt::darkGray));
      plot->xAxis->setSubTicks(true);
      plot->yAxis->setRange(0, 1);

      QCPGraph *graph = plot->addGraph();

      graph->setPen(signalPen);
      graph->setSelectable(QCP::stDataRange);
      graph->selectionDecorator()->setPen(selectPen);

      // get storage backend
      data = graph->data();

      // create range marker
      marker.reset(new RangeMarker(graph->keyAxis()));

      // create cursor marker
      cursor.reset(new CursorMarker(graph->keyAxis()));

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

      QObject::connect(plot, &QCustomPlot::mouseWheel, [=](QWheelEvent *event) {
         mouseWheel(event);
      });

      QObject::connect(plot, &QCustomPlot::selectionChangedByUser, [=]() {
         selectionChanged();
      });

      QObject::connect(plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &newRange) {
         rangeChanged(newRange);
      });

      QObject::connect(plot->yAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &newRange) {
         scaleChanged(newRange);
      });
   }

   void append(const sdr::SignalBuffer &buffer)
   {
      float sampleRate = buffer.sampleRate();
      float startTime = buffer.offset() / sampleRate;
      float endTime = startTime + buffer.elements() / sampleRate;

      // update signal range
      if (lowerSignalRange > startTime)
         lowerSignalRange = startTime;

      if (upperSignalRange < endTime)
         upperSignalRange = endTime;

      // remove old data
      if ((upperSignalRange - lowerSignalRange) > maximumRangeSpan)
      {
         lowerSignalRange = upperSignalRange - maximumRangeSpan;
         data->removeBefore(lowerSignalRange);
      }

      bool scaleChanged = false;

      for (int i = 0; i < buffer.elements(); i++)
      {
         float value = buffer[i];

         if (lowerSignalScale > value * 0.75)
         {
            scaleChanged = true;
            lowerSignalScale = value * 0.75;
         }

         if (upperSignalScale < value * 1.25)
         {
            scaleChanged = true;
            upperSignalScale = value * 1.25;
         }

         data->add({startTime + (i / sampleRate), value});
      }

      // update view scale
      if (scaleChanged)
         plot->yAxis->setRange(lowerSignalScale, upperSignalScale);
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

      if (from > lowerSignalRange && to < upperSignalRange)
      {
         QCPRange currentRange = plot->xAxis->range();

         if (from > currentRange.upper || to < currentRange.lower)
         {
            plot->xAxis->setRange(from, from + currentRange.upper - currentRange.lower);
         }
      }

      selectionChanged();
   }

   void range(double lower, double upper)
   {
      plot->xAxis->setRange(lower, upper);
      plot->replot();
   }

   void clear()
   {
      lowerSignalRange = +INT32_MAX;
      upperSignalRange = -INT32_MAX;

      lowerSignalScale = +INT32_MAX;
      upperSignalScale = -INT32_MAX;

      data->clear();

      plot->xAxis->setRange(0, 1);
      plot->yAxis->setRange(0, 1);

      for (int i = 0; i < plot->graphCount(); i++)
      {
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

      // fix scale if current value is out
      scaleChanged(plot->yAxis->range());

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
         plot->setSelectionRectMode(QCP::srmSelect);
      else
         plot->setSelectionRectMode(QCP::srmNone);
   }

   void mouseWheel(QWheelEvent *event) const
   {
      Qt::KeyboardModifiers keyModifiers = QGuiApplication::queryKeyboardModifiers();

      if (keyModifiers & Qt::ControlModifier)
         plot->axisRect()->setRangeZoom(Qt::Vertical);
      else
         plot->axisRect()->setRangeZoom(Qt::Horizontal);
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

      // check lower range limits
      if (newRange.lower < lowerSignalRange || newRange.lower > upperSignalRange)
         fixRange.lower = lowerSignalRange < +INT32_MAX ? lowerSignalRange : 0;

      // check upper range limits
      if (newRange.upper > upperSignalRange || newRange.upper < lowerSignalRange)
         fixRange.upper = upperSignalRange > -INT32_MAX ? upperSignalRange : 1;

      // fix visible range
      if (fixRange != newRange)
         plot->xAxis->setRange(fixRange);

      // get full range span
      float rangeSpan = upperSignalRange - lowerSignalRange;

      // emit range signal
      widget->rangeChanged((fixRange.lower - lowerSignalRange) / rangeSpan, (fixRange.upper - lowerSignalRange) / rangeSpan);
   }

   void scaleChanged(const QCPRange &newScale) const
   {
      QCPRange fixScale = newScale;

      // check lower scale limits
      if (newScale.lower < lowerSignalScale || newScale.lower > upperSignalScale)
         fixScale.lower = lowerSignalScale < +INT32_MAX ? lowerSignalScale : 0;

      // check lower scale limits
      if (newScale.upper > upperSignalScale || newScale.upper < lowerSignalScale)
         fixScale.upper = upperSignalScale > -INT32_MAX ? upperSignalScale : 1;

      // fix visible scale
      if (fixScale != newScale)
         plot->yAxis->setRange(fixScale);

      // get full scale span
      float scaleSpan = upperSignalScale - lowerSignalScale;

      // emit scale change signal
      widget->scaleChanged((fixScale.lower - lowerSignalScale) / scaleSpan, (fixScale.upper - lowerSignalScale) / scaleSpan);
   }

   void setCenterFreq(long value)
   {
   }

   void setSampleRate(long value)
   {
   }
};

SignalWidget::SignalWidget(QWidget *parent) : QWidget(parent), impl(new Impl(this))
{
}

void SignalWidget::setCenterFreq(long value)
{
   impl->setCenterFreq(value);
}

void SignalWidget::setSampleRate(long value)
{
   impl->setSampleRate(value);
}

void SignalWidget::append(const sdr::SignalBuffer &buffer)
{
   impl->append(buffer);
}

void SignalWidget::select(double from, double to)
{
   impl->select(from, to);
}

void SignalWidget::range(double lower, double upper)
{
   impl->range(lower, upper);
}

void SignalWidget::refresh()
{
   impl->refresh();
}

void SignalWidget::clear()
{
   impl->clear();
}

void SignalWidget::enterEvent(QEvent *event)
{
   impl->mouseEnter();
}

void SignalWidget::leaveEvent(QEvent *event)
{
   impl->mouseLeave();
}

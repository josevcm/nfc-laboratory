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

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>

#include <graph/RangeMarker.h>
#include <graph/CursorMarker.h>

#include "FourierWidget.h"

struct FourierWidget::Impl
{
   FourierWidget *widget = nullptr;

   QCustomPlot *plot = nullptr;

   QCPGraph *graph = nullptr;

   QSharedPointer<RangeMarker> marker;
   QSharedPointer<CursorMarker> cursor;
   QSharedPointer<QCPGraphDataContainer> data;

   float minimumRange = INT32_MAX;
   float maximumRange = -INT32_MAX;

   float minimumScale = INT32_MAX;
   float maximumScale = -INT32_MAX;

   float signalBuffer[65535] {0,};

   QColor signalColor {100, 255, 140, 255};
   QColor selectColor {0, 200, 255, 255};

   QPointer<QTimer> refreshTimer;

   QSemaphore refreshReady;

   explicit Impl(FourierWidget *parent) : widget(parent), plot(new QCustomPlot(parent)), refreshTimer(new QTimer())
   {
      setup();

      clear();
   }

   void setup()
   {
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
      plot->xAxis->setRange(-1, 1);

      // setup Y axis
      plot->yAxis->setBasePen(QPen(Qt::darkGray));
      plot->yAxis->setTickPen(QPen(Qt::white));
      plot->yAxis->setTickLabelColor(Qt::white);
      plot->yAxis->setSubTickPen(QPen(Qt::darkGray));
      plot->xAxis->setSubTicks(true);
      plot->yAxis->setRange(0, 1);

      graph = plot->addGraph();

      graph->setPen(QPen(signalColor));
      graph->setSelectable(QCP::stDataRange);
      graph->selectionDecorator()->setPen(QPen(selectColor));

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

      // connect refresh timer signal
      QObject::connect(refreshTimer, &QTimer::timeout, [=]() {
         refreshView();
      });

      // start timer
      refreshTimer->start(25);
   }

   void update(const sdr::SignalBuffer &buffer)
   {
      QVector<QCPGraphData> bins;

      switch (buffer.type())
      {
         case sdr::SignalType::FREQUENCY_BIN:
         {
            float startFreq = -1;
            float endFreq = +1;
            float binStep = (endFreq - startFreq) / buffer.limit();
            float binLength = buffer.limit();
            float temp[buffer.limit()];

            // update signal range
            if (minimumRange > startFreq)
               minimumRange = startFreq;

            if (maximumRange < endFreq)
               maximumRange = endFreq;

#pragma GCC ivdep
            for (int i = 0; i < buffer.elements(); i++)
            {
               temp[i] = 2.0 * 10 * log10f(buffer[i] / float(binLength));
            }

            for (int i = 2; i < buffer.elements() - 2; i++)
            {
               float range = fmaf(binStep, i, startFreq);
               float value = (temp[i - 2] + temp[i - 1] + temp[i] + temp[i + 1] + temp[i + 2]) / 5.0f;

               if (signalBuffer[i] < value)
                  signalBuffer[i] = signalBuffer[i] * (1.0 - 0.50) + value * 0.50;
               else if (signalBuffer[i] > value)
                  signalBuffer[i] = signalBuffer[i] * (1.0 - 0.30) + value * 0.30;

               value = signalBuffer[i];

               if (minimumScale > value)
                  minimumScale = value;

               if (maximumScale < value)
                  maximumScale = value;

               bins.append({range, value});
            }

            data->set(bins, true);

            refreshReady.release();

            break;
         }
      }
   }

   void select(float from, float to)
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

      if (from > minimumRange && to < maximumRange)
      {
         QCPRange currentRange = plot->xAxis->range();

         float center = float(from + to) / 2.0f;
         float length = float(currentRange.upper - currentRange.lower);

         plot->xAxis->setRange(center - length / 2, center + length / 2);
      }

      selectionChanged();
   }

   void clear()
   {
      minimumRange = +INT32_MAX;
      maximumRange = -INT32_MAX;

      minimumScale = +INT32_MAX;
      maximumScale = -INT32_MAX;

      data->clear();

      plot->xAxis->setRange(-1, 1);
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
      // refresh x range
      plot->xAxis->setRange(minimumRange, maximumRange);

      // refresh y scale
      plot->yAxis->setRange(minimumScale, maximumScale);

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
      if (newRange.lower < minimumRange || newRange.lower > maximumRange)
         fixRange.lower = minimumRange < +INT32_MAX ? minimumRange : -1;

      // check upper range limits
      if (newRange.upper > maximumRange || newRange.upper < minimumRange)
         fixRange.upper = maximumRange > -INT32_MAX ? maximumRange : 1;

      // fix visible range
      if (fixRange != newRange)
         plot->xAxis->setRange(fixRange);

      // update scatter style for high zoom values
      if ((fixRange.upper - fixRange.lower) < 1E-4)
      {
         graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, signalColor, signalColor, 4));
         graph->selectionDecorator()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, selectColor, selectColor, 4), QCPScatterStyle::spAll);
      }
      else if (!graph->scatterStyle().isNone())
      {
         graph->setScatterStyle(QCPScatterStyle::ssNone);
         graph->selectionDecorator()->setScatterStyle(QCPScatterStyle::ssNone, QCPScatterStyle::spAll);
      }

      // emit range signal
      widget->rangeChanged(fixRange.lower, fixRange.upper);
   }

   void scaleChanged(const QCPRange &newScale) const
   {
      QCPRange fixScale = newScale;

      // check lower scale limits
      if (newScale.lower < minimumScale || newScale.lower > maximumScale)
         fixScale.lower = minimumScale < +INT32_MAX ? minimumScale : -1;

      // check lower scale limits
      if (newScale.upper > maximumScale || newScale.upper < minimumScale)
         fixScale.upper = maximumScale > -INT32_MAX ? maximumScale : 0;

//      // scale not allowed to change
//      fixScale.lower = minimumScale < +INT32_MAX ? minimumScale : 0;
//      fixScale.upper = maximumScale > -INT32_MAX ? maximumScale : 1;

      // fix visible scale
      if (fixScale != newScale)
         plot->yAxis->setRange(fixScale);

      // emit scale change signal
      widget->scaleChanged(fixScale.lower, fixScale.upper);
   }

   void refreshView()
   {
      if (refreshReady.tryAcquire())
      {
         plot->replot();

//         qDebug() << plot->replotTime(true);
      }
   }

   void setCenterFreq(long value)
   {
   }

   void setSampleRate(long value)
   {
   }

   void setRange(float lower, float upper)
   {
      plot->xAxis->setRange(lower, upper);
      plot->replot();
   }

   void setCenter(float value)
   {
      qDebug() << "setCenter(" << value << ")";
   }

};

FourierWidget::FourierWidget(QWidget *parent) : QWidget(parent), impl(new Impl(this))
{
}

void FourierWidget::setCenterFreq(long value)
{
   impl->setCenterFreq(value);
}

void FourierWidget::setSampleRate(long value)
{
   impl->setSampleRate(value);
}

void FourierWidget::setRange(float lower, float upper)
{
   impl->setRange(lower, upper);
}

void FourierWidget::setCenter(float value)
{
   impl->setCenter(value);
}

void FourierWidget::refresh(const sdr::SignalBuffer &buffer)
{
   impl->update(buffer);
}

void FourierWidget::select(float from, float to)
{
   impl->select(from, to);
}

void FourierWidget::refresh()
{
   impl->refresh();
}

void FourierWidget::clear()
{
   impl->clear();
}

void FourierWidget::enterEvent(QEvent *event)
{
   impl->mouseEnter();
}

void FourierWidget::leaveEvent(QEvent *event)
{
   impl->mouseLeave();
}


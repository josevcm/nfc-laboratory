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

#include "SignalWidget.h"

struct SignalWidget::Impl
{
   SignalWidget *widget = nullptr;

   QCustomPlot *plot = nullptr;

   double lowerSignalRange = INFINITY;
   double upperSignalRange = 0;

   explicit Impl(SignalWidget *parent) : widget(parent), plot(new QCustomPlot(parent))
   {
   }

   void setup()
   {
      QPen selectPen(QColor(0, 128, 255, 255));
      QPen signalPen(QColor(0, 200, 128, 255));

      // data label for Y-axis
      QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);

      textTicker->addTick(0.5, "");

      // disable aliasing to increase performance
      plot->setNoAntialiasingOnDrag(true);

      // configure plot
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
      plot->xAxis->setSubTickPen(QPen(Qt::white));
      plot->xAxis->setSubTicks(true);
      plot->xAxis->setTickLabelColor(Qt::white);
      plot->xAxis->setRange(0, 1);

      // setup Y axis
      plot->yAxis->setBasePen(QPen(Qt::white));
      plot->yAxis->setTickPen(QPen(Qt::white));
      plot->yAxis->setSubTickPen(QPen(Qt::white));
      plot->xAxis->setSubTicks(true);
      plot->yAxis->setTickLabelColor(Qt::white);
//      plot->yAxis->setTicker(textTicker);
      plot->yAxis->setRange(0, 1);

      plot->setMouseTracking(true);

      QCPGraph *graph = plot->addGraph();

      graph->setPen(signalPen);
      graph->setSelectable(QCP::stDataRange);
      graph->selectionDecorator()->setPen(selectPen);

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
   }

   void refresh(const sdr::SignalBuffer &buffer)
   {
      float sampleRate = buffer.sampleRate();
      float startTime = buffer.offset() / sampleRate;
      float endTime = startTime + buffer.elements() / sampleRate;

      // update signal ranges
      if (lowerSignalRange > startTime)
         lowerSignalRange = startTime;

      if (upperSignalRange < endTime)
         upperSignalRange = endTime;

      QCPGraph *graph = plot->graph(0);

      for (int i = 0; i < buffer.elements(); i++)
      {
         graph->addData(startTime + (i / sampleRate), buffer[i]);
      }

      // update view range
      plot->xAxis->setRange(lowerSignalRange, upperSignalRange);
   }

   void select(double from, double to)
   {
   }

   void clear()
   {
   }

   void mouseEnter() const
   {
   }

   void mouseLeave() const
   {
   }

   void mouseMove(QMouseEvent *event) const
   {
   }

   void mousePress(QMouseEvent *event) const
   {
   }

   void selectionChanged() const
   {
   }

   void rangeChanged(const QCPRange &newRange) const
   {
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
   // setup plot
   impl->setup();

   // initialize
   impl->clear();
}

void SignalWidget::setCenterFreq(long value)
{
   impl->setCenterFreq(value);
}

void SignalWidget::setSampleRate(long value)
{
   impl->setSampleRate(value);
}

void SignalWidget::refresh(const sdr::SignalBuffer &buffer)
{
   impl->refresh(buffer);
}

void SignalWidget::select(double from, double to)
{
   impl->select(from, to);
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

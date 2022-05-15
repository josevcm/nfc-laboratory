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

#include <3party/customplot/QCustomPlot.h>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>

#include <graph/QCPAxisCursorMarker.h>
#include <graph/QCPAxisTickerFrequency.h>
#include <graph/QCPGraphValueMarker.h>

#include "FourierWidget.h"

#define DEFAULT_LOWER_RANGE (13.56E6 - 10E6 / 32)
#define DEFAULT_UPPER_RANGE (13.56E6 + 10E6 / 32)

//#define DEFAULT_LOWER_RANGE (40.68E6 - 10E6 / 32)
//#define DEFAULT_UPPER_RANGE (40.68E6 + 10E6 / 32)

#define DEFAULT_LOWER_SCALE -120
#define DEFAULT_UPPER_SCALE 0

struct FourierWidget::Impl
{
   FourierWidget *widget = nullptr;

   QCustomPlot *plot = nullptr;

   QCPGraph *graph = nullptr;

   QSharedPointer<QCPGraphValueMarker> peakMarker;
   QSharedPointer<QCPAxisCursorMarker> cursorMarker;
   QSharedPointer<QCPGraphDataContainer> graphData;
   QSharedPointer<QCPAxisTickerFrequency> axisTicker;

   double centerFreq;
   double sampleRate;

   double minimumRange = INT32_MAX;
   double maximumRange = INT32_MIN;

   double minimumScale = INT32_MAX;
   double maximumScale = INT32_MIN;

   double signalBuffer[65535] {0,};

   double signalPeak;

   QColor signalColor {255, 255, 50, 255};
   QColor selectColor {0, 200, 255, 255};
   QColor markerColor {255, 150, 150, 255};

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
      graphData.reset(new QCPGraphDataContainer());

      // create frequency ticker
      axisTicker.reset(new QCPAxisTickerFrequency());

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
      plot->xAxis->setBasePen(QPen(Qt::white));
      plot->xAxis->setTickPen(QPen(Qt::white));
      plot->xAxis->setTickLabelColor(Qt::white);
      plot->xAxis->setSubTickPen(QPen(Qt::darkGray));
      plot->xAxis->setSubTicks(true);
      plot->xAxis->setTicker(axisTicker);
      plot->xAxis->setRange(DEFAULT_LOWER_RANGE, DEFAULT_UPPER_RANGE);
      plot->xAxis->grid()->setZeroLinePen(Qt::NoPen);

      // setup Y axis
      plot->yAxis->setBasePen(QPen(Qt::white));
      plot->yAxis->setTickPen(QPen(Qt::white));
      plot->yAxis->setTickLabelColor(Qt::white);
      plot->yAxis->setSubTickPen(QPen(Qt::darkGray));
      plot->xAxis->setSubTicks(true);
      plot->yAxis->setRange(DEFAULT_LOWER_SCALE, DEFAULT_UPPER_SCALE);
      plot->yAxis->grid()->setZeroLinePen(Qt::NoPen);

      graph = plot->addGraph();

      graph->setPen(QPen(signalColor));
//      graph->setBrush(QBrush(QColor(0, 0, 255, 20)));
      graph->setSelectable(QCP::stDataRange);
      graph->selectionDecorator()->setPen(QPen(selectColor));

      // get storage backend
      graphData = graph->data();

      // create cursor marker
      cursorMarker.reset(new QCPAxisCursorMarker(graph->keyAxis()));

      // create cursor marker
      peakMarker.reset(new QCPGraphValueMarker(graph, markerColor));

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

      QObject::connect(plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &, const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &newRange, const QCPRange &oldRange) {
         rangeChanged(newRange, oldRange);
      });

      QObject::connect(plot->yAxis, static_cast<void (QCPAxis::*)(const QCPRange &, const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &newRange, const QCPRange &oldRange) {
         scaleChanged(newRange, oldRange);
      });

      // connect refresh timer signal
      QObject::connect(refreshTimer, &QTimer::timeout, [=]() {
         refreshView();
      });

      // start timer at 40FPS (25ms / frame)
      refreshTimer->start(25);
   }

   void update(const sdr::SignalBuffer &buffer)
   {
      QVector<QCPGraphData> bins;

      peakMarker->setVisible(false);

      switch (buffer.type())
      {
         case sdr::SignalType::FREQUENCY_BIN:
         {
            double temp[buffer.elements()];

            double decimation = buffer.decimation() > 0 ? (float) buffer.decimation() : 1.0f;
            double binSize = (sampleRate / decimation) / (float) buffer.elements();
            double lowerFreq = centerFreq - (sampleRate / (decimation * 2));
            double upperFreq = centerFreq + (sampleRate / (decimation * 2));
            double binLength = buffer.elements();

            // update signal range
            if (minimumRange > lowerFreq)
               minimumRange = lowerFreq;

            if (maximumRange < upperFreq)
               maximumRange = upperFreq;

            // reset frequency peak
            signalPeak = 0;

            // process signal average and variance
            double average = 0;
            double maximum = INT32_MIN;

#pragma GCC ivdep
            for (int i = 0; i < buffer.elements(); i++)
               average += temp[i] = 2.0 * 10 * log10(buffer[i] / double(binLength));

            average = average / buffer.elements();

            // process signal bins and peak detector
            for (int i = 2; i < buffer.elements() - 2; i++)
            {
               double range = fma(binSize, i, lowerFreq);
               double value = (temp[i - 2] + temp[i - 1] + temp[i] + temp[i + 1] + temp[i + 2]) / 5.0f;
               double diffv = (value - average);

               // peak detector
               if (maximum < temp[i] && (diffv > 10))
               {
                  maximum = temp[i];
                  signalPeak = range;
               }

               // attack and decay animation
               if (signalBuffer[i] < value)
                  signalBuffer[i] = signalBuffer[i] * (1.0 - 0.30) + value * 0.30;
               else if (signalBuffer[i] > value)
                  signalBuffer[i] = signalBuffer[i] * (1.0 - 0.20) + value * 0.20;

               value = signalBuffer[i];

               if (minimumScale > value)
                  minimumScale = value;

               if (maximumScale < value)
                  maximumScale = value;

               bins.append({range, value});
            }

            graphData->set(bins, true);

            if (maximum > INT32_MIN)
            {
               peakMarker->setPosition(signalPeak, frequencyString(signalPeak));
               peakMarker->setVisible(true);
            }

            refreshReady.release();

            break;
         }
      }
   }

   void clear()
   {
      minimumRange = INT32_MAX;
      maximumRange = INT32_MIN;

      minimumScale = INT32_MAX;
      maximumScale = INT32_MIN;

      graphData->clear();

      plot->xAxis->setRange(DEFAULT_LOWER_RANGE, DEFAULT_UPPER_RANGE);
      plot->yAxis->setRange(DEFAULT_LOWER_SCALE, DEFAULT_UPPER_SCALE);

      for (int i = 0; i < plot->graphCount(); i++)
      {
         plot->graph(i)->setSelection(QCPDataSelection());
      }

      cursorMarker->setVisible(false);

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
      widget->setFocus(Qt::MouseFocusReason);
      cursorMarker->setVisible(true);
      plot->replot();
   }

   void mouseLeave() const
   {
      widget->setFocus(Qt::NoFocusReason);
      cursorMarker->setVisible(false);
      plot->replot();
   }

   void mouseMove(QMouseEvent *event) const
   {
      double freq = plot->xAxis->pixelToCoord(event->pos().x());

      cursorMarker->setPosition(freq, frequencyString(freq));

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

   void rangeChanged(const QCPRange &newRange, const QCPRange &oldRange) const
   {
      QCPRange fixRange = newRange;

      // check lower range limits
      if (newRange.lower < minimumRange || newRange.lower > maximumRange)
         fixRange.lower = minimumRange < INT32_MAX ? minimumRange : DEFAULT_LOWER_RANGE;

      // check upper range limits
      if (newRange.upper > maximumRange || newRange.upper < minimumRange)
         fixRange.upper = maximumRange > INT32_MIN ? maximumRange : DEFAULT_UPPER_RANGE;

      // fix visible range
      if (fixRange != newRange)
         plot->xAxis->setRange(fixRange);

      // emit range signal
      widget->rangeChanged(fixRange.lower, fixRange.upper);
   }

   void scaleChanged(const QCPRange &newScale, const QCPRange &oldScale) const
   {
      QCPRange fixScale = newScale;

      // check lower scale limits
      if (newScale.lower < minimumScale || newScale.lower > maximumScale)
         fixScale.lower = minimumScale < INT32_MAX ? minimumScale : DEFAULT_LOWER_SCALE;

      // check lower scale limits
      if (newScale.upper > maximumScale || newScale.upper < minimumScale)
         fixScale.upper = maximumScale > INT32_MIN ? maximumScale : DEFAULT_UPPER_SCALE;

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
      }
   }

   inline QString frequencyString(double frequency) const
   {
      if (frequency > 1E9)
         return QString("%1GHz").arg(frequency / 1E9, 0, 'f', 3);

      if (frequency > 1E6)
         return QString("%1MHz").arg(frequency / 1E6, 0, 'f', 3);

      if (frequency > 1E3)
         return QString("%1KHz").arg(frequency / 1E3, 0, 'f', 3);

      return QString("%1Hz").arg(frequency, 0, 'f', 3);
   }
};

FourierWidget::FourierWidget(QWidget *parent) : QWidget(parent), impl(new Impl(this))
{
}

void FourierWidget::setCenterFreq(long value)
{
   impl->centerFreq = 13.56E6; //float(value);
}

void FourierWidget::setSampleRate(long value)
{
   impl->sampleRate = float(value);
}

void FourierWidget::refresh(const sdr::SignalBuffer &buffer)
{
   impl->update(buffer);
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


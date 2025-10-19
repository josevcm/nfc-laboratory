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

#include "FrequencyWidget.h"

#include <QTimer>
#include <QSemaphore>
#include <QMutex>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include <graph/AxisLabel.h>
#include <graph/ChannelGraph.h>
#include <graph/MarkerCursor.h>
#include <graph/MarkerPeaks.h>
#include <graph/TickerFrequency.h>

#include <styles/Theme.h>

#include <format/DataFormat.h>

#define DEFAULT_CENTER_FREQ 13.56E6
#define DEFAULT_SAMPLE_RATE 10E6

#define DEFAULT_RANGE_SPAN (10E6/16)

#define DEFAULT_LOWER_RANGE (DEFAULT_CENTER_FREQ - DEFAULT_RANGE_SPAN/2)
#define DEFAULT_UPPER_RANGE (DEFAULT_CENTER_FREQ + DEFAULT_RANGE_SPAN/2)

#define DEFAULT_LOWER_SCALE (-120)
#define DEFAULT_UPPER_SCALE 0

struct FrequencyWidget::Impl
{
   FrequencyWidget *widget = nullptr;

   QCustomPlot *plot = nullptr;
   QCPItemText *centerText = nullptr;
   QCPItemStraightLine *centerLine = nullptr;

   ChannelGraph *baseGraph = nullptr;
   ChannelGraph *binsGraph = nullptr;
   ChannelGraph *peakGraph = nullptr;

   ChannelStyle binsGraphStyle {Theme::defaultSignalPen, Theme::defaultSignalPen, Theme::defaultSignalBrush, Theme::defaultTextPen, Theme::defaultLabelFont, "FFT"};
   ChannelStyle peakGraphStyle {Theme::defaultFrequencyPen, Theme::defaultFrequencyPen, Theme::defaultFrequencyBrush, Theme::defaultTextPen, Theme::defaultCenterFreqFont, "PEAK"};

   QSharedPointer<QCPGraphDataContainer> binsGraphData;
   QSharedPointer<QCPGraphDataContainer> baseGraphData;
   QSharedPointer<QCPGraphDataContainer> peakGraphData;

   QSharedPointer<AxisLabel> scaleLabel;
   QSharedPointer<MarkerPeaks> peaksMarker;

   double centerFreq = DEFAULT_CENTER_FREQ;
   double sampleRate = DEFAULT_SAMPLE_RATE;

   double signalDecayBuffer[65535] {0,};
   double signalPeaksBuffer[65535] {0,};

   QPointer<QTimer> refreshTimer;

   QSemaphore nextFrame;
   QMutex syncMutex;

   explicit Impl(FrequencyWidget *parent) : widget(parent),
                                            plot(widget->plot()),
                                            binsGraph(new ChannelGraph(plot->xAxis, plot->yAxis)),
                                            baseGraph(new ChannelGraph(plot->xAxis, plot->yAxis)),
                                            peakGraph(new ChannelGraph(plot->xAxis, plot->yAxis)),
                                            refreshTimer(new QTimer())
   {
      // set cursor formatter
      widget->setCursorFormatter(DataFormat::frequency);
      widget->setRangeFormatter(DataFormat::frequencyRange);

      // set plot properties
      plot->xAxis->grid()->setSubGridVisible(true);
      plot->xAxis->setTicker(QSharedPointer<TickerFrequency>(new TickerFrequency));

      // initialize legend
      plot->legend->setIconSize(60, 20);

      // initialize axis label
      scaleLabel.reset(new AxisLabel(plot->yAxis));
      scaleLabel->setText("dBFS", Qt::TopLeftCorner);
      scaleLabel->setVisible(true);

      // create center line
      centerLine = new QCPItemStraightLine(plot);
      centerLine->setLayer("grid");
      centerLine->point1->setCoords(centerFreq, 0);
      centerLine->point2->setCoords(centerFreq, 1);
      centerLine->setPen(QPen(Qt::darkGray, 0, Qt::DashLine));

      // create center text
      centerText = new QCPItemText(plot);
      centerText->setText(QString("%1MHz").arg(centerFreq / 1E6, 0, 'f', 2));
      centerText->setLayer("overlay");
      centerText->setVisible(true);
      centerText->setSelectable(false);
      centerText->setClipToAxisRect(false);
      centerText->setFont(Theme::defaultCenterFreqFont);
      centerText->setColor(Theme::defaultCenterFreqColor);
      centerText->setPadding(QMargins(4, 0, 4, 4));
      centerText->setPositionAlignment(Qt::AlignBottom | Qt::AlignHCenter);
      centerText->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      centerText->position->setParentAnchorX(centerLine->point1);
      centerText->position->setCoords(0, 0);

      // create baseline for filling
      baseGraph->removeFromLegend();

      // set frequency bins properties
      binsGraph->setStyle(binsGraphStyle);
      binsGraph->setPen(binsGraphStyle.linePen);
      binsGraph->setBrush(binsGraphStyle.shapeBrush);
      binsGraph->setSelectionDecorator(nullptr);
      binsGraph->setChannelFillGraph(baseGraph);

      // set peak bins properties
      peakGraph->setStyle(peakGraphStyle);
      peakGraph->setPen(peakGraphStyle.linePen);
      peakGraph->setBrush(peakGraphStyle.shapeBrush);
      peakGraph->setSelectionDecorator(nullptr);
      peakGraph->setChannelFillGraph(baseGraph);

      // get storage backend
      baseGraphData = baseGraph->data();
      binsGraphData = binsGraph->data();
      peakGraphData = peakGraph->data();

      // create peak marker
      peaksMarker.reset(new MarkerPeaks(peakGraph, 0.50, 5, 5));
      peaksMarker->setFormatter(peakFormatter);

      // configure legend
      plot->legend->setColumnStretchFactor(0, 0.001);
      plot->legend->setColumnStretchFactor(1, 1);

      // connect refresh timer signal
      refreshTimer->callOnTimeout([=] {
         refresh();
      });

      // start timer at 25FPS (40ms / frame)
      refreshTimer->start(40);
   }

   ~Impl()
   {
      refreshTimer->stop();

      plot->removeItem(centerLine);
   }

   void update(const hw::SignalBuffer &buffer)
   {
      QVector<QCPGraphData> base;
      QVector<QCPGraphData> bins;
      QVector<QCPGraphData> peak;

      double minimumRange = widget->dataLowerRange();
      double maximumRange = widget->dataUpperRange();
      double minimumScale = widget->dataLowerScale();
      double maximumScale = widget->dataUpperScale();

      switch (buffer.type())
      {
         case hw::SignalType::SIGNAL_TYPE_FFT_BIN:
         {
            double temp[buffer.elements()];

            double decimation = buffer.decimation() > 0 ? (float)buffer.decimation() : 1.0f;
            double binStep = (sampleRate / decimation) / (float)buffer.elements();
            double lowerFreq = centerFreq - (sampleRate / (decimation * 2));
            double upperFreq = centerFreq + (sampleRate / (decimation * 2));
            double fftSize = buffer.elements();

            // update signal range
            if (minimumRange > lowerFreq)
               minimumRange = lowerFreq;

            if (maximumRange < upperFreq)
               maximumRange = upperFreq;

            // process frequency bins and transform to logarithmic scale
            const double scale = 2.0 * 10.0;
            const double fftInv = 1.0 / double(fftSize);

#pragma omp simd
            for (int i = 0; i < buffer.elements(); i++)
               temp[i] = scale * log10(buffer[i] * fftInv);

            // filter frequency and apply decay animation
            for (int i = 2; i < buffer.elements() - 2; i++)
            {
               double frequency = fma(binStep, i, lowerFreq);
               double value = (temp[i - 2] + temp[i - 1] + temp[i] + temp[i + 1] + temp[i + 2]) / 5.0f;

               // if value is inf ignore it
               if (std::isinf(value))
                  continue;

               // attack and decay animation
               if (signalDecayBuffer[i] < value)
                  signalDecayBuffer[i] = signalDecayBuffer[i] * (1.0 - 0.30) + value * 0.30;
               else if (signalDecayBuffer[i] > value)
                  signalDecayBuffer[i] = signalDecayBuffer[i] * (1.0 - 0.20) + value * 0.20;

               value = signalDecayBuffer[i];

               // update scale range
               if (minimumScale > value)
                  minimumScale = value;

               if (maximumScale < value)
                  maximumScale = value;

               // signal image buffer
               if (value > signalPeaksBuffer[i - 2])
                  signalPeaksBuffer[i - 2] = value;

               // add data
               bins.append({frequency, value});
               peak.append({frequency, signalPeaksBuffer[i - 2]});
            }

            // create base graph for filling
            base.append({lowerFreq, minimumScale});
            base.append({upperFreq, minimumScale});

            // update signal frequency data
            {
               QMutexLocker lock(&syncMutex);

               baseGraphData->set(base, true);
               binsGraphData->set(bins, true);
               peakGraphData->set(peak, true);
            }

            nextFrame.release();

            break;
         }
      }

      if (minimumRange != widget->dataLowerRange() || maximumRange != widget->dataUpperRange())
         widget->setDataRange(minimumRange, maximumRange);

      if (minimumScale != widget->dataLowerScale() || maximumScale != widget->dataUpperScale())
         widget->setDataScale(minimumScale, maximumScale);
   }

   void start()
   {
      // clear buffers
      for (double &v: signalDecayBuffer)
         v = INT_MIN;

      for (double &v: signalPeaksBuffer)
         v = INT_MIN;

      QMutexLocker lock(&syncMutex);

      // show signal and peak graphs
      binsGraph->setVisible(true);
      peakGraph->setVisible(true);

      plot->replot();

      nextFrame.release();
   }

   void stop()
   {
      QMutexLocker lock(&syncMutex);

      // hide bins and keep only signal peaks
      binsGraph->setVisible(false);
      peakGraph->setVisible(true);

      plot->replot();
   }

   void clear()
   {
      // clear buffers
      for (double &v: signalDecayBuffer)
         v = INT_MIN;

      for (double &v: signalPeaksBuffer)
         v = INT_MIN;

      QMutexLocker lock(&syncMutex);

      for (int i = 0; i < plot->graphCount(); i++)
      {
         plot->graph(i)->data()->clear();
      }

      plot->replot();
   }

   void refresh()
   {
      if (!nextFrame.tryAcquire())
         return;

      QMutexLocker lock(&syncMutex);

      plot->replot();
   }

   QCPRange selectByUser() const
   {
      return {};
   }

   static QString peakFormatter(double frequency, double power)
   {
      return DataFormat::frequency(frequency) + "\n" + QString("%1 dBFS").arg(power, 0, 'f', 2);
   }
};

FrequencyWidget::FrequencyWidget(QWidget *parent) : AbstractPlotWidget(parent), impl(new Impl(this))
{
   setDataRange(DEFAULT_LOWER_RANGE, DEFAULT_UPPER_RANGE);
   setDataScale(DEFAULT_LOWER_SCALE, DEFAULT_UPPER_SCALE);
}

void FrequencyWidget::setCenterFreq(long value)
{
   impl->centerFreq = float(value);
   impl->centerLine->point1->setCoords(value, 0);
   impl->centerLine->point2->setCoords(value, 1);
   impl->centerText->setText(QString("%1MHz").arg(impl->centerFreq / 1E6, 0, 'f', 2));

   setDataRange(impl->centerFreq - DEFAULT_RANGE_SPAN / 2, impl->centerFreq + DEFAULT_RANGE_SPAN / 2);

   reset();
}

void FrequencyWidget::setSampleRate(long value)
{
   impl->sampleRate = float(value);
}

void FrequencyWidget::update(const hw::SignalBuffer &buffer)
{
   impl->update(buffer);
}

void FrequencyWidget::start()
{
   impl->start();
}

void FrequencyWidget::stop()
{
   impl->stop();
}

void FrequencyWidget::clear()
{
   impl->clear();

   AbstractPlotWidget::clear();
}

QCPRange FrequencyWidget::selectByUser()
{
   return impl->selectByUser();
}

QCPRange FrequencyWidget::selectByRect(const QRect &rect)
{
   return AbstractPlotWidget::selectByRect(rect);
}

QCPRange FrequencyWidget::rangeFilter(const QCPRange &newRange)
{
   return AbstractPlotWidget::rangeFilter(newRange);
}

QCPRange FrequencyWidget::scaleFilter(const QCPRange &newScale)
{
   return AbstractPlotWidget::scaleFilter(newScale);
}

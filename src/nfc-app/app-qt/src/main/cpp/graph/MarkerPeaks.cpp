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

#include <rt/Logger.h>

#include <styles/Theme.h>

#include <utility>

#include "MarkerPeaks.h"

struct Peak
{
   double key;
   double value;
};

struct Marker
{
   QCPGraph *graph;
   QCustomPlot *plot;
   QCPItemText *label;
   QCPItemTracer *tracer;
   std::function<QString(double, double)> formatter;

   explicit Marker(QCPGraph *graph, std::function<QString(double, double)> formatter) : graph(graph),
                                                                                        plot(graph->parentPlot()),
                                                                                        tracer(new QCPItemTracer(plot)),
                                                                                        label(new QCPItemText(plot)),
                                                                                        formatter(std::move(formatter))
   {
      tracer->setVisible(false);
      tracer->setSelectable(false);
      tracer->setGraph(graph);
      tracer->setGraphKey(0);
      tracer->setLayer("overlay");
      tracer->setInterpolating(true);
      tracer->setStyle(QCPItemTracer::tsSquare);
      tracer->setPen(Theme::defaultMarkerPen);
      tracer->setSize(10);
      tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
      tracer->position->setTypeY(QCPItemPosition::ptPlotCoords);

      label->setVisible(false);
      label->setSelectable(false);
      label->setFont(Theme::defaultLabelFont);
      label->setColor(Theme::defaultLabelColor);
      label->setPen(Theme::defaultLabelPen);
      label->setBrush(Theme::defaultLabelBrush);
      label->setColor(Qt::white);
      label->setLayer("overlay");
      label->setClipToAxisRect(false);
      label->setPadding(QMargins(6, 2, 6, 4));
      label->setPositionAlignment(Qt::AlignBottom | Qt::AlignHCenter);
      label->position->setParentAnchor(tracer->position);
      label->position->setCoords(0, -10);
   }

   ~Marker()
   {
      plot->removeItem(label);
      plot->removeItem(tracer);
   }

   double key() const
   {
      return tracer->graphKey();
   }

   double value() const
   {
      return graph->data()->findBegin(tracer->graphKey(), false)->value;
   }

   void setPosition(double key) const
   {
      tracer->setGraphKey(key);
      label->setText(formatter(key, value()));
   }

   void setVisible(bool visible) const
   {
      tracer->setVisible(visible);
      label->setVisible(visible);
   }
};

struct MarkerPeaks::Impl
{
   QCustomPlot *plot;
   QCPGraph *graph;
   double filter;
   double threshold;
   int maxPeaks;
   std::function<QString(double, double)> formatter;

   QList<Marker *> markers;

   QMetaObject::Connection beforeReplotConnection;

   explicit Impl(QCPGraph *graph, double filter, double threshold, int maxPeaks) : graph(graph),
                                                                                   plot(graph->parentPlot()),
                                                                                   filter(filter),
                                                                                   threshold(threshold),
                                                                                   maxPeaks(maxPeaks),
                                                                                   formatter([](double key, double value) { return QString::number(value); })
   {
      beforeReplotConnection = QObject::connect(plot, &QCustomPlot::afterLayout, [=]() {
         afterLayout();
      });

      // create markers
      for (int i = 0; i < maxPeaks; i++)
      {
         markers.append(new Marker(graph, formatter));
      }
   }

   ~Impl()
   {
      for (auto marker: markers)
         delete marker;

      QObject::disconnect(beforeReplotConnection);
   }

   void afterLayout()
   {
      hide();

      if (graph->data()->isEmpty())
         return;

      double w = graph->data()->at(0)->value;
      double l = w;
      double z, y;

      QList<Peak> peaks;
      Peak *peak = nullptr;

      // apply IIR filter to remove DC offset and check threshold for peak detection
      for (auto p: *graph->data())
      {
         // IIR DC removal filter
         z = w;
         w = p.value + filter * w;
         y = w - z;

         // detect rising edge
         if (l < threshold && y > threshold)
         {
            // create new peak if needed
            peaks.append({p.key, p.value});

            // get peak
            peak = &peaks.last();
         }

            // detect falling edge to reset peak
         else if (l > threshold && y < threshold)
         {
            peak = nullptr;
         }

         // update peak value
         if (peak && peak->value < p.value)
         {
            peak->key = p.key;
            peak->value = p.value;
         }

         l = y;
      }

      // limit peaks to first maxPeaks sorted with the highest values
      std::sort(peaks.begin(), peaks.end(), [](const Peak &a, const Peak &b) { return a.value > b.value; });

      // update markers
      for (int i = 0; i < maxPeaks; i++)
      {
         if (i < peaks.size())
         {
            markers[i]->setPosition(peaks[i].key);
            markers[i]->setVisible(true);
         }
      }
   }

   void hide()
   {
      for (auto marker: markers)
         marker->setVisible(false);
   }

   void show()
   {
      for (auto marker: markers)
         marker->setVisible(true);
   }
};

MarkerPeaks::MarkerPeaks(QCPGraph *graph, double filter, double threshold, int maxPeaks) : impl(new Impl(graph, filter, threshold, maxPeaks))
{
}

void MarkerPeaks::setVisible(bool visible)
{
   if (visible)
      impl->show();
   else
      impl->hide();
}

void MarkerPeaks::setFormatter(const std::function<QString(double, double)> &formatter)
{
   // update formatter in all markers
   for (const auto &marker: impl->markers)
      marker->formatter = formatter;
}
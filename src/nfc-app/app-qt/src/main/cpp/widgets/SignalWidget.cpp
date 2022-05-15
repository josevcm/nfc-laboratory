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

#include <QList>
#include <QMouseEvent>
#include <QVBoxLayout>

#include <3party/customplot/QCustomPlot.h>

#include <sdr/SignalType.h>
#include <sdr/SignalBuffer.h>

#include <graph/QCPAxisRangeMarker.h>
#include <graph/QCPAxisCursorMarker.h>

#include "SignalWidget.h"

#define DEFAULT_LOWER_RANGE 0
#define DEFAULT_UPPER_RANGE 1

#define DEFAULT_LOWER_SCALE 0
#define DEFAULT_UPPER_SCALE 0.5

struct SignalWidget::Impl
{
   SignalWidget *widget = nullptr;

   QCustomPlot *plot = nullptr;

   QCPGraph *graph = nullptr;

   QList<QSharedPointer<QCPAxisRangeMarker>> markerList;
   QSharedPointer<QCPAxisRangeMarker> markerActive;
   QSharedPointer<QCPAxisRangeMarker> selectedRange;
   QSharedPointer<QCPAxisCursorMarker> cursorMarker;

   QSharedPointer<QCPGraphDataContainer> graphData;

   double minimumRange = INT32_MAX;
   double maximumRange = INT32_MIN;

   double minimumScale = INT32_MAX;
   double maximumScale = INT32_MIN;

   unsigned int maximumEntries;

   QColor signalColor {100, 255, 140, 255};
   QColor selectColor {0, 200, 255, 255};

   explicit Impl(SignalWidget *parent) : widget(parent), plot(new QCustomPlot(parent)), maximumEntries(512 * 1024 * 1024 / sizeof(QCPGraphData))
   {
      setup();

      clear();
   }

   void setup()
   {
      // create data container
      graphData.reset(new QCPGraphDataContainer());

      // disable aliasing to increase performance
      plot->setNoAntialiasingOnDrag(true);

      // configure plot
      plot->setMouseTracking(true);
      plot->setBackground(Qt::NoBrush);
      plot->setInteraction(QCP::iRangeDrag, true); // enable graph drag
      plot->setInteraction(QCP::iRangeZoom, true); // enable graph zoom
      plot->setInteraction(QCP::iSelectPlottables, true); // enable graph selection
      plot->setInteraction(QCP::iSelectItems, true); // enable markers selection
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
      plot->xAxis->setRange(DEFAULT_LOWER_RANGE, DEFAULT_UPPER_RANGE);

      // setup Y axis
      plot->yAxis->setBasePen(QPen(Qt::darkGray));
      plot->yAxis->setTickPen(QPen(Qt::white));
      plot->yAxis->setTickLabelColor(Qt::white);
      plot->yAxis->setSubTickPen(QPen(Qt::darkGray));
      plot->xAxis->setSubTicks(true);
      plot->yAxis->setRange(DEFAULT_LOWER_SCALE, DEFAULT_UPPER_SCALE);

      graph = plot->addGraph();

      graph->setPen(QPen(signalColor));
      graph->setSelectable(QCP::stDataRange);
      graph->selectionDecorator()->setPen(QPen(selectColor));

      // get storage backend
      graphData = graph->data();

      // create cursor marker
      cursorMarker.reset(new QCPAxisCursorMarker(graph->keyAxis()));

      // create selection marker
      selectedRange.reset(new QCPAxisRangeMarker(graph->keyAxis()));

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

      QObject::connect(plot, &QCustomPlot::mouseRelease, [=](QMouseEvent *event) {
         mouseRelease(event);
      });

      QObject::connect(plot, &QCustomPlot::mouseWheel, [=](QWheelEvent *event) {
         mouseWheel(event);
      });

      QObject::connect(plot, &QCustomPlot::selectionChangedByUser, [=]() {
         selectionChanged();
      });

      QObject::connect(plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &, const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &newRange, const QCPRange &oldRange) {
         rangeChanged(newRange, oldRange);
      });

      QObject::connect(plot->yAxis, static_cast<void (QCPAxis::*)(const QCPRange &, const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &newRange, const QCPRange &oldRange) {
         scaleChanged(newRange, oldRange);
      });
   }

   void append(const sdr::SignalBuffer &buffer)
   {
      minimumScale = DEFAULT_LOWER_SCALE;
      maximumScale = DEFAULT_UPPER_SCALE;

      switch (buffer.type())
      {
         case sdr::SignalType::SAMPLE_REAL:
         {
            double sampleRate = buffer.sampleRate();
            double sampleStep = 1 / sampleRate;
            double startTime = buffer.offset() / sampleRate;

            for (int i = 0; i < buffer.elements(); i++)
            {
               double value = buffer[i];
               double range = fma(sampleStep, i, startTime); // range = sampleStep * i + startTime

//               if (minimumScale > value * 0.75)
//                  minimumScale = value * 0.75;
//
//               if (maximumScale < value * 1.25)
//                  maximumScale = value * 1.25;

               graphData->add({range, value});
            }

            // update signal range
            if (graphData->size() > 0)
            {
               minimumRange = graphData->at(0)->key;
               maximumRange = graphData->at(graphData->size() - 1)->key;
            }

            break;
         }

         case sdr::SignalType::ADAPTIVE_REAL:
         {
            double sampleRate = buffer.sampleRate();
            double sampleStep = 1 / sampleRate;
            double startTime = buffer.offset() / sampleRate;

            for (int i = 0; i < buffer.limit(); i += 2)
            {
               double value = buffer[i + 0];
               double range = fma(sampleStep, buffer[i + 1], startTime); // range = sampleStep * buffer[i + 1] + startTime

               // update signal scale
//               if (minimumScale > value * 0.75)
//                  minimumScale = value * 0.75;
//
//               if (maximumScale < value * 1.25)
//                  maximumScale = value * 1.25;

               graphData->add({range, value});
            }

            // update signal range
            if (graphData->size() > 0)
            {
               minimumRange = graphData->at(0)->key;
               maximumRange = graphData->at(graphData->size() - 1)->key;
            }

            break;
         }
      }

      // remove old data when maximum memory threshold is reached
      if (graphData->size() > maximumEntries)
      {
         graphData->removeBefore(minimumRange);
         minimumRange = graphData->at(0)->key;
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

      plot->replot();
   }

   void refresh() const
   {
      // fix range if current value is out
      rangeChanged(plot->xAxis->range(), plot->xAxis->range());

      // refresh graph
      plot->replot();

      // statistics
      qDebug().noquote() << "total samples" << graphData->size() << "adaptive compression ratio" << QString("%1%").arg(float(graphData->size() / ((maximumRange - minimumRange) * 10E4)), 0, 'f', 2);
   }

   void mouseEnter()
   {
      widget->setFocus(Qt::MouseFocusReason);
      plot->replot();
   }

   void mouseLeave()
   {
      widget->setFocus(Qt::NoFocusReason);
      plot->replot();
   }

   void mouseMove(QMouseEvent *event)
   {
      Qt::KeyboardModifiers keyModifiers = QGuiApplication::queryKeyboardModifiers();

      // current position
      double position = plot->xAxis->pixelToCoord(event->pos().x());

      // update cursor position
      cursorMarker->setPosition(position, QString("%1 s").arg(position, 10, 'f', 6));

      // select selection mode
      if (keyModifiers & Qt::ControlModifier)
         plot->setSelectionRectMode(QCP::srmSelect);
      else
         plot->setSelectionRectMode(QCP::srmNone);

      // update marker end time if any
      if (markerActive)
         markerActive->setPositionEnd(position);

      plot->replot();
   }

   void mousePress(QMouseEvent *event)
   {
//      Qt::KeyboardModifiers keyModifiers = QGuiApplication::queryKeyboardModifiers();
   }

   void mouseRelease(QMouseEvent *event)
   {
      Qt::KeyboardModifiers keyModifiers = QGuiApplication::queryKeyboardModifiers();

      // add marker to list
      if (markerActive)
      {
         if (markerActive->width() > 0)
         {
            markerActive->setSelected(true);

            markerList.push_back(markerActive);
         }

         // reset active marker
         markerActive.reset();
      }
      else
      {
         // start new marker if Control key is pressed
         if (keyModifiers & Qt::AltModifier)
         {
            double time = plot->xAxis->pixelToCoord(event->pos().x());

            // load new range marker
            markerActive.reset(new QCPAxisRangeMarker(graph->keyAxis()));

            // fix start and end time
            markerActive->setPositionStart(time);
            markerActive->setPositionEnd(time);
            markerActive->setVisible(true);
         }
      }
   }

   void mouseWheel(QWheelEvent *event) const
   {
      Qt::KeyboardModifiers keyModifiers = QGuiApplication::queryKeyboardModifiers();

      if (keyModifiers & Qt::ControlModifier)
         plot->axisRect()->setRangeZoom(Qt::Vertical);
      else
         plot->axisRect()->setRangeZoom(Qt::Horizontal);
   }

   void keyPress(QKeyEvent *event)
   {
      cursorMarker->setVisible(event->modifiers() & Qt::AltModifier);

      plot->replot();
   }

   void keyRelease(QKeyEvent *event)
   {
      cursorMarker->setVisible(event->modifiers() & Qt::AltModifier);

      switch (event->key())
      {
         case Qt::Key_Escape:
         {
            markerActive.reset();

            break;
         }

         case Qt::Key_Delete:
         {
            QMutableListIterator<QSharedPointer<QCPAxisRangeMarker>> it(markerList);

            while (it.hasNext())
            {
               auto marker = it.next();

               if (marker->selected())
               {
                  marker->setVisible(false);

                  it.remove();
               }
            }

            selectDataset(0, 0);

            selectionChanged();

            break;
         }
      }

      plot->replot();
   }

   void selectionChanged() const
   {
      QList<QCPGraph *> selectedGraphs = plot->selectedGraphs();

      double startTime = 0;
      double endTime = 0;

      selectedRange->setVisible(false);

      if (!selectedGraphs.empty())
      {
         QList<QCPGraph *>::Iterator itGraph = selectedGraphs.begin();

         while (itGraph != selectedGraphs.end())
         {
            QCPGraph *entry = *itGraph++;

            QCPDataSelection selection = entry->selection();

            for (int i = 0; i < selection.dataRangeCount(); i++)
            {
               QCPDataRange range = selection.dataRange(i);

               QCPGraphDataContainer::const_iterator data = entry->data()->at(range.begin());
               QCPGraphDataContainer::const_iterator end = entry->data()->at(range.end());

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

      }

      for (const auto &marker: markerList)
      {
         if (marker->selected())
         {
            if (startTime == 0 || startTime > marker->positionStart())
               startTime = marker->positionStart();

            if (endTime == 0 || endTime < marker->positionEnd())
               endTime = marker->positionEnd();
         }
      }

      if (startTime > 0)
      {
         selectDataset(startTime, endTime);
      }

      // refresh graph
      plot->replot();

      // trigger selection changed signal
      widget->selectionChanged(startTime, endTime);
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

   void setCenterFreq(long value)
   {
   }

   void setSampleRate(long value)
   {
   }

   void setRange(double lower, double upper)
   {
      plot->xAxis->setRange(lower, upper);
      plot->replot();
   }

   void setCenter(double center) const
   {
      QCPRange currentRange = plot->xAxis->range();

      float length = float(currentRange.upper - currentRange.lower);

      plot->xAxis->setRange(center - length / 2, center + length / 2);
   }

   void selectDataset(double from, double to) const
   {
      selectedRange->setVisible(false);

      if (from >= 0 && to >= 0)
      {
         selectedRange->setPositionStart(from);
         selectedRange->setPositionEnd(to);
         selectedRange->setSelected(true);
         selectedRange->setVisible(true);

         for (int i = 0; i < plot->graphCount(); i++)
         {
            QCPDataSelection selection;

            QCPGraph *graph = plot->graph(i);

            int begin = graph->findBegin(from, false);
            int end = graph->findEnd(to, false);

            selection.addDataRange(QCPDataRange(begin, end));

            graph->setSelection(selection);
         }
      }
   }

   void selectAndCenter(double from, double to) const
   {
      selectDataset(from, to);

      if (from > minimumRange && to < maximumRange)
      {
         setCenter(double(from + to) / 2.0f);
      }

      selectionChanged();
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

void SignalWidget::setRange(double lower, double upper)
{
   impl->setRange(lower, upper);
}

void SignalWidget::setCenter(double value)
{
   impl->setCenter(value);
}

void SignalWidget::append(const sdr::SignalBuffer &buffer)
{
   impl->append(buffer);
}

void SignalWidget::select(double from, double to)
{
   impl->selectAndCenter(from, to);
}

void SignalWidget::refresh()
{
   impl->refresh();
}

void SignalWidget::clear()
{
   impl->clear();
}

double SignalWidget::minimumRange() const
{
   return impl->minimumRange;
}

double SignalWidget::maximumRange() const
{
   return impl->maximumRange;
}

double SignalWidget::minimumScale() const
{
   return impl->minimumScale;
}

double SignalWidget::maximumScale() const
{
   return impl->maximumScale;
}

void SignalWidget::enterEvent(QEvent *event)
{
   impl->mouseEnter();
}

void SignalWidget::leaveEvent(QEvent *event)
{
   impl->mouseLeave();
}

void SignalWidget::keyPressEvent(QKeyEvent *event)
{
   impl->keyPress(event);
}

void SignalWidget::keyReleaseEvent(QKeyEvent *event)
{
   impl->keyRelease(event);
}


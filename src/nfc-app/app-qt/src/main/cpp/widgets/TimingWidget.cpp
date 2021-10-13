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

#include "TimingWidget.h"

struct RangeMarker
{
   QCPAxis *axis;
   QCPItemTracer *tracer = nullptr;
   QCPItemTracer *start = nullptr;
   QCPItemTracer *end = nullptr;
   QCPItemText *label = nullptr;
   QCPItemLine *arrow = nullptr;

   explicit RangeMarker(QCPAxis *axis) : axis(axis)
   {
      setup();
   }

   ~RangeMarker()
   {
      delete label;
      delete arrow;
      delete end;
      delete tracer;
      delete start;
   }

   void setup()
   {
      tracer = new QCPItemTracer(axis->parentPlot());
      tracer->setVisible(false);
      tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
      tracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      tracer->position->setAxisRect(axis->axisRect());
      tracer->position->setAxes(axis, nullptr);
      tracer->position->setCoords(0, 1);

      start = new QCPItemTracer(axis->parentPlot());
      start->setVisible(false);
      start->setPen(QPen(Qt::white));
      start->position->setTypeX(QCPItemPosition::ptPlotCoords);
      start->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      start->position->setAxisRect(axis->axisRect());
      start->position->setAxes(axis, nullptr);
      start->position->setCoords(0, 1);

      end = new QCPItemTracer(axis->parentPlot());
      end->setVisible(false);
      end->setPen(QPen(Qt::white));
      end->position->setTypeX(QCPItemPosition::ptPlotCoords);
      end->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      end->position->setAxisRect(axis->axisRect());
      end->position->setAxes(axis, nullptr);
      end->position->setCoords(0, 1);

      arrow = new QCPItemLine(axis->parentPlot());
      arrow->setPen(QPen(Qt::gray));
      arrow->setLayer("overlay");
      arrow->setClipToAxisRect(false);
      arrow->setHead(QCPLineEnding::esSpikeArrow);
      arrow->setTail(QCPLineEnding::esSpikeArrow);
      arrow->start->setParentAnchor(start->position);
      arrow->end->setParentAnchor(end->position);

      label = new QCPItemText(axis->parentPlot());
      label->setPen(QPen(Qt::gray));
      label->setBrush(QBrush(Qt::white));
      label->setLayer("overlay");
      label->setVisible(false);
      label->setClipToAxisRect(false);
      label->setPadding(QMargins(5, 0, 4, 2));
      label->setPositionAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
      label->position->setParentAnchor(tracer->position);
   }

   void show(double from, double to, const QString &text)
   {
      label->setText(text);
      tracer->position->setCoords((from + to) / 2, 0);
      start->position->setCoords(from, 0);
      end->position->setCoords(to, 0);

      label->setVisible(true);
      arrow->setVisible(true);
      start->setVisible(true);
      end->setVisible(true);
   }

   void hide()
   {
      label->setVisible(false);
      arrow->setVisible(false);
      start->setVisible(false);
      end->setVisible(false);
   }
};

struct CursorMarker
{
   QCPAxis *axis;
   QCPItemTracer *tracer = nullptr;
   QCPItemText *label = nullptr;

   explicit CursorMarker(QCPAxis *axis) : axis(axis)
   {
      setup();
   }

   ~CursorMarker()
   {
      delete label;
      delete tracer;
   }

   void setup()
   {
      tracer = new QCPItemTracer(axis->parentPlot());
      tracer->setVisible(false);
      tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
      tracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
      tracer->position->setAxisRect(axis->axisRect());
      tracer->position->setAxes(axis, nullptr);
      tracer->position->setCoords(0, 0);

      label = new QCPItemText(axis->parentPlot());
      label->setPen(QPen(Qt::darkGray));
      label->setBrush(QBrush(Qt::white));
      label->setLayer("overlay");
      label->setVisible(false);
      label->setClipToAxisRect(false);
      label->setPadding(QMargins(2, 1, 4, 3));
      label->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
      label->position->setParentAnchor(tracer->position);
   }

   void show()
   {
      label->setVisible(true);
   }

   void hide()
   {
      label->setVisible(false);
   }

   void update(double from, const QString &text)
   {
      label->setText(text);
      tracer->position->setCoords(from, 1);
   }
};

struct TimingWidget::Impl
{
   TimingWidget *widget = nullptr;

   QCustomPlot *plot = nullptr;

   RangeMarker *range = nullptr;
   CursorMarker *cursor = nullptr;

   double lowerSignalRange = INFINITY;
   double upperSignalRange = 0;

   explicit Impl(TimingWidget *parent) : widget(parent), plot(new QCustomPlot(parent))
   {
   }

   ~Impl()
   {
      delete range;
      delete cursor;
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

      // disable aliasing to increase performance
      plot->setNoAntialiasingOnDrag(true);

      // data label for Y-axis
      QSharedPointer <QCPAxisTickerText> textTicker(new QCPAxisTickerText);

      textTicker->addTick(1, "RF");
      textTicker->addTick(2, "SEL");
      textTicker->addTick(3, "APP");

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
      plot->yAxis->setTickLabelColor(Qt::white);
      plot->yAxis->setTicker(textTicker);
      plot->yAxis->setRange(0, 4);

      plot->setMouseTracking(true);

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
      range = new RangeMarker(plot->graph(0)->keyAxis());

      // create cursor marker
      cursor = new CursorMarker(plot->graph(0)->keyAxis());

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

      //      QObject::connect(ui->timingPlot->xAxis, qOverload<const QCPRange &>(&QCPAxis::rangeChanged), mainWindow, &QtWindow::plotRangeChanged);
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
      lowerSignalRange = INFINITY;
      upperSignalRange = 0;

      plot->xAxis->setRange(0, 1);

      for (int i = 0; i < plot->graphCount(); i++)
      {
         plot->graph(i)->data()->clear();
         plot->graph(i)->setSelection(QCPDataSelection());
      }

      range->hide();

      plot->replot();
   }

   void refresh() const
   {
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
      QList < QCPGraph * > selectedGraphs = plot->selectedGraphs();

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
            range->show(startTime, endTime, text);
         }
         else
         {
            startTime = 0;
            endTime = 0;

            range->hide();
         }
      }
      else
      {
         range->hide();
      }

      // refresh graph
      plot->replot();

      // trigger selection changed signal
      widget->selectionChanged(startTime, endTime);
   }

   void rangeChanged(const QCPRange &newRange) const
   {
      if (newRange.lower != INFINITY && lowerSignalRange != INFINITY && newRange.lower < lowerSignalRange)
         plot->xAxis->setRangeLower(lowerSignalRange);

      if (newRange.upper != INFINITY && upperSignalRange != INFINITY && newRange.upper > upperSignalRange)
         plot->xAxis->setRangeUpper(upperSignalRange);
   }
};

TimingWidget::TimingWidget(QWidget *parent) : QWidget(parent), impl(new Impl(this))
{
   // setup plot
   impl->setup();

   // initialize
   impl->clear();
}

void TimingWidget::append(const nfc::NfcFrame &frame)
{
   impl->append(frame);
}

void TimingWidget::select(double from, double to)
{
   impl->select(from, to);
}

void TimingWidget::clear()
{
   impl->clear();
}

void TimingWidget::refresh()
{
   impl->refresh();
}

void TimingWidget::enterEvent(QEvent *event)
{
   impl->mouseEnter();
}

void TimingWidget::leaveEvent(QEvent *event)
{
   impl->mouseLeave();
}


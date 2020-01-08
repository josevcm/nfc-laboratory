/*

  Copyright (c) 2020 Jose Vicente Campos Martinez - <josevcm@gmail.com>

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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include "PlotView.h"

PlotView::PlotView()
{
   // setup signal plot
   setBackground(Qt::NoBrush);
   setInteraction(QCP::iRangeDrag, true); // enable graph drag
   setInteraction(QCP::iRangeZoom, true); // enable graph zoom
   setInteraction(QCP::iSelectPlottables, true); // enable graph selection
   setInteraction(QCP::iMultiSelect, true); // enable graph multiple selection
   axisRect()->setRangeDrag(Qt::Horizontal); // only drag horizontal axis
   axisRect()->setRangeZoom(Qt::Horizontal); // only zoom horizontal axis

   // setup time axis
   xAxis->setBasePen(QPen(Qt::white, 1));
   xAxis->setTickPen(QPen(Qt::white, 1));
   xAxis->setSubTickPen(QPen(Qt::white, 1));
   xAxis->setTickLabelColor(Qt::white);
   xAxis->setRange(0, 1);

   // setup frame types for Y-axis
   QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
   textTicker->addTick(1, "REQ");
   textTicker->addTick(2, "SEL");
   textTicker->addTick(3, "INF");

   yAxis->setBasePen(QPen(Qt::white, 1));
   yAxis->setTickPen(QPen(Qt::white, 1));
   yAxis->setSubTickPen(QPen(Qt::white, 1));
   yAxis->setTickLabelColor(Qt::white);
   yAxis->setTicker(textTicker);
   yAxis->setRange(0, 4);

   // graph for RF status
   QCPGraph *graphRf = addGraph();
   graphRf->setPen(QPen(Qt::cyan));
   graphRf->setBrush(QBrush(QColor(0, 0, 255, 20)));
   graphRf->setSelectable(QCP::stDataRange);
   graphRf->selectionDecorator()->setBrush(graphRf->brush());

   // graph for sense request phase (REQ / WUPA)
   QCPGraph *graphReq = addGraph();
   graphReq->setPen(QPen(Qt::green));
   graphReq->setBrush(QBrush(QColor(0, 255, 0, 20)));
   graphReq->setSelectable(QCP::stDataRange);
   graphReq->selectionDecorator()->setBrush(graphReq->brush());

   // graph for selection and anti-collision phase (SELx / PPS / ATS)
   QCPGraph *graphSel = addGraph();
   graphSel->setPen(QPen(Qt::red));
   graphSel->setBrush(QBrush(QColor(255, 0, 0, 20)));
   graphSel->setSelectable(QCP::stDataRange);
   graphSel->selectionDecorator()->setBrush(graphSel->brush());

   // graph for information frames phase (other types)
   QCPGraph *graphInf = addGraph();
   graphInf->setPen(QPen(Qt::gray));
   graphInf->setBrush(QBrush(QColor(255, 255, 255, 20)));
   graphInf->setSelectable(QCP::stDataRange);
   graphInf->selectionDecorator()->setBrush(graphInf->brush());

   // setup time measure and graph tracer
   mTimeMarker = new Marker(graphInf->keyAxis());
   mTimeMarker->setPen(QPen(Qt::gray));
   mTimeMarker->setBrush(QBrush(Qt::white));

   // connect graph signals
   QObject::connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(plotSelectionChanged()));
   QObject::connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(plotMousePress(QMouseEvent*)));
}

void PlotView::addFrame(NfcFrame frame)
{
   this->xAxis->setRange(0, frame.timeEnd());

   // signal present
   if (frame.isRequestFrame() || frame.isResponseFrame() || frame.isNoFrame())
   {
      int graphIndex = 0;
      int graphValue = 0;

      switch(frame.framePhase())
      {
         case NfcFrame::SenseFrame:
         {
            graphIndex = 1;
            graphValue = 1;
            break;
         }
         case NfcFrame::SelectionFrame:
         {
            graphIndex = 2;
            graphValue = 2;
            break;
         }
         case NfcFrame::InformationFrame:
         {
            graphIndex = 3;
            graphValue = 3;
            break;
         }
      }

      if (graphIndex > 0)
      {
         // move previous marker
         QCPGraphDataContainer::iterator it = this->graph(graphIndex)->data()->end() - 1;
         it->key = frame.timeStart();

         // add pulse
         this->graph(graphIndex)->addData(frame.timeStart(), graphValue + 0.5);
         this->graph(graphIndex)->addData(frame.timeEnd(), graphValue + 0.5);
         this->graph(graphIndex)->addData(frame.timeEnd(), graphValue);

         // add end marker
         this->graph(graphIndex)->addData(frame.timeEnd(), graphValue);
      }

      // add padding moving end markers
      for (int i = 1; i < this->graphCount(); i++)
      {
         if (i != graphIndex)
         {
            QCPGraphDataContainer::iterator it = this->graph(i)->data()->end() - 1;

            if (it != this->graph(i)->data()->begin())
            {
               it->key = frame.timeEnd();
            }
         }
      }
   }

   // signal not present
   else
   {
      for (int i = 0; i < this->graphCount(); i++)
      {
         if (this->graph(i)->data()->isEmpty())
         {
            this->graph(i)->addData(frame.timeStart(), 0);
            this->graph(i)->addData(frame.timeEnd(), 0);
         }
         else
         {
            QCPGraphDataContainer::iterator it = this->graph(i)->data()->end() - 1;

            it->key = frame.timeEnd();
         }
      }
   }

   this->replot();
}

void PlotView::updateMarker(double start, double end)
{
   QString text;

   double elapsed = end - start;

   if (elapsed < 1E-3)
      text = QString("%1 us").arg(elapsed * 1000000, 3, 'f', 0);
   else if (elapsed < 1)
      text = QString("%1 ms").arg(elapsed * 1000, 7, 'f', 3);
   else
      text = QString("%1 s").arg(elapsed, 7, 'f', 5);

   mTimeMarker->setRange(start, end);

   if (start < end)
      mTimeMarker->setText(text);
   else
      mTimeMarker->setText(QString("%1 s").arg(start, 7, 'f', 5));

   this->replot();
}

void PlotView::plotSelectionChanged()
{
   QList<QCPGraph*> selectedGraphs = this->selectedGraphs();

   if (selectedGraphs.size() > 0)
   {
      QString text;
      double startTime = -1;
      double endTime = -1;

      QList<QCPGraph*>::Iterator it = selectedGraphs.begin();

      while (it != selectedGraphs.end())
      {
         QCPGraph *graph = *it++;

         QCPDataSelection selection = graph->selection();

         QList<QCPDataRange>::Iterator range = selection.dataRanges().begin();

         while(range != selection.dataRanges().end())
         {
            QCPGraphDataContainer::const_iterator data = graph->data()->at(range->begin());
            QCPGraphDataContainer::const_iterator end = graph->data()->at(range->end());

            while(data != end)
            {
               double timestamp = data->key;

               if (startTime < 0 || timestamp < startTime)
                  startTime = timestamp;

               if (endTime < 0 || timestamp > endTime)
                  endTime = timestamp;

               data++;
            }

            range++;
         }
      }

      updateMarker(startTime, endTime);
   }
}

void PlotView::plotRangeChanged(const QCPRange &newRange, const QCPRange &oldRange)
{

}

void PlotView::plotMousePress(QMouseEvent *event)
{
   Qt::KeyboardModifiers keyModifiers = QGuiApplication::queryKeyboardModifiers();

   if (keyModifiers & Qt::ControlModifier)
   {
      setSelectionRectMode(QCP::srmSelect);
   }
   else
   {
      setSelectionRectMode(QCP::srmNone);
   }
}

PlotView::Marker::Marker(QCPAxis *parentAxis) :
   mAxis(parentAxis)
{
   mStart = new QCPItemTracer(mAxis->parentPlot());
   mStart->setVisible(false);
   mStart->position->setTypeX(QCPItemPosition::ptPlotCoords);
   mStart->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
   mStart->position->setAxisRect(mAxis->axisRect());
   mStart->position->setAxes(mAxis, nullptr);
   mStart->position->setCoords(0, 1);
   mStart->setPen(QPen(Qt::white));

   mMiddle = new QCPItemTracer(mAxis->parentPlot());
   mMiddle->setVisible(false);
   mMiddle->position->setTypeX(QCPItemPosition::ptPlotCoords);
   mMiddle->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
   mMiddle->position->setAxisRect(mAxis->axisRect());
   mMiddle->position->setAxes(mAxis, nullptr);
   mMiddle->position->setCoords(0, 1);

   mEnd = new QCPItemTracer(mAxis->parentPlot());
   mEnd->setVisible(false);
   mEnd->position->setTypeX(QCPItemPosition::ptPlotCoords);
   mEnd->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
   mEnd->position->setAxisRect(mAxis->axisRect());
   mEnd->position->setAxes(mAxis, nullptr);
   mEnd->position->setCoords(0, 1);
   mEnd->setPen(QPen(Qt::white));

   mArrow = new QCPItemLine(mAxis->parentPlot());
   mArrow->setLayer("overlay");
   mArrow->setClipToAxisRect(false);
   mArrow->setHead(QCPLineEnding::esSpikeArrow);
   mArrow->start->setParentAnchor(mStart->position);
   mArrow->end->setParentAnchor(mEnd->position);

   mLabel = new QCPItemText(mAxis->parentPlot());
   mLabel->setLayer("overlay");
   mLabel->setVisible(false);
   mLabel->setClipToAxisRect(false);
   mLabel->setPadding(QMargins(3, 0, 4, 2));
   mLabel->setBrush(QBrush(Qt::white));
   mLabel->setPen(QPen(Qt::white));
   mLabel->setPositionAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
   mLabel->position->setParentAnchor(mMiddle->position);
}

PlotView::Marker::~Marker()
{
   if (mLabel)
      mLabel->deleteLater();

   if (mArrow)
      mArrow->deleteLater();

   if (mEnd)
      mEnd->deleteLater();

   if (mMiddle)
      mMiddle->deleteLater();

   if (mStart)
      mStart->deleteLater();
}

QPen PlotView::Marker::pen() const
{
   return mLabel->pen();
}

void PlotView::Marker::setPen(const QPen &pen)
{
   mArrow->setPen(pen);
   mLabel->setPen(pen);
}

QBrush PlotView::Marker::brush() const
{
   return mLabel->brush();
}

void PlotView::Marker::setBrush(const QBrush &brush)
{
   mLabel->setBrush(brush);
}

QString PlotView::Marker::text() const
{
   return mLabel->text();
}

void PlotView::Marker::setText(const QString &text)
{
   mLabel->setText(text);
   mLabel->setVisible(!text.isEmpty());
}

void PlotView::Marker::setRange(double start, double end)
{
   mStart->position->setCoords(start, 0);
   mMiddle->position->setCoords((start + end) / 2, 0);
   mEnd->position->setCoords(end, 0);

   mStart->setVisible(start > 0);
   mEnd->setVisible(end > 0);
}

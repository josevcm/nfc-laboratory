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

#include <QMouseEvent>
#include <QWheelEvent>

#include <3party/customplot/QCustomPlot.h>

#include <format/DataFormat.h>

#include <styles/Theme.h>

#include <graph/MarkerCursor.h>
#include <graph/MarkerZoom.h>
#include <graph/MarkerRange.h>
#include <graph/SelectionRect.h>

#include "AbstractPlotWidget.h"

#define DEFAULT_LOWER_RANGE 0
#define DEFAULT_UPPER_RANGE 1E-6

#define DEFAULT_LOWER_SCALE 0
#define DEFAULT_UPPER_SCALE 1

#define DEFAULT_ZOOM_DRAG true

#define ZOOM_STEP_FACTOR 0.75

struct AbstractPlotWidget::Impl
{
   AbstractPlotWidget *widget = nullptr;

   QCustomPlot *plot = nullptr;

   double dataLowerRange = DEFAULT_LOWER_RANGE;
   double dataUpperRange = DEFAULT_UPPER_RANGE;
   double viewLowerRange = DEFAULT_LOWER_RANGE;
   double viewUpperRange = DEFAULT_UPPER_RANGE;

   double dataLowerScale = DEFAULT_LOWER_SCALE;
   double dataUpperScale = DEFAULT_UPPER_SCALE;
   double viewLowerScale = DEFAULT_LOWER_SCALE;
   double viewUpperScale = DEFAULT_UPPER_SCALE;

   QSharedPointer<MarkerCursor> cursorMarker;
   QSharedPointer<MarkerRange> rangeMarker;
   QSharedPointer<MarkerZoom> zoomMarker;

   bool zoomDragMode = DEFAULT_ZOOM_DRAG;

   QElapsedTimer timer;

   QMetaObject::Connection afterLayoutConnection;
   QMetaObject::Connection legendClickConnection;
   QMetaObject::Connection mouseMoveConnection;
   QMetaObject::Connection mousePressConnection;
   QMetaObject::Connection mouseReleaseConnection;
   QMetaObject::Connection mouseDoubleClickConnection;
   QMetaObject::Connection mouseWheelConnection;
   QMetaObject::Connection selectionRectAcceptedConnection;
   QMetaObject::Connection selectionChangedByUserConnection;
   QMetaObject::Connection rangeChangedConnection;
   QMetaObject::Connection scaleChangedConnection;

   explicit Impl(AbstractPlotWidget *widget) : widget(widget), plot(new QCustomPlot(widget))
   {
      // set plot properties
      plot->setMouseTracking(true);
      plot->setBackground(Qt::NoBrush);
      plot->setNoAntialiasingOnDrag(true);
      plot->setMultiSelectModifier(Qt::ShiftModifier);

      // Fix memory leak: ensure buffer device pixel ratio is set correctly
      plot->setBufferDevicePixelRatio(1.0);

      // set plot selection mode
      plot->setSelectionTolerance(10); // set tolerance for data selection
      plot->setSelectionRect(new SelectionRect(plot));
      plot->setSelectionRectMode(zoomDragMode ? QCP::srmNone : QCP::srmCustom);

      // set plot interactions
      plot->setInteraction(QCP::iRangeDrag, zoomDragMode); // set graph drag
      plot->setInteraction(QCP::iRangeZoom, zoomDragMode); // set graph zoom
      plot->setInteraction(QCP::iMultiSelect, true); // set multiple selection
      plot->setInteraction(QCP::iSelectItems, true); // set markers selection
      plot->setInteraction(QCP::iSelectPlottables, true); // enable plottable selection

      // enable legend
      QFont legendFont = QFont("Verdana", 10);

      plot->legend->setVisible(true);
      plot->legend->setFont(legendFont);
      plot->legend->setTextColor(Theme::defaultTextColor);
      plot->legend->setBorderPen(QPen(Theme::defaultAxisPen));
      plot->legend->setBrush(QBrush(Qt::transparent));
      plot->legend->setFillOrder(QCPLegend::foColumnsFirst);
      plot->legend->setMargins(QMargins(8, 6, 8, 5));
      plot->legend->setColumnSpacing(2);

      auto *legendLayout = new QCPLayoutGrid();
      legendLayout->setMargins(QMargins(25, 0, 10, 5));
      legendLayout->setColumnStretchFactor(0, 0.001);
      legendLayout->addElement(0, 0, plot->legend);
      plot->plotLayout()->addElement(1, 0, legendLayout);
      plot->plotLayout()->setRowStretchFactor(1, 0.001);

      // setup axis drag and zoom
      plot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical); // drag horizontal and vertical axes
      plot->axisRect()->setRangeZoom(Qt::Horizontal); // only zoom horizontal axis
      plot->axisRect()->setRangeZoomFactor(0.65, 0.75);

      // setup time axis
      //      plot->xAxis->setPadding(0);
      plot->xAxis->setLabelColor(Theme::defaultTextColor);
      plot->xAxis->setLabelFont(Theme::defaultTextFont);
      plot->xAxis->setBasePen(Theme::defaultAxisPen);
      plot->xAxis->setTickPen(Theme::defaultTickPen);
      plot->xAxis->setTickLabelColor(Qt::white);
      plot->xAxis->setTickLabelPadding(1);
      plot->xAxis->setSubTicks(true);
      plot->xAxis->setSubTickPen(Theme::defaultTickPen);
      plot->xAxis->setRange(dataLowerRange, dataUpperRange);
      plot->xAxis->grid()->setPen(Theme::defaultGridPen);
      plot->xAxis->grid()->setSubGridPen(Theme::defaultGridPen);

      // setup Y axis
      //      plot->yAxis->setPadding(0);
      plot->yAxis->setLabelColor(Theme::defaultTextColor);
      plot->yAxis->setLabelFont(Theme::defaultTextFont);
      plot->yAxis->setBasePen(Theme::defaultAxisPen);
      plot->yAxis->setTickPen(Theme::defaultTickPen);
      plot->yAxis->setTickLabelColor(Qt::white);
      plot->yAxis->setTickLabelPadding(2);
      plot->yAxis->setSubTicks(true);
      plot->yAxis->setSubTickPen(Theme::defaultTickPen);
      plot->yAxis->setRange(dataLowerScale, dataUpperScale);
      plot->yAxis->grid()->setPen(Theme::defaultGridPen);
      plot->yAxis->grid()->setSubGridPen(Theme::defaultGridPen);

      // initialize cursor marker
      cursorMarker.reset(new MarkerCursor(plot));

      // initialize select marker
      rangeMarker.reset(new MarkerRange(plot));
      rangeMarker->setRangeVisible(true);
      rangeMarker->setMarkerPen(Theme::defaultSelectionPen);
      rangeMarker->setMarkerBrush(Theme::defaultSelectionBrush);
      rangeMarker->setSelectedPen(Theme::defaultActiveSelectionPen);
      rangeMarker->setSelectedBrush(Theme::defaultActiveSelectionBrush);

      // initialize zoom marker
      zoomMarker.reset(new MarkerZoom(plot));
      zoomMarker->setTotalRange(dataLowerRange, dataUpperRange);
      zoomMarker->setVisible(false);

      // prepare layout
      auto *layout = new QVBoxLayout(widget);

      layout->setSpacing(0);
      layout->setContentsMargins(0, 0, 0, 0);
      layout->addWidget(plot);

      // connect signals
      afterLayoutConnection = connect(plot, &QCustomPlot::afterLayout, [=]() {
         legendLayout->setMargins(QMargins(plot->axisRect()->margins().left(), 0, plot->axisRect()->margins().right(), 5));
      });

      legendClickConnection = connect(plot, &QCustomPlot::legendClick, [=](QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event) {
         legendClick(legend, item);
      });

      mouseMoveConnection = connect(plot, &QCustomPlot::mouseMove, [=](QMouseEvent *event) {
         mouseMove(event);
      });

      mousePressConnection = connect(plot, &QCustomPlot::mousePress, [=](QMouseEvent *event) {
         mousePress(event);
      });

      mouseReleaseConnection = connect(plot, &QCustomPlot::mouseRelease, [=](QMouseEvent *event) {
         mouseRelease(event);
      });

      mouseWheelConnection = connect(plot, &QCustomPlot::mouseWheel, [=](QWheelEvent *event) {
         mouseWheel(event);
      });

      //      mouseDoubleClickConnection = connect(plot, &QCustomPlot::mouseDoubleClick, [=](QMouseEvent *event) {
      //         mouseDoubleClick(event);
      //      });

      selectionChangedByUserConnection = connect(plot, &QCustomPlot::selectionChangedByUser, [=] {
         applySelection(widget->selectByUser());
      });

      selectionRectAcceptedConnection = connect(plot->selectionRect(), &QCPSelectionRect::accepted, [=](const QRect &rect) {
         applySelection(widget->selectByRect(rect.normalized()));
      });

      rangeChangedConnection = connect(plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &range) {
         applyRange(widget->rangeFilter(range));
      });

      scaleChangedConnection = connect(plot->yAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), [=](const QCPRange &range) {
         applyScale(widget->scaleFilter(range));
      });

      timer.start();
   }

   ~Impl()
   {
      disconnect(afterLayoutConnection);
      disconnect(legendClickConnection);
      disconnect(mouseMoveConnection);
      disconnect(mousePressConnection);
      disconnect(mouseReleaseConnection);
      disconnect(mouseWheelConnection);
      //      disconnect(mouseDoubleClickConnection);
      disconnect(selectionRectAcceptedConnection);
      disconnect(selectionChangedByUserConnection);
      disconnect(rangeChangedConnection);
      disconnect(scaleChangedConnection);
   }

   void refresh()
   {
      // fix range if current value is out
      applyRange(plot->xAxis->range());
      applyScale(plot->yAxis->range());

      // refresh graph
      plot->replot();
   }

   void clear() const
   {
      qInfo().noquote() << "clearing graph" << widget->objectName();

      // hide cursor and select markers
      cursorMarker->setVisible(false);
      rangeMarker->setVisible(false);

      // reset zoom marker range
      zoomMarker->setTotalRange(dataLowerRange, dataUpperRange);

      // trigger replot
      plot->replot();
   }

   void resetView() const
   {
      plot->xAxis->setRange(dataLowerRange, dataUpperRange);
      plot->yAxis->setRange(dataLowerScale, dataUpperScale);
      plot->replot();
   }

   QCPRange select(const QRect &rect) const
   {
      QRectF normalized(rect.normalized());

      double selectionStart = plot->xAxis->pixelToCoord(normalized.left());
      double selectionEnd = plot->xAxis->pixelToCoord(normalized.right());

      return {selectionStart, selectionEnd};
   }

   void zoomStepIn(const QPoint &pos, double factor) const
   {
      // compute current values
      double position = plot->xAxis->pixelToCoord(pos.x());
      double displaySize = viewUpperRange - viewLowerRange;
      double cursorOffset = (position - viewLowerRange) / displaySize;

      // compute new range values
      double rangeSize = (viewUpperRange - viewLowerRange) * (1 - factor);
      double lowerRange = position - rangeSize * cursorOffset;
      double upperRange = lowerRange + rangeSize;

      plot->xAxis->setRange(lowerRange, upperRange);

      cursorMarker->setPosition(position);

      plot->replot();
   }

   void zoomStepOut(const QPoint &pos, double factor) const
   {
      // compute current values
      double position = plot->xAxis->pixelToCoord(pos.x());
      double displaySize = viewUpperRange - viewLowerRange;
      double cursorOffset = (position - viewLowerRange) / displaySize;

      // compute new range values
      double rangeSize = (viewUpperRange - viewLowerRange) * (1 + factor);
      double lowerRange = position - rangeSize * cursorOffset;
      double upperRange = lowerRange + rangeSize;

      plot->xAxis->setRange(lowerRange, upperRange);

      cursorMarker->setPosition(position);

      plot->replot();
   }

   void zoomReset(const QPoint &pos = {}) const
   {
      plot->xAxis->setRange(dataLowerRange, dataUpperRange);

      if (!pos.isNull())
         cursorMarker->setPosition(plot->xAxis->pixelToCoord(pos.x()));

      plot->replot();
   }

   void zoomSelection(const QPoint &pos = {}) const
   {
      if (rangeMarker->visible())
      {
         double rangeSize = rangeMarker->width() * 1.05;
         double lowerRange = rangeMarker->center() - rangeSize / 2;
         double upperRange = rangeMarker->center() + rangeSize / 2;

         plot->xAxis->setRange(lowerRange, upperRange);
      }

      if (!pos.isNull())
         cursorMarker->setPosition(plot->xAxis->pixelToCoord(pos.x()));

      plot->replot();
   }

   QCPRange currentSelection() const
   {
      if (rangeMarker->visible())
         return {rangeMarker->start(), rangeMarker->end()};

      return {};
   }

   void applySelection(const QCPRange &selection) const
   {
      // update selection marker
      rangeMarker->setVisible(selection.size() > 0);
      rangeMarker->setRange(selection.lower, selection.upper);

      // refresh graph
      plot->replot();

      // trigger selection changed signal
      widget->selectionChanged(selection.lower, selection.upper);
   }

   void applyRange(const QCPRange &newRange)
   {
      // update range if are filtered
      if (plot->xAxis->range() != newRange)
         plot->xAxis->setRange(newRange);

      // update cursor marker keep its position relative to the new range
      double offset = (cursorMarker->position() - viewLowerRange) / (viewUpperRange - viewLowerRange);
      double cursor = newRange.lower + newRange.size() * offset;

      cursorMarker->setPosition(cursor);

      // update visible area
      viewLowerRange = newRange.lower;
      viewUpperRange = newRange.upper;

      // emit range signal
      widget->rangeChanged(newRange.lower, newRange.upper);
   }

   void applyScale(const QCPRange &newScale)
   {
      // update scale if are filtered
      if (plot->yAxis->range() != newScale)
         plot->yAxis->setRange(newScale);

      // update visible area
      viewLowerScale = newScale.lower;
      viewUpperScale = newScale.upper;

      // emit range signal
      widget->scaleChanged(newScale.lower, newScale.upper);
   }

   void setCenter(double value) const
   {
      QCPRange currentRange = plot->xAxis->range();

      auto length = float(currentRange.upper - currentRange.lower);

      plot->xAxis->setRange(value - length / 2, value + length / 2);
   }

   void setLeft(double value) const
   {
      QCPRange currentRange = plot->xAxis->range();

      auto length = float(currentRange.upper - currentRange.lower);

      plot->xAxis->setRange(value, value + length);
   }

   void setRight(double value) const
   {
      QCPRange currentRange = plot->xAxis->range();

      auto length = float(currentRange.upper - currentRange.lower);

      plot->xAxis->setRange(value - length, value);
   }

   void setDataRange(double lower, double upper)
   {
      dataLowerRange = lower;
      dataUpperRange = upper;
      zoomMarker->setTotalRange(dataLowerRange, dataUpperRange);

      // if view range exceeds data range, adjust it
      if (std::abs(upper - lower) < plot->xAxis->range().size())
         setViewRange(lower, upper);
   }

   void setDataScale(double lower, double upper)
   {
      dataLowerScale = lower;
      dataUpperScale = upper;

      // if view range exceeds data range, adjust it
      if (std::abs(upper - lower) < plot->yAxis->range().size())
         setViewScale(lower, upper);
   }

   void setViewRange(double lower, double upper) const
   {
      plot->xAxis->setRange(lower, upper);
      plot->replot();
   }

   void setViewScale(double lower, double upper) const
   {
      plot->yAxis->setRange(lower, upper);
      plot->replot();
   }

   void selectAndCenter(double from, double to) const
   {
      if (from - to == 0)
      {
         applySelection({});
         return;
      }

      if (from < viewLowerRange || to > viewUpperRange)
      {
         setCenter(double(from + to) / 2.0f);
      }

      applySelection({from, to});
   }

   void selectAndShow(double from, double to) const
   {
      if (from - to == 0)
      {
         applySelection({});
         return;
      }

      if (from < viewLowerRange)
      {
         setLeft(from);
      }
      else if (to > viewUpperRange)
      {
         setRight(to);
      }

      applySelection({from, to});
   }

   // event handlers
   void legendClick(QCPLegend *legend, QCPAbstractLegendItem *item)
   {
      qDebug() << "Legend clicked:" << item->rect().center();
   }

   void mouseEnter() const
   {
      widget->setFocus(Qt::MouseFocusReason);
   }

   void mouseLeave() const
   {
      widget->setFocus(Qt::NoFocusReason);
   }

   void mouseMove(QMouseEvent *event)
   {
      QCPAxisRect *axisRect = plot->axisRect();

      if (axisRect->rect().contains(event->pos()))
      {
         if (!cursorMarker->visible())
         {
            cursorMarker->setVisible(true);
            plot->setSelectionRectMode(zoomDragMode ? QCP::srmNone : QCP::srmCustom);
         }

         cursorMarker->setPosition(plot->xAxis->pixelToCoord(event->pos().x()));
      }
      else if (cursorMarker->visible())
      {
         cursorMarker->setVisible(false);
         plot->setSelectionRectMode(QCP::srmNone);
      }

      plot->replot();
   }

   void mousePress(QMouseEvent *event)
   {
      Qt::KeyboardModifiers keyModifiers = QGuiApplication::queryKeyboardModifiers();

      // if cursor marker is hidden, do nothing
      if (!cursorMarker->visible())
         return;

      if (!zoomDragMode)
      {
         // avoid change cursor when zoom / drag is enabled by Control Key
         if (keyModifiers & Qt::ControlModifier)
            return;

         if (event->buttons() & Qt::LeftButton)
            plot->setCursor(Qt::CrossCursor);
      }
   }

   void mouseRelease(QMouseEvent *event)
   {
      Qt::KeyboardModifiers keyModifiers = QGuiApplication::queryKeyboardModifiers();

      // if cursor marker is hidden, do nothing
      if (!cursorMarker->visible())
         return;

      if (!zoomDragMode)
      {
         // avoid change cursor when zoom / drag is enabled by Control Key
         if (keyModifiers & Qt::ControlModifier)
            return;
      }

      // restore arrow cursor for all modes
      plot->setCursor(Qt::ArrowCursor);
   }

   void mouseWheel(QWheelEvent *event) const
   {
      static qint64 lastWheelEvent = 0;

      Qt::KeyboardModifiers keyModifiers = QGuiApplication::queryKeyboardModifiers();

      if (!zoomDragMode)
      {
         // mouse wheel do zoom / drag when Control is active
         if (keyModifiers & Qt::ControlModifier)
            return;

         // get wheel steps
         double delta = event->angleDelta().y();
         double steps = delta / 120.0; // a single step delta is +/-120 usually

         // variable scroll speed
         // double elapsed = std::clamp(double(timer.elapsed() - lastWheelEvent), 10.0, 250.0);
         // double speed = 0.1500 - (0.1250 / 250) * elapsed;

         // process scroll
         double view = plot->xAxis->range().upper - plot->xAxis->range().lower;
         double range = dataUpperRange - dataLowerRange;
         double offset = (view / range) * steps * 0.0500;

         // update range to scroll view
         setViewRange(plot->xAxis->range().lower + offset, plot->xAxis->range().upper + offset);

         // update elapsed time
         lastWheelEvent = timer.elapsed();
      }
   }

   void mouseDoubleClick(QMouseEvent *event)
   {
      if (event->button() == Qt::LeftButton)
      {
         if (rangeMarker->selectTest(event->pos()) > 0)
         {
            zoomSelection(event->pos());
         }
         else
         {
            zoomStepIn(event->pos(), ZOOM_STEP_FACTOR);
         }
      }
      else if (event->button() == Qt::RightButton)
      {
         zoomStepOut(event->pos(), ZOOM_STEP_FACTOR);
      }
   }

   void keyPress(QKeyEvent *event) const
   {
      if (zoomDragMode)
      {
         if (event->modifiers() & Qt::ControlModifier)
         {
            plot->setCursor(Qt::CrossCursor);
            plot->setSelectionRectMode(QCP::srmCustom);
         }
      }
      else
      {
         // enable zoom and drag on Control press
         if (event->modifiers() & Qt::ControlModifier)
         {
            plot->setCursor(Qt::SizeAllCursor);
            plot->setInteraction(QCP::iRangeZoom, true);
            plot->setInteraction(QCP::iRangeDrag, true);
            plot->setSelectionRectMode(QCP::srmNone);
         }
      }
   }

   void keyRelease(QKeyEvent *event)
   {
      if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Z)
      {
         // reset state on ESC key
         if (event->key() == Qt::Key_Escape)
         {
            // reset default zoom-drag mode
            zoomDragMode = DEFAULT_ZOOM_DRAG;

            // clear selection
            applySelection(widget->selectByRect({}));
         }

         // toggle zoom-drag mode on Z key
         else
         {
            zoomDragMode = !zoomDragMode;
         }

         qInfo() << "Zoom-drag mode:" << zoomDragMode;

         plot->setInteraction(QCP::iRangeDrag, zoomDragMode); // set graph drag
         plot->setInteraction(QCP::iRangeZoom, zoomDragMode); // set graph zoom
         plot->setSelectionRectMode(zoomDragMode ? QCP::srmNone : QCP::srmCustom);
      }

      if (zoomDragMode)
      {
         if (!(event->modifiers() & Qt::ControlModifier))
         {
            plot->setCursor(Qt::ArrowCursor);
            plot->setSelectionRectMode(QCP::srmNone);
         }
      }
      else
      {
         // disable zoom and drag on Control release
         if (!(event->modifiers() & Qt::ControlModifier))
         {
            plot->setCursor(Qt::ArrowCursor);
            plot->setInteraction(QCP::iRangeZoom, false);
            plot->setInteraction(QCP::iRangeDrag, false);
            plot->setSelectionRectMode(QCP::srmCustom);
         }
      }
   }

   QCPRange selectByUser() const
   {
      double startTime = INT32_MAX;
      double endTime = INT32_MIN;

      for (int i = 0; i < plot->graphCount(); i++)
      {
         QCPGraph *graph = plot->graph(i);

         if (graph->selection().isEmpty())
            continue;

         QCPDataSelection selection = graph->selection();

         double selectionStartTime = graph->data()->at(selection.span().begin())->key;
         double selectionEndTime = graph->data()->at(selection.span().end())->key;

         if (selectionStartTime < startTime)
            startTime = selectionStartTime;

         if (selectionEndTime > endTime)
            endTime = selectionEndTime;

         selection.clear();
      }

      return startTime < endTime ? QCPRange(startTime, endTime) : QCPRange();
   }

   QCPRange selectByRect(const QRect &rect) const
   {
      return {
         plot->xAxis->pixelToCoord(rect.left()),
         plot->xAxis->pixelToCoord(rect.right())
      };
   }

   QCPRange rangeFilter(const QCPRange &newRange) const
   {
      QCPRange fixedRange(newRange);

      // check lower range limits
      if (fixedRange.lower < dataLowerRange || fixedRange.lower > dataUpperRange)
      {
         fixedRange.lower = dataLowerRange;
         fixedRange.upper = dataLowerRange + newRange.size();

         if (fixedRange.upper > dataUpperRange || qFuzzyCompare(newRange.size(), dataUpperRange - dataLowerRange))
            fixedRange.upper = dataUpperRange;
      }
      else if (fixedRange.upper > dataUpperRange || fixedRange.upper < dataLowerRange)
      {
         fixedRange.lower = dataUpperRange - newRange.size();
         fixedRange.upper = dataUpperRange;

         if (fixedRange.lower < dataLowerRange || qFuzzyCompare(newRange.size(), dataUpperRange - dataLowerRange))
            fixedRange.lower = dataLowerRange;
      }

      return fixedRange;
   }

   QCPRange scaleFilter(const QCPRange &newScale) const
   {
      QCPRange fixedScale(newScale);

      // check lower range limits
      if (fixedScale.lower < dataLowerScale || fixedScale.lower > dataUpperScale)
      {
         fixedScale.lower = dataLowerScale;
         fixedScale.upper = dataLowerScale + newScale.size();

         if (fixedScale.upper > dataUpperScale || qFuzzyCompare(newScale.size(), dataUpperScale - dataLowerScale))
            fixedScale.upper = dataUpperScale;
      }
      else if (fixedScale.upper > dataUpperScale || fixedScale.upper < dataLowerScale)
      {
         fixedScale.lower = dataUpperScale - newScale.size();
         fixedScale.upper = dataUpperScale;

         if (fixedScale.lower < dataLowerScale || qFuzzyCompare(newScale.size(), dataUpperScale - dataLowerScale))
            fixedScale.lower = dataLowerScale;
      }

      return fixedScale;
   }
};

AbstractPlotWidget::AbstractPlotWidget(QWidget *parent) : QWidget(parent), impl(new Impl(this))
{
}

void AbstractPlotWidget::setCenter(double value)
{
   impl->setCenter(value);
}

void AbstractPlotWidget::setCursorFormatter(const std::function<QString(double)> &formatter)
{
   impl->rangeMarker->setFormatter(formatter);
   impl->cursorMarker->setFormatter(formatter);
}

void AbstractPlotWidget::setRangeFormatter(const std::function<QString(double, double)> &formatter)
{
   impl->rangeMarker->setRangeFormatter(formatter);
}

void AbstractPlotWidget::setDataRange(double lower, double upper)
{
   impl->setDataRange(lower, upper);
}

void AbstractPlotWidget::setDataLowerRange(double value)
{
   impl->setDataRange(value, impl->dataUpperRange);
}

void AbstractPlotWidget::setDataUpperRange(double value)
{
   impl->setDataRange(impl->dataLowerRange, value);
}

void AbstractPlotWidget::setDataScale(double lower, double upper)
{
   impl->setDataScale(lower, upper);
}

void AbstractPlotWidget::setDataLowerScale(double value)
{
   impl->setDataScale(value, impl->dataUpperScale);
}

void AbstractPlotWidget::setDataUpperScale(double value)
{
   impl->setDataScale(impl->dataLowerScale, value);
}

void AbstractPlotWidget::setViewRange(double lower, double upper)
{
   impl->setViewRange(lower, upper);
}

void AbstractPlotWidget::setViewLowerRange(double value)
{
   impl->setViewRange(value, impl->viewUpperRange);
}

void AbstractPlotWidget::setViewUpperRange(double value)
{
   impl->setViewRange(impl->viewLowerScale, value);
}

void AbstractPlotWidget::setViewScale(double lower, double upper)
{
   impl->setViewScale(lower, upper);
}

void AbstractPlotWidget::setViewLowerScale(double value)
{
   impl->setViewScale(value, impl->viewUpperScale);
}

void AbstractPlotWidget::setViewUpperScale(double value)
{
   impl->setViewScale(impl->viewLowerScale, value);
}

bool AbstractPlotWidget::hasData() const
{
   return false;
}

double AbstractPlotWidget::dataSizeRange() const
{
   return impl->dataUpperRange - impl->dataLowerRange;
}

double AbstractPlotWidget::dataLowerRange() const
{
   return impl->dataLowerRange;
}

double AbstractPlotWidget::dataUpperRange() const
{
   return impl->dataUpperRange;
}

double AbstractPlotWidget::dataSizeScale() const
{
   return impl->dataUpperScale - impl->dataLowerScale;
}

double AbstractPlotWidget::dataLowerScale() const
{
   return impl->dataLowerScale;
}

double AbstractPlotWidget::dataUpperScale() const
{
   return impl->dataUpperScale;
}

double AbstractPlotWidget::viewSizeRange() const
{
   return impl->viewUpperRange - impl->viewLowerRange;
}

double AbstractPlotWidget::viewLowerRange() const
{
   return impl->viewLowerRange;
}

double AbstractPlotWidget::viewUpperRange() const
{
   return impl->viewUpperRange;
}

double AbstractPlotWidget::viewSizeScale() const
{
   return impl->viewUpperScale - impl->viewLowerScale;
}

double AbstractPlotWidget::viewLowerScale() const
{
   return impl->viewLowerScale;
}

double AbstractPlotWidget::viewUpperScale() const
{
   return impl->viewUpperScale;
}

void AbstractPlotWidget::start()
{
}

void AbstractPlotWidget::stop()
{
}

void AbstractPlotWidget::select(double from, double to)
{
   impl->selectAndCenter(from, to);
}

void AbstractPlotWidget::refresh()
{
   impl->refresh();
}

void AbstractPlotWidget::clear()
{
   impl->clear();
}

void AbstractPlotWidget::clearSelection()
{
   impl->applySelection({});
}

void AbstractPlotWidget::reset() const
{
   impl->resetView();
}

void AbstractPlotWidget::zoomReset() const
{
   impl->zoomReset();
}

void AbstractPlotWidget::zoomSelection() const
{
   impl->zoomSelection();
}

double AbstractPlotWidget::selectionSizeRange() const
{
   return impl->currentSelection().size();
}

double AbstractPlotWidget::selectionLowerRange() const
{
   return impl->currentSelection().lower;
}

double AbstractPlotWidget::selectionUpperRange() const
{
   return impl->currentSelection().upper;
}

// protected methods
QCustomPlot *AbstractPlotWidget::plot() const
{
   return impl->plot;
}

QCPRange AbstractPlotWidget::selectByUser()
{
   return impl->selectByUser();
}

QCPRange AbstractPlotWidget::selectByRect(const QRect &rect)
{
   return impl->selectByRect(rect);
}

QCPRange AbstractPlotWidget::rangeFilter(const QCPRange &range)
{
   return impl->rangeFilter(range);
}

QCPRange AbstractPlotWidget::scaleFilter(const QCPRange &range)
{
   return impl->scaleFilter(range);
}

void AbstractPlotWidget::enterEvent(QEnterEvent *event)
{
   impl->mouseEnter();
}

void AbstractPlotWidget::leaveEvent(QEvent *event)
{
   impl->mouseLeave();
}

void AbstractPlotWidget::keyPressEvent(QKeyEvent *event)
{
   impl->keyPress(event);
}

void AbstractPlotWidget::keyReleaseEvent(QKeyEvent *event)
{
   impl->keyRelease(event);
}

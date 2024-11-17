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

#ifndef NFC_LAB_ABSTRACTPLOTWIDGET_H
#define NFC_LAB_ABSTRACTPLOTWIDGET_H

#include <QWidget>

class QCustomPlot;
class QCPRange;

class AbstractPlotWidget : public QWidget
{
      Q_OBJECT

      struct Impl;

   public:

      explicit AbstractPlotWidget(QWidget *parent = nullptr);

      virtual void start();

      virtual void stop();

      virtual void select(double from, double to);

      virtual void refresh();

      virtual void clear();

      virtual void clearSelection();

      virtual void reset() const;

      virtual void zoomReset() const;

      virtual void zoomSelection() const;

      virtual bool hasData() const;

      virtual void setCenter(double value);

      void setCursorFormatter(const std::function<QString(double)> &formatter);

      void setRangeFormatter(const std::function<QString(double, double)> &formatter);

      virtual void setDataRange(double lower, double upper);

      virtual void setDataLowerRange(double value);

      virtual void setDataUpperRange(double value);

      virtual void setDataScale(double lower, double upper);

      virtual void setDataLowerScale(double value);

      virtual void setDataUpperScale(double value);

      virtual void setViewRange(double lower, double upper);

      virtual void setViewLowerRange(double value);

      virtual void setViewUpperRange(double value);

      virtual void setViewScale(double lower, double upper);

      virtual void setViewLowerScale(double value);

      virtual void setViewUpperScale(double value);

      double selectionSizeRange() const;

      double selectionLowerRange() const;

      double selectionUpperRange() const;

      double dataSizeRange() const;

      double dataLowerRange() const;

      double dataUpperRange() const;

      double dataSizeScale() const;

      double dataLowerScale() const;

      double dataUpperScale() const;

      double viewSizeRange() const;

      double viewLowerRange() const;

      double viewUpperRange() const;

      double viewSizeScale() const;

      double viewLowerScale() const;

      double viewUpperScale() const;

   protected:

      QCustomPlot *plot() const;

      virtual QCPRange selectByUser();

      virtual QCPRange selectByRect(const QRect &rect);

      virtual QCPRange rangeFilter(const QCPRange &range);

      virtual QCPRange scaleFilter(const QCPRange &range);

      // override events
      void enterEvent(QEnterEvent *event) override;

      void leaveEvent(QEvent *event) override;

      void keyPressEvent(QKeyEvent *event) override;

      void keyReleaseEvent(QKeyEvent *event) override;

   public:

      Q_SIGNAL void rangeChanged(double from, double to);

      Q_SIGNAL void scaleChanged(double from, double to);

      Q_SIGNAL void selectionChanged(double from, double to);

   private:

      QSharedPointer<Impl> impl;

};

#endif //NFC_LAB_ABSTRACTPLOTWIDGET_H

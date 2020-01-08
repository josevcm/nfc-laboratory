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

#ifndef PLOTVIEW_H
#define PLOTVIEW_H

#include <QObject>

#include <support/plot/qcustomplot.h>

#include <decoder/NfcFrame.h>

class PlotView: public QCustomPlot
{
   private:

      class Marker
      {
         public:

            Marker(QCPAxis *parentAxis);
            virtual ~Marker();

            QPen pen() const;
            void setPen(const QPen &pen);

            QBrush brush() const;
            void setBrush(const QBrush &brush);

            QString text() const;
            void setText(const QString &text);

            void setRange(double start, double end);

         protected:

            QCPAxis *mAxis;

            QPointer<QCPItemTracer> mStart;
            QPointer<QCPItemTracer> mMiddle;
            QPointer<QCPItemTracer> mEnd;

            QPointer<QCPItemText> mLabel;
            QPointer<QCPItemLine> mArrow;
      };

   public:

      PlotView();

      void addFrame(NfcFrame frame);

   public:

      Q_SIGNAL void selectionChanged();

   public Q_SLOTS:

      void plotSelectionChanged();
      void plotRangeChanged(const QCPRange &newRange, const QCPRange &oldRange);
      void plotMousePress(QMouseEvent *event);

   protected:

      void updateMarker(double begin, double end);

   private:

      // chart marker
      Marker *mTimeMarker;

};

#endif // PLOTVIEW_H

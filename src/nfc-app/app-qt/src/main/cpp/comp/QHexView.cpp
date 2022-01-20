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

#include <QPainter>
#include <QScrollBar>
#include <QPaintEvent>

#include "QHexView.h"

struct QHexView::Impl
{
   QHexView *widget;
   QByteArray data;

   int addrPos;
   int addrWidth;
   int addrHeight;

   int dataPos;
   int dataWidth;
   int dataHeight;

   int textPos;
   int textWidth;
   int textHeight;

   QFont addrFont {"Courier", 10, -1, false};
   QFont dataFont {"Courier", 10, -1, false};
   QFont textFont {"Courier", 10, -1, true};

   QColor splitColor {0x455364};

   explicit Impl(QHexView *widget) : widget(widget)
   {
      // set bold text for address
      addrFont.setBold(true);

      // get font metrics
      QFontMetrics addrFontMetrics(addrFont);
      QFontMetrics dataFontMetrics(dataFont);
      QFontMetrics textFontMetrics(textFont);

      // get positions
      addrPos = 0;
      addrWidth = addrFontMetrics.horizontalAdvance("0000");
      addrHeight = addrFontMetrics.height();

      dataPos = addrPos + addrWidth + 10;
      dataWidth = dataFontMetrics.horizontalAdvance("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
      dataHeight = addrFontMetrics.height();

      textPos = dataPos + dataWidth + 10;
      textWidth = textFontMetrics.horizontalAdvance("0123456789ABCDEF");
      textHeight = addrFontMetrics.height();
   }

   QString toHexString(const QByteArray &value, int from, int to) const
   {
      QString text;

      for (int i = from; i < value.count() && i < to; i++)
      {
         text.append(QString("%1 ").arg(value[i] & 0xff, 2, 16, QLatin1Char('0')));
      }

      return text.trimmed();
   }

   QString toAsciiString(const QByteArray &value, int from, int to) const
   {
      QString text;

      for (int i = from; i < value.count() && i < to; i++)
      {
         text.append(value[i] >= 0x20 ? QString("%1").arg((char) value[i]) : ".");
      }

      return text.trimmed();
   }
};

QHexView::QHexView(QWidget *parent) : QWidget(parent), impl(new Impl(this))
{
   setMinimumWidth(impl->textPos + impl->textWidth + 20);
   setAutoFillBackground(true);
}

void QHexView::clear()
{
   impl->data.clear();
}

void QHexView::setData(const QByteArray &data)
{
   impl->data = data;
}

void QHexView::paintEvent(QPaintEvent *event)
{
   QPainter painter(this);

   // fill address background
   painter.fillRect(QRect(impl->addrPos, event->rect().top(), impl->dataPos, height()), QColor {0x3b4252});

   // draw contents
   painter.setPen(palette().color(QPalette::WindowText));

   for (int addr = 0, line = 0; addr < impl->data.count(); addr += 16, line += impl->addrHeight)
   {
      painter.setFont(impl->addrFont);
      painter.drawText(QRect(impl->addrPos + 5, line, impl->addrWidth, impl->addrHeight), Qt::AlignTop, QString("%1").arg(addr, 4, 16, QChar('0')));

      painter.setFont(impl->dataFont);
      painter.drawText(QRect(impl->dataPos + 5, line, impl->dataWidth, impl->dataHeight), Qt::AlignTop, impl->toHexString(impl->data, addr, addr + 16));

      painter.setFont(impl->textFont);
      painter.drawText(QRect(impl->textPos + 5, line, impl->textWidth, impl->textHeight), Qt::AlignTop, impl->toAsciiString(impl->data, addr, addr + 16));
   }

   // draw hex / ascii split line
   painter.setPen(impl->splitColor);
   painter.drawLine(impl->textPos, event->rect().top(), impl->textPos, height());
}

void QHexView::keyPressEvent(QKeyEvent *event)
{

}

void QHexView::mouseMoveEvent(QMouseEvent *event)
{

}

void QHexView::mousePressEvent(QMouseEvent *event)
{

}
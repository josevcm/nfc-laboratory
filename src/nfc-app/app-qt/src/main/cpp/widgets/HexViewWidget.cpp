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

#include "HexViewWidget.h"

struct HexViewWidget::Impl
{
   HexViewWidget *widget;
   QByteArray data;

   int firstLine = 0;
   int lastLine = 0;
   int cursorPos = 0;
   int lineBytes = 16;

   int addrCoord;
   int addrWidth;

   int dataCoord;
   int dataWidth;

   int textCoord;
   int textWidth;

   QFont addrFont {"Courier", 10, -1, false};
   QFont dataFont {"Courier", 10, -1, false};
   QFont textFont {"Courier", 10, -1, true};

   int charWidth;
   int charHeight;

   QColor splitColor {0x455364};

   explicit Impl(HexViewWidget *widget) : widget(widget)
   {
      // set bold text for address
      addrFont.setBold(true);

      // get font metrics
      QFontMetrics addrFontMetrics(addrFont);
      QFontMetrics dataFontMetrics(dataFont);
      QFontMetrics textFontMetrics(textFont);

      // get positions
      addrCoord = 0;
      addrWidth = addrFontMetrics.horizontalAdvance("0000");

      dataCoord = addrCoord + addrWidth + 10;
      dataWidth = dataFontMetrics.horizontalAdvance("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");

      textCoord = dataCoord + dataWidth + 10;
      textWidth = textFontMetrics.horizontalAdvance("0123456789ABCDEF");

      // char metrics
      charWidth = addrFontMetrics.averageCharWidth();
      charHeight = addrFontMetrics.height();
   }

   void reset(const QByteArray &value)
   {
      data = value;
      cursorPos = 0;

      layout();

      widget->verticalScrollBar()->setValue(0);
      widget->update();
   }

   void layout()
   {
      int areaHeight = widget->viewport()->height();
      int dataHeight = ((data.count() / lineBytes) + (data.count() % lineBytes ? 1 : 0)) * charHeight;

      widget->verticalScrollBar()->setPageStep(areaHeight / charHeight);
      widget->verticalScrollBar()->setRange(0, (dataHeight - areaHeight) / charHeight + 1);

      firstLine = widget->verticalScrollBar()->value();
      lastLine = firstLine + areaHeight / charHeight;

      if (lastLine > data.count() / lineBytes)
         lastLine = (data.count() / lineBytes) + (data.count() % lineBytes ? 1 : 0);
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

HexViewWidget::HexViewWidget(QWidget *parent) : QAbstractScrollArea(parent), impl(new Impl(this))
{
   setMinimumWidth(impl->textCoord + impl->textWidth + 30);
}

void HexViewWidget::clear()
{
   impl->reset({});
}

void HexViewWidget::setData(const QByteArray &data)
{
   impl->reset(data);
}

void HexViewWidget::setCursor(int position)
{
   impl->cursorPos = std::clamp(position, 0, impl->data.count());
}

void HexViewWidget::paintEvent(QPaintEvent *event)
{
   QAbstractScrollArea::paintEvent(event);

   QPainter painter(viewport());

   // reprocess layout
   impl->layout();

   // fill address background
   painter.fillRect(QRect(impl->addrCoord, event->rect().top(), impl->dataCoord, height()), QColor {0x3b4252});

   // draw contents
   painter.setPen(palette().color(QPalette::WindowText));

   for (int addr = impl->firstLine * impl->lineBytes, line = impl->firstLine, lineCoord = 0; addr < impl->data.count() && line <= impl->lastLine; addr += impl->lineBytes, lineCoord += impl->charHeight, line++)
   {
      painter.setFont(impl->addrFont);
      painter.drawText(QRect(impl->addrCoord + 5, lineCoord, impl->addrWidth, impl->charHeight), Qt::AlignTop, QString("%1").arg(addr, 4, 16, QChar('0')));

      painter.setFont(impl->dataFont);
      painter.drawText(QRect(impl->dataCoord + 5, lineCoord, impl->dataWidth, impl->charHeight), Qt::AlignTop, impl->toHexString(impl->data, addr, addr + impl->lineBytes));

      painter.setFont(impl->textFont);
      painter.drawText(QRect(impl->textCoord + 5, lineCoord, impl->textWidth, impl->charHeight), Qt::AlignTop, impl->toAsciiString(impl->data, addr, addr + impl->lineBytes));
   }

   // draw hex / ascii split line
   painter.setPen(impl->splitColor);
   painter.drawLine(impl->textCoord, event->rect().top(), impl->textCoord, height());

   if (hasFocus())
   {
      int x = (impl->cursorPos % impl->lineBytes) * impl->charWidth * 3;
      int y = (impl->cursorPos / impl->lineBytes) * impl->charHeight;

      painter.fillRect(impl->dataCoord + x, y, 3, impl->charHeight, palette().color(QPalette::WindowText));
   }
}

void HexViewWidget::keyPressEvent(QKeyEvent *event)
{
   bool cursorVisible = false;

   if (event->matches(QKeySequence::MoveToNextChar))
   {
      setCursor(impl->cursorPos + 1);
//      resetSelection(impl->cursorOffset);
      cursorVisible = true;
   }

   else if (event->matches(QKeySequence::MoveToPreviousChar))
   {
      setCursor(impl->cursorPos - 1);
//      resetSelection(impl->cursorOffset);
      cursorVisible = true;
   }

   else if (event->matches(QKeySequence::MoveToEndOfLine))
   {
      setCursor(impl->cursorPos | (impl->lineBytes - 1));
//      resetSelection(impl->cursorOffset);
      cursorVisible = true;
   }

   else if (event->matches(QKeySequence::MoveToStartOfLine))
   {
      setCursor(impl->cursorPos | (impl->cursorPos % (impl->lineBytes)));
//      resetSelection(impl->cursorPos);
      cursorVisible = true;
   }

   else if (event->matches(QKeySequence::MoveToPreviousLine))
   {
      setCursor(impl->cursorPos - impl->lineBytes);
//      resetSelection(impl->cursorPos);
      cursorVisible = true;
   }

   else if (event->matches(QKeySequence::MoveToNextLine))
   {
      setCursor(impl->cursorPos + impl->lineBytes);
//      resetSelection(impl->cursorPos);
      cursorVisible = true;
   }

   viewport() -> update();
}

void HexViewWidget::mouseMoveEvent(QMouseEvent *event)
{

}

void HexViewWidget::mousePressEvent(QMouseEvent *event)
{

}
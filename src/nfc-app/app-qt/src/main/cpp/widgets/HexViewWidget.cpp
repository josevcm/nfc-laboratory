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
   int startSelection = 0;
   int endSelection = 0;
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
      startSelection = 0;
      endSelection = 0;

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
   impl->cursorPos = std::clamp(position, 0, impl->data.count() - 1);

   viewport()->update();
}

void HexViewWidget::setSelection(int start, int end)
{
   impl->startSelection = std::clamp(start, 0, impl->data.count());
   impl->endSelection = std::clamp(end, 0, impl->data.count());

   viewport()->update();
}

void HexViewWidget::paintEvent(QPaintEvent *event)
{
   QAbstractScrollArea::paintEvent(event);

   QPainter painter(viewport());

   QBrush defaultBrush = painter.brush();
   QBrush selectedBursh = QBrush(palette().color(QPalette::Highlight));

   // reprocess layout
   impl->layout();

   // fill address background
   painter.fillRect(QRect(impl->addrCoord, event->rect().top(), impl->dataCoord, height()), QColor {0x3b4252});

   // draw contents
   painter.setPen(palette().color(QPalette::WindowText));

   for (int addr = impl->firstLine * impl->lineBytes, line = impl->firstLine, lineCoord = 0; addr < impl->data.count() && line <= impl->lastLine; addr += impl->lineBytes, lineCoord += impl->charHeight, line++)
   {
      painter.setFont(impl->addrFont);
      painter.setBackgroundMode(Qt::TransparentMode);
      painter.drawText(QRect(impl->addrCoord + 5, lineCoord, impl->addrWidth, impl->charHeight), Qt::AlignTop, QString("%1").arg(addr, 4, 16, QChar('0')));

      // draw hex data
      painter.setFont(impl->dataFont);
      painter.setBackgroundMode(Qt::OpaqueMode);

      for (int i = 0, pos = addr, charCoord = 0; i < impl->lineBytes && pos < impl->data.count(); i++, pos++, charCoord += impl->charWidth * 3)
      {
         if (pos >= impl->startSelection && pos < impl->endSelection)
            painter.setBackground(selectedBursh);
         else
            painter.setBackground(defaultBrush);

         painter.drawText(QRect(impl->dataCoord + charCoord + 5, lineCoord, impl->charWidth * 2, impl->charHeight), Qt::AlignCenter, QString("%1").arg((int) impl->data[pos] & 0xff, 2, 16, QChar('0')));
      }

      // draw ascii data
      painter.setFont(impl->textFont);
      painter.setBackgroundMode(Qt::OpaqueMode);

      for (int i = 0, pos = addr, charCoord = 0; i < impl->lineBytes && pos < impl->data.count(); i++, pos++, charCoord += impl->charWidth)
      {
         if (pos >= impl->startSelection && pos < impl->endSelection)
            painter.setBackground(selectedBursh);
         else
            painter.setBackground(defaultBrush);

         painter.drawText(QRect(impl->textCoord + charCoord + 5, lineCoord, impl->charWidth, impl->charHeight), Qt::AlignCenter, (int) impl->data[pos] >= 0x20 ? QString("%1").arg((char) impl->data[pos]) : ".");
      }
   }

   // draw hex / ascii split line
   painter.setPen(impl->splitColor);
   painter.drawLine(impl->textCoord, event->rect().top(), impl->textCoord, height());

   if (hasFocus())
   {
      int x = (impl->cursorPos % impl->lineBytes) * impl->charWidth * 3;
      int y = (impl->cursorPos / impl->lineBytes - impl->firstLine) * impl->charHeight;

      painter.fillRect(impl->dataCoord + x + 5, y + impl->charHeight, impl->charWidth * 2, 3, palette().color(QPalette::WindowText));
   }
}

void HexViewWidget::keyPressEvent(QKeyEvent *event)
{
   if (event->matches(QKeySequence::MoveToNextChar))
   {
      setCursor(impl->cursorPos + 1);
   }

   else if (event->matches(QKeySequence::MoveToPreviousChar))
   {
      setCursor(impl->cursorPos - 1);
   }

   else if (event->matches(QKeySequence::MoveToEndOfLine))
   {
      setCursor(impl->cursorPos | (impl->lineBytes - 1));
   }

   else if (event->matches(QKeySequence::MoveToStartOfLine))
   {
      setCursor(impl->cursorPos | (impl->cursorPos % (impl->lineBytes)));
   }

   else if (event->matches(QKeySequence::MoveToPreviousLine))
   {
      setCursor(impl->cursorPos - impl->lineBytes);
   }

   else if (event->matches(QKeySequence::MoveToNextLine))
   {
      setCursor(impl->cursorPos + impl->lineBytes);
   }
}

void HexViewWidget::mouseMoveEvent(QMouseEvent *event)
{
   if (event->buttons() & Qt::LeftButton)
   {
      QPoint click = event->pos();

      if (click.x() < impl->dataCoord || click.x() > (impl->dataCoord + impl->dataWidth))
         return;

      int line = (click.y() / impl->charHeight);
      int byte = (click.x() - impl->dataCoord) / (impl->charWidth * 3);
      int addr = (impl->firstLine + line) * impl->lineBytes + byte;

      setSelection(impl->startSelection, addr + 1);
   }
}

void HexViewWidget::mousePressEvent(QMouseEvent *event)
{
   QPoint click = event->pos();

   if (click.x() < impl->dataCoord || click.x() > (impl->dataCoord + impl->dataWidth))
      return;

   int line = (click.y() / impl->charHeight);
   int byte = (click.x() - impl->dataCoord) / (impl->charWidth * 3);
   int addr = (impl->firstLine + line) * impl->lineBytes + byte;

   setCursor(addr);
   setSelection(addr, addr + 1);
}
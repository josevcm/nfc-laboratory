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

#include <QPainter>
#include <QScrollBar>
#include <QPaintEvent>
#include <QApplication>
#include <QClipboard>
#include <QTimer>

#include "HexViewWidget.h"

struct HexViewWidget::Impl
{
   HexViewWidget *widget;
   QByteArray data;

   int lineBytes = 16;

   int firstLine = 0;
   int lastLine = 0;

   int cursorPosition = -1;
   bool cursorVisible = true;

   int selectionStart = -1;
   int selectionEnd = -1;

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

   QTimer *blinkTimer;

   explicit Impl(HexViewWidget *widget) : widget(widget), blinkTimer(new QTimer(widget))
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

      // connect refresh timer signal
      QObject::connect(blinkTimer, &QTimer::timeout, [=]() {
         if (widget->hasFocus())
         {
            cursorVisible = !cursorVisible;
            widget->viewport()->update();
         }
      });

      // start timer
      blinkTimer->start(250);
   }

   void reset(const QByteArray &value)
   {
      data = value;

      cursorPosition = -1;
      selectionStart = -1;
      selectionEnd = -1;

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

   void paint(QPaintEvent *event)
   {
      QPainter painter(widget->viewport());

      QBrush defaultBrush = painter.brush();
      QBrush selectedBrush = QBrush(widget->palette().color(QPalette::Highlight));

      // reprocess layout
      layout();

      // fill address background
      painter.fillRect(QRect(addrCoord, event->rect().top(), dataCoord, widget->height()), QColor {0x3b4252});

      // draw contents
      painter.setPen(widget->palette().color(QPalette::WindowText));

      for (int addr = firstLine * lineBytes, line = firstLine, lineCoord = 0; addr < data.count() && line <= lastLine; addr += lineBytes, lineCoord += charHeight, line++)
      {
         painter.setFont(addrFont);
         painter.setBackgroundMode(Qt::TransparentMode);
         painter.drawText(QRect(addrCoord + 5, lineCoord, addrWidth, charHeight), Qt::AlignTop, QString("%1").arg(addr, 4, 16, QChar('0')));

         // draw hex data
         painter.setFont(dataFont);
         painter.setBackgroundMode(Qt::OpaqueMode);

         for (int i = 0, pos = addr, charCoord = 0; i < lineBytes && pos < data.count(); i++, pos++, charCoord += charWidth * 3)
         {
            if (pos >= selectionStart && pos <= selectionEnd)
               painter.setBackground(selectedBrush);
            else
               painter.setBackground(defaultBrush);

            painter.drawText(QRect(dataCoord + charCoord + 5, lineCoord, charWidth * 2, charHeight), Qt::AlignCenter, QString("%1").arg((int) data[pos] & 0xff, 2, 16, QChar('0')));
         }

         // draw ascii data
         painter.setFont(textFont);
         painter.setBackgroundMode(Qt::OpaqueMode);

         for (int i = 0, pos = addr, charCoord = 0; i < lineBytes && pos < data.count(); i++, pos++, charCoord += charWidth)
         {
            if (pos >= selectionStart && pos <= selectionEnd)
               painter.setBackground(selectedBrush);
            else
               painter.setBackground(defaultBrush);

            painter.drawText(QRect(textCoord + charCoord + 5, lineCoord, charWidth, charHeight), Qt::AlignCenter, (int) data[pos] >= 0x20 ? QString("%1").arg((char) data[pos]) : ".");
         }
      }

      // draw hex / ascii split line
      painter.setPen(splitColor);
      painter.drawLine(textCoord, event->rect().top(), textCoord, widget->height());

      if (widget->hasFocus())
      {
         if (cursorVisible && cursorPosition >= 0)
         {
            int x = (cursorPosition % lineBytes) * charWidth * 3;
            int y = (cursorPosition / lineBytes - firstLine) * charHeight;

            painter.fillRect(dataCoord + x + 5, y + charHeight, charWidth * 2, 3, widget->palette().color(QPalette::WindowText));
         }
      }
   }

   static QString toHexString(const QByteArray &value, int from, int to)
   {
      QString text;

      for (int i = from; i < value.count() && i < to; i++)
      {
         text.append(QString("%1 ").arg(value[i] & 0xff, 2, 16, QLatin1Char('0')));
      }

      return text.trimmed();
   }

   static QString toAsciiString(const QByteArray &value, int from, int to)
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
   setFocusPolicy(Qt::StrongFocus);
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
   if (impl->data.size() > 0)
   {
      impl->cursorPosition = std::clamp(position, 0, (int) impl->data.size() - 1);
      impl->cursorVisible = true;

      impl->blinkTimer->start(500);

      viewport()->update();
   }
}

void HexViewWidget::setSelection(int start, int end)
{
   if (start >= 0 && end >= start && impl->data.size() > 0)
   {
      impl->selectionStart = std::clamp(start, 0, (int) impl->data.size() - 1);
      impl->selectionEnd = std::clamp(end, 0, (int) impl->data.size() - 1);

      QClipboard *clipboard = QApplication::clipboard();

      clipboard->setText(impl->toHexString(impl->data, impl->selectionStart, impl->selectionEnd + 1));

      viewport()->update();
   }
}

void HexViewWidget::paintEvent(QPaintEvent *event)
{
   QAbstractScrollArea::paintEvent(event);

   impl->paint(event);
}

void HexViewWidget::keyPressEvent(QKeyEvent *event)
{
   QAbstractScrollArea::keyPressEvent(event);

   if (event->matches(QKeySequence::MoveToNextChar))
   {
      setCursor(impl->cursorPosition + 1);
   }

   else if (event->matches(QKeySequence::MoveToPreviousChar))
   {
      setCursor(impl->cursorPosition - 1);
   }

   else if (event->matches(QKeySequence::MoveToEndOfLine))
   {
      setCursor(impl->cursorPosition + impl->lineBytes - (impl->cursorPosition % impl->lineBytes) - 1);
   }

   else if (event->matches(QKeySequence::MoveToStartOfLine))
   {
      setCursor((impl->cursorPosition / impl->lineBytes) * impl->lineBytes);
   }

   else if (event->matches(QKeySequence::MoveToPreviousLine))
   {
      setCursor(impl->cursorPosition - impl->lineBytes);
   }

   else if (event->matches(QKeySequence::MoveToNextLine))
   {
      setCursor(impl->cursorPosition + impl->lineBytes);
   }
}

void HexViewWidget::mouseMoveEvent(QMouseEvent *event)
{
   QAbstractScrollArea::mouseMoveEvent(event);

   if (event->buttons() & Qt::LeftButton)
   {
      QPoint click = event->pos();

      if (click.x() > (impl->dataCoord + 5) && click.x() < (impl->dataCoord + impl->dataWidth + 5))
      {
         int line = (click.y() / impl->charHeight);
         int byte = (click.x() - impl->dataCoord - 5) / (impl->charWidth * 3);
         int addr = (impl->firstLine + line) * impl->lineBytes + byte;

         setSelection(impl->selectionStart, addr);
      }

      else if (click.x() > (impl->textCoord + 5) && click.x() < (impl->textCoord + impl->textWidth + 5))
      {
         int line = (click.y() / impl->charHeight);
         int byte = (click.x() - impl->textCoord - 5) / impl->charWidth;
         int addr = (impl->firstLine + line) * impl->lineBytes + byte;

         setSelection(impl->selectionStart, addr);
      }
   }
}

void HexViewWidget::mousePressEvent(QMouseEvent *event)
{
   QAbstractScrollArea::mousePressEvent(event);

   QPoint click = event->pos();

   if (click.x() > (impl->dataCoord + 5) && click.x() < (impl->dataCoord + impl->dataWidth + 5))
   {
      int line = (click.y() / impl->charHeight);
      int byte = (click.x() - impl->dataCoord - 5) / (impl->charWidth * 3);
      int addr = (impl->firstLine + line) * impl->lineBytes + byte;

      setCursor(addr);
      setSelection(addr, addr);
   }

   else if (click.x() > (impl->textCoord + 5) && click.x() < (impl->textCoord + impl->textWidth + 5))
   {
      int line = (click.y() / impl->charHeight);
      int byte = (click.x() - impl->textCoord - 5) / impl->charWidth;
      int addr = (impl->firstLine + line) * impl->lineBytes + byte;

      setCursor(addr);
      setSelection(addr, addr);
   }
   else
   {
      setSelection(0, 0);
   }
}
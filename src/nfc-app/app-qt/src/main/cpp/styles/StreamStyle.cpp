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

#include <QLabel>
#include <QPainter>
#include <QByteArray>

#include <nfc/Nfc.h>

#include <model/StreamModel.h>

#include "StreamStyle.h"

struct StreamStyle::Impl
{
   QRect type;
   QRect flag;

   QPixmap requestIcon;
   QPixmap responseIcon;
   QPixmap warningIcon;
   QPixmap encryptedIcon;

   QBrush selectedInactive;

   Impl() : type(0, 0, 16, 16),
            flag(20, 0, 16, 16),
            requestIcon(QPixmap(":/app_icons/arrow-green")),
            responseIcon(QPixmap(":/app_icons/arrow-red")),
            warningIcon(QPixmap(":/app_icons/warning-icon")),
            encryptedIcon(QPixmap(":/app_icons/encrypted-icon")),
            selectedInactive(QColor(0x37414f))
   {
   }
};

StreamStyle::StreamStyle(QObject *parent) : QStyledItemDelegate(parent), impl(new Impl())
{
}

StreamStyle::~StreamStyle() = default;

QString StreamStyle::displayText(const QVariant &value, const QLocale &locale) const
{
   if (value.type() == QVariant::ByteArray)
      return {value.toByteArray().toHex(' ')};

   return QStyledItemDelegate::displayText(value, locale);
}

void StreamStyle::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   QStyleOptionViewItem style = option;

   style.state &= ~QStyle::State_MouseOver;

   if (index.isValid())
   {
      if (style.state & QStyle::State_Selected)
      {
         if (style.state & QStyle::State_Active)
            painter->fillRect(option.rect, option.palette.highlight());
         else
            painter->fillRect(option.rect, impl->selectedInactive);
      }
      else
      {
         painter->fillRect(option.rect, option.palette.window());
      }

      switch (index.column())
      {
         case StreamModel::Columns::Flags:
         {
            QVariant data = index.data();

            if (data.isValid())
            {
               int flags = data.toInt() >> 8;
               int type = data.toInt() & 0xff;

               QRect typeRect = impl->type.adjusted(option.rect.x(), option.rect.y(), option.rect.x(), option.rect.y());
               QRect flagRect = impl->flag.adjusted(option.rect.x(), option.rect.y(), option.rect.x(), option.rect.y());

               if (type == nfc::FrameType::PollFrame)
                  painter->drawPixmap(typeRect, impl->requestIcon);
               else if (type == nfc::FrameType::ListenFrame)
                  painter->drawPixmap(typeRect, impl->responseIcon);

               if (flags & nfc::FrameFlags::Encrypted)
                  painter->drawPixmap(flagRect, impl->encryptedIcon);
               else if (flags & (nfc::FrameFlags::CrcError | nfc::FrameFlags::ParityError))
                  painter->drawPixmap(flagRect, impl->warningIcon);
            }

            return;
         }
      }
   }

   QStyledItemDelegate::paint(painter, style, index);
}




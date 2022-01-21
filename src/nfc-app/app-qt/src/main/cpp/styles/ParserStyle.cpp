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

#include <model/ParserModel.h>

#include <protocol/ProtocolFrame.h>

#include "ParserStyle.h"

struct ParserStyle::Impl
{
   QRect type;
   QRect flag;

   QPixmap fieldIcon;
   QPixmap requestIcon;
   QPixmap responseIcon;
   QPixmap warningIcon;
   QPixmap encryptedIcon;

   QBrush requestBackground;
   QBrush responseBackground;
   QBrush selectedInactive;

   Impl() : type(0, 0, 16, 16),
            flag(0, 0, 16, 16),
            fieldIcon(QPixmap(":/app_icons/arrow-blue")),
            requestIcon(QPixmap(":/app_icons/arrow-green")),
            responseIcon(QPixmap(":/app_icons/arrow-red")),
            warningIcon(QPixmap(":/app_icons/warning-icon")),
            encryptedIcon(QPixmap(":/app_icons/encrypted-icon")),
            requestBackground(QColor(0x37414f)),
            responseBackground(QColor(0x37414f)),
            selectedInactive(QColor(0x37414f))
   {
   }
};

ParserStyle::ParserStyle(QObject *parent) : QStyledItemDelegate(parent), impl(new Impl())
{
}

ParserStyle::~ParserStyle() = default;

void ParserStyle::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   QStyleOptionViewItem style = option;

   style.state &= ~QStyle::State_MouseOver;

   if (index.isValid())
   {
      if (auto frame = static_cast<ProtocolFrame *>(index.internalPointer()))
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
            if (frame->isFrameField() || frame->isFieldInfo())
               painter->fillRect(option.rect, option.palette.window());
            else if (frame->isRequestFrame())
               painter->fillRect(option.rect, impl->requestBackground);
            else
               painter->fillRect(option.rect, impl->responseBackground);
         }

         switch (index.column())
         {
            case ParserModel::Columns::Flags:
            {
               QRect typeRect = impl->type.adjusted(option.rect.x(), option.rect.y() + 2, option.rect.x(), option.rect.y() + 2);
               QRect flagRect = impl->flag.adjusted(option.rect.x(), option.rect.y() + 2, option.rect.x(), option.rect.y() + 2);

               if (frame->childDeep() == 1)
               {
                  if (frame->hasCrcError() || frame->hasParityError())
                     painter->drawPixmap(flagRect, impl->warningIcon);
                  else if (frame->isRequestFrame())
                     painter->drawPixmap(typeRect, impl->requestIcon);
                  else if (frame->isResponseFrame())
                     painter->drawPixmap(typeRect, impl->responseIcon);
               }

               return;
            }

            case ParserModel::Columns::Data:
            {
               break;
            }
         }
      }
   }

   QStyledItemDelegate::paint(painter, style, index);
}



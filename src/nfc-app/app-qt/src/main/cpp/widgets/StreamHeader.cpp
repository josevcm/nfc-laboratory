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

#include <QDebug>
#include <QPainter>
#include <QMouseEvent>

#include <model/StreamFilter.h>

#include <styles/Theme.h>

#include "StreamMenu.h"
#include "StreamWidget.h"
#include "StreamHeader.h"

struct StreamHeader::Impl
{
   StreamWidget *streamWidget;
   StreamHeader *streamHeader;

   QMap<int, bool> sortingEnabledMap;

   enum FilterState
   {
      None, Enabled, Active, Void
   };

   explicit Impl(StreamWidget *streamWidget, StreamHeader *streamHeader) :
         streamWidget(streamWidget),
         streamHeader(streamHeader)
   {
   }

   bool sortingEnabled(int section) const
   {
      return sortingEnabledMap.value(section, false);
   }

   bool filterEnabled(int section) const
   {
      auto *stream = dynamic_cast<StreamFilter *>(streamHeader->model());

      return stream != nullptr && stream->isEnabled();
   }

   FilterState filterState(int section) const
   {
      auto *stream = dynamic_cast<StreamFilter *>(streamHeader->model());

      if (!stream || !stream->isEnabled())
         return None;

      if (stream->hasFilters(section))
      {
         return stream->rowsAccepted(section) ? Active : Void;
      }

      return Enabled;
   }

   void showFilterMenu(int section) const
   {
      auto *stream = dynamic_cast<StreamFilter *>(streamHeader->model());

      // create menu
      StreamMenu filterMenu(stream, section, streamHeader);

      // align with the left edge of the column.
      int x = streamHeader->sectionViewportPosition(section);
      int y = streamHeader->viewport()->height() + 2;

      filterMenu.exec(streamHeader->viewport()->mapToGlobal(QPoint(x, y)));
   }
};

StreamHeader::StreamHeader(StreamWidget *parent) : QHeaderView(Qt::Horizontal, parent), impl(new Impl(parent, this))
{
   setSortIndicatorShown(true);
   setSectionsClickable(true);
   setSectionsMovable(true);
   setSortIndicator(0, Qt::AscendingOrder);
   setObjectName("StreamHeader");
}

void StreamHeader::setSortingEnabled(int section, bool enable)
{
   impl->sortingEnabledMap[section] = enable;
}

void StreamHeader::mouseReleaseEvent(QMouseEvent *event)
{
   int section = logicalIndexAt(qRound(event->position().x()));

   if (event->button() == Qt::LeftButton)
   {
      // check if filtering is enabled for clicked column to open the filter menu
      if (impl->filterEnabled(section))
      {
         // zone for filter icon
         QRect filterRect(sectionViewportPosition(section), 0, viewport()->height(), viewport()->height());

         // check if filter icon was clicked
         if (filterRect.contains(event->pos()))
         {
            impl->showFilterMenu(section);

            return;
         }
      }

      // check if sorting is enabled for clicked column
      if (!impl->sortingEnabled(section))
      {
         return;
      }
   }

   QHeaderView::mouseReleaseEvent(event);
}

void StreamHeader::paintSection(QPainter *painter, const QRect &rect, int section) const
{
   if (!rect.isValid())
      return;

   QAbstractItemModel *m = model();

   if (m == nullptr)
      return;

   // init style options
   QStyleOptionHeader opt;
   initStyleOption(&opt);

   QStyle::State state = QStyle::State_None;

   if (isEnabled())
      state |= QStyle::State_Enabled;

   if (window()->isActiveWindow())
      state |= QStyle::State_Active;

   if (sectionsClickable())
   {
//      if (logicalIndex == d->hover)
//         state |= QStyle::State_MouseOver;
//      if (logicalIndex == d->pressed)
//         state |= QStyle::State_Sunken;
//      else if (d->highlightSelected)
//      {
//         if (d->sectionIntersectsSelection(logicalIndex))
//            state |= QStyle::State_On;
//         if (d->isSectionSelected(logicalIndex))
//            state |= QStyle::State_Sunken;
//      }
   }

   // setup the style options structure
   QVariant textAlignment = m->headerData(section, orientation(), Qt::TextAlignmentRole);

   opt.rect = rect;
   opt.state |= state;
   opt.section = section;
   opt.orientation = orientation();
   opt.sortIndicator = QStyleOptionHeader::None;
   opt.text = m->headerData(section, orientation(), Qt::DisplayRole).toString();
   opt.iconAlignment = Qt::AlignVCenter;
   opt.textAlignment = Qt::Alignment(textAlignment.isValid() ? Qt::Alignment(textAlignment.toInt()) : defaultAlignment());

   // setup text elide mode
   if (textElideMode() != Qt::ElideNone)
      opt.text = opt.fontMetrics.elidedText(opt.text, textElideMode(), rect.width() - 4);

   // setup the style decoration
   QVariant decoration = m->headerData(section, opt.orientation, Qt::DecorationRole);

   opt.icon = qvariant_cast<QIcon>(decoration);

   if (opt.icon.isNull())
      opt.icon = qvariant_cast<QPixmap>(decoration);

   QVariant foregroundBrush = m->headerData(section, opt.orientation, Qt::ForegroundRole);

   if (foregroundBrush.canConvert<QBrush>())
      opt.palette.setBrush(QPalette::ButtonText, qvariant_cast<QBrush>(foregroundBrush));

   // setup brush
   QPointF oldBO = painter->brushOrigin();

   QVariant backgroundBrush = m->headerData(section, opt.orientation, Qt::BackgroundRole);

   if (backgroundBrush.canConvert<QBrush>())
   {
      opt.palette.setBrush(QPalette::Button, qvariant_cast<QBrush>(backgroundBrush));
      opt.palette.setBrush(QPalette::Window, qvariant_cast<QBrush>(backgroundBrush));
      painter->setBrushOrigin(opt.rect.topLeft());
   }

   // the section position
   int visual = visualIndex(section);

   if (count() == 1)
      opt.position = QStyleOptionHeader::OnlyOneSection;
   else if (visual == 0)
      opt.position = QStyleOptionHeader::Beginning;
   else if (visual == count() - 1)
      opt.position = QStyleOptionHeader::End;
   else
      opt.position = QStyleOptionHeader::Middle;

   // draw section rect
   style()->drawControl(QStyle::CE_HeaderSection, &opt, painter, this);

   // draw section label
   style()->drawControl(QStyle::CE_HeaderLabel, &opt, painter, this);

   // draw section sort icon
   QRect sortIconRect(rect.right() - rect.height() + 2, rect.top() + 2, rect.height() - 2, rect.height() - 2);

   // setup sort indicator
   if (isSortIndicatorShown() && sortIndicatorSection() == section)
   {
      switch (sortIndicatorOrder())
      {
         case Qt::AscendingOrder:
            Theme::sortUpIcon.paint(painter, sortIconRect);
            break;
         case Qt::DescendingOrder:
            Theme::sortDownIcon.paint(painter, sortIconRect);
            break;
      }
   }

   // draw section filter icon
   QRect filterIconRect(rect.left() + 3, rect.top() + 4, rect.height() - 8, rect.height() - 8);

   switch (impl->filterState(section))
   {
      case Impl::Enabled:
         Theme::filterEmptyIcon.paint(painter, filterIconRect);
         break;
      case Impl::Active:
         Theme::filterFilledIcon.paint(painter, filterIconRect);
         break;
      case Impl::Void:
         Theme::filterFilledVoidIcon.paint(painter, filterIconRect);
         break;
      case Impl::None:
         break;
   }

   painter->setBrushOrigin(oldBO);
}


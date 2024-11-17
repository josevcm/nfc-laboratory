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

#include "ProtocolFrame.h"

struct ProtocolFrame::Impl
{
   // frame flags
   int flags;

   // underline frame
   lab::RawFrame frame;

   // parent frame node
   ProtocolFrame *parent;

   // frame data contents
   QVector<QVariant> data;

   // frame childs
   QList<ProtocolFrame *> childs;

   //
   int start;
   int end;

   Impl(int flags, const QVector<QVariant> &data, const lab::RawFrame &frame) : flags(flags), frame(frame), parent(nullptr), data(data), start(0), end(frame.limit())
   {
   }

   Impl(int flags, const QVector<QVariant> &data, ProtocolFrame *parent, int start, int end) : flags(flags), parent(parent), data(data), start(start), end(end)
   {
   }

   ~Impl()
   {
      qDeleteAll(childs);
   }
};

ProtocolFrame::ProtocolFrame(const QVector<QVariant> &data, int flags, const lab::RawFrame &frame) :
   QObject(nullptr), impl(new Impl(flags, data, frame))
{
}

ProtocolFrame::ProtocolFrame(const QVector<QVariant> &data, int flags, ProtocolFrame *parent, int start, int end) :
   QObject(parent), impl(new Impl(flags, data, parent, start, end))
{
}

ProtocolFrame::~ProtocolFrame()
{
}

void ProtocolFrame::clearChilds()
{
   qDeleteAll(impl->childs);
   impl->childs.clear();
}

ProtocolFrame *ProtocolFrame::child(int row)
{
   if (row >= 0 && row < impl->childs.count())
      return impl->childs.at(row);

   return nullptr;
}

int ProtocolFrame::childDeep() const
{
   return impl->parent != nullptr ? impl->parent->childDeep() + 1 : 0;
}

int ProtocolFrame::childCount() const
{
   return impl->childs.count();
}

int ProtocolFrame::columnCount() const
{
   return impl->data.count();
}

ProtocolFrame *ProtocolFrame::appendChild(ProtocolFrame *item)
{
   item->impl->parent = this;

   impl->childs.append(item);

   return item;
}

ProtocolFrame *ProtocolFrame::prependChild(ProtocolFrame *item)
{
   item->impl->parent = this;

   impl->childs.prepend(item);

   return item;
}

bool ProtocolFrame::insertChild(int position, int count, int columns)
{
   if (position < 0 || position > impl->childs.size())
      return false;

   for (int row = 0; row < count; ++row)
   {
      QVector<QVariant> data(columns);
      auto *item = new ProtocolFrame(data, 0, this);
      impl->childs.insert(position, item);
   }

   return true;
}

lab::RawFrame &ProtocolFrame::frame() const
{
   if (impl->frame.isValid())
      return impl->frame;

   return impl->parent ? impl->parent->frame() : impl->frame;
}

QVariant ProtocolFrame::data(int column) const
{
   return impl->data.value(column);
}

void ProtocolFrame::set(int column, const QVariant &value)
{
   impl->data[column] = value;
}

ProtocolFrame *ProtocolFrame::parent() const
{
   return impl->parent;
}

void ProtocolFrame::setParent(ProtocolFrame *parent)
{
   impl->parent = parent;
}

int ProtocolFrame::row() const
{
   if (impl->parent)
      return impl->parent->impl->childs.indexOf(const_cast<ProtocolFrame *>(this));

   return -1;
}

int ProtocolFrame::rangeStart() const
{
   return impl->start;
}

int ProtocolFrame::rangeEnd() const
{
   return impl->end;
}

bool ProtocolFrame::isStartupFrame() const
{
   return impl->flags & StartupFrame || (impl->parent && impl->parent->isStartupFrame());
}

bool ProtocolFrame::isRequestFrame() const
{
   return impl->flags & RequestFrame || (impl->parent && impl->parent->isRequestFrame());
}

bool ProtocolFrame::isResponseFrame() const
{
   return impl->flags & ResponseFrame || (impl->parent && impl->parent->isResponseFrame());
}

bool ProtocolFrame::isExchangeFrame() const
{
   return (impl->flags & (RequestFrame | ResponseFrame)) == (RequestFrame | ResponseFrame) || (impl->parent && impl->parent->isExchangeFrame());
}

bool ProtocolFrame::isFrameField() const
{
   return impl->flags & FrameField;
}

bool ProtocolFrame::isFieldInfo() const
{
   return impl->flags & FieldInfo;
}

bool ProtocolFrame::hasParityError() const
{
   return impl->flags & ParityError;
}

bool ProtocolFrame::hasCrcError() const
{
   return impl->flags & CrcError;
}

bool ProtocolFrame::hasSyncError() const
{
   return impl->flags & SyncError;
}

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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include "ProtocolFrame.h"

struct ProtocolFrame::Impl
{
   // frame flags
   int flags;

   // underline frame
   nfc::NfcFrame frame;

   // parent frame node
   ProtocolFrame *parent;

   // frame data contents
   QVector<QVariant> data;

   // frame childs
   QList<ProtocolFrame *> childs;

   Impl(int flags, const QVector<QVariant> &data, const nfc::NfcFrame &frame) : flags(flags), frame(frame), parent(nullptr), data(data)
   {
   }

   Impl(int flags, const QVector<QVariant> &data, ProtocolFrame *parent) : flags(flags), parent(parent), data(data)
   {
   }

   ~Impl()
   {
      qDeleteAll(childs);
   }
};

ProtocolFrame::ProtocolFrame(const QVector<QVariant> &data, int flags, const nfc::NfcFrame &frame) :
      QObject(nullptr), impl(new Impl(flags, data, frame))
{
}

ProtocolFrame::ProtocolFrame(const QVector<QVariant> &data, int flags, ProtocolFrame *parent) :
      QObject(parent), impl(new Impl(flags, data, parent))
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

bool ProtocolFrame::insertChilds(int position, int count, int columns)
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

nfc::NfcFrame &ProtocolFrame::frame() const
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

bool ProtocolFrame::isRequestFrame() const
{
   return impl->flags & Flags::RequestFrame || (impl->parent && impl->parent->isRequestFrame());
}

bool ProtocolFrame::isResponseFrame() const
{
   return impl->flags & Flags::ResponseFrame || (impl->parent && impl->parent->isResponseFrame());
}

bool ProtocolFrame::isFrameField() const
{
   return impl->flags & Flags::FrameField;
}

bool ProtocolFrame::isFieldInfo() const
{
   return impl->flags & Flags::FieldInfo;
}

bool ProtocolFrame::hasParityError() const
{
   return impl->flags & Flags::ParityError;
}

bool ProtocolFrame::hasCrcError() const
{
   return impl->flags & Flags::CrcError;
}

bool ProtocolFrame::hasSyncError() const
{
   return impl->flags & Flags::SyncError;
}

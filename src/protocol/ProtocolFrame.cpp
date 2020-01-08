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

#include "ProtocolFrame.h"

ProtocolFrame::ProtocolFrame(const QVector<QVariant> &data, QObject *parent) :
   QObject(parent), mParent(nullptr), mData(data), mType(0), mFlags(0), mRepeated(0)
{
}

ProtocolFrame::ProtocolFrame(const QVector<QVariant> &data, int flags, int type, ProtocolFrame *parent) :
   QObject(parent), mParent(parent), mData(data), mType(type), mFlags(flags), mRepeated(0)
{
}

ProtocolFrame::~ProtocolFrame()
{
   qDeleteAll(mChilds);
}

void ProtocolFrame::clearChilds()
{
   mChilds.clear();
}

ProtocolFrame *ProtocolFrame::child(int row)
{
   if (row >= 0 && row < mChilds.count())
      return mChilds.at(row);

   return nullptr;
}

int ProtocolFrame::childDeep() const
{
   return mParent != nullptr? mParent->childDeep() + 1 : 0;
}

int ProtocolFrame::childCount() const
{
   return mChilds.count();
}

int ProtocolFrame::columnCount() const
{
   return mData.count();
}

void ProtocolFrame::appendChild(ProtocolFrame *item)
{
   item->mParent = this;

   mChilds.append(item);
}

void ProtocolFrame::prependChild(ProtocolFrame *item)
{
   item->mParent = this;

   mChilds.prepend(item);
}

bool ProtocolFrame::insertChilds(int position, int count, int columns)

{
   if (position < 0 || position > mChilds.size())
      return false;

   for (int row = 0; row < count; ++row)
   {
      QVector<QVariant> data(columns);
      ProtocolFrame *item = new ProtocolFrame(data, 0, 0, this);
      mChilds.insert(position, item);
   }

   return true;
}

QVariant ProtocolFrame::data(int column) const
{
   return mData.value(column);
}

void ProtocolFrame::set(int column, const QVariant &value)
{
   mData[column] = value;
}

ProtocolFrame *ProtocolFrame::parent() const
{
   return mParent;
}

int ProtocolFrame::row() const
{
   if (mParent)
      return mParent->mChilds.indexOf(const_cast<ProtocolFrame*>(this));

   return 0;
}

int ProtocolFrame::repeated() const
{
   return mRepeated;
}

int ProtocolFrame::addRepeated(int value)
{
   mRepeated += value;

   return mRepeated;
}

bool ProtocolFrame::isSenseFrame() const
{
   return mType == Type::SenseFrame;
}

bool ProtocolFrame::isSelectionFrame() const
{
   return mType == Type::SelectionFrame;
}

bool ProtocolFrame::isInformationFrame() const
{
   return mType == Type::InformationFrame;
}

bool ProtocolFrame::isAuthFrame() const
{
   return mType == Type::AuthFrame;
}

bool ProtocolFrame::isRequestFrame() const
{
   return mFlags & Flags::RequestFrame || (mParent && mParent->isRequestFrame());
}

bool ProtocolFrame::isResponseFrame() const
{
   return mFlags & Flags::ResponseFrame || (mParent && mParent->isResponseFrame());
}

bool ProtocolFrame::isFrameField() const
{
   return mFlags & Flags::FrameField;
}

bool ProtocolFrame::isFieldInfo() const
{
   return mFlags & Flags::FieldInfo;
}

bool ProtocolFrame::hasParityError() const
{
   return mFlags & Flags::ParityError;
}

bool ProtocolFrame::hasCrcError() const
{
   return mFlags & Flags::CrcError;
}


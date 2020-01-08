#include <QThread>

#include <protocol/ProtocolFrame.h>

#include "FrameStorage.h"

FrameStorage::FrameStorage(QObject *parent) : QObject(parent)
{
}

FrameStorage::~FrameStorage()
{
}

void FrameStorage::append(NfcFrame frame)
{
   mList.append(frame);
}

NfcFrame FrameStorage::at(int index) const
{
   return mList.at(index);
}

int FrameStorage::length() const
{
   return mList.size();
}

void FrameStorage::clear()
{
   mList.clear();
}


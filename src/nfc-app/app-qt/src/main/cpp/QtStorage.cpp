/*

This file is part of NFC-LABORATORY.

  Copyright (C) 2025 Jose Vicente Campos Martinez, <josevcm@gmail.com>

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
#include <QFile>
#include <QThread>
#include <QPointer>

#include <lab/data/RawFrame.h>
#include <lab/data/StreamTree.h>

#include <hw/SignalType.h>
#include <hw/SignalBuffer.h>

#include "QtApplication.h"

#include "QtStorage.h"

// define sample structure
typedef struct
{
   double timestamp;
   float value;
} sample_t;

struct QtStorage::Impl
{
   // configuration
   QThread *writer;

   // configuration
   QSettings settings;

   // cache for frames can be in memory
   QList<lab::RawFrame> frames;

   // cache for signal buffers is in disk
   QMap<QString, QFile *> signal;

   // cache for stream tree
   lab::StreamTree streamTree;

   explicit Impl(QObject *parent) : writer(new QThread(parent)), streamTree({0.000001 /*, 0.000010, 0.000100, 0.001000, 0.010000, 0.100000*/})
   {
      // switch cache to writer thread
      parent->moveToThread(writer);

      // start writer thread
      writer->start();
   }

   // destructor
   ~Impl()
   {
      // stop writer thread
      if (writer->isRunning())
      {
         writer->quit();
         writer->wait();
      }

      delete writer;
   }

   void append(const lab::RawFrame &frame)
   {

   }

   void append(const hw::SignalBuffer &buffer)
   {
      switch (buffer.type())
      {
         case hw::SignalType::SIGNAL_TYPE_LOGIC_SAMPLES:
            addLogicBuffer(buffer);
            break;
         case hw::SignalType::SIGNAL_TYPE_RADIO_SAMPLES:
            addRadioBuffer(buffer);
            break;
         default:
            break;
      }
   }

   void addLogicBuffer(const hw::SignalBuffer &buffer)
   {
      // QFile *file = cacheFile(type, id);
      //
      // if (!file->isOpen())
      //    return;
      //
      qInfo() << "Write cache buffe:" << buffer.offset();
      //
      // // write content to file
      // file->write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
   }

   void addRadioBuffer(const hw::SignalBuffer &buffer)
   {
      // double offset = buffer.offset();
      // double period = 1.0 / buffer.sampleRate();
      //
      // for (unsigned int i = 0; i < buffer.limit(); i += buffer.stride())
      // {
      //    streamTree.insert(offset + i * period, buffer[i]);
      // }
      //
      // streamTree.logInfo();

      // QFile *file = cacheFile(type, id);
      //
      // if (!file->isOpen())
      //    return;
      //
      // qInfo() << "Write cache buffe:" << buffer.offset();
      //
      // // write content to file
      // file->write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
   }

   QFile *cacheFile(const QString &type, unsigned int id)
   {
      // create cache name
      QString name = QString("%1-%2").arg(type, QString::number(id));

      if (signal.contains(name))
         return signal[name];

      // build fileName
      QString fileName = QtApplication::tempPath().absoluteFilePath(name + ".h5");

      // hid_t file_id = H5Fcreate(fileName.toUtf8(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

      // //
      // qInfo() << "Creating cache file:" << fileName;
      //
      // // store in cache
      // signal[name] = new QFile(fileName);
      //
      // // open the file
      // if (!signal[name]->open(QIODevice::ReadWrite | QIODevice::Truncate))
      //    qWarning() << "Failed to open cache file:" << fileName;

      return signal[name];
   }
};

QtStorage::QtStorage() : impl(new Impl(this))
{
}

void QtStorage::append(const lab::RawFrame &frame)
{
   impl->append(frame);
}

void QtStorage::append(const hw::SignalBuffer &buffer)
{
   impl->append(buffer);
}

void QtStorage::clear()
{
}

long QtStorage::frames() const
{
   return 0;
}

long QtStorage::samples() const
{
   return 0;
}

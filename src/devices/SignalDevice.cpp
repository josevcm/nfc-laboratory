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

#include "RecordDevice.h"
#include "AirspyDevice.h"
#include "RealtekDevice.h"
//#include "LimeDevice.h"

#include "SignalDevice.h"

SignalDevice::SignalDevice(QObject *parent) :
   QIODevice(parent)
{
}

SignalDevice::~SignalDevice()
{
}

int SignalDevice::read(char *data, int len)
{
   return this->readData(data, len);
}

int SignalDevice::write(char *data, int len)
{
   return this->writeData(data, len);
}

bool SignalDevice::isSequential() const
{
   return true;
}

SignalDevice *SignalDevice::newInstance(const QString &name, QObject *parent)
{
//   if (name.startsWith("lime://"))
//      return new LimeDevice(name, parent);

   if (name.startsWith("airspy://"))
      return new AirspyDevice(name, parent);

   if (name.startsWith("rtlsdr://"))
      return new RealtekDevice(name, parent);

   if (name.startsWith("record://"))
      return new RecordDevice(name, parent);

   return nullptr;
}

//int SignalDevice::read(int *data, int maxSamples)
//{
//	int readed = this->readData((char*) data, (quint64) maxSamples * sizeof(int));
//
//	return readed > 0 ? readed / sizeof(int) : readed;
//}
//
//int SignalDevice::write(int *data, int maxSamples)
//{
//	int written = this->writeData((char*) data, (quint64) maxSamples * sizeof(int));
//
//	return written > 0 ? written / sizeof(int) : written;
//}


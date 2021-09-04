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

#ifndef SDR_SIGNALDEVICE_H
#define SDR_SIGNALDEVICE_H

#include <string>

namespace sdr {

class SignalBuffer;

class SignalDevice
{
   public:

      enum OpenMode
      {
         Read = 1, Write = 2, Duplex = 3
      };

      enum SampleType
      {
         Integer = 1, Float = 2
      };

   public:

      virtual ~SignalDevice() = default;

      virtual bool open(OpenMode mode) = 0;

      virtual void close() = 0;

      virtual bool isOpen() const = 0;

      virtual bool isEof() const = 0;

      virtual bool isReady() const = 0;

      virtual bool isStreaming() const = 0;

      virtual const std::string &name() = 0;

      virtual const std::string &version() = 0;

      virtual int sampleSize() const = 0;

      virtual int setSampleSize(int newSampleSize) = 0;

      virtual long sampleRate() const = 0;

      virtual int setSampleRate(long mewSampleRate) = 0;

      virtual int sampleType() const = 0;

      virtual int setSampleType(int newSampleType) = 0;

      virtual int read(SignalBuffer &buffer) = 0;

      virtual int write(SignalBuffer &buffer) = 0;
};

}
#endif

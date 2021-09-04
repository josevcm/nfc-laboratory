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

#ifndef SDR_RECORDDEVICE_H
#define SDR_RECORDDEVICE_H

#include <vector>
#include <functional>

#include <sdr/SignalDevice.h>

namespace sdr {

class RecordDevice : public SignalDevice
{
      struct Impl;

   public:

      explicit RecordDevice(const std::string &name);

      const std::string &name() override;

      const std::string &version() override;

      bool open(OpenMode mode) override;

      void close() override;

      bool isOpen() const override;

      bool isEof() const override;

      bool isReady() const override;

      bool isStreaming() const override;

      int sampleCount() const;

      int sampleOffset() const;

      int sampleSize() const override;

      int setSampleSize(int value) override;

      long sampleRate() const override;

      int setSampleRate(long value) override;

      int sampleType() const override;

      int setSampleType(int value) override;

      int channelCount() const;

      void setChannelCount(int value);

      int read(SignalBuffer &buffer) override;

      int write(SignalBuffer &buffer) override;

   private:

      std::shared_ptr<Impl> impl;
};

}

#endif /* RECORDDEVICE_H */

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

#ifndef SDR_RADIODEVICE_H
#define SDR_RADIODEVICE_H

#include <map>
#include <vector>

#include <sdr/SignalDevice.h>

namespace sdr {

class RadioDevice : public SignalDevice
{
   public:

      typedef std::function<void(SignalBuffer &)> StreamHandler;

   public:

      virtual int start(StreamHandler handler) = 0;

      virtual int stop() = 0;

      virtual long centerFreq() const = 0;

      virtual int setCenterFreq(long value) = 0;

      virtual int tunerAgc() const = 0;

      virtual int setTunerAgc(int value) = 0;

      virtual int mixerAgc() const = 0;

      virtual int setMixerAgc(int value) = 0;

      virtual int gainMode() const = 0;

      virtual int setGainMode(int value) = 0;

      virtual int gainValue() const = 0;

      virtual int setGainValue(int value) = 0;

      virtual int decimation() const = 0;

      virtual int setDecimation(int value) = 0;

      virtual long samplesReceived() = 0;

      virtual long samplesDropped() = 0;

      virtual long samplesStreamed() = 0;

      virtual std::map<int, std::string> supportedSampleRates() const = 0;

      virtual std::map<int, std::string> supportedGainValues() const = 0;

      virtual std::map<int, std::string> supportedGainModes() const = 0;
};

}
#endif

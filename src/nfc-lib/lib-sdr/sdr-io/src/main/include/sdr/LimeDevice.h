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

#ifndef SDR_LIMEDEVICE_H
#define SDR_LIMEDEVICE_H

// https://wiki.myriadrf.org/LimeSDR-Mini#Software
// https://wiki.myriadrf.org/LimeSDR_Firmware_Management

#include <vector>
#include <functional>

#include <rt/FloatBuffer.h>

#include <sdr/RadioDevice.h>

namespace sdr {

class LimeDevice : public RadioDevice
{
   public:

      struct Impl;

      enum GainMode
      {
         Auto = 0, Manual = 1
      };

   public:

      explicit LimeDevice(const std::string &name);

      static std::vector<std::string> listDevices();

   public:

      const std::string &name() override;

      const std::string &version() override;

      bool open(OpenMode mode) override;

      void close() override;

      int start(StreamHandler handler) override;

      int stop() override;

      bool isOpen() const override;

      bool isEof() const override;

      bool isReady() const override;

      bool isStreaming() const override;

      int sampleSize() const override;

      int setSampleSize(int value) override;

      long sampleRate() const override;

      int setSampleRate(long value) override;

      int sampleType() const override;

      int setSampleType(int value) override;

      long streamTime() const override;

      int setStreamTime(long value) override;

      long centerFreq() const override;

      int setCenterFreq(long value) override;

      int tunerAgc() const override;

      int setTunerAgc(int value) override;

      int mixerAgc() const override;

      int setMixerAgc(int value) override;

      int gainMode() const override;

      int setGainMode(int value) override;

      int gainValue() const override;

      int setGainValue(int value) override;

      int biasTee() const override;

      int setBiasTee(int value) override;

      int decimation() const override;

      int setDecimation(int value) override;

      int testMode() const override;

      int setTestMode(int value) override;

      long samplesReceived() override;

      long samplesDropped() override;

      std::map<int, std::string> supportedSampleRates() const override;

      std::map<int, std::string> supportedGainValues() const override;

      std::map<int, std::string> supportedGainModes() const override;

      int read(SignalBuffer &buffer) override;

      int write(SignalBuffer &buffer) override;

   private:

      std::shared_ptr<Impl> impl;
};

}

#endif //SDR_LIMEDEVICE_H

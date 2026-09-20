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

#ifndef DEV_SIGMFDEVICE_H
#define DEV_SIGMFDEVICE_H

#include <hw/SignalDevice.h>

namespace hw {

// Lossless (relative to hardware precision) complex I/Q recorder using the SigMF
// (Signal Metadata Format) convention: a headerless <name>.sigmf-data raw binary file
// (interleaved ci16_le samples) plus a <name>.sigmf-meta JSON sidecar describing it.
// Only handles SIGNAL_TYPE_RADIO_IQ buffers; unlike RecordDevice it never derives or
// stores a real/magnitude-only signal.
class SigmfDevice : public SignalDevice
{
   struct Impl;

   public:

      explicit SigmfDevice(const std::string &name);

      bool open(Mode mode) override;

      void close() override;

      using Device::get;

      using Device::set;

      rt::Variant get(int id, int channel) const override;

      bool set(int id, const rt::Variant &value, int channel) override;

      bool isOpen() const override;

      bool isEof() const override;

      bool isReady() const override;

      long read(SignalBuffer &buffer) override;

      long write(const SignalBuffer &buffer) override;

   private:

      std::shared_ptr<Impl> impl;
};

}

#endif

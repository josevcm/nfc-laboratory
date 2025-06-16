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

#ifndef DEV_SIGNALBUFFER_H
#define DEV_SIGNALBUFFER_H

#include <rt/Buffer.h>

namespace hw {

class SignalBuffer : public rt::Buffer<float>
{
      struct Impl;

   public:

      SignalBuffer();

      SignalBuffer(const SignalBuffer &other);

      SignalBuffer(unsigned int length, unsigned int stride, unsigned int interleave, unsigned int samplerate, unsigned long long offset, unsigned int decimation, unsigned int type, unsigned int id = 0, void *context = nullptr);

      SignalBuffer(float *data, unsigned int length, unsigned int stride, unsigned int interleave, unsigned int samplerate, unsigned long long offset, unsigned int decimation, unsigned int type, unsigned int id = 0, void *context = nullptr);

      SignalBuffer &operator=(const SignalBuffer &other);

      // buffer id or key
      unsigned int id() const;

      // signal sample rate
      unsigned int sampleRate() const;

      // decimation factor
      unsigned int decimation() const;

      // sample offset
      unsigned long long offset() const;

   private:

      std::shared_ptr<Impl> impl;
};

}

#endif

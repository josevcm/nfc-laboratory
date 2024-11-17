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

#ifndef RT_BYTEBUFFER_H
#define RT_BYTEBUFFER_H

#include <rt/Buffer.h>

namespace rt {

class ByteBuffer : public Buffer<unsigned char>
{
   public:

      ByteBuffer() = default;

      ByteBuffer(const ByteBuffer &other) = default;

      explicit ByteBuffer(unsigned int capacity, int type = 0, int stride = 1, int interleave = 1, void *context = nullptr) : Buffer<unsigned char>(capacity, type, stride, interleave, context)
      {
      }

      explicit ByteBuffer(unsigned char *data, unsigned int capacity, int type = 0, int stride = 1, int interleave = 1, void *context = nullptr) : Buffer<unsigned char>(data, capacity, type, stride, interleave, context)
      {
      }
};

}

#endif

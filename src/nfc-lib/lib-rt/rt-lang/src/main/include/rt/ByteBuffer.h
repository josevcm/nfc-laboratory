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

      enum Endianness
      {
         BigEndian = 0,
         LittleEndian = 1,
      };

   public:

      ByteBuffer() = default;

      ByteBuffer(const ByteBuffer &other) = default;

      ByteBuffer(std::initializer_list<unsigned char> data, int type = 0, int stride = 1, int interleave = 1, void *context = nullptr) : Buffer(data, type, stride, interleave, context)
      {
      }

      explicit ByteBuffer(unsigned int capacity, int type = 0, int stride = 1, int interleave = 1, void *context = nullptr) : Buffer(capacity, type, stride, interleave, context)
      {
      }

      explicit ByteBuffer(const unsigned char *data, unsigned int capacity, int type = 0, int stride = 1, int interleave = 1, void *context = nullptr) : Buffer(data, capacity, type, stride, interleave, context)
      {
      }

      ByteBuffer operator^(const ByteBuffer &other) const
      {
         assert(state.capacity == other.state.capacity);

         ByteBuffer output(state.capacity);

         unsigned char *tmp = output.push(state.capacity);

         for (int i = 0; i < state.capacity; ++i)
            tmp[i] = alloc->data[i] ^ other.alloc->data[i];

         output.flip();

         return output;
      }

      unsigned int getInt(int size, int endianness = LittleEndian)
      {
         unsigned int value = 0;

         if (endianness == LittleEndian)
         {
            for (int i = 0; i < size; ++i)
               value |= (get() << (i * 8));
         }
         else
         {
            for (int i = 0; i < size; ++i)
               value = (value << 8) | get();
         }

         return value;
      }

      ByteBuffer &putInt(unsigned int value, int size, int endianness = LittleEndian)
      {
         if (endianness == LittleEndian)
         {
            for (int i = 0; i < size; ++i)
               put((value >> (i * 8)) & 0xFF);
         }
         else
         {
            for (int i = size - 1; i >= 0; --i)
               put((value >> (i * 8)) & 0xFF);
         }

         return *this;
      }

      unsigned long long getLong(int size, int endianness = LittleEndian)
      {
         unsigned long long value = 0;

         if (endianness == LittleEndian)
         {
            for (int i = 0; i < size; ++i)
               value |= (get() << (i * 8));
         }
         else
         {
            for (int i = 0; i < size; ++i)
               value = (value << 8) | get();
         }

         return value;
      }

      ByteBuffer &putLong(unsigned long long value, int size, int endianness = LittleEndian)
      {
         if (endianness == LittleEndian)
         {
            for (int i = 0; i < size; ++i)
               put((value >> (i * 8)) & 0xFF);
         }
         else
         {
            for (int i = size - 1; i >= 0; --i)
               put((value >> (i * 8)) & 0xFF);
         }

         return *this;
      }

      ByteBuffer slice(unsigned int offset, unsigned int length) const
      {
         assert(alloc != nullptr);
         assert(offset < state.capacity);
         assert(offset + length <= state.capacity);

         ByteBuffer copy(length);

         copy.put(alloc->data + offset, length);
         copy.flip();

         return copy;
      }

      static ByteBuffer rotateLeft(ByteBuffer input, unsigned int count)
      {
         ByteBuffer output(input.remaining());

         for (int i = 0; i < input.remaining(); i++)
         {
            output.put(input[(count + i) % input.remaining()]);
         }

         output.flip();

         return output;
      }

      static ByteBuffer rotateRight(ByteBuffer input, unsigned int count)
      {
         ByteBuffer output(input.remaining());

         for (int i = 0; i < input.remaining(); i++)
         {
            output.put(input[(input.remaining() - count + i) % input.remaining()]);
         }

         output.flip();

         return output;
      }

      static ByteBuffer random(const int size)
      {
         ByteBuffer buffer(size);

         for (int i = 0; i < size; i++)
            buffer.put(i + 1); //rand() % 256);

         buffer.flip();

         return buffer;
      }

      static ByteBuffer zero(const int size)
      {
         ByteBuffer buffer(size);

         for (int i = 0; i < size; i++)
            buffer.put(0);

         buffer.flip();

         return buffer;
      }
};

}

#endif

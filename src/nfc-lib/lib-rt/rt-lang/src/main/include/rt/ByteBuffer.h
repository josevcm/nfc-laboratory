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

#include <random>

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

      ByteBuffer(const std::initializer_list<unsigned char> &data) : Buffer(data, 0, 1, 1, nullptr)
      {
      }

      explicit ByteBuffer(const unsigned int capacity) : Buffer(capacity, 0, 1, 1, nullptr)
      {
      }

      explicit ByteBuffer(const unsigned char *data, const unsigned int capacity) : Buffer(data, capacity, 0, 1, 1, nullptr)
      {
      }

      ByteBuffer &operator=(const ByteBuffer &other)
      {
         if (&other == this)
            return *this;

         Buffer::operator=(other);

         return *this;
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

      ByteBuffer &padding(unsigned char value, unsigned int block)
      {
         fill(value, state.position % block == 0 ? 0 : 8 - state.position % block);
         return *this;
      }

      ByteBuffer getBuffer(unsigned int count)
      {
         ByteBuffer tmp(count);
         get(tmp);
         return tmp;
      }

      ByteBuffer peekBuffer(unsigned int count)
      {
         ByteBuffer tmp(count);
         peek(tmp);
         return tmp;
      }

      ByteBuffer popBuffer(unsigned int count)
      {
         ByteBuffer tmp(count);
         pop(tmp);
         return tmp;
      }

      unsigned int getInt(unsigned int size, Endianness endianness = LittleEndian)
      {
         unsigned int value = 0;
         unsigned char tmp[size];

         get(tmp, size);

         if (endianness == LittleEndian)
         {
            for (int i = 0; i < size; ++i)
               value |= (tmp[i] << (i * 8));
         }
         else
         {
            for (int i = 0; i < size; ++i)
               value = (value << 8) | tmp[i];
         }

         return value;
      }

      unsigned int peekInt(unsigned int size, Endianness endianness = LittleEndian) const
      {
         unsigned int value = 0;
         unsigned char tmp[size];

         peek(tmp, size);

         if (endianness == LittleEndian)
         {
            for (int i = 0; i < size; ++i)
               value |= (tmp[i] << (i * 8));
         }
         else
         {
            for (int i = 0; i < size; ++i)
               value = (value << 8) | tmp[i];
         }

         return value;
      }

      unsigned int popInt(unsigned int size, Endianness endianness = LittleEndian)
      {
         unsigned int value = 0;
         unsigned char tmp[size];

         pop(tmp, size);

         if (endianness == LittleEndian)
         {
            for (int i = 0; i < size; ++i)
               value |= (tmp[i] << (i * 8));
         }
         else
         {
            for (int i = 0; i < size; ++i)
               value = (value << 8) | tmp[i];
         }

         return value;
      }

      ByteBuffer &putInt(unsigned int value, unsigned int size, Endianness endianness = LittleEndian)
      {
         if (endianness == LittleEndian)
         {
            for (int i = 0; i < size; ++i)
               put((value >> (i * 8)) & 0xFF);
         }
         else
         {
            for (int i = 0; i < size; ++i)
                put((value >> ((size - i - 1)  * 8)) & 0xFF);
         }

         return *this;
      }

      unsigned long long getLong(unsigned int size, Endianness endianness = LittleEndian)
      {
         unsigned long long value = 0;
         unsigned char tmp[size];

         get(tmp, size);

         if (endianness == LittleEndian)
         {
            for (int i = 0; i < size; ++i)
               value |= (tmp[i] << (i * 8));
         }
         else
         {
            for (int i = 0; i < size; ++i)
               value = (value << 8) | tmp[i];
         }

         return value;
      }

      ByteBuffer &putLong(unsigned long long value, unsigned int size, Endianness endianness = LittleEndian)
      {
         if (endianness == LittleEndian)
         {
            for (int i = 0; i < size; ++i)
               put((value >> (i * 8)) & 0xFF);
         }
         else
         {
            for (int i = 0; i < size; ++i)
                put((value >> ((size - i - 1)  * 8)) & 0xFF);
         }

         return *this;
      }

      ByteBuffer copy() const
      {
         assert(alloc != nullptr);

         ByteBuffer copy(state.capacity);

         std::memcpy(copy.alloc->data, alloc->data, state.capacity);

         copy.state = state;
         copy.attrs = attrs;

         return copy;
      }

      ByteBuffer slice(int offset, unsigned int length) const
      {
         assert(alloc != nullptr);

         ByteBuffer copy(length);

         if (offset >= 0)
         {
            assert(state.position + offset + length <= state.limit);
            copy.put(alloc->data + state.position + offset, length);
         }
         else
         {
            assert(state.limit + offset + length <= state.limit);
            copy.put(alloc->data + state.limit + offset, length);
         }

         copy.flip();

         return copy;
      }

      static ByteBuffer rotateBytes(const ByteBuffer &input, Direction dir)
      {
         assert(dir == Left || dir == Right);

         ByteBuffer output(input.remaining());

         output.put(input).flip().rotate(dir);

         return output;
      }

      static ByteBuffer shiftBytes(const ByteBuffer &input, Direction dir)
      {
         assert(dir == Left || dir == Right);

         ByteBuffer output(input.remaining());

         output.put(input).flip().shift(dir);

         return output;
      }

      static ByteBuffer shiftBits(const ByteBuffer &input, Direction dir)
      {
         assert(dir == Left || dir == Right);

         ByteBuffer output(input.capacity());

         const unsigned char *src = input.data();
         unsigned char *dst = output.push(input.capacity());

         switch (dir)
         {
            case Left:
            {
               for (int i = 0; i < input.capacity(); ++i)
               {
                  dst[i] = src[i] << 1;

                  if (i > 0)
                     dst[i - 1] |= src[i] >> 7;
               }

               break;
            }

            case Right:
            {
               for (int i = 0; i < input.capacity(); ++i)
               {
                  dst[i] = src[i] >> 1;

                  if (i > 0)
                     dst[i] |= src[i - 1] << 7;
               }

               break;
            }
         }

         output.flip();

         return output;
      }

      static ByteBuffer random(const unsigned int size)
      {
         ByteBuffer buffer(size);

         for (int i = 0; i < size; i++)
            buffer.put(std::rand() % 256);

         buffer.flip();

         return buffer;
      }

      static ByteBuffer zero(const unsigned int size)
      {
         ByteBuffer buffer(size);
         buffer.fill(0, size).flip();
         return buffer;
      }

      static ByteBuffer empty()
      {
         return build(0);
      }

      static ByteBuffer build(unsigned int capacity = 1024)
      {
         return ByteBuffer(capacity);
      }
};

}

#endif

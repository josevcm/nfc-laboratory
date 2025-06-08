/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2025 Jose Vicente Campos Martinez, <josevcm@gmail.com>

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

#ifndef RT_BUFFER_H
#define RT_BUFFER_H

#include <cstring>
#include <memory>
#include <functional>

#include <rt/Alloc.h>
#include <rt/Pool.h>

#define ALLOC_ALIGNMENT 128

namespace rt {

template <class T>
class Buffer
{
   Alloc<T> alloc;

   struct State
   {
      unsigned int position; // current data position
      unsigned int capacity; // buffer data capacity
      unsigned int limit; // buffer data limit

      State(unsigned int position, unsigned int capacity, unsigned int limit) : position(position), capacity(capacity), limit(limit)
      {
      }

   } state;

   struct Attrs
   {
      void *context; // context payload
      unsigned int type; // data type
      unsigned int stride; // data stride, how many data has in one chunk for all channels
      unsigned int interleave; // data interleave, how many consecutive data has per channel

      Attrs(unsigned int type, unsigned int stride, unsigned int interleave, void *context) : type(type), stride(stride), interleave(interleave), context(context)
      {
      }

   } attrs;

   static Pool<T> pool;

   public:

      Buffer() : state(0, 0, 0), attrs(0, 0, 0, nullptr)
      {
      }

      Buffer(const Buffer &other) : alloc(other.alloc), state(other.state), attrs(other.attrs)
      {
      }

      explicit Buffer(T *data, unsigned int capacity, unsigned int type = 0, unsigned int stride = 1, unsigned int interleave = 1, void *context = nullptr) : Buffer(capacity, type, stride, interleave, context)
      {
         if (data && capacity)
            put(data, capacity).flip();
      }

      explicit Buffer(unsigned int capacity, unsigned int type = 0, unsigned int stride = 1, unsigned int interleave = 1, void *context = nullptr) : state(0, capacity, capacity), attrs(type, stride, interleave, context)
      {
         alloc = pool.acquire(capacity, ALLOC_ALIGNMENT);
      }

      ~Buffer()
      {
         pool.release(alloc);
      }

      Buffer &operator=(const Buffer &other)
      {
         if (&other == this)
            return *this;

         alloc = other.alloc;
         state = other.state;
         attrs = other.attrs;

         return *this;
      }

      bool operator==(const Buffer &other) const
      {
         if (this == &other)
            return true;

         if (state.limit != other.state.limit || state.position != other.state.position)
            return false;

         if (alloc.data == other.alloc.data)
            return true;

         return std::memcmp(alloc.data + state.position, other.alloc.data + state.position, state.limit) == 0;
      }

      bool operator!=(const Buffer &other) const
      {
         return !operator==(other);
      }

      void reset()
      {
         pool.release(alloc);

         alloc = {};
         state = {0, 0, 0};
         attrs = {0, 0, 0, nullptr};
      }

      bool isValid() const
      {
         return alloc.data;
      }

      bool isEmpty() const
      {
         return state.position == state.limit;
      }

      bool isFull() const
      {
         return state.position == state.capacity;
      }

      unsigned int position() const
      {
         return state.position;
      }

      unsigned int limit() const
      {
         return state.limit;
      }

      unsigned int capacity() const
      {
         return state.capacity;
      }

      unsigned int available() const
      {
         return state.limit - state.position;
      }

      unsigned int elements() const
      {
         return alloc.data ? state.limit * attrs.interleave / attrs.stride : 0;
      }

      unsigned int stride() const
      {
         return alloc.data ? attrs.stride : 0;
      }

      unsigned int interleave() const
      {
         return alloc.data ? alloc->interleave : 0;
      }

      unsigned int size() const
      {
         return alloc.data ? state.limit * sizeof(T) : 0;
      }

      unsigned int chunk() const
      {
         return alloc.data ? attrs.stride * sizeof(T) : 0;
      }

      operator bool() const
      {
         return isValid();
      }

      unsigned int references() const
      {
         return alloc.data ? static_cast<unsigned int>(alloc->references) : 0;
      }

      unsigned int type() const
      {
         return alloc.data ? attrs.type : 0;
      }

      void *context() const
      {
         return alloc.data ? attrs.context : nullptr;
      }

      T *data() const
      {
         return alloc.data ? alloc.data : nullptr;
      }

      T *pull(unsigned int size)
      {
         if (alloc.data && state.position + size <= state.capacity)
         {
            T *ptr = alloc.data + state.position;
            state.position += size;
            return ptr;
         }

         return nullptr;
      }

      Buffer &resize(unsigned int newCapacity)
      {
         if (alloc.data)
         {
            const int count = std::min(newCapacity, state.limit);

            Alloc<T> newAlloc = pool.acquire(newCapacity, ALLOC_ALIGNMENT);

            std::memcpy(newAlloc.block, alloc.block, count * sizeof(T));

            pool.release(alloc);

            alloc = newAlloc;
            state.limit = newCapacity > state.limit ? state.limit : newCapacity;
            state.capacity = newCapacity;
         }

         return *this;
      }

      Buffer &clear()
      {
         if (alloc.data)
         {
            state.limit = state.capacity;
            state.position = 0;
         }

         return *this;
      }

      Buffer &flip()
      {
         if (alloc.data)
         {
            state.limit = state.position;
            state.position = 0;
         }

         return *this;
      }

      Buffer &rewind()
      {
         if (alloc.data)
            state.position = 0;

         return *this;
      }

      Buffer &get(T *data)
      {
         if (alloc && state.position < state.limit)
            *data = alloc.data[state.position++];

         return *this;
      }

      Buffer &put(const T *data)
      {
         if (alloc.data && state.position < state.limit)
            alloc.data[state.position++] = *data;

         return *this;
      }

      Buffer &get(T &value)
      {
         if (alloc.data && state.position < state.limit)
            value = alloc.data[state.position++];

         return *this;
      }

      Buffer &put(const T &value)
      {
         if (alloc.data && state.position < state.limit)
            alloc.data[state.position++] = value;

         return *this;
      }

      Buffer &get(T *data, unsigned int size)
      {
         if (alloc.data)
         {
            int count = std::min(size, state.limit - state.position);
            std::memcpy(data, alloc.data + state.position, count * sizeof(T));
            state.position += count;
         }

         return *this;
      }

      Buffer &put(const T *data, unsigned int size)
      {
         if (alloc.data)
         {
            int count = std::min(size, state.limit - state.position);
            std::memcpy(alloc.data + state.position, data, count * sizeof(T));
            state.position += count;
         }

         return *this;
      }

      template <typename E>
      E reduce(E value, const std::function<E(E, T)> &handler) const
      {
         if (alloc.data)
         {
            for (int i = state.position; i < state.limit; ++i)
               value = handler(value, alloc.data[i]);
         }

         return value;
      }

      void stream(const std::function<void(const T *, unsigned int)> &handler) const
      {
         if (alloc.data)
         {
            for (int i = state.position; i < state.limit; i += attrs.stride)
               handler(alloc.data + i, attrs.stride);
         }
      }

      T &operator[](unsigned int index)
      {
         return alloc.data[index];
      }

      const T &operator[](unsigned int index) const
      {
         return alloc.data[index];
      }
};

template <class T>
Pool<T> Buffer<T>::pool;

}

#endif

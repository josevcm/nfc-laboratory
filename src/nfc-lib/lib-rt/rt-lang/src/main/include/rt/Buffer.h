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

#ifndef RT_BUFFER_H
#define RT_BUFFER_H

#include <atomic>
#include <cstring>
#include <memory>
#include <functional>
#include <cstdlib>

#define BUFFER_ALIGNMENT 256

namespace rt {

template<class T>
class Buffer
{
   struct Alloc
      {
         T *data = nullptr; // aligned payload data pointer
         void *block = nullptr;  // raw memory block pointer
         void *context = nullptr; // context payload
         unsigned int type = 0; // data type
         unsigned int stride = 0; // data stride, how many data has in one chunk for all channels
         unsigned int interleave = 0; // data interleave, how many consecutive data has per channel
         std::atomic<int> references; // block reference count

         Alloc(unsigned int type, unsigned int capacity, unsigned int stride, unsigned int interleave, void *context) : data(nullptr), type(type), references(1), stride(stride), interleave(interleave), context(context)
         {
            // allocate raw memory including alignment space
            block = malloc(capacity * sizeof(T) + BUFFER_ALIGNMENT);

            // align data buffer
            data = (T *) ((reinterpret_cast<uintptr_t>(block) + BUFFER_ALIGNMENT) & ~(BUFFER_ALIGNMENT - 1));
         }

         ~Alloc()
         {
            // release allocated buffer
            free(block);
         }

         int attach()
         {
            return ++references;
         }

         int detach()
         {
            return --references;
         }

      } *alloc {};

      struct State
      {
         unsigned int position; // current data position
         unsigned int capacity; // buffer data capacity
         unsigned int limit; // buffer data limit

         State(unsigned int position, unsigned int capacity, unsigned int limit) : position(position), capacity(capacity), limit(limit)
         {
         }

      } state;

   public:

      Buffer() : alloc(nullptr), state(0, 0, 0)
      {
      }

      Buffer(const Buffer &other) : alloc(other.alloc), state(other.state)
      {
         if (alloc)
            alloc->attach();
      }

      explicit Buffer(T *data, unsigned int capacity, unsigned int type = 0, unsigned int stride = 1, unsigned int interleave = 1, void *context = nullptr) : state(0, capacity, capacity), alloc(new Alloc(type, capacity, stride, interleave, context))
      {
         if (data && capacity)
            put(data, capacity).flip();
      }

      explicit Buffer(unsigned int capacity, unsigned int type = 0, unsigned int stride = 1, unsigned int interleave = 1, void *context = nullptr) : state(0, capacity, capacity), alloc(new Alloc(type, capacity, stride, interleave, context))
      {
      }

      ~Buffer()
      {
         if (alloc && alloc->detach() == 0)
            delete alloc;
      }

      Buffer &operator=(const Buffer &other)
      {
         if (&other == this)
            return *this;

         if (alloc && alloc->detach() == 0)
            delete alloc;

         state = other.state;
         alloc = other.alloc;

         if (alloc)
            alloc->attach();

         return *this;
      }

      bool operator==(const Buffer &other) const
      {
         if (this == &other)
            return true;

         if (state.limit != other.state.limit || state.position != other.state.position)
            return false;

         if (alloc == other.alloc)
            return true;

         return std::memcmp(alloc->data + state.position, other.alloc->data + state.position, state.limit) == 0;
      }

      bool operator!=(const Buffer &other) const
      {
         return !operator==(other);
      }

      void reset()
      {
         if (alloc && alloc->detach() == 0)
            delete alloc;

         state = {0, 0, 0};
         alloc = {nullptr};
      }

      bool isValid() const
      {
         return alloc;
      }

      bool isEmpty() const
      {
         return state.position == state.limit;
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
         return alloc ? state.limit * alloc->interleave / alloc->stride : 0;
      }

      unsigned int stride() const
      {
         return alloc ? alloc->stride : 0;
      }

      unsigned int interleave() const
      {
         return alloc ? alloc->interleave : 0;
      }

      unsigned int size() const
      {
         return alloc ? state.limit * sizeof(T) : 0;
      }

      unsigned int chunk() const
      {
         return alloc ? alloc->stride * sizeof(T) : 0;
      }

      operator bool() const
      {
         return isValid();
      }

      unsigned int references() const
      {
         return alloc ? static_cast<unsigned int>(alloc->references) : 0;
      }

      unsigned int type() const
      {
         return alloc ? alloc->type : 0;
      }

      void *context() const
      {
         return alloc ? alloc->context : nullptr;
      }

      T *data() const
      {
         return alloc ? alloc->data : nullptr;
      }

      T *pull(unsigned int size)
      {
         if (alloc && state.position + size <= state.capacity)
         {
            T *ptr = alloc->data + state.position;

            state.position += size;

            return ptr;
         }

         return nullptr;
      }

      Buffer &resize(unsigned int newCapacity)
      {
         if (alloc)
         {
            auto data = new T[newCapacity];

            for (int i = 0; i < newCapacity && i < state.limit; i++)
            {
               data[i] = alloc->data[i];
            }

            delete[] alloc->data;

            alloc->data = data;
            state.limit = newCapacity > state.limit ? state.limit : newCapacity;
            state.capacity = newCapacity;
         }

         return *this;
      }

      Buffer &clear()
      {
         if (alloc)
         {
            state.limit = state.capacity;
            state.position = 0;
         }

         return *this;
      }

      Buffer &flip()
      {
         if (alloc)
         {
            state.limit = state.position;
            state.position = 0;
         }

         return *this;
      }

      Buffer &rewind()
      {
         if (alloc)
         {
            state.position = 0;
         }

         return *this;
      }

      Buffer &get(T *data)
      {
         if (alloc && state.position < state.limit)
         {
            *data = alloc->data[state.position++];
         }

         return *this;
      }

      Buffer &put(const T *data)
      {
         if (alloc && state.position < state.limit)
         {
            alloc->data[state.position++] = *data;
         }

         return *this;
      }

      Buffer &get(T &value)
      {
         if (alloc && state.position < state.limit)
         {
            value = alloc->data[state.position++];
         }

         return *this;
      }

      Buffer &put(const T &value)
      {
         if (alloc && state.position < state.limit)
         {
            alloc->data[state.position++] = value;
         }

         return *this;
      }

      Buffer &get(T *data, unsigned int size)
      {
         if (alloc)
         {
            for (int i = 0; state.position < state.limit && i < size; ++i, ++state.position)
            {
               data[i] = alloc->data[state.position];
            }
         }

         return *this;
      }

      Buffer &put(const T *data, unsigned int size)
      {
         if (alloc)
         {
            for (int i = 0; i < size && state.position < state.limit; ++i, ++state.position)
            {
               alloc->data[state.position] = data[i];
            }
         }

         return *this;
      }

      template<typename E>
      E reduce(E value, const std::function<E(E, T)> &handler) const
      {
         if (alloc)
         {
            for (int i = state.position; i < state.limit; ++i)
            {
               value = handler(value, alloc->data[i]);
            }
         }

         return value;
      }

      void stream(const std::function<void(const T *, unsigned int)> &handler) const
      {
         if (alloc)
         {
            for (int i = state.position; i < state.limit; i += alloc->stride)
            {
               handler(alloc->data + i, alloc->stride);
            }
         }
      }

      T &operator[](unsigned int index)
      {
         return alloc->data[index];
      }

      const T &operator[](unsigned int index) const
      {
         return alloc->data[index];
      }
};

}

#endif

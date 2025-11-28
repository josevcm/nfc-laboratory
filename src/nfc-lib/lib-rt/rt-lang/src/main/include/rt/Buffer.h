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

#include <cassert>
#include <cstring>
#include <memory>
#include <functional>

#include <rt/Heap.h>

#define ALLOC_ALIGNMENT 128

namespace rt {

template <class T>
class Buffer
{
   public:

      enum Direction
      {
         Left = 0,
         Right = 1
      };

   protected:

      std::shared_ptr<Alloc<T>> alloc;

      struct State
      {
         unsigned int position; // current data position
         unsigned int capacity; // buffer data capacity
         unsigned int limit; // buffer data limit
      } state;

      struct Attrs
      {
         unsigned int type; // data type
         unsigned int stride; // data stride, how many data has in one chunk for all channels
         unsigned int interleave; // data interleave, how many consecutive data has per channel
         void *context; // context payload
      } attrs;

   public:

      static Heap<T> heap;

      Buffer() : state {0, 0, 0}, attrs {0, 0, 0, nullptr}
      {
      }

      Buffer(const Buffer &other) : alloc(other.alloc), state(other.state), attrs(other.attrs)
      {
      }

      explicit Buffer(const T *data, unsigned int capacity, unsigned int type = 0, unsigned int stride = 1, unsigned int interleave = 1, void *context = nullptr) : Buffer(capacity, type, stride, interleave, context)
      {
         if (data && capacity)
         {
            put(data, capacity);
            flip();
         }
      }

      explicit Buffer(unsigned int capacity, unsigned int type = 0, unsigned int stride = 1, unsigned int interleave = 1, void *context = nullptr) : state {0, capacity, capacity}, attrs {type, stride, interleave, context}
      {
         alloc = heap.alloc(capacity, ALLOC_ALIGNMENT);
      }

      Buffer(std::initializer_list<T> data, unsigned int type = 0, unsigned int stride = 1, unsigned int interleave = 1, void *context = nullptr) : Buffer(data.size(), type, stride, interleave, context)
      {
         put(data);
         flip();
      }

      ~Buffer() = default;

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

         if (remaining() != other.remaining())
            return false;

         if (alloc == other.alloc)
            return true;

         return std::memcmp(ptr(), other.ptr(), remaining()) == 0;
      }

      bool operator!=(const Buffer &other) const
      {
         return !operator==(other);
      }

      void reset()
      {
         alloc.reset();
         state = {0, 0, 0};
         attrs = {0, 0, 0, nullptr};
      }

      bool isValid() const
      {
         return alloc != nullptr;
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

      unsigned int remaining() const
      {
         return state.limit - state.position;
      }

      unsigned int elements() const
      {
         return state.limit * attrs.interleave / attrs.stride;
      }

      unsigned int stride() const
      {
         return attrs.stride;
      }

      unsigned int interleave() const
      {
         assert(alloc != nullptr);
         return alloc->interleave;
      }

      unsigned int size() const
      {
         return state.limit * sizeof(T);
      }

      unsigned int chunk() const
      {
         return attrs.stride * sizeof(T);
      }

      unsigned int type() const
      {
         return attrs.type;
      }

      void *context() const
      {
         return attrs.context;
      }

      T *data() const
      {
         assert(alloc != nullptr);
         assert(alloc->data != nullptr);
         return alloc->data;
      }

      T *ptr() const
      {
         assert(alloc != nullptr);
         assert(alloc->data != nullptr);
         return alloc->data + state.position;
      }

      Buffer &resize(unsigned int newCapacity)
      {
         assert(alloc != nullptr);

         const int count = std::min(newCapacity, state.limit);

         Alloc<T> newAlloc = heap.acquire(newCapacity, ALLOC_ALIGNMENT);

         std::memcpy(newAlloc->data, alloc->data, count * sizeof(T));

         heap.release(alloc);

         alloc = newAlloc;
         state.limit = newCapacity > state.limit ? state.limit : newCapacity;
         state.capacity = newCapacity;

         return *this;
      }

      Buffer &clear()
      {
         assert(alloc != nullptr);

         state.limit = state.capacity;
         state.position = 0;

         return *this;
      }

      Buffer &fill(T value, unsigned int count)
      {
         assert(alloc != nullptr);
         assert(state.position + count <= state.limit);

         memset(alloc->data + state.position, value, count * sizeof(T));

         state.position += count;

         return *this;
      }

      Buffer &flip()
      {
         assert(alloc != nullptr);

         state.limit = state.position;
         state.position = 0;

         return *this;
      }

      Buffer &rewind()
      {
         assert(alloc != nullptr);

         state.position = 0;

         return *this;
      }

      Buffer &room(unsigned int size)
      {
         assert(alloc != nullptr);
         assert(state.limit + size <= state.capacity);

         state.limit += size;

         return *this;
      }

      /*
       * extract one element from head
       */
      T get()
      {
         assert(alloc != nullptr);
         assert(state.position < state.limit);

         return alloc->data[state.position++];
      }

      /*
       * read one element from head (without update head pointer)
       */
      T peek() const
      {
         assert(alloc != nullptr);
         assert(state.position < state.limit);

         return alloc->data[state.position];
      }

      /*
       * extract one element from tail
       */
      T pop()
      {
         assert(alloc != nullptr);
         assert(state.position < state.limit);

         return alloc->data[--state.limit];
      }

      /*
       * add one element to tail
       */
      Buffer &put(const T &value)
      {
         assert(alloc != nullptr);
         assert(state.position < state.limit);

         alloc->data[state.position++] = value;

         return *this;
      }

      /*
       * add element list to tail
       */
      Buffer &put(std::initializer_list<T> data)
      {
         for (auto b: data)
            put(b);

         return *this;
      }

      /*
       * extract elements from head
       */
      Buffer &get(T *data, unsigned int elements)
      {
         assert(alloc != nullptr);
         assert(elements <= state.limit - state.position);

         std::memcpy(data, alloc->data + state.position, elements * sizeof(T));

         state.position += elements;

         return *this;
      }

      /**
       Read elements from head (without update head pointer)
       */
      const Buffer &peek(T *data, unsigned int elements) const
      {
         assert(alloc != nullptr);
         assert(elements <= state.limit - state.position);

         std::memcpy(data, alloc->data + state.position, elements * sizeof(T));

         return *this;
      }

      /*
       * extract elements from tail
       */
      Buffer &pop(T *data, unsigned int elements)
      {
         assert(alloc != nullptr);
         assert(elements <= state.limit - state.position);

         std::memcpy(data, alloc->data + state.limit - elements, elements * sizeof(T));

         state.limit -= elements;

         return *this;
      }

      /*
       * add elements from head
       */
      Buffer &put(const T *data, unsigned int elements)
      {
         assert(alloc != nullptr);
         assert(elements <= state.limit - state.position);

         std::memcpy(alloc->data + state.position, data, elements * sizeof(T));

         state.position += elements;

         return *this;
      }

      /*
       * extract buffer from head
       */
      Buffer &get(Buffer &data, unsigned int elements)
      {
         assert(alloc != nullptr);
         assert(data.remaining() >= elements);

         int count = std::min(elements, state.limit - state.position);
         data.put(alloc->data + state.position, count);
         data.flip();

         state.position += count;

         return *this;
      }

      /*
       * read buffer from head (without update head pointer)
       */
      const Buffer &peek(Buffer &data, unsigned int elements) const
      {
         assert(alloc != nullptr);
         assert(data.remaining() >= elements);

         int count = std::min(elements, state.limit - state.position);
         data.put(alloc->data + state.position, count);
         data.flip();

         return *this;
      }

      /*
       * extract buffer from tail
       */
      Buffer &pop(Buffer &data, unsigned int elements)
      {
         assert(alloc != nullptr);
         assert(elements <= state.limit - state.position);
         assert(data.remaining() >= elements);

         pop(data.ptr(), elements);

         return *this;
      }

      /*
       * add buffer to head
       */
      Buffer &put(const Buffer &data, unsigned int elements)
      {
         assert(alloc != nullptr);
         assert(elements <= state.limit - state.position);
         assert(data.remaining() >= elements);

         put(data.ptr(), elements);

         return *this;
      }

      /*
       * extract buffer from head
       */
      Buffer &get(Buffer &data)
      {
         return get(data, data.remaining());
      }

      /*
       * read buffer from head (without update head pointer)
       */
      const Buffer &peek(Buffer &data) const
      {
         return peek(data, data.remaining());
      }

      /*
       * get buffer from tail
       */
      Buffer &pop(Buffer &data)
      {
         return pop(data, data.remaining());
      }

      /*
       * add buffer to head
       */
      Buffer &put(const Buffer &data)
      {
         return put(data, data.remaining());
      }

      T *push(unsigned int elements, bool clear = false)
      {
         assert(alloc != nullptr);
         assert(state.position + elements <= state.capacity);

         if (clear)
            std::memset(alloc->data + state.position, 0, elements * sizeof(T));

         state.position += elements;

         return alloc->data + state.position - elements;
      }

      T *pull(unsigned int elements, bool clear = false)
      {
         assert(alloc != nullptr);
         assert(state.position - elements >= 0);

         state.position -= elements;

         if (clear)
            std::memset(alloc->data + state.position, 0, elements * sizeof(T));

         return alloc->data + state.position;
      }

      Buffer &rotate(Direction dir, unsigned int count = 1)
      {
         if (count > state.capacity)
            count %= state.capacity;

         T tmp[count];

         switch (dir)
         {
            case Left:
            {
               for (int i = 0; i < state.capacity; ++i)
               {
                  if (i < count)
                     tmp[i] = alloc->data[i];

                  if (i < state.capacity - count)
                     alloc->data[i] = alloc->data[i + count];
                  else
                     alloc->data[i] = tmp[i - state.capacity + count];
               }

               break;
            }

            case Right:
            {
               for (int i = state.capacity - 1; i >= 0; --i)
               {
                  if (i >= state.capacity - count)
                     tmp[i - state.capacity + count] = alloc->data[i];

                  if (i >= count)
                     alloc->data[i] = alloc->data[i - count];
                  else
                     alloc->data[i] = tmp[i];
               }

               break;
            }
         }

         return *this;
      }

      Buffer &shift(Direction dir, unsigned int count = 1)
      {
         if (count > state.capacity)
            count %= state.capacity;

         switch (dir)
         {
            case Left:
            {
               for (int i = 0; i < state.capacity; ++i)
               {
                  if (i < state.capacity - count)
                     alloc->data[i] = alloc->data[i + count];
                  else
                     alloc->data[i] = 0;
               }

               break;
            }

            case Right:
            {
               for (int i = state.capacity - 1; i >= 0; --i)
               {
                  if (i >= count)
                     alloc->data[i] = alloc->data[i - count];
                  else
                     alloc->data[i] = 0;
               }

               break;
            }
         }

         return *this;
      }

      Buffer &set(const Buffer &data, unsigned int offset, unsigned int elements)
      {
         assert(alloc != nullptr);
         assert(offset + elements <= state.capacity);
         assert(data.remaining() >= elements);

         T *src = data.ptr();

         for (int i = 0; i < elements; i++)
            alloc->data[offset + i] = src[i];

         return *this;
      }

      Buffer &set(const Buffer &data, unsigned int offset)
      {
         return set(data, offset, data.remaining());
      }

      Buffer &trim(unsigned int size)
      {
         assert(alloc != nullptr);
         assert(state.position <= state.limit - size);

         state.limit = state.limit - size;

         return *this;
      }

      template <typename E>
      E reduce(E value, const std::function<E(E, T)> &handler) const
      {
         assert(alloc != nullptr);

         for (int i = state.position; i < state.limit; ++i)
            value = handler(value, alloc->data[i]);

         return value;
      }

      void stream(const std::function<void(const T *, unsigned int)> &handler) const
      {
         assert(alloc != nullptr);

         for (int i = state.position; i < state.limit; i += attrs.stride)
            handler(alloc->data + i, attrs.stride);
      }

      T &operator[](unsigned int index)
      {
         assert(alloc != nullptr);

         return alloc->data[index];
      }

      const T &operator[](unsigned int index) const
      {
         assert(alloc != nullptr);

         return alloc->data[index];
      }
};

template <class T>
Heap<T> Buffer<T>::heap;

}

#endif

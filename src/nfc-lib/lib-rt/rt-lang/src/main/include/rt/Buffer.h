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
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#ifndef LANG_BUFFER_H
#define LANG_BUFFER_H

#include <atomic>
#include <memory>
#include <functional>

#define BUFFER_ALIGNMENT 256

namespace rt {

template<class T>
class Buffer
{
   private:

      struct Alloc
      {
         T *data = nullptr; // aligned payload data pointer
         void *block = nullptr;  // raw memory block pointer
         void *context = nullptr; // custom context payload
         unsigned int type = 0; // custom data type
         unsigned int stride = 0; // custom data stride
         std::atomic<int> references; // block reference count

         Alloc(unsigned int type, unsigned int capacity, unsigned int stride, void *context) : data(nullptr), type(type), references(1), stride(stride), context(context)
         {
            // allocate raw memory including alignment space
            block = malloc(capacity * sizeof(T) + BUFFER_ALIGNMENT);

            // align data buffer
            data = (T *) ((((uintptr_t) block) + BUFFER_ALIGNMENT) & ~(BUFFER_ALIGNMENT - 1));
         }

         ~Alloc()
         {
            // release allocated buffer
            free(block);
         }

         inline int attach()
         {
            return ++references;
         }

         inline int detach()
         {
            return --references;
         }

      } *alloc;

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

      Buffer() : state(0, 0, 0), alloc(nullptr)
      {
      }

      Buffer(const Buffer &other) : state(other.state), alloc(other.alloc)
      {
         if (alloc)
            alloc->attach();
      }

      explicit Buffer(T *data, unsigned int capacity, unsigned int type = 0, unsigned int stride = 1, void *context = nullptr) : state(0, capacity, capacity), alloc(new Alloc(type, capacity, stride, context))
      {
         if (data && capacity)
            put(data, capacity).flip();
      }

      explicit Buffer(unsigned int capacity, unsigned int type = 0, unsigned int stride = 1, void *context = nullptr) : state(0, capacity, capacity), alloc(new Alloc(type, capacity, stride, context))
      {
      }

      ~Buffer()
      {
         if (alloc && alloc->detach() == 0)
            delete alloc;
      }

      inline void reset()
      {
         if (alloc && alloc->detach() == 0)
            delete alloc;

         state = {0, 0, 0};
         alloc = {nullptr};
      }

      inline Buffer &operator=(const Buffer &other)
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

      inline bool isValid() const
      {
         return alloc;
      }

      inline bool isEmpty() const
      {
         return state.position == state.limit;
      }

      inline unsigned int position() const
      {
         return state.position;
      }

      inline unsigned int limit() const
      {
         return state.limit;
      }

      inline unsigned int capacity() const
      {
         return state.capacity;
      }

      inline unsigned int available() const
      {
         return state.limit - state.position;
      }

      inline unsigned int elements() const
      {
         return alloc ? state.limit / alloc->stride : 0;
      }

      virtual inline operator bool() const
      {
         return alloc != nullptr;
      }

      inline unsigned int references() const
      {
         return alloc ? (unsigned int) alloc->references : 0;
      }

      inline unsigned int type() const
      {
         return alloc ? alloc->type : 0;
      }

      inline unsigned int stride() const
      {
         return alloc ? alloc->stride : 0;
      }

      inline unsigned int size() const
      {
         return alloc ? state.capacity * sizeof(T) : 0;
      }

      inline void *context() const
      {
         return alloc ? alloc->context : nullptr;
      }

      inline T *data() const
      {
         return alloc ? alloc->data : nullptr;
      }

      inline T *pull(unsigned int size)
      {
         if (alloc && state.position + size <= state.capacity)
         {
            T *ptr = alloc->data + state.position;

            state.position += size;

            return ptr;
         }

         return nullptr;
      }

      inline Buffer<T> &resize(unsigned int newCapacity)
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

      inline Buffer<T> &clear()
      {
         if (alloc)
         {
            state.limit = state.capacity;
            state.position = 0;
         }

         return *this;
      }

      inline Buffer<T> &flip()
      {
         if (alloc)
         {
            state.limit = state.position;
            state.position = 0;
         }

         return *this;
      }

      inline Buffer<T> &rewind()
      {
         if (alloc)
         {
            state.position = 0;
         }

         return *this;
      }

      inline Buffer<T> &get(T *data)
      {
         if (alloc && state.position < state.limit)
         {
            *data = alloc->data[state.position++];
         }

         return *this;
      }

      inline Buffer<T> &put(const T *data)
      {
         if (alloc && state.position < state.limit)
         {
            alloc->data[state.position++] = *data;
         }

         return *this;
      }

      inline Buffer<T> &get(T &value)
      {
         if (alloc && state.position < state.limit)
         {
            value = alloc->data[state.position++];
         }

         return *this;
      }

      inline Buffer<T> &put(const T &value)
      {
         if (alloc && state.position < state.limit)
         {
            alloc->data[state.position++] = value;
         }

         return *this;
      }

      inline Buffer<T> &get(T *data, unsigned int size)
      {
         if (alloc)
         {
            for (int i = 0; state.position < state.limit && i < size; i++, state.position++)
            {
               data[i] = alloc->data[state.position];
            }
         }

         return *this;
      }

      inline Buffer<T> &put(const T *data, unsigned int size)
      {
         if (alloc)
         {
            for (int i = 0; i < size && state.position < state.limit; i++, state.position++)
            {
               alloc->data[state.position] = data[i];
            }
         }

         return *this;
      }

      template<typename E>
      inline E reduce(E value, const std::function<E(E, T)> &handler) const
      {
         if (alloc)
         {
            for (int i = state.position; i < state.limit; i++)
            {
               value = handler(value, alloc->data[i]);
            }
         }

         return value;
      }

      inline void stream(const std::function<void(const T *, unsigned int)> &handler) const
      {
         if (alloc)
         {
            for (int i = state.position; i < state.limit; i += alloc->stride)
            {
               handler(alloc->data + i, alloc->stride);
            }
         }
      }

      inline T &operator[](unsigned int index)
      {
         return alloc->data[index];
      }

      inline const T &operator[](unsigned int index) const
      {
         return alloc->data[index];
      }
};

}

#endif

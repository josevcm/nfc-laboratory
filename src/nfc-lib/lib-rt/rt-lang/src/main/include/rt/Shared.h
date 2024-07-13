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

#ifndef LANG_SHARED_H
#define LANG_SHARED_H

#include <atomic>
#include <functional>

namespace rt {

template<typename T>
class Shared
{
   private:

      struct SharedRef
      {
         T *value {nullptr};
         std::atomic<int> references {1};
         std::function<void()> deleter {nullptr};

         SharedRef(T *value, std::function<void()> deleter) : value(value), deleter(deleter)
         {
         }

         ~SharedRef()
         {
            if (deleter)
            {
               deleter();
            }
         }

      } *shared;

   public:

      explicit Shared(T *value = nullptr, std::function<void()> deleter = nullptr) : shared(new SharedRef(value, deleter))
      {
      }

      Shared(const Shared &other) : shared(other.shared)
      {
         shared->references.fetch_add(1);
      }

      virtual ~Shared()
      {
         if (shared->references.fetch_sub(1) == 1)
         {
            delete shared;
         }
      }

      inline Shared &operator=(const Shared &other)
      {
         if (&other == this)
            return *this;

         // free current data
         if (shared->references.fetch_sub(1) == 1)
         {
            delete shared;
         }

         // copy shared data
         shared = other.shared;

         // increase reference count
         shared->references.fetch_add(1);

         // return this object
         return *this;
      }

      inline int references() const
      {
         return shared->references;
      }

      operator bool()
      {
         return shared->value;
      }

      T *operator->()
      {
         return shared->value;
      }

      const T *operator->() const
      {
         return shared->value;
      }

      T &operator*()
      {
         return shared->value;
      }

      const T &operator*() const
      {
         return shared->value;
      }
};

}

#endif //NFC_LAB_SHARED_H

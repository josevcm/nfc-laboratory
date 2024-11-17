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

#ifndef RT_SHARED_H
#define RT_SHARED_H

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

#endif

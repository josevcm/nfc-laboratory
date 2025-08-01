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

#ifndef RT_HEAP_H
#define RT_HEAP_H

#include <list>
#include <mutex>

#include <rt/Alloc.h>

namespace rt {

template <class T>
class Heap
{
   public:

      using Ptr = std::shared_ptr<Alloc<T>>;

      Ptr alloc(unsigned int size, unsigned int alignment)
      {
         std::lock_guard lock(mutex);

         // check if there is a suitable allocation block in the pool
         for (auto it = available.begin(); it != available.end(); ++it)
         {
            if (it->alloc->size >= size && it->alloc->alignment == alignment)
            {
               Alloc<T> *found = it->alloc.release();
               available.erase(it);
               return wrap(found);
            }
         }

         // create a new allocation block if no suitable one found and add it to the pool
         return wrap(new Alloc<T>(size, alignment));
      }

      void cleanup()
      {
         std::lock_guard lock(mutex);

         available.clear();
      }

      std::string statistics()
      {
         std::lock_guard lock(mutex);

         std::string stats = "Heap statistics: available allocations: " + std::to_string(available.size());

         return stats;
      }

   private:

      using Weak = std::unique_ptr<Alloc<T>>;
      using Clock = std::chrono::steady_clock;
      using Time = Clock::time_point;

      struct Entry
      {
         Weak alloc;
         Time created;
      };

      Ptr wrap(Alloc<T> *raw)
      {
         return Ptr(raw, [this](Alloc<T> *ptr) {
            std::lock_guard lock(mutex);
            available.push_back({Weak(ptr), Clock::now()});
         });
      }

      std::mutex mutex;
      std::list<Entry> available; // object pool
};

}

#endif

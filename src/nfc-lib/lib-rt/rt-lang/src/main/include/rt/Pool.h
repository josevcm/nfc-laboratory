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

#ifndef RT_POOL_H
#define RT_POOL_H

#include <mutex>

namespace rt {
template <class T>
class Pool
{
   public:

      Alloc<T> acquire(unsigned int size, unsigned int alignment)
      {
         std::lock_guard lock(mutex);

         for (auto it = available.begin(); it != available.end(); ++it)
         {
            if (it->size >= size && it->alignment == alignment)
            {
               Alloc<T> found = *it;
               available.erase(it);
               return found;
            }
         }

         // create a new allocation block if no suitable one found and add it to the pool
         return {size, alignment};
      }

      void release(const Alloc<T> &alloc)
      {
         std::lock_guard lock(mutex);

         if (alloc.data)
            available.push_back(alloc);
      }

   private:

      std::mutex mutex;
      std::vector<Alloc<T>> available; // object pool
};

}

#endif

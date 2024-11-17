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

#ifndef RT_FINALLY_H
#define RT_FINALLY_H

#include <atomic>
#include <utility>

namespace rt {

class Finally
{
   private:

      struct Impl
      {
         std::atomic<int> references{};
         std::function<void()> finally{};

         explicit Impl(std::function<void()> finally) : finally(std::move(finally))
         {
         }

         ~Impl()
         {
            if (finally)
            {
               finally();
            }
         }

      } *impl;

   public:

      Finally(std::function<void()> cleanup = nullptr) : impl(new Impl(std::move(cleanup)))
      {
         impl->references.fetch_add(1);
      }

      Finally(const Finally &other) : impl(other.impl)
      {
         impl->references.fetch_add(1);
      }

      virtual ~Finally()
      {
         if (impl->references.fetch_sub(1) == 1)
         {
            delete impl;
         }
      }

      Finally &operator=(const Finally &other)
      {
         if (&other == this)
            return *this;

         if (impl->references.fetch_sub(1) == 1)
         {
            delete impl;
         }

         impl = other.impl;

         impl->references.fetch_add(1);

         return *this;
      }

      int references()
      {
         return impl->references;
      }
};

}

#endif

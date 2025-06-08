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

#ifndef RT_ALLOC_POOL_H
#define RT_ALLOC_POOL_H

#include <atomic>
#include <cstring>
#include <memory>
#include <stdexcept>

namespace rt {

template <class T>
struct Alloc
{
   T *data; // aligned payload data pointer
   unsigned int size; // block size
   unsigned int alignment; // block alignment
   std::shared_ptr<std::atomic_int> references;

   Alloc() : data(nullptr), size(0), alignment(0), references(std::make_shared<std::atomic_int>(1))
   {
   }

   Alloc(unsigned int size, unsigned int alignment, bool clean = false) : data(nullptr), size(size), alignment(alignment), references(std::make_shared<std::atomic_int>(1))
   {
      if (size == 0 || alignment == 0)
         throw std::invalid_argument("Size and alignment must be greater than zero");

      // ensure alignment is a power of two
      if ((alignment & (alignment - 1)) != 0)
         throw std::invalid_argument("Alignment must be a power of two");

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
      if (!((data = (T *)_aligned_malloc(size * sizeof(T), alignment))))
         throw std::bad_alloc();
#else
      if (posix_memalign(&block, alignment, size * sizeof(T)) != 0)
         throw std::bad_alloc();
#endif

      // clean memory
      if (clean)
         std::memset((void *)data, 0, size * sizeof(T));
   }

   Alloc(const Alloc &other) : data(other.data), size(other.size), alignment(other.alignment), references(other.references)
   {
      references->fetch_add(1, std::memory_order_acq_rel);
   }

   ~Alloc()
   {
      // Decrease reference count and free memory if no references left
      if (references->fetch_sub(1, std::memory_order_acq_rel) > 1)
         return; // still has references, do not free

      if (!data)
         return; // nothing to free

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
      _aligned_free(data);
#else
      std::free(data);
#endif
   }
};

}

#endif

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

   Alloc() : data(nullptr), size(0), alignment(0)
   {
   }

   Alloc(unsigned int size, unsigned int alignment, bool clean = false) : data(nullptr), size(size), alignment(alignment)
   {
      if (alignment == 0)
         throw std::invalid_argument("Alignment must be greater than zero");

      // ensure alignment is a power of two
      if ((alignment & (alignment - 1)) != 0)
         throw std::invalid_argument("Alignment must be a power of two");

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
      if (!((data = static_cast<T *>(_aligned_malloc(size * sizeof(T), alignment)))))
         throw std::bad_alloc();
#else
      if (posix_memalign((void**)(&data), alignment, size * sizeof(T)) != 0)
         throw std::bad_alloc();
#endif

      // clean memory
      if (clean)
         std::memset((void *)data, 0, size * sizeof(T));
   }

   ~Alloc()
   {
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
      _aligned_free(data);
#else
      std::free(data);
#endif
   }
};

}

#endif

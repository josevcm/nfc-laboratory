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

#ifndef RT_VARIANT_H
#define RT_VARIANT_H

#include <variant>
#include <thread>
#include <string>
#include <map>

#include <rt/Catalog.h>
#include <rt/ByteBuffer.h>

namespace rt {

typedef std::variant<
      bool,
      char,
      short,
      int,
      long,
      long long,
      unsigned char,
      unsigned short,
      unsigned int,
      unsigned long,
      unsigned long long,
      float,
      double,
      char *,
      void *,
      std::string,
      std::thread::id,
      std::vector<int>,
      std::chrono::duration<long long, std::ratio<1, 1000000000>>,
      ByteBuffer,
      Catalog
> Variant;

}

#endif

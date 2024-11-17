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

#ifndef RT_EVENT_H
#define RT_EVENT_H

#include <rt/Map.h>
#include <rt/Promise.h>

namespace rt {

struct Event : Promise, Map
{
   int code;

   Event(int code, const std::initializer_list<Entry> values = {}) : code(code), Map(values)
   {
   }

   Event(int code, const ResolveHandler &resolve, const RejectHandler &reject = nullptr, const std::initializer_list<Entry> values = {}) : Promise(resolve, reject), Map(values), code(code)
   {
   }
};

}

#endif

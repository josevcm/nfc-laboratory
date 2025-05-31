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

#ifndef RT_THROUGHPUT_H
#define RT_THROUGHPUT_H

#include <chrono>

namespace rt {

class Throughput
{
   private:

      // average time
      double a = 0;

      // average throughput
      double r = 0;

      // elapsed time
      std::chrono::microseconds e {};

      // start time
      std::chrono::steady_clock::time_point t;

   public:

      inline void begin()
      {
         a = 0;
         r = 0;
         t = std::chrono::steady_clock::now();
      }

      inline void end()
      {
         a = 0;
         r = 0;
      }

      inline void update(double elements = 1)
      {
         // calculate elapsed time since last update
         auto s = std::chrono::steady_clock::now() - t;

         // get elapsed time in microseconds
         e = std::chrono::duration_cast<std::chrono::microseconds>(s);

         // process exponential average time
         a = a * (1 - 0.01) + static_cast<double>(e.count()) * 0.01;

         // process exponential average throughput
         r = r * (1 - 0.01) + (elements / static_cast<double>(e.count()) * 1E6) * 0.01;

         // update last time
         t = std::chrono::steady_clock::now();
      }

      inline double elapsed() const
      {
         return a;
      }

      inline double average() const
      {
         return r;
      }
};

}

#endif

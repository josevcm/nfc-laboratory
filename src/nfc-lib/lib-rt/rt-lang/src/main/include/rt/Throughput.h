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

#include <mutex>
#include <chrono>
#include <cstring>

#define QUEUE_SIZE (1<<8)

namespace rt {

class Throughput
{
   private:

      // current value
      double value = 0;

      // window for moving average
      double values[QUEUE_SIZE] {};

      // moving average window for time
      std::chrono::steady_clock::time_point chrono[QUEUE_SIZE] {};

      // current index in moving average window
      unsigned int index = 0;

      // mutex
      mutable std::mutex mutex;

   public:

      inline void begin()
      {
         index = 0;
         value = 0;
         ::memset(values, 0, sizeof(double) * QUEUE_SIZE);
      }

      inline void end()
      {
         index = 0;
         value = 0;
         ::memset(values, 0, sizeof(double) * QUEUE_SIZE);
      }

      inline void update(double n = 1)
      {
         std::lock_guard lock(mutex);

         value += n;
         value -= values[index & (QUEUE_SIZE - 1)];

         values[index & (QUEUE_SIZE - 1)] = n;
         chrono[index & (QUEUE_SIZE - 1)] = std::chrono::steady_clock::now();

         ++index;
      }

      inline double average() const
      {
         std::lock_guard lock(mutex);

         if (index < QUEUE_SIZE)
            return 0;

         auto t1 = chrono[(index & (QUEUE_SIZE - 1))];
         auto t2 = chrono[(index - 1) & (QUEUE_SIZE - 1)];

         double t = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

         return value / (t / 1E6);
      }
};

}

#endif

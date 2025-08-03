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

#ifndef RT_DOWNSAMPLER_H
#define RT_DOWNSAMPLER_H

#include <map>
#include <limits>
#include <cmath>
#include <memory>
#include <vector>

namespace rt {

class Downsampler
{
   struct Impl;

   public:

      struct Bucket
      {
         unsigned long long t_min, t_max;
         float y_min, y_max, y_avg;
      };

      explicit Downsampler(std::vector<double> resolutions); // default: 5ms

      void append(unsigned long long time, float value);

      float query(unsigned long long time, double resolution) const;

      std::vector<Bucket> query(unsigned long long timeStart, unsigned long long timeEnd, double resolution) const;

      void logInfo() const;

   private:

      std::shared_ptr<Impl> impl;
};

}

#endif //RT_DOWNSAMPLER_H

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

#ifndef DATA_STREAMTREE_H
#define DATA_STREAMTREE_H

#include <map>
#include <limits>
#include <cmath>
#include <memory>
#include <vector>

#endif //DATA_STREAMTREE_H

namespace lab {
class StreamTree
{
   struct Impl;

   public:

      struct Bucket
      {
         int64_t t_min, t_max;
         float y_min, y_max;
         float y_avg;
      };

      explicit StreamTree(std::vector<double> resolutions); // default: 5ms

      void append(double t, double y);

      std::vector<Bucket> query(double t_start, double t_end, double pixelWidth) const;

      void logInfo() const;

   private:

      std::shared_ptr<Impl> impl;
};

}

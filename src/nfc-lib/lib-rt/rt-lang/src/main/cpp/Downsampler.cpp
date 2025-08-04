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

#include <rt/Logger.h>

#include <rt/Downsampler.h>

namespace rt {

struct Downsampler::Impl
{
   Logger *log = Logger::getLogger("rt.Downsampler");

   std::vector<double> resolutions;

   std::vector<std::map<double, Bucket>> levels;

   Impl(std::vector<double> resolutions) : resolutions(resolutions)
   {
      for (int i = 0; i < resolutions.size(); ++i)
      {
         levels.emplace_back();
      }
   }

   void append(double time, double value)
   {
      for (int level = 0; level < resolutions.size(); ++level)
      {
         aggregate(level, time, value);
      }
   }

   void aggregate(int level, double time, double value)
   {
      if (level < 0 || level >= resolutions.size())
         return;

      std::map<double, Bucket> &buckets = levels[level];

      const auto y = static_cast<float>(value);

      if (auto it = buckets.lower_bound(time); it != buckets.begin())
      {
         // move iterator to the previous bucket
         --it;

         // if the deviation is small enough, update the existing bucket
         if (float dev = std::abs(y - it->second.y_avg); dev < 0.005f)
         {
            // update time range
            it->second.t_max = time;

            // update average (using exponential average)
            it->second.y_avg = it->second.y_avg * 0.75f + y * 0.25f;

            // update min/max y values for the bucket
            if (it->second.y_min > y)
               it->second.y_min = y;
            else if (it->second.y_max < y)
               it->second.y_max = y;

            return;
         }
      }

      // generate new bucket
      buckets[time] = {time, time, y, y, y};
   }

   std::vector<Bucket> query(const double start, const double end, const double resolution) const
   {
      if (levels.empty())
         return {};

      // search for the appropriate resolution level
      int levelIdx = 0;

      for (int i = 0; i < resolutions.size(); ++i)
      {
         if (resolutions[i] >= resolution)
         {
            levelIdx = i;
            break;
         }

         levelIdx = i;
      }

      const std::map<double, Bucket> &buckets = levels[levelIdx];

      auto it = buckets.lower_bound(start);
      const auto itEnd = buckets.upper_bound(end);

      // move to previous bucket if it exists
      if (it != buckets.begin())
         --it;

      std::vector<Bucket> result;

      for (; it != itEnd; ++it)
      {
         result.push_back(it->second);
      }

      return result;
   }

   void logInfo() const
   {
      size_t bytes = 0;
      size_t buckets = 0;

      for (const auto &level: levels)
      {
         buckets += level.size();
         bytes += (sizeof(unsigned long long) + sizeof(Bucket)) * level.size();
      }

      log->info("StreamTree: {} levels, {} buckets, {} bytes", {levels.size(), buckets, bytes});
   }
};

Downsampler::Downsampler(std::vector<double> resolutions) : impl(std::make_shared<Impl>(resolutions))
{
}

void Downsampler::append(double time, double value)
{
   impl->append(time, value);
}

float Downsampler::query(double time, double resolution) const
{
   return 0;
}

std::vector<Downsampler::Bucket> Downsampler::query(double start, double end, double resolution) const
{
   return impl->query(start, end, resolution);
}

void Downsampler::logInfo() const
{
   impl->logInfo();
}

}

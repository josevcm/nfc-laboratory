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

   std::vector<std::map<unsigned long long, Bucket>> levels;

   Impl(std::vector<double> resolutions) : resolutions(resolutions)
   {
      for (int i = 0; i < resolutions.size(); ++i)
      {
         levels.emplace_back();
      }
   }

   void append(unsigned long long time, float value)
   {
      for (int level = 0; level < resolutions.size(); ++level)
      {
         aggregate(level, time, value);
      }
   }

   void aggregate(int level, unsigned long long time, float value)
   {
      if (level < 0 || level >= resolutions.size())
         return;

      std::map<unsigned long long, Bucket> &buckets = levels[level];

      if (auto it = buckets.lower_bound(time); it != buckets.begin())
      {
         // move iterator to the previous bucket
         --it;

         // detect deviation from average
         float dev = std::abs(value - it->second.y_avg);

         // if the deviation is small enough, update the existing bucket
         if (dev <= 0.05f)
         {
            // update time range
            if (it->second.t_max < time)
               it->second.t_max = time;

            // update min/max y values for the bucket
            if (it->second.y_min > value)
               it->second.y_min = value;
            else if (it->second.y_max < value)
               it->second.y_max = value;

            // update average (using exponential average)
            it->second.y_avg = it->second.y_avg * 0.9f + value * 0.1f;

            return;
         }
      }

      // generate new bucket
      buckets[time] = {time, time, value, value, value};
   }

   std::vector<Bucket> query(const unsigned long long timeStart, const unsigned long long timeEnd, const double resolution) const
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

      const std::map<unsigned long long, Bucket> &buckets = levels[levelIdx];

      auto it = buckets.lower_bound(timeStart);
      const auto itEnd = buckets.upper_bound(timeEnd);

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

void Downsampler::append(unsigned long long time, float value)
{
   impl->append(time, value);
}

float Downsampler::query(unsigned long long time, double resolution) const
{
   return 0;
}

std::vector<Downsampler::Bucket> Downsampler::query(unsigned long long timeStart, unsigned long long timeEnd, double resolution) const
{
   return impl->query(timeStart, timeEnd, resolution);
}

void Downsampler::logInfo() const
{
   impl->logInfo();
}

}

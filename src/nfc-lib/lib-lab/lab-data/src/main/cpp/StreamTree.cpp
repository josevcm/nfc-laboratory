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

#include <lab/data/StreamTree.h>

namespace lab {

struct StreamTree::Impl
{
   rt::Logger *log = rt::Logger::getLogger("data.StreamTree");

   std::vector<double> resolutions;

   std::vector<std::map<int64_t, Bucket>> levels;

   Impl(std::vector<double> resolutions) : resolutions(resolutions)
   {
      for (int i = 0; i < resolutions.size(); ++i)
      {
         levels.emplace_back();
      }
   }

   void insert(double t, double y)
   {
      for (int level = 0; level < resolutions.size(); ++level)
      {
         aggregate(level, t, y);
      }
   }

   void aggregate(int level, double t, double y)
   {
      int64_t index = timeToBucket(level, t);

      if (index < 0)
         return;

      std::map<int64_t, Bucket> &buckets = levels[level];

      if (auto it = buckets.find(index); it != buckets.end())
      {
         if (it->second.y_min > y)
            it->second.y_min = y;
         else if (it->second.y_max < y)
            it->second.y_max = y;
      }
      else
      {
         double resolution = resolutions[level];

         buckets[index] = {static_cast<double>(index) * resolution, static_cast<double>(index + 1) * resolution, y, y};
      }
   }

   std::vector<Bucket> query(double t_start, double t_end, double pixelWidth) const
   {
      if (levels.empty())
         return {};

      double visibleRes = (t_end - t_start) / pixelWidth;

      // search for the appropriate resolution level
      int levelIdx = 0;

      for (int i = 0; i < resolutions.size(); ++i)
      {
         if (resolutions[i] >= visibleRes)
         {
            levelIdx = i;
            break;
         }

         levelIdx = i;
      }

      const std::map<int64_t, Bucket> &mapLevel = levels[levelIdx];
      const int64_t startBucket = timeToBucket(levelIdx, t_start);
      const int64_t endBucket = timeToBucket(levelIdx, t_end);

      std::vector<Bucket> result;

      for (int64_t b = startBucket; b <= endBucket; ++b)
      {
         if (auto it = mapLevel.find(b); it != mapLevel.end())
         {
            result.push_back(it->second);
         }
      }

      return result;
   }

   int64_t timeToBucket(int level, double t) const
   {
      return static_cast<int64_t>(std::floor(t / resolutions[level]));
   }

   void logInfo()
   {
      size_t bytes = 0;
      size_t buckets = 0;

      for (const auto &level: levels)
      {
         buckets += level.size();
         bytes += (sizeof(uint64_t) + sizeof(Bucket)) * level.size();
      }

      log->info("StreamTree: {} levels, {} buckets, {} bytes", {levels.size(), buckets, bytes});
   }
};

StreamTree::StreamTree(std::vector<double> resolutions) : impl(std::make_shared<Impl>(resolutions))
{
}

void StreamTree::insert(double t, double y)
{
   impl->insert(t, y);
}

std::vector<StreamTree::Bucket> StreamTree::query(double t_start, double t_end, double pixelWidth) const
{
   return impl->query(t_start, t_end, pixelWidth);
}

void StreamTree::logInfo() const
{
   impl->logInfo();
}

}

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

#ifndef RT_LIST_H
#define RT_LIST_H

#include <list>
#include <mutex>
#include <utility>
#include <optional>

namespace rt {

template<typename T>
class List
{
   public:

      List() = default;

      List(const List<T> &) = delete;

      List &operator=(const List<T> &) = delete;

      List(List<T> &other)
      {
         std::lock_guard lock(mutex);
         list = std::move(other.list);
      }

      virtual ~List() = default;

      inline long size() const
      {
         std::lock_guard lock(mutex);
         return list.size();
      }

      inline bool empty() const
      {
         return size() == 0;
      }

      inline std::optional<T> front()
      {
         std::lock_guard lock(mutex);

         if (!list.empty())
            return list.front();

         return {};
      }

      inline void append(const T &item)
      {
         std::lock_guard lock(mutex);
         list.push_back(item);
      }

      template<class... S>
      inline void emplace(S &&... args)
      {
         std::lock_guard lock(mutex);
         list.emplace_back(args...);
      }

   private:

      std::list<T> list;
      mutable std::mutex mutex;
};

}

#endif

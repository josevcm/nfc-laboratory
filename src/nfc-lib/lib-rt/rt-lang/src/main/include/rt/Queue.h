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

#ifndef RT_QUEUE_H
#define RT_QUEUE_H

#include <queue>
#include <mutex>
#include <optional>

namespace rt {

template <typename T>
class Queue
{
   public:

      Queue() = default;

      Queue(const Queue &) = delete;

      Queue &operator=(const Queue &) = delete;

      Queue(Queue &other)
      {
         std::lock_guard lock(mutex);
         queue = std::move(other.queue);
      }

      virtual ~Queue() = default;

      long size() const
      {
         std::lock_guard lock(mutex);
         return queue.size();
      }

      bool empty() const
      {
         return size() == 0;
      }

      std::optional<T> pop()
      {
         std::lock_guard lock(mutex);

         if (!queue.empty())
         {
            T value = queue.front();
            queue.pop();
            return value;
         }

         return {};
      }

      void push(const T &item)
      {
         std::lock_guard lock(mutex);
         queue.push(item);
      }

      T &back()
      {
         std::lock_guard lock(mutex);
         return queue.back();
      }

   private:

      std::queue<T> queue;
      mutable std::mutex mutex;
};

}

#endif

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

#ifndef RT_BLOCKINGQUEUE_H
#define RT_BLOCKINGQUEUE_H

#include <list>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace rt {

template <typename T>
class BlockingQueue
{
   public:

      struct Iterator
      {
         std::mutex &mutex;

         typename std::list<T>::iterator it;

         Iterator(typename std::list<T>::iterator it, std::mutex &mutex) : it(it), mutex(mutex)
         {
         }

         T *operator->()
         {
            return it.operator->();
         }

         T &operator*()
         {
            return it.operator*();
         }

         bool operator!=(const Iterator &other)
         {
            return it != other.it;
         }

         Iterator &operator++()
         {
            ++it;

            return *this;
         }
      };

   public:

      BlockingQueue() = default;

      void add(T e)
      {
         std::lock_guard lock(mutex);

         // add element to list
         queue.push_back(e);

         // notify for unlock wait
         sync.notify_all();
      }

      template <typename... A>
      void add(A &&... args)
      {
         std::lock_guard lock(mutex);

         // add element to list
         queue.emplace_back(std::forward<A>(args)...);

         // notify for unlock wait
         sync.notify_all();
      }

      std::optional<T> get(int milliseconds = 0)
      {
         std::unique_lock lock(mutex);

         const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);

         while (queue.empty())
         {
            if (milliseconds >= 0)
            {
               if (milliseconds == 0 || sync.wait_until(lock, timeout) == std::cv_status::timeout)
               {
                  return {};
               }
            }
            else
            {
               sync.wait(lock);
            }
         }

         auto value = queue.front();

         queue.erase(queue.begin());

         return value;
      }

      void remove(const T &e)
      {
         std::lock_guard lock(mutex);

         queue.remove(e);
      }

      void clear()
      {
         std::lock_guard lock(mutex);

         return queue.clear();
      }

      int size() const
      {
         std::lock_guard lock(mutex);

         return queue.size();
      }

      Iterator begin()
      {
         return {queue.begin(), mutex};
      }

      Iterator end()
      {
         return {queue.end(), mutex};
      }

   private:

      // queue elements
      std::list<T> queue;

      // queue mutex
      mutable std::mutex mutex;

      // synchronization condition
      mutable std::condition_variable sync;
};

}

#endif

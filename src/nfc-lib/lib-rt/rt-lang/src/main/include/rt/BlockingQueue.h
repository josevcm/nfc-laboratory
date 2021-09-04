/*

  Copyright (c) 2021 Jose Vicente Campos Martinez - <josevcm@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#ifndef LANG_BLOCKINGQUEUE_H
#define LANG_BLOCKINGQUEUE_H

#include <list>
#include <mutex>
#include <condition_variable>

namespace rt {

template<typename T>
class BlockingQueue
{
   public:

      struct Iterator
      {
         std::mutex &mutex;

         typename std::list<T>::iterator it;

         inline Iterator(typename std::list<T>::iterator it, std::mutex &mutex) : it(it), mutex(mutex)
         {
         }

         inline T *operator->()
         {
            return it.operator->();
         }

         inline T &operator*()
         {
            return it.operator*();
         }

         inline bool operator==(const Iterator &other)
         {
            return it.operator==(other.it);
         }

         inline bool operator!=(const Iterator &other)
         {
            return it.operator!=(other.it);
         }

         inline Iterator &operator++()
         {
            it++;

            return *this;
         }
      };

   public:

      BlockingQueue() = default;

      inline void add(T e)
      {
         std::lock_guard<std::mutex> lock(mutex);

         // add element to list
         queue.push_back(e);

         // notify for unlock wait
         sync.notify_all();
      }

      template<typename... A>
      inline void add(A &&... args)
      {
         std::lock_guard<std::mutex> lock(mutex);

         // add element to list
         queue.emplace_back(std::forward<A>(args)...);

         // notify for unlock wait
         sync.notify_all();
      }

      inline std::optional<T> get(int milliseconds = 0)
      {
         std::unique_lock<std::mutex> lock(mutex);

         auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);

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

      inline void remove(const T &e)
      {
         std::lock_guard<std::mutex> lock(mutex);

         queue.remove(e);
      }

      inline void clear()
      {
         std::lock_guard<std::mutex> lock(mutex);

         return queue.clear();
      }

      inline int size() const
      {
         std::lock_guard<std::mutex> lock(mutex);

         return queue.size();
      }

      inline Iterator begin()
      {
         return {queue.begin(), mutex};
      }

      inline Iterator end()
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

#endif //NFC_LAB_BLOCKINGQUEUE_H

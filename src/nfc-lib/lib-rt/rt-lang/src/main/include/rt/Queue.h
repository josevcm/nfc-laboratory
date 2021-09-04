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

#ifndef LANG_QUEUE_H
#define LANG_QUEUE_H

#include <queue>
#include <mutex>
#include <optional>

template<typename T>
class Queue
{
   public:

      Queue() = default;

      Queue(const Queue<T> &) = delete;

      Queue &operator=(const Queue<T> &) = delete;

      Queue(Queue<T> &other)
      {
         std::lock_guard<std::mutex> lock(mutex);
         queue = std::move(other.queue);
      }

      virtual ~Queue() = default;

      inline long size() const
      {
         std::lock_guard<std::mutex> lock(mutex);
         return queue.size();
      }

      inline bool empty() const
      {
         return size() == 0;
      }

      inline std::optional<T> pop()
      {
         std::lock_guard<std::mutex> lock(mutex);

         if (!queue.empty())
         {
            T value = queue.front();
            queue.pop();
            return value;
         }

         return {};
      }

      inline void push(const T &item)
      {
         std::lock_guard<std::mutex> lock(mutex);
         queue.push(item);
      }

   private:

      std::queue<T> queue;
      mutable std::mutex mutex;
};

#endif //NFC_LAB_QUEUE_H

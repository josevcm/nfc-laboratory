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

#ifndef LANG_LIST_H
#define LANG_LIST_H

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
         std::lock_guard<std::mutex> lock(mutex);
         list = std::move(other.list);
      }

      virtual ~List() = default;

      inline long size() const
      {
         std::lock_guard<std::mutex> lock(mutex);
         return list.size();
      }

      inline bool empty() const
      {
         return size() == 0;
      }

      inline std::optional<T> front()
      {
         std::lock_guard<std::mutex> lock(mutex);

         if (!list.empty())
            return list.front();

         return {};
      }

      inline void append(const T &item)
      {
         std::lock_guard<std::mutex> lock(mutex);
         list.push_back(item);
      }

      template<class... S>
      inline void emplace(S &&... args)
      {
         std::lock_guard<std::mutex> lock(mutex);
         list.emplace_back(args...);
      }

   private:

      std::list<T> list;
      mutable std::mutex mutex;
};

}

#endif //NFC_LAB_QUEUE_H

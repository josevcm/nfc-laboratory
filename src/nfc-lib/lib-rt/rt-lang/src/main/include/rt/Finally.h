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

#ifndef LANG_FINALLY_H
#define LANG_FINALLY_H

#include <atomic>
#include <utility>

namespace rt {

class Finally
{
   private:

      struct Impl
      {
         std::atomic<int> references {};
         std::function<void()> finally {};

         explicit Impl(std::function<void()> finally) : finally(std::move(finally))
         {
         }

         ~Impl()
         {
            if (finally)
            {
               finally();
            }
         }

      } *impl;

   public:

      explicit Finally(std::function<void()> cleanup = nullptr) : impl(new Impl(std::move(cleanup)))
      {
         impl->references.fetch_add(1);
      }

      Finally(const Finally &other) : impl(other.impl)
      {
         impl->references.fetch_add(1);
      }

      virtual ~Finally()
      {
         if (impl->references.fetch_sub(1) == 1)
         {
            delete impl;
         }
      }

      inline Finally &operator=(const Finally &other)
      {
         if (&other == this)
            return *this;

         if (impl->references.fetch_sub(1) == 1)
         {
            delete impl;
         }

         impl = other.impl;

         impl->references.fetch_add(1);

         return *this;
      }

      inline int references()
      {
         return impl->references;
      }
};

}

#endif //NFCLAB_REF_H

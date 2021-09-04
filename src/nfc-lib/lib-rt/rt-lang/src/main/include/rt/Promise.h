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

#ifndef LANG_PROMISE_H
#define LANG_PROMISE_H

#include <functional>
#include <utility>

namespace rt {

struct Promise
{
   public:

      enum Status
      {
         Pending,
         Fulfilled,
         Rejected
      };

      typedef std::function<void()> ResolveHandler;
      typedef std::function<void()> Rejecthandler;

      Promise() : status(Pending)
      {
      }

      Promise(ResolveHandler resolve, Rejecthandler reject) : status(Pending), resolveHandler(std::move(resolve)), rejectHandler(std::move(reject))
      {
      }

      inline void resolve() const
      {
         if (resolveHandler)
         {
            resolveHandler();
         }

         status = Fulfilled;
      }

      inline void reject() const
      {
         if (rejectHandler)
         {
            rejectHandler();
         }

         status = Rejected;
      }

   private:

      mutable Status status;
      ResolveHandler resolveHandler;
      Rejecthandler rejectHandler;
};

}
#endif //NFC_LAB_PROMISE_H

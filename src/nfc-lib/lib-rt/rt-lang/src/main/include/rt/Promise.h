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

#ifndef RT_PROMISE_H
#define RT_PROMISE_H

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
      typedef std::function<void(int, const std::string &)> RejectHandler;

      Promise() : status(Pending)
      {
      }

      Promise(ResolveHandler resolve, RejectHandler reject) : status(Pending), resolveHandler(std::move(resolve)), rejectHandler(std::move(reject))
      {
      }

      void resolve() const
      {
         if (resolveHandler)
         {
            resolveHandler();
         }

         status = Fulfilled;
      }

      void reject(int error = 0, const std::string &message = std::string()) const
      {
         if (rejectHandler)
         {
            rejectHandler(error, message);
         }

         status = Rejected;
      }

   private:

      mutable Status status;
      ResolveHandler resolveHandler;
      RejectHandler rejectHandler;
};

}
#endif

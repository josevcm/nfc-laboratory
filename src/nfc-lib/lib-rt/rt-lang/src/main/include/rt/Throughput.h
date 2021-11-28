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

#ifndef NFC_LAB_THROUGHPUT_H
#define NFC_LAB_THROUGHPUT_H

#include <chrono>

namespace rt {

class Throughput
{
   private:

      // average time
      double a = 0;

      // average throughput
      double r = 0;

      // elapsed time
      std::chrono::microseconds e;

      // start time
      std::chrono::steady_clock::time_point t;

   public:

      inline void begin()
      {
         t = std::chrono::steady_clock::now();
      }

      inline void update(double elements = 1)
      {
         // calculate elapsed time since start
         auto s = std::chrono::steady_clock::now() - t;

         // get elapsed time in microseconds
         e = std::chrono::duration_cast<std::chrono::microseconds>(s);

         // process exponential average time
         a = a * (1 - 0.01) + double(e.count()) * 0.01;

         // process exponential average throughput
         r = r * (1 - 0.01) + (elements / double(e.count()) * 1E6) * 0.01;
      }

      inline double time() const
      {
         return a;
      }

      inline double rate() const
      {
         return r;
      }
};

}

#endif //NFC_LAB_THROUGHPUT_H

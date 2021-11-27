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

#include <fft.h>

#include <sdr/SignalBuffer.h>
#include <sdr/FourierTransform.h>

namespace sdr {

struct FourierTransform::Impl
{
   int points;

   mufft_plan_1d *plan;

   explicit Impl(int points) : points(points)
   {
      plan = mufft_create_plan_1d_c2c(points, MUFFT_FORWARD, MUFFT_FLAG_CPU_ANY);
   }

   ~Impl()
   {
      mufft_free_plan_1d(plan);
   }

   void execute(SignalBuffer &dataIn, SignalBuffer &dataOut) const
   {
      // execute FFT
      mufft_execute_plan_1d(plan, dataOut.pull(2 * points), dataIn.data());

      // prepare to read
      dataOut.flip();
   }
};

FourierTransform::FourierTransform(int points) : impl(new Impl(points))
{
}

void FourierTransform::execute(SignalBuffer &in, SignalBuffer &out)
{
   impl->execute(in, out);
}

}
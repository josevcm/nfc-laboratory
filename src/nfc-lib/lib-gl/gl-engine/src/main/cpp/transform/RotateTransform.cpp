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

#include <gl/engine/Model.h>

#include <gl/transform/RotateTransform.h>

namespace gl {

RotateTransform::RotateTransform(float a, float x, float y, float z, float duration) : a(a), x(x), y(y), z(z), elapsed(0), duration(duration)
{
}

bool RotateTransform::transform(Model &model, float time, float delta)
{
   if (duration == 0)
   {
      model.rotate(a * delta, x, y, z);

      return true;
   }

   if (elapsed < duration)
   {
      // ajustamos paso
      if (elapsed + delta > duration)
         delta = duration - elapsed;

      // aumentamos paso
      elapsed += delta;

      // ejecutamos rotacion
      model.rotate(a * delta, x, y, z);

      return true;
   }

   return false;
}

}
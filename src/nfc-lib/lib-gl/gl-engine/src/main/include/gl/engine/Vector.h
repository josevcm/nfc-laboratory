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

#ifndef UI_GL_VECTOR_H
#define UI_GL_VECTOR_H

namespace gl {

struct Vector
{
   float x;
   float y;
   float z;

   static const Vector UX;
   static const Vector UY;
   static const Vector UZ;

   Vector(float x = 0, float y = 0, float z = 0);

   bool isNill() const;

   Vector &set(float x, float y, float z);

   Vector add(const Vector &v) const;

   Vector &addInPlace(const Vector &v);

   Vector sub(const Vector &v) const;

   Vector &subInPlace(const Vector &v);

   float angle(const Vector &v) const;

   float angle(const Vector &v, const Vector &u) const;

   float cosine(const Vector &v) const;

   float dot(const Vector &v) const;

   Vector cross(const Vector &v) const;

   Vector scale(float s) const;

   Vector &scaleInPlace(float s);

   Vector normalize() const;

   Vector &normalizeInPlace();

   float modulus() const;

   static float modulus(float x, float y, float z);
};

}
#endif //ANDROID_RADIO_VECTOR_H

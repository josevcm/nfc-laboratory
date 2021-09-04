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

#include <cmath>

#include <gl/engine/Vector.h>

namespace gl {

const Vector Vector::UX(1, 0, 0);
const Vector Vector::UY(0, 1, 0);
const Vector Vector::UZ(0, 0, 1);

Vector::Vector(float x, float y, float z) : x(x), y(y), z(z)
{
}

bool Vector::isNill() const
{
   return x == 0 && y == 0 && z == 0;
}

Vector &Vector::set(float vx, float vy, float vz)
{
   x = vx;
   y = vy;
   z = vz;

   return *this;
}

Vector Vector::add(const Vector &v) const
{
   return Vector(*this).addInPlace(v);
}


Vector &Vector::addInPlace(const Vector &v)
{
   x += v.x;
   y += v.y;
   z += v.z;

   return *this;
}

Vector Vector::sub(const Vector &v) const
{
   return Vector(*this).subInPlace(v);
}

Vector &Vector::subInPlace(const Vector &v)
{
   x -= v.x;
   y -= v.y;
   z -= v.z;

   return *this;
}

float Vector::angle(const Vector &v) const
{
   return (float) acos(cosine(v));
}

float Vector::angle(const Vector &v, const Vector &u) const
{
   auto a = angle(v);

   return u.dot(cross(v)) < 0 ? -a : a;
}

float Vector::cosine(const Vector &v) const
{
   return normalize().dot(v.normalize());
}

float Vector::dot(const Vector &v) const
{
   return x * v.x + this->y * v.y + this->z * v.z;
}

Vector Vector::cross(const Vector &v) const
{
   return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
}

Vector Vector::scale(float s) const
{
   return {x * s, y * s, z * s};
}

Vector &Vector::scaleInPlace(float s)
{
   x *= s;
   y *= s;
   z *= s;

   return *this;
}

Vector Vector::normalize() const
{
   float modulus = sqrt(x * x + y * y + z * z);

//   assert(modulus != 0);

   return {x / modulus, y / modulus, z / modulus};
}

Vector &Vector::normalizeInPlace()
{
   float modulus = sqrt(x * x + y * y + z * z);

//   assert(modulus != 0);

   this->x /= modulus;
   this->y /= modulus;
   this->z /= modulus;

   return *this;
}

float Vector::modulus() const
{
   return sqrt(x * x + y * y + z * z);
}

float Vector::modulus(float x, float y, float z)
{
   return sqrt(x * x + y * y + z * z);
}

}
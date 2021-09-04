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

#include <memory>
#include <cmath>

#include <gl/engine/Vector.h>
#include <gl/engine/Matrix.h>

namespace gl {

Matrix::Matrix(float *data)
{
   if (data != nullptr)
      memcpy(matrix, data, sizeof(matrix));
   else
      setIdentity();
}

Matrix &Matrix::setIdentity()
{
   for (int i = 0; i < 16; i++)
      matrix[i] = (i % 5 ? 0.0 : 1.0);

   return *this;
}

Matrix &Matrix::setRow(int row, const float *v)
{
#pragma GCC ivdep
   for (int i = 0; i < 4; i++)
      matrix[4 * row + i] = v[i];

   return *this;
}

Matrix &Matrix::setCol(int col, const float *v)
{
#pragma GCC ivdep
   for (int i = 0; i < 4; i++)
      matrix[i * 4 + col] = v[i];

   return *this;
}

Matrix &Matrix::setFrustum(float left, float right, float bottom, float top, float near, float far)
{
//   assert(right > left);
//   assert(top > bottom);
//   assert(far > near);

   float rwidth = 1.0f / (right - left);
   float rheight = 1.0f / (top - bottom);
   float rdepth = 1.0f / (near - far);
   float x = 2.0f * (near * rwidth);
   float y = 2.0f * (near * rheight);
   float A = (right + left) * rwidth;
   float B = (top + bottom) * rheight;
   float C = (far + near) * rdepth;
   float D = 2.0f * (far * near * rdepth);

   matrix[0 * 4 + 0] = x;
   matrix[0 * 4 + 1] = 0.0f;
   matrix[0 * 4 + 2] = 0.0f;
   matrix[0 * 4 + 3] = 0.0f;

   matrix[1 * 4 + 0] = 0.0f;
   matrix[1 * 4 + 1] = y;
   matrix[1 * 4 + 2] = 0.0f;
   matrix[1 * 4 + 3] = 0.0f;

   matrix[2 * 4 + 0] = A;
   matrix[2 * 4 + 1] = B;
   matrix[2 * 4 + 2] = C;
   matrix[2 * 4 + 3] = -1.0f;

   matrix[3 * 4 + 0] = 0.0f;
   matrix[3 * 4 + 1] = 0.0f;
   matrix[3 * 4 + 2] = D;
   matrix[3 * 4 + 3] = 0.0f;

   return *this;
}

Matrix &Matrix::setPerspective(float fovy, float aspect, float zNear, float zFar)
{
   float f = 1.0f / (float) tan(fovy);
   float rangeReciprocal = 1.0f / (zNear - zFar);

   matrix[0 * 4 + 0] = f / aspect;
   matrix[0 * 4 + 1] = 0.0f;
   matrix[0 * 4 + 2] = 0.0f;
   matrix[0 * 4 + 3] = 0.0f;

   matrix[1 * 4 + 0] = 0.0f;
   matrix[1 * 4 + 1] = f;
   matrix[1 * 4 + 2] = 0.0f;
   matrix[1 * 4 + 3] = 0.0f;

   matrix[2 * 4 + 0] = 0.0f;
   matrix[2 * 4 + 1] = 0.0f;
   matrix[2 * 4 + 2] = (zFar + zNear) * rangeReciprocal;
   matrix[2 * 4 + 3] = -1.0f;

   matrix[3 * 4 + 0] = 0.0f;
   matrix[3 * 4 + 1] = 0.0f;
   matrix[3 * 4 + 2] = 2.0f * zFar * zNear * rangeReciprocal;
   matrix[3 * 4 + 3] = 0.0f;

   return *this;
}

Matrix &Matrix::setOrtho(float left, float right, float bottom, float top, float near, float far)
{
//   assert(right != left);
//   assert(top != bottom);
//   assert(far != near);

   float rwidth = 1.0f / (right - left);
   float rheight = 1.0f / (top - bottom);
   float rdepth = 1.0f / (far - near);

   float x = +2.0f * (rwidth);
   float y = +2.0f * (rheight);
   float z = -2.0f * (rdepth);

   float tx = -(right + left) * rwidth;
   float ty = -(top + bottom) * rheight;
   float tz = -(far + near) * rdepth;

   matrix[0 * 4 + 0] = x;
   matrix[0 * 4 + 1] = 0.0f;
   matrix[0 * 4 + 2] = 0.0f;
   matrix[0 * 4 + 3] = 0.0f;

   matrix[1 * 4 + 0] = 0.0f;
   matrix[1 * 4 + 1] = y;
   matrix[1 * 4 + 2] = 0.0f;
   matrix[1 * 4 + 3] = 0.0f;

   matrix[2 * 4 + 0] = 0.0f;
   matrix[2 * 4 + 1] = 0.0f;
   matrix[2 * 4 + 2] = z;
   matrix[2 * 4 + 3] = 0.0f;

   matrix[3 * 4 + 0] = tx;
   matrix[3 * 4 + 1] = ty;
   matrix[3 * 4 + 2] = tz;
   matrix[3 * 4 + 3] = 1.0f;

   return *this;
}

Matrix &Matrix::setLookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ)
{
   float fx = centerX - eyeX;
   float fy = centerY - eyeY;
   float fz = centerZ - eyeZ;

   // Normalize f
   float rlf = 1.0f / sqrt(fx * fx + fy * fy + fz * fz);
   fx *= rlf;
   fy *= rlf;
   fz *= rlf;

   // compute s = f x up (x means "cross product")
   float sx = fy * upZ - fz * upY;
   float sy = fz * upX - fx * upZ;
   float sz = fx * upY - fy * upX;

   // and normalize s
   float rls = 1.0f / sqrt(sx * sx + sy * sy + sz * sz);
   sx *= rls;
   sy *= rls;
   sz *= rls;

   // compute u = s x f
   float ux = sy * fz - sz * fy;
   float uy = sz * fx - sx * fz;
   float uz = sx * fy - sy * fx;

   matrix[0 * 4 + 0] = sx;
   matrix[0 * 4 + 1] = ux;
   matrix[0 * 4 + 2] = -fx;
   matrix[0 * 4 + 3] = 0.0f;

   matrix[1 * 4 + 0] = sy;
   matrix[1 * 4 + 1] = uy;
   matrix[1 * 4 + 2] = -fy;
   matrix[1 * 4 + 3] = 0.0f;

   matrix[2 * 4 + 0] = sz;
   matrix[2 * 4 + 1] = uz;
   matrix[2 * 4 + 2] = -fz;
   matrix[2 * 4 + 3] = 0.0f;

   matrix[3 * 4 + 0] = 0.0f;
   matrix[3 * 4 + 1] = 0.0f;
   matrix[3 * 4 + 2] = 0.0f;
   matrix[3 * 4 + 3] = 1.0f;

   return translateInPlace(-eyeX, -eyeY, -eyeZ);
}

Matrix Matrix::transpose() const
{
   return Matrix(*this).transposeInPlace();
}

Matrix &Matrix::transposeInPlace()
{
   float temp[16];

   for (int i = 0; i < 4; i++)
   {
      temp[0 * 4 + i] = matrix[i * 4 + 0];
      temp[1 * 4 + i] = matrix[i * 4 + 1];
      temp[2 * 4 + i] = matrix[i * 4 + 2];
      temp[3 * 4 + i] = matrix[i * 4 + 3];
   }

   memcpy(matrix, temp, sizeof(matrix));

   return *this;
}

Matrix Matrix::invert() const
{
   return Matrix();
}

Matrix &Matrix::invertInPlace()
{
   // Invert a 4 x 4 matrix using Cramer's Rule

   // transpose matrix
   float src00 = matrix[0 * 4 + 0];
   float src01 = matrix[0 * 4 + 1];
   float src02 = matrix[0 * 4 + 2];
   float src03 = matrix[0 * 4 + 3];

   float src10 = matrix[1 * 4 + 0];
   float src11 = matrix[1 * 4 + 1];
   float src12 = matrix[1 * 4 + 2];
   float src13 = matrix[1 * 4 + 3];

   float src20 = matrix[2 * 4 + 0];
   float src21 = matrix[2 * 4 + 1];
   float src22 = matrix[2 * 4 + 2];
   float src23 = matrix[2 * 4 + 3];

   float src30 = matrix[3 * 4 + 0];
   float src31 = matrix[3 * 4 + 1];
   float src32 = matrix[3 * 4 + 2];
   float src33 = matrix[3 * 4 + 3];

   // calculate pairs for first 8 elements (cofactors)
   float atmp0 = src22 * src33;
   float atmp1 = src32 * src23;
   float atmp2 = src12 * src33;
   float atmp3 = src32 * src13;
   float atmp4 = src12 * src23;
   float atmp5 = src22 * src13;
   float atmp6 = src02 * src33;
   float atmp7 = src32 * src03;
   float atmp8 = src02 * src23;
   float atmp9 = src22 * src03;
   float atmp10 = src02 * src13;
   float atmp11 = src12 * src03;

   // calculate first 8 elements (cofactors)
   float dst00 = (atmp0 * src11 + atmp3 * src21 + atmp4 * src31) - (atmp1 * src11 + atmp2 * src21 + atmp5 * src31);
   float dst01 = (atmp1 * src01 + atmp6 * src21 + atmp9 * src31) - (atmp0 * src01 + atmp7 * src21 + atmp8 * src31);
   float dst02 = (atmp2 * src01 + atmp7 * src11 + atmp10 * src31) - (atmp3 * src01 + atmp6 * src11 + atmp11 * src31);
   float dst03 = (atmp5 * src01 + atmp8 * src11 + atmp11 * src21) - (atmp4 * src01 + atmp9 * src11 + atmp10 * src21);
   float dst10 = (atmp1 * src10 + atmp2 * src20 + atmp5 * src30) - (atmp0 * src10 + atmp3 * src20 + atmp4 * src30);
   float dst11 = (atmp0 * src00 + atmp7 * src20 + atmp8 * src30) - (atmp1 * src00 + atmp6 * src20 + atmp9 * src30);
   float dst12 = (atmp3 * src00 + atmp6 * src10 + atmp11 * src30) - (atmp2 * src00 + atmp7 * src10 + atmp10 * src30);
   float dst13 = (atmp4 * src00 + atmp9 * src10 + atmp10 * src20) - (atmp5 * src00 + atmp8 * src10 + atmp11 * src20);

   // calculate pairs for second 8 elements (cofactors)
   float btmp0 = src20 * src31;
   float btmp1 = src30 * src21;
   float btmp2 = src10 * src31;
   float btmp3 = src30 * src11;
   float btmp4 = src10 * src21;
   float btmp5 = src20 * src11;
   float btmp6 = src00 * src31;
   float btmp7 = src30 * src01;
   float btmp8 = src00 * src21;
   float btmp9 = src20 * src01;
   float btmp10 = src00 * src11;
   float btmp11 = src10 * src01;

   // calculate second 8 elements (cofactors)
   float dst20 = (btmp0 * src13 + btmp3 * src23 + btmp4 * src33) - (btmp1 * src13 + btmp2 * src23 + btmp5 * src33);
   float dst21 = (btmp1 * src03 + btmp6 * src23 + btmp9 * src33) - (btmp0 * src03 + btmp7 * src23 + btmp8 * src33);
   float dst22 = (btmp2 * src03 + btmp7 * src13 + btmp10 * src33) - (btmp3 * src03 + btmp6 * src13 + btmp11 * src33);
   float dst23 = (btmp5 * src03 + btmp8 * src13 + btmp11 * src23) - (btmp4 * src03 + btmp9 * src13 + btmp10 * src23);
   float dst30 = (btmp2 * src22 + btmp5 * src32 + btmp1 * src12) - (btmp4 * src32 + btmp0 * src12 + btmp3 * src22);
   float dst31 = (btmp8 * src32 + btmp0 * src02 + btmp7 * src22) - (btmp6 * src22 + btmp9 * src32 + btmp1 * src02);
   float dst32 = (btmp6 * src12 + btmp11 * src32 + btmp3 * src02) - (btmp10 * src32 + btmp2 * src02 + btmp7 * src12);
   float dst33 = (btmp10 * src22 + btmp4 * src02 + btmp9 * src12) - (btmp8 * src12 + btmp11 * src22 + btmp5 * src02);

   // calculate determinant
   float det = src00 * dst00 + src10 * dst01 + src20 * dst02 + src30 * dst03;

   // matrix invert, divide by zero
//   assert(det != 0.0f);

   // calculate matrix inverse
   float invdet = 1.0f / det;

   matrix[0 * 4 + 0] = dst00 * invdet;
   matrix[0 * 4 + 1] = dst01 * invdet;
   matrix[0 * 4 + 2] = dst02 * invdet;
   matrix[0 * 4 + 3] = dst03 * invdet;

   matrix[1 * 4 + 0] = dst10 * invdet;
   matrix[1 * 4 + 1] = dst11 * invdet;
   matrix[1 * 4 + 2] = dst12 * invdet;
   matrix[1 * 4 + 3] = dst13 * invdet;

   matrix[2 * 4 + 0] = dst20 * invdet;
   matrix[2 * 4 + 1] = dst21 * invdet;
   matrix[2 * 4 + 2] = dst22 * invdet;
   matrix[2 * 4 + 3] = dst23 * invdet;

   matrix[3 * 4 + 0] = dst30 * invdet;
   matrix[3 * 4 + 1] = dst31 * invdet;
   matrix[3 * 4 + 2] = dst32 * invdet;
   matrix[3 * 4 + 3] = dst33 * invdet;

   return *this;
}

Matrix Matrix::scale(float sx, float sy, float sz) const
{
   return Matrix(*this).scaleInPlace(sx, sy, sz);
}


Matrix &Matrix::scaleInPlace(float sx, float sy, float sz)
{
   for (int i = 0; i < 4; i++)
   {
      matrix[0 * 4 + i] *= sx;
      matrix[1 * 4 + i] *= sy;
      matrix[2 * 4 + i] *= sz;
   }

   return *this;
}

Matrix Matrix::resize(float rx, float ry, float rz) const
{
   return Matrix(*this).resizeInPlace(rx, ry, rz);
}

Matrix &Matrix::resizeInPlace(float x, float y, float z)
{
   matrix[0 * 4 + 0] = x;
   matrix[1 * 4 + 1] = y;
   matrix[2 * 4 + 2] = z;

   return *this;
}

Matrix Matrix::translate(float dx, float dy, float dz) const
{
   return Matrix(*this).translateInPlace(dx, dy, dz);
}

Matrix &Matrix::translateInPlace(float dx, float dy, float dz)
{
   for (int i = 0; i < 4; i++)
   {
      matrix[3 * 4 + i] += matrix[0 * 4 + i] * dx + matrix[1 * 4 + i] * dy + matrix[2 * 4 + i] * dz;
   }

   return *this;
}

Matrix Matrix::rotate(float a, float rx, float ry, float rz) const
{
   return Matrix(*this).rotateInPlace(a, rx, ry, rz);
}

Matrix &Matrix::rotateInPlace(float a, float rx, float ry, float rz)
{
   // Rotation matrix:
   //      xx(1−c)+c  xy(1−c)+zs xz(1−c)-ys 0
   //      xy(1−c)-zs yy(1−c)+c  yz(1−c)+xs 0
   //      xz(1−c)+ys yz(1−c)-xs zz(1−c)+c  0
   //      0            0            0            1

   auto k = a;// * (float) Math.PI / 180.0f;
   auto s = (float) sin(k);
   auto c = (float) cos(k);

   float h[16], t[16];

   h[0 * 4 + 3] = 0;
   h[1 * 4 + 3] = 0;
   h[2 * 4 + 3] = 0;
   h[3 * 4 + 0] = 0;
   h[3 * 4 + 1] = 0;
   h[3 * 4 + 2] = 0;
   h[3 * 4 + 3] = 1;

   if (1.0f == rx && 0.0f == ry && 0.0f == rz)
   {
      h[1 * 4 + 1] = c;
      h[2 * 4 + 2] = c;
      h[1 * 4 + 2] = s;
      h[2 * 4 + 1] = -s;
      h[0 * 4 + 1] = 0;
      h[0 * 4 + 2] = 0;
      h[1 * 4 + 0] = 0;
      h[2 * 4 + 0] = 0;
      h[0 * 4 + 0] = 1;
   }
   else if (0.0f == rx && 1.0f == ry && 0.0f == rz)
   {
      h[0 * 4 + 0] = c;
      h[2 * 4 + 2] = c;
      h[2 * 4 + 0] = s;
      h[0 * 4 + 2] = -s;
      h[0 * 4 + 1] = 0;
      h[1 * 4 + 0] = 0;
      h[1 * 4 + 2] = 0;
      h[2 * 4 + 1] = 0;
      h[1 * 4 + 1] = 1;
   }
   else if (0.0f == rx && 0.0f == ry && 1.0f == rz)
   {
      h[0 * 4 + 0] = c;
      h[1 * 4 + 1] = c;
      h[0 * 4 + 1] = s;
      h[1 * 4 + 0] = -s;
      h[0 * 4 + 2] = 0;
      h[1 * 4 + 2] = 0;
      h[2 * 4 + 0] = 0;
      h[2 * 4 + 1] = 0;
      h[2 * 4 + 2] = 1;
   }
   else
   {
      float len = std::sqrt(rx * rx + ry * ry + rz * rz);

      if (1.0f != len)
      {
         float recipLen = 1.0f / len;
         rx *= recipLen;
         ry *= recipLen;
         rz *= recipLen;
      }

      float nc = 1.0f - c;
      float xx = rx * rx;
      float yy = ry * ry;
      float zz = rz * rz;
      float xy = rx * ry;
      float xz = rx * rz;
      float xs = rx * s;
      float ys = ry * s;
      float yz = ry * rz;
      float zs = rz * s;

      h[0 * 4 + 0] = xx * nc + c;
      h[0 * 4 + 1] = xy * nc + zs;
      h[0 * 4 + 2] = xz * nc - ys;

      h[1 * 4 + 0] = xy * nc - zs;
      h[1 * 4 + 1] = yy * nc + c;
      h[1 * 4 + 2] = yz * nc + xs;

      h[2 * 4 + 0] = xz * nc + ys;
      h[2 * 4 + 1] = yz * nc - xs;
      h[2 * 4 + 2] = zz * nc + c;
   }

   multiply(t, matrix, h);

   memcpy(matrix, t, sizeof(matrix));

   return *this;
}

Matrix Matrix::multiply(const Matrix &other, bool reverse) const
{
   return Matrix(*this).multiplyInPlace(other, reverse);
}

Matrix &Matrix::multiplyInPlace(const Matrix &other, bool reverse)
{
   float temp[16];

   if (reverse)
      multiply(temp, other.matrix, this->matrix);
   else
      multiply(temp, this->matrix, other.matrix);

   memcpy(matrix, temp, sizeof(matrix));

   return *this;
}

Vector Matrix::multiply(Vector v) const
{
   float x = v.x * matrix[0 * 4 + 0] + v.y * matrix[1 * 4 + 0] + v.z * matrix[2 * 4 + 0] + 1.0f * matrix[3 * 4 + 0];
   float y = v.x * matrix[0 * 4 + 1] + v.y * matrix[1 * 4 + 1] + v.z * matrix[2 * 4 + 1] + 1.0f * matrix[3 * 4 + 1];
   float z = v.x * matrix[0 * 4 + 2] + v.y * matrix[1 * 4 + 2] + v.z * matrix[2 * 4 + 2] + 1.0f * matrix[3 * 4 + 2];
   float w = v.x * matrix[0 * 4 + 3] + v.y * matrix[1 * 4 + 3] + v.z * matrix[2 * 4 + 3] + 1.0f * matrix[3 * 4 + 3];

//   assert(w != 0);

   return {x / w, y / w, z / w};
}

void Matrix::multiply(float *r, const float *a, const float *b)
{
   for (int i = 0; i < 4; i++)
   {
      float ai0 = a[4 * 0 + i];
      float ai1 = a[4 * 1 + i];
      float ai2 = a[4 * 2 + i];
      float ai3 = a[4 * 3 + i];

      r[4 * 0 + i] = ai0 * b[0 * 4 + 0] + ai1 * b[0 * 4 + 1] + ai2 * b[0 * 4 + 2] + ai3 * b[0 * 4 + 3];
      r[4 * 1 + i] = ai0 * b[1 * 4 + 0] + ai1 * b[1 * 4 + 1] + ai2 * b[1 * 4 + 2] + ai3 * b[1 * 4 + 3];
      r[4 * 2 + i] = ai0 * b[2 * 4 + 0] + ai1 * b[2 * 4 + 1] + ai2 * b[2 * 4 + 2] + ai3 * b[2 * 4 + 3];
      r[4 * 3 + i] = ai0 * b[3 * 4 + 0] + ai1 * b[3 * 4 + 1] + ai2 * b[3 * 4 + 2] + ai3 * b[3 * 4 + 3];
   }
}

}
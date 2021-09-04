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

#ifndef UI_GL_MATRIX_H
#define UI_GL_MATRIX_H

#include <gl/engine/Vector.h>

namespace gl {

struct Matrix
{
   float matrix[16] {};

   explicit Matrix(float *data = nullptr);

   Matrix &setIdentity();

   Matrix &setRow(int row, const float *v);

   Matrix &setCol(int col, const float *v);

   Matrix &setFrustum(float left, float right, float bottom, float top, float near, float far);

   Matrix &setPerspective(float fovy, float aspect, float zNear, float zFar);

   Matrix &setOrtho(float left, float right, float bottom, float top, float near, float far);

   Matrix &setLookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ);

   Matrix transpose() const;

   Matrix &transposeInPlace();

   Matrix invert() const;

   Matrix &invertInPlace();

   Matrix scale(float x, float y, float z) const;

   Matrix &scaleInPlace(float x, float y, float z);

   Matrix resize(float x, float y, float z) const;

   Matrix &resizeInPlace(float x, float y, float z);

   Matrix translate(float dx, float dy, float dz) const;

   Matrix &translateInPlace(float dx, float dy, float dz);

   Matrix rotate(float a, float x, float y, float z) const;

   Matrix &rotateInPlace(float a, float rx, float ry, float rz);

   Matrix multiply(const Matrix &other, bool reverse = false) const;

   Matrix &multiplyInPlace(const Matrix &other, bool reverse = false);

   Vector multiply(Vector v) const;

   static void multiply(float *r, const float *a, const float *b);
};

}
#endif //ANDROID_RADIO_MATRIX_H

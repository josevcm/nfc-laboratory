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

#include <gl/engine/Viewer.h>

namespace gl {

Viewer::Viewer() : eye(0, 0, 0), look(0, 0, -1), up(0, 1, 0), right(1, 0, 0), viewDirty(true), projDirty(true)
{
}

void Viewer::clearDirty()
{
   viewDirty = projDirty = false;
}

bool Viewer::isDirty()
{
   return viewDirty || projDirty;
}

Viewer &Viewer::setCamera(float fovy, float aspect, float zNear, float zFar)
{
   projMatrix.setPerspective(fovy, aspect, zNear, zFar);
   projDirty = true;

   return *this;
}

Viewer &Viewer::setOrtho(float left, float right, float bottom, float top, float near, float far)
{
   projMatrix.setOrtho(left, right, bottom, top, near, far);
   projDirty = true;

   return *this;
}

Viewer &Viewer::setViewer(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ)
{
   viewMatrix.setLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

   // obtenemos vectores unitarios para cada eje de camara

   // lateral
   right.x = viewMatrix.matrix[0 * 4 + 0];
   right.y = viewMatrix.matrix[1 * 4 + 0];
   right.z = viewMatrix.matrix[2 * 4 + 0];

   // superior
   up.x = viewMatrix.matrix[0 * 4 + 1];
   up.y = viewMatrix.matrix[1 * 4 + 1];
   up.z = viewMatrix.matrix[2 * 4 + 1];

   // frontal
   look.x = -viewMatrix.matrix[0 * 4 + 2];
   look.y = -viewMatrix.matrix[1 * 4 + 2];
   look.z = -viewMatrix.matrix[2 * 4 + 2];

   // posicion
   eye.x = viewMatrix.matrix[3 * 4 + 0];
   eye.y = viewMatrix.matrix[3 * 4 + 1];
   eye.z = viewMatrix.matrix[3 * 4 + 2];

   viewDirty = true;

   return *this;
}

Viewer &Viewer::heading(float a)
{
   viewMatrix.rotateInPlace(a, 1, 0, 0);

   // actualizamos vectores unitarios
   up.x = viewMatrix.matrix[0 * 4 + 1];
   up.y = viewMatrix.matrix[1 * 4 + 1];
   up.z = viewMatrix.matrix[2 * 4 + 1];

   look.x = -viewMatrix.matrix[0 * 4 + 2];
   look.y = -viewMatrix.matrix[1 * 4 + 2];
   look.z = -viewMatrix.matrix[2 * 4 + 2];

   viewDirty = true;

   return *this;
}


Viewer &Viewer::pitch(float a)
{
   viewMatrix.rotateInPlace(a, 0, 1, 0);

   // actualizamos vectores unitarios
   right.x = viewMatrix.matrix[0 * 4 + 0];
   right.y = viewMatrix.matrix[1 * 4 + 0];
   right.z = viewMatrix.matrix[2 * 4 + 0];

   look.x = -viewMatrix.matrix[0 * 4 + 2];
   look.y = -viewMatrix.matrix[1 * 4 + 2];
   look.z = -viewMatrix.matrix[2 * 4 + 2];

   viewDirty = true;

   return *this;
}

Viewer &Viewer::roll(float a)
{
   viewMatrix.rotateInPlace(a, 0, 0, 1);

   // actualizamos vector de mira
   right.x = viewMatrix.matrix[0 * 4 + 0];
   right.y = viewMatrix.matrix[1 * 4 + 0];
   right.z = viewMatrix.matrix[2 * 4 + 0];

   // actualizamos vector superior
   up.x = viewMatrix.matrix[0 * 4 + 1];
   up.y = viewMatrix.matrix[1 * 4 + 1];
   up.z = viewMatrix.matrix[2 * 4 + 1];

   viewDirty = true;

   return *this;
}

Viewer &Viewer::move(float x, float y, float z)
{
   float dx = look.x * z + right.x * x + up.x * y;
   float dy = look.y * z + right.y * x + up.y * y;
   float dz = look.z * z + right.z * x + up.z * y;

   viewMatrix.translateInPlace(dx, dy, dz);
   viewDirty = true;

   eye.x = viewMatrix.matrix[3 * 4 + 0];
   eye.y = viewMatrix.matrix[3 * 4 + 1];
   eye.z = viewMatrix.matrix[3 * 4 + 2];

   return *this;
}

Vector Viewer::eyeRay(float winx, float winy, int width, int height)
{
   Matrix invertedMatrix = projMatrix.multiply(viewMatrix).invert();

   Vector normalized(-1 + (2 * winx / width), +1 - (2 * winy / height), 1);

   Vector transformed = invertedMatrix.multiply(normalized);

   return transformed.sub(eye).normalize();
}

}
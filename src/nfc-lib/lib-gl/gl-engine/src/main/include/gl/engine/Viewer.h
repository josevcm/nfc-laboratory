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

#ifndef UI_GL_VIEWER_H
#define UI_GL_VIEWER_H

#include <gl/engine/Vector.h>
#include <gl/engine/Matrix.h>

namespace gl {

struct Viewer
{
   Vector eye;
   Vector look;
   Vector up;
   Vector right;

   Matrix viewMatrix;
   Matrix projMatrix;

   bool viewDirty;
   bool projDirty;

   Viewer();

   bool isDirty();

   void clearDirty();

   Viewer &setCamera(float fovy, float aspect, float zNear, float zFar);

   Viewer &setOrtho(float left, float right, float bottom, float top, float near, float far);

   Viewer &setViewer(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ);

   Viewer &heading(float a);

   Viewer &pitch(float a);

   Viewer &roll(float a);

   Viewer &move(float x, float y, float z);

   Vector eyeRay(float winx, float winy, int width, int height);
};

}
#endif //ANDROID_RADIO_VIEWER_H

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

#ifndef UI_GL_GEOMETRY_H
#define UI_GL_GEOMETRY_H

#include <cstddef>

#include <gl/engine/Buffer.h>

namespace gl {

struct Rect
{
   float xmin;
   float xmax;
   float ymin;
   float ymax;
};

struct Bounds
{
   float xmin;
   float xmax;
   float ymin;
   float ymax;
   float zmin;
   float zmax;
};

struct Size
{
   int width;
   int height;
};

struct Point
{
   float x;
   float y;
   float z;
} __attribute__((packed));

struct Normal
{
   float x;
   float y;
   float z;
} __attribute__((packed));

struct Color
{
   float r;
   float g;
   float b;
   float a;

   static Color rgb(unsigned int rgb)
   {
      return rgba(rgb << 8 | 0x7f);
   }

   static Color rgba(unsigned int rgba)
   {
      return {
            static_cast<float>(((rgba >> 24) & 0xff) / 255.0),
            static_cast<float>(((rgba >> 16) & 0xff) / 255.0),
            static_cast<float>(((rgba >> 8) & 0xff) / 255.0),
            static_cast<float>((rgba & 0xff) / 255.0)};
   }

} __attribute__((packed));

struct Texel
{
   float u;
   float v;
} __attribute__((packed));

struct Vertex
{
   Point point;
   Color color;
   Texel texel;
   Normal normal;
} __attribute__((packed));

struct Ligth
{
   Point point;
   Point vector;
   Color color;
} __attribute__((packed));

struct Ligthing
{
   Ligth ambientLigth;
   Ligth diffuseLigth;
   Ligth specularLigth;
} __attribute__((packed));

struct Geometry
{
   const int POINT_OFFSET = offsetof(Vertex, point);
   const int COLOR_OFFSET = offsetof(Vertex, color);
   const int TEXEL_OFFSET = offsetof(Vertex, texel);
   const int NORMAL_OFFSET = offsetof(Vertex, normal);

   Buffer vertex;
   Buffer index;
};

}

#endif //ANDROID_RADIO_GEOMETRY_H

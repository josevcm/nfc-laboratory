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

#include <gl/engine/Widget.h>

namespace gl {

struct Widget::Impl
{
   int x {};
   int y {};
   int width {};
   int height {};
   float aspect {};
   float pixel {};
   Rect bounds {};
};

Widget::Widget() : self(std::make_shared<Impl>())
{
}

Widget *Widget::move(int x, int y)
{
   self->x = x;
   self->y = y;

   // trigger children layout
   walk([=](Model *model) {
      if (auto widget = dynamic_cast<Widget *>(model))
         widget->layout();
   });

   return this;
}

Widget *Widget::resize(int width, int height)
{
   self->width = width;
   self->height = height;
   self->aspect = (float) width / (float) height;

   if (width >= height)
   {
      self->pixel = 2.0f / (float) height;
      self->bounds = {-self->aspect, +self->aspect, -1.0f, +1.0f};
   }
   else
   {
      self->pixel = 2.0f / (float) width;
      self->bounds = {-1.0f, +1.0f, -1 / self->aspect, +1 / self->aspect};
   }

   // trigger children layout
   walk([=](Model *model) {
      if (auto widget = dynamic_cast<Widget *>(model))
         widget->layout();
   });

   return this;
}

void Widget::layout()
{
}

Widget *Widget::add(Model *child)
{
   Model::add(child);

   return this;
}

Widget *Widget::remove(Model *child)
{
   Model::remove(child);

   return this;
}

Widget *Widget::parent()
{
   return dynamic_cast<Widget *>(Model::parent());
}

int Widget::x() const
{
   return self->x;
}

int Widget::y() const
{
   return self->y;
}

int Widget::width() const
{
   return self->width;
}

int Widget::height() const
{
   return self->height;
}

float Widget::pixelSize() const
{
   return self->pixel;
}

float Widget::aspectRatio() const
{
   return self->aspect;
}

const Rect &Widget::bounds() const
{
   return self->bounds;
}

};

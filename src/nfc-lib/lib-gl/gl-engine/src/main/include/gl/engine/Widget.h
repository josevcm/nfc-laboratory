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

#ifndef UI_GL_WIDGET_H
#define UI_GL_WIDGET_H

#include <memory>

#include <gl/engine/Model.h>
#include <gl/engine/Geometry.h>

namespace gl {

class Widget : public Model
{
      struct Impl;

   public:

      Widget();

      virtual Widget *move(int x, int y);

      virtual Widget *resize(int width, int height);

      Widget *add(Model *child) override;

      Widget *remove(Model *child) override;

      Widget *parent() override;

      int x() const;

      int y() const;

      int width() const;

      int height() const;

      float pixelSize() const;

      float aspectRatio() const;

      const Rect &bounds() const;

   protected:

      virtual void layout();

   private:

      std::shared_ptr<Impl> self;
};

}


#endif //NFC_LAB_WIDGET_H

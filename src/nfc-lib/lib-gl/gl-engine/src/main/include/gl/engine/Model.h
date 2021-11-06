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

#ifndef UI_GL_MODEL_H
#define UI_GL_MODEL_H

#include <memory>
#include <vector>
#include <functional>

#include <gl/engine/Matrix.h>
#include <gl/engine/Drawable.h>

namespace gl {

class Viewer;
class Lighting;
class Transform;

class Model : public Drawable
{
      friend class Engine;

      struct Impl;

   public:

      Model();

      ~Model() override;

      bool isVisible() const;

      Model *setVisible(bool value);

      virtual Model *parent();

      virtual Model *reset();

      virtual Model *add(Model *child);

      virtual Model *remove(Model *child);

      virtual Model *add(Transform *tr);

      virtual Model *remove(Transform *tr);

      virtual void update(float time, float delta);

      Model *find(const std::function<bool(Model *)> &fn);

      Model *walk(const std::function<void(Model *)> &fn);

      Model *updateView(const Matrix &updateMatrix);

      Model *updateProj(const Matrix &updateMatrix);

      Model *resize(float x, float y, float z);

      Model *rotate(float a, float x, float y, float z);

      Model *scale(float x, float y, float z);

      Model *translate(float x, float y, float z);

      Matrix &worldMatrix() const;

      Matrix &modelMatrix() const;

      Matrix &normalMatrix() const;

      Matrix &projMatrix() const;

      void draw(Device *device, Program *shader) const override;

      void dispose();

   private:

      bool isDirty() const;

      void clearDirty();

      void compute(Viewer *viewer, Model *parent = nullptr);

   private:

      std::shared_ptr<Impl> self;
};

}
#endif //ANDROID_RADIO_MODEL_H

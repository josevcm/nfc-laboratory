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

#include <algorithm>

#include <gl/engine/Matrix.h>
#include <gl/engine/Viewer.h>
#include <gl/engine/Transform.h>
#include <gl/engine/Model.h>

namespace gl {

struct Model::Impl
{
   // visibility flag
   bool visible = true;

   // parent model
   Model *parent = nullptr;

   // model local matrix, controls scaling and rotating
   Matrix modelMatrix;

   // model local matrix for normal vectors scaling and rotating
   Matrix normalMatrix;

   // world matrix, translation and rotation from camera
   Matrix worldMatrix;

   // proyection matrix, world from camera perspective
   Matrix projMatrix;

   // matrix update flags
   bool modelDirty = true;
   bool viewDirty = false;
   bool projDirty = false;

   // childs models
   std::vector<Model *> childs;

   // model transforms
   std::vector<Transform *> transforms;
};

Model::Model() : self(new Impl)
{
}

Model::~Model()
{
   for (auto child : self->childs)
      delete child;

   for (auto transform : self->transforms)
      delete transform;
}

bool Model::isVisible() const
{
   return self->visible;
}

Model *Model::setVisible(bool value)
{
   self->visible = value;

   return this;
}

Model *Model::parent()
{
   return self->parent;
}

Model *Model::reset()
{
   self->modelMatrix.setIdentity();
   self->modelDirty = true;
   return this;
}

Model *Model::add(Model *child)
{
   child->self->parent = this;
   self->childs.push_back(child);
   return this;
}

Model *Model::remove(Model *child)
{
   self->childs.erase(std::remove(self->childs.begin(), self->childs.end(), child), self->childs.end());
   return this;
}

Model *Model::add(Transform *tr)
{
   self->transforms.push_back(tr);
   return this;
}

Model *Model::remove(Transform *tr)
{
   self->transforms.erase(std::remove(self->transforms.begin(), self->transforms.end(), tr), self->transforms.end());
   return this;
}

Model *Model::find(const std::function<bool(Model *)> &fn)
{
   for (auto child : self->childs)
   {
      if (fn(child))
      {
         return child;
      }
   }

   return nullptr;
}

Model *Model::walk(const std::function<void(Model *)> &fn)
{
   for (auto child : self->childs)
   {
      fn(child);

      child->walk(fn);
   }

   return this;
}

void Model::update(float time, float delta)
{
   auto it = self->transforms.begin();

   while (it != self->transforms.end())
   {
      auto tr = *it;

      if (!tr->transform(*this, time, delta))
      {
         it = self->transforms.erase(it);
      }
      else
      {
         it++;
      }
   }

   // process childs
   for (auto child : self->childs)
   {
      child->update(time, delta);
   }
}

Model *Model::updateView(const Matrix &updateMatrix)
{
   if (!self->viewDirty)
      self->worldMatrix = self->modelMatrix;

   self->worldMatrix.multiplyInPlace(updateMatrix, true);
   self->viewDirty = true;

   return this;
}

Model *Model::updateProj(const Matrix &updateMatrix)
{
   self->projMatrix = self->worldMatrix;
   self->projMatrix.multiplyInPlace(updateMatrix, true);
   self->projDirty = true;

   return this;
}

Model *Model::resize(float x, float y, float z)
{
   self->modelMatrix.resizeInPlace(x, y, z);
   self->modelDirty = true;
   return this;
}

Model *Model::rotate(float a, float x, float y, float z)
{
   self->modelMatrix.rotateInPlace(a, x, y, z);
   self->modelDirty = true;
   return this;
}

Model *Model::scale(float x, float y, float z)
{
   self->modelMatrix.scaleInPlace(x, y, z);
   self->modelDirty = true;
   return this;
}

Model *Model::translate(float x, float y, float z)
{
   self->modelMatrix.translateInPlace(x, y, z);
   self->modelDirty = true;
   return this;
}

Matrix &Model::worldMatrix() const
{
   return self->worldMatrix;
}

Matrix &Model::modelMatrix() const
{
   return self->modelMatrix;
}

Matrix &Model::normalMatrix() const
{
   return self->normalMatrix;
}

Matrix &Model::projMatrix() const
{
   return self->projMatrix;
}

void Model::draw(Device *device, Program *shader) const
{
   for (auto child : self->childs)
   {
      child->draw(device, shader);
   }
}

void Model::dispose()
{
   for (auto child : self->childs)
      delete child;

   self->childs.clear();
}

bool Model::isDirty() const
{
   return self->modelDirty || self->viewDirty || self->projDirty;
}

void Model::clearDirty()
{
   self->modelDirty = self->viewDirty = self->projDirty = false;
}

void Model::compute(Viewer *viewer, Model *parent)
{
   if (!parent)
   {
      if (viewer->isDirty() || this->isDirty())
      {
         // root model update view from camera
         this->updateView(viewer->viewMatrix); // modelMatrix * viewMatrix
      }
   }

   else if (parent->isDirty() || this->isDirty())
   {
      // child models update view from parent
      this->updateView(parent->self->worldMatrix);
   }

   // si el modelo o el observador han sido modificados actualizamos matriz de proyeccion
   if (this->isDirty() || viewer->isDirty())
   {
      // aplicamos sobre el modelo la transformacion de camara
//      this->updateDraw(viewer->viewMatrix);

      // finalmente actualizamos transformacion de proyeccion
      this->updateProj(viewer->projMatrix);
   }

   // process childs
   for (auto child : self->childs)
   {
      child->compute(viewer, this);
   }

   // finalmente marcamos modelo como procesado
   this->clearDirty();
}

}
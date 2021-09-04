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

#include <opengl/GL.h>

#include <gl/engine/Buffer.h>

namespace gl {

struct Buffer::Impl
{
   unsigned int id;
   unsigned int target;
   unsigned int size;
   unsigned int elements;
   unsigned int stride;

   Impl(int target, unsigned int size, const void *data, unsigned int elements, unsigned int stride) : id(0), target(target), size(size), elements(elements), stride(stride)
   {
      glGenBuffers(1, &this->id);
      glBindBuffer(target, this->id);

      switch (target)
      {
         case GL_ARRAY_BUFFER:
            glBufferData(target, size, data, GL_STATIC_DRAW);
            break;

         case GL_ELEMENT_ARRAY_BUFFER:
            glBufferData(target, size, data, GL_STATIC_DRAW);
            break;

         case GL_SHADER_STORAGE_BUFFER:
            glBufferData(target, size, data, GL_DYNAMIC_COPY);
            break;

         case GL_UNIFORM_BUFFER:
            glBufferData(target, size, data, GL_DYNAMIC_DRAW);
            break;
      }

      glBindBuffer(target, 0);
   }

   ~Impl()
   {
      glDeleteBuffers(1, &id);
   }

   void bind(int index) const
   {
      glBindBuffer(target, this->id);
      glBindBufferBase(this->target, index, this->id);
      glBindBuffer(target, 0);
   }

   void update(const void *data, unsigned int offset, unsigned int size)
   {
      glBindBuffer(this->target, this->id);
      glBufferSubData(this->target, offset, size ? size : this->size, data);
      glBindBuffer(this->target, 0);
   }
};

Buffer::Buffer() : self(nullptr)
{
}

Buffer::Buffer(struct Impl *shared) : self(shared)
{
}

Buffer::Buffer(const Buffer &other) : self(other.self)
{
}

Buffer::~Buffer()
{
}

Buffer &Buffer::operator=(const Buffer &other)
{
   if (this != &other)
      self = other.self;

   return *this;
}

bool Buffer::valid() const
{
   return self && self->id;
}

unsigned int Buffer::id() const
{
   return self ? self->id : 0;
}

unsigned int Buffer::target() const
{
   return self ? self->target : 0;
}

unsigned int Buffer::size() const
{
   return self ? self->size : 0;
}

unsigned int Buffer::elements() const
{
   return self ? self->elements : 0;
}

unsigned int Buffer::stride() const
{
   return self ? self->stride : 0;
}

Buffer Buffer::bind(int index) const
{
   if (self)
      self->bind(index);

   return *this;
}

Buffer Buffer::update(const void *data, unsigned int offset, unsigned int size) const
{
   if (self)
      self->update(data, offset, size);

   return *this;
}

Buffer Buffer::release()
{
   self.reset();

   return *this;
}

Buffer Buffer::createArrayBuffer(unsigned int size, const void *data, unsigned int elements, unsigned int stride)
{
   return Buffer(new Impl(GL_ARRAY_BUFFER, size, data, elements, stride));
}

Buffer Buffer::createElementBuffer(unsigned int size, const void *data, unsigned int elements, unsigned int stride)
{
   return Buffer(new Impl(GL_ELEMENT_ARRAY_BUFFER, size, data, elements, stride));
}

Buffer Buffer::createStorageBuffer(unsigned int size, const void *data, unsigned int elements, unsigned int stride)
{
   return Buffer(new Impl(GL_SHADER_STORAGE_BUFFER, size, data, elements, stride));
}

Buffer Buffer::createUniformBuffer(unsigned int size, const void *data, unsigned int elements, unsigned int stride)
{
   return Buffer(new Impl(GL_UNIFORM_BUFFER, size, data, elements, stride));
}

}
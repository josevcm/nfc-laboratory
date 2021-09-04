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

#ifndef UI_GL_BUFFER_H
#define UI_GL_BUFFER_H

#include <memory>

namespace gl {

class Buffer
{
      struct Impl;

   public:

      Buffer();

      Buffer(const Buffer &other);

      ~Buffer();

      Buffer &operator=(const Buffer &other);

      bool valid() const;

      unsigned int id() const;

      unsigned int target() const;

      unsigned int size() const;

      unsigned int elements() const;

      unsigned int stride() const;

      Buffer release();

      Buffer bind(int index) const;

      Buffer update(const void *data, unsigned int offset = 0, unsigned int size = 0) const;

      static Buffer createArrayBuffer(unsigned int size, const void *data = nullptr, unsigned int elements = 0, unsigned int stride = 0);

      static Buffer createElementBuffer(unsigned int size, const void *data = nullptr, unsigned int elements = 0, unsigned int stride = 0);

      static Buffer createStorageBuffer(unsigned int size, const void *data = nullptr, unsigned int elements = 0, unsigned int stride = 0);

      static Buffer createUniformBuffer(unsigned int size, const void *data = nullptr, unsigned int elements = 0, unsigned int stride = 0);

   private:

      explicit Buffer(struct Impl *impl);

   private:

      std::shared_ptr<Impl> self;
};

}

#endif //ANDROID_RADIO_BUFFER_H

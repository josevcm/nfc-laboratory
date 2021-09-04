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

#include <rt/Logger.h>

#include <gl/engine/Program.h>

namespace gl {

struct Program::Impl
{
   rt::Logger log {"Program"};

   long programId = -1;

   Impl()
   {
      programId = glCreateProgram();
   }

   ~Impl()
   {
      glDeleteProgram(programId);
   }
};

Program::Program() : self(new Impl)
{
}

bool Program::loadShader(int type, const std::string &source)
{
   if (!source.empty())
   {
      int status;

      // source buffer pointer
      const char *buffer = source.c_str();

      // create shader type
      int shaderId = glCreateShader(type);

      // add the source code to the shader...
      glShaderSource(shaderId, 1, &buffer, nullptr);

      // and compile it
      glCompileShader(shaderId);

      // get shader compile status
      glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);

      // check shader compile status
      if (status != GL_TRUE)
      {
         char message[1024];

         glGetShaderInfoLog(shaderId, sizeof(message), nullptr, message);

         glDeleteShader(shaderId);

         self->log.error("shader compile error: {}", {std::string(message)});

         return false;
      }

      // attach shader to self
      glAttachShader(self->programId, shaderId);

      // and flagged for deletion, but it will not be deleted while it attached to any self
      glDeleteShader(shaderId);

      return true;
   }

   return false;
}

bool Program::linkProgram()
{
   int status;

   // prepare transform feedback varyings
//   if (!feedbackVaryings.empty())
//   {
//      const char *varyings[feedbackVaryings.size()];
//
//      for (int i = 0; i < feedbackVaryings.size(); i++)
//      {
//         varyings[i] = feedbackVaryings[i].c_str();
//      }
//
//      glTransformFeedbackVaryings(shader->programId, feedbackVaryings.size(), varyings, GL_INTERLEAVED_ATTRIBS);
//
//      // check varyings status
//      if ((status = glGetError()) != GL_NO_ERROR)
//      {
//         self->log.error("transform feedback varyings error: %d", status);
//
//         return false;
//      }
//   }

   // create OpenGL self executables
   glLinkProgram(self->programId);

   // get self link status
   glGetProgramiv(self->programId, GL_LINK_STATUS, &status);

   if (status != GL_TRUE)
   {
      static char message[1024];

      glGetProgramInfoLog(self->programId, sizeof(message), nullptr, message);

      self->log.error("self link error: {}", {std::string(message)});

      return false;
   }

   // create OpenGL self executables
   glValidateProgram(self->programId);

   // get self validate status
   glGetProgramiv(self->programId, GL_VALIDATE_STATUS, &status);

   if (status != GL_TRUE)
   {
      static char message[1024];

      glGetProgramInfoLog(self->programId, sizeof(message), nullptr, message);

      self->log.error("self validate error: {}", {std::string(message)});

      return false;
   }

   return true;
}

void Program::useProgram() const
{
   glUseProgram(self->programId);
}

void Program::endProgram() const
{
   glUseProgram(0);
}

void Program::barrier(unsigned int mask) const
{
   glMemoryBarrier(mask);
}

long Program::attribLocation(const std::string &name) const
{
   int id = glGetAttribLocation(self->programId, name.c_str());

   self->log.debug("attrib location [{}]: {}", {name, id});

   return id;
}

long Program::uniformLocation(const std::string &name) const
{
   int id = glGetUniformLocation(self->programId, name.c_str());

   self->log.debug("uniform location [{}]: {}", {name, id});

   return id;
}

long Program::uniformBlock(const std::string &name) const
{
   int id = glGetUniformBlockIndex(self->programId, name.c_str());

   self->log.debug("uniform block [{}]: {}", {name, id});

   return id;
}

long Program::storageBlock(const std::string &name) const
{
   int id = glGetProgramResourceIndex(self->programId, GL_SHADER_STORAGE_BLOCK, name.c_str());

   self->log.debug("storage block [{}]: {}", {name, id});

   return id;
}

long Program::uniformBlockOffset(int id, const std::string &name) const
{
   unsigned int index;
   const char *names = name.c_str();

   glGetUniformIndices(self->programId, 1, &names, &index);

   if (index != GL_INVALID_INDEX)
   {
      int offset;

      glGetActiveUniformsiv(self->programId, 1, &index, GL_UNIFORM_OFFSET, &offset);

      self->log.debug("uniform block offset [{}]: {}", {name, offset});

      return offset;
   }

   return -1;
}

long Program::storageBlockOffset(int id, const std::string &name) const
{
   unsigned int index;
   const char *names = name.c_str();

//   glGetProgramResourceiv(shader->programId, GL_SHADER_STORAGE_BLOCK, id, 1, &names, &index);
//
//   if (index != GL_INVALID_INDEX)
//   {
//      int offset;
//
//      glGetActiveUniformsiv(shader->programId, 1, &index, GL_UNIFORM_OFFSET, &offset);
//
//      self->log.debug("uniform block offset [%s]: %d", name.c_str(), offset);
//
//      return offset;
//   }

   return -1;
}

}
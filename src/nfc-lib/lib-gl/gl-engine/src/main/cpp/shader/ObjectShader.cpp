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

#include <gl/engine/Model.h>
#include <gl/engine/Buffer.h>
#include <gl/engine/Geometry.h>
#include <gl/engine/Assets.h>

#include <gl/shader/ObjectShader.h>

namespace gl {

struct ObjectShader::Impl
{
   rt::Logger log {"ObjectShader"};

   const Assets *assets {};

   int vertexColorId = -1;
   int vertexNormalId = -1;
   int vertexPointId = -1;
   int vertexTexelId = -1;

   int objectColorId = -1;
   int ambientLightColorId = -1;
   int diffuseLightColorId = -1;
   int diffuseLightVectorId = -1;

   explicit Impl(const Assets *assets) : assets(assets)
   {
   }
};

ObjectShader::ObjectShader(const Assets *assets, const std::string &name) : self(new Impl(assets))
{
   if (!name.empty())
   {
      load(name);
   }
}

ObjectShader::~ObjectShader()
{
   delete self;
}

bool ObjectShader::load(const std::string &name)
{
   self->log.info("loading shader program {}", {name});

   // load vertex shader
   if (!loadShader(GL_VERTEX_SHADER, self->assets->readText("shader/" + name + ".v.glsl")))
   {
      self->log.warn("vertex shader {} do not not exists!", {name});
   }

   // load geometry shader
   if (!loadShader(GL_GEOMETRY_SHADER, self->assets->readText("shader/" + name + ".g.glsl")))
   {
      self->log.warn("geometry shader {} do not not exists!", {name});
   }

   // load fragment shader
   if (!loadShader(GL_FRAGMENT_SHADER, self->assets->readText("shader/" + name + ".f.glsl")))
   {
      self->log.warn("fragment shader {} do not not exists!", {name});
   }

   // load shader program
   if (linkProgram())
   {
      // setup attributes location
      self->vertexColorId = attribLocation("aVertexColor");
      self->vertexNormalId = attribLocation("aVertexNormal");
      self->vertexPointId = attribLocation("aVertexPoint");
      self->vertexTexelId = attribLocation("aVertexTexel");

      // setup light uniform locations
      self->ambientLightColorId = uniformLocation("uAmbientLightColor");
      self->diffuseLightColorId = uniformLocation("uDiffuseLightColor");
      self->diffuseLightVectorId = uniformLocation("uDiffuseLightVector");

      // setup object color uniform location
      self->objectColorId = uniformLocation("uObjectColor");

      return true;
   }

   return false;
}


void ObjectShader::useProgram() const
{
   AbstractShader::useProgram();

   enableAttribArray(self->vertexPointId);
   enableAttribArray(self->vertexColorId);
   enableAttribArray(self->vertexNormalId);
   enableAttribArray(self->vertexTexelId);
}

void ObjectShader::endProgram() const
{
   disableAttribArray(self->vertexPointId);
   disableAttribArray(self->vertexColorId);
   disableAttribArray(self->vertexNormalId);
   disableAttribArray(self->vertexTexelId);

   AbstractShader::endProgram();
}

void ObjectShader::setLightingModel(const Lighting &lighting) const
{
   setAmbientLightColor(lighting.ambientLight.color);
   setDiffuseLightColor(lighting.diffuseLight.color);
   setDiffuseLightVector(lighting.diffuseLight.vector);
}

void ObjectShader::setAmbientLightColor(const Color &color) const
{
   setUniformFloat(self->ambientLightColorId, {color.r, color.g, color.b});
}

void ObjectShader::setDiffuseLightColor(const Color &color) const
{
   setUniformFloat(self->diffuseLightColorId, {color.r, color.g, color.b});
}

void ObjectShader::setDiffuseLightVector(const Point &vector) const
{
   setUniformFloat(self->diffuseLightVectorId, {vector.x, vector.y, vector.z});
}

void ObjectShader::setObjectColor(const Color &color) const
{
   setUniformFloat(self->objectColorId, {color.r, color.g, color.b, color.a});
}

void ObjectShader::setVertexColors(const Buffer &buffer, int components, int offset, int stride) const
{
   setVertexFloatArray(self->vertexColorId, buffer, components, offset, stride);
}

void ObjectShader::setVertexNormals(const Buffer &buffer, int components, int offset, int stride) const
{
   setVertexFloatArray(self->vertexNormalId, buffer, components, offset, stride);
}

void ObjectShader::setVertexPoints(const Buffer &buffer, int components, int offset, int stride) const
{
   setVertexFloatArray(self->vertexPointId, buffer, components, offset, stride);
}

void ObjectShader::setVertexTexels(const Buffer &buffer, int components, int offset, int stride) const
{
   setVertexFloatArray(self->vertexTexelId, buffer, components, offset, stride);
}

void ObjectShader::drawGeometry(const Geometry &geometry, int elements) const
{
   setVertexColors(geometry.vertex, 4, geometry.COLOR_OFFSET);
   setVertexPoints(geometry.vertex, 3, geometry.POINT_OFFSET);
   setVertexTexels(geometry.vertex, 2, geometry.TEXEL_OFFSET);
   setVertexNormals(geometry.vertex, 3, geometry.NORMAL_OFFSET);

   drawTriangles(geometry.index, elements);
}

const Assets *ObjectShader::assets()
{
   return self->assets;
}

}

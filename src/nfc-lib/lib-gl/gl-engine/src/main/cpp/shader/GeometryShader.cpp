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

#include <gl/shader/GeometryShader.h>

namespace gl {

struct GeometryShader::Impl
{
   rt::Logger log {"GeometryShader"};

   const Assets *assets {};

   int vertexColorId = -1;
   int vertexNormalId = -1;
   int vertexPointId = -1;
   int vertexTexelId = -1;

   int objectColorId = -1;
   int ambientLigthColorId = -1;
   int diffuseLigthColorId = -1;
   int diffuseLigthVectorId = -1;

   explicit Impl(const Assets *assets) : assets(assets)
   {
   }
};

GeometryShader::GeometryShader(const Assets *assets, const std::string &name) : self(new Impl(assets))
{
   if (!name.empty())
   {
      load(name);
   }
}

GeometryShader::~GeometryShader()
{
   delete self;
}

bool GeometryShader::load(const std::string &name)
{
   self->log.info("loading shader program {}", {name});

   // load vertex shader
   if (!loadShader(GL_VERTEX_SHADER, self->assets->readText("shader/" + name + ".v.glsl")))
   {
      self->log.warn("vertex shader not available!");
   }

   // load geometry shader
   if (!loadShader(GL_GEOMETRY_SHADER, self->assets->readText("shader/" + name + ".g.glsl")))
   {
      self->log.warn("geometry shader not available!");
   }

   // load fragment shader
   if (!loadShader(GL_FRAGMENT_SHADER, self->assets->readText("shader/" + name + ".f.glsl")))
   {
      self->log.warn("fragment shader not available!");
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
      self->ambientLigthColorId = uniformLocation("uAmbientLigthColor");
      self->diffuseLigthColorId = uniformLocation("uDiffuseLigthColor");
      self->diffuseLigthVectorId = uniformLocation("uDiffuseLigthVector");

      // setup object color uniform location
      self->objectColorId = uniformLocation("uObjectColor");

      return true;
   }

   return false;
}


void GeometryShader::useProgram() const
{
   AbstractShader::useProgram();

   enableAttribArray(self->vertexPointId);
   enableAttribArray(self->vertexColorId);
   enableAttribArray(self->vertexNormalId);
   enableAttribArray(self->vertexTexelId);
}

void GeometryShader::endProgram() const
{
   disableAttribArray(self->vertexPointId);
   disableAttribArray(self->vertexColorId);
   disableAttribArray(self->vertexNormalId);
   disableAttribArray(self->vertexTexelId);

   AbstractShader::endProgram();
}

void GeometryShader::setLightingModel(const Ligthing &ligthing) const
{
   setAmbientLigthColor(ligthing.ambientLigth.color);
   setDiffuseLigthColor(ligthing.diffuseLigth.color);
   setDiffuseLigthVector(ligthing.diffuseLigth.vector);
}

void GeometryShader::setAmbientLigthColor(const Color &color) const
{
   setUniformFloat(self->ambientLigthColorId, {color.r, color.g, color.b});
}

void GeometryShader::setDiffuseLigthColor(const Color &color) const
{
   setUniformFloat(self->diffuseLigthColorId, {color.r, color.g, color.b});
}

void GeometryShader::setDiffuseLigthVector(const Point &vector) const
{
   setUniformFloat(self->diffuseLigthVectorId, {vector.x, vector.y, vector.z});
}

void GeometryShader::setObjectColor(const Color &color) const
{
   setUniformFloat(self->objectColorId, {color.r, color.g, color.b, color.a});
}

void GeometryShader::setVertexColors(const Buffer &buffer, int components, int offset, int stride) const
{
   setVertexFloatArray(self->vertexColorId, buffer, components, offset, stride);
}

void GeometryShader::setVertexNormals(const Buffer &buffer, int components, int offset, int stride) const
{
   setVertexFloatArray(self->vertexNormalId, buffer, components, offset, stride);
}

void GeometryShader::setVertexPoints(const Buffer &buffer, int components, int offset, int stride) const
{
   setVertexFloatArray(self->vertexPointId, buffer, components, offset, stride);
}

void GeometryShader::setVertexTexels(const Buffer &buffer, int components, int offset, int stride) const
{
   setVertexFloatArray(self->vertexTexelId, buffer, components, offset, stride);
}

void GeometryShader::drawGeometry(const Geometry &geometry, int elements) const
{
   setVertexColors(geometry.vertex, 4, geometry.COLOR_OFFSET);
   setVertexPoints(geometry.vertex, 3, geometry.POINT_OFFSET);
   setVertexTexels(geometry.vertex, 2, geometry.TEXEL_OFFSET);
   setVertexNormals(geometry.vertex, 3, geometry.NORMAL_OFFSET);

   drawTriangles(geometry.index, elements);
}

const Assets *GeometryShader::assets()
{
   return self->assets;
}

}

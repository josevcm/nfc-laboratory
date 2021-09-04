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

#include <map>
#include <utility>
#include <vector>

#include <opengl/GL.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BITMAP_H
#include FT_LCD_FILTER_H

#include <rt/Logger.h>
#include <rt/Format.h>

#include <gl/engine/Font.h>
#include <gl/engine/Text.h>

#include <gl/shader/TypeFaceShader.h>

#include <gl/typeface/FreeType.h>

// https://learnopengl.com/In-Practice/Text-Rendering
// https://github.com/rougier/freetype-gl/blob/master/texture-font.c
// https://www.freetype.org/freetype2/docs/reference/ft2-lcd_rendering.html

namespace gl {

struct Rgba
{
   unsigned char r;
   unsigned char g;
   unsigned char b;
   unsigned char a;
} __attribute__((packed));

struct Char
{
   int ch;
   int left;
   int top;
   int width;
   int height;
   int advance;
   Rgba *buffer;

   Char(int ch, int left, int top, int width, int height, int advance) : ch(ch), left(left), top(top), width(width), height(height), advance(advance)
   {
      buffer = new Rgba[width * height];
   }

   ~Char()
   {
      delete buffer;
   }
};

struct TextImpl : public Text
{
   rt::Logger log {"Text"};

   Font font;
   std::string text;

   Geometry geometry;

   TextImpl() = default;

   explicit TextImpl(const Font &font, std::string text) : font(font), text(std::move(text))
   {
      geometry.vertex = Buffer::createArrayBuffer(256 * sizeof(Vertex) * 4, nullptr, 256 * 4, sizeof(Vertex));
      geometry.index = Buffer::createElementBuffer(256 * sizeof(unsigned int) * 6, nullptr, 256 * 6);
   }

   Text *setText(const std::string &value) override
   {
      text = value;

      layout();

      return this;
   }

   Widget *move(int x, int y) override
   {
      Widget::move(x, y);
      layout();
      return this;
   }

   Widget *resize(int width, int height) override
   {
      // ignore resize
      return this;
   }

   void layout() override
   {
      if (parent())
      {
         float pixelSize = parent()->pixelSize();
         float fontSize = pixelSize * font.size();

         // normalized origin coordinates
         float ox = (float) (Widget::x() - parent()->width() / 2) * pixelSize;
         float oy = (float) (Widget::y() - parent()->height() / 2) * pixelSize;

         int width = 0;
         int height = font.size();

         if (!text.empty())
         {
            Vertex v[4 * text.length()];
            unsigned int i[6 * text.length()];

            for (int n = 0; n < text.length(); n++)
            {
               char ch = text.at(n);
               auto quad = font.getQuad(ch);

               if (quad)
               {
                  i[n * 6 + 0] = n * 4 + 0;
                  i[n * 6 + 1] = n * 4 + 1;
                  i[n * 6 + 2] = n * 4 + 3;
                  i[n * 6 + 3] = n * 4 + 1;
                  i[n * 6 + 4] = n * 4 + 2;
                  i[n * 6 + 5] = n * 4 + 3;

                  v[n * 4 + 0] = {.point = {ox + fontSize * quad.glyphLeft, oy + fontSize * quad.glyphBottom, 0}, .texel = {quad.texelLeft, quad.texelBottom}};
                  v[n * 4 + 1] = {.point = {ox + fontSize * quad.glyphRight, oy + fontSize * quad.glyphBottom, 0}, .texel = {quad.texelRight, quad.texelBottom}};
                  v[n * 4 + 2] = {.point = {ox + fontSize * quad.glyphRight, oy + fontSize * quad.glyphTop, 0}, .texel = {quad.texelRight, quad.texelTop}};
                  v[n * 4 + 3] = {.point = {ox + fontSize * quad.glyphLeft, oy + fontSize * quad.glyphTop, 0}, .texel = {quad.texelLeft, quad.texelTop}};

                  ox += pixelSize * (float) quad.charAdvance;

                  width += quad.charWidth;
               }
            }

            // update geometry buffers
            geometry.vertex.update(v, 0, sizeof(v));
            geometry.index.update(i, 0, sizeof(i));
         }
         else
         {
            setVisible(false);
         }

         // update widget size
         Widget::resize(width, height);
      }
   }

   void draw(Device *device, Program *shader) const override
   {
      if (isVisible() && font)
      {
         if (auto typeFaceShader = dynamic_cast<TypeFaceShader *>(shader))
         {
            font.bind(0);
            typeFaceShader->setMatrixBlock(*this);
            typeFaceShader->setObjectColor({1.0, 1.0, 1.0, 1.0});
            typeFaceShader->drawGeometry(geometry, text.length() * 6);
         }
      }
   }
};

struct FreeTypeImpl
{
   rt::Logger log {"TypeFace"};

   std::map<std::string, Font> fonts;

   Font getFont(const std::string &name, int size, int dpi)
   {
      auto key = rt::Format::format("{}/{}/{}", {name, size, dpi});

      auto it = fonts.find(key);

      if (it != fonts.end())
         return it->second;

      if (auto font = loadFont(name, size, dpi))
         return fonts[key] = font;

      return {};
   }

   Font loadFont(const std::string &name, int size, int dpi) const
   {
      FT_Library ft;

      if (FT_Init_FreeType(&ft) == 0)
      {
         FT_Face face;

         std::string file = "fonts/" + name + ".ttf";

         if (FT_New_Face(ft, file.data(), 0, &face) == 0)
         {
            log.debug("loading font {} from file", {name, file});

            float diff = 1e20;
            int bestMatch = 0;

            if (face->face_flags & FT_FACE_FLAG_SCALABLE)
            {
               log.debug("font {} selected scalable size {}, at {} dpi", {name, size, dpi});

               FT_Set_Char_Size(face, size << 6, size << 6, dpi, dpi);
            }
            else if (face->face_flags & FT_FACE_FLAG_FIXED_SIZES)
            {
               for (int i = 0; i < face->num_fixed_sizes; i++)
               {
                  float newSize = face->available_sizes[i].size / 64.0f;
                  float ndiff = size > newSize ? size / newSize : newSize / size;

                  if (ndiff < diff)
                  {
                     bestMatch = i;
                     diff = ndiff;
                  }
               }

               log.debug("font {} selected by fixed best match {}, size {}", {name, bestMatch, face->available_sizes[bestMatch].size / 64.0f});

               FT_Select_Size(face, bestMatch);
            }
            else
            {
               log.debug("font {} selected by pixel size {}", {name, size});

               FT_Set_Pixel_Sizes(face, 0, size);
            }

//            FT_Matrix matrix = {
//                  (int) ((1.0 / 64) * 0x10000L),
//                  (int) ((0.0) * 0x10000L),
//                  (int) ((0.0) * 0x10000L),
//                  (int) ((1.0) * 0x10000L)};
//
//            FT_Set_Transform(face, &matrix, NULL);

            // font character metrics
            std::vector<std::shared_ptr<Char>> characters;

            // texture parameters
            int width = 0;
            int height = size;
            int padding = 5;

            FT_Library_SetLcdFilter(ft, FT_LCD_FILTER_LIGHT);

//            unsigned char weights[] = {0x00, 0x55, 0x56, 0x55, 0x00};
//            unsigned char weights[] = {0x10, 0x40, 0x70, 0x40, 0x10};

//            FT_Library_SetLcdFilterWeights(ft, weights);

            for (int code = 0; code < 128; code++)
            {
               // load character glyph
               if (FT_Load_Char(face, code, FT_LOAD_RENDER | FT_LOAD_TARGET_LCD)  == 0)
               {
                  auto glyph = face->glyph;

//                  log.debug("loading char {}: size {} x {}, left {} top {} pitch {} pixel {}", {code, glyph->bitmap.width, glyph->bitmap.rows, glyph->bitmap_left, glyph->bitmap_top, glyph->bitmap.pitch, glyph->bitmap.pixel_mode});

                  Char *character = nullptr;

                  // single pixel bitmap
                  if (glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
                  {
                     FT_Bitmap target;

                     FT_Bitmap_New(&target);

                     // convert to 8bpp
                     if (FT_Bitmap_Convert(ft, &glyph->bitmap, &target, 1) == 0)
                     {
                        character = new Char(code, glyph->bitmap_left, glyph->bitmap_top, target.width, target.rows, glyph->advance.x >> 6);

                        unsigned char *src = target.buffer;

                        for (int r = 0, n = 0; r < target.rows; r++)
                        {
                           #pragma GCC ivdep
                           for (int w = 0; w < target.width; w++, n++)
                           {
                              character->buffer[n] = Rgba {255, 255, 255, src[w] ? (unsigned char) 255 : (unsigned char) 0};
                           }

                           src += target.pitch;
                        }
                     }

                     FT_Bitmap_Done(ft, &target);
                  }

                     // grayscale bitmap
                  else if (glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
                  {
                     character = new Char(code, glyph->bitmap_left, glyph->bitmap_top, glyph->bitmap.width, glyph->bitmap.rows, glyph->advance.x >> 6);

                     unsigned char *src = glyph->bitmap.buffer;

                     for (int r = 0, n = 0; r < glyph->bitmap.rows; r++)
                     {
                        #pragma GCC ivdep
                        for (int w = 0; w < glyph->bitmap.width; w++, n++)
                        {
                           character->buffer[n] = Rgba {255, 255, 255, src[w]};
                        }

                        src += glyph->bitmap.pitch;
                     }
                  }

                     // LCD - subpixel
                  else if (glyph->bitmap.pixel_mode == FT_PIXEL_MODE_LCD)
                  {
                     character = new Char(code, glyph->bitmap_left, glyph->bitmap_top, glyph->bitmap.width / 3, glyph->bitmap.rows, glyph->advance.x >> 6);

                     unsigned char *src = glyph->bitmap.buffer;

                     for (int r = 0, n = 0; r < glyph->bitmap.rows; r++)
                     {
                        #pragma GCC ivdep
                        for (int w = 0; w < glyph->bitmap.width; w += 3, n++)
                        {
                           auto a = (unsigned char) (0.30f * (float) src[w + 0] + 0.59f * (float) src[w + 1] + 0.11f * (float) src[w + 2]);

                           character->buffer[n] = Rgba {src[w + 0], src[w + 1], src[w + 2], a};
                        }

                        src += glyph->bitmap.pitch;
                     }
                  }

                  if (character)
                  {
                     // increase bitmap width
                     width += padding + character->width + character->advance;

                     // update max height
                     height = character->height > height ? character->height : height;

                     // add char to vector
                     characters.emplace_back(character);
                  }
               }
            }

            // release freetype
            FT_Done_Face(face);
            FT_Done_FreeType(ft);

            // font characters
            std::vector<Quad> quads;

            // texture buffer
            Rgba texture[width * height];

            // clear texture buffer
            memset(texture, 0, sizeof(texture));

            // texture character position
            int position = 0;

            for (std::shared_ptr<Char> &character : characters)
            {
               // copy texture width y flip
               for (int row = 0, n = 0; row < character->height; row++)
               {
                  for (int col = 0; col < character->width; col++, n++)
                  {
                     texture[(character->height - row - 1) * width + position + col] = character->buffer[n];
                  }
               }

               // add character to quad vector
               quads.push_back({
                                     .ch = character->ch,

                                     // texture coordinates
                                     .texelLeft = (float) position / (float) width,
                                     .texelRight = (float) (position + character->width) / (float) width,
                                     .texelTop = (float) character->height / (float) size,
                                     .texelBottom = 0,

                                     // normalized glyp position from origin
                                     .glyphLeft = (float) character->left / (float) size,
                                     .glyphRight = (float) (character->left + character->width) / (float) size,
                                     .glyphTop = (float) character->top / (float) size,
                                     .glyphBottom = (float) (character->top - character->height) / (float) size,

                                     // glyp size and advance
                                     .charWidth =  character->width,
                                     .charHeight =  character->height,
                                     .charAdvance =  character->advance});

               // advance character offset inside texture
               position += padding + character->width + character->advance;
            }

            return Font(size, quads, Texture::createTexture(GL_RGBA, texture, sizeof(texture), width, size));
         }
         else
         {
            log.error("failed to load {}", {file});
         }

         return {};
      }

      else
      {
         log.error("freetype could not be initialized!");
      }

      return {};
   }

   Text
   *

   text(const std::string &family, const std::string &text, int size)
   {
      if (auto font = getFont(family, size, 96))
         return new TextImpl(font, text);

      return new TextImpl();
   }

} *impl = nullptr;

Text *FreeType::text(const std::string &family, int size, const std::string &text)
{
   if (!impl)
      impl = new FreeTypeImpl();

   return impl->text(family, text, size);
}

}
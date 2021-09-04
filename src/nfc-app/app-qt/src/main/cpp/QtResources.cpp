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

#include <GL/gl.h>

#include <QDebug>
#include <QFile>
#include <QImageReader>

#include "QtResources.h"

gl::Buffer QtResources::readBuffer(const std::string &name) const
{
   return gl::Buffer();
}

//gl::Font QtResources::readFont(const std::string &name) const
//{
//   QString uri = QString::fromStdString(name);
//
//   qDebug() << "reading font from" << uri;
//
//   QFile file(":" + uri);
//
//   if (file.exists() && file.open(QFile::ReadOnly | QFile::Text))
//   {
//      gl::Quad quad;
//
//      QString index, left, right, top, bottom, advance, spacing;
//
//      QTextStream stream(&file);
//
//      std::vector<gl::Quad> quads;
//
//      while (!stream.atEnd())
//      {
//         stream >> index;
//         stream >> left;
//         stream >> right;
//         stream >> top;
//         stream >> bottom;
//         stream >> advance;
//         stream >> spacing;
//
//         quad.ch = index.toInt();
//         quad.left = left.toFloat();
//         quad.right = right.toFloat();
//         quad.top = top.toFloat();
//         quad.bottom = bottom.toFloat();
//         quad.advance = advance.toFloat();
//         quad.spacing = spacing.toFloat();
//
//         quads.push_back(quad);
//      }
//
//      gl::Texture texture = readImage(uri.replace(".qua", ".png").toStdString());
//
//      return gl::Font(texture, quads);
//   }
//
//   return gl::Font();
//}

gl::Font QtResources::readFont(const std::string &name) const
{
   return gl::Font();
}

gl::Texture QtResources::readImage(const std::string &name) const
{
   QString uri = QString::fromStdString(name);

   qDebug() << "reading image from" << uri;

   QFile file(":" + uri);

   QImageReader reader(&file);

   QImage image = reader.read();

   if (!image.isNull())
   {
      QByteArray buffer;

      QSize size = image.size();

      qDebug() << "\twidth" << size.width();
      qDebug() << "\theight" << size.height();
      qDebug() << "\tformat" << image.format();

      for (int y = 0; y < size.height(); y++)
      {
         for (int x = 0; x < size.width(); x++)
         {
            QRgb pixel = image.pixel(x, y);

            switch (image.format())
            {
               case QImage::Format_RGB32:
               case QImage::Format_ARGB32:
               {
                  buffer.append((char) qRed(pixel)); // RED
                  buffer.append((char) qGreen(pixel)); // GREEN
                  buffer.append((char) qBlue(pixel)); // BLUE
                  buffer.append((char) qAlpha(pixel)); // ALPHA
                  break;
               }

               default:
                  qDebug() << "unsupported pixel format:" << image.format();
                  return gl::Texture();
            }
         }
      }

      return gl::Texture::createTexture(GL_RGBA, buffer.data(), buffer.size(), size.width(), size.height());
   }

   qDebug() << "texture read error:" << reader.errorString();

   return gl::Texture();
}

std::string QtResources::readText(const std::string &name) const
{
   QString uri = QString::fromStdString(name);

   qDebug() << "reading string from" << uri;

   QFile file(":" + uri);

   if (file.exists() && file.open(QFile::ReadOnly | QFile::Text))
   {
      return file.readAll().toStdString();
   }

   return "";
}

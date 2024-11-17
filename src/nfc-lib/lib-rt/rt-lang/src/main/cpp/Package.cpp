/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2024 Jose Vicente Campos Martinez, <josevcm@gmail.com>

  NFC-LABORATORY is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NFC-LABORATORY is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NFC-LABORATORY. If not, see <http://www.gnu.org/licenses/>.

*/

#include <zlib.h>
#include <microtar.h>

#include <rt/Logger.h>
#include <rt/Package.h>

#if defined(OS2) || defined(WIN32) || defined(__CYGWIN__)

#  include <fcntl.h>

#include <utility>

#  define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

namespace rt {

struct Package::Impl
{
   rt::Logger *log = Logger::getLogger("rt.Package");

   std::string filename;

   mtar_t tar;

   explicit Impl(std::string filename) : filename(std::move(filename)), tar({})
   {
      // initialize callbacks
      tar.read = readCallback;
      tar.write = writeCallback;
      tar.seek = seekCallback;
      tar.close = closeCallback;
   }

   ~Impl()
   {
      close();
   }

   int open(Mode mode)
   {
      switch (mode)
      {
         case Read:
         {
            // open GZ file for read
            tar.stream = gzopen(filename.c_str(), "rb");

            // check if file is open
            if (!tar.stream)
            {
               log->error("failed to open compressed file {}", {filename});
               return -1;
            }

            if (mtar_open(&tar, "r") != 0)
            {
               log->error("failed to open tar archive file {}", {filename});
               tar = {};
               return -1;
            }

            return 0;
         }
         case Write:
         {
            // open GZ file for write at maximum compression
            tar.stream = gzopen(filename.c_str(), "wb9");

            // check if file is open
            if (!tar.stream)
            {
               log->error("failed to create compressed file {}", {filename});
               return -1;
            }

            if (mtar_open(&tar, "w") != 0)
            {
               log->error("failed to create archive file {}", {filename});
               tar = {};
               return -1;
            }

            return 0;
         }
         default:
         {
            log->error("failed to open file {}, invalid mode", {filename});
            return -1;
         }
      }
   }

   void close()
   {
      if (tar.stream)
      {
         // finalize tar
         mtar_finalize(&tar);

         // close tar file
         mtar_close(&tar);

         // reset stream
         tar.stream = nullptr;
      }
   }

   int addEntry(const std::string &name, unsigned int length)
   {
      if (!tar.stream)
         return -1;

      return mtar_write_file_header(&tar, name.c_str(), length);
   }

   int getEntry(std::string &name, unsigned int &length)
   {
      if (!tar.stream)
         return -1;

      mtar_header_t h;

      if (mtar_read_header(&tar, &h) != MTAR_ESUCCESS)
         return -1;

      name = std::string(h.name);
      length = h.size;

      return 0;
   }

   int nextEntry()
   {
      if (!tar.stream)
         return -1;

      if (mtar_next(&tar) != MTAR_ESUCCESS)
         return -1;

      return 0;
   }

   int findEntry(const std::string &name, unsigned int &length)
   {
      if (!tar.stream)
         return -1;

      mtar_header_t h;

      if (mtar_find(&tar, name.c_str(), &h) != MTAR_ESUCCESS)
         return -1;

      length = h.size;

      return 0;
   }

   int readData(void *data, unsigned size)
   {
      if (!tar.stream)
         return -1;

      return mtar_read_data(&tar, data, size);
   }

   int writeData(const void *data, unsigned size)
   {
      if (!tar.stream)
         return -1;

      return mtar_write_data(&tar, data, size);
   }

   static int readCallback(mtar_t *tar, void *data, unsigned size)
   {
      int res = gzread((gzFile_s *)tar->stream, data, size);
      return int(res == size ? MTAR_ESUCCESS : MTAR_EREADFAIL);
   }

   static int writeCallback(mtar_t *tar, const void *data, unsigned size)
   {
      int res = gzwrite((gzFile_s *)tar->stream, data, size);
      return int(res == size ? MTAR_ESUCCESS : MTAR_EWRITEFAIL);
   }

   static int seekCallback(mtar_t *tar, unsigned offset)
   {
      int res = gzseek((gzFile_s *)tar->stream, (off_t)offset, SEEK_SET);
      return int(res == offset ? MTAR_ESUCCESS : MTAR_ESEEKFAIL);
   }

   static int closeCallback(mtar_t *tar)
   {
      gzclose((gzFile_s *)tar->stream);
      return int(MTAR_ESUCCESS);
   }
};

Package::Package(const std::string &filename) : impl(new Impl(filename))
{
   impl->filename = filename;
}

int Package::open(Mode mode)
{
   return impl->open(mode);
}

void Package::close()
{
   impl->close();
}

bool Package::isOpen() const
{
   return impl->tar.stream != nullptr;
}

int Package::addEntry(const std::string &name, unsigned int length)
{
   return impl->addEntry(name, length);
}

int Package::getEntry(std::string &name, unsigned int &length)
{
   return impl->getEntry(name, length);
}

int Package::findEntry(const std::string &name, unsigned int &length)
{
   return impl->findEntry(name, length);
}

int Package::nextEntry()
{
   return impl->nextEntry();
}

int Package::readData(void *data, unsigned size)
{
   return impl->readData(data, size);
}

int Package::writeData(const void *data, unsigned size)
{
   return impl->writeData(data, size);
}

}

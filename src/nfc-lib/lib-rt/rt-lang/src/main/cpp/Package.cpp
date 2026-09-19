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

#include <algorithm>
#include <cstdio>
#include <vector>

#include <zlib.h>
#include <zstd.h>
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

// zstd compression level used for new archives (1..22). Measured on a synthetic APCM v3 radio
// entry: level 19 ("high") is ~3.5x slower to compress than the old gzip -9 for only ~2% extra
// size reduction, which does not pay off on the multi-hundred-MB entries an intensive capture
// can produce. Level 12 already beats gzip -9 on both size and speed, so it is the better default;
// existing files compressed with the old gzip-based writer are still auto-detected and read
// transparently, see PackStream::kind below.
#define ZSTD_COMPRESSION_LEVEL 12

namespace rt {

enum class StreamKind
{
   GzipRead,  // legacy archives, written by the previous gzip-based Package implementation
   ZstdRead,
   ZstdWrite
};

// state for the single compressed stream backing a Package: either a legacy gzFile (read only,
// kept for backward compatibility with archives written before this change) or a zstd stream.
struct PackStream
{
   StreamKind kind {};
   std::string filename;

   // legacy gzip read path
   gzFile gz = nullptr;

   // zstd read/write path
   FILE *file = nullptr;
   ZSTD_DStream *dstream = nullptr;
   ZSTD_CCtx *cctx = nullptr;
   std::vector<unsigned char> ioBuf;
   size_t ioPos = 0;
   size_t ioSize = 0;
   unsigned int decodedPos = 0; // read path only: absolute position in the decompressed stream

   ~PackStream()
   {
      if (gz)
         gzclose(gz);

      if (dstream)
         ZSTD_freeDStream(dstream);

      if (cctx)
         ZSTD_freeCCtx(cctx);

      if (file)
         fclose(file);
   }
};

// refills the raw (compressed) input buffer from disk; returns false on EOF
static bool zstdFillInput(PackStream *z)
{
   if (z->ioPos < z->ioSize)
      return true;

   z->ioSize = fread(z->ioBuf.data(), 1, z->ioBuf.size(), z->file);
   z->ioPos = 0;

   return z->ioSize > 0;
}

// decompresses up to 'size' bytes into 'data'; returns the number of bytes actually produced
// (like gzread), which is less than 'size' only at a legitimate end of stream or on error
static int zstdRead(PackStream *z, void *data, unsigned size)
{
   size_t produced = 0;

   while (produced < size)
   {
      if (z->ioPos >= z->ioSize && !zstdFillInput(z))
         break;

      ZSTD_inBuffer input {z->ioBuf.data(), z->ioSize, z->ioPos};
      ZSTD_outBuffer output {data, size, produced};

      const size_t ret = ZSTD_decompressStream(z->dstream, &output, &input);

      z->ioPos = input.pos;
      produced = output.pos;

      if (ZSTD_isError(ret))
         return -1;
   }

   z->decodedPos += static_cast<unsigned int>(produced);

   return static_cast<int>(produced);
}

// zstd has no built-in random access on the stable API, so a backward seek restarts
// decompression from the beginning of the file (the same technique zlib's gzseek uses
// internally for backward seeks: rewind, then decompress-and-discard back up to target)
static bool zstdReopenForRead(PackStream *z)
{
   if (z->file)
      fclose(z->file);

   z->file = fopen(z->filename.c_str(), "rb");

   if (!z->file)
      return false;

   if (!z->dstream)
      z->dstream = ZSTD_createDStream();

   ZSTD_initDStream(z->dstream);

   z->ioPos = z->ioSize = 0;
   z->decodedPos = 0;

   return true;
}

static int zstdSeek(PackStream *z, unsigned int target)
{
   if (target < z->decodedPos && !zstdReopenForRead(z))
      return -1;

   std::vector<unsigned char> scratch(65536);

   while (z->decodedPos < target)
   {
      const unsigned int want = std::min<unsigned int>(static_cast<unsigned int>(scratch.size()), target - z->decodedPos);

      if (zstdRead(z, scratch.data(), want) != static_cast<int>(want))
         return -1;
   }

   return 0;
}

// compresses 'size' bytes from 'data' and streams the result to disk; returns 'size' on success
static int zstdWrite(PackStream *z, const void *data, unsigned size)
{
   ZSTD_inBuffer input {data, size, 0};

   while (input.pos < input.size)
   {
      ZSTD_outBuffer output {z->ioBuf.data(), z->ioBuf.size(), 0};

      const size_t ret = ZSTD_compressStream2(z->cctx, &output, &input, ZSTD_e_continue);

      if (ZSTD_isError(ret))
         return -1;

      if (output.pos && fwrite(z->ioBuf.data(), 1, output.pos, z->file) != output.pos)
         return -1;
   }

   return static_cast<int>(size);
}

// flushes and closes the current zstd frame; must be called once before the file is closed
static bool zstdFinishWrite(PackStream *z)
{
   size_t remaining;

   do
   {
      ZSTD_inBuffer input {nullptr, 0, 0};
      ZSTD_outBuffer output {z->ioBuf.data(), z->ioBuf.size(), 0};

      remaining = ZSTD_compressStream2(z->cctx, &output, &input, ZSTD_e_end);

      if (ZSTD_isError(remaining))
         return false;

      if (output.pos && fwrite(z->ioBuf.data(), 1, output.pos, z->file) != output.pos)
         return false;
   }
   while (remaining > 0);

   return true;
}

struct Package::Impl
{
   rt::Logger *log = Logger::getLogger("rt.Package");

   std::string filename;

   mtar_t tar;

   Mode openMode {};

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
      openMode = mode;

      switch (mode)
      {
         case Read:
            return openRead();

         case Write:
            return openWrite();

         default:
         {
            log->error("failed to open file {}, invalid mode", {filename});
            return -1;
         }
      }
   }

   int openRead()
   {
      // peek the first bytes to tell a legacy gzip archive from a zstd one, so files written
      // by the previous implementation can still be opened
      unsigned char magic[4] = {};

      if (FILE *probe = fopen(filename.c_str(), "rb"))
      {
         const size_t n = fread(magic, 1, sizeof(magic), probe);
         fclose(probe);

         if (n < 2)
         {
            log->error("failed to open compressed file {}", {filename});
            return -1;
         }
      }
      else
      {
         log->error("failed to open compressed file {}", {filename});
         return -1;
      }

      auto *z = new PackStream();
      z->filename = filename;

      if (magic[0] == 0x1F && magic[1] == 0x8B)
      {
         // legacy gzip-compressed archive
         z->kind = StreamKind::GzipRead;
         z->gz = gzopen(filename.c_str(), "rb");

         if (!z->gz)
         {
            log->error("failed to open compressed file {}", {filename});
            delete z;
            return -1;
         }
      }
      else if (magic[0] == 0x28 && magic[1] == 0xB5 && magic[2] == 0x2F && magic[3] == 0xFD)
      {
         z->kind = StreamKind::ZstdRead;
         z->file = fopen(filename.c_str(), "rb");

         if (!z->file)
         {
            log->error("failed to open compressed file {}", {filename});
            delete z;
            return -1;
         }

         z->dstream = ZSTD_createDStream();
         ZSTD_initDStream(z->dstream);
         z->ioBuf.resize(ZSTD_DStreamInSize());
      }
      else
      {
         log->error("unrecognized compressed file format {}", {filename});
         delete z;
         return -1;
      }

      tar.stream = z;

      if (mtar_open(&tar, "r") != 0)
      {
         log->error("failed to open tar archive file {}", {filename});
         delete z;
         tar = {};
         return -1;
      }

      return 0;
   }

   int openWrite()
   {
      auto *z = new PackStream();
      z->filename = filename;
      z->kind = StreamKind::ZstdWrite;
      z->file = fopen(filename.c_str(), "wb");

      if (!z->file)
      {
         log->error("failed to create compressed file {}", {filename});
         delete z;
         return -1;
      }

      z->cctx = ZSTD_createCCtx();
      ZSTD_CCtx_setParameter(z->cctx, ZSTD_c_compressionLevel, ZSTD_COMPRESSION_LEVEL);
      z->ioBuf.resize(ZSTD_CStreamOutSize());

      tar.stream = z;

      if (mtar_open(&tar, "w") != 0)
      {
         log->error("failed to create archive file {}", {filename});
         delete z;
         tar = {};
         return -1;
      }

      return 0;
   }

   void close()
   {
      if (tar.stream)
      {
         // finalize tar (write mode only: this appends the trailing padding records, which
         // makes no sense - and, for the zstd write path, crashes - when reading)
         if (openMode == Write)
            mtar_finalize(&tar);

         // close tar file (invokes closeCallback, which flushes/frees the PackStream)
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
      auto *z = static_cast<PackStream *>(tar->stream);

      const int res = z->kind == StreamKind::GzipRead ? gzread(z->gz, data, size) : zstdRead(z, data, size);

      return res == static_cast<int>(size) ? MTAR_ESUCCESS : MTAR_EREADFAIL;
   }

   static int writeCallback(mtar_t *tar, const void *data, unsigned size)
   {
      auto *z = static_cast<PackStream *>(tar->stream);

      const int res = zstdWrite(z, data, size);

      return res == static_cast<int>(size) ? MTAR_ESUCCESS : MTAR_EWRITEFAIL;
   }

   static int seekCallback(mtar_t *tar, unsigned offset)
   {
      auto *z = static_cast<PackStream *>(tar->stream);

      if (z->kind == StreamKind::GzipRead)
      {
         const auto res = gzseek(z->gz, (off_t) offset, SEEK_SET);
         return int(res == offset ? MTAR_ESUCCESS : MTAR_ESEEKFAIL);
      }

      return zstdSeek(z, offset) == 0 ? MTAR_ESUCCESS : MTAR_ESEEKFAIL;
   }

   static int closeCallback(mtar_t *tar)
   {
      if (auto *z = static_cast<PackStream *>(tar->stream))
      {
         if (z->kind == StreamKind::ZstdWrite)
            zstdFinishWrite(z);

         delete z;

         tar->stream = nullptr;
      }

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

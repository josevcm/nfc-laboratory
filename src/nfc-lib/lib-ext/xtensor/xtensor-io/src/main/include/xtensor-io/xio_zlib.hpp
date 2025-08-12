/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_IO_ZLIB_HPP
#define XTENSOR_IO_ZLIB_HPP

#include <fstream>

#include "zlib.h"

#include "xtensor/xadapt.hpp"
#include "xtensor-io.hpp"
#include "xfile_array.hpp"
#include "xio_stream_wrapper.hpp"

#ifndef ZLIB_CHUNK
#define ZLIB_CHUNK 0x4000
#endif

namespace xt
{
    namespace detail
    {
        inline std::string zlib_err(int ret)
        {
            switch (ret)
            {
                case Z_STREAM_ERROR:
                    return "invalid compression level";
                case Z_DATA_ERROR:
                    return "invalid or incomplete deflate data";
                case Z_MEM_ERROR:
                    return "out of memory";
                case Z_VERSION_ERROR:
                    return "zlib version mismatch";
                default:
                    return "unknown";
            }
        }

        template <typename T, class I>
        inline xt::svector<T> load_zlib(I& stream, bool as_big_endian)
        {
            z_stream strm;
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            strm.avail_in = 0;
            strm.next_in = Z_NULL;
            int ret = inflateInit(&strm);
            if (ret != Z_OK)
            {
                XTENSOR_THROW(std::runtime_error, "zlib decompression failed (" + zlib_err(ret) + ")");
            }
            char in[ZLIB_CHUNK];
            T out[ZLIB_CHUNK / sizeof(T)];
            xt::svector<T> uncompressed_buffer;
            do
            {
                stream.read(in, sizeof(in));
                strm.avail_in = static_cast<unsigned int>(stream.gcount());
                if (strm.avail_in == 0)
                {
                    break;
                }
                strm.next_in = reinterpret_cast<Bytef*>(in);
                do
                {
                    strm.avail_out = ZLIB_CHUNK;
                    strm.next_out = reinterpret_cast<Bytef*>(out);
                    ret = inflate(&strm, Z_NO_FLUSH);
                    if (ret == Z_STREAM_ERROR)
                    {
                        XTENSOR_THROW(std::runtime_error, "zlib decompression failed (" + zlib_err(Z_STREAM_ERROR) + ")");
                    }
                    switch (ret) {
                        case Z_NEED_DICT:
                            ret = Z_DATA_ERROR;
                        case Z_DATA_ERROR:
                        case Z_MEM_ERROR:
                            static_cast<void>(inflateEnd(&strm));
                            XTENSOR_THROW(std::runtime_error, "zlib decompression failed (" + zlib_err(ret) + ")");
                    }
                    unsigned int have = ZLIB_CHUNK - strm.avail_out;
                    uncompressed_buffer.insert(std::end(uncompressed_buffer), out, out + have / sizeof(T));
                }
                while (strm.avail_out == 0);
            }
            while (ret != Z_STREAM_END);

            static_cast<void>(inflateEnd(&strm));
            if (ret != Z_STREAM_END)
            {
                XTENSOR_THROW(std::runtime_error, "zlib decompression failed (" + zlib_err(Z_DATA_ERROR) + ")");
            }
            if ((sizeof(T) > 1) && (as_big_endian != is_big_endian()))
            {
                swap_endianness(uncompressed_buffer);
            }
            return uncompressed_buffer;
        }

        template <class O, class E>
        inline void dump_zlib(O& stream, const xexpression<E>& e, bool as_big_endian, int level)
        {
            using value_type = typename E::value_type;
            const E& ex = e.derived_cast();
            auto&& eval_ex = eval(ex);
            auto shape = eval_ex.shape();
            std::size_t size = compute_size(shape);
            std::size_t uncompressed_size = size * sizeof(value_type);
            const char* uncompressed_buffer;
            xt::svector<value_type> swapped_buffer;
            if ((sizeof(value_type) > 1) && (as_big_endian != is_big_endian()))
            {
                swapped_buffer.resize(size);
                std::copy(eval_ex.data(), eval_ex.data() + size, swapped_buffer.begin());
                swap_endianness(swapped_buffer);
                uncompressed_buffer = reinterpret_cast<const char*>(swapped_buffer.data());
            }
            else
            {
                uncompressed_buffer = reinterpret_cast<const char*>(eval_ex.data());
            }

            z_stream strm;
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            int ret = deflateInit(&strm, level);
            if (ret != Z_OK)
            {
                XTENSOR_THROW(std::runtime_error, "zlib compression failed (" + zlib_err(ret) + ")");
            }

            const char* in = uncompressed_buffer;
            char out[ZLIB_CHUNK];
            int flush;
            do
            {
                strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(in));
                in += ZLIB_CHUNK;
                flush = (in >= uncompressed_buffer + uncompressed_size) ? Z_FINISH : Z_NO_FLUSH;
                long int in_size = (flush == Z_FINISH) ? (ZLIB_CHUNK - (in - (uncompressed_buffer + uncompressed_size))) : ZLIB_CHUNK;
                strm.avail_in = static_cast<uInt>(in_size);
                do
                {
                    strm.avail_out = ZLIB_CHUNK;
                    strm.next_out = reinterpret_cast<Bytef*>(out);
                    ret = deflate(&strm, flush);
                    if (ret == Z_STREAM_ERROR)
                    {
                        XTENSOR_THROW(std::runtime_error, "zlib compression failed (" + zlib_err(Z_STREAM_ERROR) + ")");
                    }
                    unsigned int have = ZLIB_CHUNK - strm.avail_out;
                    stream.write(reinterpret_cast<const char*>(out), static_cast<std::streamsize>(have));
                }
                while (strm.avail_out == 0);
                if (strm.avail_in != 0)
                {
                    XTENSOR_THROW(std::runtime_error, "zlib compression failed (remaining input data)");
                }
            }
            while (flush != Z_FINISH);
            if (ret != Z_STREAM_END)
            {
                XTENSOR_THROW(std::runtime_error, "zlib compression failed (stream not complete)");
            }
            static_cast<void>(deflateEnd(&strm));
            stream.flush();
        }
    }  // namespace detail

    /**
     * Save xexpression to ZLIB format
     *
     * @param stream An output stream to which to dump the data
     * @param e the xexpression
     */
    template <typename E, class O>
    inline void dump_zlib(O& stream, const xexpression<E>& e, bool as_big_endian=is_big_endian(), int level=1)
    {
        detail::dump_zlib(stream, e, as_big_endian, level);
    }

    template <typename E>
    inline void dump_zlib(std::ostream& stream, const xexpression<E>& e, bool as_big_endian=is_big_endian(), int level=1)
    {
        auto s = xostream_wrapper(stream);
        detail::dump_zlib(s, e, as_big_endian, level);
    }

    /**
     * Save xexpression to ZLIB format
     *
     * @param filename The filename or path to dump the data
     * @param e the xexpression
     */
    template <typename E>
    inline void dump_zlib(const char* filename, const xexpression<E>& e, bool as_big_endian=is_big_endian(), int level=1)
    {
        std::ofstream stream(filename, std::ofstream::binary);
        if (!stream.is_open())
        {
            std::runtime_error("IO Error: failed to open file");
        }
        auto s = xostream_wrapper(stream);
        detail::dump_zlib(s, e, as_big_endian, level);
    }

    template <typename E>
    inline void dump_zlib(const std::string& filename, const xexpression<E>& e, bool as_big_endian=is_big_endian(), int level=1)
    {
        dump_zlib<E>(filename.c_str(), e, as_big_endian, level);
    }

    /**
     * Save xexpression to ZLIB format in a string
     *
     * @param e the xexpression
     */
    template <typename E>
    inline std::string dump_zlib(const xexpression<E>& e, bool as_big_endian=is_big_endian(), int level=1)
    {
        std::stringstream stream;
        auto s = xostream_wrapper(stream);
        detail::dump_zlib(s, e, as_big_endian, level);
        return stream.str();
    }

    /**
     * Loads a ZLIB file
     *
     * @param stream An input stream from which to load the file
     * @tparam T select the type of the ZLIB file
     * @tparam L select layout_type::column_major if you stored data in
     *           Fortran format
     * @return xarray with contents from ZLIB file
     */
    template <typename T, layout_type L = layout_type::dynamic, class I>
    inline auto load_zlib(I& stream, bool as_big_endian=is_big_endian())
    {
        xt::svector<T> uncompressed_buffer = detail::load_zlib<T>(stream, as_big_endian);
        std::vector<std::size_t> shape = {uncompressed_buffer.size()};
        auto array = adapt(std::move(uncompressed_buffer), shape);
        return array;
    }

    /**
     * Loads a ZLIB file
     *
     * @param filename The filename or path to the file
     * @tparam T select the type of the ZLIB file
     * @tparam L select layout_type::column_major if you stored data in
     *           Fortran format
     * @return xarray with contents from ZLIB file
     */
    template <typename T, layout_type L = layout_type::dynamic>
    inline auto load_zlib(const char* filename, bool as_big_endian=is_big_endian())
    {
        std::ifstream stream(filename, std::ifstream::binary);
        if (!stream.is_open())
        {
            std::runtime_error(std::string("load_zlib: failed to open file ") + filename);
        }
        auto s = xistream_wrapper(stream);
        return load_zlib<T, L>(s, as_big_endian);
    }

    template <typename T, layout_type L = layout_type::dynamic>
    inline auto load_zlib(const std::string& filename, bool as_big_endian=is_big_endian())
    {
        return load_zlib<T, L>(filename.c_str(), as_big_endian);
    }

    struct xio_zlib_config
    {
        std::string name;
        std::string version;
        bool big_endian;
        int level;

        xio_zlib_config()
            : name("zlib")
            , version(ZLIB_VERSION)
            , big_endian(is_big_endian())
            , level(1)
        {
        }

        template <class T>
        void write_to(T& j) const
        {
            j["level"] = level;
        }

        template <class T>
        void read_from(T& j)
        {
            level = j["level"];
        }

        bool will_dump(xfile_dirty dirty)
        {
            return dirty.data_dirty;
        }
    };

    template <class E, class I>
    void load_file(I& stream, xexpression<E>& e, const xio_zlib_config& config)
    {
        E& ex = e.derived_cast();
        auto shape = ex.shape();
        ex = load_zlib<typename E::value_type>(stream, config.big_endian);
        if (!shape.empty())
        {
            if (compute_size(shape) != ex.size())
            {
                XTENSOR_THROW(std::runtime_error, "load_file: size mismatch");
            }
            ex.reshape(shape);
        }
    }

    template <class E, class O>
    void dump_file(O& stream, const xexpression<E> &e, const xio_zlib_config& config)
    {
        dump_zlib(stream, e, config.big_endian, config.level);
    }
}  // namespace xt

#endif

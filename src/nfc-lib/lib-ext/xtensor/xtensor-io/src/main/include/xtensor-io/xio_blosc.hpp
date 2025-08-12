/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_IO_BLOSC_HPP
#define XTENSOR_IO_BLOSC_HPP

#include <fstream>

#include "xtensor/xadapt.hpp"
#include "xtensor-io.hpp"
#include "xfile_array.hpp"
#include "blosc.h"
#include "xio_stream_wrapper.hpp"

namespace xt
{
    namespace detail
    {
        inline void init_blosc()
        {
            static bool initialized = false;
            if (!initialized)
            {
                blosc_init();
                initialized = true;
            }
        }

        template <typename T, class I>
        inline xt::svector<T> load_blosc(I& stream, bool as_big_endian)
        {
            init_blosc();
            std::string compressed_buffer;
            stream.read_all(compressed_buffer);
            auto compressed_size = compressed_buffer.size();
            std::size_t uncompressed_size = 0;
            int res = blosc_cbuffer_validate(compressed_buffer.data(), compressed_size, &uncompressed_size);
            if (res == -1)
            {
                XTENSOR_THROW(std::runtime_error, "Blosc: unsupported file format version");
            }
            size_t ubuf_size = uncompressed_size / sizeof(T);
            if (uncompressed_size % sizeof(T) != size_t(0))
                ubuf_size += size_t(1);
            xt::svector<T> uncompressed_buffer(ubuf_size);
            res = blosc_decompress(compressed_buffer.data(), uncompressed_buffer.data(), uncompressed_size);
            if (res <= 0)
            {
                XTENSOR_THROW(std::runtime_error, "Blosc: unsupported file format version");
            }
            if ((sizeof(T) > 1) && (as_big_endian != is_big_endian()))
            {
                swap_endianness(uncompressed_buffer);
            }
            return uncompressed_buffer;
        }

        template <class O, class E>
        inline void dump_blosc(O& stream, const xexpression<E>& e, bool as_big_endian, int clevel, int shuffle, const char* cname, std::size_t blocksize)
        {
            init_blosc();
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
            std::size_t max_compressed_size = uncompressed_size + BLOSC_MAX_OVERHEAD;
            std::allocator<char> char_allocator;
            char* compressed_buffer = char_allocator.allocate(max_compressed_size);
            blosc_set_blocksize(blocksize);
            if (blosc_set_compressor(cname) == -1)
            {
                XTENSOR_THROW(std::runtime_error, "Blosc: compressor not supported (" + std::string(cname) + ")");
            }
            int true_compressed_size = blosc_compress(clevel, shuffle, sizeof(value_type), uncompressed_size, uncompressed_buffer, compressed_buffer, max_compressed_size);
            if (true_compressed_size == 0)
            {
                XTENSOR_THROW(std::runtime_error, "Blosc: buffer is uncompressible");
            }
            else if (true_compressed_size < 0)
            {
                XTENSOR_THROW(std::runtime_error, "Blosc: compression error");
            }
            stream.write(compressed_buffer, std::streamsize(true_compressed_size));
            stream.flush();
            char_allocator.deallocate(compressed_buffer, max_compressed_size);
        }
    }  // namespace detail

    /**
     * Save xexpression to blosc format
     *
     * @param stream An output stream to which to dump the data
     * @param e the xexpression
     */
    template <typename E, class O>
    inline void dump_blosc(O& stream, const xexpression<E>& e, bool as_big_endian=is_big_endian(), int clevel=5, int shuffle=1, const char* cname="blosclz", std::size_t blocksize=0)
    {
        detail::dump_blosc(stream, e, as_big_endian, clevel, shuffle, cname, blocksize);
    }

    template <typename E>
    inline void dump_blosc(std::ostream& stream, const xexpression<E>& e, bool as_big_endian=is_big_endian(), int clevel=5, int shuffle=1, const char* cname="blosclz", std::size_t blocksize=0)
    {
        auto s = xostream_wrapper(stream);
        detail::dump_blosc(s, e, as_big_endian, clevel, shuffle, cname, blocksize);
    }

    /**
     * Save xexpression to blosc format
     *
     * @param filename The filename or path to dump the data
     * @param e the xexpression
     */
    template <typename E>
    inline void dump_blosc(const char* filename, const xexpression<E>& e, bool as_big_endian=is_big_endian(), int clevel=5, int shuffle=1, const char* cname="blosclz", std::size_t blocksize=0)
    {
        std::ofstream stream(filename, std::ofstream::binary);
        if (!stream.is_open())
        {
            XTENSOR_THROW(std::runtime_error, std::string("Blosc: failed to open file ") + filename);
        }
        auto s = xostream_wrapper(stream);
        detail::dump_blosc(s, e, as_big_endian, clevel, shuffle, cname, blocksize);
    }

    template <typename E>
    inline void dump_blosc(const std::string& filename, const xexpression<E>& e, bool as_big_endian=is_big_endian(), int clevel=5, int shuffle=1, const char* cname="blosclz", std::size_t blocksize=0)
    {
        dump_blosc<E>(filename.c_str(), e, as_big_endian, clevel, shuffle, cname, blocksize);
    }

    /**
     * Save xexpression to blosc format in a string
     *
     * @param e the xexpression
     */
    template <typename E>
    inline std::string dump_blosc(const xexpression<E>& e, bool as_big_endian=is_big_endian(), int clevel=5, int shuffle=1, const char* cname="blosclz", std::size_t blocksize=0)
    {
        std::stringstream stream;
        auto s = xostream_wrapper(stream);
        detail::dump_blosc(s, e, as_big_endian, clevel, shuffle, cname, blocksize);
        return stream.str();
    }

    /**
     * Loads a blosc file
     *
     * @param stream An input stream from which to load the file
     * @tparam T select the type of the blosc file
     * @tparam L select layout_type::column_major if you stored data in
     *           Fortran format
     * @return xarray with contents from blosc file
     */
    template <typename T, layout_type L = layout_type::dynamic, class I>
    inline auto load_blosc(I& stream, bool as_big_endian=is_big_endian())
    {
        xt::svector<T> uncompressed_buffer = detail::load_blosc<T>(stream, as_big_endian);
        std::vector<std::size_t> shape = {uncompressed_buffer.size()};
        auto array = adapt(std::move(uncompressed_buffer), shape);
        return array;
    }

    /**
     * Loads a blosc file
     *
     * @param filename The filename or path to the file
     * @tparam T select the type of the blosc file
     * @tparam L select layout_type::column_major if you stored data in
     *           Fortran format
     * @return xarray with contents from blosc file
     */
    template <typename T, layout_type L = layout_type::dynamic>
    inline auto load_blosc(const char* filename, bool as_big_endian=is_big_endian())
    {
        std::ifstream stream(filename, std::ifstream::binary);
        if (!stream.is_open())
        {
            XTENSOR_THROW(std::runtime_error, std::string("Blosc: failed to open file ") + filename);
        }
        auto s = xistream_wrapper(stream);;
        return load_blosc<T, L>(s, as_big_endian);
    }

    template <typename T, layout_type L = layout_type::dynamic>
    inline auto load_blosc(const std::string& filename, bool as_big_endian=is_big_endian())
    {
        return load_blosc<T, L>(filename.c_str(), as_big_endian);
    }

    struct xio_blosc_config
    {
        std::string name;
        std::string version;
        bool big_endian;
        int clevel;
        int shuffle;
        std::string cname;
        std::size_t blocksize;

        xio_blosc_config()
            : name("blosc")
            , version(BLOSC_VERSION_STRING)
            , big_endian(is_big_endian())
            , clevel(5)
            , shuffle(1)
            , cname("blosclz")
            , blocksize(0)
        {
        }

        template <class T>
        void write_to(T& j) const
        {
            j["clevel"] = clevel;
            j["shuffle"] = shuffle;
            j["cname"] = cname;
            j["blocksize"] = blocksize;
        }

        template <class T>
        void read_from(T& j)
        {
            clevel = j["clevel"];
            shuffle = j["shuffle"];
            cname = std::string(j["cname"]);
            blocksize = j["blocksize"];
        }

        bool will_dump(xfile_dirty dirty)
        {
            return dirty.data_dirty;
        }
    };

    template <class E, class I>
    void load_file(I& stream, xexpression<E>& e, const xio_blosc_config& config)
    {
        E& ex = e.derived_cast();
        auto shape = ex.shape();
        ex = load_blosc<typename E::value_type>(stream, config.big_endian);
        if (!shape.empty())
        {
            if (compute_size(shape) != ex.size())
            {
                XTENSOR_THROW(std::runtime_error, "Blosc: expected size (" + std::to_string(compute_size(shape)) + ") and actual size (" + std::to_string(ex.size()) + ") mismatch");
            }
            ex.reshape(shape);
        }
    }

    template <class E, class O>
    void dump_file(O& stream, const xexpression<E> &e, const xio_blosc_config& config)
    {
        dump_blosc(stream, e, config.big_endian, config.clevel, config.shuffle, config.cname.c_str(), config.blocksize);
    }
}  // namespace xt

#endif

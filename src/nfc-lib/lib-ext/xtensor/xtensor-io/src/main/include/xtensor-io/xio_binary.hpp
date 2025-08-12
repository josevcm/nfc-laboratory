/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_IO_BINARY_HPP
#define XTENSOR_IO_BINARY_HPP

#include <fstream>

#include "xtensor/xadapt.hpp"
#include "xtensor-io.hpp"
#include "xfile_array.hpp"
#include "xio_stream_wrapper.hpp"

namespace xt
{
    namespace detail
    {
        template <typename T, class I>
        inline xt::svector<T> load_bin(I& stream, bool as_big_endian)
        {
            std::string buffer;
            stream.read_all(buffer);
            std::size_t uncompressed_size = buffer.size() / sizeof(T);
            xt::svector<T> uncompressed_buffer(uncompressed_size);
            std::copy((const T*)(buffer.data()), (const T*)(buffer.data()) + uncompressed_size, uncompressed_buffer.begin());
            if ((sizeof(T) > 1) && (as_big_endian != is_big_endian()))
            {
                swap_endianness(uncompressed_buffer);
            }
            return uncompressed_buffer;
        }

        template <class O, class E>
        inline void dump_bin(O& stream, const xexpression<E>& e, bool as_big_endian)
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
            stream.write(uncompressed_buffer, uncompressed_size);
            stream.flush();
        }
    }  // namespace detail

    /**
     * Save xexpression to binary format
     *
     * @param stream An output stream to which to dump the data
     * @param e the xexpression
     */
    template <typename E, class O>
    inline void dump_bin(O& stream, const xexpression<E>& e, bool as_big_endian=is_big_endian())
    {
        detail::dump_bin(stream, e, as_big_endian);
    }

    template <typename E>
    inline void dump_bin(std::ofstream& stream, const xexpression<E>& e, bool as_big_endian=is_big_endian())
    {
        auto s = xostream_wrapper(stream);
        detail::dump_bin(s, e, as_big_endian);
    }

    /**
     * Save xexpression to binary format
     *
     * @param filename The filename or path to dump the data
     * @param e the xexpression
     */
    template <typename E>
    inline void dump_bin(const char* filename, const xexpression<E>& e, bool as_big_endian=is_big_endian())
    {
        std::ofstream stream(filename, std::ofstream::binary);
        if (!stream.is_open())
        {
            std::runtime_error("IO Error: failed to open file");
        }
        auto s = xostream_wrapper(stream);
        detail::dump_bin(s, e, as_big_endian);
    }

    template <typename E>
    inline void dump_bin(const std::string& filename, const xexpression<E>& e, bool as_big_endian=is_big_endian())
    {
        dump_bin<E>(filename.c_str(), e, as_big_endian);
    }

    /**
     * Save xexpression to binary format in a string
     *
     * @param e the xexpression
     */
    template <typename E>
    inline std::string dump_bin(const xexpression<E>& e, bool as_big_endian=is_big_endian())
    {
        std::stringstream stream;
        auto s = xostream_wrapper(stream);
        detail::dump_bin(s, e, as_big_endian);
        return stream.str();
    }

    /**
     * Loads a binary file
     *
     * @param stream An input stream from which to load the file
     * @tparam T select the type of the binary file
     * @tparam L select layout_type::column_major if you stored data in
     *           Fortran format
     * @return xarray with contents from binary file
     */
    template <typename T, layout_type L = layout_type::dynamic, class I>
    inline auto load_bin(I& stream, bool as_big_endian=is_big_endian())
    {
        xt::svector<T> uncompressed_buffer = detail::load_bin<T>(stream, as_big_endian);
        std::vector<std::size_t> shape = {uncompressed_buffer.size()};
        auto array = adapt(std::move(uncompressed_buffer), shape);
        return array;
    }

    /**
     * Loads a binary file
     *
     * @param filename The filename or path to the file
     * @tparam T select the type of the binary file
     * @tparam L select layout_type::column_major if you stored data in
     *           Fortran format
     * @return xarray with contents from binary file
     */
    template <typename T, layout_type L = layout_type::dynamic>
    inline auto load_bin(const char* filename, bool as_big_endian=is_big_endian())
    {
        std::ifstream stream(filename, std::ifstream::binary);
        if (!stream.is_open())
        {
            std::runtime_error(std::string("load_bin: failed to open file ") + filename);
        }
        auto s = xistream_wrapper(stream);
        return load_bin<T, L>(s, as_big_endian);
    }

    template <typename T, layout_type L = layout_type::dynamic>
    inline auto load_bin(const std::string& filename, bool as_big_endian=is_big_endian())
    {
        return load_bin<T, L>(filename.c_str(), as_big_endian);
    }

    struct xio_binary_config
    {
        std::string name;
        std::string version;
        bool big_endian;

        xio_binary_config()
            : name("binary")
            , version("1.0")
            , big_endian(is_big_endian())
        {
        }

        template <class T>
        void write_to(T& j) const
        {
        }

        template <class T>
        void read_from(T& j)
        {
        }

        bool will_dump(xfile_dirty dirty)
        {
            return dirty.data_dirty;
        }
    };

    template <class E, class I>
    void load_file(I& stream, xexpression<E>& e, const xio_binary_config& config)
    {
        E& ex = e.derived_cast();
        auto shape = ex.shape();
        ex = load_bin<typename E::value_type>(stream, config.big_endian);
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
    void dump_file(O& stream, const xexpression<E> &e, const xio_binary_config& config)
    {
        dump_bin(stream, e, config.big_endian);
    }
}  // namespace xt

#endif

/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_COMPRESSOR_HPP
#define XTENSOR_ZARR_COMPRESSOR_HPP

#include "xzarr_common.hpp"
#include "xtensor-io/xchunk_store_manager.hpp"
#include "xtensor-io/xio_binary.hpp"
#include "zarray/zarray.hpp"
#include "xtl/xhalf_float.hpp"

namespace xt
{
    template <class T>
    inline T get_nan()
    {
        return 0;
    }

    template <>
    inline float get_nan<float>()
    {
        return std::nanf("");
    }

    template <>
    inline double get_nan<double>()
    {
        return std::nan("");
    }

    template <>
    inline long double get_nan<long double>()
    {
        return std::nanl("");
    }

    template <class store_type, class data_type, class io_handler, class format_config>
    zarray build_chunked_array_impl(store_type& store, char chunk_memory_layout, std::vector<std::size_t>& shape, std::vector<std::size_t>& chunk_shape, const std::string& path, char separator, const nlohmann::json& attrs, char endianness, format_config&& config, const nlohmann::json& config_json, std::size_t chunk_pool_size, const nlohmann::json& fill_value_json, std::size_t zarr_version)
    {
        config.read_from(config_json);
        config.big_endian = (endianness == '>');
        layout_type layout;
        switch (chunk_memory_layout)
        {
            case 'C':
                layout = layout_type::row_major;
                break;
            case 'F':
                layout = layout_type::column_major;
                break;
            default:
                XTENSOR_THROW(std::runtime_error, "Unrecognized chunk memory layout: " + std::string(1, chunk_memory_layout));
        }
        if (fill_value_json.is_null())
        {
            auto a = chunked_file_array<data_type, io_handler, layout_type::dynamic, xzarr_index_path>(shape, chunk_shape, path, chunk_pool_size, layout);
            auto& i2p = a.chunks().get_index_path();
            i2p.set_separator(separator);
            i2p.set_zarr_version(zarr_version);
            auto io_config = store.get_io_config();
            a.chunks().configure(config, io_config);
            auto z = zarray(std::move(a));
            auto metadata = z.get_metadata();
            metadata["zarr"] = attrs;
            z.set_metadata(metadata);
            return z;
        }
        else
        {
            data_type fill_value;
            if (fill_value_json == "NaN")
            {
                fill_value = get_nan<data_type>();
            }
            else
            {
                fill_value = fill_value_json;
            }
            auto a = chunked_file_array<data_type, io_handler, layout_type::dynamic, xzarr_index_path>(shape, chunk_shape, path, fill_value, chunk_pool_size, layout);
            auto& i2p = a.chunks().get_index_path();
            i2p.set_separator(separator);
            i2p.set_zarr_version(zarr_version);
            auto io_config = store.get_io_config();
            a.chunks().configure(config, io_config);
            auto z = zarray(std::move(a));
            auto metadata = z.get_metadata();
            metadata["zarr"] = attrs;
            z.set_metadata(metadata);
            return z;
        }
    }

    template <class store_type, class data_type, class format_config>
    zarray build_chunked_array_with_compressor(store_type& store, char chunk_memory_layout, std::vector<std::size_t>& shape, std::vector<std::size_t>& chunk_shape, const std::string& path, char separator, const nlohmann::json& attrs, char endianness, nlohmann::json& config, std::size_t chunk_pool_size, const nlohmann::json& fill_value_json, std::size_t zarr_version)
    {
        using io_handler = typename store_type::template io_handler<format_config>;
        return build_chunked_array_impl<store_type, data_type, io_handler>(store, chunk_memory_layout, shape, chunk_shape, path, separator, attrs, endianness, format_config(), config, chunk_pool_size, fill_value_json, zarr_version);
    }

    template <class store_type, class data_type>
    class xcompressor_factory
    {
    public:

        template <class format_config>
        static void add_compressor(format_config&& c)
        {
            auto fun = instance().m_builders.find(c.name);
            if (fun != instance().m_builders.end())
            {
                XTENSOR_THROW(std::runtime_error, "Compressor already registered: " + std::string(c.name));
            }
            instance().m_builders.insert(std::make_pair(c.name, &build_chunked_array_with_compressor<store_type, data_type, format_config>));
        }

        static zarray build(store_type& store, const std::string& compressor, char chunk_memory_layout, std::vector<std::size_t>& shape, std::vector<std::size_t>& chunk_shape, const std::string& path, char separator, const nlohmann::json& attrs, char endianness, nlohmann::json& config, std::size_t chunk_pool_size, const nlohmann::json& fill_value_json, std::size_t zarr_version)
        {
            auto fun = instance().m_builders.find(compressor);

            if (fun != instance().m_builders.end())
            {
                zarray z = (fun->second)(store, chunk_memory_layout, shape, chunk_shape, path, separator, attrs, endianness, config, chunk_pool_size, fill_value_json, zarr_version);
                return z;
            }

            XTENSOR_THROW(std::runtime_error, "Unkown compressor type: " + compressor);
        }

    private:

        using self_type = xcompressor_factory;

        static self_type& instance()
        {
            static self_type instance;
            return instance;
        }

        xcompressor_factory()
        {
            using format_config = xio_binary_config;
            m_builders.insert(std::make_pair(format_config().name, &build_chunked_array_with_compressor<store_type, data_type, format_config>));
        }

        std::map<std::string, zarray (*)(store_type& store, char chunk_memory_layout, std::vector<std::size_t>& shape, std::vector<std::size_t>& chunk_shape, const std::string& path, char separator, const nlohmann::json& attrs, char endianness, nlohmann::json& config, std::size_t chunk_pool_size, const nlohmann::json& fill_value_json, std::size_t zarr_version)> m_builders;
    };

    template <class store_type, class format_config>
    void xzarr_register_compressor()
    {
        xcompressor_factory<store_type, bool>::add_compressor(format_config());
        xcompressor_factory<store_type, int8_t>::add_compressor(format_config());
        xcompressor_factory<store_type, int16_t>::add_compressor(format_config());
        xcompressor_factory<store_type, int32_t>::add_compressor(format_config());
        xcompressor_factory<store_type, int64_t>::add_compressor(format_config());
        xcompressor_factory<store_type, uint8_t>::add_compressor(format_config());
        xcompressor_factory<store_type, uint16_t>::add_compressor(format_config());
        xcompressor_factory<store_type, uint32_t>::add_compressor(format_config());
        xcompressor_factory<store_type, uint64_t>::add_compressor(format_config());
        xcompressor_factory<store_type, xtl::half_float>::add_compressor(format_config());
        xcompressor_factory<store_type, float>::add_compressor(format_config());
        xcompressor_factory<store_type, double>::add_compressor(format_config());
    }

}

#endif

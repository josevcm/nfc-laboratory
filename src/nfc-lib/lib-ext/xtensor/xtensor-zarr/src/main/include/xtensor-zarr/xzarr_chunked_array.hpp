/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_CHUNKED_ARRAY_HPP
#define XTENSOR_ZARR_CHUNKED_ARRAY_HPP

#include "xtensor/xchunked_array.hpp"
#include "zarray/zarray.hpp"
#include "xzarr_common.hpp"
#include "xzarr_compressor.hpp"

namespace xt
{
    template <class store_type, class data_type>
    zarray build_chunked_array_with_dtype(store_type& store, const std::string& compressor, char chunk_memory_layout, std::vector<std::size_t>& shape, std::vector<std::size_t>& chunk_shape, const std::string& path, char separator, const nlohmann::json& attrs, char endianness, nlohmann::json& config, std::size_t chunk_pool_size, const nlohmann::json& fill_value_json, std::size_t zarr_version)
    {
        return xcompressor_factory<store_type, data_type>::build(store, compressor, chunk_memory_layout, shape, chunk_shape, path, separator, attrs, endianness, config, chunk_pool_size, fill_value_json, zarr_version);
    }

    template <class store_type>
    class xchunked_array_factory
    {
    public:

        template <class data_type>
        static void add_dtype(const std::string& name)
        {
            auto fun = instance().m_builders.find(name);
            if (fun != instance().m_builders.end())
            {
                XTENSOR_THROW(std::runtime_error, "Data type already registered: " + std::string(name));
            }
            instance().m_builders.insert(std::make_pair(name, &build_chunked_array_with_dtype<store_type, data_type>));
        }

        static zarray build(store_type& store, const std::string& compressor, const std::string& dtype, char chunk_memory_layout, std::vector<std::size_t>& shape, std::vector<std::size_t>& chunk_shape, const std::string& path, char separator, const nlohmann::json& attrs, nlohmann::json& config, std::size_t chunk_pool_size, const nlohmann::json& fill_value_json, std::size_t zarr_version)
        {
            std::string dtype_noendian = dtype;
            char endianness = dtype[0];
            if ((dtype[0] == '<') || (dtype[0] == '>'))
            {
                dtype_noendian = dtype.substr(1);
            }
            else if ((zarr_version == 2) && (dtype[0] == '|'))
            {
                dtype_noendian = dtype.substr(1);
            }
            auto fun = instance().m_builders.find(dtype_noendian);
            if (fun != instance().m_builders.end())
            {
                zarray z = (fun->second)(store, compressor, chunk_memory_layout, shape, chunk_shape, path, separator, attrs, endianness, config, chunk_pool_size, fill_value_json, zarr_version);
                return z;
            }
            else
            {
                XTENSOR_THROW(std::runtime_error, "Unknown data type: " + dtype);
            }
        }

    private:

        using self_type = xchunked_array_factory;

        static self_type& instance()
        {
            static self_type instance;
            return instance;
        }

        xchunked_array_factory()
        {
            m_builders.insert(std::make_pair("bool", &build_chunked_array_with_dtype<store_type, bool>));
            // signed integer
            m_builders.insert(std::make_pair("i1", &build_chunked_array_with_dtype<store_type, int8_t>));
            m_builders.insert(std::make_pair("i2", &build_chunked_array_with_dtype<store_type, int16_t>));
            m_builders.insert(std::make_pair("i4", &build_chunked_array_with_dtype<store_type, int32_t>));
            m_builders.insert(std::make_pair("i8", &build_chunked_array_with_dtype<store_type, int64_t>));
            // unsigned integer
            m_builders.insert(std::make_pair("u1", &build_chunked_array_with_dtype<store_type, uint8_t>));
            m_builders.insert(std::make_pair("u2", &build_chunked_array_with_dtype<store_type, uint16_t>));
            m_builders.insert(std::make_pair("u4", &build_chunked_array_with_dtype<store_type, uint32_t>));
            m_builders.insert(std::make_pair("u8", &build_chunked_array_with_dtype<store_type, uint64_t>));
            // float
            m_builders.insert(std::make_pair("f2", &build_chunked_array_with_dtype<store_type, xtl::half_float>));
            m_builders.insert(std::make_pair("f4", &build_chunked_array_with_dtype<store_type, float>));
            m_builders.insert(std::make_pair("f8", &build_chunked_array_with_dtype<store_type, double>));
        }

        std::map<std::string, zarray (*)(store_type& store, const std::string& compressor, char chunk_memory_layout, std::vector<std::size_t>& shape, std::vector<std::size_t>& chunk_shape, const std::string& path, char separator, const nlohmann::json& attrs, char endianness, nlohmann::json& config, std::size_t chunk_pool_size, const nlohmann::json& fill_value_json, std::size_t zarr_version)> m_builders;
    };
}

#endif

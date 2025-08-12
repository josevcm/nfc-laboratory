/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_ARRAY_HPP
#define XTENSOR_ZARR_ARRAY_HPP

#include "nlohmann/json.hpp"
#include "zarray/zarray.hpp"
#include "xtensor-io/xio_binary.hpp"
#include "xzarr_chunked_array.hpp"
#include "xzarr_common.hpp"

namespace xt
{
    template <class store_type, class shape_type, class C>
    zarray create_zarr_array(store_type store, const std::string& path, shape_type shape, shape_type chunk_shape, const std::string& dtype, char chunk_memory_layout, char chunk_separator, const C& compressor, const nlohmann::json& attrs, std::size_t chunk_pool_size, const nlohmann::json& fill_value, const std::size_t zarr_version_major)
    {
        nlohmann::json j;
        nlohmann::json compressor_config;
        switch (zarr_version_major)
        {
            case 3:
                j["chunk_grid"]["type"] = "regular";
                j["chunk_grid"]["chunk_shape"] = chunk_shape;
                if (chunk_separator == 0)
                {
                    chunk_separator = '/';
                }
                j["chunk_grid"]["separator"] = std::string(1, chunk_separator);
                j["data_type"] = dtype;
                j["chunk_memory_layout"] = std::string(1, chunk_memory_layout);
                if (compressor.name != "binary")
                {
                    j["compressor"]["codec"] = "https://purl.org/zarr/spec/codec/" + compressor.name + "/1.0";
                    compressor.write_to(compressor_config);
                    j["compressor"]["configuration"] = compressor_config;
                }
                j["attributes"] = attrs;
                j["extensions"] = nlohmann::json::array();
                break;
            case 2:
                j["chunks"] = chunk_shape;
                if (chunk_separator == 0)
                {
                    chunk_separator = '.';
                }
                else
                {
                    j["dimension_separator"] = std::string(1, chunk_separator);
                }
                j["dtype"] = dtype;
                j["order"] = std::string(1, chunk_memory_layout);
                if (compressor.name == "binary")
                {
                    j["compressor"] = nlohmann::json();
                }
                else
                {
                    compressor.write_to(compressor_config);
                    j["compressor"] = compressor_config;
                    j["compressor"]["id"] = compressor.name;
                }
                j["filters"] = nlohmann::json();
                j["zarr_format"] = 2;
                break;
            default:
                break;
        }
        j["shape"] = shape;
        j["fill_value"] = fill_value;
        std::string full_path;
        switch (zarr_version_major)
        {
            case 3:
                store["meta/root" + path + ".array.json"] = j.dump(4);
                full_path = store.get_root() + "/data/root" + path;
                break;
            case 2:
                store[path + "/.zarray"] = j.dump(4);
                full_path = store.get_root() + '/' + path;
                if (!attrs.empty())
                {
                    store[path + "/.zattrs"] = attrs.dump(4);
                }
                break;
            default:
                break;
        }
        return xchunked_array_factory<store_type>::build(store, compressor.name, dtype, chunk_memory_layout, shape, chunk_shape, full_path, chunk_separator, attrs, compressor_config, chunk_pool_size, fill_value, zarr_version_major);
    }

    template <class store_type>
    zarray get_zarr_array(store_type store, const std::string& path, std::size_t chunk_pool_size, const std::size_t zarr_version_major)
    {
        std::string s;
        nlohmann::json attrs;
        std::vector<std::string> keys;
        std::vector<std::string> prefixes;
        switch (zarr_version_major)
        {
            case 3:
                s = store[std::string("meta/root") + path + ".array.json"];
                break;
            case 2:
                s = store[path + "/.zarray"];
                store.list_dir(path, keys, prefixes);
                if (std::count(keys.begin(), keys.end(), ".zattrs"))
                {
                    attrs = std::string(store[path + "/.zattrs"]);
                }
                break;
            default:
                break;
        }
        auto j = nlohmann::json::parse(s);
        auto json_shape = j["shape"];
        nlohmann::json json_chunk_shape;
        std::string dtype;
        std::string chunk_memory_layout;
        std::string compressor;
        nlohmann::json compressor_config;
        std::string chunk_separator;
        std::string full_path;
        switch (zarr_version_major)
        {
            case 3:
                json_chunk_shape = j["chunk_grid"]["chunk_shape"];
                dtype = j["data_type"];
                chunk_memory_layout = j["chunk_memory_layout"];
                if (j.contains("compressor"))
                {
                    compressor = j["compressor"]["codec"];
                    std::size_t i;
                    i = compressor.rfind('/');
                    compressor = compressor.substr(0, i);
                    i = compressor.rfind('/') + 1;
                    compressor = compressor.substr(i, std::string::npos);
                    compressor_config = j["compressor"]["configuration"];
                }
                else
                {
                    compressor = "binary";
                }
                chunk_separator = j["chunk_grid"]["separator"];
                full_path = store.get_root() + "/data/root" + path;
                attrs = j["attributes"];
                break;
            case 2:
                json_chunk_shape = j["chunks"];
                dtype = j["dtype"];
                chunk_memory_layout = j["order"];
                if (j["compressor"].is_null())
                {
                    compressor = "binary";
                }
                else
                {
                    compressor = j["compressor"]["id"];
                    compressor_config = j["compressor"];
                    compressor_config.erase("id");
                }
                if (j.contains("dimension_separator"))
                {
                    chunk_separator = j["dimension_separator"];
                }
                else
                {
                    chunk_separator = '.';
                }
                full_path = store.get_root() + '/' + path;
                break;
            default:
                break;
        }
        std::vector<std::size_t> shape(json_shape.size());
        std::vector<std::size_t> chunk_shape(json_chunk_shape.size());
        std::transform(json_shape.begin(), json_shape.end(), shape.begin(),
                       [](nlohmann::json& size) -> int { return stoi(size.dump()); });
        std::transform(json_chunk_shape.begin(), json_chunk_shape.end(), chunk_shape.begin(),
                       [](nlohmann::json& size) -> int { return stoi(size.dump()); });
        return xchunked_array_factory<store_type>::build(store, compressor, dtype, chunk_memory_layout[0], shape, chunk_shape, full_path, chunk_separator[0], attrs, compressor_config, chunk_pool_size, j["fill_value"], zarr_version_major);
    }
}

#endif

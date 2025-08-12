/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_HIERARCHY_HPP
#define XTENSOR_ZARR_HIERARCHY_HPP

#include "nlohmann/json.hpp"
#include "zarray/zarray.hpp"
#include "xzarr_node.hpp"
#include "xzarr_array.hpp"
#include "xzarr_group.hpp"
#include "xzarr_common.hpp"
#include "xzarr_file_system_store.hpp"
#include "xzarr_gdal_store.hpp"
#include "xtensor_zarr_config.hpp"
#include "xtensor-io/xio_gzip.hpp"
#include "xtensor-io/xio_zlib.hpp"
#include "xtensor-io/xio_blosc.hpp"

namespace xt
{
    /**
     * @class xzarr_hierarchy
     * @brief Zarr hierarchy handler.
     *
     * The xzarr_hierarchy class implements a handler for creating and accessing
     * a hierarchy, an array or a group, as well as exploring the hierarchy.
     *
     * @tparam store_type The type of the store (e.g. xzarr_file_system_store)
     * @sa zarray, xzarr_group, xzarr_node
     */
    template <class store_type>
    class xzarr_hierarchy
    {
    public:
        xzarr_hierarchy(store_type& store, const std::string& zarr_version = "3");

        void check_hierarchy();
        void create_hierarchy();

        template <class shape_type, class O = xzarr_create_array_options<xio_binary_config>>
        zarray create_array(const std::string& path, shape_type shape, shape_type chunk_shape, const std::string& dtype, O o=O());

        zarray get_array(const std::string& path, std::size_t chunk_pool_size=1);

        xzarr_group<store_type> create_group(const std::string& path, const nlohmann::json& attrs=nlohmann::json::object(), const nlohmann::json& extensions=nlohmann::json::array());

        xzarr_node<store_type> operator[](const std::string& path);

        nlohmann::json get_children(const std::string& path="/");
        nlohmann::json get_nodes(const std::string& path="/");

    private:
        store_type m_store;
        std::size_t m_zarr_version_major;
    };

    /**********************************
     * xzarr_hierarchy implementation *
     **********************************/

    template <class store_type>
    xzarr_hierarchy<store_type>::xzarr_hierarchy(store_type& store, const std::string& zarr_version)
        : m_store(store)
        , m_zarr_version_major(get_zarr_version_major(zarr_version))
    {
    }

    template <class store_type>
    void xzarr_hierarchy<store_type>::check_hierarchy()
    {
        std::string s;
        if (m_zarr_version_major == 3)
        {
            std::string s = m_store["zarr.json"];
            auto j = nlohmann::json::parse(s);
            if (!j.contains("zarr_format"))
            {
                XTENSOR_THROW(std::runtime_error, "Not a Zarr hierarchy: " + m_store.get_root());
            }
        }
    }

    template <class store_type>
    void xzarr_hierarchy<store_type>::create_hierarchy()
    {
        if (m_zarr_version_major == 3)
        {
            nlohmann::json j;
            j["zarr_format"] = "https://purl.org/zarr/spec/protocol/core/3.0";
            j["metadata_encoding"] = "https://purl.org/zarr/spec/protocol/core/3.0";
            j["metadata_key_suffix"] = ".json";
            j["extensions"] = nlohmann::json::array();

            m_store["zarr.json"] = j.dump(4);
        }
    }

    template <class store_type>
    template <class shape_type, class O>
    zarray xzarr_hierarchy<store_type>::create_array(const std::string& path, shape_type shape, shape_type chunk_shape, const std::string& dtype, O o)
    {
        return create_zarr_array(m_store, path, shape, chunk_shape, dtype, o.chunk_memory_layout, o.chunk_separator, o.compressor, o.attrs, o.chunk_pool_size, o.fill_value, m_zarr_version_major);
    }


    template <class store_type>
    zarray xzarr_hierarchy<store_type>::get_array(const std::string& path, std::size_t chunk_pool_size)
    {
        return get_zarr_array(m_store, path, chunk_pool_size, m_zarr_version_major);
    }

    template <class store_type>
    xzarr_group<store_type> xzarr_hierarchy<store_type>::create_group(const std::string& path, const nlohmann::json& attrs, const nlohmann::json& extensions)
    {
        xzarr_group<store_type> g(m_store, path, m_zarr_version_major);
        return g.create_group(attrs, extensions);
    }

    template <class store_type>
    xzarr_node<store_type> xzarr_hierarchy<store_type>::operator[](const std::string& path)
    {
        return xzarr_node<store_type>(m_store, path, m_zarr_version_major);
    }

    template <class store_type>
    nlohmann::json xzarr_hierarchy<store_type>::get_children(const std::string& path)
    {
        return xzarr_node<store_type>(m_store, path, m_zarr_version_major).get_children();
    }

    template <class store_type>
    nlohmann::json xzarr_hierarchy<store_type>::get_nodes(const std::string& path)
    {
        return xzarr_node<store_type>(m_store, path, m_zarr_version_major).get_nodes();
    }

    /************************************
     * zarr hierarchy factory functions *
     ************************************/

    /**
     * Creates a Zarr hierarchy.
     * This function creates a hierarchy in a store and returns a ``xzarr_hierarchy`` handler to it.
     *
     * @tparam store_type The type of the store (e.g. xzarr_file_system_store)
     *
     * @param store The hierarchy store
     *
     * @return returns a ``xzarr_hierarchy`` handler.
     */
    template <class store_type>
    inline xzarr_hierarchy<store_type> create_zarr_hierarchy(store_type& store, const std::string& zarr_version = "3")
    {
        xzarr_hierarchy<store_type> h(store, zarr_version);
        h.create_hierarchy();
        return h;
    }

    inline xzarr_hierarchy<xzarr_file_system_store> create_zarr_hierarchy(const char* local_store_path, const std::string& zarr_version = "3")
    {
        xzarr_file_system_store store(local_store_path);
        return create_zarr_hierarchy(store, zarr_version);
    }

    inline xzarr_hierarchy<xzarr_file_system_store> create_zarr_hierarchy(const std::string& local_store_path, const std::string& zarr_version = "3")
    {
        return create_zarr_hierarchy(local_store_path.c_str(), zarr_version);
    }

    /**
     * Accesses a Zarr hierarchy.
     * This function returns a ``xzarr_hierarchy`` handler to a hierarchy in a given store.
     *
     * @tparam store_type The type of the store (e.g. xzarr_file_system_store)
     *
     * @param store The hierarchy store
     * @param zarr_version The version of the Zarr specification for the store
     *
     * @return returns a ``xzarr_hierarchy`` handler.
     */
    template <class store_type>
    inline xzarr_hierarchy<store_type> get_zarr_hierarchy(store_type& store, const std::string& zarr_version = "")
    {
        std::string zarr_ver;
        if (zarr_version.empty())
        {
            std::vector<std::string> keys;
            std::vector<std::string> prefixes;
            store.list_dir("", keys, prefixes);
            if (std::count(keys.begin(), keys.end(), "zarr.json"))
            {
                zarr_ver = "3";
            }
            else
            {
                zarr_ver = "2";
            }
        }
        else
        {
            zarr_ver = zarr_version;
        }
        xzarr_hierarchy<store_type> h(store, zarr_ver);
        h.check_hierarchy();
        return h;
    }

    inline xzarr_hierarchy<xzarr_file_system_store> get_zarr_hierarchy(const char* local_store_path, const std::string& zarr_version = "")
    {
        xzarr_file_system_store store(local_store_path);
        return get_zarr_hierarchy(store, zarr_version);
    }

    inline xzarr_hierarchy<xzarr_file_system_store> get_zarr_hierarchy(const std::string& local_store_path, const std::string& zarr_version = "")
    {
        return get_zarr_hierarchy(local_store_path.c_str(), zarr_version);
    }

    /*********************
     * precompiled types *
     *********************/

    extern template void xzarr_register_compressor<xzarr_gdal_store, xio_gzip_config>();
    extern template void xzarr_register_compressor<xzarr_gdal_store, xio_zlib_config>();
    extern template void xzarr_register_compressor<xzarr_gdal_store, xio_blosc_config>();
    extern template class xchunked_array_factory<xzarr_gdal_store>;

    extern template void xzarr_register_compressor<xzarr_file_system_store, xio_gzip_config>();
    extern template void xzarr_register_compressor<xzarr_file_system_store, xio_zlib_config>();
    extern template void xzarr_register_compressor<xzarr_file_system_store, xio_blosc_config>();
    extern template class xchunked_array_factory<xzarr_file_system_store>;
}

#endif

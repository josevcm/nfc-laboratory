/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_NODE_HPP
#define XTENSOR_ZARR_NODE_HPP

#include "nlohmann/json.hpp"
#include "xzarr_array.hpp"
#include "xzarr_group.hpp"
#include "xzarr_common.hpp"

namespace xt
{
    enum class xzarr_node_type { implicit_group, explicit_group, array };

    inline bool endswith(const std::string& str, const std::string& end)
    {
        if (str.length() >= end.length())
        {
            return (0 == str.compare(str.length() - end.length(), end.length(), end));
        }
        else
        {
            return false;
        }
    }

    template <class store_type>
    class xzarr_node
    {
    public:
        xzarr_node(store_type& store, const std::string& path, const std::size_t zarr_version_major);

        xzarr_group<store_type> create_group(const std::string& name, const nlohmann::json& attrs=nlohmann::json::object(), const nlohmann::json& extensions=nlohmann::json::array());

        template <class shape_type, class O = xzarr_create_array_options<xio_binary_config>>
        zarray create_array(const std::string& name, shape_type shape, shape_type chunk_shape, const std::string& dtype, O o=O());

        zarray get_array(std::size_t chunk_pool_size=1);
        xzarr_group<store_type> get_group();
        nlohmann::json get_children();
        nlohmann::json get_nodes();
        xzarr_node<store_type> operator[](const std::string& name);
        bool is_group();
        bool is_array();

    private:
        store_type& m_store;
        nlohmann::json m_json;
        std::string m_path;
        xzarr_node_type m_node_type;
        std::size_t m_zarr_version_major;
    };

    template <class store_type>
    xzarr_node<store_type>::xzarr_node(store_type& store, const std::string& path, const std::size_t zarr_version_major)
        : m_store(store)
        , m_zarr_version_major(zarr_version_major)
    {
        m_path = path;
        if (m_path.front() != '/')
        {
            m_path = '/' + m_path;
        }
        while (m_path.back() == '/')
        {
            m_path = m_path.substr(0, m_path.size() - 1);
        }
        std::string file_path;
        bool done = false;
        if (!done)
        {
            file_path = "meta/root" + m_path + ".group.json";
            if (m_store[file_path].exists())
            {
                m_node_type = xzarr_node_type::explicit_group;
                done = true;
            }
        }
        if (!done)
        {
            file_path = "meta/root" + m_path + ".array.json";
            if (m_store[file_path].exists())
            {
                m_node_type = xzarr_node_type::array;
                done = true;
            }
        }
        if (!done)
        {
            m_node_type = xzarr_node_type::implicit_group;
        }
    }

    template <class store_type>
    xzarr_group<store_type> xzarr_node<store_type>::create_group(const std::string& name, const nlohmann::json& attrs, const nlohmann::json& extensions)
    {
        m_node_type = xzarr_node_type::explicit_group;
        xzarr_group<store_type> g(m_store, m_path + '/' + name, m_zarr_version_major);
        return g.create_group(attrs, extensions);
    }

    template <class store_type>
    xzarr_group<store_type> xzarr_node<store_type>::get_group()
    {
        if (!is_group())
        {
            XTENSOR_THROW(std::runtime_error, "Node is not a group: " + m_path);
        }
        xzarr_group<store_type> g(m_store, m_path, m_zarr_version_major);
        return g;
    }

    template <class store_type>
    template <class shape_type, class O>
    zarray xzarr_node<store_type>::create_array(const std::string& name, shape_type shape, shape_type chunk_shape, const std::string& dtype, O o)
    {
        m_node_type = xzarr_node_type::array;
        return create_zarr_array(m_store, m_path + '/' + name, shape, chunk_shape, dtype, o.chunk_memory_layout, o.chunk_separator, o.compressor, o.attrs, o.chunk_pool_size, o.fill_value, m_zarr_version_major);
    }

    template <class store_type>
    zarray xzarr_node<store_type>::get_array(std::size_t chunk_pool_size)
    {
        if (!is_array())
        {
            XTENSOR_THROW(std::runtime_error, "Node is not an array: " + m_path);
        }
        return get_zarr_array(m_store, m_path, chunk_pool_size, m_zarr_version_major);
    }

    template <class store_type>
    nlohmann::json xzarr_node<store_type>::get_children()
    {
        nlohmann::json j;
        std::vector<std::string> keys;
        std::vector<std::string> prefixes;
        std::string full_path = "meta/root" + m_path;
        if (full_path.back() != '/')
        {
            full_path.push_back('/');
        }
        m_store.list_dir(full_path, keys, prefixes);
        for (const auto& prefix: prefixes)
        {
            std::string name = prefix.substr(full_path.size());
            j[name] = "implicit_group";
        }
        for (const auto& key: keys)
        {
            std::string name = key.substr(full_path.size());
            // remove trailing ".array.json" or ".group.json"
            name = name.substr(0, name.size() - 11);
            if ((*this)[name].is_array())
            {
                j[name] = "array";
            }
            else
            {
                j[name] = "explicit_group";
            }
        }
        return j;
    }

    template <class store_type>
    nlohmann::json xzarr_node<store_type>::get_nodes()
    {
        nlohmann::json j;
        std::string full_path = "meta/root" + m_path;
        if (full_path.back() != '/')
        {
            full_path.push_back('/');
        }
        std::vector<std::string> keys = m_store.list_prefix(full_path);
        for (auto key: keys)
        {
            if (endswith(key, ".array.json"))
            {
                j[key.substr(full_path.size(), key.size() - 11 - full_path.size())] = "array";
            }
            else if (endswith(key, ".group.json"))
            {
                j[key.substr(full_path.size(), key.size() - 11 - full_path.size())] = "explicit_group";
            }
            // now check for implicit groups
            std::string name = key.substr(full_path.size(), key.size() - 11 - full_path.size());
            while (true)
            {
                std::size_t i = name.rfind('/');
                if (i == std::string::npos)
                {
                    break;
                }
                name = name.substr(0, i);
                if (j.find(name) != j.end())
                {
                    break;
                }
                j[name] = "implicit_group";
            }
        }
        return j;
    }

    template <class store_type>
    xzarr_node<store_type> xzarr_node<store_type>::operator[](const std::string& name)
    {
        return xzarr_node<store_type>(m_store, m_path + '/' + name, m_zarr_version_major);
    }

    template <class store_type>
    bool xzarr_node<store_type>::is_group()
    {
        return (m_node_type == xzarr_node_type::explicit_group) || (m_node_type == xzarr_node_type::implicit_group);
    }

    template <class store_type>
    bool xzarr_node<store_type>::is_array()
    {
        return m_node_type == xzarr_node_type::array;
    }
}

#endif

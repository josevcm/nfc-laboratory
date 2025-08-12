/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_GROUP_HPP
#define XTENSOR_ZARR_GROUP_HPP

#include "nlohmann/json.hpp"

namespace xt
{
    template <class store_type>
    class xzarr_group
    {
    public:
        xzarr_group(store_type& store, const std::string& path, const std::size_t zarr_version_major);

        xzarr_group create_group(const nlohmann::json& attrs=nlohmann::json::object(), const nlohmann::json& extensions=nlohmann::json::array());

        nlohmann::json attrs();
        std::string name();
        std::string path();
    private:
        store_type& m_store;
        nlohmann::json m_json;
        std::string m_path;
        std::size_t m_zarr_version_major;
    };

    template <class store_type>
    xzarr_group<store_type>::xzarr_group(store_type& store, const std::string& path, const std::size_t zarr_version_major)
        : m_store(store)
        , m_path(path)
        , m_zarr_version_major(zarr_version_major)
    {
        if (zarr_version_major == 3)
        {
            auto f = m_store["meta/root" + m_path + ".group.json"];
            if (f.exists())
            {
                m_json = nlohmann::json::parse(std::string(f));
            }
        }
    }

    template <class store_type>
    xzarr_group<store_type> xzarr_group<store_type>::create_group(const nlohmann::json& attrs, const nlohmann::json& extensions)
    {
        m_json = nlohmann::json::object();
        switch (m_zarr_version_major)
        {
            case 3:
                m_json["attributes"] = attrs;
                m_json["extensions"] = extensions;
                m_store["meta/root" + m_path + ".group.json"] = m_json.dump(4);
                break;
            case 2:
                m_json["zarr_format"] = 2;
                m_store[m_path + ".zgroup"] = m_json.dump(4);
                break;
            default:
                break;
        }
        return *this;
    }

    template <class store_type>
    nlohmann::json xzarr_group<store_type>::attrs()
    {
        return m_json["attributes"];
    }

    template <class store_type>
    std::string xzarr_group<store_type>::name()
    {
        std::size_t i = m_path.rfind('/');
        return m_path.substr(i + 1);
    }

    template <class store_type>
    std::string xzarr_group<store_type>::path()
    {
        return m_path;
    }
}

#endif

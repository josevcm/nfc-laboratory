/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTL_JSON_HPP
#define XTL_JSON_HPP

// WARNING:
// All the code in this file and in the
// files it includes must be C++11 compliant,
// otherwise it breaks xeus-cling C++11 kernel

#include <cstddef>
#include <string>
#include "xtl/xvariant.hpp"
#include "nlohmann/json.hpp"

namespace xtl
{
    /***********************************************************
     * to_json and from_json specialization for xtl::xoptional *
     ***********************************************************/

    // xoptional forward declaration.
    template <class D, class B>
    class xoptional;

    template <class T>
    xoptional<T, bool> missing() noexcept;

    // to_json and from_json ADL overload
    template <class D, class B>
    void to_json(nlohmann::json& j, const xoptional<D, B>& o)
    {
        if (!o.has_value())
        {
            j = nullptr;
        }
        else
        {
            j = o.value();
        }
    }

    template <class D, class B>
    void from_json(const nlohmann::json& j, xoptional<D, B>& o)
    {
        if (j.is_null())
        {
            o = missing<D>();
        }
        else
        {
            o = j.get<D>();
        }
    }

    /********************************************************************
     * to_json and from_json specialization for xtl::basic_fixed_string *
     ********************************************************************/

    // xbasic_fixed_string forward declaration.
    template <class CT, std::size_t N, int ST, template <std::size_t> class EP, class TR>
    class xbasic_fixed_string;

    // to_json and from_json ADL overload
    template <class CT, std::size_t N, int ST, template <std::size_t> class EP, class TR>
    void to_json(::nlohmann::json& j, const xbasic_fixed_string<CT, N, ST, EP, TR>& str)
    {
        j = str.c_str();
    }

    template <class CT, std::size_t N, int ST, template <std::size_t> class EP, class TR>
    void from_json(const ::nlohmann::json& j, xbasic_fixed_string<CT, N, ST, EP, TR>& str)
    {
        str = j.get<std::string>();
    }
}

// overloading in the mpark namespace because `xtl::variant` is just a typedef on `mpark::variant`
namespace mpark
{
    template <class... Ts>
    void to_json(nlohmann::json& j, const xtl::variant<Ts...>& data) {
        xtl::visit
        (
            [&j] (const auto & arg) { j = arg; },
            data
        );
    }
}

#endif

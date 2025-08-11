/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZMPL_TYPES_HPP
#define XTENSOR_ZMPL_TYPES_HPP

#include <type_traits>

#include "xtensor/xexpression.hpp"

namespace xt
{
    class zarray;
    class zarray_expression_tag;

    namespace detail
    {
        template<class T>
        struct is_zarray : public std::false_type
        {
        };
        template<>
        struct is_zarray<zarray> : public std::true_type
        {
        };

        template<class E>
        using enable_zarray_t = std::enable_if_t<is_zarray<E>::value>;

        template<class E>
        using has_zexpression_tag = std::is_same<extension::get_expression_tag_t<std::decay_t<E>>, zarray_expression_tag>;

        template<class E>
        using enable_zexpressions_t = std::enable_if_t<has_zexpression_tag<E>::value>;


        template<class E>
        using disable_zarray_enable_zexpressions_t = std::enable_if_t<
            !is_zarray<E>::value &&
            has_zexpression_tag<E>::value
        >;
    }

}
#endif
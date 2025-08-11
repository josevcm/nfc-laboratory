/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZFUNCTORS_HPP
#define XTENSOR_ZFUNCTORS_HPP

#include "zassign.hpp"

namespace xt
{

    template <class T, class U, class R = void>
    using enable_same_types_t = std::enable_if_t<std::is_same<T, U>::value, R>;

    template <class T, class U, class R = void>
    using disable_same_types_t = std::enable_if_t<!std::is_same<T, U>::value, R>;


    template <class XF>
    struct get_zmapped_functor;

    template <class XF>
    using get_zmapped_functor_t = typename get_zmapped_functor<XF>::type;

    namespace detail
    {
        struct xassign_dummy_functor {};
        struct xmove_dummy_functor {};
    }

    struct zassign_functor
    {
        template <class T, class R>
        static void run(const ztyped_array<T>& z, ztyped_array<R>& zres, const zassign_args& args)
        {
            if (!args.chunk_assign)
                zassign_wrapped_expression(zres, z.get_array(), args);
            else
                zassign_wrapped_expression(zres,  z.get_chunk(args.slices()), args);
        }

        template <class T>
        static size_t index(const ztyped_array<T>&)
        {
            return ztyped_array<T>::get_class_static_index();
        }
    };

    struct zmove_functor
    {
        template <class T, class R>
        static disable_same_types_t<T, R> run(const ztyped_array<T>& z, ztyped_array<R>& zres, const zassign_args& args)
        {
            // resize is not called in the move constructor of zarray
            // to avoid useless dyanmic allocation if RHS is about
            // to be moved, therefore we have to call it here.
            zres.resize(z.shape());
            if (!args.chunk_assign)
                zassign_wrapped_expression(zres, z.get_array(), args);
            else
                zassign_wrapped_expression(zres, z.get_chunk(args.slices()), args);
        }

        template <class T, class R>
        static enable_same_types_t<T, R> run(const ztyped_array<T>& z, ztyped_array<R>& zres, const zassign_args& args)
        {
            if (zres.is_array())
            {
                ztyped_array<T>& uz = const_cast<ztyped_array<T>&>(z);
                xarray<T>& ar = uz.get_array();
                zres.get_array() = std::move(ar);
            }
            else if (zres.is_chunked())
            {
                zassign_wrapped_expression(zres, z.get_chunk(args.slices()), args);
            }
            else
            {
                using array_type = ztyped_expression_wrapper<T>;
                array_type& lhs = static_cast<array_type&>(zres);
                ztyped_array<T>& uz = const_cast<ztyped_array<T>&>(z);
                lhs.assign(std::move(uz.get_array()));
            }
        }

        template <class T>
        static size_t index(const ztyped_array<T>&)
        {
            return ztyped_array<T>::get_class_static_index();
        }
    };

    template <>
    struct get_zmapped_functor<detail::xassign_dummy_functor>
    {
        using type = zassign_functor;
    };

    template <>
    struct get_zmapped_functor<detail::xmove_dummy_functor>
    {
        using type = zmove_functor;
    };
}

#endif
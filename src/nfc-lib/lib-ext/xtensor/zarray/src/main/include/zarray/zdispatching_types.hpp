/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZDISPATCHING_TYPES_HPP
#define XTENSOR_ZDISPATCHING_TYPES_HPP

#include <xtl/xmeta_utils.hpp>

namespace xt
{
    namespace mpl = xtl::mpl;

    // TODO: move to XTL
    namespace detail
    {
        template <class... L>
        struct concatenate;

        template <class... T, class... U>
        struct concatenate<mpl::vector<T...>, mpl::vector<U...>>
        {
            using type = mpl::vector<T..., U...>;
        };

        template <class... T, class... U, class... L>
        struct concatenate<mpl::vector<T...>, mpl::vector<U...>, L...>
        {
            using type = typename concatenate<
                            typename concatenate<
                                mpl::vector<T...>,
                                mpl::vector<U...>
                            >::type,
                            L...
                         >::type;
        };

        template <class... L>
        using concatenate_t = typename concatenate<L...>::type;


        // while mpl::transformer_t take template of template
        // as first argument we need smth which takes 
        // a class holding a templated meta function 
        template <class F, class L>
        struct meta_func_transform_impl;

        template < class F, template <class...> class L, class... T>
        struct meta_func_transform_impl<F, L<T...>>
        {
            using type = L<typename F:: template type <T>...>;
        };
    

        template <class F, class L>
        struct  meta_func_transform : meta_func_transform_impl<F, L>
        {
        };

        template < class F, class L>
        using meta_func_transform_t = typename  meta_func_transform<F, L>::type;


        template <class L>
        struct pairwise_combinations_impl;

        template <template <class...> class L, class ... T>
        struct pairwise_combinations_impl<L<T ...>>
        {   
            using vector_t =  mpl::vector<T ...>;

            template <class U>
            struct helper
            {
                template<class R>
                using type = mpl::vector<U, R>;
            };

            // unfortunately mpl::transform_t does not work here
            template<class U>
            using transformer_t = meta_func_transform_t<helper<U>, vector_t>;
 
            using type = concatenate_t< transformer_t< T > ...>;
        };

        template<class ...L>
        using pairwise_combinations_t = typename pairwise_combinations_impl<L ...>::type;
    }
    /***********
     * z types *
     ***********/

    using z_int_types = mpl::vector<uint8_t, int8_t,
                                    uint16_t, int16_t,
                                    uint32_t, int32_t,
                                    uint64_t, int64_t>;
    using z_small_int_types = mpl::vector<uint8_t, int8_t, uint16_t, int16_t>;
    using z_big_int_types = mpl::vector<uint32_t, int32_t, uint64_t, int64_t>;
    using z_float_types = mpl::vector<float, double>;

    using z_types = detail::concatenate_t<z_int_types,
                                          z_float_types>;

    /*************************
     * unary operation types *
     *************************/

    template <class T, class R>
    struct build_unary_impl
    {
        using type = mpl::vector<T, R>;
    };

    template <class T, class R>
    using build_unary_impl_t = typename build_unary_impl<T, R>::type;

    template <class T>
    using build_unary_identity_t = build_unary_impl_t<T, T>;

    template <class T>
    using build_unary_bool_t = build_unary_impl_t<T, bool>;

    template <class T>
    using build_unary_int32_t = build_unary_impl_t<T, int32_t>;

    template <class T>
    using build_unary_int64_t = build_unary_impl_t<T, int64_t>;

    template <class T>
    using build_unary_double_t = build_unary_impl_t<T, double>;



    using zunary_all_ztypes_combinations_types = detail::pairwise_combinations_t<z_types>;

    using zunary_ident_types = mpl::transform_t<build_unary_identity_t, z_types>;

    using zunary_func_types = detail::concatenate_t<
                                  mpl::transform_t<build_unary_identity_t, z_float_types>,
                                  mpl::transform_t<build_unary_double_t, z_int_types>,
                                  mpl::transform_t<build_unary_double_t, z_int_types>,
                                  mpl::transform_t<build_unary_int64_t, z_float_types>
                              >;

    using zunary_op_types = detail::concatenate_t<
                                mpl::transform_t<build_unary_identity_t, z_big_int_types>,
                                mpl::transform_t<build_unary_identity_t, z_float_types>,
                                mpl::transform_t<build_unary_int32_t, z_small_int_types>
                            >;

    using zreducer_types = detail::concatenate_t<
                            mpl::transform_t<build_unary_identity_t, z_big_int_types>,
                            mpl::transform_t<build_unary_identity_t, z_float_types>,
                            mpl::transform_t<build_unary_int32_t, z_small_int_types>,
                            mpl::transform_t<build_unary_double_t, mpl::vector<float>>,
                            mpl::transform_t<build_unary_double_t, z_small_int_types>,
                            mpl::transform_t<build_unary_double_t, z_big_int_types>
                        >;

    using zunary_bool_func_types = mpl::transform_t<build_unary_bool_t, z_types>;

    using zunary_classify_types = mpl::transform_t<build_unary_bool_t, z_float_types>;

    /**************************
     * binary operation types *
     **************************/

    template <class T, class U, class R>
    struct build_binary_impl
    {
        using type = mpl::vector<T, U, R>;
    };
    
    template <class T, class U, class R>
    using build_binary_impl_t = typename build_binary_impl<T, U, R>::type;

    template <class T>
    using build_binary_identity_t = build_binary_impl_t<T, T, T>;

    template <class T>
    using build_binary_int32_t = build_binary_impl_t<T, T, int32_t>;

    template <class T>
    using build_binary_int64_t = build_binary_impl_t<T, T, int64_t>;

    template <class T>
    using build_binary_double_t = build_binary_impl_t<T, T, double>;

    using zbinary_func_types = detail::concatenate_t<
                                   mpl::transform_t<build_binary_identity_t, z_float_types>,
                                   mpl::transform_t<build_binary_double_t, z_int_types>
                               >;

    using zbinary_op_types = detail::concatenate_t<
                                 mpl::transform_t<build_binary_identity_t, z_big_int_types>,
                                 mpl::transform_t<build_binary_identity_t, z_float_types>,
                                 mpl::transform_t<build_binary_int32_t, z_small_int_types>
                             >;

    using zbinary_int_op_types = mpl::transform_t<build_binary_identity_t, z_int_types>;

}

#endif


/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZDISPATCHER_HPP
#define XTENSOR_ZDISPATCHER_HPP

#include <xtl/xmultimethods.hpp>

#include "zdispatching_types.hpp"
#include "zfunctors.hpp"

namespace xt
{
    // forward declaration
    class zreducer_options;

    // reducer functor forward declarations
    struct zsum_zreducer_functor;
    struct zprod_zreducer_functor;
    struct zmean_zreducer_functor;
    struct zvariance_zreducer_functor;
    struct zstddev_zreducer_functor;
    struct zamin_zreducer_functor;
    struct zamax_zreducer_functor;
    struct znorm_l0_zreducer_functor;
    struct znorm_l1_zreducer_functor;
    struct znorm_l2_zreducer_functor;
    struct znorm_sq_zreducer_functor;
    struct znorm_linf_zreducer_functor;
    struct znorm_lp_to_p_zreducer_functor;
    struct znorm_induced_l1_zreducer_functor;
    struct znorm_induced_linf_zreducer_functor;

    namespace mpl = xtl::mpl;

    template <class type_list, class undispatched_type_list = mpl::vector<const zassign_args>>
    using zrun_dispatcher_impl = xtl::functor_dispatcher
    <
        type_list,
        void,
        undispatched_type_list,
        xtl::static_caster,
        xtl::basic_fast_dispatcher
    >;

    template <class type_list, class undispatched_type_list = mpl::vector<>>
    using ztype_dispatcher_impl = xtl::functor_dispatcher
    <
        type_list,
        size_t,
        undispatched_type_list,
        xtl::static_caster,
        xtl::basic_fast_dispatcher
    >;

    /**********************
     * zdouble_dispatcher *
     **********************/

    // Double dispatchers are used for unary operations.
    // They dispatch on the single argument and on the
    // result. 
    // Furthermore they are used for the reducers.
    // the dispatch on the single argument of the reducer
    // and the result
    template <class F, class URL = mpl::vector<const zassign_args>, class UTL = mpl::vector<>>
    class zdouble_dispatcher
    {
    public:

        template <class T, class R>
        static void insert();

        template <class T, class R, class... U>
        static void register_dispatching(mpl::vector<mpl::vector<T, R>, U...>);

        static void init();

        // this is a bit of a hack st we can use the same double dispatcher impl
        // independent of the actual undispatched type list
        template<class ... A>
        static void dispatch(const zarray_impl& z1, zarray_impl& res, A && ... );

        // this is a bit of a hack st we can use the same double dispatcher impl
        // independent of the actual undispatched type list
        template<class ... A>
        static size_t get_type_index(const zarray_impl& z1, A && ...);

    private:

        using undispatched_run_type_list = URL;
        using undispatched_type_type_list = UTL;
        static zdouble_dispatcher& instance();

        zdouble_dispatcher();
        ~zdouble_dispatcher() = default;

        template <class T, class R>
        void insert_impl();

        template <class T, class R, class...U>
        inline void register_dispatching_impl(mpl::vector<mpl::vector<T, R>, U...>);
        inline void register_dispatching_impl(mpl::vector<>);

        using zfunctor_type = get_zmapped_functor_t<F>;
        using ztype_dispatcher = ztype_dispatcher_impl<
            mpl::vector<const zarray_impl>,
            undispatched_type_type_list>;
        using zrun_dispatcher = zrun_dispatcher_impl<
            mpl::vector<const zarray_impl, zarray_impl>,
            undispatched_run_type_list>;

        ztype_dispatcher m_type_dispatcher;
        zrun_dispatcher m_run_dispatcher;
    };


    /**********************
     * ztriple_dispatcher *
     **********************/

    // Triple dispatchers are used for binary operations.
    // They dispatch on both arguments and on the result.

    template <class F>
    class ztriple_dispatcher
    {
    public:

        template <class T1, class T2, class R>
        static void insert();

        template <class T1, class T2, class R, class... U>
        static void register_dispatching(mpl::vector<mpl::vector<T1, T2, R>, U...>);

        static void init();
        static void dispatch(const zarray_impl& z1,
                             const zarray_impl& z2,
                             zarray_impl& res,
                             const zassign_args& args);
        static size_t get_type_index(const zarray_impl& z1, const zarray_impl& z2);

    private:
        static ztriple_dispatcher& instance();

        ztriple_dispatcher();
        ~ztriple_dispatcher() = default;

        template <class T1, class T2, class R>
        void insert_impl();

        template <class T1, class T2, class R, class...U>
        inline void register_dispatching_impl(mpl::vector<mpl::vector<T1, T2, R>, U...>);
        inline void register_dispatching_impl(mpl::vector<>);

        using zfunctor_type = get_zmapped_functor_t<F>;
        using ztype_dispatcher = ztype_dispatcher_impl<mpl::vector<const zarray_impl, const zarray_impl>>;
        using zrun_dispatcher = zrun_dispatcher_impl<mpl::vector<const zarray_impl, const zarray_impl, zarray_impl>>;

        ztype_dispatcher m_type_dispatcher;
        zrun_dispatcher m_run_dispatcher;
    };

    /***************
     * zdispatcher *
     ***************/

    template <class F, size_t N, size_t M = 0>
    struct zdispatcher;

    template <class F>
    struct zdispatcher<F, 1>
    {
        using type = zdouble_dispatcher<F>;
    };

    template <class F>
    struct zdispatcher<F, 2>
    {
        using type = ztriple_dispatcher<F>;
    };

    template <class F, size_t N, size_t M = 0>
    using zdispatcher_t = typename zdispatcher<F, N, M>::type;


    template<class F>
    using zreducer_dispatcher = zdouble_dispatcher<F,
        mpl::vector<const zassign_args, const zreducer_options>,
        mpl::vector<const zreducer_options>
    >;



    int init_zsystem();

    namespace detail
    {
        template <class F>
        struct unary_dispatching_types
        {
            using type = zunary_func_types;
        };

        template <>
        struct unary_dispatching_types<xassign_dummy_functor>
        {
            using type = zunary_all_ztypes_combinations_types;
        };

        template <>
        struct unary_dispatching_types<xmove_dummy_functor>
        {
            using type = zunary_all_ztypes_combinations_types;
        };

        template <>
        struct unary_dispatching_types<negate>
        {
            using type = zunary_op_types;
        };

        template <>
        struct unary_dispatching_types<identity>
        {
            using type = zunary_op_types;
        };

        template <>
        struct unary_dispatching_types<bitwise_not>
        {
            using type = mpl::transform_t<build_unary_identity_t, z_int_types>;
        };

        // TODO: replace zunary_classify_types with zunary_bool_func_types
        // when xsimd is fixed
        template <>
        struct unary_dispatching_types<xt::math::isfinite_fun>
        {
            using type = zunary_classify_types;
        };

        template <>
        struct unary_dispatching_types<xt::math::isinf_fun>
        {
            using type = zunary_classify_types;
        };

        template <>
        struct unary_dispatching_types<xt::math::isnan_fun>
        {
            using type = zunary_classify_types;
        };


        // reducers
        #define  XTENSOR_DISPATCHING_TYPES(FUNCTOR, TYPES)\
            template <>\
            struct unary_dispatching_types<FUNCTOR>\
            {\
                using type = TYPES;\
            }

        XTENSOR_DISPATCHING_TYPES(zsum_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(zprod_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(zmean_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(zvariance_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(zstddev_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(zamin_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(zamax_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(znorm_l0_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(znorm_l1_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(znorm_l2_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(znorm_sq_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(znorm_linf_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(znorm_lp_to_p_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(znorm_induced_l1_zreducer_functor, zreducer_types);
        XTENSOR_DISPATCHING_TYPES(znorm_induced_linf_zreducer_functor, zreducer_types);

        #undef XTENSOR_DISPATCHING_TYPES

        template <class F>
        using unary_dispatching_types_t = typename unary_dispatching_types<F>::type;
    }

    /*************************************
     * zdouble_dispatcher implementation *
     *************************************/

    template <class F, class URL, class UTL>
    template <class T, class R>
    inline void zdouble_dispatcher<F,URL, UTL>::insert()
    {
        instance().template insert_impl<T, R>();
    }

    template <class F, class URL, class UTL>
    template <class T, class R, class... U>
    inline void zdouble_dispatcher<F,URL, UTL>::register_dispatching(mpl::vector<mpl::vector<T, R>, U...>)
    {
        instance().register_dispatching_impl(mpl::vector<mpl::vector<T, R>, U...>());
    }

    template <class F, class URL, class UTL>
    inline void zdouble_dispatcher<F,URL, UTL>::init()
    {
        instance();
    }

    // the variance template here is a bit of a hack st. we can use 
    // the same dispatcher impl for the unary operations  and the reducers
    template <class F, class URL, class UTL>
    template<class ... A>
    inline void zdouble_dispatcher<F,URL, UTL>::dispatch(const zarray_impl& z1, zarray_impl& res, A && ... args)
    {
        instance().m_run_dispatcher.dispatch(z1, res, std::forward<A>(args) ...);
    }

    // the variance template here is a bit of a hack st. we can use 
    // the same dispatcher impl for the unary operations  and the reducers
    template <class F, class URL, class UTL>
    template<class ... A>
    inline size_t zdouble_dispatcher<F,URL, UTL>::get_type_index(const zarray_impl& z1,A && ... args)
    {
        return instance().m_type_dispatcher.dispatch(z1, std::forward<A>(args) ...);
    }

    template <class F, class URL, class UTL>
    inline zdouble_dispatcher<F,URL, UTL>& zdouble_dispatcher<F,URL, UTL>::instance()
    {
        static zdouble_dispatcher<F,URL, UTL> inst;
        return inst;
    }

    template <class F, class URL, class UTL>
    inline zdouble_dispatcher<F,URL, UTL>::zdouble_dispatcher()
    {
        register_dispatching_impl(detail::unary_dispatching_types_t<F>());
    }

    template <class F, class URL, class UTL>
    template <class T, class R>
    inline void zdouble_dispatcher<F,URL, UTL>::insert_impl()
    {
        using arg_type = const ztyped_array<T>;
        using res_type = ztyped_array<R>;
        m_run_dispatcher.template insert<arg_type, res_type>(&zfunctor_type::template run<T, R>);
        m_type_dispatcher.template insert<arg_type>(&zfunctor_type::template index<T>);
    }

    template <class F, class URL, class UTL>
    template <class T, class R, class...U>
    inline void zdouble_dispatcher<F,URL, UTL>::register_dispatching_impl(mpl::vector<mpl::vector<T, R>, U...>)
    {
        insert_impl<T, R>();
        register_dispatching_impl(mpl::vector<U...>());
    }

    template <class F, class URL, class UTL>
    inline void zdouble_dispatcher<F,URL, UTL>::register_dispatching_impl(mpl::vector<>)
    {
    }

    /*************************************
     * ztriple_dispatcher implementation *
     *************************************/

    namespace detail
    {
        using zbinary_func_list = mpl::vector
        <
            math::atan2_fun,
            math::hypot_fun,
            math::pow_fun,
            math::fdim_fun,
            math::fmax_fun,
            math::fmin_fun,
            math::remainder_fun,
            math::fmod_fun
        >;

        using zbinary_int_op_list = mpl::vector
        <
            detail::modulus,
            detail::bitwise_and,
            detail::bitwise_or,
            detail::bitwise_xor,
            detail::bitwise_not,
            detail::left_shift,
            detail::right_shift
        >;

        template <class F>
        struct binary_dispatching_types
        {
            using type = std::conditional_t<mpl::contains<zbinary_func_list, F>::value,
                                            zbinary_func_types,
                                            std::conditional_t<mpl::contains<zbinary_int_op_list, F>::value,
                                                               zbinary_int_op_types,
                                                               zbinary_op_types>>;
        };

        template <>
        struct binary_dispatching_types<detail::modulus>
        {
            using type = zbinary_int_op_types;
        };

        template <class F>
        using binary_dispatching_types_t = typename binary_dispatching_types<F>::type;
    }

    template <class F>
    template <class T1, class T2, class R>
    inline void ztriple_dispatcher<F>::insert()
    {
        instance().template insert_impl<T1, T2, R>();
    }

    template <class F>
    template <class T1, class T2, class R, class... U>
    inline void ztriple_dispatcher<F>::register_dispatching(mpl::vector<mpl::vector<T1, T2, R>, U...>)
    {
        instance().register_impl(mpl::vector<mpl::vector<T1, T2, R>, U...>());
    }

    template <class F>
    inline void ztriple_dispatcher<F>::init()
    {
        instance();
    }

    template <class F>
    inline void ztriple_dispatcher<F>::dispatch(const zarray_impl& z1,
                                                const zarray_impl& z2,
                                                zarray_impl& res,
                                                const zassign_args& args)
    {
        instance().m_run_dispatcher.dispatch(z1, z2, res, args);
    }

    template <class F>
    inline size_t ztriple_dispatcher<F>::get_type_index(const zarray_impl& z1, const zarray_impl& z2)
    {
        return instance().m_type_dispatcher.dispatch(z1, z2);
    }

    template <class F>
    inline ztriple_dispatcher<F>& ztriple_dispatcher<F>::instance()
    {
        static ztriple_dispatcher<F> inst;
        return inst;
    }

    template <class F>
    inline ztriple_dispatcher<F>::ztriple_dispatcher()
    {
        register_dispatching_impl(detail::binary_dispatching_types_t<F>());
    }

    template <class F>
    template <class T1, class T2, class R>
    inline void ztriple_dispatcher<F>::insert_impl()
    {
        using arg_type1 = const ztyped_array<T1>;
        using arg_type2 = const ztyped_array<T2>;
        using res_type = ztyped_array<R>;
        m_run_dispatcher.template insert<arg_type1, arg_type2, res_type>(&zfunctor_type::template run<T1, T2, R>);
        m_type_dispatcher.template insert<arg_type1, arg_type1>(&zfunctor_type::template index<T1, T2>);
    }


    template <class F>
    template <class T1, class T2, class R, class...U>
    inline void ztriple_dispatcher<F>::register_dispatching_impl(mpl::vector<mpl::vector<T1, T2, R>, U...>)
    {
        insert_impl<T1, T2, R>();
        register_dispatching_impl(mpl::vector<U...>());
    }

    template <class F>
    inline void ztriple_dispatcher<F>::register_dispatching_impl(mpl::vector<>)
    {
    }

}

#endif

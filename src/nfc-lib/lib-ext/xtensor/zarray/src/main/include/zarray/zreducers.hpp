/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZREDUCERS_HPP
#define XTENSOR_ZREDUCERS_HPP

#include "xtensor/xmath.hpp"
#include "xtensor/xnorm.hpp"
#include "xtensor/xtensor.hpp"
#include "xtensor/xarray.hpp"

#include "zassign.hpp"
#include "zreducer.hpp"
#include "zreducer_options.hpp"
#include "zmpl.hpp"

namespace xt
{

    template <class XF>
    struct get_zmapped_functor;

    template <class XF>
    using get_zmapped_functor_t = typename get_zmapped_functor<XF>::type;

#define XTENSOR_ZMAPPED_FUNCTOR(ZFUN, XFUN)                                        \
    template <>                                                                    \
    struct get_zmapped_functor<XFUN>                                               \
    { using type = ZFUN; }


    template <class T>
    inline std::size_t zassign_init_value_functor::index(const ztyped_array<T>&, const zreducer_options& )
    {
        throw std::runtime_error("should be unreachable");
        return 0;
    }

    template<class F>
    struct zreducer_functor
    {
        template <class T, class R>
        static void run (const ztyped_array<T>& in,ztyped_array<R>& zres,
            const zassign_args& assign_args,const zreducer_options& options
        );
        template <class T>
        static size_t index(const ztyped_array<T>& in, const zreducer_options& options );
    };

    template<class F>
    template <class T, class R>
    inline void zreducer_functor<F>::run
    (
        const ztyped_array<T>& input_array,
        ztyped_array<R>& zres,
        const zassign_args& assign_args,
        const zreducer_options& options
    )
    {
        if (!assign_args.chunk_assign)
        {
            options.visit_reducer_options<T>(false /*force_lazy*/,[&assign_args, &input_array, &zres](auto&&... reduce_args)
            {
                auto res_expr = F::run(input_array.get_array(), std::forward<decltype(reduce_args)>(reduce_args)...);
                zassign_wrapped_expression(zres, std::move(res_expr), assign_args);
            });
        }
        else
        {
            options.visit_reducer_options<T>(true /*force_lazy*/, [&assign_args, &input_array, &zres](auto&&... reduce_args)
            {
                auto res_expr = F::run(input_array.get_array(), std::forward<decltype(reduce_args)>(reduce_args)...);
                auto chunk_res = xt::strided_view(std::move(res_expr), assign_args.slices());
                zassign_wrapped_expression(zres, std::move(chunk_res), assign_args);
            });
        }
    }

    namespace detail
    {
        // for gcc4.9
        template<class T, class F>
        struct index_helper
        {
            index_helper(
                const ztyped_array<T>& input_array
            )
            :   m_input_array(input_array)
            {
            }
            template <class ... A>
            void operator()(A && ... reduce_args)
            {
                using expr_type = decltype(F::run(m_input_array.get_array(), std::forward<A>(reduce_args)...));
                using expr_value_type = typename std::decay_t<expr_type>::value_type;
                m_result = ztyped_array<expr_value_type>::get_class_static_index();
            }

            const ztyped_array<T>& m_input_array;
            std::size_t m_result;
        };
    }

    template<class F>
    template <class T>
    inline std::size_t zreducer_functor<F>::index
    (
        const ztyped_array<T>& input_array,
        const zreducer_options& options
    )
    {
        detail::index_helper<T, F> functor(input_array);
        options.visit_reducer_options(functor);
        return functor.m_result;
    }

    #define  XTENSOR_ZREDUCER_FUNCTOR_HELPER(FUNCTOR_NAME, FUNC_NAME)\
    namespace detail\
    {\
        struct FUNCTOR_NAME ## _helper\
        {\
            template<class ... T>\
            auto static run(T && ... args) { return FUNC_NAME(std::forward<T>(args) ...);}\
        };\
    } \
    struct FUNCTOR_NAME : zreducer_functor<detail:: FUNCTOR_NAME ## _helper>{ \
    };\
    XTENSOR_ZMAPPED_FUNCTOR(FUNCTOR_NAME, FUNCTOR_NAME);\
    namespace zt\
    {\
        template<class E, class A, class EVS = DEFAULT_STRATEGY_REDUCERS,\
             XTL_REQUIRES(detail::has_zexpression_tag<E>)\
        >\
        inline auto FUNC_NAME(E && e, std::initializer_list<A> axis, EVS && options = EVS())\
        {\
            zreducer_options zoptions(axis, std::forward<EVS>(options));\
            return make_zreducer<FUNCTOR_NAME>(std::forward<E>(e), zoptions);\
        }\
        template<class E, class A, class EVS = DEFAULT_STRATEGY_REDUCERS,\
             XTL_REQUIRES(xtl::negation<is_reducer_options<A>>, detail::has_zexpression_tag<E>)\
        >\
        inline auto FUNC_NAME(E && e, A && axis, EVS && options = EVS())\
        {\
            zreducer_options zoptions(std::forward<A>(axis), std::forward<EVS>(options));\
            return make_zreducer<FUNCTOR_NAME>(std::forward<E>(e), zoptions);\
        }\
        template<class E, class EVS = DEFAULT_STRATEGY_REDUCERS,\
             XTL_REQUIRES(is_reducer_options<EVS>, detail::has_zexpression_tag<E>)\
        >\
        inline auto FUNC_NAME(E && e, EVS && options = EVS())\
        {\
            auto axis = dynamic_shape<std::size_t>(e.dimension());\
            std::iota(axis.begin(), axis.end(), 0);\
            zreducer_options zoptions(std::move(axis), options);\
            return make_zreducer<FUNCTOR_NAME>(std::forward<E>(e), zoptions);\
        }\
    }

    XTENSOR_ZREDUCER_FUNCTOR_HELPER(zsum_zreducer_functor,               sum)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(zprod_zreducer_functor,              prod)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(zmean_zreducer_functor,              mean)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(zvariance_zreducer_functor,          variance)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(zstddev_zreducer_functor,            stddev)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(zamax_zreducer_functor,              amax)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(zamin_zreducer_functor,              amin)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(znorm_l0_zreducer_functor,           norm_l0)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(znorm_l1_zreducer_functor,           norm_l1)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(znorm_l2_zreducer_functor,           norm_l2)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(znorm_sq_zreducer_functor,           norm_sq)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(znorm_linf_zreducer_functor,         norm_linf)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(znorm_lp_to_p_zreducer_functor,      norm_lp_to_p)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(znorm_induced_l1_zreducer_functor,   norm_induced_l1)
    XTENSOR_ZREDUCER_FUNCTOR_HELPER(znorm_induced_linf_zreducer_functor, norm_induced_linf)

#undef XTENSOR_ZMAPPED_FUNCTOR

}

#endif

/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZINIT_HPP
#define XTENSOR_ZINIT_HPP

#include "xtensor/xmath.hpp"
#include "xtensor/xnorm.hpp"

#include "zdispatcher.hpp"
#include "zdispatching_types.hpp"
#include "zmath.hpp"
#include "zreducers.hpp"


namespace xt
{
    namespace mpl = xtl::mpl;

    /****************
     * init_zsystem *
     ****************/

    // Early initialization of all dispatchers
    // and zarray_impl_register
    // return int so it can be assigned to a
    // static variable and be automatically
    // called when loading a shared library
    // for instance.

    int init_zsystem();

    /******************************
     * init operators dispatchers *
     ******************************/

    template <class T>
    inline int init_zassign_dispatchers()
    {
        zdispatcher_t<detail::xassign_dummy_functor, 1>::init();
        zdispatcher_t<detail::xmove_dummy_functor, 1>::init();
        return 0;
    }

    template <class T>
    inline int init_zarithmetic_dispatchers()
    {
        zdispatcher_t<detail::identity, 1>::init();
        zdispatcher_t<detail::negate, 1>::init();
        zdispatcher_t<detail::plus, 2>::init();
        zdispatcher_t<detail::minus, 2>::init();
        zdispatcher_t<detail::multiplies, 2>::init();
        zdispatcher_t<detail::divides, 2>::init();
        zdispatcher_t<detail::modulus, 2>::init();
        return 0;
    }

    template <class T>
    inline int init_zlogical_dispatchers()
    {
        zdispatcher_t<detail::logical_or, 2>::init();
        zdispatcher_t<detail::logical_and, 2>::init();
        zdispatcher_t<detail::logical_not, 1>::init();
        return 0;
    }

    template <class T>
    inline int init_zbitwise_dispatchers()
    {
        zdispatcher_t<detail::bitwise_or, 2>::init();
        zdispatcher_t<detail::bitwise_and, 2>::init();
        zdispatcher_t<detail::bitwise_xor, 2>::init();
        zdispatcher_t<detail::bitwise_not, 1>::init();
        zdispatcher_t<detail::left_shift, 2>::init();
        zdispatcher_t<detail::right_shift, 2>::init();
        return 0;
    }

    template <class T>
    inline int init_zcomparison_dispatchers()
    {
        zdispatcher_t<detail::less, 2>::init();
        zdispatcher_t<detail::less_equal, 2>::init();
        zdispatcher_t<detail::greater, 2>::init();
        zdispatcher_t<detail::greater_equal, 2>::init();
        zdispatcher_t<detail::equal_to, 2>::init();
        zdispatcher_t<detail::not_equal_to, 2>::init();
        return 0;
    }

    /*************************
     * init math dispatchers *
     *************************/

    template <class T>
    inline int init_zbasic_math_dispatchers()
    {
        zdispatcher_t<math::fabs_fun, 1>::init();
        zdispatcher_t<math::fmod_fun, 2>::init();
        zdispatcher_t<math::remainder_fun, 2>::init();
        zdispatcher_t<math::fmax_fun, 2>::init();
        zdispatcher_t<math::fmin_fun, 2>::init();
        zdispatcher_t<math::fdim_fun, 2>::init();
        return 0;
    }

    template <class T>
    inline int init_zexp_dispatchers()
    {
        zdispatcher_t<math::exp_fun, 1>::init();
        zdispatcher_t<math::exp2_fun, 1>::init();
        zdispatcher_t<math::expm1_fun, 1>::init();
        zdispatcher_t<math::log_fun, 1>::init();
        zdispatcher_t<math::log10_fun, 1>::init();
        zdispatcher_t<math::log2_fun, 1>::init();
        zdispatcher_t<math::log1p_fun, 1>::init();
        return 0;
    }

    template <class T>
    inline int init_zpower_dispatchers()
    {
        zdispatcher_t<math::pow_fun, 2>::init();
        zdispatcher_t<math::sqrt_fun, 1>::init();
        zdispatcher_t<math::cbrt_fun, 1>::init();
        zdispatcher_t<math::hypot_fun, 2>::init();
        return 0;
    }

    template <class T>
    inline int init_ztrigonometric_dispatchers()
    {
        zdispatcher_t<math::sin_fun, 1>::init();
        zdispatcher_t<math::cos_fun, 1>::init();
        zdispatcher_t<math::tan_fun, 1>::init();
        zdispatcher_t<math::asin_fun, 1>::init();
        zdispatcher_t<math::acos_fun, 1>::init();
        zdispatcher_t<math::atan_fun, 1>::init();
        zdispatcher_t<math::atan2_fun, 2>::init();
        return 0;
    }

    template <class T>
    inline int init_zhyperbolic_dispatchers()
    {
        zdispatcher_t<math::sinh_fun, 1>::init();
        zdispatcher_t<math::cosh_fun, 1>::init();
        zdispatcher_t<math::tanh_fun, 1>::init();
        zdispatcher_t<math::asinh_fun, 1>::init();
        zdispatcher_t<math::acosh_fun, 1>::init();
        zdispatcher_t<math::atanh_fun, 1>::init();
        return 0;
    }

    template <class T>
    inline int init_zerf_dispatchers()
    {
        zdispatcher_t<math::erf_fun, 1>::init();
        zdispatcher_t<math::erfc_fun, 1>::init();
        return 0;
    }

    template <class T>
    inline int init_zgamma_dispatchers()
    {
        zdispatcher_t<math::tgamma_fun, 1>::init();
        zdispatcher_t<math::lgamma_fun, 1>::init();
        return 0;
    }

    template <class T>
    inline int init_zrounding_dispatchers()
    {
        zdispatcher_t<math::ceil_fun, 1>::init();
        zdispatcher_t<math::floor_fun, 1>::init();
        zdispatcher_t<math::trunc_fun, 1>::init();
        zdispatcher_t<math::round_fun, 1>::init();
        zdispatcher_t<math::nearbyint_fun, 1>::init();
        zdispatcher_t<math::rint_fun, 1>::init();
        return 0;
    }

    template <class T>
    inline int init_zclassification_dispatchers()
    {
        zdispatcher_t<math::isfinite_fun, 1>::init();
        zdispatcher_t<math::isinf_fun, 1>::init();
        zdispatcher_t<math::isnan_fun, 1>::init();
        return 0;
    }


    template <class T>
    inline int init_zreducer_dispatchers()
    {
        zreducer_dispatcher<zassign_init_value_functor>::init();

        zreducer_dispatcher<zsum_zreducer_functor>::init();
        zreducer_dispatcher<zprod_zreducer_functor>::init();
        zreducer_dispatcher<zmean_zreducer_functor>::init();
        zreducer_dispatcher<zvariance_zreducer_functor>::init();
        zreducer_dispatcher<zstddev_zreducer_functor>::init();
        zreducer_dispatcher<zamin_zreducer_functor>::init();
        zreducer_dispatcher<zamax_zreducer_functor>::init();

        // norms
        zreducer_dispatcher<znorm_l0_zreducer_functor>::init();
        zreducer_dispatcher<znorm_l1_zreducer_functor>::init();
        zreducer_dispatcher<znorm_l2_zreducer_functor>::init();
        zreducer_dispatcher<znorm_sq_zreducer_functor>::init();
        
        // THESE NORMS CANNOT BE INSTANTIATED WITH ALL TYPES?
        zreducer_dispatcher<znorm_linf_zreducer_functor>::init();
        zreducer_dispatcher<znorm_lp_to_p_zreducer_functor>::init();
        zreducer_dispatcher<znorm_induced_l1_zreducer_functor>::init();
        zreducer_dispatcher<znorm_induced_linf_zreducer_functor>::init();

        return 0;
    }

    /************************************
     * global dispatcher initialization *
     ************************************/

    template <class T>
    inline int init_zoperator_dispatchers()
    {
        init_zassign_dispatchers<T>();
        init_zarithmetic_dispatchers<T>();
        init_zlogical_dispatchers<T>();
        init_zbitwise_dispatchers<T>();
        init_zcomparison_dispatchers<T>();
        return 0;
    }

    template <class T>
    inline int init_zmath_dispatchers()
    {
        init_zbasic_math_dispatchers<T>();
        init_zexp_dispatchers<T>();
        init_zpower_dispatchers<T>();
        init_ztrigonometric_dispatchers<T>();
        init_zhyperbolic_dispatchers<T>();
        init_zerf_dispatchers<T>();
        init_zgamma_dispatchers<T>();
        init_zrounding_dispatchers<T>();
        init_zclassification_dispatchers<T>();
        return 0;
    }

    template <class T = void>
    int init_zsystem()
    {
        init_zreducer_dispatchers<T>();
        init_zoperator_dispatchers<T>();
        init_zmath_dispatchers<T>();
        return 0;
    }
}

#endif

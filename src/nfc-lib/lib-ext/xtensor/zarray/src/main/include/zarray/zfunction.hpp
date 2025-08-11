/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZFUNCTION_HPP
#define XTENSOR_ZFUNCTION_HPP

#include <tuple>
#include <utility>

#include "zdispatcher.hpp"
#include "zarray_impl_register.hpp"
#include "zarray_temporary_pool.hpp"
#include "zwrappers.hpp"

namespace xt
{
    template <class F, class... CT>
    class zfunction : public xexpression<zfunction<F, CT...>>
    {
    public:
        using expression_tag = zarray_expression_tag;

        using self_type = zfunction<F, CT...>;
        using tuple_type = std::tuple<CT...>;

        using functor_type = F;
        using shape_type = dynamic_shape<std::size_t>;

        template <class Func, class... CTA, class U = std::enable_if_t<!std::is_base_of<std::decay_t<Func>, self_type>::value>>
        zfunction(Func&& f, CTA&&... e) noexcept;

        std::size_t dimension() const;
        const shape_type& shape() const;
        bool broadcast_shape(shape_type& shape, bool reuse_cache = false) const;

        std::unique_ptr<zarray_impl> allocate_result() const;
        std::size_t get_result_type_index() const;
        zarray_impl& assign_to(zarray_impl& res, const zassign_args& args) const;
        zarray_impl& assign_to(detail::zarray_temporary_pool & res, const zassign_args& args) const;
    private:
        std::size_t get_result_type_index_impl() const;
        using dispatcher_type = zdispatcher_t<F, sizeof...(CT)>;

        std::size_t compute_dimension() const;

        template <std::size_t... I>
        std::size_t get_result_type_index_impl(std::index_sequence<I...>) const;

        template<std::size_t ... I>
        zarray_impl& assign_to_impl(std::index_sequence<I ...>, detail::zarray_temporary_pool & temporary_pool, const zassign_args& args) const;

        struct cache
        {
            cache() : m_shape(), m_initialized(false), m_trivial_broadcast(false) {}

            shape_type m_shape;
            bool m_initialized;
            bool m_trivial_broadcast;
        };

        tuple_type m_e;
        mutable cache m_cache;
        std::size_t m_result_type_index;
    };

    namespace detail
    {
        template <class E>
        struct zargument_type
        {
            using type = E;
        };

        template <class T>
        struct zargument_type<xscalar<T>>
        {
            using type = zscalar_wrapper<xscalar<T>>;
        };

        template <class E>
        using zargument_type_t = typename zargument_type<E>::type;

        template <class F, class... E>
        struct select_xfunction_expression<zarray_expression_tag, F, E...>
        {
            using type = zfunction<F, zargument_type_t<E>...>;
        };
    }

    /****************************
     * zfunction implementation *
     ****************************/

    class zarray;
    
    namespace detail
    {

        // this can be a  zreducer or something similar
        template <class E>
        struct zfunction_argument
        {
            using argument_type = E;

            static std::size_t get_index(const argument_type& e)
            {
                return e.get_result_type_index();
            }

            static const std::tuple<const zarray_impl*, bool>  get_array_impl(const argument_type & e, detail::zarray_temporary_pool & temporary_pool, const zassign_args& args)
            {
                auto buffer_ptr = temporary_pool.get_free_buffer(e.get_result_type_index());
                const auto & array_impl =  e.assign_to(*buffer_ptr, args);
                return std::make_tuple(&array_impl, true);
            }
        };

        template <class F, class... CT>
        struct zfunction_argument<zfunction<F, CT ...>>
        {
            using argument_type = zfunction<F, CT ...>;

            static std::size_t get_index(const argument_type & e)
            {
                return e.get_result_type_index();
            }

            static std::tuple<const zarray_impl*, bool> get_array_impl(const argument_type & e,  detail::zarray_temporary_pool & temporary_pool, const zassign_args& args)
            {
                const auto & array_impl = e.assign_to(temporary_pool, args);
                return std::make_tuple(&array_impl, true);
            }
        };

        template <>
        struct zfunction_argument<zarray>
        {
            using argument_type = zarray;
            template <class E>
            static std::size_t get_index(const E& e)
            {
                return e.get_implementation().get_class_index();
            }

            template <class E>
            static std::tuple<const zarray_impl*, bool> get_array_impl(const E& e,  detail::zarray_temporary_pool & , const zassign_args&)
            {
                auto & impl = e.get_implementation();
                return std::make_tuple(&impl, false);
            }
        };

        template <class CTE>
        struct zfunction_argument<zscalar_wrapper<CTE>>
        {
            using argument_type = zscalar_wrapper<CTE>;

            static std::size_t get_index(const argument_type& e)
            {
                return e.get_class_index();
            }

            static std::tuple<const zarray_impl*, bool> get_array_impl(const argument_type& e,  detail::zarray_temporary_pool &, const zassign_args&)
            {
                const zarray_impl & impl = e;
                return std::make_tuple(&impl, false);
            }
        };

        template <class E>
        inline size_t get_result_type_index(const E& e)
        {
            return zfunction_argument<E>::get_index(e);
        }

        template <class E>
        inline std::tuple<const zarray_impl*, bool> get_array_impl(const E& e, detail::zarray_temporary_pool & temporary_pool, const zassign_args& args)
        {
            return zfunction_argument<E>::get_array_impl(e, temporary_pool, args);
        }
    }

    template <class F, class... CT>
    template <class Func, class... CTA, class U>
    inline zfunction<F, CT...>::zfunction(Func&&, CTA&&... e) noexcept
        : m_e(std::forward<CTA>(e)...),
          m_cache(),
          m_result_type_index()

    {
        m_result_type_index = this->get_result_type_index_impl(std::make_index_sequence<sizeof...(CT)>()); 
    }

    template <class F, class... CT>
    inline std::size_t zfunction<F, CT...>::dimension() const
    {
        return m_cache.m_initialized ? m_cache.m_shape.size() : compute_dimension();
    }

    template <class F, class... CT>
    inline auto zfunction<F, CT...>::shape() const -> const shape_type&
    {
        if (!m_cache.m_initialized)
        {
            m_cache.m_shape = uninitialized_shape<shape_type>(compute_dimension());
            m_cache.m_trivial_broadcast = broadcast_shape(m_cache.m_shape, false);
            m_cache.m_initialized = true;
        }
        return m_cache.m_shape;
    }

    template <class F, class... CT>
    inline bool zfunction<F, CT...>::broadcast_shape(shape_type& shape, bool reuse_cache) const
    {
        if (reuse_cache && m_cache.m_initialized)
        {
            std::copy(m_cache.m_shape.cbegin(), m_cache.m_shape.cend(), shape.begin());
            return m_cache.m_trivial_broadcast;
        }
        else
        {
            auto func = [&shape](bool b, const auto& e) { return e.broadcast_shape(shape) && b; };
            return accumulate(func, true, m_e);
        }
    }
    
    template <class F, class... CT>
    inline std::unique_ptr<zarray_impl> zfunction<F, CT...>::allocate_result() const
    {
        std::size_t idx = get_result_type_index();
        return std::unique_ptr<zarray_impl>(zarray_impl_register::get(idx).clone());
    }

    template <class F, class... CT>
    inline std::size_t zfunction<F, CT...>::get_result_type_index() const
    {
        return m_result_type_index;
    }

    template <class F, class... CT>
    inline zarray_impl& zfunction<F, CT...>::assign_to(zarray_impl& res, const zassign_args& args) const
    {
        detail::zarray_temporary_pool  temporary_pool(res);
        auto & r = this->assign_to(temporary_pool, args);
        if(&r != &res)
        {
            // this has great overlap with zarrays assigment operator
            // and detail::run_chunked_assign_loop
            zassign_args args;
            args.trivial_broadcast = true;
            if (res.is_chunked())
            {
                const zchunked_array& arr = dynamic_cast<const zchunked_array&>(res);
                args.chunk_iter = arr.chunk_begin();
                args.chunk_assign = true;
                auto chunk_end = arr.chunk_end();
                while (args.chunk_iter != chunk_end)
                {
                    zdispatcher_t<detail::xassign_dummy_functor, 1>::dispatch(r, res, args);
                    ++args.chunk_iter;
                }
            }
            else
            {
                zdispatcher_t<detail::xassign_dummy_functor, 1>::dispatch(r, res, args);
            }
        }
        return res;

    }

    template <class F, class... CT>
    inline zarray_impl& zfunction<F, CT...>::assign_to(detail::zarray_temporary_pool & buffer, const zassign_args& args) const
    {
        return assign_to_impl(std::make_index_sequence<sizeof...(CT)>(), buffer, args);
    }


    template <class F, class... CT>
    inline std::size_t zfunction<F, CT...>::compute_dimension() const
    {
        auto func = [](std::size_t d, auto&& e) noexcept { return (std::max)(d, e.dimension()); };
        return accumulate(func, std::size_t(0), m_e);
    }

    template <class F, class... CT>
    template <std::size_t... I>
    std::size_t zfunction<F, CT...>::get_result_type_index_impl(std::index_sequence<I...>) const
    {
        return dispatcher_type::get_type_index(
                zarray_impl_register::get(
                    detail::get_result_type_index(std::get<I>(m_e))
                )...
               );
    }

    template <class F, class... CT>
    template<std::size_t ... I>
    inline zarray_impl& zfunction<F, CT...>::assign_to_impl
    (
        std::index_sequence<I ...>, 
        detail::zarray_temporary_pool & temporary_pool, 
        const zassign_args& args
    ) const
    {

        // inputs
        constexpr std::size_t arity = sizeof...(CT);
        std::array< std::tuple<const zarray_impl *, bool>, arity> inputs = {
            detail::get_array_impl(std::get<I>(m_e), temporary_pool, args) ...
        };

        // the output
        zarray_impl * result_ptr = nullptr;

        // reuse / free  buffers of the arguments
        for(std::size_t i=0; i<arity; ++i)
        {
            const auto is_buffer = std::get<1>(inputs[i]);
            const auto impl_ptr = std::get<0>(inputs[i]);
            // only consider buffers
            if(is_buffer)
            {   
                if(result_ptr==nullptr && m_result_type_index == impl_ptr->get_class_index())
                {
                    // get the same buffer as non-const!
                    result_ptr = const_cast<zarray_impl *>(impl_ptr);
                }
                else 
                {
                    temporary_pool.mark_as_free(impl_ptr);
                }
            }
        }

        // in case we did not find a matching temporary
        if(result_ptr == nullptr)
        {
            result_ptr = temporary_pool.get_free_buffer(m_result_type_index);
        }

        // call the operator dispatcher
        dispatcher_type::dispatch(
            *std::get<0>(std::get<I>(inputs))...,
            *result_ptr, args);

        return *result_ptr;
    }
}

#endif


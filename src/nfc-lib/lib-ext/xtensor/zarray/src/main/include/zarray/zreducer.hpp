#ifndef XTENSOR_ZREDUCER_HPP
#define XTENSOR_ZREDUCER_HPP

#include "xtensor/xexpression.hpp"
#include "xtensor/xreducer.hpp"

#include "zdispatcher.hpp"
#include "zreducer_options.hpp"
#include "zarray_zarray.hpp"
#include "zarray_impl_register.hpp"

namespace xt
{

    template <class XF>
    struct get_zmapped_functor;

    struct zassign_init_value_functor
    {

        template <class T, class R>
        static void run(
            const ztyped_array<T>& z,
            ztyped_array<R>& zres,
            const zassign_args& args,
            const zreducer_options&
        );
        template <class T>
        static std::size_t index(const ztyped_array<T>&, const zreducer_options& );
    };
    template <> 
    struct get_zmapped_functor<zassign_init_value_functor>
    {
        using type = zassign_init_value_functor;
    };

    template <class T, class R>
    inline void zassign_init_value_functor::run
    (
        const ztyped_array<T>& z,
        ztyped_array<R>& zres,
        const zassign_args& args,
        const zreducer_options&
    )
    {
        if (!args.chunk_assign)
        {
            // write the initial value which is wrapped in a zscalar_wrapper 
            // to the first position of the result array
            *(zres.get_array().begin()) = static_cast<R>(*(z.get_array().begin()));
        }
        else
        {
            auto init_value = static_cast<R>(*(z.get_array().begin()));

            auto & chunked_array = dynamic_cast<ztyped_chunked_array<R>&>(zres);
            auto chunk_iter = args.chunk_iter;
            auto shape = chunked_array.chunk_shape();
            auto tmp = xarray<R>::from_shape(shape);
            tmp.fill(init_value);
            chunked_array.assign_chunk(std::move(tmp), chunk_iter);
        }
    }




    template <class F, class CT>
    class zreducer : public xexpression<zreducer<F, CT>>
    {
    public:
        using expression_tag = zarray_expression_tag;
        using self_type = zreducer<F, CT>;
        using shape_type = dynamic_shape<std::size_t>;

        template <class CTA, class O>
        zreducer(CTA&& e, O && options);

        std::size_t dimension() const;
        const auto& shape() const;
        bool broadcast_shape(shape_type& shape, bool /*reuse_cache*/ = false) const;

        std::unique_ptr<zarray_impl> allocate_result() const;
        std::size_t get_result_type_index() const;
        zarray_impl& assign_to(zarray_impl& res, const zassign_args& args) const;

    private:

        using init_value_dispatcher_type =  zreducer_dispatcher<zassign_init_value_functor>;
        using dispatcher_type = zreducer_dispatcher<F>;

        CT m_e;
        zreducer_options m_reducer_options;
        shape_type m_shape;


        void init_result_shape();
    };

    template <class F, class CT>
    template <class CTA, class O>
    zreducer<F,CT>::zreducer(CTA&& e, O && options)
    :   m_e(std::forward<CTA>(e)),
        m_reducer_options(options)
    {
        this->init_result_shape();
    }

    template <class F, class CT>
    std::size_t zreducer<F,CT>::dimension() const
    {
        return m_shape.size();
    }

    template <class F, class CT>
    const auto & zreducer<F,CT>::shape() const
    {
        return m_shape;
    }

    template <class F, class CT>
    bool zreducer<F,CT>::broadcast_shape(shape_type& shape, bool /*reuse_cache*/) const
    {
        return xt::broadcast_shape(m_shape, shape);
    }

    template <class F, class CT>
    std::unique_ptr<zarray_impl> zreducer<F,CT>::allocate_result() const
    {
        std::size_t idx = get_result_type_index();
        return std::unique_ptr<zarray_impl>(zarray_impl_register::get(idx).clone());
    }

    template <class F, class CT>
    std::size_t zreducer<F,CT>::get_result_type_index() const
    {
        // we know that me is always a zarray
        return dispatcher_type::get_type_index(m_e.get_implementation(), m_reducer_options);
    }

    template <class F, class CT>
    zarray_impl& zreducer<F,CT>::assign_to(zarray_impl& res, const zassign_args& args) const
    {

        if(m_reducer_options.has_initial_value())
        {
            // write the inital value
            const zarray_impl &  init_value = m_reducer_options.m_initial_value.get_implementation();
            init_value_dispatcher_type::dispatch(init_value, res, args, m_reducer_options);
        }

        // do the actual dispatching (ie this will at some point call the actual xreducer)
        dispatcher_type::dispatch(m_e.get_implementation(), res, args, m_reducer_options);
        return res;
    }


    // this has a great overlap with xt::detail::shape_computation in xreducer
    template <class F, class CT>
    void zreducer<F,CT>::init_result_shape()
    {

        // dimension
        const auto dim_in = m_e.dimension();

        // output dimension
        std::size_t dim_out;
        if(m_reducer_options.keep_dims())
        {
            dim_out = dim_in;
        }
        else
        {

            dim_out = dim_in - m_reducer_options.axes().size();
        }

        // the axes over which we accumulate
        auto && axes =  m_reducer_options.axes();

        // the own shape
        m_shape.resize(dim_out);
        if(m_reducer_options.keep_dims())
        {

            std::copy(m_e.shape().cbegin(), m_e.shape().cend(), m_shape.begin());
            for(auto a : m_reducer_options.axes())
            {
                m_shape[a] = 1;
            }

        }
        else
        {
            for (std::size_t i = 0, idx = 0; i < dim_in; ++i)
            {
                if (std::find(axes.begin(), axes.end(), i) == axes.end())
                {
                    // i not in axes!
                    m_shape[idx] = m_e.shape()[i];
                    ++idx;
                }
            }
        }
    }

    // factory function where the argument is a zarray
    template<class F, class E, typename detail::enable_zarray_t<std::decay_t<E>> * = nullptr>
    auto make_zreducer(E && e, const zreducer_options & options)
    {
        using closure_type = xtl::const_closure_type_t<E>;
        using reducer_type = zreducer<F, closure_type>;
        return reducer_type(std::forward<E>(e), options);
    }

    // factory function where the argument is *NOT* an zarray
    // but has a zexpression tag. ie a:
    // - zfunction
    // - zreducer
    // - ...
    // We need to evaluate these expressions into an zarray
    // before we pass them to the reducer.
    template<class F, class E, typename detail::disable_zarray_enable_zexpressions_t<std::decay_t<E>> * = nullptr>
    auto make_zreducer(E && e, const zreducer_options & options)
    {
        zarray a(std::forward<E>(e));
        using closure_type = xtl::const_closure_type_t<zarray>;
        using reducer_type = zreducer<F, closure_type>;
        return reducer_type(std::move(a), options);
    }
}
#endif
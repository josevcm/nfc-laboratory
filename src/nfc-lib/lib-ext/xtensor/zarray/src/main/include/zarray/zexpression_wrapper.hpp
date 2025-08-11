/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZEXPRESSION_WRAPPER_HPP
#define XTENSOR_ZEXPRESSION_WRAPPER_HPP

#include "zarray_impl.hpp"

namespace xt
{
    /***********************
     * zexpression_wrapper *
     ***********************/

    template <class T>
    class ztyped_expression_wrapper : public ztyped_array<T>
    {
    public:

        virtual ~ztyped_expression_wrapper() = default;

        virtual void assign(xarray<T>&& rhs) = 0;
    };

    template <class CTE>
    class zexpression_wrapper : public ztyped_expression_wrapper<typename std::decay_t<CTE>::value_type>
    {
    public:

        using self_type = zexpression_wrapper<CTE>;
        using value_type = typename std::decay_t<CTE>::value_type;
        using base_type = ztyped_expression_wrapper<value_type>;
        using shape_type = typename base_type::shape_type;
        using slice_vector = typename base_type::slice_vector;

        template <class E>
        zexpression_wrapper(E&& e);

        virtual ~zexpression_wrapper() = default;

        bool is_array() const override;
        bool is_chunked() const override;

        xarray<value_type>& get_array() override;
        const xarray<value_type>& get_array() const override;
        xarray<value_type> get_chunk(const slice_vector& slices) const override;

        void assign(xarray<value_type>&& rhs) override;

        self_type* clone() const override;
        std::ostream& print(std::ostream& out) const override;

        zarray_impl* strided_view(slice_vector& slices) override;

        const nlohmann::json& get_metadata() const override;
        void set_metadata(const nlohmann::json& metadata) override;
        std::size_t dimension() const override;
        const shape_type& shape() const override;
        void reshape(const shape_type&) override;
        void reshape(shape_type&&) override;
        void resize(const shape_type&) override;
        void resize(shape_type&&) override;
        bool broadcast_shape(shape_type& shape, bool reuse_cache = 0) const override;

    private:

        zexpression_wrapper(const zexpression_wrapper&) = default;

        void compute_cache() const;

        zarray_impl* strided_view_impl(xstrided_slice_vector& slices, std::true_type);
        zarray_impl* strided_view_impl(xstrided_slice_vector& slices, std::false_type);

        template <class CT>
        using enable_assignable_t = enable_assignable_expression<CT, xarray<value_type>>;

        template <class CT>
        using enable_not_assignable_t = enable_not_assignable_expression<CT, xarray<value_type>>;

        template <class CT = CTE>
        enable_assignable_t<CT> assign_impl(xarray<value_type>&& rhs);

        template <class CT = CTE>
        enable_not_assignable_t<CT> assign_impl(xarray<value_type>&& rhs);

        template <class CT = CTE>
        enable_assignable_t<CT> resize_impl();

        template <class CT = CTE>
        enable_not_assignable_t<CT> resize_impl();

        CTE m_expression;
        mutable xarray<value_type> m_cache;
        mutable bool m_cache_initialized;
        shape_type m_shape;
        nlohmann::json m_metadata;
    };

    /**************************************
     * zexpression_wrapper implementation *
     **************************************/

    template <class CTE>
    template <class E>
    inline zexpression_wrapper<CTE>::zexpression_wrapper(E&& e)
        : base_type()
        , m_expression(std::forward<E>(e))
        , m_cache()
        , m_cache_initialized(false)
        , m_shape(m_expression.dimension())
    {
        detail::set_data_type<value_type>(m_metadata);
        std::copy(m_expression.shape().begin(), m_expression.shape().end(), m_shape.begin());
    }

    template <class CTE>
    bool zexpression_wrapper<CTE>::is_array() const
    {
        return false;
    }

    template <class CTE>
    bool zexpression_wrapper<CTE>::is_chunked() const
    {
        return false;
    }

    template <class CTE>
    auto zexpression_wrapper<CTE>::get_array() -> xarray<value_type>&
    {
        compute_cache();
        return m_cache;
    }

    template <class CTE>
    auto zexpression_wrapper<CTE>::get_array() const -> const xarray<value_type>&
    {
        compute_cache();
        return m_cache;
    }

    template <class CTE>
    auto zexpression_wrapper<CTE>::get_chunk(const slice_vector& slices) const -> xarray<value_type>
    {
        return xt::strided_view(m_expression, slices);
    }

    template <class CTE>
    void zexpression_wrapper<CTE>::assign(xarray<value_type>&& rhs)
    {
        assign_impl(std::move(rhs));
    }

    template <class CTE>
    auto zexpression_wrapper<CTE>::clone() const -> self_type*
    {
        return new self_type(*this);
    }

    template <class CTE>
    std::ostream& zexpression_wrapper<CTE>::print(std::ostream& out) const
    {
        return out << m_expression;
    }

    template <class CTE>
    zarray_impl* zexpression_wrapper<CTE>::strided_view(slice_vector& slices)
    {
        return strided_view_impl(slices, detail::is_xstrided_view<CTE>());
    }

    template <class CTE>
    inline zarray_impl* zexpression_wrapper<CTE>::strided_view_impl(xstrided_slice_vector& slices, std::true_type)
    {
        auto e = xt::strided_view(get_array(), slices);
        return detail::build_zarray(std::move(e));
    }

    template <class CTE>
    inline zarray_impl* zexpression_wrapper<CTE>::strided_view_impl(xstrided_slice_vector& slices, std::false_type)
    {
        auto e = xt::strided_view(m_expression, slices);
        return detail::build_zarray(std::move(e));
    }

    template <class CTE>
    auto zexpression_wrapper<CTE>::get_metadata() const -> const nlohmann::json&
    {
        return m_metadata;
    }

    template <class CTE>
    void zexpression_wrapper<CTE>::set_metadata(const nlohmann::json& metadata)
    {
        m_metadata = metadata;
    }

    template <class CTE>
    std::size_t zexpression_wrapper<CTE>::dimension() const
    {
        return m_expression.dimension();
    }

    template <class CTE>
    auto zexpression_wrapper<CTE>::shape() const -> const shape_type&
    {
        return m_shape;
    }

    template <class CTE>
    void zexpression_wrapper<CTE>::reshape(const shape_type&)
    {
        // No op
    }

    template <class CTE>
    void zexpression_wrapper<CTE>::reshape(shape_type&&)
    {
        // No op
    }

    template <class CTE>
    void zexpression_wrapper<CTE>::resize(const shape_type&)
    {
        resize_impl();
    }

    template <class CTE>
    void zexpression_wrapper<CTE>::resize(shape_type&&)
    {
        resize_impl();
    }

    template <class CTE>
    bool zexpression_wrapper<CTE>::broadcast_shape(shape_type& shape, bool reuse_cache) const
    {
        return m_expression.broadcast_shape(shape, reuse_cache);
    }

    template <class CTE>
    inline void zexpression_wrapper<CTE>::compute_cache() const
    {
        if (!m_cache_initialized)
        {
            noalias(m_cache) = m_expression;
            m_cache_initialized = true;
        }
    }

    template <class CTE>
    template <class CT>
    inline auto zexpression_wrapper<CTE>::assign_impl(xarray<value_type>&& rhs) -> enable_assignable_t<CT>
    {
        // aliasing is handled before this method, therefore we are sure that
        // m_expression is not involved in rhs.
        xt::noalias(m_expression) = rhs;
    }

    template <class CTE>
    template <class CT>
    inline auto zexpression_wrapper<CTE>::assign_impl(xarray<value_type>&&) -> enable_not_assignable_t<CT>
    {
        throw std::runtime_error("unevaluated expression is not assignable");
    }

    template <class CTE>
    template <class CT>
    inline auto zexpression_wrapper<CTE>::resize_impl() -> enable_assignable_t<CT>
    {
        // Only wrappers on views are assignable. Resizing is a no op.
    }

    template <class CTE>
    template <class CT>
    inline auto zexpression_wrapper<CTE>::resize_impl() -> enable_not_assignable_t<CT>
    {
        throw std::runtime_error("cannot resize not assignable expression wrapper");
    }
}

#endif

/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZCHUNKED_WRAPPER_HPP
#define XTENSOR_ZCHUNKED_WRAPPER_HPP

#include "zarray_impl.hpp"
#include "zchunked_iterator.hpp"

namespace xt
{
    /********************
     * zchunked_wrapper *
     ********************/

    class zchunked_array
    {
    public:

        using shape_type = xt::dynamic_shape<std::size_t>;

        virtual ~zchunked_array() = default;
        virtual const shape_type& chunk_shape() const = 0;
        virtual size_t grid_size() const = 0;

        virtual zchunked_iterator chunk_begin() const = 0;
        virtual zchunked_iterator chunk_end() const = 0;
    };

    template <class T>
    class ztyped_chunked_array : public ztyped_array<T>,
                                 public zchunked_array
    {
    public:

        using value_type = T;

        virtual ~ztyped_chunked_array() = default;

        virtual void assign_chunk(xarray<value_type>&& rhs, const zchunked_iterator& chunk_it) = 0;
    };

    template <class CTE>
    class zchunked_wrapper : public ztyped_chunked_array<typename std::decay_t<CTE>::value_type>
    {
    public:

        using self_type = zchunked_wrapper;
        using base_type = ztyped_chunked_array<typename std::decay_t<CTE>::value_type>;
        using value_type = typename base_type::value_type;
        using shape_type = zchunked_array::shape_type;
        using slice_vector = typename base_type::slice_vector;

        template <class E>
        zchunked_wrapper(E&& e);

        virtual ~zchunked_wrapper() = default;

        bool is_array() const override;
        bool is_chunked() const override;

        xarray<value_type>& get_array() override;
        const xarray<value_type>& get_array() const override;
        xarray<value_type> get_chunk(const slice_vector& slices) const override;

        self_type* clone() const override;
        std::ostream& print(std::ostream& out) const override;

        zarray_impl* strided_view(slice_vector& slices) override;

        const nlohmann::json& get_metadata() const override;
        void set_metadata(const nlohmann::json& metadata) override;
        std::size_t dimension() const override;
        const shape_type& shape() const override;
        void reshape(const shape_type& shape) override;
        void reshape(shape_type&&) override;
        void resize(const shape_type& shape) override;
        void resize(shape_type&&) override;
        bool broadcast_shape(shape_type& shape, bool reuse_cache = 0) const override;

        const shape_type& chunk_shape() const override;
        size_t grid_size() const override;

        zchunked_iterator chunk_begin() const override;
        zchunked_iterator chunk_end() const override;

        void assign_chunk(xarray<value_type>&& rhs, const zchunked_iterator& chunk_it) override;

    private:

        zchunked_wrapper(const zchunked_wrapper&) = default;

        void compute_cache() const;

        template <class CT = CTE>
        detail::enable_const_t<CT> assign_chunk_impl(xarray<value_type>&& rhs,
                                                     const zchunked_iterator& chunk_it);

        template <class CT = CTE>
        detail::disable_const_t<CT> assign_chunk_impl(xarray<value_type>&& rhs,
                                                      const zchunked_iterator& chunk_it);

        CTE m_chunked_array;
        shape_type m_chunk_shape;
        mutable xarray<value_type> m_cache;
        mutable bool m_cache_initialized;
        mutable dynamic_shape<std::ptrdiff_t> m_strides;
        mutable bool m_strides_initialized;

        nlohmann::json m_metadata;
    };

    /***********************************
     * zchunked_wrapper implementation *
     ***********************************/

    template <class CTE>
    template <class E>
    inline zchunked_wrapper<CTE>::zchunked_wrapper(E&& e)
        : base_type()
        , m_chunked_array(std::forward<E>(e))
        , m_chunk_shape(m_chunked_array.chunk_shape().size())
        , m_cache()
        , m_cache_initialized(false)
        , m_strides_initialized(false)
    {
        std::copy(m_chunked_array.chunk_shape().begin(),
                  m_chunked_array.chunk_shape().end(),
                  m_chunk_shape.begin());
        detail::set_data_type<value_type>(m_metadata);
    }

    template <class CTE>
    bool zchunked_wrapper<CTE>::is_array() const
    {
        return false;
    }

    template <class CTE>
    bool zchunked_wrapper<CTE>::is_chunked() const
    {
        return true;
    }

    template <class CTE>
    auto zchunked_wrapper<CTE>::get_array() -> xarray<value_type>&
    {
        compute_cache();
        return m_cache;
    }

    template <class CTE>
    auto zchunked_wrapper<CTE>::get_array() const -> const xarray<value_type>&
    {
        compute_cache();
        return m_cache;
    }

    template <class CTE>
    auto zchunked_wrapper<CTE>::get_chunk(const slice_vector& slices) const -> xarray<value_type>
    {
        compute_cache();
        return xt::strided_view(m_cache, slices);
    }

    template <class CTE>
    auto zchunked_wrapper<CTE>::clone() const -> self_type*
    {
        return new self_type(*this);
    }

    template <class CTE>
    std::ostream& zchunked_wrapper<CTE>::print(std::ostream& out) const
    {
        return out << m_chunked_array;
    }

    template <class CTE>
    zarray_impl* zchunked_wrapper<CTE>::strided_view(slice_vector& slices)
    {
        auto e = xt::strided_view(m_chunked_array, slices);
        return detail::build_zarray(std::move(e));
    }

    template <class CTE>
    auto zchunked_wrapper<CTE>::get_metadata() const -> const nlohmann::json&
    {
        return m_metadata;
    }

    template <class CTE>
    void zchunked_wrapper<CTE>::set_metadata(const nlohmann::json& metadata)
    {
        m_metadata = metadata;
    }

    template <class CTE>
    std::size_t zchunked_wrapper<CTE>::dimension() const
    {
        return m_chunked_array.dimension();
    }

    template <class CTE>
    auto zchunked_wrapper<CTE>::shape() const -> const shape_type&
    {
        return m_chunked_array.shape();
    }

    template <class CTE>
    void zchunked_wrapper<CTE>::reshape(const shape_type&)
    {
        // No op
    }

    template <class CTE>
    void zchunked_wrapper<CTE>::reshape(shape_type&&)
    {
        // No op
    }

    template <class CTE>
    void zchunked_wrapper<CTE>::resize(const shape_type&)
    {
        // This function is called because zarray (and therefore
        // all the wrappers) implements the container semantic,
        // whatever it wraps (and it cannot be different, since
        // the type of the wrappee is erased).
        // We can see this as a mapping container_semantic => chunked_semantic,
        // meaning this function call must be authorized, and must not do
        // anything.
    }

    template <class CTE>
    void zchunked_wrapper<CTE>::resize(shape_type&&)
    {
        // See comment above.
    }

    template <class CTE>
    bool zchunked_wrapper<CTE>::broadcast_shape(shape_type& shape, bool reuse_cache) const
    {
        return m_chunked_array.broadcast_shape(shape, reuse_cache);
    }

    template <class CTE>
    size_t zchunked_wrapper<CTE>::grid_size() const
    {
        return m_chunked_array.grid_size();
    }

    template <class CTE>
    auto zchunked_wrapper<CTE>::chunk_shape() const -> const shape_type&
    {
        return m_chunk_shape;
    }

    template <class CTE>
    zchunked_iterator zchunked_wrapper<CTE>::chunk_begin() const
    {
        return zchunked_iterator(m_chunked_array.chunk_begin());
    }

    template <class CTE>
    zchunked_iterator zchunked_wrapper<CTE>::chunk_end() const
    {
        return zchunked_iterator(m_chunked_array.chunk_end());
    }

    template <class CTE>
    void zchunked_wrapper<CTE>::assign_chunk(xarray<value_type>&& rhs, const zchunked_iterator& chunk_it)
    {
        assign_chunk_impl(std::move(rhs), chunk_it);
    }

    template <class CTE>
    inline void zchunked_wrapper<CTE>::compute_cache() const
    {
        if (!m_cache_initialized)
        {
            m_cache.resize(m_chunked_array.shape());
            as_chunked(m_cache,  m_chunked_array.chunk_shape()) = m_chunked_array;
            m_cache_initialized = true;
        }
    }

    template <class CTE>
    template <class CT>
    inline detail::enable_const_t<CT>
    zchunked_wrapper<CTE>::assign_chunk_impl(xarray<value_type>&&, const zchunked_iterator&)
    {
        throw std::runtime_error("const array is not assignable");
    }

    template <class CTE>
    template <class CT>
    inline detail::disable_const_t<CT>
    zchunked_wrapper<CTE>::assign_chunk_impl(xarray<value_type>&& rhs, const zchunked_iterator& chunk_it)
    {
        const auto& it = chunk_it.get_xchunked_iterator<decltype(m_chunked_array.chunk_begin())>();
        const auto& chunk_shape = m_chunked_array.chunk_shape();
        if (rhs.shape() != chunk_shape)
        {
            xt::noalias(xt::strided_view(*it, it.get_chunk_slice_vector())) = rhs;
        }
        else
        {
            xt::noalias(*it) = rhs;
        }
    }
}

#endif


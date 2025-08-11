/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARRAY_WRAPPER_HPP
#define XTENSOR_ZARRAY_WRAPPER_HPP

#include "zarray_impl.hpp"

namespace xt
{
    /******************
     * zarray_wrapper *
     ******************/

    template <class CTE>
    class zarray_wrapper : public ztyped_array<typename std::decay_t<CTE>::value_type>
    {
    public:

        using self_type = zarray_wrapper;
        using value_type = typename std::decay_t<CTE>::value_type;
        using base_type = ztyped_array<value_type>;
        using shape_type = typename base_type::shape_type;
        using slice_vector = typename base_type::slice_vector;

        template <class E>
        zarray_wrapper(E&& e);

        virtual ~zarray_wrapper() = default;

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
        void reshape(const shape_type&) override;
        void reshape(shape_type&&) override;
        void resize(const shape_type&) override;
        void resize(shape_type&&) override;
        bool broadcast_shape(shape_type& shape, bool reuse_cache = 0) const override;

    private:

        zarray_wrapper(const zarray_wrapper&) = default;

        CTE m_array;
        nlohmann::json m_metadata;
    };

    /*********************************
     * zarray_wrapper implementation *
     *********************************/

    namespace detail
    {
        template <class T>
        struct zarray_wrapper_helper
        {
            static inline xarray<T>& get_array(xarray<T>& ar)
            {
                return ar;
            }

            static inline xarray<T>& get_array(const xarray<T>&)
            {
                throw std::runtime_error("Cannot return non const array from const array");
            }

            template <class S>
            static inline void reshape(xarray<T>& ar, S&& shape)
            {
                ar.reshape(std::forward<S>(shape));
            }

            template <class S>
            static inline void reshape(const xarray<T>&, S&&)
            {
                throw std::runtime_error("Cannot reshape const array");
            }

            template <class S>
            static inline void resize(xarray<T>& ar, S&& shape)
            {
                ar.resize(std::forward<S>(shape));
            }

            template <class S>
            static inline void resize(const xarray<T>&, S&&)
            {
                throw std::runtime_error("Cannot resize const array");
            }
        };
    }

    template <class CTE>
    template <class E>
    inline zarray_wrapper<CTE>::zarray_wrapper(E&& e)
        : base_type()
        , m_array(std::forward<E>(e))
    {
        detail::set_data_type<value_type>(m_metadata);
    }

    template <class CTE>
    bool zarray_wrapper<CTE>::is_array() const
    {
        return true;
    }

    template <class CTE>
    bool zarray_wrapper<CTE>::is_chunked() const
    {
        return false;
    }

    template <class CTE>
    auto zarray_wrapper<CTE>::get_array() -> xarray<value_type>&
    {
        return detail::zarray_wrapper_helper<value_type>::get_array(m_array);
    }

    template <class CTE>
    auto zarray_wrapper<CTE>::get_array() const -> const xarray<value_type>&
    {
        return m_array;
    }

    template <class CTE>
    auto zarray_wrapper<CTE>::get_chunk(const slice_vector& slices) const -> xarray<value_type>
    {
        return xt::strided_view(m_array, slices);
    }

    template <class CTE>
    auto zarray_wrapper<CTE>::clone() const -> self_type*
    {
        return new self_type(*this);
    }

    template <class CTE>
    std::ostream& zarray_wrapper<CTE>::print(std::ostream& out) const
    {
        return out << m_array;
    }

    template <class CTE>
    zarray_impl* zarray_wrapper<CTE>::strided_view(slice_vector& slices)
    {
        auto e = xt::strided_view(m_array, slices);
        return detail::build_zarray(std::move(e));
    }

    template <class CTE>
    auto zarray_wrapper<CTE>::get_metadata() const -> const nlohmann::json&
    {
        return m_metadata;
    }

    template <class CTE>
    void zarray_wrapper<CTE>::set_metadata(const nlohmann::json& metadata)
    {
        m_metadata = metadata;
    }

    template <class CTE>
    std::size_t zarray_wrapper<CTE>::dimension() const
    {
        return m_array.dimension();
    }

    template <class CTE>
    auto zarray_wrapper<CTE>::shape() const -> const shape_type&
    {
        return m_array.shape();
    }

    template <class CTE>
    void zarray_wrapper<CTE>::reshape(const shape_type& shape)
    {
        detail::zarray_wrapper_helper<value_type>::reshape(m_array, shape);
    }

    template <class CTE>
    void zarray_wrapper<CTE>::reshape(shape_type&& shape)
    {
        detail::zarray_wrapper_helper<value_type>::reshape(m_array, std::move(shape));
    }

    template <class CTE>
    void zarray_wrapper<CTE>::resize(const shape_type& shape)
    {
        detail::zarray_wrapper_helper<value_type>::resize(m_array, shape);
    }

    template <class CTE>
    void zarray_wrapper<CTE>::resize(shape_type&& shape)
    {
        detail::zarray_wrapper_helper<value_type>::resize(m_array, std::move(shape));
    }

    template <class CTE>
    bool zarray_wrapper<CTE>::broadcast_shape(shape_type& shape, bool reuse_cache) const
    {
        return m_array.broadcast_shape(shape, reuse_cache);
    }
}

#endif


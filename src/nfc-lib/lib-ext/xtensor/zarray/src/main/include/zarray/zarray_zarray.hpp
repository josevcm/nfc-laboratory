/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARRAY_ZARRAY_HPP
#define XTENSOR_ZARRAY_ZARRAY_HPP

#include <memory>

#include <nlohmann/json.hpp>
#include <xtl/xmultimethods.hpp>
#include <xtensor/xarray.hpp>

#include "zassign.hpp"
#include "zfunctors.hpp"
#include "zwrappers.hpp"


namespace xt
{

    /**********
     * zarray *
     **********/

    class zarray;

    template <>
    struct xcontainer_inner_types<zarray>
    {
        using temporary_type = zarray;
    };

    class zarray : public xcontainer_semantic<zarray>
    {
    public:

        using expression_tag = zarray_expression_tag;
        using semantic_base = xcontainer_semantic<zarray>;
        using implementation_ptr = std::unique_ptr<zarray_impl>;
        using shape_type = zarray_impl::shape_type;

        zarray() = default;
        ~zarray() = default;

        template <class T>
        zarray(std::initializer_list<T> t);
        template <class T>
        zarray(std::initializer_list<std::initializer_list<T>> t);
        template <class T>
        zarray(std::initializer_list<
                   std::initializer_list<
                       std::initializer_list<T>
                   >
               > t);

        zarray(implementation_ptr&& impl);
        zarray& operator=(implementation_ptr&& impl);

        zarray(const zarray& rhs);
        zarray& operator=(const zarray& rhs);

        zarray(zarray&& rhs);
        zarray& operator=(zarray&& rhs);

        template <class E>
        zarray(const xexpression<E>& e);

        template <class E>
        zarray(xexpression<E>& e);

        template <class E>
        zarray(xexpression<E>&& e);

        template <class E>
        zarray& operator=(const xexpression<E>&);

        void swap(zarray& rhs);

        bool has_implementation() const;

        zarray_impl& get_implementation();
        const zarray_impl& get_implementation() const;

        template <class T>
        bool can_get_array()const;

        template <class T>
        xarray<T>& get_array();

        template <class T>
        const xarray<T>& get_array() const;

        void assign_to(zarray_impl& dst, const zassign_args& args) const;

        std::size_t dimension() const;
        const shape_type& shape() const;
        void reshape(const shape_type& shape);
        void reshape(shape_type&& shape);
        void resize(const shape_type& shape);
        void resize(shape_type&& shape);
        bool broadcast_shape(shape_type& shape, bool reuse_cache = false) const;

        const zchunked_array& as_chunked_array() const;

        const nlohmann::json& get_metadata() const;
        void set_metadata(const nlohmann::json& metadata);

    private:

        template <class E>
        void init_implementation(E&& e, xtensor_expression_tag);

        template <class E>
        void init_implementation(const xexpression<E>& e, zarray_expression_tag);

        template <class E>
        zarray& assign_expression(const xexpression<E>& e, xtensor_expression_tag);

        template <class E>
        zarray& assign_expression(const xexpression<E>& e, zarray_expression_tag);

        implementation_ptr p_impl;
    };

    zarray strided_view(zarray& z, xstrided_slice_vector& slices);
    std::ostream& operator<<(std::ostream& out, const zarray& ar);

    /*************************
     * zarray implementation *
     *************************/

    template <class E>
    inline void zarray::init_implementation(E&& e, xtensor_expression_tag)
    {
        p_impl = implementation_ptr(detail::build_zarray(std::forward<E>(e)));
    }

    template <class E>
    inline void zarray::init_implementation(const xexpression<E>& e, zarray_expression_tag)
    {
        p_impl = e.derived_cast().allocate_result();
        semantic_base::assign(e);
    }

    template <class E>
    inline zarray& zarray::assign_expression(const xexpression<E>& e, xtensor_expression_tag)
    {
        zarray tmp(e);
        return (*this = tmp);
    }

    template <class E>
    inline zarray& zarray::assign_expression(const xexpression<E>& e, zarray_expression_tag)
    {
        return semantic_base::operator=(e);
    }

    template <class T>
    inline zarray::zarray(std::initializer_list<T> t)
        : zarray(xarray<T>(t))
    {
    }

    template <class T>
    inline zarray::zarray(std::initializer_list<std::initializer_list<T>> t)
        : zarray(xarray<T>(t))
    {
    }

    template <class T>
    inline zarray::zarray(std::initializer_list<
                              std::initializer_list<
                                  std::initializer_list<T>
                              >
                          > t)
        : zarray(xarray<T>(t))
    {
    }

    inline zarray::zarray(implementation_ptr&& impl)
        : p_impl(std::move(impl))
    {
    }

    inline zarray& zarray::operator=(implementation_ptr&& impl)
    {
        p_impl = std::move(impl);
        return *this;
    }

    inline zarray::zarray(const zarray& rhs)
        : p_impl()
    {
        if(rhs.has_implementation())
        {
            p_impl.reset(rhs.p_impl->clone());
        }
}

    inline zarray& zarray::operator=(const zarray& rhs)
    {
        if(this->has_implementation())
        {
            resize(rhs.shape());
            zassign_args args;
            args.trivial_broadcast = true;
            if (p_impl->is_chunked())
            {
                auto l = [](zarray& lhs, const zarray& rhs, zassign_args& args)
                {
                    zdispatcher_t<detail::xassign_dummy_functor, 1>::dispatch(*(rhs.p_impl), *(lhs.p_impl), args);
                };
                detail::run_chunked_assign_loop(*this, rhs, args, l);
            }
            else
            {
                zdispatcher_t<detail::xassign_dummy_functor, 1>::dispatch(*(rhs.p_impl), *p_impl, args);
            }
        }
        else
        {
            p_impl.reset(rhs.p_impl->clone());
        }
        return *this;
    }

    inline zarray::zarray(zarray&& rhs)
        : p_impl(std::move(rhs.p_impl))
    {
    }

    inline zarray& zarray::operator=(zarray&& rhs)
    {
        if(this->has_implementation())
        {
            zassign_args args;
            args.trivial_broadcast = true;
            if (p_impl->is_chunked())
            {
                auto l = [](zarray& lhs, const zarray& rhs, zassign_args& args)
                {
                    zdispatcher_t<detail::xmove_dummy_functor, 1>::dispatch(*(rhs.p_impl), *(lhs.p_impl), args);
                };
                detail::run_chunked_assign_loop(*this, rhs, args, l);
            }
            else
            {
                zdispatcher_t<detail::xmove_dummy_functor, 1>::dispatch(*(rhs.p_impl), *p_impl, args);
            }
        }
        else
        {
            p_impl = std::move(rhs.p_impl);
        }
        return *this;
    }

    template <class E>
    inline zarray::zarray(const xexpression<E>& e)
    {
        init_implementation(e.derived_cast(), extension::get_expression_tag_t<std::decay_t<E>>());
    }

    template <class E>
    inline zarray::zarray(xexpression<E>& e)
    {
        init_implementation(e.derived_cast(), extension::get_expression_tag_t<std::decay_t<E>>());
    }

    template <class E>
    inline zarray::zarray(xexpression<E>&& e)
    {
        init_implementation(std::move(e).derived_cast(), extension::get_expression_tag_t<std::decay_t<E>>());
    }

    template <class E>
    inline zarray& zarray::operator=(const xexpression<E>& e)
    {
        return assign_expression(e, extension::get_expression_tag_t<std::decay_t<E>>());
    }

    inline void zarray::swap(zarray& rhs)
    {
        std::swap(p_impl, rhs.p_impl);
    }

    inline bool zarray::has_implementation() const
    {
        return bool(p_impl);
    }

    inline zarray_impl& zarray::get_implementation()
    {
        return *p_impl;
    }

    inline const zarray_impl& zarray::get_implementation() const
    {
        return *p_impl;
    }

    template <class T>
    inline bool zarray::can_get_array()const
    {
        return dynamic_cast<ztyped_array<T>*>(p_impl.get()) != nullptr;
    }


    template <class T>
    inline xarray<T>& zarray::get_array()
    {
        return dynamic_cast<ztyped_array<T>*>(p_impl.get())->get_array();
    }

    template <class T>
    inline const xarray<T>& zarray::get_array() const
    {
        return dynamic_cast<const ztyped_array<T>*>(p_impl.get())->get_array();
    }

    inline void zarray::assign_to(zarray_impl& dst, const zassign_args& args) const
    {
        dst.resize(shape());
        zdispatcher_t<detail::xassign_dummy_functor, 1>::dispatch(get_implementation(), dst, args);
    }

    inline std::size_t zarray::dimension() const
    {
        return p_impl->dimension();
    }

    inline auto zarray::shape() const -> const shape_type&
    {
        return p_impl->shape();
    }

    inline void zarray::reshape(const shape_type& shape)
    {
        p_impl->reshape(shape);
    }

    inline void zarray::reshape(shape_type&& shape)
    {
        p_impl->reshape(std::move(shape));
    }

    inline void zarray::resize(const shape_type& shape)
    {
        p_impl->resize(shape);
    }

    inline void zarray::resize(shape_type&& shape)
    {
        p_impl->resize(std::move(shape));
    }

    inline bool zarray::broadcast_shape(shape_type& shape, bool reuse_cache) const
    {
        return p_impl->broadcast_shape(shape, reuse_cache);
    }

    inline const zchunked_array& zarray::as_chunked_array() const
    {
        return dynamic_cast<const zchunked_array&>(*(p_impl.get()));
    }

    inline const nlohmann::json& zarray::get_metadata() const
    {
        return p_impl->get_metadata();
    }

    inline void zarray::set_metadata(const nlohmann::json& metadata)
    {
        return p_impl->set_metadata(metadata);
    }

    inline zarray strided_view(zarray& z, xstrided_slice_vector& slices)
    {
        std::unique_ptr<zarray_impl> p(z.get_implementation().strided_view(slices));
        return zarray(std::move(p));
    }

    inline std::ostream& operator<<(std::ostream& out, const zarray& ar)
    {
        return ar.get_implementation().print(out);
    }
}

#endif

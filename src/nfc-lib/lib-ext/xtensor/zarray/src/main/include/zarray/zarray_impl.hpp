/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARRAY_IMPL_HPP
#define XTENSOR_ZARRAY_IMPL_HPP

#include <nlohmann/json.hpp>

#include <xtl/xplatform.hpp>
#include <xtl/xhalf_float.hpp>
#include <xtl/xmeta_utils.hpp>
#include <xtensor/xarray.hpp>
#include <xtensor/xchunked_array.hpp>
#include <xtensor/xchunked_view.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xnoalias.hpp>
#include <xtensor/xscalar.hpp>
#include <xtensor/xshape.hpp>
#include <xtensor/xshape.hpp>
#include <xtensor/xstrided_view.hpp>

namespace xt
{
    template <class CTE>
    class zarray_wrapper;

    template <class CTE>
    class zchunked_wrapper;

    template <class CTE>
    class zscalar_wrapper;

    template <class CTE>
    class zexpression_wrapper;

    /******************
     * zarray builder *
     ******************/

    namespace detail
    {
        template <class E>
        struct is_xarray : std::false_type
        {
        };

        template <class T, layout_type L, class A, class SA>
        struct is_xarray<xarray<T, L, A, SA>> : std::true_type
        {
        };

        template <class E>
        struct is_chunked_array : std::false_type
        {
        };

        template <class CS>
        struct is_chunked_array<xchunked_array<CS>> : std::true_type
        {
        };

        template <class E>
        struct is_xscalar : std::false_type
        {
        };

        template <class T>
        struct is_xscalar<xscalar<T>> : std::true_type
        {
        };

        template <class E>
        struct is_xstrided_view : std::false_type
        {
        };

        template <class CT, class S, layout_type L, class FST>
        struct is_xstrided_view<xstrided_view<CT, S, L, FST>> : std::true_type
        {
        };

        template <class E>
        struct zwrapper_builder
        {
            using closure_type = xtl::closure_type_t<E>;

            using wrapper_type =  xtl::mpl::switch_t<
                is_xarray<std::decay_t<E>>,         zarray_wrapper<closure_type>,
                is_chunked_array<std::decay_t<E>>,  zchunked_wrapper<closure_type>,
                is_xscalar<std::decay_t<E>>,        zscalar_wrapper<closure_type>,
                std::true_type,                     zexpression_wrapper<closure_type>
            >;

            template <class OE>
            static wrapper_type* run(OE&& e)
            {
                return new wrapper_type(std::forward<OE>(e));
            }
        };

        template <class E>
        inline auto build_zarray(E&& e)
        {
            return zwrapper_builder<E>::run(std::forward<E>(e));
        }

        template <class CT>
        using is_const = std::is_const<std::remove_reference_t<CT>>;

        template <class CT, class R = void>
        using disable_xstrided_view_t = std::enable_if_t<!is_xstrided_view<CT>::value || is_const<CT>::value, R>;

        template <class CT, class R = void>
        using enable_const_t = std::enable_if_t<is_const<CT>::value, R>;

        template <class CT, class R = void>
        using disable_const_t = std::enable_if_t<!is_const<CT>::value, R>;
    }

    /*************************
     * zarray_expression_tag *
     *************************/

    struct zarray_expression_tag {};

    namespace extension
    {
        template <>
        struct expression_tag_and<xtensor_expression_tag, zarray_expression_tag>
        {
            using type = zarray_expression_tag;
        };

        template <>
        struct expression_tag_and<zarray_expression_tag, xtensor_expression_tag>
            : expression_tag_and<xtensor_expression_tag, zarray_expression_tag>
        {
        };

        template <>
        struct expression_tag_and<zarray_expression_tag, zarray_expression_tag>
        {
            using type = zarray_expression_tag;
        };
    }

    /***************
     * zarray_impl *
     ***************/

    class zarray_impl
    {
    public:

        using self_type = zarray_impl;
        using shape_type = dynamic_shape<std::size_t>;

        virtual ~zarray_impl() = default;

        zarray_impl(zarray_impl&&) = delete;
        zarray_impl& operator=(const zarray_impl&) = delete;
        zarray_impl& operator=(zarray_impl&&) = delete;

        virtual self_type* clone() const = 0;
        virtual std::ostream& print(std::ostream& out) const = 0;

        virtual bool is_array() const = 0;
        virtual bool is_chunked() const = 0;

        virtual self_type* strided_view(xstrided_slice_vector& slices) = 0;

        virtual const nlohmann::json& get_metadata() const = 0;
        virtual void set_metadata(const nlohmann::json& metadata) = 0;

        virtual std::size_t dimension() const = 0;
        virtual const shape_type& shape() const = 0;
        virtual void reshape(const shape_type& shape) = 0;
        virtual void reshape(shape_type&& shape) = 0;
        virtual void resize(const shape_type& shape) = 0;
        virtual void resize(shape_type&& shape) = 0;
        virtual bool broadcast_shape(shape_type& shape, bool reuse_cache = 0) const = 0;

        XTL_IMPLEMENT_INDEXABLE_CLASS()

    protected:

        zarray_impl() = default;
        zarray_impl(const zarray_impl&) = default;
    };

    /****************
     * ztyped_array *
     ****************/

    template <class T>
    class ztyped_array : public zarray_impl
    {
    public:

        using base_type = zarray_impl;
        using shape_type = base_type::shape_type;
        using slice_vector = xstrided_slice_vector;

        virtual ~ztyped_array() = default;

        virtual xarray<T>& get_array() = 0;
        virtual const xarray<T>& get_array() const = 0;
        virtual xarray<T> get_chunk(const slice_vector& slices) const = 0;

        XTL_IMPLEMENT_INDEXABLE_CLASS()

    protected:

        ztyped_array() = default;
        ztyped_array(const ztyped_array&) = default;
    };

    /*****************
     * set_data_type *
     *****************/

    namespace detail
    {
        inline const std::string endianness_string()
        {
            static std::string endianness = (xtl::endianness() == xtl::endian::little_endian) ? "<" : ">";
            return endianness;
        }

        template <class T>
        inline void set_data_type(nlohmann::json& metadata)
        {
        }

        template <>
        inline void set_data_type<bool>(nlohmann::json& metadata)
        {
            metadata["data_type"] = "bool";
        }

        template <>
        inline void set_data_type<uint8_t>(nlohmann::json& metadata)
        {
            metadata["data_type"] = "u1";
        }

        template <>
        inline void set_data_type<int8_t>(nlohmann::json& metadata)
        {
            metadata["data_type"] = "i1";
        }

        template <>
        inline void set_data_type<int16_t>(nlohmann::json& metadata)
        {
            metadata["data_type"] = endianness_string() + "i2";
        }

        template <>
        inline void set_data_type<uint16_t>(nlohmann::json& metadata)
        {
            metadata["data_type"] = endianness_string() + "u2";
        }

        template <>
        inline void set_data_type<int32_t>(nlohmann::json& metadata)
        {
            metadata["data_type"] = endianness_string() + "i4";
        }

        template <>
        inline void set_data_type<uint32_t>(nlohmann::json& metadata)
        {
            metadata["data_type"] = endianness_string() + "u4";
        }

        template <>
        inline void set_data_type<int64_t>(nlohmann::json& metadata)
        {
            metadata["data_type"] = endianness_string() + "i8";
        }

        template <>
        inline void set_data_type<uint64_t>(nlohmann::json& metadata)
        {
            metadata["data_type"] = endianness_string() + "u8";
        }

        template <>
        inline void set_data_type<xtl::half_float>(nlohmann::json& metadata)
        {
            metadata["data_type"] = endianness_string() + "f2";
        }

        template <>
        inline void set_data_type<float>(nlohmann::json& metadata)
        {
            metadata["data_type"] = endianness_string() + "f4";
        }

        template <>
        inline void set_data_type<double>(nlohmann::json& metadata)
        {
            metadata["data_type"] = endianness_string() + "f8";
        }
    }
}

#endif

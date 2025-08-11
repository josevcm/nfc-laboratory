#ifndef XTENSOR_ZARRAY_IMPL_REGISTER_HPP
#define XTENSOR_ZARRAY_IMPL_REGISTER_HPP

#include "zarray_impl.hpp"

namespace xt
{
    /************************
     * zarray_impl_register *
     ************************/

    class zarray_impl_register
    {
    public:

        template <class T>
        static void insert();

        static void init();
        static const zarray_impl& get(size_t index);

    private:

        static zarray_impl_register& instance();

        zarray_impl_register();
        ~zarray_impl_register() = default;

        template <class T>
        void insert_impl();

        size_t m_next_index;
        std::vector<std::unique_ptr<zarray_impl>> m_register;
    };


    /***************************************
     * zarray_impl_register implementation *
     ***************************************/

    template <class T>
    inline void zarray_impl_register::insert()
    {
        instance().template insert_impl<T>();
    }

    inline void zarray_impl_register::init()
    {
        instance();
    }

    inline const zarray_impl& zarray_impl_register::get(size_t index)
    {
        return *(instance().m_register[index]);
    }

    inline zarray_impl_register& zarray_impl_register::instance()
    {
        static zarray_impl_register r;
        return r;
    }

    inline zarray_impl_register::zarray_impl_register()
        : m_next_index(0)
    {

        insert_impl<bool>();

        insert_impl<uint8_t>();
        insert_impl<uint16_t>();
        insert_impl<uint32_t>();
        insert_impl<uint64_t>();

        insert_impl<int8_t>();
        insert_impl<int16_t>();
        insert_impl<int32_t>();
        insert_impl<int64_t>();

        insert_impl<float>();
        insert_impl<double>();

    }

    template <class T>
    inline void zarray_impl_register::insert_impl()
    {
        size_t& idx = ztyped_array<T>::get_class_static_index();
        if (idx == SIZE_MAX)
        {
            m_register.resize(++m_next_index);
            idx = m_register.size() - 1u;

        }
        else if (m_register.size() <= idx)
        {
            m_register.resize(idx + 1u);
        }
        m_register[idx] = std::unique_ptr<zarray_impl>(detail::build_zarray(std::move(xarray<T>())));
    }

}

#endif
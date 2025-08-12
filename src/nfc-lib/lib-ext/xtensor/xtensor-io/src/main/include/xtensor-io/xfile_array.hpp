#ifndef XTENSOR_IO_FILE_ARRAY_HPP
#define XTENSOR_IO_FILE_ARRAY_HPP

#include <istream>
#include <fstream>
#include <iostream>

#include <xtl/xtype_traits.hpp>

#include <xtensor/xarray.hpp>
#include <xtensor/xnoalias.hpp>

namespace xt
{
    enum class xfile_mode { load, init, init_on_fail };

    struct xfile_dirty
    {
        bool data_dirty;
        bool shape_dirty;

        xfile_dirty(bool is_dirty=false)
        {
            data_dirty = is_dirty;
            shape_dirty = is_dirty;
        }

        operator bool() const
        {
            return data_dirty || shape_dirty;
        }
    };

    template <class T>
    class xfile_value_reference
    {
    public:

        using self_type = xfile_value_reference<T>;
        using const_reference = const T&;

        xfile_value_reference(T& value, xfile_dirty& dirty, bool& invalidate);
        ~xfile_value_reference() = default;

        xfile_value_reference(const xfile_value_reference&) = default;
        xfile_value_reference(xfile_value_reference&&) = default;

        self_type& operator=(const self_type&);
        self_type& operator=(self_type&&);

        template <class V>
        self_type& operator=(const V&);

        template <class V>
        self_type& operator+=(const V&);

        template <class V>
        self_type& operator-=(const V&);

        template <class V>
        self_type& operator*=(const V&);

        template <class V>
        self_type& operator/=(const V&);

        operator const_reference() const;

    private:

        T& m_value;
        xfile_dirty& m_dirty;
        bool& m_invalidate;
    };
}

namespace xtl
{
    template <class T>
    struct is_signed<xt::xfile_value_reference<T>>
        : is_signed<T>
    {
    };
}

namespace xt
{
    template <class E, class IOH>
    class xfile_array_container;

    template <class E, class IOH>
    struct xcontainer_inner_types<xfile_array_container<E, IOH>>
    {
        using storage_type = E;
        using value_type = typename storage_type::value_type;
        using reference = xfile_value_reference<value_type>;
        using const_reference = typename storage_type::const_reference;
        using size_type = typename storage_type::size_type;
        using temporary_type = xfile_array_container<E, IOH>;
    };

    template <class E, class IOH>
    struct xiterable_inner_types<xfile_array_container<E, IOH>>
    {
        using inner_shape_type = typename E::shape_type;
        using const_stepper = xindexed_stepper<xfile_array_container<E, IOH>, true>;
        using stepper = xindexed_stepper<xfile_array_container<E, IOH>, false>;
    };

    /**
     * @class xfile_array_container
     * @brief Dense multidimensional file-backed cached container with tensor semantic.
     *
     * The xfile_array_container class implements a dense multidimensional container
     * with tensor semantic, stored on a file system. It acts as a file-backed cached
     * container, allowing for data persistence.
     *
     * @tparam E The type of the container holding the elements
     * @tparam IOH The type of the IO handler (e.g. xio_disk_handler)
     * @sa xchunk_store_manager
     */
    template <class E, class IOH>
    class xfile_array_container : public xaccessible<xfile_array_container<E, IOH>>,
                                  public xiterable<xfile_array_container<E, IOH>>,
                                  public xcontainer_semantic<xfile_array_container<E, IOH>>
    {
    public:

        using self_type = xfile_array_container<E, IOH>;
        using semantic_base = xcontainer_semantic<self_type>;
        using iterable_base = xconst_iterable<self_type>;
        using inner_types = xcontainer_inner_types<self_type>;
        using storage_type = typename inner_types::storage_type;
        using value_type = typename storage_type::value_type;
        using reference = typename inner_types::reference;
        using const_reference = typename inner_types::const_reference;
        using pointer = typename storage_type::pointer;
        using const_pointer = typename storage_type::const_pointer;
        using size_type = typename inner_types::size_type;
        using difference_type = typename storage_type::difference_type;
        using shape_type = typename storage_type::shape_type;
        using strides_type = typename storage_type::strides_type;
        using stepper = typename iterable_base::stepper;
        using const_stepper = typename iterable_base::const_stepper;
        using temporary_type = typename inner_types::temporary_type;
        using bool_load_type = xt::bool_load_type<value_type>;
        static constexpr layout_type static_layout = layout_type::dynamic;
        static constexpr bool contiguous_layout = true;

        xfile_array_container(const std::string& path, xfile_mode mode=xfile_mode::load);

        template <class IOC>
        xfile_array_container(const std::string& path, IOC& io_config, xfile_mode mode=xfile_mode::load);

        xfile_array_container(const std::string& path, xfile_mode mode, const value_type& init_value);

        ~xfile_array_container();

        xfile_array_container(const self_type&) = default;
        self_type& operator=(const self_type&) = default;

        xfile_array_container(self_type&&) = default;
        self_type& operator=(self_type&&) = default;

        template <class OE>
        xfile_array_container(const xexpression<OE>& e);

        template <class OE>
        xfile_array_container(const xexpression<OE>& e, const std::string& path);

        template <class OE>
        self_type& operator=(const xexpression<OE>& e);

        size_type size() const noexcept;
        const shape_type& shape() const noexcept;
        layout_type layout() const noexcept;
        bool is_contiguous() const noexcept;

        template <class S = shape_type>
        void resize(S&& shape, bool force = false);
        template <class S = shape_type>
        void resize(S&& shape, layout_type l);
        template <class S = shape_type>
        void resize(S&& shape, const strides_type& strides);

        template <class S = shape_type>
        self_type& reshape(S&& shape, layout_type layout = static_layout) &;

        template <class T>
        self_type& reshape(std::initializer_list<T> shape, layout_type layout = static_layout) &;

        template <class... Idxs>
        reference operator()(Idxs... idxs);

        template <class... Idxs>
        const_reference operator()(Idxs... idxs) const;

        template <class It>
        reference element(It first, It last);

        template <class It>
        const_reference element(It first, It last) const;

        storage_type& storage() noexcept;
        const storage_type& storage() const noexcept;

        template <class S>
        bool broadcast_shape(S& s, bool reuse_cache = false) const;

        template <class S>
        bool has_linear_assign(const S& strides) const noexcept;

        template <class O>
        stepper stepper_begin(const O& shape) noexcept;
        template <class O>
        stepper stepper_end(const O& shape, layout_type) noexcept;

        template <class O>
        const_stepper stepper_begin(const O& shape) const noexcept;
        template <class O>
        const_stepper stepper_end(const O& shape, layout_type) const noexcept;

        reference data_element(size_type i);
        const_reference data_element(size_type i) const;

        template <class requested_type>
        using simd_return_type = xt_simd::simd_return_type<value_type, requested_type>;

        template <class align, class simd>
        void store_simd(size_type i, const simd& e);
        template <class align, class requested_type = value_type,
                  std::size_t N = xt_simd::simd_traits<requested_type>::size>
        container_simd_return_type_t<storage_type, value_type, requested_type>
        load_simd(size_type i) const;

        const std::string& path() const noexcept;
        void set_path(const std::string& path);

        template <class FC, class IOC>
        void configure(FC& format_config, IOC& io_config);

        template <class IOC>
        void configure_io(IOC& io_config);

        void flush();

    private:

        E m_storage;
        xfile_dirty m_dirty;
        bool m_invalidate;
        IOH m_io_handler;
        std::string m_path;
        xfile_mode m_file_mode;
        value_type m_init_value;
        bool m_init;
    };

    template <class T,
              class IOH,
              layout_type L = XTENSOR_DEFAULT_LAYOUT,
              class A = XTENSOR_DEFAULT_ALLOCATOR(T),
              class SA = std::allocator<typename std::vector<T, A>::size_type>>
    using xfile_array = xfile_array_container<xarray<T, L, A, SA>, IOH>;

    /****************************************
     * xfile_value_reference implementation *
     ****************************************/

    template <class T>
    inline xfile_value_reference<T>::xfile_value_reference(T& value, xfile_dirty& dirty, bool& invalidate)
        : m_value(value)
        , m_dirty(dirty)
        , m_invalidate(invalidate)
    {
    }

    template <class T>
    template <class V>
    inline auto xfile_value_reference<T>::operator=(const V& v) -> self_type&
    {
        if (v != m_value || m_invalidate)
        {
            m_value = v;
            m_dirty.data_dirty = true;
            m_invalidate = false;
        }
        return *this;
    }

    template <class T>
    template <class V>
    inline auto xfile_value_reference<T>::operator+=(const V& v) -> self_type&
    {
        if (v != T(0) || m_invalidate)
        {
            m_value += v;
            m_dirty.data_dirty = true;
            m_invalidate = false;
        }
        return *this;
    }

    template <class T>
    template <class V>
    inline auto xfile_value_reference<T>::operator-=(const V& v) -> self_type&
    {
        if (v != T(0) || m_invalidate)
        {
            m_value -= v;
            m_dirty.data_dirty = true;
            m_invalidate = false;
        }
        return *this;
    }

    template <class T>
    template <class V>
    inline auto xfile_value_reference<T>::operator*=(const V& v) -> self_type&
    {
        if (v != T(1) || m_invalidate)
        {
            m_value *= v;
            m_dirty.data_dirty = true;
            m_invalidate = false;
        }
        return *this;
    }

    template <class T>
    template <class V>
    inline auto xfile_value_reference<T>::operator/=(const V& v) -> self_type&
    {
        if (v != T(1) || m_invalidate)
        {
            m_value /= v;
            m_dirty.data_dirty = true;
            m_invalidate = false;
        }
        return *this;
    }

    template <class T>
    inline xfile_value_reference<T>::operator const_reference() const
    {
        return m_value;
    }

    /****************************************
     * xfile_array_container implementation *
     ****************************************/

    namespace detail
    {
        // Workaround for VS2015
        template <class E>
        using try_path = decltype(std::declval<E>().path());

        template <class E, template <class> class OP, class = void>
        struct file_helper_impl
        {
            using is_stored = std::false_type;

            static const char* path(const xexpression<E>&)
            {
                return "";
            }
        };

        template <class E, template <class> class OP>
        struct file_helper_impl<E, OP, void_t<OP<E>>>
        {
            using is_stored = std::true_type;

            static const char* path(const xexpression<E>& e)
            {
                return e.derived_cast().path();
            }
        };

        template <class E>
        using file_helper = file_helper_impl<E, try_path>;
    }

    template<class E>
    constexpr bool is_stored(const xexpression<E>&)
    {
        using return_type = typename detail::file_helper<E>::is_stored;
        return return_type::value;
    }

    template <class E, class IOH>
    inline xfile_array_container<E, IOH>::xfile_array_container(const std::string& path, xfile_mode file_mode)
        : m_storage()
        , m_dirty(false)
        , m_invalidate(false)
        , m_io_handler()
        , m_file_mode(file_mode)
        , m_init(false)
    {
        set_path(path);
    }

    template <class E, class IOH>
    template <class IOC>
    inline xfile_array_container<E, IOH>::xfile_array_container(const std::string& path, IOC& io_config, xfile_mode file_mode)
        : m_storage()
        , m_dirty(false)
        , m_invalidate(false)
        , m_io_handler()
        , m_file_mode(file_mode)
        , m_init(false)
    {
        m_io_handler.configure_io(io_config);
        set_path(path);
    }

    template <class E, class IOH>
    inline xfile_array_container<E, IOH>::xfile_array_container(const std::string& path, xfile_mode file_mode, const value_type& init_value)
        : m_storage()
        , m_dirty(false)
        , m_invalidate(false)
        , m_io_handler()
        , m_file_mode(file_mode)
        , m_init_value(init_value)
        , m_init(true)
    {
        set_path(path);
    }

    template <class E, class IOH>
    inline xfile_array_container<E, IOH>::~xfile_array_container()
    {
        flush();
    }

    template <class E, class IOH>
    template <class OE>
    inline xfile_array_container<E, IOH>::xfile_array_container(const xexpression<OE>& e)
        : m_storage(e)
        , m_dirty(true)
        , m_invalidate(false)
        , m_io_handler()
        , m_path(detail::file_helper<E>::path(e))
        , m_file_mode(xfile_mode::init)
        , m_init(false)
    {
    }

    template <class E, class IOH>
    template <class OE>
    inline xfile_array_container<E, IOH>::xfile_array_container(const xexpression<OE>& e, const std::string& path)
        : m_storage(e)
        , m_dirty(true)
        , m_invalidate(false)
        , m_io_handler()
        , m_path(path)
        , m_file_mode(xfile_mode::init)
        , m_init(false)
    {
    }

    template <class E, class IOH>
    template <class OE>
    inline auto xfile_array_container<E, IOH>::operator=(const xexpression<OE>& e) -> self_type&
    {
        return semantic_base::operator=(e);
    }

    template <class E, class IOH>
    inline auto xfile_array_container<E, IOH>::size() const noexcept -> size_type
    {
        return m_storage.size();
    }

    template <class E, class IOH>
    inline auto xfile_array_container<E, IOH>::shape() const noexcept -> const shape_type&
    {
        return m_storage.shape();
    }

    template <class E, class IOH>
    inline auto xfile_array_container<E, IOH>::layout() const noexcept -> layout_type
    {
        return m_storage.layout();
    }

    template <class E, class IOH>
    inline bool xfile_array_container<E, IOH>::is_contiguous() const noexcept
    {
        return m_storage.is_contiguous();
    }

    template <class E, class IOH>
    template <class S>
    inline void xfile_array_container<E, IOH>::resize(S&& shape, bool force)
    {
        m_storage.resize(std::forward<S>(shape), force);
        m_dirty.shape_dirty = true;
    }

    template <class E, class IOH>
    template <class S>
    inline void xfile_array_container<E, IOH>::resize(S&& shape, layout_type l)
    {
        m_storage.resize(std::forward<S>(shape), l);
        m_dirty.shape_dirty = true;
    }

    template <class E, class IOH>
    template <class S>
    inline void xfile_array_container<E, IOH>::resize(S&& shape, const strides_type& strides)
    {
        m_storage.resize(std::forward<S>(shape), strides);
        m_dirty.shape_dirty = true;
    }

    template <class E, class IOH>
    template <class S>
    inline auto xfile_array_container<E, IOH>::reshape(S&& shape, layout_type layout) & -> self_type&
    {
        m_storage.reshape(std::forward<S>(shape), layout);
        m_dirty.shape_dirty = true;
        return *this;
    }

    template <class E, class IOH>
    template <class T>
    inline auto xfile_array_container<E, IOH>::reshape(std::initializer_list<T> shape, layout_type layout) & -> self_type&
    {
        m_storage.reshape(shape, layout);
        m_dirty.shape_dirty = true;
        return *this;
    }

    template <class E, class IOH>
    template <class... Idxs>
    inline auto xfile_array_container<E, IOH>::operator()(Idxs... idxs) -> reference
    {
        return reference(m_storage(idxs...), m_dirty);
    }

    template <class E, class IOH>
    template <class... Idxs>
    inline auto xfile_array_container<E, IOH>::operator()(Idxs... idxs) const -> const_reference
    {
        return m_storage(idxs...);
    }

    template <class E, class IOH>
    template <class It>
    inline auto xfile_array_container<E, IOH>::element(It first, It last) -> reference
    {
        return reference(m_storage.element(first, last), m_dirty, m_invalidate);
    }

    template <class E, class IOH>
    template <class It>
    inline auto xfile_array_container<E, IOH>::element(It first, It last) const -> const_reference
    {
        return m_storage.element(first, last);
    }

    template <class E, class IOH>
    inline auto xfile_array_container<E, IOH>::storage() noexcept -> storage_type&
    {
        return m_storage;
    }

    template <class E, class IOH>
    inline auto xfile_array_container<E, IOH>::storage() const noexcept -> const storage_type&
    {
        return m_storage;
    }

    template <class E, class IOH>
    template <class S>
    inline bool xfile_array_container<E, IOH>::broadcast_shape(S& s, bool reuse_cache) const
    {
        return m_storage.broadcast_shape(s, reuse_cache);
    }

    template <class E, class IOH>
    template <class S>
    inline bool xfile_array_container<E, IOH>::has_linear_assign(const S& strides) const noexcept
    {
        return m_storage.has_linear_assign(strides);
    }

    template <class E, class IOH>
    template <class O>
    inline auto xfile_array_container<E, IOH>::stepper_begin(const O& shape) noexcept -> stepper
    {
        size_type offset = shape.size() - this->dimension();
        return stepper(this, offset);
    }

    template <class E, class IOH>
    template <class O>
    inline auto xfile_array_container<E, IOH>::stepper_end(const O& shape, layout_type) noexcept -> stepper
    {
        size_type offset = shape.size() - this->dimension();
        return stepper(this, offset, true);
    }

    template <class E, class IOH>
    template <class O>
    inline auto xfile_array_container<E, IOH>::stepper_begin(const O& shape) const noexcept -> const_stepper
    {
        size_type offset = shape.size() - this->dimension();
        return const_stepper(this, offset);
    }

    template <class E, class IOH>
    template <class O>
    inline auto xfile_array_container<E, IOH>::stepper_end(const O& shape, layout_type) const noexcept -> const_stepper
    {
        size_type offset = shape.size() - this->dimension();
        return const_stepper(this, offset, true);
    }

    template <class E, class IOH>
    inline auto xfile_array_container<E, IOH>::data_element(size_type i) -> reference
    {
        return reference(m_storage.data_element(i), m_dirty);
    }

    template <class E, class IOH>
    inline auto xfile_array_container<E, IOH>::data_element(size_type i) const -> const_reference
    {
        return m_storage.element(i);
    }

    template <class E, class IOH>
    template <class align, class simd>
    inline void xfile_array_container<E, IOH>::store_simd(size_type i, const simd& e)
    {
        m_storage.store_simd(i, e);
        m_dirty = true;
    }

    template <class E, class IOH>
    template <class align, class requested_type, std::size_t N>
    inline auto xfile_array_container<E, IOH>::load_simd(size_type i) const
        -> container_simd_return_type_t<storage_type, value_type, requested_type>
    {
        return m_storage.load_simd(i);
    }

    template <class E, class IOH>
    inline const std::string& xfile_array_container<E, IOH>::path() const noexcept
    {
        return m_path;
    }

    template <class E, class IOH>
    template <class FC, class IOC>
    inline void xfile_array_container<E, IOH>::configure(FC& format_config, IOC& io_config)
    {
        m_io_handler.configure(format_config, io_config);
    }

    template <class E, class IOH>
    template <class IOC>
    inline void xfile_array_container<E, IOH>::configure_io(IOC& io_config)
    {
        m_io_handler.configure_io(io_config);
    }

    template <class E, class IOH>
    inline void xfile_array_container<E, IOH>::set_path(const std::string& path)
    {
        if (path != m_path)
        {
            // maybe write to old file
            flush();
            m_path = path;
            if (m_file_mode != xfile_mode::init)
            {
                // read new file
                try
                {
                    m_io_handler.read(m_storage, path);
                }
                catch (const std::runtime_error& e)
                {
                    if (m_file_mode == xfile_mode::load)
                    {
                        throw e;
                    }
                    // m_file_mode == xfile_mode::init_on_fail
                    if (m_init)
                    {
                        std::fill(m_storage.begin(), m_storage.end(), m_init_value);
                    }
                    else
                    {
                        m_invalidate = true;
                    }
                }
            }
        }
    }

    template <class E, class IOH>
    inline void xfile_array_container<E, IOH>::flush()
    {
        if (m_dirty)
        {
            m_io_handler.write(m_storage, m_path, m_dirty);
            m_dirty = false;
        }
    }
}

#endif

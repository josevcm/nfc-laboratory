#ifndef XTENSOR_CHUNK_STORE_MANAGER_HPP
#define XTENSOR_CHUNK_STORE_MANAGER_HPP

#include <vector>
#include <array>

#include "ghc/filesystem.hpp"

#include <xtl/xsequence.hpp>

#include "xtensor/xarray.hpp"
#include "xtensor/xchunked_array.hpp"
#include "xfile_array.hpp"

namespace xt
{
    template <class EC, class IP>
    class xchunk_store_manager;

    /***************************
     * xindex_path declaration *
     ***************************/

    class xindex_path
    {
    public:

        std::string get_directory() const;
        void set_directory(const std::string& directory);

        template <class I>
        void index_to_path(I, I, std::string&) const;

    private:

        std::string m_directory;
    };

    /*********************************
     * xchunked_assigner declaration *
     *********************************/

    template <class T, class EC, class IP>
    class xchunked_assigner<T, xchunk_store_manager<EC, IP>>
    {
    public:

        using temporary_type = T;

        template <class E, class DST>
        void build_and_assign_temporary(const xexpression<E>& e, DST& dst);
    };

    /************************************
     * xchunk_store_manager declaration *
     ************************************/

    template <class EC, class IP>
    struct xcontainer_inner_types<xchunk_store_manager<EC, IP>>
    {
        using storage_type = EC;
        using reference = EC&;
        using const_reference = const EC&;
        using size_type = std::size_t;
        using temporary_type = xchunk_store_manager<EC, IP>;
    };

    template <class EC, class IP>
    struct xiterable_inner_types<xchunk_store_manager<EC, IP>>
    {
        using inner_shape_type = std::vector<std::size_t>;
        using stepper = xindexed_stepper<xchunk_store_manager<EC, IP>, false>;
        using const_stepper = xindexed_stepper<xchunk_store_manager<EC, IP>, true>;
    };

    /**
     * @class xchunk_store_manager
     * @brief Multidimensional chunk container and manager.
     *
     * The xchunk_store_manager class implements a multidimensional chunk container.
     * Chunks are managed in a pool, allowing for a limited number of chunks
     * that can simultaneously be hold in memory, by swapping chunks when the pool
     * is full. Should not be used directly, instead use chunked_file_array factory
     * functions.
     *
     * @tparam EC The type of a chunk (e.g. xfile_array)
     * @tparam IP The type of the index-to-path transformer (default: xindex_path)
     */
    template <class EC, class IP = xindex_path>
    class xchunk_store_manager: public xaccessible<xchunk_store_manager<EC, IP>>,
                                public xiterable<xchunk_store_manager<EC, IP>>
    {
    public:

        using self_type = xchunk_store_manager<EC, IP>;
        using inner_types = xcontainer_inner_types<self_type>;
        using storage_type = typename inner_types::storage_type;
        using value_type = storage_type;
        using reference = EC&;
        using const_reference = const EC&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using size_type = typename inner_types::size_type;
        using difference_type = std::ptrdiff_t;
        using iterable_base = xconst_iterable<self_type>;
        using stepper = typename iterable_base::stepper;
        using const_stepper = typename iterable_base::const_stepper;
        using shape_type = typename iterable_base::inner_shape_type;

        template <class S>
        xchunk_store_manager(S&& shape,
                             S&& chunk_shape,
                             const std::string& directory,
                             std::size_t pool_size,
                             layout_type chunk_memory_layout = XTENSOR_DEFAULT_LAYOUT);

        template <class S, class T>
        xchunk_store_manager(S&& shape,
                             S&& chunk_shape,
                             const std::string& directory,
                             std::size_t pool_size,
                             const T& init_value,
                             layout_type chunk_memory_layout = XTENSOR_DEFAULT_LAYOUT);

        ~xchunk_store_manager() = default;

        xchunk_store_manager(const xchunk_store_manager&) = default;
        xchunk_store_manager& operator=(const xchunk_store_manager&) = default;

        xchunk_store_manager(xchunk_store_manager&&) = default;
        xchunk_store_manager& operator=(xchunk_store_manager&&) = default;

        const shape_type& shape() const noexcept;
        const shape_type& chunk_shape() const noexcept;

        template <class... Idxs>
        reference operator()(Idxs... idxs);

        template <class... Idxs>
        const_reference operator()(Idxs... idxs) const;

        template <class It>
        reference element(It first, It last);

        template <class It>
        const_reference element(It first, It last) const;

        template <class O>
        stepper stepper_begin(const O& shape) noexcept;
        template <class O>
        stepper stepper_end(const O& shape, layout_type) noexcept;

        template <class O>
        const_stepper stepper_begin(const O& shape) const noexcept;
        template <class O>
        const_stepper stepper_end(const O& shape, layout_type) const noexcept;

        template <class S>
        void resize(S&& shape);

        size_type size() const;
        const std::string& get_directory() const;
        bool get_pool_size() const;

        IP& get_index_path();
        void flush();

        template <class FC, class IOC>
        void configure(FC& format_config, IOC& io_config);

        template <class I>
        reference map_file_array(I first, I last);

        template <class I>
        const_reference map_file_array(I first, I last) const;

        std::string get_temporary_directory() const;
        void reset_to_directory(const std::string& directory);

    private:

        template <class... Idxs>
        std::array<std::size_t, sizeof...(Idxs)> get_indexes(Idxs... idxs) const;

        template <class S, class T>
        void initialize(S&& shape,
                        S&& chunk_shape,
                        const std::string& directory,
                        bool init,
                        const T& init_value,
                        std::size_t pool_size,
                        layout_type chunk_memory_layout);

        using chunk_pool_type = std::vector<EC>;
        using index_pool_type = std::vector<shape_type>;

        shape_type m_shape;
        shape_type m_chunk_shape;
        chunk_pool_type m_chunk_pool;
        index_pool_type m_index_pool;
        std::size_t m_unload_index;
        IP m_index_path;
    };

    /**
     * Creates a chunked file array.
     * This function returns an uninitialized ``xchunked_array<xchunk_store_manager<xfile_array<T, IOH>>>``.
     *
     * @tparam T The type of the elements (e.g. double)
     * @tparam IOH The type of the IO handler (e.g. xio_disk_handler)
     * @tparam L The layout_type of the array
     * @tparam IP The type of the index-to-path transformer (default: xindex_path)
     *
     * @param shape The shape of the array
     * @param chunk_shape The shape of a chunk
     * @param path The path to the chunk store
     * @param pool_size The size of the chunk pool (default: 1)
     * @param chunk_memory_layout The layout of each chunk (default: XTENSOR_DEFAULT_LAYOUT)
     *
     * @return returns a ``xchunked_array<xchunk_store_manager<xfile_array<T, IOH>>>`` with the given shape, chunk shape and memory layout.
     */
    template <class T, class IOH, layout_type L = XTENSOR_DEFAULT_LAYOUT, class IP = xindex_path, class S>
    xchunked_array<xchunk_store_manager<xfile_array<T, IOH, L>, IP>>
    chunked_file_array(S&& shape,
                       S&& chunk_shape,
                       const std::string& path,
                       std::size_t pool_size = 1,
                       layout_type chunk_memory_layout = XTENSOR_DEFAULT_LAYOUT);

    template <class T, class IOH, layout_type L = XTENSOR_DEFAULT_LAYOUT, class IP = xindex_path, class S>
    xchunked_array<xchunk_store_manager<xfile_array<T, IOH, L>, IP>>
    chunked_file_array(std::initializer_list<S> shape,
                       std::initializer_list<S> chunk_shape,
                       const std::string& path,
                       std::size_t pool_size = 1,
                       layout_type chunk_memory_layout = XTENSOR_DEFAULT_LAYOUT);

    /**
     * Creates a chunked file array.
     * This function returns an uninitialized ``xchunked_array<xchunk_store_manager<xfile_array<T, IOH>>>``.
     *
     * @tparam T The type of the elements (e.g. double)
     * @tparam IOH The type of the IO handler (e.g. xio_disk_handler)
     * @tparam L The layout_type of the array
     * @tparam IP The type of the index-to-path transformer (default: xindex_path)
     *
     * @param shape The shape of the array
     * @param chunk_shape The shape of a chunk
     * @param path The path to the chunk store
     * @param init_value The value with which to initialize the chunks when they are not already stored
     * @param pool_size The size of the chunk pool (default: 1)
     * @param chunk_memory_layout The layout of each chunk (default: XTENSOR_DEFAULT_LAYOUT)
     *
     * @return returns a ``xchunked_array<xchunk_store_manager<xfile_array<T, IOH>>>`` with the given shape, chunk shape and memory layout.
     */
    template <class T, class IOH, layout_type L = XTENSOR_DEFAULT_LAYOUT, class IP = xindex_path, class S>
    xchunked_array<xchunk_store_manager<xfile_array<T, IOH, L>, IP>>
    chunked_file_array(S&& shape,
                       S&& chunk_shape,
                       const std::string& path,
                       const T& init_value,
                       std::size_t pool_size = 1,
                       layout_type chunk_memory_layout = XTENSOR_DEFAULT_LAYOUT);

    template <class T, class IOH, layout_type L = XTENSOR_DEFAULT_LAYOUT, class IP = xindex_path, class S>
    xchunked_array<xchunk_store_manager<xfile_array<T, IOH, L>, IP>>
    chunked_file_array(std::initializer_list<S> shape,
                       std::initializer_list<S> chunk_shape,
                       const std::string& path,
                       const T& init_value,
                       std::size_t pool_size = 1,
                       layout_type chunk_memory_layout = XTENSOR_DEFAULT_LAYOUT);

    /**
     * Creates a chunked file array.
     * This function returns a ``xchunked_array<xchunk_store_manager<xfile_array<T, IOH>>>`` initialized from an expression.
     *
     * @tparam T The type of the elements (e.g. double)
     * @tparam IOH The type of the IO handler (e.g. xio_disk_handler)
     * @tparam L The layout_type of the array
     * @tparam IP The type of the index-to-path transformer (default: xindex_path)
     *
     * @param e The expression to initialize the chunked array from
     * @param chunk_shape The shape of a chunk
     * @param path The path to the chunk store
     * @param pool_size The size of the chunk pool (default: 1)
     * @param chunk_memory_layout The layout of each chunk (default: XTENSOR_DEFAULT_LAYOUT)
     *
     * @return returns a ``xchunked_array<xchunk_store_manager<xfile_array<T, IOH>>>`` from the given expression, with the given chunk shape and memory layout.
     */
    template <class IOH, layout_type L = XTENSOR_DEFAULT_LAYOUT, class IP = xindex_path, class E, class S>
    xchunked_array<xchunk_store_manager<xfile_array<typename E::value_type, IOH, L>, IP>>
    chunked_file_array(const xexpression<E>& e,
                       S&& chunk_shape,
                       const std::string& path,
                       std::size_t pool_size = 1,
                       layout_type chunk_memory_layout = XTENSOR_DEFAULT_LAYOUT);

    /**
     * Creates a chunked file array.
     * This function returns a ``xchunked_array<xchunk_store_manager<xfile_array<T, IOH>>>`` initialized from an expression.
     *
     * @tparam IOH The type of the IO handler (e.g. xio_disk_handler)
     * @tparam L The layout_type of the array
     * @tparam IP The type of the index-to-path transformer (default: xindex_path)
     *
     * @param e The expression to initialize the chunked array from
     * @param path The path to the chunk store
     * @param pool_size The size of the chunk pool (default: 1)
     * @param chunk_memory_layout The layout of each chunk (default: XTENSOR_DEFAULT_LAYOUT)
     *
     * @return returns a ``xchunked_array<xchunk_store_manager<xfile_array<T, IOH>>>`` from the given expression, with the expression's chunk shape and the given memory layout.
     */
    template <class IOH, layout_type L = XTENSOR_DEFAULT_LAYOUT, class IP = xindex_path, class E>
    xchunked_array<xchunk_store_manager<xfile_array<typename E::value_type, IOH, L>, IP>>
    chunked_file_array(const xexpression<E>& e,
                       const std::string& path,
                       std::size_t pool_size = 1,
                       layout_type chunk_memory_layout = XTENSOR_DEFAULT_LAYOUT);

    /******************************
     * xindex_path implementation *
     ******************************/

    inline std::string xindex_path::get_directory() const
    {
        return m_directory;
    }

    inline void xindex_path::set_directory(const std::string& directory)
    {
        m_directory = directory;
        if (m_directory.back() != '/')
        {
            m_directory.push_back('/');
        }
    }

    template <class I>
    void xindex_path::index_to_path(I first, I last, std::string& path) const
    {
        std::string fname;
        for (auto it = first; it != last; ++it)
        {
            if (!fname.empty())
            {
                fname.push_back('.');
            }
            fname.append(std::to_string(*it));
        }
        path = m_directory + fname;
    }

    /************************************
     * xchunked_assigner implementation *
     ************************************/

    template <class T, class EC, class IP>
    template <class E, class DST>
    inline void xchunked_assigner<T, xchunk_store_manager<EC, IP>>::build_and_assign_temporary(const xexpression<E>& e,
                                                                                               DST& dst)
    {
        using store_type = xchunk_store_manager<EC, IP>;
        store_type store(e.derived_cast().shape(), dst.chunk_shape(), dst.chunks().get_temporary_directory(), dst.chunks().get_pool_size());
        temporary_type tmp(e, std::move(store), dst.chunk_shape());
        tmp.chunks().flush();
        dst.chunks().reset_to_directory(tmp.chunks().get_directory());
    }

    /******************************************
     * xchunk_store_manager factory functions *
     ******************************************/

    template <class T, class IOH, layout_type L, class IP, class S>
    inline xchunked_array<xchunk_store_manager<xfile_array<T, IOH, L>, IP>>
    chunked_file_array(S&& shape, S&& chunk_shape, const std::string& path, std::size_t pool_size, layout_type chunk_memory_layout)
    {
        using chunk_storage = xchunk_store_manager<xfile_array<T, IOH, L>, IP>;
        chunk_storage chunks(shape, chunk_shape, path, pool_size, chunk_memory_layout);
        return xchunked_array<chunk_storage>(std::move(chunks), std::forward<S>(shape), std::forward<S>(chunk_shape));
    }

    template <class T, class IOH, layout_type L, class IP, class S>
    inline xchunked_array<xchunk_store_manager<xfile_array<T, IOH, L>, IP>>
    chunked_file_array(std::initializer_list<S> shape, std::initializer_list<S> chunk_shape, const std::string& path, std::size_t pool_size, layout_type chunk_memory_layout)
    {
        using sh_type = std::vector<std::size_t>;
        auto sh = xtl::forward_sequence<sh_type, std::initializer_list<S>>(shape);
        auto ch_sh = xtl::forward_sequence<sh_type, std::initializer_list<S>>(chunk_shape);
        return chunked_file_array<T, IOH, L, IP, sh_type>(std::move(sh), std::move(ch_sh), path, pool_size, chunk_memory_layout);
    }

    template <class T, class IOH, layout_type L, class IP, class S>
    inline xchunked_array<xchunk_store_manager<xfile_array<T, IOH, L>, IP>>
    chunked_file_array(S&& shape, S&& chunk_shape, const std::string& path, const T& init_value, std::size_t pool_size, layout_type chunk_memory_layout)
    {
        using chunk_storage = xchunk_store_manager<xfile_array<T, IOH, L>, IP>;
        chunk_storage chunks(shape, chunk_shape, path, pool_size, init_value, chunk_memory_layout);
        return xchunked_array<chunk_storage>(std::move(chunks), std::forward<S>(shape), std::forward<S>(chunk_shape));
    }

    template <class T, class IOH, layout_type L, class IP, class S>
    inline xchunked_array<xchunk_store_manager<xfile_array<T, IOH, L>, IP>>
    chunked_file_array(std::initializer_list<S> shape, std::initializer_list<S> chunk_shape, const std::string& path, const T& init_value, std::size_t pool_size, layout_type chunk_memory_layout)
    {
        using sh_type = std::vector<std::size_t>;
        auto sh = xtl::forward_sequence<sh_type, std::initializer_list<S>>(shape);
        auto ch_sh = xtl::forward_sequence<sh_type, std::initializer_list<S>>(chunk_shape);
        return chunked_file_array<T, IOH, L, IP, sh_type>(std::move(sh), std::move(ch_sh), path, init_value, pool_size, chunk_memory_layout);
    }

    template <class IOH, layout_type L, class IP, class E, class S>
    inline xchunked_array<xchunk_store_manager<xfile_array<typename E::value_type, IOH, L>, IP>>
    chunked_file_array(const xexpression<E>& e, S&& chunk_shape, const std::string& path, std::size_t pool_size, layout_type chunk_memory_layout)
    {
        using chunk_storage = xchunk_store_manager<xfile_array<typename E::value_type, IOH, L>, IP>;
        chunk_storage chunks(e.derived_cast().shape(), chunk_shape, path, pool_size, chunk_memory_layout);
        return xchunked_array<chunk_storage>(e, chunk_storage(), std::forward<S>(chunk_shape));
    }

    template <class IOH, layout_type L, class IP, class E>
    inline xchunked_array<xchunk_store_manager<xfile_array<typename E::value_type, IOH, L>, IP>>
    chunked_file_array(const xexpression<E>& e, const std::string& path, std::size_t pool_size, layout_type chunk_memory_layout)
    {
        using chunk_storage = xchunk_store_manager<xfile_array<typename E::value_type, IOH, L>, IP>;
        chunk_storage chunks(e.derived_cast().shape(), detail::chunk_helper<E>::chunk_shape(e), path, pool_size, chunk_memory_layout);
        return xchunked_array<chunk_storage>(e, chunk_storage());
    }

    /***************************************
     * xchunk_store_manager implementation *
     ***************************************/

    template <class EC, class IP>
    template <class S>
    inline xchunk_store_manager<EC, IP>::xchunk_store_manager(S&& shape,
                                                              S&& chunk_shape,
                                                              const std::string& directory,
                                                              std::size_t pool_size,
                                                              layout_type chunk_memory_layout)
        : m_shape(xtl::forward_sequence<shape_type, S>(shape))
        , m_chunk_shape(xtl::forward_sequence<shape_type, S>(chunk_shape))
        , m_unload_index(0u)
    {
        initialize(shape, chunk_shape, directory, false, 0, pool_size, chunk_memory_layout);
    }

    template <class EC, class IP>
    template <class S, class T>
    inline xchunk_store_manager<EC, IP>::xchunk_store_manager(S&& shape,
                                                              S&& chunk_shape,
                                                              const std::string& directory,
                                                              std::size_t pool_size,
                                                              const T& init_value,
                                                              layout_type chunk_memory_layout)
        : m_shape(xtl::forward_sequence<shape_type, S>(shape))
        , m_chunk_shape(xtl::forward_sequence<shape_type, S>(chunk_shape))
        , m_unload_index(0u)
    {
        initialize(shape, chunk_shape, directory, true, init_value, pool_size, chunk_memory_layout);
    }

    template <class EC, class IP>
    template <class S, class T>
    inline void xchunk_store_manager<EC, IP>::initialize(S&& shape,
                                                         S&& chunk_shape,
                                                         const std::string& directory,
                                                         bool init,
                                                         const T& init_value,
                                                         std::size_t pool_size,
                                                         layout_type chunk_memory_layout)
    {
        if (pool_size == SIZE_MAX)
        {
            // as many "physical" chunks in the pool as there are "logical" chunks
            pool_size = size();
        }
        if (init)
        {
            m_chunk_pool.resize(pool_size, EC("", xfile_mode::init_on_fail, init_value));
        }
        else
        {
            m_chunk_pool.resize(pool_size, EC("", xfile_mode::init_on_fail));
        }
        m_index_pool.resize(pool_size);
        // resize the pool chunks
        for (auto& chunk: m_chunk_pool)
        {
            chunk.resize(chunk_shape, chunk_memory_layout);
        }
        m_index_path.set_directory(directory);
    }

    template <class EC, class IP>
    inline auto xchunk_store_manager<EC, IP>::shape() const noexcept -> const shape_type&
    {
        return m_shape;
    }

    template <class EC, class IP>
    inline auto xchunk_store_manager<EC, IP>::chunk_shape() const noexcept -> const shape_type&
    {
        return m_chunk_shape;
    }

    template <class EC, class IP>
    template <class... Idxs>
    inline auto xchunk_store_manager<EC, IP>::operator()(Idxs... idxs) -> reference
    {
        auto index = get_indexes(idxs...);
        return map_file_array(index.cbegin(), index.cend());
    }

    template <class EC, class IP>
    template <class... Idxs>
    inline auto xchunk_store_manager<EC, IP>::operator()(Idxs... idxs) const -> const_reference
    {
        auto index = get_indexes(idxs...);
        return map_file_array(index.cbegin(), index.cend());
    }

    template <class EC, class IP>
    template <class It>
    inline auto xchunk_store_manager<EC, IP>::element(It first, It last) -> reference
    {
        return map_file_array(first, last);
    }

    template <class EC, class IP>
    template <class It>
    inline auto xchunk_store_manager<EC, IP>::element(It first, It last) const -> const_reference
    {
        return map_file_array(first, last);
    }

    template <class EC, class IP>
    template <class O>
    inline auto xchunk_store_manager<EC, IP>::stepper_begin(const O& shape) noexcept -> stepper
    {
        size_type offset = shape.size() - this->dimension();
        return stepper(this, offset);
    }

    template <class EC, class IP>
    template <class O>
    inline auto xchunk_store_manager<EC, IP>::stepper_end(const O& shape, layout_type) noexcept -> stepper
    {
        size_type offset = shape.size() - this->dimension();
        return stepper(this, offset, true);
    }

    template <class EC, class IP>
    template <class O>
    inline auto xchunk_store_manager<EC, IP>::stepper_begin(const O& shape) const noexcept -> const_stepper
    {
        size_type offset = shape.size() - this->dimension();
        return const_stepper(this, offset);
    }

    template <class EC, class IP>
    template <class O>
    inline auto xchunk_store_manager<EC, IP>::stepper_end(const O& shape, layout_type) const noexcept -> const_stepper
    {
        size_type offset = shape.size() - this->dimension();
        return const_stepper(this, offset, true);
    }

    template <class EC, class IP>
    template <class S>
    inline void xchunk_store_manager<EC, IP>::resize(S&& shape)
    {
        // don't resize according to total number of chunks
        // instead the pool manages a number of in-memory chunks
        m_shape = shape;
    }

    template <class EC, class IP>
    inline auto xchunk_store_manager<EC, IP>::size() const -> size_type
    {
        return compute_size(m_shape);
    }

    template <class EC, class IP>
    inline const std::string& xchunk_store_manager<EC, IP>::get_directory() const
    {
        return m_index_path.get_directory();
    }

    template <class EC, class IP>
    inline bool  xchunk_store_manager<EC, IP>::get_pool_size() const
    {
        return m_chunk_pool.size();
    }

    template <class EC, class IP>
    inline void xchunk_store_manager<EC, IP>::flush()
    {
        for (auto& chunk: m_chunk_pool)
        {
            chunk.flush();
        }
    }

    template <class EC, class IP>
    template <class FC, class IOC>
    void xchunk_store_manager<EC, IP>::configure(FC& format_config, IOC& io_config)
    {
        for (auto& chunk: m_chunk_pool)
        {
            chunk.configure(format_config, io_config);
        }
    }

    template <class EC, class IP>
    IP& xchunk_store_manager<EC, IP>::get_index_path()
    {
        return m_index_path;
    }

    template <class EC, class IP>
    template <class I>
    inline auto xchunk_store_manager<EC, IP>::map_file_array(I first, I last) -> reference
    {
        std::string path;
        m_index_path.index_to_path(first, last, path);
        if (first == last)
        {
            return m_chunk_pool[0];
        }
        else
        {
            // check if the chunk is already loaded in memory
            const auto it1 = std::find_if(m_index_pool.cbegin(), m_index_pool.cend(), [first, last](const auto& v)
                { return std::equal(v.cbegin(), v.cend(), first, last); });
            std::size_t i;
            if (it1 != m_index_pool.cend())
            {
                i = static_cast<std::size_t>(std::distance(m_index_pool.cbegin(), it1));
                return m_chunk_pool[i];
            }
            // if not, find a free chunk in the pool
            std::vector<std::size_t> empty_index;
            const auto it2 = std::find(m_index_pool.cbegin(), m_index_pool.cend(), empty_index);
            if (it2 != m_index_pool.cend())
            {
                i = static_cast<std::size_t>(std::distance(m_index_pool.cbegin(), it2));
                m_chunk_pool[i].set_path(path);
                m_index_pool[i].resize(static_cast<size_t>(std::distance(first, last)));
                std::copy(first, last, m_index_pool[i].begin());
                return m_chunk_pool[i];
            }
            // no free chunk, take one (which will thus be unloaded)
            // fairness is guaranteed through the use of a walking index
            m_chunk_pool[m_unload_index].set_path(path);
            m_index_pool[m_unload_index].resize(static_cast<size_t>(std::distance(first, last)));
            std::copy(first, last, m_index_pool[m_unload_index].begin());
            auto& chunk = m_chunk_pool[m_unload_index];
            m_unload_index = (m_unload_index + 1) % m_index_pool.size();
            return chunk;
        }
    }

    template <class EC, class IP>
    template <class I>
    inline auto xchunk_store_manager<EC, IP>::map_file_array(I first, I last) const -> const_reference
    {
        return const_cast<xchunk_store_manager<EC, IP>*>(this)->map_file_array(first, last);
    }

    template <class EC, class IP>
    inline std::string xchunk_store_manager<EC, IP>::get_temporary_directory() const
    {
        namespace fs = ghc::filesystem;
        fs::path tmp_dir = fs::temp_directory_path();
        std::size_t count = 0;
        while(fs::exists(tmp_dir / std::to_string(count)))
        {
            ++count;
        }
        return tmp_dir / std::to_string(count);
    }

    template <class EC, class IP>
    inline void xchunk_store_manager<EC, IP>::reset_to_directory(const std::string& directory)
    {
        namespace fs = ghc::filesystem;
        fs::remove_all(get_directory());
        fs::rename(directory, get_directory());
        m_unload_index = 0u;
    }

    template <class EC, class IP>
    template <class... Idxs>
    inline std::array<std::size_t, sizeof...(Idxs)>
    xchunk_store_manager<EC, IP>::get_indexes(Idxs... idxs) const
    {
        std::array<std::size_t, sizeof...(Idxs)> indexes = {{idxs...}};
        return indexes;
    }
}

#endif

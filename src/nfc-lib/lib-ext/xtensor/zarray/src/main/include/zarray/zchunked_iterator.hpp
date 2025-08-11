/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZCHUNKED_ITERATOR_HPP
#define XTENSOR_ZCHUNKED_ITERATOR_HPP

#include <memory>

#include <xtensor/xstrided_view.hpp>

namespace xt
{

    /*********************
     * zchunked_iterator *
     *********************/

    class zchunked_iterator_impl;

    class zchunked_iterator
    {
    public:
        
        using implementation_ptr = std::unique_ptr<zchunked_iterator_impl>;

        zchunked_iterator() = default;
        ~zchunked_iterator() = default;
        
        template <class It>
        explicit zchunked_iterator(It&& iter);

        zchunked_iterator(const zchunked_iterator&);
        zchunked_iterator& operator=(const zchunked_iterator&);
        
        zchunked_iterator(zchunked_iterator&&);
        zchunked_iterator& operator=(zchunked_iterator&&);

        zchunked_iterator& operator++();

        const xstrided_slice_vector& get_slice_vector() const;
        xstrided_slice_vector get_chunk_slice_vector() const;

        template <class It>
        const It& get_xchunked_iterator() const;

        bool operator==(const zchunked_iterator& other) const;
        bool operator!=(const zchunked_iterator& other) const;

    private:

        implementation_ptr p_impl;
    };

    /**************************
     * zchunked_iterator_impl *
     **************************/

    class zchunked_iterator_impl
    {
    public:

        virtual ~zchunked_iterator_impl() = default;

        zchunked_iterator_impl(const zchunked_iterator_impl&) = delete;
        zchunked_iterator_impl& operator=(const zchunked_iterator_impl&) = delete;

        zchunked_iterator_impl(zchunked_iterator_impl&&) = delete;
        zchunked_iterator_impl& operator=(zchunked_iterator_impl&&) = delete;

        virtual zchunked_iterator_impl* clone() const = 0;

        virtual void increment() = 0;
        virtual const xstrided_slice_vector& get_slice_vector() const = 0;
        virtual xstrided_slice_vector get_chunk_slice_vector() const = 0;

        virtual bool equal(const zchunked_iterator_impl& other) const = 0;

    protected:

        zchunked_iterator_impl() = default;
    };

    /***************************
     * zchunk_iterator_wrapper *
     ***************************/

    template <class It>
    class zchunked_iterator_wrapper : public zchunked_iterator_impl
    {
    public:

        template <class OIT>
        explicit zchunked_iterator_wrapper(OIT&& it);

        virtual ~zchunked_iterator_wrapper() = default;

        zchunked_iterator_wrapper* clone() const override;
        
        void increment() override;
        const xstrided_slice_vector& get_slice_vector() const override;
        xstrided_slice_vector get_chunk_slice_vector() const override;

        const It& get_xchunked_iterator() const;

        bool equal(const zchunked_iterator_impl& other) const override;

    private:

        It m_iterator;
    };

    /************************************
     * zchunked_iterator implementation *
     ************************************/

    template <class It>
    inline zchunked_iterator::zchunked_iterator(It&& iter)
        : p_impl(new zchunked_iterator_wrapper<std::decay_t<It>>(std::forward<It>(iter)))
    {
    }

    inline zchunked_iterator::zchunked_iterator(const zchunked_iterator& rhs)
        : p_impl(rhs.p_impl->clone())
    {
    }

    inline zchunked_iterator& zchunked_iterator::operator=(const zchunked_iterator& rhs)
    {
        zchunked_iterator tmp(rhs);
        tmp.p_impl.swap(p_impl);
        return *this;
    }
        
    inline zchunked_iterator::zchunked_iterator(zchunked_iterator&& rhs)
        : p_impl(std::move(rhs.p_impl))
    {
    }

    inline zchunked_iterator& zchunked_iterator::operator=(zchunked_iterator&& rhs)
    {
        rhs.p_impl.swap(p_impl);
        return *this;
    }

    inline zchunked_iterator& zchunked_iterator::operator++()
    {
        p_impl->increment();
        return *this;
    }

    inline const xstrided_slice_vector& zchunked_iterator::get_slice_vector() const
    {
        return p_impl->get_slice_vector();
    }

    inline xstrided_slice_vector zchunked_iterator::get_chunk_slice_vector() const
    {
        return p_impl->get_chunk_slice_vector();
    }

    template <class It>
    const It& zchunked_iterator::get_xchunked_iterator() const
    {
        const auto& impl = static_cast<const zchunked_iterator_wrapper<It>&>(*p_impl);
        return impl.get_xchunked_iterator();
    }

    inline bool zchunked_iterator::operator==(const zchunked_iterator& other) const
    {
        return p_impl->equal(*(other.p_impl));
    }

    inline bool zchunked_iterator::operator!=(const zchunked_iterator& other) const
    {
        return !(*this == other);
    }

    /********************************************
     * zchunked_iterator_wrapper implementation *
     ********************************************/

    template <class It>
    template <class OIT>
    inline zchunked_iterator_wrapper<It>::zchunked_iterator_wrapper(OIT&& it)
        : m_iterator(std::forward<OIT>(it))
    {
    }

    template <class It>
    inline zchunked_iterator_wrapper<It>* zchunked_iterator_wrapper<It>::clone() const
    {
        return new zchunked_iterator_wrapper(m_iterator);
    }
        
    template <class It>
    inline void zchunked_iterator_wrapper<It>::increment()
    {
        ++m_iterator;
    }

    template <class It>
    inline const xstrided_slice_vector& zchunked_iterator_wrapper<It>::get_slice_vector() const
    {
        return m_iterator.get_slice_vector();
    }
    
    template <class It>
    inline xstrided_slice_vector zchunked_iterator_wrapper<It>::get_chunk_slice_vector() const
    {
        return m_iterator.get_chunk_slice_vector();
    }

    template <class It>
    inline const It& zchunked_iterator_wrapper<It>::get_xchunked_iterator() const
    {
        return m_iterator;
    }

    template <class It>
    inline bool zchunked_iterator_wrapper<It>::equal(const zchunked_iterator_impl& other) const
    {
        const auto* tmp = dynamic_cast<const zchunked_iterator_wrapper<It>*>(&other);
        if (tmp != nullptr)
        {
            return m_iterator == tmp->m_iterator;
        }
        return false;
    }
}

#endif


/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARRAY_TEMPORARY_POOL_HPP
#define XTENSOR_ZARRAY_TEMPORARY_POOL_HPP

#include <array>
#include <set>
#include <map>
#include <memory>

namespace xt
{
    // forward declare
    class zarray_impl;

    namespace detail
    {
        class zarray_temporary_pool
        {
        public:
            using shape_type = typename zarray_impl::shape_type;

            explicit zarray_temporary_pool(zarray_impl & res)
            :   m_shape(res.shape()),
                m_buffers(),
                m_free_buffers()
            {
                this->mark_as_free(&res);
            }

            auto get_free_buffer(const std::size_t type_index)
            {
                auto r = m_free_buffers.find(type_index);
                if(r == m_free_buffers.end() || r->second.empty())
                {
                    // make new buffer
                    auto buffer_ptr = zarray_impl_register::get(type_index).clone();
                    buffer_ptr->resize(m_shape);
                    m_buffers.emplace_back(buffer_ptr);
                    return buffer_ptr;
                }
                else
                {
                    // remove from free set
                    auto iter = r->second.begin();
                    auto buffer_ptr = *iter;
                    r->second.erase(iter);

                    return buffer_ptr;
                }
            };

            void mark_as_free(const zarray_impl * buffer_ptr)
            {
                m_free_buffers[buffer_ptr->get_class_index()].insert(
                    const_cast<zarray_impl *>(buffer_ptr)
                );
            }

            std::size_t size() const
            {
                return m_buffers.size();
            }

        private:

            const shape_type & m_shape;

            // a vector of buffers since an arbitrary number of temps can be needed
            std::vector<std::unique_ptr<zarray_impl>>  m_buffers;

            // free buffers of different types
            std::map<std::size_t, std::set<zarray_impl * > >  m_free_buffers;
        };
    }
}

#endif
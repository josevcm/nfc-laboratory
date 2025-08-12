/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_IO_XTENSORIO_HPP
#define XTENSOR_IO_XTENSORIO_HPP

namespace xt
{
    enum class file_mode
    {
        create,
        overwrite,
        append,
        read
    };

    enum class dump_mode
    {
        create,
        overwrite
    };

    inline bool is_big_endian(void)
    {
        union {
            uint16_t i;
            char c[2];
        } bint = {0x0102};

        return bint.c[0] == 1;
    }

    template <class T>
    void swap_endianness(xt::svector<T>& buffer)
    {
        char* buf = reinterpret_cast<char*>(buffer.data());
        char* end  = buf + buffer.size() * sizeof(T);
        while(buf != end)
        {
            std::reverse(buf, buf + sizeof(T));
            buf += sizeof(T);
        }
    }
}

#endif

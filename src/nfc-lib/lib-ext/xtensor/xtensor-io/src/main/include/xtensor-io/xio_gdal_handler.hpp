#ifndef XTENSOR_IO_GDAL_HANDLER_HPP
#define XTENSOR_IO_GDAL_HANDLER_HPP

#include <xtensor/xarray.hpp>
#include <xtensor/xexpression.hpp>
#include "xfile_array.hpp"
#include "xio_vsilfile_wrapper.hpp"

#include <cpl_vsi.h>

namespace xt
{
    struct xio_gdal_config
    {
    };

    template <class C>
    class xio_gdal_handler
    {
    public:
        using io_config = xio_gdal_config;

        template <class E>
        void write(const xexpression<E>& expression, const std::string& path, xfile_dirty dirty);

        template <class ET>
        void read(ET& array, const std::string& path);

        void configure(const C& format_config, const xio_gdal_config& io_config);
        void configure_io(const xio_gdal_config& io_config);

    private:

        C m_format_config;
    };

    template <class C>
    template <class E>
    inline void xio_gdal_handler<C>::write(const xexpression<E>& expression, const std::string& path, xfile_dirty dirty)
    {
        if (m_format_config.will_dump(dirty))
        {
            VSILFILE* out_file = VSIFOpenL(path.c_str(), "wb");
            if (out_file != NULL)
            {
                auto f = xvsilfile_wrapper(out_file);
                dump_file(f, expression, m_format_config);
            }
            else
            {
                XTENSOR_THROW(std::runtime_error, "write: failed to open file " + path);
            }
        }
    }

    template <class C>
    template <class ET>
    inline void xio_gdal_handler<C>::read(ET& array, const std::string& path)
    {
        VSILFILE* in_file = VSIFOpenL(path.c_str(), "rb");
        if (in_file != NULL)
        {
            auto f = xvsilfile_wrapper(in_file);
            load_file<ET>(f, array, m_format_config);
        }
        else
        {
            XTENSOR_THROW(std::runtime_error, "read: failed to open file " + path);
        }
    }

    template <class C>
    inline void xio_gdal_handler<C>::configure(const C& format_config, const xio_gdal_config& io_config)
    {
        m_format_config = format_config;
    }

    template <class C>
    inline void xio_gdal_handler<C>::configure_io(const xio_gdal_config& io_config)
    {
    }

}

#endif

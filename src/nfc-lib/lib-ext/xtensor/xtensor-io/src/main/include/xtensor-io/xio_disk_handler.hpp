#ifndef XTENSOR_IO_DISK_HANDLER_HPP
#define XTENSOR_IO_DISK_HANDLER_HPP

#include <ghc/filesystem.hpp>
#include <xtensor/xarray.hpp>
#include <xtensor/xexpression.hpp>
#include "xio_stream_wrapper.hpp"
#include "xfile_array.hpp"

namespace fs = ghc::filesystem;

namespace xt
{
    struct xio_disk_config
    {
        bool create_directories;
    };

    template <class C>
    class xio_disk_handler
    {
    public:
        using io_config = xio_disk_config;

        xio_disk_handler();

        template <class E>
        void write(const xexpression<E>& expression, const std::string& path, xfile_dirty dirty);

        template <class ET>
        void read(ET& array, const std::string& path);

        void configure(const C& format_config, const xio_disk_config& io_config);
        void configure_io(const xio_disk_config& io_config);

    private:

        C m_format_config;
        bool m_create_directories;
    };

    template <class C>
    xio_disk_handler<C>::xio_disk_handler()
        : m_create_directories(true)
    {
    }

    template <class C>
    template <class E>
    inline void xio_disk_handler<C>::write(const xexpression<E>& expression, const std::string& path, xfile_dirty dirty)
    {
        if (m_format_config.will_dump(dirty))
        {
            if (m_create_directories)
            {
                // maybe create directories
                std::size_t i = path.rfind('/');
                if (i != std::string::npos)
                {
                    fs::path directory = path.substr(0, i);
                    if (fs::exists(directory))
                    {
                        if (!fs::is_directory(directory))
                        {
                            XTENSOR_THROW(std::runtime_error, "Path is not a directory: " + std::string(directory.string()));
                        }
                    }
                    else
                    {
                        fs::create_directories(directory);
                    }
                }
            }
            std::ofstream out_file(path, std::ofstream::binary);
            if (out_file.is_open())
            {
                auto s = xostream_wrapper(out_file);
                dump_file(s, expression, m_format_config);
            }
            else
            {
                XTENSOR_THROW(std::runtime_error, "write: failed to open file " + path);
            }
        }
    }

    template <class C>
    template <class ET>
    inline void xio_disk_handler<C>::read(ET& array, const std::string& path)
    {
        std::ifstream in_file(path, std::ifstream::binary);
        if (in_file.is_open())
        {
            auto s = xistream_wrapper(in_file);
            load_file<ET>(s, array, m_format_config);
        }
        else
        {
            XTENSOR_THROW(std::runtime_error, "read: failed to open file " + path);
        }
    }

    template <class C>
    inline void xio_disk_handler<C>::configure(const C& format_config, const xio_disk_config& io_config)
    {
        m_format_config = format_config;
        m_create_directories = io_config.create_directories;
    }

    template <class C>
    inline void xio_disk_handler<C>::configure_io(const xio_disk_config& io_config)
    {
        m_create_directories = io_config.create_directories;
    }

}

#endif

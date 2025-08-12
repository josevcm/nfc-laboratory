#ifndef XTENSOR_IO_AWS_HANDLER_HPP
#define XTENSOR_IO_AWS_HANDLER_HPP

#include <xtensor/xarray.hpp>
#include <xtensor/xexpression.hpp>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include "xfile_array.hpp"
#include "xio_stream_wrapper.hpp"

namespace xt
{
    struct xio_aws_config
    {
        Aws::S3::S3Client client;
        Aws::String bucket;
    };

    template <class C>
    class xio_aws_handler
    {
    public:
        using io_config = xio_aws_config;

        xio_aws_handler();

        template <class E>
        void write(const xexpression<E>& expression, const std::string& path, xfile_dirty dirty);

        template <class ET>
        void read(ET& array, const std::string& path);

        void configure(const C& format_config, const xio_aws_config& io_config);
        void configure_io(const xio_aws_config& io_config);

    private:
        template <class E>
        void write(const xexpression<E>& expression, const char* path, xfile_dirty dirty);

        template <class ET>
        void read(ET& array, const char* path);

        C m_format_config;
        Aws::S3::S3Client m_client;
        Aws::String m_bucket;
    };

    template <class C>
    xio_aws_handler<C>::xio_aws_handler()
    {
    }

    template <class C>
    template <class E>
    inline void xio_aws_handler<C>::write(const xexpression<E>& expression, const std::string& path, xfile_dirty dirty)
    {
        write(expression, path.c_str(), dirty);
    }

    template <class C>
    template <class E>
    inline void xio_aws_handler<C>::write(const xexpression<E>& expression, const char* path, xfile_dirty dirty)
    {
        if (m_format_config.will_dump(dirty))
        {
            Aws::String path2 = path;
            Aws::S3::Model::PutObjectRequest request;
            request.SetBucket(m_bucket);
            request.SetKey(path2);

            std::shared_ptr<Aws::IOStream> writer = Aws::MakeShared<Aws::FStream>("SampleAllocationTag", path, std::ios_base::in | std::ios_base::binary);
            auto s = xostream_wrapper(*writer);
            dump_file(s, expression, m_format_config);

            request.SetBody(writer);

            Aws::S3::Model::PutObjectOutcome outcome = m_client.PutObject(request);

            if (!outcome.IsSuccess())
            {
                auto err = outcome.GetError();
                XTENSOR_THROW(std::runtime_error, std::string("Error: PutObject: ") + err.GetExceptionName().c_str() + ": " + err.GetMessage().c_str());
            }
        }
    }

    template <class C>
    template <class ET>
    inline void xio_aws_handler<C>::read(ET& array, const std::string& path)
    {
        read(array, path.c_str());
    }

    template <class C>
    template <class ET>
    inline void xio_aws_handler<C>::read(ET& array, const char* path)
    {
        Aws::String path2 = path;
        Aws::S3::Model::GetObjectRequest request;
        request.SetBucket(m_bucket);
        request.SetKey(path2);

        Aws::S3::Model::GetObjectOutcome outcome = m_client.GetObject(request);

        if (!outcome.IsSuccess())
        {
            auto err = outcome.GetError();
            XTENSOR_THROW(std::runtime_error, std::string("Error: GetObject: ") + err.GetExceptionName().c_str() + ": " + err.GetMessage().c_str());
        }

        auto& reader = outcome.GetResultWithOwnership().GetBody();
        auto s = xistream_wrapper(reader);
        load_file<ET>(s, array, m_format_config);
    }

    template <class C>
    inline void xio_aws_handler<C>::configure(const C& format_config, const xio_aws_config& io_config)
    {
        m_format_config = format_config;
        m_client = io_config.client;
        m_bucket = io_config.bucket;
    }

    template <class C>
    inline void xio_aws_handler<C>::configure_io(const xio_aws_config& io_config)
    {
        m_client = io_config.client;
        m_bucket = io_config.bucket;
    }

}

#endif

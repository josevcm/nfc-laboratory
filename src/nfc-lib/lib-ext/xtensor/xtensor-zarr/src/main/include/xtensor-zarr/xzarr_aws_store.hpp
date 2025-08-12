/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_AWS_STORE_HPP
#define XTENSOR_ZARR_AWS_STORE_HPP

#include <iomanip>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "xtensor-io/xio_aws_handler.hpp"
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/Object.h>
#include "xzarr_common.hpp"

namespace xt
{
    class xzarr_aws_stream
    {
    public:
        xzarr_aws_stream(const Aws::String& path, const Aws::String& bucket,  const Aws::S3::S3Client& client);
        operator std::string() const;
        xzarr_aws_stream& operator=(const std::vector<char>& value);
        xzarr_aws_stream& operator=(const std::string& value);

    private:
        void assign(const char* value, std::size_t size);

        Aws::String m_path;
        Aws::String m_bucket;
        const Aws::S3::S3Client& m_client;
    };

    class xzarr_aws_store
    {
    public:
        template <class C>
        using io_handler = xio_aws_handler<C>;

        xzarr_aws_store(const std::string& root, const Aws::S3::S3Client& client);
        xzarr_aws_stream operator[](const std::string& key) const;
        void set(const std::string& key, const std::vector<char>& value);
        void set(const std::string& key, const std::string& value);
        std::string get(const std::string& key) const;
        void list_dir(const std::string& prefix, std::vector<std::string>& keys, std::vector<std::string>& prefixes) const;
        std::vector<std::string> list() const;
        std::vector<std::string> list_prefix(const std::string& prefix) const;
        void erase(const std::string& key);
        void erase_prefix(const std::string& prefix);
        const std::string& get_root() const;
        xio_aws_config get_io_config() const;

    private:
        std::string m_root;
        Aws::String m_bucket;
        const Aws::S3::S3Client& m_client;
    };

    /***********************************
     * xzarr_aws_stream implementation *
     ***********************************/

    xzarr_aws_stream::xzarr_aws_stream(const Aws::String& path, const Aws::String& bucket, const Aws::S3::S3Client& client)
        : m_path(path)
        , m_bucket(bucket)
        , m_client(client)
    {
    }

    xzarr_aws_stream::operator std::string() const
    {
        Aws::S3::Model::GetObjectRequest object_request;
        object_request.SetBucket(m_bucket);
        object_request.SetKey(m_path);

        Aws::S3::Model::GetObjectOutcome outcome = m_client.GetObject(object_request);

        if (!outcome.IsSuccess())
        {
            auto err = outcome.GetError();
            XTENSOR_THROW(std::runtime_error, std::string("Error: GetObject: ") + err.GetExceptionName().c_str() + ": " + err.GetMessage().c_str());
        }

        auto& reader = outcome.GetResultWithOwnership().GetBody();
        std::string bytes(std::istreambuf_iterator<char>(reader), {});

        return bytes;
    }

    xzarr_aws_stream& xzarr_aws_stream::operator=(const std::vector<char>& value)
    {
        assign(value.data(), value.size());
        return *this;
    }

    xzarr_aws_stream& xzarr_aws_stream::operator=(const std::string& value)
    {
        assign(value.c_str(), value.size());
        return *this;
    }

    void xzarr_aws_stream::assign(const char* value, std::size_t size)
    {
        Aws::S3::Model::PutObjectRequest request;
        request.SetBucket(m_bucket);
        request.SetKey(m_path);

        std::shared_ptr<Aws::IOStream> writer = Aws::MakeShared<Aws::FStream>("SampleAllocationTag", m_path.c_str(), std::ios_base::in | std::ios_base::binary);
        writer->write(value, static_cast<std::streamsize>(size));
        writer->flush();

        request.SetBody(writer);

        Aws::S3::Model::PutObjectOutcome outcome = m_client.PutObject(request);

        if (!outcome.IsSuccess())
        {
            auto err = outcome.GetError();
            XTENSOR_THROW(std::runtime_error, std::string("Error: PutObject: ") + err.GetExceptionName().c_str() + ": " + err.GetMessage().c_str());
        }
    }

    /**********************************
     * xzarr_aws_store implementation *
     **********************************/

    xzarr_aws_store::xzarr_aws_store(const std::string& root, const Aws::S3::S3Client& client)
        : m_root(root)
        , m_client(client)
    {
        if (m_root.empty())
        {
            XTENSOR_THROW(std::runtime_error, "Root directory cannot be empty");
        }
        std::size_t i = m_root.find('/');
        if (i == std::string::npos)
        {
            m_bucket = m_root.c_str();
            m_root = "";
        }
        else
        {
            m_bucket = m_root.substr(0, i).c_str();
            m_root = m_root.substr(i + 1);
        }
        while (m_root.back() == '/')
        {
            m_root.pop_back();
        }
    }

    xzarr_aws_stream xzarr_aws_store::operator[](const std::string& key) const
    {
        std::string key2 = ensure_startswith_slash(key);
        return xzarr_aws_stream((m_root + key2).c_str(), m_bucket, m_client);
    }

    void xzarr_aws_store::set(const std::string& key, const std::vector<char>& value)
    {
        std::string key2 = ensure_startswith_slash(key);
        xzarr_aws_stream((m_root + key2).c_str(), m_bucket, m_client) = value;
    }

    void xzarr_aws_store::set(const std::string& key, const std::string& value)
    {
        std::string key2 = ensure_startswith_slash(key);
        xzarr_aws_stream((m_root + key2).c_str(), m_bucket, m_client) = value;
    }

    std::string xzarr_aws_store::get(const std::string& key) const
    {
        std::string key2 = ensure_startswith_slash(key);
        return xzarr_aws_stream((m_root + key2).c_str(), m_bucket, m_client);
    }

    void xzarr_aws_store::list_dir(const std::string& prefix, std::vector<std::string>& keys, std::vector<std::string>& prefixes) const
    {
        std::string full_prefix = prefix;
        if (!m_root.empty())
        {
            std::string prefix2 = ensure_startswith_slash(prefix);
            full_prefix = m_root + prefix2;
        }
        Aws::S3::Model::ListObjectsRequest request;
        request.WithBucket(m_bucket).WithPrefix(full_prefix.c_str());
        auto outcome = m_client.ListObjects(request);
        if (!outcome.IsSuccess())
        {
            auto err = outcome.GetError();
            XTENSOR_THROW(std::runtime_error, std::string("Error: ListObjects: ") + err.GetExceptionName().c_str() + ": " + err.GetMessage().c_str());
        }
        Aws::Vector<Aws::S3::Model::Object> objects = outcome.GetResult().GetContents();

        for (Aws::S3::Model::Object& object: objects)
        {
            auto key = object.GetKey();
            std::size_t i = key.find('/');
            if (i == std::string::npos)
            {
                keys.push_back(key.c_str());
            }
            else
            {
                key = key.substr(0, i + 1);
                if (prefixes.empty())
                {
                    prefixes.push_back(key.c_str());
                }
                else
                {
                    if (prefixes.back() != key.c_str())
                    {
                        prefixes.push_back(key.c_str());
                    }
                }
            }
        }
    }

    std::vector<std::string> xzarr_aws_store::list() const
    {
        return list_prefix("");
    }

    std::vector<std::string> xzarr_aws_store::list_prefix(const std::string& prefix) const
    {
        std::string full_prefix = prefix;
        if (!m_root.empty())
        {
            std::string prefix2 = ensure_startswith_slash(prefix);
            full_prefix = m_root + prefix2;
        }
        Aws::S3::Model::ListObjectsRequest request;
        request.WithBucket(m_bucket).WithPrefix(full_prefix.c_str());
        auto outcome = m_client.ListObjects(request);
        if (!outcome.IsSuccess())
        {
            auto err = outcome.GetError();
            XTENSOR_THROW(std::runtime_error, std::string("Error: ListObjects: ") + err.GetExceptionName().c_str() + ": " + err.GetMessage().c_str());
        }
        Aws::Vector<Aws::S3::Model::Object> objects = outcome.GetResult().GetContents();

        std::vector<std::string> keys(objects.size());
        std::transform(objects.begin(), objects.end(), keys.begin(), [](const auto& k) { return k.GetKey().c_str(); });
        return keys;
    }

    void xzarr_aws_store::erase(const std::string& key)
    {
        Aws::S3::Model::DeleteObjectRequest request;
        request.WithKey((m_root + '/' + key).c_str()).WithBucket(m_bucket);
        Aws::S3::Model::DeleteObjectOutcome outcome = m_client.DeleteObject(request);
        if (!outcome.IsSuccess())
        {
            auto err = outcome.GetError();
            XTENSOR_THROW(std::runtime_error, std::string("Error: DeleteObject: ") + err.GetExceptionName().c_str() + ": " + err.GetMessage().c_str());
        }
    }

    void xzarr_aws_store::erase_prefix(const std::string& prefix)
    {
        for (const auto& key: list_prefix(prefix))
        {
            erase(key);
        }
    }

    const std::string& xzarr_aws_store::get_root() const
    {
        return m_root;
    }

    xio_aws_config xzarr_aws_store::get_io_config() const
    {
        xio_aws_config c = {m_client, m_bucket};
        return c;
    }

}

#endif

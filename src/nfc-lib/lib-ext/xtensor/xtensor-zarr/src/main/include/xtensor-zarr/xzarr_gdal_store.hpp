/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_GDAL_STORE_HPP
#define XTENSOR_ZARR_GDAL_STORE_HPP

#include <iomanip>
#include <fstream>
#include <vector>
#include <string>

#include "xtensor-io/xio_gdal_handler.hpp"
#include "cpl_vsi.h"
#include "cpl_string.h"

namespace xt
{
    class xzarr_gdal_stream
    {
    public:
        xzarr_gdal_stream(const std::string& path);
        operator std::string() const;
        void operator=(const std::vector<char>& value);
        void operator=(const std::string& value);
        void erase();
        bool exists();

    private:
        void assign(const char* value, std::size_t size);

        std::string m_path;
    };

    /**
     * @class xzarr_gdal_store
     * @brief Zarr store handler for a GDAL Virtual File System.
     *
     * The xzarr_gdal_store class implements a handler to a Zarr store,
     * and supports the read, write and list operations.
     *
     * @sa xzarr_hierarchy
     */
    class xzarr_gdal_store
    {
    public:
        template <class C>
        using io_handler = xio_gdal_handler<C>;

        xzarr_gdal_store(const std::string& root);
        xzarr_gdal_stream operator[](const std::string& key);
        void list_dir(const std::string& prefix, std::vector<std::string>& keys, std::vector<std::string>& prefixes);
        std::vector<std::string> list();
        std::vector<std::string> list_prefix(const std::string& prefix);
        void erase(const std::string& key);
        void erase_prefix(const std::string& prefix);
        void set(const std::string& key, const std::vector<char>& value);
        void set(const std::string& key, const std::string& value);
        std::string get(const std::string& key);

        xio_gdal_config get_io_config();
        std::string get_root();

    private:
        std::string m_root;
    };

    /************************************
     * xzarr_gdal_stream implementation *
     ************************************/

    inline xzarr_gdal_stream::xzarr_gdal_stream(const std::string& path)
        : m_path(path)
    {
        if (path.substr(0, 4) != "/vsi")
        {
            XTENSOR_THROW(std::runtime_error, "Path must start with /vsi: " + path);
        }
    }

    inline void xzarr_gdal_stream::erase()
    {
        VSIUnlink(m_path.c_str());
    }

    inline bool xzarr_gdal_stream::exists()
    {
        VSIStatBufL sStat;
        return VSIStatL(m_path.c_str(), &sStat) == 0;
    }

    inline xzarr_gdal_stream::operator std::string() const
    {
        std::string bytes;
        VSILFILE* pfile = VSIFOpenL(m_path.c_str(), "rb");
        if (pfile == NULL)
        {
            XTENSOR_THROW(std::runtime_error, "Could not read file: " + m_path);
        }
        auto file = xvsilfile_wrapper(pfile);
        file.read_all(bytes);
        return bytes;
    }

    inline void xzarr_gdal_stream::operator=(const std::vector<char>& value)
    {
        assign(value.data(), value.size());
    }

    inline void xzarr_gdal_stream::operator=(const std::string& value)
    {
        assign(value.c_str(), value.size());
    }

    inline void xzarr_gdal_stream::assign(const char* value, std::size_t size)
    {
        // maybe create directories
        std::size_t j = m_path.find('/', 1) + 1;
        std::size_t i = m_path.rfind('/');
        if ((i != std::string::npos) && (i != j))
        {
            std::string directory = m_path.substr(0, i);
            VSIStatBufL sStat;
            if (VSIStatL(directory.c_str(), &sStat) == 0) // exists
            {
                if (!VSI_ISDIR(sStat.st_mode)) // not a directory
                {
                    XTENSOR_THROW(std::runtime_error, "Path is not a directory: " + directory);
                }
            }
            else
            {
                VSIMkdirRecursive(directory.c_str(), 0755);
            }
        }
        VSILFILE* pfile = VSIFOpenL(m_path.c_str(), "wb");
        if (pfile == NULL)
        {
            XTENSOR_THROW(std::runtime_error, "Could not write file: " + m_path);
        }
        auto file = xvsilfile_wrapper(pfile);
        file.write(value, size);
        file.flush();
    }

    /***********************************
     * xzarr_gdal_store implementation *
     ***********************************/

    inline xzarr_gdal_store::xzarr_gdal_store(const std::string& root)
        : m_root(root)
    {
        if (m_root.empty())
        {
            XTENSOR_THROW(std::runtime_error, "Root directory cannot be empty");
        }
        while (m_root.back() == '/')
        {
            m_root.pop_back();
        }
    }

    inline xzarr_gdal_stream xzarr_gdal_store::operator[](const std::string& key)
    {
        return xzarr_gdal_stream(m_root + '/' + key);
    }

    inline void xzarr_gdal_store::set(const std::string& key, const std::vector<char>& value)
    {
        xzarr_gdal_stream(m_root + '/' + key) = value;
    }

    /**
     * Store a (key, value) pair.
     * @param key the key
     * @param value the value
     */
    inline void xzarr_gdal_store::set(const std::string& key, const std::string& value)
    {
        xzarr_gdal_stream(m_root + '/' + key) = value;
    }

    /**
     * Retrieve the value associated with a given key.
     * @param key the key to get the value from
     *
     * @return returns the value for the given key.
     */
    inline std::string xzarr_gdal_store::get(const std::string& key)
    {
        return xzarr_gdal_stream(m_root + '/' + key);
    }

    inline std::string xzarr_gdal_store::get_root()
    {
        return m_root;
    }

    inline xio_gdal_config xzarr_gdal_store::get_io_config()
    {
        xio_gdal_config c;
        return c;
    }

    /**
     * Retrieve all keys and prefixes with a given prefix and which do not contain the character “/” after the given prefix.
     *
     * @param prefix the prefix
     * @param keys set of keys to be returned by reference
     * @param prefixes set of prefixes to be returned by reference
     */
    inline void xzarr_gdal_store::list_dir(const std::string& prefix, std::vector<std::string>& keys, std::vector<std::string>& prefixes)
    {
        std::string path = m_root;
        if (!prefix.empty())
        {
            path += '/' + prefix;
        }
        char** names = VSIReadDir(path.c_str());
        if (names == NULL)
        {
            XTENSOR_THROW(std::runtime_error, "Directory does not exist: " + path);
        }
        std::size_t i = 0;
        while (true)
        {
            const char* p = names[i];
            if (p == NULL)
            {
                break;
            }
            VSIStatBufL sStat;
            int tmp = VSIStatL(p, &sStat);
            if (VSI_ISDIR(sStat.st_mode)) // this is a directory
            {
                prefixes.push_back(p);
            }
            else
            {
                keys.push_back(p);
            }
            i++;
        }
        CSLDestroy(names);
    }

    /**
     * Retrieve all keys from the store.
     *
     * @return returns a set of keys.
     */
    inline std::vector<std::string> xzarr_gdal_store::list()
    {
        return list_prefix("");
    }

    /**
     * Retrieve all keys with a given prefix from the store.
     *
     * @param prefix the prefix
     *
     * @return returns a set of keys with a given prefix.
     */
    inline std::vector<std::string> xzarr_gdal_store::list_prefix(const std::string& prefix)
    {
        std::string path = m_root + '/' + prefix;
        std::vector<std::string> keys;
        char** names = VSIReadDirRecursive(path.c_str());
        if (names == NULL)
        {
            XTENSOR_THROW(std::runtime_error, "Directory does not exist: " + path);
        }
        std::size_t i = 0;
        while (true)
        {
            const char* p = names[i];
            if (p == NULL)
            {
                break;
            }
            std::string s = p;
            keys.push_back(s.substr(m_root.size() + 1));
            i++;
        }
        CSLDestroy(names);
        return keys;
    }

    /**
     * Erase the given (key, value) pair from the store.
     * @param key the key
     */
    inline void xzarr_gdal_store::erase(const std::string& key)
    {
        VSIUnlink((m_root + '/' + key).c_str());
    }

    /**
     * Erase all the keys with the given prefix from the store.
     * @param prefix the prefix
     */
    inline void xzarr_gdal_store::erase_prefix(const std::string& prefix)
    {
        VSIRmdirRecursive((m_root + '/' + prefix).c_str());
    }
}

#endif

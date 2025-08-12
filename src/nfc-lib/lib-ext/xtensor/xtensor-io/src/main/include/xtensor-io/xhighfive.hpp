/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_IO_XHIGHFIVE_HPP
#define XTENSOR_IO_XHIGHFIVE_HPP

#include <iostream>
#include <string>
#include <vector>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Easy.hpp>

#include <xtensor/xarray.hpp>
#include <xtensor/xtensor.hpp>

#include "xtensor-io.hpp"

namespace xt
{
    namespace extensions
    {
        /**
        * Check if a path exists (is a Group or DataSet) in an open HDF5 file.
        *
        * @param file opened HighFive::File
        * @param path path of the Group/DataSet
        */
        inline bool exist(const HighFive::File& file, const std::string& path)
        {
            return file.exist(path);
        }

        /**
        * Recursively create groups in an open HDF5 file such that a DataSet can be created.
        * For example if the path = "/path/to/dataset", this function will create the groups
        * "/path" and "/path/to".
        *
        * @param file opened HighFive::File
        * @param path path of the DataSet
        */
        inline void create_group(HighFive::File& file, const std::string& path)
        {
            file.createGroup(path);
        }

        /**
        * Get the size of an existing DataSet in an open HDF5 file.
        *
        * @param file opened HighFive::File
        * @param path path of the DataSet
        *
        * @return size the size of the HighFive::DataSet
        */
        inline std::size_t size(const HighFive::File& file, const std::string& path)
        {
            return H5Easy::getSize(file, path);
        }

        /**
        * Get the shape of an existing DataSet in an open HDF5 file.
        *
        * @param file opened HighFive::File
        * @param path path of the DataSet
        *
        * @return shape the shape of the HighFive::DataSet
        */
        inline std::vector<std::size_t> shape(const HighFive::File& file, const std::string& path)
        {
            return H5Easy::getShape(file, path);
        }
    }

    namespace detail
    {
        inline auto highfive_file_mode(xt::file_mode mode)
        {
            switch (mode)
            {
                case xt::file_mode::create:
                  return HighFive::File::Create;
                case xt::file_mode::overwrite:
                  return HighFive::File::Overwrite;
                case xt::file_mode::append:
                  return HighFive::File::ReadWrite;
                case xt::file_mode::read:
                  return HighFive::File::ReadOnly;
                default:
                  return HighFive::File::ReadOnly;
            }
        }

        inline auto highfive_dump_mode(xt::dump_mode mode)
        {
            switch (mode)
            {
                case xt::dump_mode::create:
                  return H5Easy::DumpMode::Create;
                case xt::dump_mode::overwrite:
                  return H5Easy::DumpMode::Overwrite;
                default:
                  return H5Easy::DumpMode::Create;
            }
        }
    }

    /**
    * Write scalar/string to a new DataSet in an open HDF5 file.
    *
    * @param file opened HighFive::File (has to be writeable)
    * @param path path of the DataSet
    * @param data the data to write
    * @param dmode DataSet-write mode (xt::dump_mode::create | xt::dump_mode::overwrite)
    *
    * @return dataset the newly created HighFive::DataSet (e.g. to add an attribute)
    */
    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const T& data,
                                  xt::dump_mode dmode=xt::dump_mode::create)
    {
        return H5Easy::dump(file, path, data, detail::highfive_dump_mode(dmode));
    }

    /**
    * Write a scalar to a (new, extendible) DataSet in an open HDF5 file.
    *
    * @param file opened HighFive::File (has to be writeable)
    * @param path path of the DataSet
    * @param data the data to write
    * @param idx the indices to which to write
    *
    * @return dataset the (newly created) HighFive::DataSet (e.g. to add an attribute)
    */
    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const T& data,
                                  const std::vector<std::size_t>& idx)
    {
        return H5Easy::dump(file, path, data, idx);
    }

    /**
    * Load entry "{i,...}" from a DataSet in an open HDF5 file to a scalar.
    *
    * @param file opened HighFive::File (has to be writeable)
    * @param idx the indices to load
    * @param path path of the DataSet
    *
    * @return data the read data
    */
    template <class T>
    inline auto load(const HighFive::File& file, const std::string& path, const std::vector<std::size_t>& idx)
    {
        return H5Easy::load<T>(file, path, idx);
    }

    /**
    * Load a DataSet in an open HDF5 file to an object (templated).
    *
    * @param file opened HighFive::File (has to be writeable)
    * @param path path of the DataSet
    *
    * @return data the read data
    */
    template <class T>
    inline auto load(const HighFive::File& file, const std::string& path)
    {
        return H5Easy::load<T>(file, path);
    }

    /**
    * Write field to a new DataSet in an HDF5 file.
    *
    * @param file opened HighFive::File (has to be writeable)
    * @param path path of the DataSet
    * @param data the data to write
    * @param dmode DataSet-write mode (xt::dump_mode::create | xt::dump_mode::overwrite)
    *
    * @return dataset the newly created HighFive::DataSet (e.g. to add an attribute)
    */
    template <class T>
    inline void dump_hdf5(const std::string& fname, const std::string& path, const T& data,
                          xt::file_mode fmode=xt::file_mode::create,
                          xt::dump_mode dmode=xt::dump_mode::create)
    {
        HighFive::File file(fname, detail::highfive_file_mode(fmode));

        xt::dump(file, path, data, dmode);
    }

    /**
    * Load a DataSet in an open HDF5 file to an object (templated).
    *
    * @param file opened HighFive::File (has to be writeable)
    * @param path path of the DataSet
    *
    * @return data the read data
    */
    template <class T>
    inline auto load_hdf5(const std::string& fname, const std::string& path)
    {
        HighFive::File file(fname, detail::highfive_file_mode(xt::file_mode::read));

        return xt::load<T>(file, path);
    }
}

#endif

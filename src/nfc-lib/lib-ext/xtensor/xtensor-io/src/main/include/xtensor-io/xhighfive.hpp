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
#include <highfive/H5DataType.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>

#include <xtensor/xarray.hpp>
#include <xtensor/xtensor.hpp>

#include "xtensor-io.hpp"

namespace xt
{
    namespace detail
    {
        inline auto error(const HighFive::File& file, const std::string& path, std::string message)
        {
            message += "\n";
            message += "path: '" + path + "'\n";
            message += "filename: '" + file.getName() + "'\n";

            return std::runtime_error(message);
        }
    }

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
            // find first "/"
            std::size_t idx = path.find("/");

            // loop over all groups
            while (true)
            {
                // terminate if all "/" have been found
                if (std::string::npos == idx)
                {
                    break;
                }

                // create group if needed
                if (idx > 0)
                {
                    // get group name
                    std::string name(path.substr(0,idx));
                    // create if needed
                    if (!file.exist(name))
                    {
                        return false;
                    }
                }

                // proceed to next "/"
                idx = path.find("/",idx+1);
            }

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
            // find first "/"
            std::size_t idx = path.find("/");

            // loop over all groups
            while (true)
            {
                // terminate if all "/" have been found
                if (std::string::npos == idx)
                {
                    return;
                }

                // create group if needed
                if (idx > 0)
                {
                    // get group name
                    std::string name(path.substr(0,idx));

                    // create if needed
                    if (!file.exist(name))
                    {
                        file.createGroup(name);
                    }
                }

                // proceed to next "/"
                idx = path.find("/",idx+1);
            }
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
            if (!exist(file, path))
            {
                throw detail::error(file, path, "xt::extensions::size: Field does not exist");
            }

            HighFive::DataSet dataset = file.getDataSet(path);

            auto dataspace = dataset.getSpace();

            auto dims = dataspace.getDimensions();

            std::size_t size = 1;

            for (std::size_t i = 0; i < dims.size(); ++i)
            {
                size *= dims[i];
            }

            return size;
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
            if (!exist(file, path))
            {
                throw detail::error(file, path, "xt::extensions::shape: Field does not exist");
            }

            HighFive::DataSet dataset = file.getDataSet(path);

            auto dataspace = dataset.getSpace();

            auto dims = dataspace.getDimensions();

            std::vector<std::size_t> shape(dims.size());

            for (std::size_t i = 0; i < dims.size(); ++i)
            {
                shape[i] = dims[i];
            }

            return shape;
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

        namespace type
        {
            namespace scalar
            {
                template <class T, class E = void>
                struct dump_impl
                {
                    template <class C>
                    static HighFive::DataSet run(HighFive::File& file, const std::string& path, const C& data)
                    {
                        extensions::create_group(file, path);

                        HighFive::DataSet dataset = file.createDataSet<C>(path, HighFive::DataSpace::From(data));

                        dataset.write(data);

                        file.flush();

                        return dataset;
                    }
                };

                template <class T, class E = void>
                struct overwrite_impl
                {
                    template <class C>
                    static HighFive::DataSet run(HighFive::File& file, const std::string& path, const C& data)
                    {
                        HighFive::DataSet dataset = file.getDataSet(path);

                        auto dataspace = dataset.getSpace();

                        auto dims = dataspace.getDimensions();

                        if (dims.size() != 0)
                        {
                            throw detail::error(file, path, "xt::dump: Existing field not a scalar");
                        }

                        dataset.write(data);

                        file.flush();

                        return dataset;
                    }
                };

                template <class T, class E = void>
                struct dump_extend_impl
                {
                    template <class C>
                    static HighFive::DataSet run(HighFive::File& file, const std::string& path,
                        const C& data, const std::vector<std::size_t>& idx)
                    {
                        std::vector<std::size_t> ones(idx.size(), 1);

                        if (extensions::exist(file, path))
                        {
                            HighFive::DataSet dataset = file.getDataSet(path);

                            auto dataspace = dataset.getSpace();

                            auto dims = dataspace.getDimensions();

                            auto shape = dims;

                            if (dims.size() != idx.size())
                            {
                                throw detail::error(file, path, "xt::dump: Rank of the index and the existing field do not match");
                            }

                            for (std::size_t i = 0; i < dims.size(); ++i)
                            {
                                shape[i] = std::max(dims[i], idx[i]+1);
                            }

                            if (shape != dims)
                            {
                                dataset.resize(shape);
                            }

                            dataset.select(idx, ones).write(data);

                            file.flush();

                            return dataset;
                        }

                        extensions::create_group(file, path);

                        auto shape = idx;

                        const size_t unlim = HighFive::DataSpace::UNLIMITED;
                        std::vector<std::size_t> unlim_shape(idx.size(), unlim);

                        std::vector<hsize_t> chuncks(idx.size(), 10);

                        for (auto& i: shape)
                        {
                            i++;
                        }

                        HighFive::DataSpace dataspace = HighFive::DataSpace(shape, unlim_shape);

                        HighFive::DataSetCreateProps props;
                        props.add(HighFive::Chunking(chuncks));

                        HighFive::DataSet dataset = file.createDataSet(path, dataspace, HighFive::AtomicType<T>(), props);

                        dataset.select(idx, ones).write(data);

                        file.flush();

                        return dataset;
                    }
                };

                template <class T>
                struct load_impl
                {
                    static auto run(const HighFive::File& file, const std::string& path, const std::vector<std::size_t>& idx)
                    {
                        std::vector<std::size_t> ones(idx.size(), 1);

                        HighFive::DataSet dataset = file.getDataSet(path);

                        T data;

                        dataset.select(idx, ones).read(data);

                        return data;
                    }
                };
            }

            namespace vector
            {
                template <class T, class E = void>
                struct dump_impl
                {
                    template <class C>
                    static HighFive::DataSet run(HighFive::File& file, const std::string& path, const std::vector<C>& data)
                    {
                        extensions::create_group(file, path);

                        HighFive::DataSet dataset = file.createDataSet<T>(path, HighFive::DataSpace::From(data));

                        dataset.write(data);

                        file.flush();

                        return dataset;
                    }
                };

                template <class T, class E = void>
                struct overwrite_impl
                {
                    template <class C>
                    static HighFive::DataSet run(HighFive::File& file, const std::string& path, const C& data)
                    {
                        HighFive::DataSet dataset = file.getDataSet(path);

                        auto dataspace = dataset.getSpace();

                        auto dims = dataspace.getDimensions();

                        if (dims.size() > 1)
                        {
                            throw detail::error(file, path, "xt::dump: Can only overwrite 1-d vectors");
                        }

                        if (dims[0] != data.size())
                        {
                            throw detail::error(file, path, "xt::dump: Inconsistent dimensions");
                        }

                        dataset.write(data);

                        file.flush();

                        return dataset;
                    }
                };
            }

            namespace xtensor
            {
                template<class T>
                struct dump_impl
                {
                    template <class C>
                    static HighFive::DataSet run(HighFive::File& file, const std::string& path, const C& data)
                    {
                        extensions::create_group(file, path);

                        std::vector<std::size_t> dims(data.shape().cbegin(), data.shape().cend());

                        HighFive::DataSet dataset = file.createDataSet<typename C::value_type>(path, HighFive::DataSpace(dims));

                        dataset.write(data.begin());

                        file.flush();

                        return dataset;
                    }
                };

                template<class T>
                struct overwrite_impl
                {
                    template <class C>
                    static HighFive::DataSet run(HighFive::File& file, const std::string& path, const C& data)
                    {
                        HighFive::DataSet dataset = file.getDataSet(path);

                        auto dataspace = dataset.getSpace();

                        auto dims = dataspace.getDimensions();

                        if (data.shape().size() != dims.size())
                        {
                            throw detail::error(file, path, "xt::dump: Inconsistent rank");
                        }

                        for (std::size_t i = 0; i < data.shape().size(); ++i)
                        {
                            if (data.shape()[i] != dims[i])
                            {
                                throw detail::error(file, path, "xt::dump: Inconsistent dimensions");
                            }
                        }

                        dataset.write(data.begin());

                        file.flush();

                        return dataset;
                    }
                };

                template <class T>
                struct load_impl
                {
                    static auto run(const HighFive::File& file, const std::string& path)
                    {
                        HighFive::DataSet dataset = file.getDataSet(path);

                        auto dataspace = dataset.getSpace();

                        auto dims = dataspace.getDimensions();

                        T data = T::from_shape(dims);

                        dataset.read(data.data());

                        return data;
                    }
                };
            }
        } // namespace type

        // scalar/string
        template <class T, class E = void>
        struct load_impl
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                HighFive::DataSet dataset = file.getDataSet(path);

                auto dataspace = dataset.getSpace();

                auto dims = dataspace.getDimensions();

                if (dims.size() != 0)
                {
                    throw detail::error(file, path, "xt::load: Field not a scalar");
                }

                T data;

                dataset.read(data);

                return data;
            }
        };

        template <class T>
        struct load_impl<std::vector<T>>
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                HighFive::DataSet dataset = file.getDataSet(path);

                auto dataspace = dataset.getSpace();

                auto dims = dataspace.getDimensions();

                if (dims.size() != 1)
                {
                    throw detail::error(file, path, "xt::load: Field not rank 1");
                }

                std::vector<T> data;

                dataset.read(data);

                return data;
            }
        };

        template <class T>
        struct load_impl<xt::xarray<T>>
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                return detail::type::xtensor::load_impl<xt::xarray<T>>::run(file, path);
            }
        };

        template <class T, std::size_t rank>
        struct load_impl<xt::xtensor<T,rank>>
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                return detail::type::xtensor::load_impl<xt::xtensor<T,rank>>::run(file, path);
            }
        };
    }

    /**
    * Write "xt::xarray<T>" to a new DataSet in an open HDF5 file.
    *
    * @param file opened HighFive::File (has to be writeable)
    * @param path path of the DataSet
    * @param data the data to write
    * @param dmode DataSet-write mode (xt::dump_mode::create | xt::dump_mode::overwrite)
    *
    * @return dataset the newly created HighFive::DataSet (e.g. to add an attribute)
    */
    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const xt::xarray<T>& data, xt::dump_mode dmode=xt::dump_mode::create)
    {
        if ((dmode == xt::dump_mode::create) or (dmode == xt::dump_mode::overwrite and !extensions::exist(file, path)))
        {
            return detail::type::xtensor::dump_impl<xt::xarray<T>>::run(file, path, data);
        }

        return detail::type::xtensor::overwrite_impl<xt::xarray<T>>::run(file, path, data);
    }

    /**
    * Write "xt::xtensor<T,rank>" to a new DataSet in an open HDF5 file.
    *
    * @param file opened HighFive::File (has to be writeable)
    * @param path path of the DataSet
    * @param data the data to write
    * @param dmode DataSet-write mode (xt::dump_mode::create | xt::dump_mode::overwrite)
    *
    * @return dataset the newly created HighFive::DataSet (e.g. to add an attribute)
    */
    template <class T, std::size_t rank>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const xt::xtensor<T,rank>& data, xt::dump_mode dmode=xt::dump_mode::create)
    {
        if ((dmode == xt::dump_mode::create) or (dmode == xt::dump_mode::overwrite and !extensions::exist(file, path)))
        {
            return detail::type::xtensor::dump_impl<xt::xtensor<T,rank>>::run(file, path, data);
        }

        return detail::type::xtensor::overwrite_impl<xt::xtensor<T,rank>>::run(file, path, data);
    }

    /**
    * Write "std::vector<T>" to a new DataSet in an open HDF5 file.
    *
    * @param file opened HighFive::File (has to be writeable)
    * @param path path of the DataSet
    * @param data the data to write
    * @param dmode DataSet-write mode (xt::dump_mode::create | xt::dump_mode::overwrite)
    *
    * @return dataset the newly created HighFive::DataSet (e.g. to add an attribute)
    */
    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const std::vector<T>& data, xt::dump_mode dmode=xt::dump_mode::create)
    {
        if ((dmode == xt::dump_mode::create) or (dmode == xt::dump_mode::overwrite and !extensions::exist(file, path)))
        {
            return detail::type::vector::dump_impl<T>::run(file, path, data);
        }

        return detail::type::vector::overwrite_impl<T>::run(file, path, data);
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
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const T& data, xt::dump_mode dmode=xt::dump_mode::create)
    {
        if ((dmode == xt::dump_mode::create) or (dmode == xt::dump_mode::overwrite and !extensions::exist(file, path)))
        {
            return detail::type::scalar::dump_impl<T>::run(file, path, data);
        }

        return detail::type::scalar::overwrite_impl<T>::run(file, path, data);
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
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const T& data, const std::vector<std::size_t>& idx)
    {
        return detail::type::scalar::dump_extend_impl<T>::run(file, path, data, idx);
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
        return detail::type::scalar::load_impl<T>::run(file, path, idx);
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
        return detail::load_impl<T>::run(file, path);
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
    inline void dump_hdf5(const std::string& fname, const std::string& path, const T& data, xt::file_mode fmode=xt::file_mode::create, xt::dump_mode dmode=xt::dump_mode::create)
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

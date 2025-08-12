/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
* Copyright (c) Andrew Hardin                                              *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_IO_XGDAL_HPP
#define XTENSOR_IO_XGDAL_HPP

#include <functional>
#include <map>

#include <xtensor/xtensor.hpp>
#include "xtensor-io.hpp"

// We rely exclusively on the stable C interface to GDAL.
#include <gdal.h>
#include <cpl_string.h>

namespace xt
{

    /**
     * The GDAL IO module supports reading and writing arrays with arbitrary index order
     * (e.g. [band,row,col]). The component enumeration contains tags for each dimension.
     */
    enum class component
    {
        band,
        row,
        column
    };

    using layout = std::array<component, 3>;

    /**
     * Get a band sequential layout; index order = [band, row, column].
     */
    inline layout layout_band_sequential()
    {
        return {component::band, component::row, component::column};
    }

    /**
     * Get a band interleaved by pixel layout; index order = [row, column, band].
     */
    inline layout layout_band_interleaved_pixel()
    {
        return {component::row, component::column, component::band};
    }

    /**
     * Get a band interleaved by line layout; index order = [row, band, column].
     */
    inline layout layout_band_interleaved_line()
    {
        return {component::row, component::band, component::column};
    }

    namespace detail
    {

        /** A type trait for converting a template type to GDALDataType value.
         */
        template<typename T>
        struct to_gdal_type;
#define XT_GDAL_DEFINE_TYPE(from, to) \
          template<> struct to_gdal_type<from> { \
              static const GDALDataType value = GDALDataType::to; \
          };
        XT_GDAL_DEFINE_TYPE(int8_t, GDT_Byte)
        XT_GDAL_DEFINE_TYPE(uint8_t, GDT_Byte)
        XT_GDAL_DEFINE_TYPE(uint16_t, GDT_UInt16)
        XT_GDAL_DEFINE_TYPE(int16_t, GDT_Int16)
        XT_GDAL_DEFINE_TYPE(uint32_t, GDT_UInt32)
        XT_GDAL_DEFINE_TYPE(int32_t, GDT_Int32)
        XT_GDAL_DEFINE_TYPE(float, GDT_Float32)
        XT_GDAL_DEFINE_TYPE(double, GDT_Float64)
#undef XT_GDAL_DEFINE_TYPE


        /**
         * A utility structure that records the spacing of pixel values (in bytes).
         * Used to provide arguments to GDALRasterIO.
         */
        struct space
        {
            /**
             * From GDAL documentation:
             * The byte offset from the start of one pixel value to the start of the next pixel value within a scanline.
             * If defaulted (0) the size of the datatype eBufType is used.
             */
            GSpacing pixel;
            /**
             * From GDAL documentation:
             * The byte offset from the start of one scanline in pData to the start of the next.
             * If defaulted (0) the size of the datatype eBufType * nBufXSize is used.
             */
            GSpacing line;
            /**
             * From GDAL documentation:
             * The byte offset from the start of one bands data to the start of the next.
             * If defaulted (0) the value will be nLineSpace * nBufYSize implying band sequential organization of the data buffer.
             */
            GSpacing band;
        };


        /**
         * Check that the given layout is valid.
         * @param item the layout to check.
         * @return false in the layout has duplicate entries (e.g. {band, band, row}).
         */
        static bool valid_layout(layout item)
        {
            return
                    item[0] != item[1] &&
                    item[0] != item[2] &&
                    item[1] != item[2];
        }

        /**
         * Utility structure that stores the basic raster shape {band, rows, columns}.
         */
        template<typename T>
        struct raster_shape
        {
            T band_count;
            T ny;
            T nx;

            template<typename U>
            raster_shape<U> cast() const {
                return { static_cast<U>(band_count), static_cast<U>(ny), static_cast<U>(nx) };
            }
        };

        /**
         * Convert standard raster dimensions into an xt shape.
         * @param item the layout that should be converted into a shape.
         * @param shape the raster dimensions.
         * @return an xt shape.
         */
        std::array<size_t, 3> layout_as_shape(layout item, raster_shape<size_t> shape)
        {
            std::map<component, size_t> dimmap
                {
                    {component::band,   shape.band_count},
                    {component::row,    shape.ny},
                    {component::column, shape.nx}
                };
            return {
                    dimmap[item[0]],
                    dimmap[item[1]],
                    dimmap[item[2]]
            };
        }

        /**
         * Convert an xtensor shape into a raster shape (mapping the shape{...} into [bands, rows, columns]).
         * @tparam T the output type
         * @tparam U the native shape type
         * @param item the layout of the input shape.
         * @param shape the shape of the xtensor
         * @param out the raster shape
         * @return true if the conversion was supported.
         */
        template<typename T, typename U>
        bool shape_as_raster_shape(layout item, U shape, raster_shape<T>& out)
        {
            auto size = shape.size();
            if(size == 2)
            {
                // This is a convenience hook that lets us support 2D shapes.
                // The layout still contains a band, but we ignore it and just look
                // at whether columns or rows come first.
                out = {
                    T(1),
                    T(shape[0]),
                    T(shape[1])
                };
                if(item[0] == component::column || item[1] == component::column)
                {
                    std::swap(out.nx, out.ny);
                }
                return true;
            }
            if(size == 3)
            {
                std::map<component, T> dimmap;
                for(size_t i = 0; i < 3; i++)
                {
                    dimmap[item[i]] = T(shape[i]);
                }
                out = {
                    dimmap[component::band],
                    dimmap[component::row],
                    dimmap[component::column]
                };
                return true;
            }

            // Not 2 or 3 dimensions - no idea how to map that into raster dimensions.
            return false;
        }

        /**
         * Take the standard raster dimensions and convert them into byte level strides for GDALRasterIO().
         * @param item the layout to be converted into spacing.
         * @param dim the dimensions of the raster.
         * @param pixel_byte_count number of bytes in one pixel.
         * @return spacing in bytes.
         */
        static space
        layout_as_space(layout item, raster_shape<GSpacing> dim, GSpacing pixel_byte_count)
        {
            // Unpack to reduce verbosity.
            GSpacing nx = dim.nx, ny = dim.ny, band_count = dim.band_count;

            // All 6 cases are enumerated.
            // Perhaps there's a cleaner way to write this method?
            space ans{};
            if (item[0] == component::band)
            {
                if (item[1] == component::row)
                {
                    // {b, y, x }
                    ans.band = nx * ny;
                    ans.line = nx;
                    ans.pixel = 1;
                }
                else
                {
                    // {b, x, y}
                    ans.band = nx * ny;
                    ans.line = 1;
                    ans.pixel = ny;
                }
            }
            else if (item[1] == component::band)
            {
                if (item[0] == component::row)
                {
                    // {y, b, x}
                    ans.pixel = 1;
                    ans.line = nx * band_count;
                    ans.band = nx;
                }
                else
                {
                    // {x, b, y}
                    ans.pixel = ny * band_count;
                    ans.line = 1;
                    ans.band = ny;
                }
            }
            else
            {
                if (item[0] == component::row)
                {
                    // {y, x, b}
                    ans.pixel = band_count;
                    ans.line = band_count * nx;
                    ans.band = 1;
                }
                else
                {
                    // {x, y, b}
                    ans.pixel = band_count * ny;
                    ans.line = band_count;
                    ans.band = 1;
                }
            }
            // Convert from pixel -> bytes.
            ans.pixel *= pixel_byte_count;
            ans.band *= pixel_byte_count;
            ans.line *= pixel_byte_count;
            return ans;
        }
    }  // namespace detail

    /**
     * Options for loading a GDAL dataset.
     */
    struct load_gdal_options
    {
        /**
         * Load every band and return band-sequential memory (index order = [band, row, column]).
         */
        load_gdal_options()
                : interleave(layout_band_sequential()),
                  bands_to_load(/*load every available band */),
                  error_handler([](const std::string &msg)
                                { throw std::runtime_error("load_gdal(): " + msg); })
        {}

        /**
         * The desired layout of the returned tensor - defaults to band sequential([band, row, col]).
         * Arbitrary index order is supported, like band interleaved by pixel ([row, col, band]).
         */
        layout interleave;

        /**
         * The indices of bands to read from the dataset. All bands are read if empty.
         * @remark Per GDAL convention, these indices start at @em one (not zero).
         */
        std::vector<int> bands_to_load;

        /**
         * The error handler used to report errors (like file missing or a read fails).
         * By default, a std::runtime_error is thrown when an error is encountered
         */
        std::function<void(const std::string &)> error_handler;
    };

    /**
     * Options for dumping a GDAL dataset.
     */
    struct dump_gdal_options
    {
        /**
         * Write a band sequential xarray using the GeoTIFF driver.
         */
        dump_gdal_options()
            : mode(dump_mode::create),
              driver_name("GTiff"),
              creation_options(),
              interleave(layout_band_sequential()),
              return_opened_dataset(false),
              error_handler([](const std::string& msg)
                            { throw std::runtime_error("dump_gdal(): " + msg); })
        {}

        /**
         * Whether the file should be created or overwritten.
         */
        dump_mode mode;

        /**
         * The name of the GDAL driver to use (like GTiff).
         */
        std::string driver_name;

        /**
         * Options passed to GDAL when the dataset is created (like COMPRESS=JPEG).
         */
        std::vector<std::string> creation_options;

        /**
         * The layout of the xarray that's dumped to disk (such as [band, row, column]).
         */
        layout interleave;

        /**
         * Whether or not the dump method should return the opened dataset.
         * If true, the open dataset will be returned - the user must later call GDALClose().
         * @remark this is useful for writing metadata to the dataset after the pixels have been written.
         */
        bool return_opened_dataset;

        /**
         * The error handler used to report errors (like failed to open dataset).
         * By default, a std::runtime_error is thrown when an error is encountered.
         */
        std::function<void(const std::string &)> error_handler;
    };


    /**
     * Load pixels from a GDAL dataset.
     * @tparam T the type of pixels to return. If it doesn't match the content of the raster,
     *           GDAL will silently cast to this data type (which may cause loss of precision).
     * @param file_path the file to open.
     * @param options options to use when loading.
     * @return pixels; band sequential by default, but index order is controlled by options.
     */
    template<typename T>
    xtensor<T, 3> load_gdal(const std::string &file_path, load_gdal_options options = {})
    {
        auto dataset = GDALOpen(file_path.c_str(), GDALAccess::GA_ReadOnly);
        if (!dataset)
        {
            options.error_handler("error opening GDAL dataset '" + file_path + "'.");
            return {};
        }
        auto ans = load_gdal < T > (dataset, std::move(options));
        GDALClose(dataset);
        return ans;
    }

    /**
     * Load pixels from an opened GDAL dataset.
     * @tparam T in type of pixels to return. If this doesn't match the content of the dataset,
     *           GDAL will silently cast to this data type (which may cause loss of precision).
     * @param dataset an opened GDAL dataset.
     * @param options options to used when loading.
     * @return pixels; band sequential by default, but index order is controlled by options.
     */
    template<typename T>
    xtensor<T, 3> load_gdal(GDALDatasetH dataset, load_gdal_options options = {})
    {
        // Check if the user provided bands. If not, we'll load every band in the dataset.
        auto &bands = options.bands_to_load;
        if (bands.empty())
        {
            int count = GDALGetRasterCount(dataset);
            bands.resize(static_cast<size_t>(count));
            std::iota(bands.begin(), bands.end(), 1);
        }

        // Pull dataset dimensions.
        detail::raster_shape<int> gdal_dim
        {
            static_cast<int>(bands.size()),
            GDALGetRasterYSize(dataset),
            GDALGetRasterXSize(dataset)
        };

        // Setup the shape and pixel spacing based on the user provided layout.
        if (!detail::valid_layout(options.interleave))
        {
            options.error_handler("the given interleave option has duplicate entries");
            return {};
        }
        auto shape = detail::layout_as_shape(options.interleave, gdal_dim.cast<size_t>());
        auto spacing = detail::layout_as_space(options.interleave, gdal_dim.cast<GSpacing>(), sizeof(T));
        xt::xtensor<T, 3> ans(shape);

        // Read from the dataset. Again, there are some hurdles related
        // to the arbitrary layout order.
        auto error = GDALDatasetRasterIOEx(
            dataset,
            GDALRWFlag::GF_Read,
            0, 0,
            gdal_dim.nx, gdal_dim.ny,
            ans.data(),
            gdal_dim.nx, gdal_dim.ny,
            detail::to_gdal_type<T>::value,
            gdal_dim.band_count,
            bands.data(),
            spacing.pixel,
            spacing.line,
            spacing.band,
            nullptr);
        if (error != CPLErr::CE_None)
        {
            options.error_handler("failed to read from dataset");
            return {};
        }
        return ans;
    }

    /**
     * Dump a 2D or 3D xexpression to a GDALDataset.
     * @param e data to dump; must evaluate to a 2D or 3D shape.
     * @param path where to save the data.
     * @param options options to use when dumping.
     * @return nullptr by default, or, an open dataset if dump_gdal_options.return_opened_dataset (user must close).
     */
    template<typename T>
    GDALDatasetH dump_gdal(const xexpression<T> &e, const std::string& path, dump_gdal_options options = {})
    {
        // Get the requested driver.
        auto driver = GDALGetDriverByName(options.driver_name.c_str());
        if(driver == nullptr)
        {
            options.error_handler("failed to find driver '" + options.driver_name + "'");
            return nullptr;
        }

        // Evaluate the expression and convert its shape into a raster shape (bands, rows, and columns).
        // Convert its shape into raster dimensions.
        auto&& de = xt::eval(e.derived_cast());
        auto shape = de.shape();
        detail::raster_shape<int> gdal_dim{};
        if (!detail::valid_layout(options.interleave))
        {
            options.error_handler("the given interleave option has duplicate entries");
            return nullptr;
        }
        if(!detail::shape_as_raster_shape(options.interleave, shape, gdal_dim))
        {
            options.error_handler("failed to convert the shape into a count of the number of bands, rows, and columns");
            return nullptr;
        }

        // Take the creation options and munge them into something that GDAL accepts.
        // This is a C structure that'll need to be manually destroyed.
        char** create_options_c = nullptr;
        for(auto &i : options.creation_options)
        {
            create_options_c = CSLAddString(create_options_c, i.c_str());
        }

        // Convert the expressions value into a GDALDataType.
        // If this fails during compilation, there isn't a (known) conversion.
        using value_type = typename std::decay_t<decltype(de)>::value_type;
        GDALDataType raster_type = detail::to_gdal_type<value_type>::value;

        // Create the dataset.
        auto dataset = GDALCreate(driver, path.c_str(), gdal_dim.nx, gdal_dim.ny, gdal_dim.band_count, raster_type, create_options_c);
        CSLDestroy(create_options_c);
        if(dataset == nullptr)
        {
            options.error_handler("failed to create a " + options.driver_name + " dataset at '" + path + "'");
            return nullptr;
        }

        // Embark on RasterIO.
        auto spacing = detail::layout_as_space(options.interleave, gdal_dim.cast<GSpacing>(), sizeof(value_type));
        auto error = GDALDatasetRasterIOEx(dataset, GDALRWFlag::GF_Write,
                                           0, 0,
                                           gdal_dim.nx, gdal_dim.ny,
                                           const_cast<value_type*>(de.data()), // pinky promise not to modify.
                                           gdal_dim.nx, gdal_dim.ny,
                                           raster_type,
                                           gdal_dim.band_count,
                                           nullptr,
                                           spacing.pixel, spacing.line, spacing.band,
                                           nullptr);

        // Close the dataset (if not requested to stay open).
        // Then check the error and (maybe) throw if the rasterio failed.
        if(!options.return_opened_dataset)
        {
            GDALClose(dataset);
            dataset = nullptr;
        }
        if(error != CPLErr::CE_None)
        {
            options.error_handler("rasterio failed on '" + path + "'");
            return nullptr;
        }
        return dataset;
    }

}  // namespace xt


#endif // XTENSOR_IO_XGDAL_HPP

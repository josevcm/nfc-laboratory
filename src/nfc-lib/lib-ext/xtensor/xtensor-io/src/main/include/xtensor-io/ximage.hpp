/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_IO_XIMAGE_HPP
#define XTENSOR_IO_XIMAGE_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <memory>

#include <xtensor/xarray.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xeval.hpp>

#include "xtensor_io_config.hpp"

#ifdef __CLING__
    #pragma clang diagnostic push
    // silencing warnings from OpenEXR 2.2.0 / OpenImageIO
    #pragma clang diagnostic ignored "-Wdeprecated-register"
    #pragma cling load("OpenImageIO")
#endif

#include <OpenImageIO/imageio.h>

#ifdef __CLING__
    #pragma clang diagnostic pop
#endif

namespace xt
{
    /**
     * Load an image from file at filename.
     * Storage format is deduced from file ending.
     *
     * @param filename The path of the file to load
     *
     * @return xarray with image contents. The shape of the xarray
     *         is ``HEIGHT x WIDTH x CHANNELS`` of the loaded image, where
     *         ``CHANNELS`` are ordered in standard ``R G B (A)``.
     */
    template <class T = unsigned char>
    xarray<T> load_image(std::string filename)
    {
        auto in(OIIO::ImageInput::open(filename));
        if (!in)
        {
            throw std::runtime_error("load_image(): Error reading image '" + filename + "'.");
        }

        const OIIO::ImageSpec& spec = in->spec();

        // allocate memory
        auto image = xarray<T>::from_shape({static_cast<std::size_t>(spec.height),
                                            static_cast<std::size_t>(spec.width),
                                            static_cast<std::size_t>(spec.nchannels)});

        in->read_image(OIIO::BaseTypeFromC<T>::value, image.data());

        in->close(); 

        return image;
    }

        /** \brief Pass options to dump_image().
        */
    struct dump_image_options
    {
            /** \brief Initialize options to default values.
            */
        dump_image_options()
        : spec(0,0,0)
        , autoconvert(true)
        {
            spec.attribute("CompressionQuality", 90);
        }

            /** \brief Forward an attribute to an OpenImageIO ImageSpec.

                See the documentation of OIIO::ImageSpec::attribute() for a list
                of supported attributes.

                Default: "CompressionQuality" = 90
            */
        template <class T>
        dump_image_options & attribute(OIIO::string_view name, T const & v)
        {
            spec.attribute(name, v);
            return *this;
        }

        OIIO::ImageSpec spec;
        bool autoconvert;
    };

    /**
     * Save image to disk.
     * The desired image format is deduced from ``filename``.
     * Supported formats are those supported by OpenImageIO.
     * Most common formats are supported (jpg, png, gif, bmp, tiff).
     * The shape of the array must be ``HEIGHT x WIDTH`` or ``HEIGHT x WIDTH x CHANNELS``.
     *
     * @param filename The path to the desired file
     * @param data Image data
     * @param options Pass a dump_image_options object to fine-tune image export
     */
    template <class E>
    void dump_image(std::string filename, const xexpression<E>& data,
                    dump_image_options const & options = dump_image_options())
    {
        using value_type = typename std::decay_t<decltype(data.derived_cast())>::value_type;

        auto shape = data.derived_cast().shape();
        XTENSOR_PRECONDITION(shape.size() == 2 || shape.size() == 3,
            "dump_image(): data must have 2 or 3 dimensions (channels must be last).");

        auto out(OIIO::ImageOutput::create(filename)); 
        if (!out)
        {
            throw std::runtime_error("dump_image(): Error opening file '" + filename + "' to write image.");
        }

        OIIO::ImageSpec spec = options.spec;

        spec.width     = static_cast<int>(shape[1]);
        spec.height    = static_cast<int>(shape[0]);
        spec.nchannels = (shape.size() == 2)
                           ? 1
                           : static_cast<int>(shape[2]);
        spec.format    = OIIO::BaseTypeFromC<value_type>::value;

        out->open(filename, spec);

        auto && ex = eval(data.derived_cast());
        if(out->spec().format != OIIO::BaseTypeFromC<value_type>::value)
        {
            // OpenImageIO changed the target type because the file format doesn't support value_type.
            // It will do automatic conversion, but the data should be in the range 0...1
            // for good results.
            auto mM = minmax(ex)();

            if(mM[0] != mM[1])
            {
                using real_t = xtl::real_promote_type_t<value_type>;
                auto && normalized = eval((real_t(1.0) / (mM[1] - mM[0])) * (ex - mM[0]));
                out->write_image(OIIO::BaseTypeFromC<real_t>::value, normalized.data());
            }
            else
            {
                out->write_image(OIIO::BaseTypeFromC<value_type>::value, ex.data());
            }
        }
        else
        {
            out->write_image(OIIO::BaseTypeFromC<value_type>::value, ex.data());
        }
        out->close();
    }
}

#endif

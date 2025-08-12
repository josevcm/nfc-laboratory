/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZARR_CONFIG_HPP
#define XTENSOR_ZARR_CONFIG_HPP

#ifdef _WIN32
    #ifdef XTENSOR_ZARR_STATIC_LIB
        #define XTENSOR_ZARR_API
    #else
        #ifdef XTENSOR_ZARR_EXPORTS
            #define XTENSOR_ZARR_API __declspec(dllexport)
        #else
            #define XTENSOR_ZARR_API __declspec(dllimport)
        #endif
    #endif
#else
    #define XTENSOR_ZARR_API
#endif

// Project version
#define XTENSOR_ZARR_VERSION_MAJOR 0
#define XTENSOR_ZARR_VERSION_MINOR 0
#define XTENSOR_ZARR_VERSION_PATCH 7

// Binary version
#define XTENSOR_ZARR_BINARY_CURRENT 0
#define XTENSOR_ZARR_BINARY_REVISION 0
#define XTENSOR_ZARR_BINARY_AGE 0

#endif

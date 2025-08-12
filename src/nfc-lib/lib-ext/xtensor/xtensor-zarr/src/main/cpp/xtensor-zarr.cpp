#include "xtensor-zarr/xzarr_hierarchy.hpp"

namespace xt
{
template XTENSOR_ZARR_API void xzarr_register_compressor<xzarr_file_system_store, xio_gzip_config>();
template XTENSOR_ZARR_API void xzarr_register_compressor<xzarr_file_system_store, xio_zlib_config>();
template XTENSOR_ZARR_API void xzarr_register_compressor<xzarr_file_system_store, xio_blosc_config>();
template class XTENSOR_ZARR_API xchunked_array_factory<xzarr_file_system_store>;
}
#include "xtensor-io/xio_gzip.hpp"
#include "xtensor-zarr/xzarr_compressor.hpp"
#include "xtensor-zarr/xzarr_file_system_store.hpp"
#include "xtensor-zarr/xtensor_zarr_config.hpp"

namespace xt {
template XTENSOR_ZARR_API void xzarr_register_compressor<xzarr_file_system_store, xio_gzip_config>();
}

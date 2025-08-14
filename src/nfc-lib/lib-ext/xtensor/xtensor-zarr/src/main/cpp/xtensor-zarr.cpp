#include "xtensor-zarr/xzarr_chunked_array.hpp"
#include "xtensor-zarr/xzarr_file_system_store.hpp"
#include "xtensor-zarr/xtensor_zarr_config.hpp"

namespace xt {
template class XTENSOR_ZARR_API xchunked_array_factory<xzarr_file_system_store>;
}

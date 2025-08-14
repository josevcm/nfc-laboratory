# z5

[![Anaconda-Server Badge](https://anaconda.org/conda-forge/z5py/badges/version.svg)](https://anaconda.org/conda-forge/z5py)
[![Build Status](https://github.com/constantinpape/z5/workflows/build/badge.svg)](https://github.com/constantinpape/z5/actions)
[![Documentation Status](https://readthedocs.org/projects/z5/badge/?version=latest)](https://z5.readthedocs.io/en/latest/?badge=latest)
[![DOI](https://zenodo.org/badge/101700504.svg)](https://zenodo.org/badge/latestdoi/101700504)



C++ and Python wrapper for [zarr](https://github.com/zarr-developers/zarr-python) and [n5](https://github.com/saalfeldlab/n5) file formats.
Implements the file system specification of these formats. Implementations for cloud based storage are work in progress. Any
help is highly appreciated. See issues [#136](https://github.com/constantinpape/z5/issues/136) and [#137](https://github.com/constantinpape/z5/issues/137) for details.

Support for the following compression codecs:
- [Blosc](https://github.com/Blosc/c-blosc)
- [Zlib / Gzip](https://zlib.net/)
- [Bzip2](http://www.bzip.org/)
- [XZ](https://tukaani.org/xz/)
- [LZ4](https://github.com/lz4/lz4)

## Installation

### Conda

Conda packages for the relevant systems and python versions are hosted on conda-forge:

```
$ conda install -c conda-forge z5py
```

### From Source

The easiest way to build the library from source is using a conda-environment with all necessary dependencies.
You can find the conda environment files for build environments in `.environments/unix`

To set up the conda environment and install the package on unix:

```bash
$ conda env create -f environments/unix/z5-dev.yaml
$ conda activate z5-dev
$ mkdir bld
$ cd bld
$ cmake -DWITH_ZLIB=ON -DWITH_BZIP2=ON -DCMAKE_INSTALL_PREFIX=/path/to/install ..
$ make install
```

Note that in the CMakeLists.txt, we try to infer the active conda-environment automatically.
If this fails, you can set it manually via `-DCMAKE_PREFIX_PATH=/path/to/conda-env`.
To specify where to install the package, set:

- `CMAKE_INSTALL_PREFIX`: where to install the C++ headers
- `PYTHON_MODULE_INSTALL_DIR`: where to install the python package (set to `site-packages` of active conda env by default)

If you want to include z5 in another C++ project, note that the library itself is header-only. However, you need to link against the compression codecs that you use.

If you don't want to use conda for dependency management, the following dependencies are necessary:
- [xtensor](https://github.com/xtensor-stack/xtensor)
- [nlohmann_json](https://github.com/nlohmann/json)
- [pybind11](https://github.com/pybind/pybind11) (only for python bindings)
- [xtensor_python](https://github.com/xtensor-stack/xtensor-python) (only for python bindings)

## Examples / Usage

### Python

The Python API is very similar to `h5py`.
Some differences are: 
- The constructor of `File` takes the boolean argument `use_zarr_format`, which determines whether
the zarr or N5 format is used (if set to `None`, an attempt is made to automatically infer the format).
- There is no need to close `File`, hence the `with` block isn't necessary (but supported).
- Linked datasets (`my_file['new_ds'] = my_file['old_ds']`) are not supported
- Broadcasting is only supported for scalars in `Dataset.__setitem__`
- Arbitrary leading and trailing singleton dimensions can be added/removed/rolled through in `Dataset.__setitem__`
- Compatibility of exception handling is a goal, but not necessarily guaranteed.
- Because zarr/N5 are usually used with large data, `z5py` compresses blocks by default where `h5py` does not. The default compressors are
  - zarr: `"blosc"`
  - n5: `"gzip"`

Some examples:

```python
import z5py
import numpy as np

# create a file and a dataset
f = z5py.File('array.zr', use_zarr_format=True)
ds = f.create_dataset('data', shape=(1000, 1000), chunks=(100, 100), dtype='float32')

# write array to a roi
x = np.random.random_sample(size=(500, 500)).astype('float32')
ds[:500, :500] = x

# broadcast a scalar to a roi
ds[500:, 500:] = 42.

# read array from a roi
y = ds[250:750, 250:750]

# create a group and create a dataset in the group
g = f.create_group('local_group')
g.create_dataset('local_data', shape=(100, 100), chunks=(10, 10), dtype='uint32')

# open dataset from group or file
ds_local1 = f['local_group/local_data']
ds_local2 = g['local_data']

# read and write attributes
attributes = ds.attrs
attributes['foo'] = 'bar'
baz = attributes['foo']
```

There are convenience functions to convert n5 and zarr files to and from hdf5 or tif.
Additional data formats will follow.

```python
# convert existing h5 file to n5
# this only works if h5py is available
from z5py.converter import convert_from_h5

h5_file = '/path/to/file.h5'
n5_file = '/path/to/file.n5'
h5_key = n5_key = 'data'
target_chunks = (64, 64, 64)
n_threads = 8

convert_from_h5(h5_file, n5_file,
                in_path_in_file=h5_key,
                out_path_in_file=n5_key,
                chunks=target_chunks,
                n_threads=n_threads,
                compression='gzip')
```

### C++

`Z5` aims to supports different storage implementations. The default is to use the filesystem, implementations to also supports AWS-S3 and Google Cloud Storage are work in progress.
The API implements factory functions like `createFile` or `createDataset` in [the factory header](https://github.com/constantinpape/z5/blob/master/include/z5/factory.hxx). 
These functions need to be called with the corresponding handle, like `z5::filesystem::handle::File` or `z5::s3::handle::File` in order to specify which backend to use.

The library is intended to be used with a multiarray, that holds data in memory.
By default [xtensor](https://github.com/QuantStack/xtensor) is used, see [implementation](https://github.com/constantinpape/z5/blob/master/include/z5/multiarray/xtensor_access.hxx).
There also exists an interface for [marray](https://github.com/bjoern-andres/marray), see [implementation](https://github.com/constantinpape/z5/blob/master/include/z5/multiarray/marray_access.hxx).
To interface with other multiarray implementation, reimplement `readSubarray` and `writeSubarray`.
Pull requests for additional multiarray support are welcome.

Some examples:

```c++
#include "json.hpp"
#include "xtensor/xarray.hpp"

// factory functions to create files, groups and datasets
#include "z5/factory.hxx"
// handles for z5 filesystem objects
#include "z5/filesystem/handle.hxx"
// io for xtensor multi-arrays
#include "z5/multiarray/xtensor_access.hxx"
// attribute functionality
#include "z5/attributes.hxx"

int main() {

  // get handle to a File on the filesystem
  z5::filesystem::handle::File f("data.zr");
  // if you wanted to use a different backend, for example AWS, you
  // would need to use this instead:
  // z5::s3::handle::File f;

  // create the file in zarr format
  const bool createAsZarr = true;
  z5::createFile(f, createAsZarr);

  // create a new zarr dataset
  const std::string dsName = "data";
  std::vector<size_t> shape = { 1000, 1000, 1000 };
  std::vector<size_t> chunks = { 100, 100, 100 };
  auto ds = z5::createDataset(f, dsName, "float32", shape, chunks);

  // write array to roi
  z5::types::ShapeType offset1 = { 50, 100, 150 };
  xt::xarray<float>::shape_type shape1 = { 150, 200, 100 };
  xt::xarray<float> array1(shape1, 42.0);
  z5::multiarray::writeSubarray<float>(ds, array1, offset1.begin());

  // read array from roi (values that were not written before are filled with a fill-value)
  z5::types::ShapeType offset2 = { 100, 100, 100 };
  xt::xarray<float>::shape_type shape2 = { 300, 200, 75 };
  xt::xarray<float> array2(shape2);
  z5::multiarray::readSubarray<float>(ds, array2, offset2.begin());

  // get handle for the dataset
  const auto dsHandle = z5::filesystem::handle::Dataset(f, dsName);

  // read and write json attributes
  nlohmann::json attributesIn;
  attributesIn["bar"] = "foo";
  attributesIn["pi"] = 3.141593;
  z5::writeAttributes(dsHandle, attributesIn);

  nlohmann::json attributesOut;
  z5::readAttributes(dsHandle, attributesOut);
  
  return 0;
}
```

### C

There are external efforts to implement a C-Api wrapper for z5.
You can check it out [here](https://github.com/kmpaul/cz5test).

### R

There exists a prototype by @gdkrmr to provide [R bindings for z5](https://github.com/gdkrmr/zarr-R).
It is still in an early stage, but looks very promising.

## Citation

If you use this library in your research, please cite it via the associated DOI:
```
@misc{pape_z5_2019,
  doi = {10.5281/ZENODO.3585752},
  url = {https://zenodo.org/record/3585752},
  author = {Pape,  Constantin},
  title = {constantinpape/z5},
  publisher = {Zenodo},
  year = {2019}
}
```

## When to use this library?

This library implements the zarr and n5 data specification in C++ and Python.
Use it, if you need access to these formats from these languages.
Zarr / n5 have native implementations in Python / Java.
If you only need access in the respective native language,
it is recommended to use these implementations, which are more thoroughly tested.


## Current Limitations / TODOs

- No thread / process synchronization -> writing to the same chunk in parallel will lead to undefined behavior.
- Supports only little endianness and C-order for the zarr format.


## A note on axis ordering

Internally, n5 uses column-major (i.e. x, y, z) axis ordering, while z5 uses row-major (i.e. z, y, x).
While this is mostly handled internally, it means that the metadata does not transfer
1 to 1, but needs to be reversed for most shapes. Concretely:

|           |n5                      |z5              |
|----------:|-----------------------:|---------------:|  
|Shape      | s_x, s_y, s_z          |s_z, s_y, s_x   |
|Chunk-Shape| c_x, c_y, c_z          |c_z, c_y, c_x   | 
|Chunk-Ids  | i_x, i_y, i_z          |i_z, i_y, i_x   |

# ![xtensor-io](docs/source/xtensor-io.svg)

[![Azure Pipelines](https://dev.azure.com/xtensor-stack/xtensor-stack/_apis/build/status/xtensor-stack.xtensor-io?branchName=master)](https://dev.azure.com/xtensor-stack/xtensor-stack/_build/latest?definitionId=8&branchName=master)
[![ReadTheDocs](https://readthedocs.org/projects/xtensor-io/badge/?version=stable)](http://xtensor-io.readthedocs.io/en/stable/)
[![Binder](https://img.shields.io/badge/launch-binder-brightgreen.svg)](https://mybinder.org/v2/gh/xtensor-stack/xtensor-io/stable?filepath=notebooks%2Fdemo.ipynb)
[![Join the Gitter Chat](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/QuantStack/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Reading and writing image, sound and npz file formats to and from xtensor data structures. <br>
[Try it live in the browser!](https://mybinder.org/v2/gh/xtensor-stack/xtensor-io/stable?filepath=notebooks%2Fdemo.ipynb)

## Introduction

`xtensor-io` offers an API to read and write various file formats into `xtensor` data structures:

 - images,
 - audio files,
 - NumPy's compressed storage format (NPZ),
 - HDF5 files,
 - Blosc.

`xtensor-io` wraps the [OpenImageIO](https://github.com/OpenImageIO/oiio), [libsndfile](https://github.com/erikd/libsndfile), [zlib](https://github.com/madler/zlib), [HighFive](https://github.com/BlueBrain/HighFive), and [blosc](https://github.com/Blosc/c-blosc) libraries.

## Installation

`xtensor-io` is a header-only library. We provide a package for the mamba (or conda) package manager.

```bash
mamba install xtensor-io -c conda-forge
```

- `xtensor-io` depends on `xtensor` `^0.23.0`.

- `OpenImageIO`, `libsndfile`, `zlib`, `HighFive`, and `blosc` are optional dependencies to `xtensor-io`

  - `OpenImageIO` is required to read and write image files.
  - `libsndfile` is required to read and write sound files.
  - `zlib` is required to load NPZ files.
  - `HighFive` (and the `HDF5` library) is required to read and write HDF5 files.
  - `blosc` is required to read and write Blosc files.

All five libraries are available for the conda package manager.

You can also install `xtensor-io` from source:

```
mkdir build
cd build
cmake ..
make
sudo make install
```

## Trying it online

To try out xtensor-io interactively in your web browser, just click on the binder
link:

[![Binder](binder-logo.svg)](https://mybinder.org/v2/gh/xtensor-stack/xtensor-io/stable?filepath=notebooks%2Fdemo.ipynb)

## Documentation

To get started with using `xtensor-io`, check out the full documentation

http://xtensor-io.readthedocs.io/

## Usage

```cpp
// loads png image into xarray with shape HEIGHT x WIDTH x CHANNELS
auto img_arr = xt::load_image("test.png");

// write xarray out to JPEG image
xt::dump_image("dumptest.jpg", img_arr + 5);

// load npz file containing multiple arrays
auto npy_map = xt::load_npz("test.npz");

auto arr_0 = npy_map["arr_0"].cast<double>();
auto arr_1 = npy_map["arr_1"].cast<unsigned long>();

// open a wav file
auto audio = xt::load_audio("files/xtensor.wav");
auto& audio_arr = std::get<1>(audio); // audio contents (like scipy.io.wavfile results)

// save a sine wave sound
int freq = 2000;
int sampling_freq = 44100;
double duration = 1.0;

auto t = xt::arange(0.0, duration, 1.0 / sampling_freq);
auto y = xt::sin(2.0 * xt::numeric_constants<double>::PI * freq * t);

xt::dump_audio("files/sine.wav", y, sampling_freq);
```

## License

We use a shared copyright model that enables all contributors to maintain the
copyright on their contributions.

This software is licensed under the BSD-3-Clause license. See the [LICENSE](LICENSE) file for details.

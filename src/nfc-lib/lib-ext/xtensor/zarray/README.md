[![Azure Pipelines](https://dev.azure.com/xtensor-stack/xtensor-stack/_apis/build/status/xtensor-stack.zarray?branchName=master)](https://dev.azure.com/xtensor-stack/xtensor-stack/_build/latest?definitionId=10&branchName=master)
[![Join the Gitter Chat](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/QuantStack/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Dynamically typed N-D expression system based on [xtensor](thhps://github.com/xtensor-stack/xtensor).

## Introduction

`zarray` is a dynamically typed N-D expression system built on top of [xtensor](thhps://github.com/xtensor-stack/xtensor).

## Installation

### Package managers

We provide a package for the mamba (or conda) package manager:

```bash
mamba install -c conda-forge zarray
```

### Install from sources

`zarray` is a header-only library.

You can directly install it from the sources:

```bash
cmake -D CMAKE_INSTALL_PREFIX=your_install_prefix
make install
```

## Dependencies

`zarray` depends on `xtensor` and `nlohmann_json`:

| `zarray` | `xtensor` | `nlohmann_json` |
|----------|-----------|-----------------|
|  master  |  0.23.8   |  3.2.0          |
|  0.1.0   |  0.23.8   |  3.2.0          |
|  0.0.6   |  0.23.8   |  3.2.0          |

## Usage

**Initialize a 2-D array and compute the sum of one of its rows and a 1-D array.**

```cpp
#include <zarray/zarray.hpp>

xt::zarray arr1 =
  {{1.0, 2.0, 3.0},
   {4.0, 5.0, 6.0},
   {7.0, 8.0, 9.0}};

xt::zarray arr2 =
  {5.0, 6.0, 7.0};

xt::zarray res = xt::strided_view(arr1, {1, xt::all()}) + arr2;
std::cout << res << std::endl;
```

Outputs:

```
{7, 11, 14}
```

**Initialize a 1-D array and reshape it inplace.**

```cpp
#include <zarray/zarray.hpp>

xt::zarray arr =
  {1, 2, 3, 4, 5, 6, 7, 8, 9};

arr.reshape({3, 3});

std::cout << arr << std::endl;
```

Outputs:

```
{{1, 2, 3},
 {4, 5, 6},
 {7, 8, 9}}
```

## License

We use a shared copyright model that enables all contributors to maintain the
copyright on their contributions.

This software is licensed under the BSD-3-Clause license. See the
[LICENSE](LICENSE) file for details.


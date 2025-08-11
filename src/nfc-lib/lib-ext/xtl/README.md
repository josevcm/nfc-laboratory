# ![xtl](docs/source/xtl.svg)

[![Linux](https://github.com/xtensor-stack/xtl/actions/workflows/linux.yml/badge.svg)](https://github.com/xtensor-stack/xtl/actions/workflows/linux.yml)
[![OSX](https://github.com/xtensor-stack/xtl/actions/workflows/osx.yml/badge.svg)](https://github.com/xtensor-stack/xtl/actions/workflows/osx.yml)
[![Windows](https://github.com/xtensor-stack/xtl/actions/workflows/windows.yml/badge.svg)](https://github.com/xtensor-stack/xtl/actions/workflows/windows.yml)
[![Documentation Status](http://readthedocs.org/projects/xtl/badge/?version=latest)](https://xtl.readthedocs.io/en/latest/?badge=latest)
[![Join the Gitter Chat](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/QuantStack/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Basic tools (containers, algorithms) used by other quantstack packages

## Installation

### Package managers

We provide a package for the mamba (or conda) package manager:

```bash
mamba install -c conda-forge xtl
```

### Install from sources

`xtl` is a header-only library.

You can directly install it from the sources:

```bash
cmake -D CMAKE_INSTALL_PREFIX=your_install_prefix
make install
```

## Documentation

To get started with using `xtl`, check out the full documentation

http://xtl.readthedocs.io/


## Building the HTML documentation

xtl's documentation is built with three tools

 - [doxygen](http://www.doxygen.org)
 - [sphinx](http://www.sphinx-doc.org)
 - [breathe](https://breathe.readthedocs.io)

While doxygen must be installed separately, you can install breathe by typing

```bash
pip install breathe
```

Breathe can also be installed with `conda`

```bash
conda install -c conda-forge breathe
```

Finally, build the documentation with

```bash
make html
```

from the `docs` subdirectory.

## License

We use a shared copyright model that enables all contributors to maintain the
copyright on their contributions.

This software is licensed under the BSD-3-Clause license. See the [LICENSE](LICENSE) file for details.

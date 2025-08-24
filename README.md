# libOCTypes

![CI Status](https://github.com/pjgrandinetti/OCTypes/actions/workflows/ci.yml/badge.svg)
[![Documentation Status](https://readthedocs.org/projects/OCTypes/badge/?version=latest)](https://octypes.readthedocs.io/en/latest/?badge=latest)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/pjgrandinetti/OCTypes)](https://github.com/pjgrandinetti/OCTypes/releases/latest)
[![License](https://img.shields.io/github/license/pjgrandinetti/OCTypes)](https://github.com/pjgrandinetti/OCTypes/blob/main/LICENSE)
[![GitHub issues](https://img.shields.io/github/issues/pjgrandinetti/OCTypes)](https://github.com/pjgrandinetti/OCTypes/issues)
[![GitHub last commit](https://img.shields.io/github/last-commit/pjgrandinetti/OCTypes)](https://github.com/pjgrandinetti/OCTypes/commits/main)
[![Platform Support](https://img.shields.io/badge/platform-macOS%20%7C%20Linux%20%7C%20Windows-blue)](https://github.com/pjgrandinetti/OCTypes#requirements)

A lightweight library for doing object oriented programming in C.

## Requirements

Ensure you have installed:

- A C compiler (e.g., clang or gcc)
- Make
- Doxygen
- Python 3 with `sphinx` and `breathe` (`pip install -r docs/requirements.txt`)

## Building the Library

Compile the static library:

```bash
make
```

This produces `libOCTypes.a`.

## Documentation

To build the API docs (after the library is built):

```bash
make docs  # from project root
```

The HTML output appears in `docs/_build/html`.

### Online Documentation

The latest documentation is hosted on Read the Docs: https://OCTypes.readthedocs.io/en/latest/

A Read the Docs configuration file (`.readthedocs.yml`) is included at the project root to automate the documentation build.

## Running the Test Suite

After building the library, run:

```bash
make test        # build and run all tests
make test-debug  # run under LLDB
make test-asan   # with AddressSanitizer
```

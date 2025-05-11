# libOCTypes

![CI Status](https://github.com/YourUsername/OCTypes/actions/workflows/ci.yml/badge.svg)
[![Documentation Status](https://readthedocs.org/projects/octypes/badge/?version=latest)](https://octypes.readthedocs.io/en/latest/?badge=latest)

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

The latest documentation is hosted on Read the Docs: https://octypes.readthedocs.io/en/latest/

A Read the Docs configuration file (`.readthedocs.yml`) is included at the project root to automate the documentation build.

## Running the Test Suite

After building the library, run:

```bash
make test        # build and run all tests
make test-debug  # run under LLDB
make test-asan   # with AddressSanitizer
```


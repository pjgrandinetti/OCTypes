# libOCTypes

![CI Status](https://github.com/YourUsername/OCTypes/actions/workflows/ci.yml/badge.svg)

A lightweight library for doing object oriented programming in C.

## Documentation

The API documentation for OCTypes is built using Doxygen and Sphinx with the Breathe extension. 

To build the documentation locally:

1.  Ensure you have Doxygen, Python, and the necessary Python packages installed. You can install the Python packages using pip:
    ```bash
    pip install -r docs/requirements.txt
    ```
2.  Navigate to the `docs` directory:
    ```bash
    cd docs
    ```
3.  Build the Doxygen XML output and then the Sphinx HTML documentation:
    ```bash
    make html 
    # or, if you don't have a Makefile target yet for the full docs build:
    # doxygen Doxyfile && sphinx-build -b html . _build
    ```
    (Assuming a Doxyfile exists in the `docs` directory or project root that outputs XML to `docs/doxygen/xml`)

The documentation will be available in `docs/_build/html`.

For more details, see the [documentation itself](<link-to-hosted-docs-if-available>).


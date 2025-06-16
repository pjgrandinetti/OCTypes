OCTypes 
=======

A lightweight library for doing object oriented programming in C.
-----------------------------------------------------------------
OCTypes is designed to be portable and should work on any POSIX compliant system. It has been tested on macOS, Linux, and Windows.  It is a C library that provides a subset of object-oriented programming features. It is designed to be lightweight and easy to use, making it a great choice for C programmers who want to take advantage of many object-oriented design patterns without the overhead of a full-fledged C++ compiler.  It includes collections, strings, memory management, and a simple syntax for defining classes and methods. It is inspired by Apple's CoreFoundation, but is not a direct port of it. 

Requirements
~~~~~~~~~~~~~~~

Ensure you have installed:

- A C compiler (e.g., clang or gcc)
- Make
- Doxygen
- Python 3 with ``sphinx`` and ``breathe`` (``pip install -r docs/requirements.txt``)

Building the Library
~~~~~~~~~~~~~~~~~~~~~~~

Compile the static library::

    make

This produces ``libOCTypes.a``.




.. toctree::
   :maxdepth: 2
   :caption: Background

   background

.. toctree::
   :maxdepth: 2
   :caption: API Reference

   api/OCType
   api/OCString
   api/OCBoolean
   api/OCNumber
   api/OCArray
   api/OCSet
   api/OCDictionary
   api/OCData
   api/OCIndexArray
   api/OCIndexSet
   api/OCIndexPairSet
   api/OCAutoreleasePool
   api/OCMath
   api/OCLibrary
   api/OCFileUtilities

Indices and Tables
==================

* :ref:`genindex`
* :ref:`search`

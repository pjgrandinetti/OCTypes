# Changelog

All notable changes to this project will be documented in this file.

## [v0.1.6] - 2025-08-18
### Fixed
- Windows build and install improvements
- GitHub Actions build fixes
- Build system refactoring under build/ directory

## [v0.1.1] - 2025-05-20
### Added
- Created `CHANGELOG.md` to track project releases.
- Fixed numerous bugs in OCString

### Changed
- Parser grammar updated (`explist` and function call rules) to correctly handle nested single-argument functions (e.g., `conj(sqrt(...))`).
- Refactored include guards in `OCComplexParser.h` and `OCMath.h` to prevent multiple definition errors.
- Test suite reorganized: moved `OCComplexFromCString` tests into `test_complex_parser.c`.

### Fixed
- Corrected evaluation logic to use C99 complex math functions (`csqrt`, `cexp`, `ccbrt`, `cpow` for `qtrt`, `conj`, `carg`, `creal`, `cimag`, etc.).
- Resolved static vs. non-static conflicts and argument mismatches for `builtInMathFunction`.

## [v0.1.0] - 2025-05-10
### Initial release
- Basic `OCComplexFromCString`, arithmetic operations, and simple functions support.
- Core modules: `OCComplexParser`, `OCMath`, `OCString`, `OCNumber`, `OCArray`, `OCDictionary`, etc.
- Documentation setup with Sphinx and Doxygen.
- Initial test suite covering string parsing, math operations, and data structures.

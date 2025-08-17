#!/bin/bash

# Linux Debug Script for OCTypes
# Run this in your Linux VM to quickly test all scenarios

set -euo pipefail

echo "=== OCTypes Linux Debug Script ==="
echo "Current commit: $(git rev-parse --short HEAD)"
echo "Current branch: $(git branch --show-current)"
echo

echo "=== System Info ==="
uname -a
echo "Compiler versions:"
clang --version 2>/dev/null || echo "clang not found"
gcc --version 2>/dev/null || echo "gcc not found"
flex --version 2>/dev/null || echo "flex not found"
bison --version 2>/dev/null || echo "bison not found"
echo

echo "=== Test 1: Default build ==="
make clean
make test
echo "✅ Default build passed"
echo

echo "=== Test 2: Clang build ==="
make clean
make test CC="clang"
echo "✅ Clang build passed"
echo

echo "=== Test 3: GCC build ==="
make clean
make test CC="gcc"
echo "✅ GCC build passed"
echo

echo "=== Test 4: Shared library ==="
make clean
make lib/libOCTypes.so
echo "✅ Shared library build passed"
echo

echo "=== Test 5: CI simulation ==="
make -j1 clean
make -j1 test CC="clang"
echo "✅ CI simulation passed"
echo

echo "=== All Linux tests passed! ==="

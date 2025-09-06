#!/bin/bash

# Pre-commit script to format C/C++ files and automatically stage changes
# This prevents the "dirty working directory" issue with clang-format

set -e

# Get list of staged C/C++ files
staged_files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(c|h)$' | grep -v -E '^(build/|third_party/|.*\.tab\.(c|h)|.*Scanner\.c|src/cJSON)' || true)

if [ -z "$staged_files" ]; then
    echo "No C/C++ files to format"
    exit 0
fi

echo "Formatting C/C++ files with clang-format..."

# Format each file
for file in $staged_files; do
    if [ -f "$file" ]; then
        echo "Formatting: $file"
        clang-format --style=file --fallback-style=LLVM -i "$file"
        # Re-stage the file after formatting
        git add "$file"
    fi
done

echo "Formatting complete and changes staged"
exit 0

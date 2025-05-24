cmake_minimum_required(VERSION 3.15)
project(OCTypes C)
set(CMAKE_C_STANDARD 11)

# Find Flex and Bison for scanner/parser generation
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

# Generate parser and scanner sources into the build directory
BISON_TARGET(OCComplexParser src/OCComplexParser.y
    ${CMAKE_CURRENT_BINARY_DIR}/OCComplexParser.tab.c
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/OCComplexParser.tab.h
    COMPILE_FLAGS "-y -d"
)
FLEX_TARGET(OCComplexScanner src/OCComplexScanner.l
    ${CMAKE_CURRENT_BINARY_DIR}/OCComplexScanner.c
)
ADD_FLEX_BISON_DEPENDENCY(OCComplexScanner OCComplexParser)

# Gather static sources (exclude any existing generated files)
file(GLOB STATIC_SOURCES "src/*.c")
list(REMOVE_ITEM STATIC_SOURCES
    "src/OCComplexScanner.c"
    "src/OCComplexParser.tab.c"
)

# Combine all sources: generated parser/scanner + static modules
set(ALL_SOURCES
    ${BISON_OCComplexParser_OUTPUTS}
    ${FLEX_OCComplexScanner_OUTPUTS}
    ${STATIC_SOURCES}
)

# Build the static library
add_library(OCTypes STATIC ${ALL_SOURCES})

# Public include dirs: source and generated headers location
target_include_directories(OCTypes PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Tests
enable_testing()
file(GLOB TEST_SOURCES "tests/*.c")
add_executable(runTests ${TEST_SOURCES})
target_include_directories(runTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(runTests PRIVATE OCTypes m)
add_test(NAME runTests COMMAND runTests)

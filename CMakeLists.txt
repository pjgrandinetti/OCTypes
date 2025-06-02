cmake_minimum_required(VERSION 3.15)
project(OCTypes C)
set(CMAKE_C_STANDARD 11)

# -------------------------------------------------------------------
# 1) Find Flex and Bison for scanner/parser generation
# -------------------------------------------------------------------
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

# -------------------------------------------------------------------
# 2) Generate parser and scanner sources into the build directory
# -------------------------------------------------------------------
BISON_TARGET(OCComplexParser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCComplexParser.y
    ${CMAKE_CURRENT_BINARY_DIR}/OCComplexParser.tab.c
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/OCComplexParser.tab.h
    COMPILE_FLAGS "-y -d"
)

FLEX_TARGET(OCComplexScanner
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCComplexScanner.l
    ${CMAKE_CURRENT_BINARY_DIR}/OCComplexScanner.c
)

ADD_FLEX_BISON_DEPENDENCY(OCComplexScanner OCComplexParser)

# -------------------------------------------------------------------
# 3) Define public headers explicitly
# -------------------------------------------------------------------
set(OCTYPE_HDR
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCArray.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCAutoreleasePool.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCBoolean.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCDictionary.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCLibrary.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCMath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCNumber.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCString.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OCType.h
)

# -------------------------------------------------------------------
# 4) Gather static sources (excluding generated ones)
# -------------------------------------------------------------------
file(GLOB STATIC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM STATIC_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OCComplexScanner.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OCComplexParser.tab.c"
)

# -------------------------------------------------------------------
# 5) Combine all sources into a static library
# -------------------------------------------------------------------
set(ALL_SOURCES
    ${STATIC_SOURCES}
    ${BISON_OCComplexParser_OUTPUTS}
    ${FLEX_OCComplexScanner_OUTPUTS}
)

add_library(OCTypes STATIC ${ALL_SOURCES})

# -------------------------------------------------------------------
# 6) Set target properties to expose public headers in Xcode and install
# -------------------------------------------------------------------
set_target_properties(OCTypes PROPERTIES
    PUBLIC_HEADER "${OCTYPE_HDR}"
)

# -------------------------------------------------------------------
# 7) Include directories
# -------------------------------------------------------------------
target_include_directories(OCTypes PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

# -------------------------------------------------------------------
# 8) Optional install step for headers and static library
# -------------------------------------------------------------------
install(TARGETS OCTypes
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/OCTypes
)

# -------------------------------------------------------------------
# 9) Unit tests setup
# -------------------------------------------------------------------
enable_testing()
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c")

add_executable(runTests ${TEST_SOURCES})

target_include_directories(runTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(runTests PRIVATE OCTypes m)

add_test(NAME runTests COMMAND runTests)

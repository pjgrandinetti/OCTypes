cmake_minimum_required(VERSION 2.8)
project(library_octypes)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(MyParser src/OCComplexParser.y COMPILE_FLAGS -y ${CMAKE_CURRENT_BINARY_DIR}/OCComplexParser.tab.c DEFINES_FILE OCComplexParser.tab.h)
FLEX_TARGET(MyScanner src/OCComplexScanner.l ${CMAKE_CURRENT_BINARY_DIR}/OCComplexScanner.yy.c COMPILE_FLAGS -d)
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SOURCES src/OCArray.c src/OCAutoreleasePool.c src/OCBoolean.c src/OCData.c src/OCDictionary.c src/OCMath.c src/OCNumber.c src/OCString.c src/OCType.c ${BISON_MyParser_OUTPUTS} ${FLEX_MyScanner_OUTPUTS})

add_library(libOCTypes STATIC ${SOURCES} )

install(TARGETS libOCTypes DESTINATION /usr/lib)


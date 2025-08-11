# Makefile for OCTypes static library (with install packaging support)

CC = clang
AR = ar
LEX = flex
YACC = bison
YFLAGS = -d

INCLUDES := -I . -I src
WARNINGS := -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-missing-field-initializers
OPT := -O3
CFLAGS := -fPIC $(INCLUDES) $(WARNINGS) $(OPT) -DDEBUG -g

SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
GEN_DIR := $(BUILD_DIR)/gen
BIN_DIR := $(BUILD_DIR)/bin

LEX_SRC := $(SRC_DIR)/OCComplexScanner.l
YACC_SRC := $(SRC_DIR)/OCComplexParser.y

# Generated source files
GEN_C := $(GEN_DIR)/OCComplexScanner.c $(GEN_DIR)/OCComplexParser.tab.c
GEN_H := $(GEN_DIR)/OCComplexParser.tab.h
GEN_SRC := $(notdir $(GEN_C))

# Static (handwritten) source files
STATIC_SRC := $(filter-out $(YACC_SRC) $(LEX_SRC), $(wildcard $(SRC_DIR)/*.c))
SRC_SRC := $(notdir $(STATIC_SRC))

# All source files (for object generation)
ALL_C := $(SRC_SRC) $(GEN_SRC)
OBJ := $(addprefix $(OBJ_DIR)/, $(ALL_C:.c=.o))

# Test files
TEST_SRC_DIR := tests
TEST_FILES := $(wildcard $(TEST_SRC_DIR)/test_*.c) $(TEST_SRC_DIR)/main.c
TEST_OBJ := $(addprefix $(OBJ_DIR)/, $(notdir $(TEST_FILES:.c=.o)))

RM := rm -f
MKDIR_P := mkdir -p
LIBDIR := lib

# Platform-conditional variables for shared library
UNAME_S := $(shell uname -s)
ifeq ($(OS),Windows_NT)
  SHLIB_EXT      = .dll
  SHLIB_FLAGS    = -shared
  SHLIB_LDFLAGS  = -Wl,--out-implib=$(LIBDIR)/libOCTypes.dll.a
  PLATFORM_LIBS  = -ldbghelp
else ifeq ($(UNAME_S),Darwin)
  SHLIB_EXT      = .dylib
  SHLIB_FLAGS    = -dynamiclib -fPIC
  SHLIB_LDFLAGS  = -install_name @rpath/libOCTypes.dylib
  PLATFORM_LIBS  =
else
  SHLIB_EXT      = .so
  SHLIB_FLAGS    = -shared -fPIC
  SHLIB_LDFLAGS  =
  PLATFORM_LIBS  =
endif
SHLIB = $(LIBDIR)/libOCTypes$(SHLIB_EXT)

# Install target layout
INSTALL_DIR := install
INSTALL_LIB_DIR := $(INSTALL_DIR)/lib
INSTALL_INC_DIR := $(INSTALL_DIR)/include/OCTypes

.PHONY: all dirs install install-shared clean-objects clean test test-debug test-asan docs clean-docs doxygen html xcode compdb help

.DEFAULT_GOAL := all
all: dirs $(LIBDIR)/libOCTypes.a $(SHLIB)

dirs:
	$(MKDIR_P) $(LIBDIR) $(BUILD_DIR) $(OBJ_DIR) $(GEN_DIR) $(BIN_DIR)

# Generate scanner
$(GEN_DIR)/OCComplexScanner.c: $(LEX_SRC) | dirs
	$(LEX) -o $@ $<

# Generate parser
$(GEN_DIR)/OCComplexParser.tab.c $(GEN_DIR)/OCComplexParser.tab.h: $(YACC_SRC) | dirs
	$(YACC) $(YFLAGS) $<
	mv OCComplexParser.tab.c $(GEN_DIR)/OCComplexParser.tab.c
	mv OCComplexParser.tab.h $(GEN_DIR)/OCComplexParser.tab.h

# Object build rules

# For generated source files
$(OBJ_DIR)/%.o: $(GEN_DIR)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $<

# For static source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $<

# For test source files
$(OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c | dirs
	$(CC) $(CFLAGS) -Isrc -Itests -c -o $@ $<

# Ensure scanner object builds after parser header is available
$(OBJ_DIR)/OCComplexScanner.o: $(GEN_DIR)/OCComplexParser.tab.h

# Suppress warnings for specific files
$(OBJ_DIR)/OCComplexScanner.o: CFLAGS += -Wno-sign-compare -Wno-unused-function -Wno-unneeded-internal-declaration
$(OBJ_DIR)/OCComplexParser.tab.o: CFLAGS += -Wno-unused-parameter
$(OBJ_DIR)/OCAutoreleasePool.o: CFLAGS += -Wno-unused-function
$(OBJ_DIR)/OCString.o: CFLAGS += -Wno-unused-but-set-variable

# Build static library
$(LIBDIR)/libOCTypes.a: $(OBJ) | dirs
	$(AR) rcs $@ $^

# Build shared library
$(SHLIB): $(OBJ) | dirs
	$(CC) $(CFLAGS) $(SHLIB_FLAGS) $(SHLIB_LDFLAGS) -o $@ $^ $(PLATFORM_LIBS)

# Test targets
$(BIN_DIR)/runTests: $(LIBDIR)/libOCTypes.a $(TEST_OBJ)
	$(CC) $(CFLAGS) -Isrc -Itests $(TEST_OBJ) $(LIBDIR)/libOCTypes.a -lm $(PLATFORM_LIBS) -o $@

test: $(BIN_DIR)/runTests
	$<

test-debug: $(LIBDIR)/libOCTypes.a $(TEST_OBJ)
	$(CC) $(CFLAGS) -g -O0 -Isrc -Itests $(TEST_OBJ) $(LIBDIR)/libOCTypes.a -lm $(PLATFORM_LIBS) -o $(BIN_DIR)/runTests.debug
	@echo "Launching under LLDB..."
	@lldb -- $(BIN_DIR)/runTests.debug

test-asan: $(LIBDIR)/libOCTypes.a $(TEST_OBJ)
	$(CC) $(CFLAGS) -g -O1 -fsanitize=address -fno-omit-frame-pointer -Isrc -Itests $(TEST_OBJ) $(LIBDIR)/libOCTypes.a -lm $(PLATFORM_LIBS) -o $(BIN_DIR)/runTests.asan
	@echo "Running AddressSanitizer build..."
	@$(BIN_DIR)/runTests.asan

# Install target: package headers and both libraries
install: all
	$(MKDIR_P) $(INSTALL_LIB_DIR) $(INSTALL_INC_DIR)
	cp $(LIBDIR)/libOCTypes.a $(INSTALL_LIB_DIR)/
	cp $(SHLIB) $(INSTALL_LIB_DIR)/
	cp src/*.h $(INSTALL_INC_DIR)/

# Install both static and shared libraries (alias for install)
install-shared: install

clean-objects:
	$(RM) $(OBJ)

clean:
	$(RM) -r $(BUILD_DIR) $(LIBDIR) runTests runTests.debug runTests.asan *.dSYM OCComplexParser.output *.tab.* *.yy.* $(INSTALL_DIR)

# Documentation
# Compilation database for clang-tidy and IDE integration
compile_commands.json: dirs
	@echo "Generating compilation database..."
	@echo '[' > compile_commands.json
	@comma=""; \
	for src in $(STATIC_SRC); do \
		echo "$$comma{" >> compile_commands.json; \
		echo "  \"directory\": \"$(PWD)\"," >> compile_commands.json; \
		echo "  \"command\": \"$(CC) $(CFLAGS) -c $$src -o $(OBJ_DIR)/$$(basename $$src .c).o\"," >> compile_commands.json; \
		echo "  \"file\": \"$$src\"" >> compile_commands.json; \
		echo "}" >> compile_commands.json; \
		comma=","; \
	done; \
	for gen_src in $(GEN_C); do \
		echo "$$comma{" >> compile_commands.json; \
		echo "  \"directory\": \"$(PWD)\"," >> compile_commands.json; \
		echo "  \"command\": \"$(CC) $(CFLAGS) -c $$gen_src -o $(OBJ_DIR)/$$(basename $$gen_src .c).o\"," >> compile_commands.json; \
		echo "  \"file\": \"$$gen_src\"" >> compile_commands.json; \
		echo "}" >> compile_commands.json; \
		comma=","; \
	done; \
	echo ']' >> compile_commands.json
	@echo "✅ Compilation database generated: compile_commands.json"

compdb: compile_commands.json

doxygen:
	@echo "Building Doxygen documentation…"
	cd docs && doxygen Doxyfile

html:
	@echo "Building Sphinx HTML…"
	cd docs && sphinx-build -W -E -b html . _build/html

docs: html

clean-docs:
	@echo "Cleaning documentation…"
	rm -rf docs/doxygen docs/_build

xcode:
	@echo "Generating Xcode project in build-xcode..."
	@mkdir -p build-xcode
	@cmake -G "Xcode" -S . -B build-xcode

# Help target
help:
	@echo "OCTypes Makefile - Available targets:"
	@echo ""
	@echo "Building:"
	@echo "  all                 - Build static and dynamic libraries (default)"
	@echo "  lib/libOCTypes.a    - Build static library only"
	@echo "  lib/libOCTypes.dylib - Build dynamic library only"
	@echo "  dirs                - Create build directories"
	@echo ""
	@echo "Testing:"
	@echo "  test                - Run all tests"
	@echo "  test-debug          - Run tests with debug output"
	@echo "  test-asan           - Run tests with AddressSanitizer"
	@echo ""
	@echo "Installation:"
	@echo "  install             - Install to install/ directory"
	@echo "  install-shared      - Install shared libraries"
	@echo ""
	@echo "Documentation:"
	@echo "  docs                - Generate HTML documentation"
	@echo "  doxygen             - Generate Doxygen documentation"
	@echo "  html                - Generate Sphinx HTML documentation"
	@echo "  clean-docs          - Clean documentation files"
	@echo ""
	@echo "Development:"
	@echo "  xcode               - Generate Xcode project"
	@echo "  compdb              - Generate compile_commands.json"
	@echo ""
	@echo "Cleaning:"
	@echo "  clean-objects       - Remove object files"
	@echo "  clean               - Full clean"
	@echo ""
	@echo "Examples:"
	@echo "  make                       # Build libraries"
	@echo "  make test                  # Run test suite"
	@echo "  OC_LEAK_TRACKING=1 make test  # Run tests with leak tracking"
	@echo "  make install               # Install to local install/ directory"

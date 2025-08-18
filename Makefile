# OCTypes Makefile — builds static+shared libs, tests, docs, Xcode project
# Cross-platform (Linux/macOS/Windows-MinGW). Robust bison/flex generation.

# Tools (override CC via environment if desired)
CC       ?= clang
AR        = ar
LEX       = flex
YACC      = bison
YFLAGS    = -d

# Flags
INCLUDES  := -I . -I src
WARNINGS  := -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-missing-field-initializers
OPT       := -O3
CSTD      := -std=c99
CFLAGS    := -fPIC $(CSTD) $(INCLUDES) $(WARNINGS) $(OPT) -g
ifeq ($(OC_LEAK_TRACKING),1)
  CFLAGS  += -DOC_LEAK_TRACKING
endif

# Dirs
SRC_DIR    := src
TEST_SRC_DIR := tests
BUILD_DIR  := build
OBJ_DIR    := $(BUILD_DIR)/obj
GEN_DIR    := $(BUILD_DIR)/gen
BIN_DIR    := $(BUILD_DIR)/bin
LIBDIR     := lib
INSTALL_DIR      ?= install
INSTALL_LIB_DIR  := $(INSTALL_DIR)/lib
INSTALL_INC_DIR  := $(INSTALL_DIR)/include/OCTypes

# Sources
LEX_SRC    := $(SRC_DIR)/OCComplexScanner.l
YACC_SRC   := $(SRC_DIR)/OCComplexParser.y

# Generated files (write directly into GEN_DIR to avoid racing/mv)
GEN_PARSER_C := $(GEN_DIR)/OCComplexParser.tab.c
GEN_PARSER_H := $(GEN_DIR)/OCComplexParser.tab.h
GEN_SCANNER  := $(GEN_DIR)/OCComplexScanner.c
GEN_C        := $(GEN_SCANNER) $(GEN_PARSER_C)
GEN_H        := $(GEN_PARSER_H)
GEN_SRC      := $(notdir $(GEN_C))

# Handwritten sources
STATIC_SRC   := $(filter-out $(YACC_SRC) $(LEX_SRC), $(wildcard $(SRC_DIR)/*.c))
SRC_SRC      := $(notdir $(STATIC_SRC))

# All .c → objects
ALL_C        := $(SRC_SRC) $(GEN_SRC)
OBJ          := $(addprefix $(OBJ_DIR)/, $(ALL_C:.c=.o))

# Tests
TEST_FILES   := $(wildcard $(TEST_SRC_DIR)/test_*.c) $(TEST_SRC_DIR)/main.c
TEST_OBJ     := $(addprefix $(OBJ_DIR)/, $(notdir $(TEST_FILES:.c=.o)))

# Tools
RM       := rm -f
MKDIR_P  := mkdir -p

# Version information - extracted from git tags or manual override
VERSION ?= $(shell git describe --tags --abbrev=0 2>/dev/null | sed 's/^v//' || echo "0.1.5")
VERSION_MAJOR := $(shell echo $(VERSION) | cut -d. -f1)
VERSION_MINOR := $(shell echo $(VERSION) | cut -d. -f2)
VERSION_PATCH := $(shell echo $(VERSION) | cut -d. -f3)

# Platform
UNAME_S := $(shell uname -s)
ifeq ($(OS),Windows_NT)
  SHLIB_EXT     = .dll
  SHLIB_FLAGS   = -shared -Wl,--export-all-symbols -Wl,--enable-auto-import
  SHLIB_LDFLAGS = -Wl,--out-implib=$(LIBDIR)/libOCTypes.dll.a
  PLATFORM_LIBS = -ldbghelp
else ifeq ($(UNAME_S),Darwin)
  SHLIB_EXT     = .dylib
  SHLIB_FLAGS   = -dynamiclib -fPIC
  SHLIB_LDFLAGS = -install_name @rpath/libOCTypes.dylib -current_version $(VERSION) -compatibility_version $(VERSION_MAJOR).$(VERSION_MINOR)
  PLATFORM_LIBS =
else
  SHLIB_EXT     = .so
  SHLIB_FLAGS   = -shared -fPIC
  SHLIB_LDFLAGS =
  PLATFORM_LIBS =
endif
SHLIB := $(LIBDIR)/libOCTypes$(SHLIB_EXT)

# Phony
.PHONY: all dirs clean clean-docs compdb doxygen html docs test test-debug test-asan install install-shared xcode help
.DEFAULT_GOAL := all

# ───────── Build ─────────
all: dirs $(LIBDIR)/libOCTypes.a $(SHLIB)

dirs:
	$(MKDIR_P) $(LIBDIR) $(BUILD_DIR) $(OBJ_DIR) $(GEN_DIR) $(BIN_DIR)

# Bison/Flex: generate directly into GEN_DIR (no temporary files)
$(GEN_PARSER_C) $(GEN_PARSER_H): $(YACC_SRC) | dirs
	$(YACC) $(YFLAGS) --defines=$(GEN_PARSER_H) -o $(GEN_PARSER_C) $<

$(GEN_SCANNER): $(LEX_SRC) $(GEN_PARSER_H) | dirs
	$(LEX) -o $@ $<

# Objects
$(OBJ_DIR)/%.o: $(GEN_DIR)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c | dirs
	$(CC) $(CFLAGS) -Isrc -Itests -c -o $@ $<

# Per-file tweaks
$(OBJ_DIR)/OCComplexScanner.o: $(GEN_PARSER_H)
$(OBJ_DIR)/OCComplexScanner.o: CFLAGS += -Wno-sign-compare -Wno-unused-function -Wno-unneeded-internal-declaration
$(OBJ_DIR)/OCComplexParser.tab.o: CFLAGS += -Wno-unused-parameter
$(OBJ_DIR)/OCAutoreleasePool.o: CFLAGS += -Wno-unused-function
$(OBJ_DIR)/OCString.o: CFLAGS += -Wno-unused-but-set-variable

# Libraries
$(LIBDIR)/libOCTypes.a: $(OBJ) | dirs
	$(AR) rcs $@ $^

$(SHLIB): $(OBJ) | dirs
	$(CC) $(CFLAGS) $(SHLIB_FLAGS) $(SHLIB_LDFLAGS) -o $@ $^ $(PLATFORM_LIBS)

# ───────── Tests ─────────
$(BIN_DIR)/runTests: $(LIBDIR)/libOCTypes.a $(TEST_OBJ)
	$(CC) $(CFLAGS) -Isrc -Itests $(TEST_OBJ) $(LIBDIR)/libOCTypes.a -lm $(PLATFORM_LIBS) -o $@

test: $(BIN_DIR)/runTests
	$<

test-debug: $(LIBDIR)/libOCTypes.a $(TEST_OBJ)
	$(CC) $(filter-out -O3,$(CFLAGS)) -O0 -g -Isrc -Itests $(TEST_OBJ) $(LIBDIR)/libOCTypes.a -lm $(PLATFORM_LIBS) -o $(BIN_DIR)/runTests.debug
	@echo "Run: $(BIN_DIR)/runTests.debug"
	@command -v lldb >/dev/null 2>&1 && echo "Or debug: lldb -- $(BIN_DIR)/runTests.debug" || true

test-asan: $(LIBDIR)/libOCTypes.a $(TEST_OBJ)
	$(CC) $(filter-out -O3,$(CFLAGS)) -O1 -g -fsanitize=address -fno-omit-frame-pointer -Isrc -Itests $(TEST_OBJ) $(LIBDIR)/libOCTypes.a -lm $(PLATFORM_LIBS) -o $(BIN_DIR)/runTests.asan
	@echo "Running ASan (set OC_LEAK_TRACKING=1 for leak tracker instrumentation)"
	@OC_LEAK_TRACKING=$(OC_LEAK_TRACKING) $(BIN_DIR)/runTests.asan

# ───────── Install ─────────
install: all
	$(MKDIR_P) $(INSTALL_LIB_DIR) $(INSTALL_INC_DIR)
	cp $(LIBDIR)/libOCTypes.a $(INSTALL_LIB_DIR)/
	cp $(SHLIB)               $(INSTALL_LIB_DIR)/
	cp src/*.h                $(INSTALL_INC_DIR)/

install-shared: install

# ───────── Docs ─────────
doxygen:
	@echo "Building Doxygen…"
	cd docs && doxygen Doxyfile

html:
	@echo "Building Sphinx HTML…"
	cd docs && sphinx-build -W -E -b html . _build/html

docs: doxygen html

clean-docs:
	rm -rf docs/doxygen docs/_build

# ───────── Dev helpers ─────────
# Simple compile_commands.json for IDE tooling
compile_commands.json: dirs
	@echo "[" > $@
	@comma=""; \
	for src in $(STATIC_SRC); do \
	  obj=$(OBJ_DIR)/$$(basename $$src .c).o; \
	  printf "$$comma{\n  \"directory\": \"$(PWD)\",\n  \"command\": \"$(CC) $(CFLAGS) -c %s -o %s\",\n  \"file\": \"%s\"\n}" "$$src" "$$obj" "$$src" >> $@; \
	  comma=","; \
	done; \
	for src in $(GEN_C); do \
	  obj=$(OBJ_DIR)/$$(basename $$src .c).o; \
	  printf "$$comma{\n  \"directory\": \"$(PWD)\",\n  \"command\": \"$(CC) $(CFLAGS) -c %s -o %s\",\n  \"file\": \"%s\"\n}" "$$src" "$$obj" "$$src" >> $@; \
	  comma=","; \
	done; \
	echo "]" >> $@
	@echo "compile_commands.json written"

compdb: compile_commands.json

xcode:
	@echo "Generating Xcode project in build-xcode…"
	@mkdir -p build-xcode
	@cmake -G "Xcode" -S . -B build-xcode
	@echo "Open: build-xcode/OCTypes.xcodeproj (or your project name)"

# ───────── Clean / Help ─────────
clean:
	$(RM) -r $(BUILD_DIR) $(LIBDIR) $(INSTALL_DIR) $(BIN_DIR) \
	      runTests runTests.debug runTests.asan *.dSYM \
	      OCComplexParser.output *.tab.* *.yy.* compile_commands.json

help:
	@echo "OCTypes Makefile — targets:"
	@echo "  all (default)     : build static+shared"
	@echo "  test / test-debug : run unit tests (asan: test-asan)"
	@echo "  install           : copy libs+headers to ./install"
	@echo "  install-shared    : alias of install"
	@echo "  docs              : doxygen + sphinx html"
	@echo "  xcode             : generate Xcode project (build-xcode/)"
	@echo "  compdb            : write compile_commands.json"
	@echo "  clean / clean-docs"
	@echo ""
	@echo "Env:"
	@echo "  CC=clang|gcc, OC_LEAK_TRACKING=1 to add -DOC_LEAK_TRACKING"

# OCTypes Makefile (drop-in replacement)
# Builds static + shared libraries, supports Linux/macOS/Windows, fixes bison race

CC       = clang
AR       = ar
LEX      = flex
YACC     = bison
YFLAGS   = -d

INCLUDES := -I . -I src
WARNINGS := -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-missing-field-initializers
OPT      := -O3
CFLAGS   := -fPIC $(INCLUDES) $(WARNINGS) $(OPT) -DDEBUG -g

SRC_DIR   := src
BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj
GEN_DIR   := $(BUILD_DIR)/gen
BIN_DIR   := $(BUILD_DIR)/bin
LIBDIR    := lib

LEX_SRC  := $(SRC_DIR)/OCComplexScanner.l
YACC_SRC := $(SRC_DIR)/OCComplexParser.y

# Generated files
GEN_PARSER_C := $(GEN_DIR)/OCComplexParser.tab.c
GEN_PARSER_H := $(GEN_DIR)/OCComplexParser.tab.h
GEN_SCANNER  := $(GEN_DIR)/OCComplexScanner.c
GEN_C := $(GEN_SCANNER) $(GEN_PARSER_C)
GEN_H := $(GEN_PARSER_H)
GEN_SRC := $(notdir $(GEN_C))

# Handwritten sources
STATIC_SRC := $(filter-out $(YACC_SRC) $(LEX_SRC), $(wildcard $(SRC_DIR)/*.c))
SRC_SRC := $(notdir $(STATIC_SRC))

# All sources → objects
ALL_C := $(SRC_SRC) $(GEN_SRC)
OBJ   := $(addprefix $(OBJ_DIR)/, $(ALL_C:.c=.o))

# Tests
TEST_SRC_DIR := tests
TEST_FILES   := $(wildcard $(TEST_SRC_DIR)/test_*.c) $(TEST_SRC_DIR)/main.c
TEST_OBJ     := $(addprefix $(OBJ_DIR)/, $(notdir $(TEST_FILES:.c=.o)))

# Tools
RM      := rm -f
MKDIR_P := mkdir -p

# Platform-conditional settings
UNAME_S := $(shell uname -s)
ifeq ($(OS),Windows_NT)
  SHLIB_EXT     = .dll
  SHLIB_FLAGS   = -shared
  SHLIB_LDFLAGS = -Wl,--out-implib=$(LIBDIR)/libOCTypes.dll.a
  PLATFORM_LIBS = -ldbghelp
else ifeq ($(UNAME_S),Darwin)
  SHLIB_EXT     = .dylib
  SHLIB_FLAGS   = -dynamiclib -fPIC
  SHLIB_LDFLAGS = -install_name @rpath/libOCTypes.dylib
  PLATFORM_LIBS =
else
  SHLIB_EXT     = .so
  SHLIB_FLAGS   = -shared -fPIC
  SHLIB_LDFLAGS =
  PLATFORM_LIBS =
endif

SHLIB = $(LIBDIR)/libOCTypes$(SHLIB_EXT)

# Install dirs
INSTALL_DIR     := install
INSTALL_LIB_DIR := $(INSTALL_DIR)/lib
INSTALL_INC_DIR := $(INSTALL_DIR)/include/OCTypes

.PHONY: all dirs clean test install help
.DEFAULT_GOAL := all

all: dirs $(LIBDIR)/libOCTypes.a $(SHLIB)

dirs:
	$(MKDIR_P) $(LIBDIR) $(BUILD_DIR) $(OBJ_DIR) $(GEN_DIR) $(BIN_DIR)

# ── Lex / Bison (robust: no racing, no mv) ───────────────────────────────────
# Use modern bison flags to write outputs directly into $(GEN_DIR).
$(GEN_PARSER_C) $(GEN_PARSER_H): $(YACC_SRC) | dirs
	$(YACC) $(YFLAGS) --defines=$(GEN_PARSER_H) -o $(GEN_PARSER_C) $<

$(GEN_SCANNER): $(LEX_SRC) $(GEN_PARSER_H) | dirs
	$(LEX) -o $@ $<

# ── Objects ──────────────────────────────────────────────────────────────────
$(OBJ_DIR)/%.o: $(GEN_DIR)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c | dirs
	$(CC) $(CFLAGS) -Isrc -Itests -c -o $@ $<

# Ensure scanner rebuilds after parser header changes
$(OBJ_DIR)/OCComplexScanner.o: $(GEN_PARSER_H)
$(OBJ_DIR)/OCComplexScanner.o: CFLAGS += -Wno-sign-compare -Wno-unused-function -Wno-unneeded-internal-declaration
$(OBJ_DIR)/OCComplexParser.tab.o: CFLAGS += -Wno-unused-parameter
$(OBJ_DIR)/OCAutoreleasePool.o: CFLAGS += -Wno-unused-function
$(OBJ_DIR)/OCString.o: CFLAGS += -Wno-unused-but-set-variable

# ── Libraries ────────────────────────────────────────────────────────────────
$(LIBDIR)/libOCTypes.a: $(OBJ) | dirs
	$(AR) rcs $@ $^

$(SHLIB): $(OBJ) | dirs
	$(CC) $(CFLAGS) $(SHLIB_FLAGS) $(SHLIB_LDFLAGS) -o $@ $^ $(PLATFORM_LIBS)

# ── Tests ────────────────────────────────────────────────────────────────────
$(BIN_DIR)/runTests: $(LIBDIR)/libOCTypes.a $(TEST_OBJ)
	$(CC) $(CFLAGS) -Isrc -Itests $(TEST_OBJ) $(LIBDIR)/libOCTypes.a -lm $(PLATFORM_LIBS) -o $@

test: $(BIN_DIR)/runTests
	$<

# ── Install ──────────────────────────────────────────────────────────────────
install: all
	$(MKDIR_P) $(INSTALL_LIB_DIR) $(INSTALL_INC_DIR)
	cp $(LIBDIR)/libOCTypes.a $(INSTALL_LIB_DIR)/
	cp $(SHLIB) $(INSTALL_LIB_DIR)/
	cp src/*.h $(INSTALL_INC_DIR)/

# ── Cleanup ──────────────────────────────────────────────────────────────────
clean:
	$(RM) -r $(BUILD_DIR) $(LIBDIR) $(INSTALL_DIR) runTests runTests.* *.dSYM OCComplexParser.output *.tab.* *.yy.*
	
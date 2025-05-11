# Makefile for OCTypes static library

CC = clang
AR = ar
LEX = flex
YACC = bison -y
YFLAGS = -d
CFLAGS = -I . -I src -O3 -Wall -Wextra \
         -Wno-sign-compare -Wno-unused-parameter -Wno-missing-field-initializers

SRC_DIR = src
LEX_SRC = $(SRC_DIR)/OCComplexScanner.l
YACC_SRC = $(SRC_DIR)/OCComplexParser.y

# Generated sources
GEN_C = OCComplexScanner.c OCComplexParser.tab.c
GEN_H = OCComplexParser.tab.h

# All C sources (include generated, exclude flex/yacc inputs)
STATIC_SRC = $(filter-out $(YACC_SRC) $(LEX_SRC), $(wildcard $(SRC_DIR)/*.c))
ALL_C = $(GEN_C) $(notdir $(STATIC_SRC))
OBJ = $(ALL_C:.c=.o)

# Test sources and objects
TEST_SRC_DIR = tests
TEST_FILES = $(wildcard $(TEST_SRC_DIR)/test_*.c) $(TEST_SRC_DIR)/main.c
TEST_OBJ = $(notdir $(TEST_FILES:.c=.o))

.PHONY: all clean test docs clean-docs

all: libOCTypes.a

# Generate scanner
OCComplexScanner.c: $(LEX_SRC)
	$(LEX) -o $@ $<

# Generate parser
OCComplexParser.tab.c OCComplexParser.tab.h: $(YACC_SRC)
	$(YACC) $(YFLAGS) $<
	mv y.tab.c OCComplexParser.tab.c
	mv y.tab.h OCComplexParser.tab.h

# Ensure scanner object builds after parser header is available
OCComplexScanner.o: OCComplexParser.tab.h

# Suppress noisy warnings in generated code
OCComplexScanner.o: CFLAGS += -Wno-sign-compare -Wno-unused-function -Wno-unneeded-internal-declaration
OCComplexParser.tab.o: CFLAGS += -Wno-unused-parameter
# Suppress unused-function warning in autorelease pool implementation
OCAutoreleasePool.o: CFLAGS += -Wno-unused-function
# Suppress unused-but-set-variable warning in string implementation
OCString.o: CFLAGS += -Wno-unused-but-set-variable

# Compile C sources located in src/ to objects
%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile C sources to objects
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile C sources located in tests/ to objects
%.o: tests/%.c
	$(CC) $(CFLAGS) -Isrc -Itests -c -o $@ $<

# Build static library
libOCTypes.a: $(OBJ)
	$(AR) rcs $@ $^

.PHONY: test
test: libOCTypes.a $(TEST_OBJ)
	$(CC) $(CFLAGS) -Isrc -Itests $(TEST_OBJ) -L. -lOCTypes -lm -o runTests
	./runTests

.PHONY: test-debug
test-debug: libOCTypes.a $(TEST_OBJ)
	$(CC) $(CFLAGS) -g -O0 -Isrc -Itests $(TEST_OBJ) -L. -lOCTypes -lm -o runTests.debug
	@echo "Launching under LLDB..."
	@lldb -- ./runTests.debug

.PHONY: test-asan
test-asan: libOCTypes.a $(TEST_OBJ)
	$(CC) $(CFLAGS) -g -O1 -fsanitize=address -fno-omit-frame-pointer -Isrc -Itests $(TEST_OBJ) -L. -lOCTypes -lm -o runTests.asan
	@echo "Running AddressSanitizer build..."
	@./runTests.asan

clean:
	rm -f $(OBJ) $(TEST_OBJ) libOCTypes.a $(GEN_C) $(GEN_H) runTests runTests.debug runTests.asan *.dSYM -rf

# Documentation targets
docs: docs/Doxyfile
	@echo "Generating Doxygen XML output..."
	cd docs && doxygen Doxyfile
	@echo "Building Sphinx documentation..."
	cd docs && sphinx-build -b html . _build

clean-docs:
	@echo "Cleaning documentation..."
	rm -rf docs/doxygen
	rm -rf docs/_build

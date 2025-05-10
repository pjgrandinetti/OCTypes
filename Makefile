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

.PHONY: all clean test

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

# Build static library
libOCTypes.a: $(OBJ)
	$(AR) rcs $@ $^

.PHONY: test
test: libOCTypes.a
	$(CC) $(CFLAGS) -Isrc tests/main.c -L. -lOCTypes -lm -o runTests
	./runTests

clean:
	rm -f $(OBJ) libOCTypes.a $(GEN_C) $(GEN_H)

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <complex.h>
#include <math.h>
#include <stdint.h>

#include "../src/OCLibrary.h" // General OCTypes definitions
#include "../src/OCType.h"   // For OCTypeID, OCEqual, OCGetTypeID etc.

// PRINTERROR macro for consistent error reporting in tests
// Added __FILE__ to identify which test file failed.
#define PRINTERROR do { \
    fprintf(stderr, "Error: Test failed in function %s, file %s, line %d\n", __FUNCTION__, __FILE__, __LINE__); \
    return false; \
} while (0)

#endif /* TEST_UTILS_H */

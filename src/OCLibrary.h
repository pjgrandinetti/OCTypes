/**
 * @file OCLibrary.h
 * @brief Core definitions, macros, and types for the OCTypes library.
 *
 * This header centralizes common includes, constants, flags, and helper
 * macros used throughout the OCTypes framework.
 */

#ifndef OCLibrary_h
#define OCLibrary_h

#ifndef __private_extern__
#define __private_extern__
#endif

#include <sys/types.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <complex.h>
#include <setjmp.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup OCLibrary OCLibrary
 * @brief Core types and definitions shared across the OCTypes system.
 * @{
 */

/**
 * @brief Result values returned by OCComparatorFunction.
 */
typedef enum OCComparisonResult {
    kOCCompareLessThan = -1,     /**< First value is less than the second. */
    kOCCompareEqualTo = 0,       /**< Values are equal. */
    kOCCompareGreaterThan = 1,   /**< First value is greater than the second. */
    kOCCompareUnequalDimensionalities = 2, /**< Values have different dimensionalities. */
    kOCCompareNoSingleValue = 3,           /**< No singular comparison result available. */
    kOCCompareError = 99                  /**< An error occurred during comparison. */
} OCComparisonResult;

/**
 * @brief Function pointer type for comparing two values.
 *
 * @param val1 Pointer to the first value.
 * @param val2 Pointer to the second value.
 * @param context Optional user-defined context pointer.
 * @return Comparison result.
 */
typedef OCComparisonResult (*OCComparatorFunction)(const void *val1, const void *val2, void *context);

/**
 * @brief Option flags used for string comparisons.
 */
typedef unsigned long OCOptionFlags;

/**
 * @brief Flags for string comparison operations.
 *
 * Use with OCStringCompareFlags to modify comparison behavior.
 */
enum {
    kOCCompareCaseInsensitive        = 1,   /**< Case-insensitive comparison. */
    kOCCompareBackwards              = 4,   /**< Compare from the end of the string. */
    kOCCompareAnchored               = 8,   /**< Anchor comparison to the beginning. */
    kOCCompareNonliteral             = 16,  /**< Non-literal comparison (e.g., equivalence normalization). */
    kOCCompareLocalized              = 32,  /**< Locale-aware comparison. */
    kOCCompareNumerically            = 64,  /**< Numeric-aware comparison. */
    kOCCompareDiacriticInsensitive   = 128, /**< Ignore diacritics. */
    kOCCompareWidthInsensitive       = 256, /**< Ignore character width differences. */
    kOCCompareForcedOrdering         = 512  /**< Enforce ordering even if equal. */
};

/**
 * @brief Type alias for string comparison flags.
 */
typedef OCOptionFlags OCStringCompareFlags;

/**
 * @brief Aliases and compatibility masks for diacritic-insensitive comparison.
 */
enum {
    kOCCompareDiacriticsInsensitive = 128, /**< Alias for diacritic insensitivity. */
    kOCCompareDiacriticsInsensitiveCompatibilityMask = ((1 << 28) | kOCCompareDiacriticInsensitive)
};

/**
 * @brief A structure representing a contiguous byte or element range.
 */
typedef struct {
    uint64_t location; /**< Start index of the range. */
    uint64_t length;   /**< Number of elements in the range. */
} OCRange;

#if !defined(OC_INLINE)
#define OC_INLINE static __inline__
#endif

#if defined(OC_INLINE)
/**
 * @brief Convenience function to create an OCRange struct.
 *
 * @param loc Start index.
 * @param len Number of elements.
 * @return OCRange with specified location and length.
 */
OC_INLINE OCRange OCRangeMake(uint64_t loc, uint64_t len) {
    OCRange range;
    range.location = loc;
    range.length = len;
    return range;
}
#else
#define OCRangeMake(LOC, LEN) __OCRangeMake(LOC, LEN)
#endif

#include "OCMath.h"
#include "OCAutoreleasePool.h"
#include "OCType.h"
#include "OCString.h"
#include "OCData.h"
#include "OCBoolean.h"
#include "OCNumber.h"
#include "OCDictionary.h"
#include "OCArray.h"

/**
 * @brief Tears down the OCTypes library and releases any global resources.
 * @ingroup OCLibrary
 */
void OCLibraryTeardown(void);

/** @} */ // end of OCLibrary group

#endif /* OCLibrary_h */

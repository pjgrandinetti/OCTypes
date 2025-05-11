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

/** @defgroup OCLibrary OCLibrary */
/** @addtogroup OCLibrary
 *  @{
 */

/**
 * OCComparisonResult values returned by comparison callbacks.
 */
typedef enum OCComparisonResult {
    kOCCompareLessThan = -1,   /**< First value is less than the second. */
    kOCCompareEqualTo = 0,     /**< Values are equal. */
    kOCCompareGreaterThan = 1, /**< First value is greater than the second. */
    kOCCompareUnequalDimensionalities = 2, /**< Values have different dimensionalities. */
    kOCCompareNoSingleValue = 3,           /**< Comparison has no single value. */
    kOCCompareError = 99                  /**< An error occurred during comparison. */
} OCComparisonResult;

/**
 * Callback function type for comparing two values.
 * 
 * :param val1: Pointer to the first value.
 * :param val2: Pointer to the second value.
 * :param context: User-defined context pointer.
 * :return: An OCComparisonResult indicating ordering.
 */
typedef OCComparisonResult (* OCComparatorFunction)(const void *val1, const void *val2, void *context);

typedef unsigned long OCOptionFlags;

/**
 * :brief: Flags for string comparison operations.
 * :note: These values are used with OCStringCompareFlags.
 */
enum {
    kOCCompareCaseInsensitive = 1, /**< Perform a case-insensitive comparison. */
    kOCCompareBackwards = 4,    /**< Start comparison from the end of the string. */
    kOCCompareAnchored = 8,     /**< Comparison is anchored to the starting point. */
    kOCCompareNonliteral = 16,  /**< Perform loose equivalence (e.g., o-umlaut == o, umlaut). */
    kOCCompareLocalized = 32,   /**< Use the user's default locale for comparisons. */
    kOCCompareNumerically = 64, /**< Perform numeric comparison (e.g., Foo2.txt < Foo7.txt < Foo25.txt). */
    kOCCompareDiacriticInsensitive = 128, /**< Ignore diacritics during comparison (e.g., o-umlaut == o). */
    kOCCompareWidthInsensitive = 256, /**< Ignore width differences (e.g., 'a' == UFF41). */
    kOCCompareForcedOrdering = 512 /**< Force an ordering if strings are equivalent but not strictly equal (for sorting stability). */
};
typedef OCOptionFlags OCStringCompareFlags; /**< :type: OCStringCompareFlags - Type for string comparison option flags. */

/**
 * :brief: Additional comparison flags, primarily for diacritics.
 */
enum {
    kOCCompareDiacriticsInsensitive = 128, /**< Alias for kOCCompareDiacriticInsensitive. */
    kOCCompareDiacriticsInsensitiveCompatibilityMask = ((1 << 28)|kOCCompareDiacriticInsensitive), /**< Mask for compatibility regarding diacritic insensitivity. */
};

#if !defined(OC_INLINE)
#define OC_INLINE static __inline__
#endif

/**
 * Range structure representing a contiguous set of elements.
 * 
 * :ivar location: Starting index of the range.
 * :ivar length:   Number of elements in the range.
 */
typedef struct {
    uint64_t location; 
    uint64_t length;   
} OCRange;

#if defined(OC_INLINE)
/**
 * Creates a range with a given location and length.
 * 
 * :param loc: Starting index of the range.
 * :param len: Number of elements in the range.
 * :return: A new OCRange with specified parameters.
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
 * @brief Initializes the OCTypes library.
 */
void OCLibraryTeardown(void);

/** @} */ // end of OCLibrary group

#endif /* OCLibrary_h */

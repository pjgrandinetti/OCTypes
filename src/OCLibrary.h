/**
 * @file OCLibrary.h
 * @brief Core definitions, macros, and types for the OCTypes library.
 *
 * This header centralizes the core types and helper macros used throughout
 * the OCTypes framework, and then includes all the public OCTypes APIs.
 */

#ifndef OCLibrary_h
#define OCLibrary_h

// Define __private_extern__ for compatibility if not already defined
#ifndef __private_extern__
  #ifdef __APPLE__ // Or another macro specific to your Apple builds if __APPLE__ isn't right
    #define __private_extern__ __attribute__((__visibility__("hidden")))
  #else
    #define __private_extern__ // Define as empty for other platforms like Windows
  #endif
#endif

/* Minimal C types every module needs: */
#include <stddef.h>   /* for size_t, NULL */
#include <stdint.h>   /* for uint64_t, int32_t, etc. */
#include <stdbool.h>  /* for bool */

// Forward declarations for all opaque struct types
struct __OCType;
struct __OCString;
struct __OCArray;
struct __OCDictionary;
struct __OCBoolean;
struct __OCData;
struct __OCNumber;
// OCAutoreleasePoolRef is already a typedef to a pointer to a non-const struct,
// so it doesn't follow the same "Ref" pattern for const structs.
// No forward declaration needed here for __OCAutoreleasePool if OCAutoreleasePoolRef
// is defined as 'typedef struct _OCAutoreleasePool *OCAutoreleasePoolRef;'
// However, if it were 'typedef const struct __OCAutoreleasePool *OCAutoreleasePoolRef;',
// then 'struct __OCAutoreleasePool;' would be needed.
// Based on OCAutoreleasePool.h, it's 'typedef struct _OCAutoreleasePool *OCAutoreleasePoolRef;'
// so we don't add a forward declaration for __OCAutoreleasePool here.

/**
 * @defgroup OCLibrary OCLibrary
 * @brief Core types and definitions shared across the OCTypes system.
 * @{
 */

/**
 * @typedef OCOptionFlags
 * @brief Base type for option flags, typically an unsigned long.
 * @ingroup OCLibrary
 */
typedef unsigned long OCOptionFlags;

/**
 * @enum OCComparisonResult
 * @brief Result values returned by OCComparatorFunction.
 * @ingroup OCLibrary
 */
typedef enum { // Anonymous enum
    kOCCompareLessThan              = -1,  /**< First value is less than the second. */
    kOCCompareEqualTo               =  0,  /**< Values are equal. */
    kOCCompareGreaterThan           =  1,  /**< First value is greater than the second. */
    kOCCompareUnequalDimensionalities = 2, /**< Different dimensionalities. */
    kOCCompareNoSingleValue         =  3,  /**< No singular comparison result available. */
    kOCCompareError                 = 99   /**< An error occurred during comparison. */
} OCComparisonResult;

/**
 * @brief Function pointer type for comparing two values.
 *
 * @param val1    Pointer to the first value.
 * @param val2    Pointer to the second value.
 * @param context Optional user-defined context pointer.
 * @return Comparison result.
 */
typedef OCComparisonResult (*OCComparatorFunction)(const void *val1,
                                                   const void *val2,
                                                   void *context);

/**
 * @brief Option flags used for string comparisons.
 */
typedef unsigned long OCOptionFlags;

/**
 * @enum OCStringComparisonFlagsEnum
 * @brief Defines constant flags for string comparison operations.
 *
 * These flags are used with the OCStringCompareFlags type (which is an OCOptionFlags)
 * to modify comparison behavior.
 * @ingroup OCLibrary
 * @see OCStringCompareFlags
 */
typedef enum {
    kOCCompareCaseInsensitive        =   1,  /**< Case-insensitive comparison. */
    kOCCompareBackwards              =   4,  /**< Compare from the end of the string. */
    kOCCompareAnchored               =   8,  /**< Anchor comparison to the beginning. */
    kOCCompareNonliteral             =  16,  /**< Non-literal comparison (e.g. normalization). */
    kOCCompareLocalized              =  32,  /**< Locale-aware comparison. */
    kOCCompareNumerically            =  64,  /**< Numeric-aware comparison. */
    kOCCompareDiacriticInsensitive   = 128,  /**< Ignore diacritics. */
    kOCCompareWidthInsensitive       = 256,  /**< Ignore character width differences. */
    kOCCompareForcedOrdering         = 512   /**< Enforce ordering even if equal. */
} OCStringComparisonFlagsEnum;

/**
 * @typedef OCStringCompareFlags
 * @brief Type used to hold string comparison flags.
 *
 * This is an alias for OCOptionFlags (unsigned long).
 * Use constants from OCStringComparisonFlagsEnum with this type.
 * @ingroup OCLibrary
 * @see OCStringComparisonFlagsEnum
 */
typedef OCOptionFlags OCStringCompareFlags;

// Centralized Ref typedefs
typedef const struct __OCType *OCTypeRef;
typedef const struct __OCString *OCStringRef;
typedef const struct __OCArray *OCArrayRef;
typedef const struct __OCDictionary *OCDictionaryRef;
typedef const struct __OCBoolean *OCBooleanRef;
typedef const struct __OCData *OCDataRef;
typedef const struct __OCNumber *OCNumberRef;
// OCAutoreleasePoolRef is typically 'typedef struct _OCAutoreleasePool *OCAutoreleasePoolRef;'
// and not a 'const struct'. So, it's usually defined directly in OCAutoreleasePool.h.

// Mutable Ref typedefs
typedef struct __OCArray *OCMutableArrayRef;
typedef struct __OCData *OCMutableDataRef;
typedef struct __OCDictionary *OCMutableDictionaryRef;
typedef struct __OCString *OCMutableStringRef;

/**
 * @enum OCDiacriticCompatibilityFlagsEnum
 * @brief Defines compatibility flags for diacritic-insensitive comparison.
 * @ingroup OCLibrary
 */
typedef enum {
    kOCCompareDiacriticsInsensitive                    = 128, /**< Alias for diacritic insensitive. */
    kOCCompareDiacriticsInsensitiveCompatibilityMask   = ((1 << 28) | kOCCompareDiacriticInsensitive) /**< Compatibility mask. */
} OCDiacriticCompatibilityFlagsEnum;

/**
 * @brief A structure representing a contiguous byte or element range.
 */
typedef struct {
    uint64_t location; /**< Start index of the range. */
    uint64_t length;   /**< Number of elements in the range. */
} OCRange;

#if !defined(OC_INLINE)
#define OC_INLINE static inline // Changed from __inline__ to inline
#endif

#if defined(OC_INLINE)
/**
 * @brief Convenience function to create an OCRange.
 *
 * @param loc Start index.
 * @param len Number of elements.
 * @return OCRange with the given location and length.
 */
OC_INLINE OCRange OCRangeMake(uint64_t loc, uint64_t len) {
    OCRange r = { loc, len };
    return r;
}
#else
#define OCRangeMake(LOC, LEN) __OCRangeMake(LOC, LEN)
#endif

/* Now pull in the rest of the public OCTypes APIs: */
#include "OCMath.h"
#include "OCAutoreleasePool.h"
#include "OCType.h"
#include "OCString.h"
#include "OCData.h"
#include "OCBoolean.h"
#include "OCNumber.h"
#include "OCDictionary.h"
#include "OCArray.h"

/** @} */ // end of OCLibrary group

#endif /* OCLibrary_h */

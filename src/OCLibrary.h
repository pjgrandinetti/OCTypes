/**
 * @file OCLibrary.h
 * @brief Core definitions, macros, and types for the OCTypes library.
 *
 * This header centralizes the core types and helper macros used throughout
 * the OCTypes framework, and then includes all the public OCTypes APIs.
 * 
 * OCTypes is a lightweight reference-counting object system for C that provides 
 * common data structures (arrays, dictionaries, strings) and memory management 
 * facilities similar to Core Foundation, but with a simpler API.
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
 * 
 * The OCLibrary module provides fundamental types, structures and constants used
 * throughout the OCTypes framework. It includes comparison flags, range manipulation,
 * and common type definitions that serve as the foundation for the higher-level
 * data structures and memory management facilities.
 * @{
 */

/**
 * @typedef OCOptionFlags
 * @brief Base type for option flags, typically an unsigned long.
 * 
 * OCOptionFlags is used throughout the OCTypes library to represent bit flags and options.
 * This type provides a consistent way to pass multiple boolean options to functions.
 * 
 * @ingroup OCLibrary
 * 
 * @code
 * // Example of using OCOptionFlags with string comparison
 * OCStringRef str1 = OCStringCreateWithCString("Hello");
 * OCStringRef str2 = OCStringCreateWithCString("HELLO");
 * 
 * // Combine multiple options using bitwise OR
 * OCOptionFlags options = kOCCompareCaseInsensitive | kOCCompareNonliteral;
 * 
 * OCComparisonResult result = OCStringCompare(str1, str2, options);
 * // result will be kOCCompareEqualTo because case is ignored
 * 
 * OCRelease(str1);
 * OCRelease(str2);
 * @endcode
 */
typedef unsigned long OCOptionFlags;

/**
 * @enum OCComparisonResult
 * @brief Result values returned by OCComparatorFunction.
 * 
 * These constants represent the possible outcomes when comparing two values
 * using OCTypes comparison functions. They follow the same pattern as many
 * standard comparison functions: negative for less-than, zero for equal,
 * and positive for greater-than.
 * 
 * @ingroup OCLibrary
 * 
 * @code
 * // Example of using OCComparisonResult
 * OCNumberRef num1 = OCNumberCreateWithInt(10);
 * OCNumberRef num2 = OCNumberCreateWithInt(20);
 * 
 * OCComparisonResult result = OCNumberCompare(num1, num2);
 * 
 * switch (result) {
 *     case kOCCompareLessThan:
 *         printf("num1 is less than num2\n");  // This will be printed
 *         break;
 *     case kOCCompareEqualTo:
 *         printf("num1 is equal to num2\n");
 *         break;
 *     case kOCCompareGreaterThan:
 *         printf("num1 is greater than num2\n");
 *         break;
 *     default:
 *         printf("Error in comparison\n");
 *         break;
 * }
 * 
 * OCRelease(num1);
 * OCRelease(num2);
 * @endcode
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
 * OCComparatorFunction is used throughout OCTypes for sorting and searching operations.
 * It's a callback function that compares two values and returns their relative order.
 * 
 * @param val1    Pointer to the first value.
 * @param val2    Pointer to the second value.
 * @param context Optional user-defined context pointer.
 * @return Comparison result as OCComparisonResult.
 * 
 * @ingroup OCLibrary
 * 
 * @code
 * // Example custom comparator for integers
 * OCComparisonResult compareInts(const void *val1, const void *val2, void *context) {
 *     int int1 = *(int *)val1;
 *     int int2 = *(int *)val2;
 *     
 *     // For descending sort, swap the comparison
 *     bool descending = (context != NULL) ? *(bool *)context : false;
 *     
 *     if (descending) {
 *         if (int1 > int2) return kOCCompareLessThan;
 *         if (int1 < int2) return kOCCompareGreaterThan;
 *         return kOCCompareEqualTo;
 *     } else {
 *         if (int1 < int2) return kOCCompareLessThan;
 *         if (int1 > int2) return kOCCompareGreaterThan;
 *         return kOCCompareEqualTo;
 *     }
 * }
 * 
 * // Using the comparator with OCArray
 * int values[] = {3, 1, 4, 2};
 * OCMutableArrayRef array = OCArrayCreateMutable(4);
 * 
 * for (int i = 0; i < 4; i++) {
 *     OCNumberRef num = OCNumberCreateWithInt(values[i]);
 *     OCArrayAppendValue(array, num);
 *     OCRelease(num);
 * }
 * 
 * // Sort using the custom comparator
 * bool descending = true;
 * OCArraySortValues(array, OCRangeMake(0, OCArrayGetCount(array)), 
 *                  compareInts, &descending);
 * 
 * // array now contains: [4, 3, 2, 1]
 * 
 * OCRelease(array);
 * @endcode
 */
typedef OCComparisonResult (*OCComparatorFunction)(const void *val1,
                                                   const void *val2,
                                                   void *context);

/**
 * @enum OCStringComparisonFlagsEnum
 * @brief Defines constant flags for string comparison operations.
 *
 * These flags are used with the OCStringCompareFlags type (which is an OCOptionFlags)
 * to modify comparison behavior. Multiple flags can be combined using bitwise OR.
 * 
 * @ingroup OCLibrary
 * @see OCStringCompareFlags
 * 
 * @code
 * OCStringRef str1 = OCStringCreateWithCString("café");
 * OCStringRef str2 = OCStringCreateWithCString("cafe");
 * OCStringRef str3 = OCStringCreateWithCString("CAFÉ");
 * 
 * // Basic comparison (sensitive to case and diacritics)
 * OCComparisonResult result1 = OCStringCompare(str1, str2, 0);
 * // result1 will be kOCCompareGreaterThan, since 'é' > 'e'
 * 
 * // Diacritic-insensitive comparison
 * OCComparisonResult result2 = OCStringCompare(str1, str2, kOCCompareDiacriticInsensitive);
 * // result2 will be kOCCompareEqualTo
 * 
 * // Case and diacritic insensitive comparison
 * OCComparisonResult result3 = OCStringCompare(str1, str3, 
 *                             kOCCompareCaseInsensitive | kOCCompareDiacriticInsensitive);
 * // result3 will be kOCCompareEqualTo
 * 
 * OCRelease(str1);
 * OCRelease(str2);
 * OCRelease(str3);
 * @endcode
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
 * 
 * @ingroup OCLibrary
 * @see OCStringComparisonFlagsEnum
 * 
 * @code
 * // Example using OCStringCompareFlags for finding a substring
 * OCStringRef haystack = OCStringCreateWithCString("Hello World Hello Universe");
 * OCStringRef needle = OCStringCreateWithCString("hello");
 * 
 * // Search case-insensitively
 * OCStringCompareFlags flags = kOCCompareCaseInsensitive;
 * OCRange foundRange = OCStringFind(haystack, needle, flags);
 * 
 * // foundRange will be {0, 5} indicating "Hello" was found at the start
 * 
 * OCRelease(haystack);
 * OCRelease(needle);
 * @endcode
 */
typedef OCOptionFlags OCStringCompareFlags;

/** @cond INTERNAL */
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
/** @endcond */

/**
 * @enum OCDiacriticCompatibilityFlagsEnum
 * @brief Defines compatibility flags for diacritic-insensitive comparison.
 * 
 * These flags provide compatibility options for handling diacritic marks in string comparisons,
 * allowing for more flexible text matching across different languages and character sets.
 * 
 * @ingroup OCLibrary
 * 
 * @code
 * OCStringRef str1 = OCStringCreateWithCString("résumé");
 * OCStringRef str2 = OCStringCreateWithCString("resume");
 * 
 * // Standard comparison (sensitive to diacritics)
 * OCComparisonResult result1 = OCStringCompare(str1, str2, 0);
 * // result1 will NOT be kOCCompareEqualTo
 * 
 * // Using diacritics insensitive flag
 * OCComparisonResult result2 = OCStringCompare(str1, str2, 
 *                              kOCCompareDiacriticsInsensitive);
 * // result2 will be kOCCompareEqualTo
 * 
 * OCRelease(str1);
 * OCRelease(str2);
 * @endcode
 */
typedef enum {
    kOCCompareDiacriticsInsensitive                    = 128, /**< Alias for diacritic insensitive. */
    kOCCompareDiacriticsInsensitiveCompatibilityMask   = ((1 << 28) | kOCCompareDiacriticInsensitive) /**< Compatibility mask. */
} OCDiacriticCompatibilityFlagsEnum;

/**
 * @brief A structure representing a contiguous byte or element range.
 * 
 * OCRange is used throughout OCTypes to represent a range of elements in arrays, strings,
 * and other indexed collections. It consists of a starting index (location) and the
 * number of elements in the range (length).
 * 
 * @ingroup OCLibrary
 * 
 * @code
 * // Creating and using OCRange with strings
 * OCStringRef str = OCStringCreateWithCString("Hello, world!");
 * 
 * // Extract substring "world" (starts at index 7, length 5)
 * OCRange range = {7, 5};
 * OCStringRef substring = OCStringCreateWithSubstring(str, range);
 * 
 * // Alternative way to create the range
 * OCRange anotherRange = OCRangeMake(0, 5);
 * OCStringRef hello = OCStringCreateWithSubstring(str, anotherRange);
 * 
 * // Using OCRange with array operations
 * OCMutableArrayRef array = OCArrayCreateMutable(10);
 * // ... add items to array ...
 * 
 * // Remove elements at indices 2 through 4
 * OCArrayRemoveValueAtIndex(array, OCRangeMake(2, 3));
 * 
 * // Clean up
 * OCRelease(str);
 * OCRelease(substring);
 * OCRelease(hello);
 * OCRelease(array);
 * @endcode
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
 * OCRangeMake provides a simple way to create OCRange structures without having to
 * manually initialize the structure fields. This is the preferred way to create ranges
 * for use with OCTypes API functions.
 *
 * @param loc Start index.
 * @param len Number of elements.
 * @return OCRange with the given location and length.
 * 
 * @ingroup OCLibrary
 * 
 * @code
 * // Create a range starting at index 5 with length 10
 * OCRange range = OCRangeMake(5, 10);
 * 
 * // Use the range with OCTypes APIs
 * OCStringRef str = OCStringCreateWithCString("This is a test string for demonstration");
 * OCStringRef substring = OCStringCreateWithSubstring(str, range);
 * // substring will contain "a test st"
 * 
 * // Use with an array
 * OCMutableArrayRef array = OCArrayCreateMutable(20);
 * // ... populate array ...
 * 
 * // Get a subarray from indices 5 through 14
 * OCArrayRef subarray = OCArrayCreateArrayByTakingValueRange(array, range);
 * 
 * OCRelease(str);
 * OCRelease(substring);
 * OCRelease(array);
 * OCRelease(subarray);
 * @endcode
 */
OC_INLINE OCRange OCRangeMake(uint64_t loc, uint64_t len) {
    OCRange r = { loc, len };
    return r;
}
#else
#define OCRangeMake(LOC, LEN) __OCRangeMake(LOC, LEN)
#endif

// Expose cleanup functions for master cleanup
void cleanupConstantStringTable(void);
void cleanupTypeIDTable(void);

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
#ifdef DEBUG
#include "OCLeakTracker.h"
#endif

/** @} */ // end of OCLibrary group

#endif /* OCLibrary_h */

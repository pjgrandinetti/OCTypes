//
//  OCLibrary
//  OCTypes
//
//  Created by philip on 5/17/17.
//  Copyright © 2017 Philip Grandinetti. All rights reserved.
//

#ifndef OCLibrary_h
#define OCLibrary_h

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

/*!
 @enum OCComparisonResult
 @constant kOCCompareLessThan Returned by a comparison function if the first value is less than the second value..
 @constant kOCCompareEqualTo Returned by a comparison function if the first value is equal to the second value.
 @constant kOCCompareGreaterThan Returned by a comparison function if the first value is greater than the second value.
 @constant kOCCompareUnequalDimensionalities Returned by a comparison function if the two values have different dimensionalities.
 */
typedef enum OCComparisonResult {
    kOCCompareLessThan = -1,
    kOCCompareEqualTo = 0,
    kOCCompareGreaterThan = 1,
    kOCCompareUnequalDimensionalities = 2,
    kOCCompareNoSingleValue = 3,
    kOCCompareError = 99
} OCComparisonResult;

/* A standard comparison function */
typedef OCComparisonResult (* OCComparatorFunction)(const void *val1, const void *val2, void *context);

typedef unsigned long OCOptionFlags;

enum {
    kOCCompareCaseInsensitive = 1,
    kOCCompareBackwards = 4,		/* Starting from the end of the string */
    kOCCompareAnchored = 8,		/* Only at the specified starting point */
    kOCCompareNonliteral = 16,		/* If specified, loose equivalence is performed (o-umlaut == o, umlaut) */
    kOCCompareLocalized = 32,		/* User's default locale is used for the comparisons */
    kOCCompareNumerically = 64,		/* Numeric comparison is used; that is, Foo2.txt < Foo7.txt < Foo25.txt */
    kOCCompareDiacriticInsensitive = 128, /* If specified, ignores diacritics (o-umlaut == o) */
    kOCCompareWidthInsensitive = 256, /* If specified, ignores width differences ('a' == UFF41) */
    kOCCompareForcedOrdering = 512 /* If specified, comparisons are forced to return either kCFCompareLessThan or kCFCompareGreaterThan if the strings are equivalent but not strictly equal, for stability when sorting (e.g. "aaa" > "AAA" with kCFCompareCaseInsensitive specified) */
};
typedef OCOptionFlags OCStringCompareFlags;

enum {
    kOCCompareDiacriticsInsensitive = 128, /* kOCCompareDiacriticInsensitive */
    kOCCompareDiacriticsInsensitiveCompatibilityMask = ((1 << 28)|kOCCompareDiacriticInsensitive),
};

#if !defined(OC_INLINE)
#define OC_INLINE static __inline__
#endif

/* Range type */
typedef struct {
    uint64_t location;
    uint64_t length;
} OCRange;

#if defined(OC_INLINE)
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

#endif /* OCLibrary_h */

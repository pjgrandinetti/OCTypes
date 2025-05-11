#ifndef TEST_ARRAY_H
#define TEST_ARRAY_H

#include "test_utils.h"
#include "../src/OCArray.h" // For OCComparisonResult, OCRangeMake etc.

// Test prototype for array tests
bool arrayTest0(void);

// Adapter for OCArraySortValues on OCStringRefs (used by arrayTest0)
OCComparisonResult OCStringCompareAdapter(const void *a, const void *b, void *ctx);

#endif /* TEST_ARRAY_H */

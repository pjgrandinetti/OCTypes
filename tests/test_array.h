#ifndef TEST_ARRAY_H
#define TEST_ARRAY_H
#include <stdbool.h>         // Required for bool type
#include "../src/OCArray.h"  // For OCComparisonResult, OCRangeMake etc.
#include "test_utils.h"
// Test function declarations
bool arrayTest0(void);
bool arrayTest1_creation(void);
bool arrayTest2_access(void);
bool arrayTest3_modification(void);
bool arrayTest4_search_sort(void);
// Adapter for OCArraySortValues on OCStringRefs (used by arrayTest0)
OCComparisonResult OCStringCompareAdapter(const void *a, const void *b, void *ctx);
#endif /* TEST_ARRAY_H */

/**
 * @file test_OCIndexArray.h
 * @brief Unit tests for OCIndexArray functions.
 *
 * Declares test cases verifying creation, access, mutation,
 * and removal functionality of OCIndexArray and OCMutableIndexArray types.
 */
#ifndef TEST_OCINDEXARRAY_H
#define TEST_OCINDEXARRAY_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
bool OCIndexArrayCreateAndCount_test(void);
bool OCIndexArrayGetValueAtIndex_test(void);
bool OCIndexArraySetValueAtIndex_test(void);
bool OCIndexArrayRemoveValueAtIndex_test(void);
bool OCIndexArrayRemoveValuesAtIndexes_test(void);
bool OCIndexArrayDeepCopy_test(void);
#ifdef __cplusplus
}
#endif
#endif /* TEST_OCINDEXARRAY_H */

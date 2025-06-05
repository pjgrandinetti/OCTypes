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

void test_OCIndexArrayCreateAndCount(void);
void test_OCIndexArrayGetValueAtIndex(void);
void test_OCIndexArraySetValueAtIndex(void);
void test_OCIndexArrayRemoveValueAtIndex(void);
void test_OCIndexArrayRemoveValuesAtIndexes(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_OCINDEXARRAY_H */

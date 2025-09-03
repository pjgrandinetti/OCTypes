#ifndef TEST_DATA_H
#define TEST_DATA_H
#include "test_utils.h"
// Test prototype for data tests
bool dataTest0(void);
// New: mutable data tests
bool dataTest1(void);
bool dataTest_deepcopy(void);
// Base64 roundtrip test
bool dataTest_base64_roundtrip(void);
// JSON encoding tests
bool dataTest_json_encoding(void);
#endif /* TEST_DATA_H */

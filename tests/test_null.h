//
//  test_null.h
//  OCTypes
//
//  OCNull test function declarations
//
#ifndef test_null_h
#define test_null_h

#include <stdbool.h>

// OCNull test functions
bool test_OCNull_singleton(void);
bool test_OCNull_json_serialization(void);
bool test_OCNull_in_arrays(void);
bool test_OCNull_in_dictionaries(void);
bool test_OCNull_roundtrip(void);
bool test_OCNull_comprehensive(void);

#endif /* test_null_h */

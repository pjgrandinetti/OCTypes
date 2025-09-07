#ifndef TEST_NUMBER_H
#define TEST_NUMBER_H
#include "test_utils.h"  // Include common test utilities, includes PRINTERROR and stdint/stdbool etc.
/// Run basic OCNumber functionality tests. Returns true on success.
bool numberTest0(void);
/// Test untyped complex number JSON serialization
bool test_ocnumber_json_untyped_complex(void);
/// Test typed complex number JSON serialization
bool test_ocnumber_json_typed_complex(void);
/// Test untyped real number JSON serialization
bool test_ocnumber_json_untyped_real(void);
/// Test typed real number JSON serialization
bool test_ocnumber_json_typed_real(void);
/// Test backward compatibility with old JSON formats
bool test_ocnumber_json_backward_compatibility(void);
/// Run all OCNumber tests (basic + JSON). Returns true on success.
bool test_number_comprehensive(void);
#endif /* TEST_NUMBER_H */

/**
 * @file test_json_typed.h
 * @brief Header for JSONTyped serialization roundtrip tests.
 *
 * Tests the comprehensive JSONTyped serialization system across all OCTypes.
 * Each test performs a complete roundtrip: create object → serialize to JSONTyped →
 * deserialize from JSONTyped → verify equality with original.
 */
#ifndef test_json_typed_h
#define test_json_typed_h
#include <stdbool.h>
/**
 * @brief Test JSONTyped roundtrip for OCData objects.
 * Tests binary data serialization with base64 encoding.
 */
bool jsonTypedTest_OCData(void);
/**
 * @brief Test JSONTyped roundtrip for OCArray objects.
 * Tests recursive array serialization with mixed element types.
 */
bool jsonTypedTest_OCArray(void);
/**
 * @brief Test JSONTyped roundtrip for OCNumber objects.
 * Tests all numeric types including complex numbers.
 */
bool jsonTypedTest_OCNumber(void);
/**
 * @brief Test JSONTyped roundtrip for OCString objects.
 * Tests native JSON string representation.
 */
bool jsonTypedTest_OCString(void);
/**
 * @brief Test JSONTyped roundtrip for OCBoolean objects.
 * Tests native JSON boolean representation.
 */
bool jsonTypedTest_OCBoolean(void);
/**
 * @brief Test JSONTyped roundtrip for OCDictionary objects.
 * Tests key-value pair serialization with recursive values.
 */
bool jsonTypedTest_OCDictionary(void);
/**
 * @brief Test JSONTyped roundtrip for OCSet objects.
 * Tests unordered collection serialization.
 */
bool jsonTypedTest_OCSet(void);
/**
 * @brief Test JSONTyped roundtrip for OCIndexSet objects.
 * Tests range-based index set serialization.
 */
bool jsonTypedTest_OCIndexSet(void);
/**
 * @brief Test JSONTyped roundtrip for OCIndexArray objects.
 * Tests index array with capacity management.
 */
bool jsonTypedTest_OCIndexArray(void);
/**
 * @brief Test JSONTyped roundtrip for OCIndexPairSet objects.
 * Tests paired index serialization.
 */
bool jsonTypedTest_OCIndexPairSet(void);
/**
 * @brief Test JSONTyped roundtrip for OCAutoreleasePool objects.
 * Tests autorelease pool state serialization.
 */
bool jsonTypedTest_OCAutoreleasePool(void);
/**
 * @brief Test global OCTypeCopyJSONTyped and OCTypeCreateFromJSONTyped functions.
 * Tests the polymorphic dispatch system.
 */
bool jsonTypedTest_GlobalFunctions(void);
/**
 * @brief Run all JSONTyped roundtrip tests.
 * @return true if all tests pass, false otherwise.
 */
bool runAllJSONTypedTests(void);
#endif /* test_json_typed_h */

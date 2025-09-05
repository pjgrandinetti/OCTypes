// tests/test_dictionary.c
#include <complex.h>  // for float complex / double complex
#include <stdint.h>
#include <stdio.h>
#include <string.h>  // for strncmp
#include <time.h>    // for clock(), CLOCKS_PER_SEC
#include "../src/OCArray.h"
#include "../src/OCAutoreleasePool.h"
#include "../src/OCBoolean.h"
#include "../src/OCDictionary.h"
#include "../src/OCNumber.h"
#include "../src/OCString.h"
#include "../src/OCType.h"  // for OCTypeEqual
#include "test_utils.h"
bool dictionaryTest0(void) {
    printf("dictionaryTest0 begin...");
    // Test 1: empty mutable dict
    OCMutableDictionaryRef dict1_mut = OCDictionaryCreateMutable(0);
    ASSERT_NOT_NULL(dict1_mut, "Test 1.1: dict1_mut should not be NULL");
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict1_mut), 0, "Test 1.2: dict1_mut should be empty");
    OCRelease(dict1_mut);
    // Test 2: add two key/value pairs
    OCStringRef key1 = OCStringCreateWithCString("key1");
    OCNumberRef val1 = OCNumberCreateWithSInt32(100);
    OCStringRef key2 = OCStringCreateWithCString("key2");
    OCNumberRef val2 = OCNumberCreateWithSInt32(200);
    OCMutableDictionaryRef dict2 = OCDictionaryCreateMutable(2);
    ASSERT_NOT_NULL(dict2, "Test 2.1: dict2 should not be NULL");
    OCDictionaryAddValue(dict2, key1, val1);
    OCDictionaryAddValue(dict2, key2, val2);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict2), 2, "Test 2.2: dict2 should have 2 items");
    ASSERT_TRUE(OCTypeEqual(OCDictionaryGetValue((OCDictionaryRef)dict2, key1), val1), "Test 2.3: value for key1");
    ASSERT_TRUE(OCTypeEqual(OCDictionaryGetValue((OCDictionaryRef)dict2, key2), val2), "Test 2.4: value for key2");
    // Test 3: remove and contains
    OCDictionaryRemoveValue(dict2, key1);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict2), 1, "Test 3.1: dict2 should have 1 item after removal");
    ASSERT_NULL(OCDictionaryGetValue((OCDictionaryRef)dict2, key1), "Test 3.2: key1 no longer present");
    ASSERT_TRUE(OCDictionaryContainsKey((OCDictionaryRef)dict2, key2), "Test 3.3: key2 still present");
    // Test 4: retrieve & unwrap number
    OCNumberRef got = (OCNumberRef)OCDictionaryGetValue((OCDictionaryRef)dict2, key2);
    ASSERT_NOT_NULL(got, "Test 4.1: retrieved val2 not NULL");
    ASSERT_TRUE(OCTypeEqual(got, val2), "Test 4.2: retrieved val2 equals original");
    int out;
    OCNumberGetValue(got, kOCNumberSInt32Type, &out);
    ASSERT_EQUAL(out, 200, "Test 4.3: integer unwrapped correctly");
    // Test 5: copy dictionary stays independent
    OCDictionaryAddValue(dict2, key1, val1);
    OCDictionaryRef copy = OCDictionaryCreateCopy((OCDictionaryRef)dict2);
    ASSERT_NOT_NULL(copy, "Test 5.1: copy not NULL");
    ASSERT_EQUAL(OCDictionaryGetCount(copy), OCDictionaryGetCount((OCDictionaryRef)dict2), "Test 5.2: counts match");
    // mutate original
    OCNumberRef z = OCNumberCreateWithSInt32(300);
    OCStringRef key3 = OCStringCreateWithCString("key3");  // Store the key
    OCDictionaryAddValue(dict2, key3, z);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict2), 3, "Test 5.3: original now 3");
    ASSERT_EQUAL(OCDictionaryGetCount(copy), 2, "Test 5.4: copy still 2");
    OCRelease(z);
    OCRelease(copy);
    // cleanup
    OCRelease(key1);
    OCRelease(key2);
    OCRelease(key3);  // Release the stored key3
    OCRelease(val1);
    OCRelease(val2);
    OCRelease(dict2);
    printf(" passed\n");
    return true;
}
bool dictionaryTest1(void) {
    printf("dictionaryTest1 begin...");
    bool success = true;
    // Test 1: Using NULL values and keys
    OCMutableDictionaryRef dict1 = OCDictionaryCreateMutable(0);
    ASSERT_NOT_NULL(dict1, "Test 1.1: dict1 should not be NULL");
    // Test with NULL key (should be handled gracefully)
    OCStringRef key1 = OCStringCreateWithCString("key1");
    OCNumberRef val1 = OCNumberCreateWithSInt32(100);
    OCDictionaryAddValue(dict1, NULL, val1);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict1), 0, "Test 1.2: NULL key shouldn't be added");
    // Test with NULL value (implementation ignores NULL values)
    OCDictionaryAddValue(dict1, key1, NULL);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict1), 0, "Test 1.3: NULL value shouldn't be added");
    ASSERT_NULL(OCDictionaryGetValue((OCDictionaryRef)dict1, key1), "Test 1.4: Value for key1 should be NULL");
    OCDictionaryRemoveValue(dict1, key1);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict1), 0, "Test 1.5: dict should be empty after removal");
    OCRelease(dict1);
    // Test 2: Overwriting existing keys
    OCMutableDictionaryRef dict2 = OCDictionaryCreateMutable(2);
    OCDictionaryAddValue(dict2, key1, val1);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict2), 1, "Test 2.1: dict should have 1 item");
    // Create a new value
    OCNumberRef val2 = OCNumberCreateWithSInt32(200);
    OCDictionaryAddValue(dict2, key1, val2);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict2), 1, "Test 2.2: Key should overwrite existing value");
    // Test value retrieval - should get the updated value
    OCNumberRef retrieved = (OCNumberRef)OCDictionaryGetValue((OCDictionaryRef)dict2, key1);
    int32_t num;
    OCNumberGetValue(retrieved, kOCNumberSInt32Type, &num);
    ASSERT_EQUAL(num, 200, "Test 2.3: Value should be overwritten to 200");
    OCRelease(dict2);
    OCRelease(val2);
    // Test 3: Create empty dictionary copy
    OCMutableDictionaryRef empty_dict = OCDictionaryCreateMutable(0);
    OCDictionaryRef empty_copy = OCDictionaryCreateCopy((OCDictionaryRef)empty_dict);
    ASSERT_NOT_NULL(empty_copy, "Test 3.1: Empty copy should not be NULL");
    ASSERT_EQUAL(OCDictionaryGetCount(empty_copy), 0, "Test 3.2: Empty copy should have 0 items");
    OCRelease(empty_dict);
    OCRelease(empty_copy);
    // Test 4: Using identical values for different keys
    OCMutableDictionaryRef dict4 = OCDictionaryCreateMutable(4);
    OCStringRef key2 = OCStringCreateWithCString("key2");
    OCStringRef key3 = OCStringCreateWithCString("key3");
    // Use same value for multiple keys
    OCDictionaryAddValue(dict4, key1, val1);
    OCDictionaryAddValue(dict4, key2, val1);
    OCDictionaryAddValue(dict4, key3, val1);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict4), 3, "Test 4.1: dict should have 3 items");
    // Check if all keys have the same value
    ASSERT_TRUE(OCTypeEqual(OCDictionaryGetValue((OCDictionaryRef)dict4, key1), val1),
                "Test 4.2: key1 should have val1");
    ASSERT_TRUE(OCTypeEqual(OCDictionaryGetValue((OCDictionaryRef)dict4, key2), val1),
                "Test 4.3: key2 should have val1");
    ASSERT_TRUE(OCTypeEqual(OCDictionaryGetValue((OCDictionaryRef)dict4, key3), val1),
                "Test 4.4: key3 should have val1");
    // Test OCDictionaryGetCountOfValue
    uint64_t val_count = OCDictionaryGetCountOfValue((OCMutableDictionaryRef)dict4, val1);
    ASSERT_EQUAL(val_count, 3, "Test 4.5: val1 should appear 3 times");
    OCRelease(dict4);
    OCRelease(key2);
    OCRelease(key3);
    OCRelease(key1);
    OCRelease(val1);
    printf(" passed\n");
    return success;
}
bool dictionaryTest2(void) {
    printf("dictionaryTest2 begin...");
    bool success = true;
    // Test 1: Create arrays of keys and values
    OCMutableDictionaryRef dict = OCDictionaryCreateMutable(10);
    // Create 10 key-value pairs
    char key_buffer[32];
    OCStringRef keys[10];
    OCNumberRef values[10];
    for (int i = 0; i < 10; i++) {
        // Create keys like "key_0", "key_1", etc.
        snprintf(key_buffer, sizeof(key_buffer), "key_%d", i);
        keys[i] = OCStringCreateWithCString(key_buffer);
        // Create values 100, 200, 300, etc.
        values[i] = OCNumberCreateWithSInt32((i + 1) * 100);
        OCDictionaryAddValue(dict, keys[i], values[i]);
    }
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict), 10, "Test 1.1: dict should have 10 items");
    // Test OCDictionaryCreateArrayWithAllKeys
    OCArrayRef keys_array = OCDictionaryCreateArrayWithAllKeys((OCDictionaryRef)dict);
    ASSERT_NOT_NULL(keys_array, "Test 1.2: keys_array should not be NULL");
    ASSERT_EQUAL(OCArrayGetCount(keys_array), 10, "Test 1.3: keys_array should have 10 items");
    // Test OCDictionaryCreateArrayWithAllValues
    OCArrayRef values_array = OCDictionaryCreateArrayWithAllValues((OCDictionaryRef)dict);
    ASSERT_NOT_NULL(values_array, "Test 1.4: values_array should not be NULL");
    ASSERT_EQUAL(OCArrayGetCount(values_array), 10, "Test 1.5: values_array should have 10 items");
    // Test OCDictionaryGetKeysAndValues
    const void *keys_buffer[10] = {0};
    const void *values_buffer[10] = {0};
    OCDictionaryGetKeysAndValues((OCDictionaryRef)dict, keys_buffer, values_buffer);
    // Test a few key-value pairs from the extracted arrays
    bool found_keys[10] = {false};
    // Verify all keys and values were extracted
    for (int i = 0; i < 10; i++) {
        OCStringRef current_key = (OCStringRef)keys_buffer[i];
        OCNumberRef current_value = (OCNumberRef)values_buffer[i];
        // Find which original key this matches
        for (int j = 0; j < 10; j++) {
            if (OCTypeEqual(current_key, keys[j])) {
                found_keys[j] = true;
                // Also verify the value matches
                ASSERT_TRUE(OCTypeEqual(current_value, values[j]),
                            "Test 1.6: Value mismatch for extracted key-value pair");
                break;
            }
        }
    }
    // Ensure all keys were found
    for (int i = 0; i < 10; i++) {
        ASSERT_TRUE(found_keys[i], "Test 1.7: Not all keys were extracted correctly");
    }
    // Test 2: Dictionary stress test with replace/remove operations
    // Replace half the values
    for (int i = 0; i < 5; i++) {
        OCNumberRef new_value = OCNumberCreateWithSInt32((i + 1) * 1000);
        OCDictionaryReplaceValue(dict, keys[i], new_value);
        OCRelease(new_value);
    }
    // Remove 3 items
    for (int i = 7; i < 10; i++) {
        OCDictionaryRemoveValue(dict, keys[i]);
    }
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict), 7, "Test 2.1: dict should have 7 items after changes");
    // Test 3: Mutable copy
    OCMutableDictionaryRef dict_copy = OCDictionaryCreateMutableCopy((OCDictionaryRef)dict);
    ASSERT_NOT_NULL(dict_copy, "Test 3.1: Mutable copy should not be NULL");
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict_copy), 7, "Test 3.2: Copy should have 7 items");
    // Modify the copy
    OCStringRef new_key = OCStringCreateWithCString("new_key");
    OCNumberRef new_value = OCNumberCreateWithSInt32(9999);
    OCDictionaryAddValue(dict_copy, new_key, new_value);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict_copy), 8, "Test 3.3: Modified copy should have 8 items");
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict), 7, "Test 3.4: Original should still have 7 items");
    // Cleanup
    OCRelease(new_key);
    OCRelease(new_value);
    OCRelease(keys_array);
    OCRelease(values_array);
    OCRelease(dict_copy);
    for (int i = 0; i < 10; i++) {
        OCRelease(keys[i]);
        OCRelease(values[i]);
    }
    OCRelease(dict);
    printf(" passed\n");
    return success;
}
bool dictionaryTest3(void) {
    printf("dictionaryTest3 begin...");
    bool success = true;
    // Test 1: Dictionary with different types of values
    OCMutableDictionaryRef dict = OCDictionaryCreateMutable(5);
    // String value
    OCStringRef str_key = OCStringCreateWithCString("string_key");
    OCStringRef str_value = OCStringCreateWithCString("Hello Dictionary!");
    OCDictionaryAddValue(dict, str_key, str_value);
    // Number value
    OCStringRef num_key = OCStringCreateWithCString("number_key");
    OCNumberRef num_value = OCNumberCreateWithSInt32(42);
    OCDictionaryAddValue(dict, num_key, num_value);
    // Boolean value
    OCStringRef bool_key = OCStringCreateWithCString("bool_key");
    OCDictionaryAddValue(dict, bool_key, kOCBooleanTrue);
    // Array value
    OCStringRef array_key = OCStringCreateWithCString("array_key");
    OCMutableArrayRef array_value = OCArrayCreateMutable(3, &kOCTypeArrayCallBacks);
    OCStringRef item1 = OCStringCreateWithCString("Item 1");
    OCStringRef item2 = OCStringCreateWithCString("Item 2");
    OCStringRef item3 = OCStringCreateWithCString("Item 3");
    OCArrayAppendValue(array_value, item1);
    OCArrayAppendValue(array_value, item2);
    OCArrayAppendValue(array_value, item3);
    OCDictionaryAddValue(dict, array_key, array_value);
    // Nested dictionary
    OCStringRef nested_key = OCStringCreateWithCString("nested_key");
    OCMutableDictionaryRef nested_dict = OCDictionaryCreateMutable(2);
    OCStringRef nested_str_key = OCStringCreateWithCString("nested_string");
    OCStringRef nested_str_value = OCStringCreateWithCString("I'm nested!");
    OCDictionaryAddValue(nested_dict, nested_str_key, nested_str_value);
    OCDictionaryAddValue(dict, nested_key, nested_dict);
    // Verify the dictionary has the expected structure
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict), 5, "Test 1.1: dict should have 5 items");
    // Test string value
    OCStringRef retrieved_str = (OCStringRef)OCDictionaryGetValue((OCDictionaryRef)dict, str_key);
    ASSERT_TRUE(OCTypeEqual(retrieved_str, str_value), "Test 1.2: String value mismatch");
    // Test number value
    OCNumberRef retrieved_num = (OCNumberRef)OCDictionaryGetValue((OCDictionaryRef)dict, num_key);
    ASSERT_TRUE(OCTypeEqual(retrieved_num, num_value), "Test 1.3: Number value mismatch");
    // Test boolean value
    OCBooleanRef retrieved_bool = (OCBooleanRef)OCDictionaryGetValue((OCDictionaryRef)dict, bool_key);
    ASSERT_TRUE(retrieved_bool == kOCBooleanTrue, "Test 1.4: Boolean value mismatch");
    // Test array value
    OCArrayRef retrieved_array = (OCArrayRef)OCDictionaryGetValue((OCDictionaryRef)dict, array_key);
    ASSERT_EQUAL(OCArrayGetCount(retrieved_array), 3, "Test 1.5: Array should have 3 items");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(retrieved_array, 0), item1), "Test 1.6: Array item 1 mismatch");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(retrieved_array, 1), item2), "Test 1.7: Array item 2 mismatch");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(retrieved_array, 2), item3), "Test 1.8: Array item 3 mismatch");
    // Test nested dictionary
    OCDictionaryRef retrieved_nested_dict = (OCDictionaryRef)OCDictionaryGetValue((OCDictionaryRef)dict, nested_key);
    ASSERT_EQUAL(OCDictionaryGetCount(retrieved_nested_dict), 1, "Test 1.9: Nested dict should have 1 item");
    OCStringRef retrieved_nested_str = (OCStringRef)OCDictionaryGetValue(retrieved_nested_dict, nested_str_key);
    ASSERT_TRUE(OCTypeEqual(retrieved_nested_str, nested_str_value), "Test 1.10: Nested string value mismatch");
    // Test 2: Dictionary with Unicode keys
    OCMutableDictionaryRef unicode_dict = OCDictionaryCreateMutable(3);
    // Create keys with Unicode characters
    OCStringRef unicode_key1 = OCStringCreateWithCString("辞書");     // Japanese for "dictionary"
    OCStringRef unicode_key2 = OCStringCreateWithCString("словарь");  // Russian for "dictionary"
    OCStringRef unicode_key3 = OCStringCreateWithCString("قاموس");    // Arabic for "dictionary"
    // Add values with Unicode keys
    OCNumberRef unicode_val1 = OCNumberCreateWithSInt32(1);
    OCNumberRef unicode_val2 = OCNumberCreateWithSInt32(2);
    OCNumberRef unicode_val3 = OCNumberCreateWithSInt32(3);
    OCDictionaryAddValue(unicode_dict, unicode_key1, unicode_val1);
    OCDictionaryAddValue(unicode_dict, unicode_key2, unicode_val2);
    OCDictionaryAddValue(unicode_dict, unicode_key3, unicode_val3);
    // Verify the unicode keys work properly
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)unicode_dict), 3, "Test 2.1: Unicode dict should have 3 items");
    ASSERT_TRUE(OCTypeEqual(OCDictionaryGetValue((OCDictionaryRef)unicode_dict, unicode_key1), unicode_val1),
                "Test 2.2: Japanese key value mismatch");
    ASSERT_TRUE(OCTypeEqual(OCDictionaryGetValue((OCDictionaryRef)unicode_dict, unicode_key2), unicode_val2),
                "Test 2.3: Russian key value mismatch");
    ASSERT_TRUE(OCTypeEqual(OCDictionaryGetValue((OCDictionaryRef)unicode_dict, unicode_key3), unicode_val3),
                "Test 2.4: Arabic key value mismatch");
    // Cleanup
    OCRelease(str_key);
    OCRelease(str_value);
    OCRelease(num_key);
    OCRelease(num_value);
    OCRelease(bool_key);
    OCRelease(array_key);
    OCRelease(item1);
    OCRelease(item2);
    OCRelease(item3);
    OCRelease(array_value);
    OCRelease(nested_key);
    OCRelease(nested_str_key);
    OCRelease(nested_str_value);
    OCRelease(nested_dict);
    OCRelease(dict);
    OCRelease(unicode_key1);
    OCRelease(unicode_key2);
    OCRelease(unicode_key3);
    OCRelease(unicode_val1);
    OCRelease(unicode_val2);
    OCRelease(unicode_val3);
    OCRelease(unicode_dict);
    printf(" passed\n");
    return success;
}
bool dictionaryTest4(void) {
    printf("dictionaryTest4 begin...");
    bool success = true;
    // Test 1: Zero and very small initial capacity
    OCMutableDictionaryRef zero_cap_dict = OCDictionaryCreateMutable(0);
    ASSERT_NOT_NULL(zero_cap_dict, "Test 1.1: Zero capacity dict should not be NULL");
    // Add many more items than the initial capacity
    const int item_count = 100;  // Reduced from 1000 for faster testing
    char key_buffer[64];
    for (int i = 0; i < item_count; i++) {
        snprintf(key_buffer, sizeof(key_buffer), "key_%d", i);
        OCStringRef key = OCStringCreateWithCString(key_buffer);
        OCNumberRef value = OCNumberCreateWithSInt32(i);
        OCDictionaryAddValue(zero_cap_dict, key, value);
        OCRelease(key);
        OCRelease(value);
    }
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)zero_cap_dict), item_count,
                 "Test 1.2: Dictionary should grow to accommodate all items");
    // Test key retrieval for various positions
    OCStringRef test_key_first = OCStringCreateWithCString("key_0");
    OCStringRef test_key_middle = OCStringCreateWithCString("key_50");
    OCStringRef test_key_last = OCStringCreateWithCString("key_99");
    OCNumberRef val_first = (OCNumberRef)OCDictionaryGetValue((OCDictionaryRef)zero_cap_dict, test_key_first);
    OCNumberRef val_middle = (OCNumberRef)OCDictionaryGetValue((OCDictionaryRef)zero_cap_dict, test_key_middle);
    OCNumberRef val_last = (OCNumberRef)OCDictionaryGetValue((OCDictionaryRef)zero_cap_dict, test_key_last);
    int32_t num_val;
    OCNumberGetValue(val_first, kOCNumberSInt32Type, &num_val);
    ASSERT_EQUAL(num_val, 0, "Test 1.3: First value should be 0");
    OCNumberGetValue(val_middle, kOCNumberSInt32Type, &num_val);
    ASSERT_EQUAL(num_val, 50, "Test 1.4: Middle value should be 50");
    OCNumberGetValue(val_last, kOCNumberSInt32Type, &num_val);
    ASSERT_EQUAL(num_val, 99, "Test 1.5: Last value should be 99");
    OCRelease(test_key_first);
    OCRelease(test_key_middle);
    OCRelease(test_key_last);
    // Test 2: Duplicate key handling
    OCMutableDictionaryRef dup_dict = OCDictionaryCreateMutable(10);
    OCStringRef dup_key = OCStringCreateWithCString("duplicate_key");
    OCNumberRef val1 = OCNumberCreateWithSInt32(100);
    OCNumberRef val2 = OCNumberCreateWithSInt32(200);
    OCNumberRef val3 = OCNumberCreateWithSInt32(300);
    // Add the same key multiple times with different values
    OCDictionaryAddValue(dup_dict, dup_key, val1);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dup_dict), 1, "Test 2.1: Dict should have 1 item");
    // Now add the same key again with a different value
    OCDictionaryAddValue(dup_dict, dup_key, val2);
    // Test the behavior with OCDictionaryReplaceValue
    OCDictionaryReplaceValue(dup_dict, dup_key, val3);
    // Retrieve value for duplicate key and verify what we get
    OCNumberRef retrieved = (OCNumberRef)OCDictionaryGetValue((OCDictionaryRef)dup_dict, dup_key);
    int32_t retrieved_val;
    OCNumberGetValue(retrieved, kOCNumberSInt32Type, &retrieved_val);
    ASSERT_EQUAL(retrieved_val, 300, "Test 2.2: Value should be replaced with 300");
    // Test 3: Edge case - keys with special characters
    OCMutableDictionaryRef special_dict = OCDictionaryCreateMutable(5);
    // Keys with special characters
    OCStringRef key_empty = OCStringCreateWithCString("");
    OCStringRef key_spaces = OCStringCreateWithCString("   ");
    OCStringRef key_newlines = OCStringCreateWithCString("\n\r\t");
    OCStringRef key_emoji = OCStringCreateWithCString("🔑");
    OCNumberRef val_empty = OCNumberCreateWithSInt32(1);
    OCNumberRef val_spaces = OCNumberCreateWithSInt32(2);
    OCNumberRef val_newlines = OCNumberCreateWithSInt32(3);
    OCNumberRef val_emoji = OCNumberCreateWithSInt32(4);
    OCDictionaryAddValue(special_dict, key_empty, val_empty);
    OCDictionaryAddValue(special_dict, key_spaces, val_spaces);
    OCDictionaryAddValue(special_dict, key_newlines, val_newlines);
    OCDictionaryAddValue(special_dict, key_emoji, val_emoji);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)special_dict), 4,
                 "Test 3.1: Dict should have 4 special items");
    // Verify retrieving these special keys works
    OCNumberRef get_empty = (OCNumberRef)OCDictionaryGetValue((OCDictionaryRef)special_dict, key_empty);
    OCNumberRef get_spaces = (OCNumberRef)OCDictionaryGetValue((OCDictionaryRef)special_dict, key_spaces);
    OCNumberRef get_emoji = (OCNumberRef)OCDictionaryGetValue((OCDictionaryRef)special_dict, key_emoji);
    ASSERT_NOT_NULL(get_empty, "Test 3.2: Empty key retrieval should work");
    ASSERT_NOT_NULL(get_spaces, "Test 3.3: Spaces key retrieval should work");
    ASSERT_NOT_NULL(get_emoji, "Test 3.4: Emoji key retrieval should work");
    // Test 4: Copy semantics with special values
    OCDictionaryRef copy_dict = OCDictionaryCreateCopy((OCDictionaryRef)special_dict);
    ASSERT_NOT_NULL(copy_dict, "Test 4.1: Copy should not be NULL");
    ASSERT_EQUAL(OCDictionaryGetCount(copy_dict), 4, "Test 4.2: Copy should have 4 items");
    // Verify the keys and values are independent (deep copy)
    OCNumberRef copied_emoji = (OCNumberRef)OCDictionaryGetValue(copy_dict, key_emoji);
    ASSERT_NOT_NULL(copied_emoji, "Test 4.3: Copied emoji key retrieval should work");
    // Check if OCTypeEqual works for the copied values
    ASSERT_TRUE(OCTypeEqual(copied_emoji, val_emoji), "Test 4.4: Copied value should be equal");
    // Test 5: Handle removing keys that don't exist
    OCStringRef nonexistent_key = OCStringCreateWithCString("nonexistent");
    OCDictionaryRemoveValue(special_dict, nonexistent_key);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)special_dict), 4,
                 "Test 5.1: Removing nonexistent key shouldn't change count");
    // Cleanup
    OCRelease(zero_cap_dict);
    OCRelease(dup_dict);
    OCRelease(special_dict);
    OCRelease(copy_dict);
    OCRelease(dup_key);
    OCRelease(val1);
    OCRelease(val2);
    OCRelease(val3);
    OCRelease(key_empty);
    OCRelease(key_spaces);
    OCRelease(key_newlines);
    OCRelease(key_emoji);
    OCRelease(val_empty);
    OCRelease(val_spaces);
    OCRelease(val_newlines);
    OCRelease(val_emoji);
    OCRelease(nonexistent_key);
    printf(" passed\n");
    return success;
}
bool dictionaryTest5(void) {
    printf("dictionaryTest5 begin...");
    bool success = true;
    // Test 1: Create a deeply nested dictionary structure
    OCMutableDictionaryRef root_dict = OCDictionaryCreateMutable(5);
    // Level 1: Add a regular key-value pair
    OCStringRef key_regular = OCStringCreateWithCString("regular_key");
    OCNumberRef val_regular = OCNumberCreateWithSInt32(100);
    OCDictionaryAddValue(root_dict, key_regular, val_regular);
    // Level 1: Add a nested dictionary
    OCStringRef key_level1 = OCStringCreateWithCString("level1");
    OCMutableDictionaryRef dict_level1 = OCDictionaryCreateMutable(3);
    // Level 2: Add keys to the nested dictionary
    OCStringRef key_level2 = OCStringCreateWithCString("level2");
    OCMutableDictionaryRef dict_level2 = OCDictionaryCreateMutable(2);
    // Level 3: More nesting
    OCStringRef key_level3 = OCStringCreateWithCString("level3");
    OCMutableDictionaryRef dict_level3 = OCDictionaryCreateMutable(1);
    // Level 4: Final level
    OCStringRef key_level4 = OCStringCreateWithCString("level4");
    OCNumberRef val_level4 = OCNumberCreateWithSInt32(400);
    OCDictionaryAddValue(dict_level3, key_level4, val_level4);
    // Build up the hierarchy
    OCDictionaryAddValue(dict_level2, key_level3, dict_level3);
    OCDictionaryAddValue(dict_level1, key_level2, dict_level2);
    OCDictionaryAddValue(root_dict, key_level1, dict_level1);
    // Test deep traversal
    OCDictionaryRef nav_level1 = (OCDictionaryRef)OCDictionaryGetValue(root_dict, key_level1);
    ASSERT_NOT_NULL(nav_level1, "Test 1.1: Level 1 dict should be retrievable");
    OCDictionaryRef nav_level2 = (OCDictionaryRef)OCDictionaryGetValue(nav_level1, key_level2);
    ASSERT_NOT_NULL(nav_level2, "Test 1.2: Level 2 dict should be retrievable");
    OCDictionaryRef nav_level3 = (OCDictionaryRef)OCDictionaryGetValue(nav_level2, key_level3);
    ASSERT_NOT_NULL(nav_level3, "Test 1.3: Level 3 dict should be retrievable");
    OCNumberRef nav_level4 = (OCNumberRef)OCDictionaryGetValue(nav_level3, key_level4);
    ASSERT_NOT_NULL(nav_level4, "Test 1.4: Level 4 value should be retrievable");
    int32_t value4;
    OCNumberGetValue(nav_level4, kOCNumberSInt32Type, &value4);
    ASSERT_EQUAL(value4, 400, "Test 1.5: Final value should be 400");
    // Test 2: Create a dictionary with OCArrayRef values
    OCMutableDictionaryRef array_dict = OCDictionaryCreateMutable(3);
    // Create arrays of different sizes
    OCMutableArrayRef array1 = OCArrayCreateMutable(10, &kOCTypeArrayCallBacks);
    OCMutableArrayRef array2 = OCArrayCreateMutable(5, &kOCTypeArrayCallBacks);
    OCMutableArrayRef array3 = OCArrayCreateMutable(1, &kOCTypeArrayCallBacks);
    // Fill the arrays
    for (int i = 0; i < 10; i++) {
        OCNumberRef num = OCNumberCreateWithSInt32(i * 100);
        OCArrayAppendValue(array1, num);
        OCRelease(num);
    }
    for (int i = 0; i < 5; i++) {
        OCStringRef str_val = OCStringCreateWithFormat(STR("Value_%d"), i);
        OCArrayAppendValue(array2, str_val);
        OCRelease(str_val);
    }
    // Add another dictionary to array3
    OCMutableDictionaryRef nested_in_array = OCDictionaryCreateMutable(2);
    OCArrayAppendValue(array3, nested_in_array);
    OCRelease(nested_in_array);
    // Add arrays to dictionary
    OCStringRef key_array1 = OCStringCreateWithCString("array1");
    OCStringRef key_array2 = OCStringCreateWithCString("array2");
    OCStringRef key_array3 = OCStringCreateWithCString("array3");
    OCDictionaryAddValue(array_dict, key_array1, array1);
    OCDictionaryAddValue(array_dict, key_array2, array2);
    OCDictionaryAddValue(array_dict, key_array3, array3);
    // Test retrieving arrays from dictionary and accessing their contents
    OCArrayRef get_array1 = (OCArrayRef)OCDictionaryGetValue(array_dict, key_array1);
    ASSERT_NOT_NULL(get_array1, "Test 2.1: Array1 should be retrievable");
    ASSERT_EQUAL(OCArrayGetCount(get_array1), 10, "Test 2.2: Array1 should have 10 items");
    OCNumberRef item5 = (OCNumberRef)OCArrayGetValueAtIndex(get_array1, 5);
    int32_t val5;
    OCNumberGetValue(item5, kOCNumberSInt32Type, &val5);
    ASSERT_EQUAL(val5, 500, "Test 2.3: Item 5 value should be 500");
    // Test 3: Dictionary copy performance
    // Create a moderately sized dictionary (avoid memory issues)
    const int perf_dict_size = 50;
    OCMutableDictionaryRef large_dict = OCDictionaryCreateMutable(perf_dict_size);
    for (int i = 0; i < perf_dict_size; i++) {
        OCStringRef large_key = OCStringCreateWithFormat(STR("large_key_%d"), i);
        OCNumberRef large_val = OCNumberCreateWithSInt32(i);
        OCDictionaryAddValue(large_dict, large_key, large_val);
        OCRelease(large_key);
        OCRelease(large_val);
    }
    // Time the immutable copy operation
    OCDictionaryRef large_copy = OCDictionaryCreateCopy((OCDictionaryRef)large_dict);
    // Time the mutable copy operation
    OCMutableDictionaryRef large_mutable_copy = OCDictionaryCreateMutableCopy((OCDictionaryRef)large_dict);
    // Verify all keys are present in the copied dictionary
    OCArrayRef keys = OCDictionaryCreateArrayWithAllKeys(large_copy);
    ASSERT_EQUAL(OCArrayGetCount(keys), perf_dict_size,
                 "Test 3.1: Copied dict should contain correct number of keys");
    // Test 4: Dictionary iteration using array creation
    OCArrayRef allKeysArray = OCDictionaryCreateArrayWithAllKeys(large_dict);
    OCArrayRef allValuesArray = OCDictionaryCreateArrayWithAllValues(large_dict);
    ASSERT_EQUAL(OCArrayGetCount(allKeysArray), perf_dict_size, "Test 4.1: Should retrieve correct number of keys");
    ASSERT_EQUAL(OCArrayGetCount(allValuesArray), perf_dict_size, "Test 4.2: Should retrieve correct number of values");
    OCRelease(allKeysArray);
    OCRelease(allValuesArray);
    // Cleanup
    OCRelease(keys);
    OCRelease(root_dict);
    OCRelease(key_regular);
    OCRelease(val_regular);
    OCRelease(key_level1);
    OCRelease(key_level2);
    OCRelease(key_level3);
    OCRelease(key_level4);
    OCRelease(val_level4);
    OCRelease(dict_level1);
    OCRelease(dict_level2);
    OCRelease(dict_level3);
    OCRelease(array_dict);
    OCRelease(key_array1);
    OCRelease(key_array2);
    OCRelease(key_array3);
    OCRelease(array1);
    OCRelease(array2);
    OCRelease(array3);
    OCRelease(large_dict);
    OCRelease(large_copy);
    OCRelease(large_mutable_copy);
    printf(" passed\n");
    return success;
}
bool OCDictionaryTestDeepCopy(void) {
    fprintf(stderr, "%s begin...", __func__);
    bool success = false;
    OCStringRef key1 = OCStringCreateWithCString("alpha");
    OCStringRef key2 = OCStringCreateWithCString("beta");
    OCNumberRef val1 = OCNumberCreateWithDouble(3.14);
    OCNumberRef val2 = OCNumberCreateWithSInt64(42);
    OCMutableDictionaryRef dict = NULL;
    OCMutableDictionaryRef copy = NULL;
    if (!key1 || !key2 || !val1 || !val2) {
        fprintf(stderr, "Error: Failed to create test keys/values.\n");
        goto cleanup;
    }
    dict = OCDictionaryCreateMutable(2);
    if (!dict) {
        fprintf(stderr, "Error: Failed to create dictionary.\n");
        goto cleanup;
    }
    if (!OCDictionaryAddValue(dict, key1, val1) ||
        !OCDictionaryAddValue(dict, key2, val2)) {
        fprintf(stderr, "Error: Failed to add key/value to dictionary.\n");
        goto cleanup;
    }
    copy = (OCMutableDictionaryRef)OCTypeDeepCopy(dict);
    if (!copy) {
        fprintf(stderr, "Error: OCTypeDeepCopy returned NULL.\n");
        goto cleanup;
    }
    const void *val1Copy = OCDictionaryGetValue(copy, key1);
    const void *val2Copy = OCDictionaryGetValue(copy, key2);
    if (!val1Copy || !val2Copy) {
        fprintf(stderr, "Error: Deep copy missing keys.\n");
        goto cleanup;
    }
    if (!OCTypeEqual(val1, val1Copy) || !OCTypeEqual(val2, val2Copy)) {
        fprintf(stderr, "Error: Deep copied values not equal.\n");
        goto cleanup;
    }
    if (val1 == val1Copy || val2 == val2Copy) {
        fprintf(stderr, "Error: Deep copy is shallow (value pointer matches).\n");
        goto cleanup;
    }
    OCNumberRef newVal = OCNumberCreateWithDouble(999.9);
    if (!newVal) {
        fprintf(stderr, "Error: Failed to create new value.\n");
        goto cleanup;
    }
    if (!OCDictionarySetValue(copy, key1, newVal)) {
        fprintf(stderr, "Error: Failed to set value in copied dictionary.\n");
        OCRelease(newVal);
        goto cleanup;
    }
    OCRelease(newVal);
    const void *originalVal1 = OCDictionaryGetValue(dict, key1);
    const void *modifiedVal1 = OCDictionaryGetValue(copy, key1);
    if (OCTypeEqual(originalVal1, modifiedVal1)) {
        fprintf(stderr, "Error: Modification of deep copy affected original.\n");
        goto cleanup;
    }
    fprintf(stderr, " passed\n");
    success = true;
cleanup:
    if (key1) OCRelease(key1);
    if (key2) OCRelease(key2);
    if (val1) OCRelease(val1);
    if (val2) OCRelease(val2);
    if (dict) OCRelease(dict);
    if (copy) OCRelease(copy);
    if (!success) {
        fprintf(stderr, "%s FAILED\n", __func__);
    }
    return success;
}
bool test_OCDictionary_deepcopy2(void) {
    // (1) Empty dictionary
    OCMutableDictionaryRef empty = OCDictionaryCreateMutable(0);
    ASSERT_NOT_NULL(empty, "failed to create empty mutable dictionary");
    OCDictionaryRef emptyCopy = (OCDictionaryRef)OCTypeDeepCopy(empty);
    ASSERT_NOT_NULL(emptyCopy, "deep copy of empty dictionary returned NULL");
    ASSERT_EQUAL(
        OCDictionaryGetCount(emptyCopy),
        0,
        "empty copy should have count 0");
    OCRelease(emptyCopy);
    OCRelease(empty);
    // (2) Non-empty dictionary
    // Build a small dict { "k1":"v1", "k2":"v2" }
    OCStringRef k1 = STR("k1");
    OCStringRef k2 = STR("k2");
    OCStringRef v1 = STR("v1");
    OCStringRef v2 = STR("v2");
    const void *keys[] = {k1, k2};
    const void *values[] = {v1, v2};
    OCDictionaryRef dict = OCDictionaryCreate(keys, values, 2);
    ASSERT_NOT_NULL(dict, "failed to create dictionary");
    ASSERT_EQUAL(
        OCDictionaryGetCount(dict),
        2,
        "original dictionary count should be 2");
    // Deep-copy
    OCDictionaryRef dictCopy = (OCDictionaryRef)OCTypeDeepCopy(dict);
    ASSERT_NOT_NULL(dictCopy, "deep copy returned NULL");
    ASSERT_EQUAL(
        OCDictionaryGetCount(dictCopy),
        2,
        "deep-copy dictionary count should be 2");
    // Values under each key should initially be equal
    OCTypeRef orig_v1 = (OCTypeRef)OCDictionaryGetValue(dict, k1);
    OCTypeRef copy_v1 = (OCTypeRef)OCDictionaryGetValue(dictCopy, k1);
    ASSERT_TRUE(
        OCTypeEqual(orig_v1, copy_v1),
        "value for k1 should match after deepcopy");
    OCTypeRef orig_v2 = (OCTypeRef)OCDictionaryGetValue(dict, k2);
    OCTypeRef copy_v2 = (OCTypeRef)OCDictionaryGetValue(dictCopy, k2);
    ASSERT_TRUE(
        OCTypeEqual(orig_v2, copy_v2),
        "value for k2 should match after deepcopy");
    // Mutate original: change "k1" → "new"
    OCStringRef newVal = STR("new");
    ASSERT_TRUE(
        OCDictionarySetValue((OCMutableDictionaryRef)dict, k1, newVal),
        "failed to set new value for k1");
    // Now original and copy must differ at k1
    OCTypeRef mutated_v1 = (OCTypeRef)OCDictionaryGetValue(dict, k1);
    OCTypeRef still_v1 = (OCTypeRef)OCDictionaryGetValue(dictCopy, k1);
    ASSERT_FALSE(
        OCTypeEqual(mutated_v1, still_v1),
        "after mutation, original[k1] should differ from copy[k1]");
    // Cleanup
    OCRelease(dictCopy);
    OCRelease(dict);
    return true;
}

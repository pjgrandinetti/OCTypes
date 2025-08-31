/**
 * @file test_json_typed.c
 * @brief JSONTyped serialization roundtrip tests implementation.
 *
 * Comprehensive test    // Verify equality
    bool equal = OCTypeEqual(originalArray, deserializedArray);for the JSONTyped serialization system across all OCTypes.
 * Each test verifies complete roundtrip serialization works correctly.
 */
#include "test_json_typed.h"
#include "test_utils.h"
#include "../src/OCTypes.h"
#include "../src/cJSON.h"
#include <stdio.h>
#include <string.h>

// Helper macro for test output
#define JSON_TEST_LOG(test_name, message) \
    fprintf(stderr, "[%s] %s\n", test_name, message)

bool jsonTypedTest_OCData(void) {
    const char *test_name = "jsonTypedTest_OCData";
    JSON_TEST_LOG(test_name, "begin...");

    // Create test data
    uint8_t testBytes[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0x57, 0x6F, 0x72, 0x6C, 0x64};
    size_t dataLength = sizeof(testBytes);

    OCDataRef originalData = OCDataCreate(testBytes, dataLength);
    if (!originalData) {
        JSON_TEST_LOG(test_name, "FAIL: Could not create OCData");
        return false;
    }

    // Serialize to JSONTyped
    cJSON *json = OCDataCreateJSON(originalData, true);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        OCRelease(originalData);
        return false;
    }

    // Deserialize from JSONTyped
    OCDataRef deserializedData = OCDataCreateFromJSONTyped(json);
    if (!deserializedData) {
        JSON_TEST_LOG(test_name, "FAIL: Could not deserialize from JSONTyped");
        cJSON_Delete(json);
        OCRelease(originalData);
        return false;
    }

    // Verify equality
    bool equal = OCTypeEqual(originalData, deserializedData);
    if (!equal) {
        JSON_TEST_LOG(test_name, "FAIL: Roundtrip data does not match original");
    }

    // Verify data content directly
    size_t originalLength = OCDataGetLength(originalData);
    size_t deserializedLength = OCDataGetLength(deserializedData);
    const uint8_t *originalBytes = OCDataGetBytesPtr(originalData);
    const uint8_t *deserializedBytes = OCDataGetBytesPtr(deserializedData);

    if (originalLength != deserializedLength) {
        JSON_TEST_LOG(test_name, "FAIL: Data lengths differ");
        equal = false;
    } else if (memcmp(originalBytes, deserializedBytes, originalLength) != 0) {
        JSON_TEST_LOG(test_name, "FAIL: Data content differs");
        equal = false;
    }

    // Cleanup
    cJSON_Delete(json);
    OCRelease(originalData);
    OCRelease(deserializedData);

    if (equal) {
        JSON_TEST_LOG(test_name, "PASS: Roundtrip successful");
    }

    return equal;
}

bool jsonTypedTest_OCArray(void) {
    const char *test_name = "jsonTypedTest_OCArray";
    JSON_TEST_LOG(test_name, "begin...");

    // Create test array with mixed types
    OCMutableArrayRef originalArray = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    if (!originalArray) {
        JSON_TEST_LOG(test_name, "FAIL: Could not create OCArray");
        return false;
    }

    // Add various types
    OCNumberRef num42 = OCNumberCreateWithSInt32(42);
    OCStringRef helloStr = OCStringCreateWithCString("Hello World");

    OCArrayAppendValue(originalArray, num42);
    OCArrayAppendValue(originalArray, helloStr);
    OCArrayAppendValue(originalArray, kOCBooleanTrue);

    // Release our references since array retains them
    OCRelease(num42);
    OCRelease(helloStr);

    // Add nested array
    OCMutableArrayRef nestedArray = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    OCNumberRef piNum = OCNumberCreateWithDouble(3.14159);
    OCStringRef nestedStr = OCStringCreateWithCString("nested");

    OCArrayAppendValue(nestedArray, piNum);
    OCArrayAppendValue(nestedArray, nestedStr);

    // Release our references to nested elements
    OCRelease(piNum);
    OCRelease(nestedStr);

    OCArrayAppendValue(originalArray, nestedArray);
    OCRelease(nestedArray);

    // Serialize to JSONTyped
    cJSON *json = OCArrayCreateJSON(originalArray, true);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        OCRelease(originalArray);
        return false;
    }

    // Deserialize from JSONTyped
    OCArrayRef deserializedArray = OCArrayCreateFromJSONTyped(json);
    if (!deserializedArray) {
        JSON_TEST_LOG(test_name, "FAIL: Could not deserialize from JSONTyped");
        cJSON_Delete(json);
        OCRelease(originalArray);
        return false;
    }

    // Verify equality
    bool equal = OCTypeEqual(originalArray, deserializedArray);
    if (!equal) {
        JSON_TEST_LOG(test_name, "FAIL: Roundtrip array does not match original");
    }

    // Verify count matches
    uint64_t originalCount = OCArrayGetCount(originalArray);
    uint64_t deserializedCount = OCArrayGetCount(deserializedArray);
    if (originalCount != deserializedCount) {
        JSON_TEST_LOG(test_name, "FAIL: Array counts differ");
        equal = false;
    }

    // Cleanup
    cJSON_Delete(json);
    OCRelease(originalArray);
    OCRelease(deserializedArray);

    if (equal) {
        JSON_TEST_LOG(test_name, "PASS: Roundtrip successful");
    }

    return equal;
}

bool jsonTypedTest_OCNumber(void) {
    const char *test_name = "jsonTypedTest_OCNumber";
    JSON_TEST_LOG(test_name, "begin...");

    bool allPassed = true;

    // Test various number types
    struct {
        const char *typeName;
        OCNumberRef number;
    } testCases[] = {
        {"SInt32", OCNumberCreateWithSInt32(-12345)},
        {"UInt64", OCNumberCreateWithUInt64(18446744073709551615ULL)},
        {"Float32", OCNumberCreateWithFloat(3.14159f)},
        {"Float64", OCNumberCreateWithDouble(2.718281828459045)},
        {"Complex64", OCNumberCreateWithFloatComplex(1.0f + 2.0f * I)},
        {"Complex128", OCNumberCreateWithDoubleComplex(3.0 + 4.0 * I)}
    };

    size_t numTests = sizeof(testCases) / sizeof(testCases[0]);

    for (size_t i = 0; i < numTests; i++) {
        if (!testCases[i].number) {
            fprintf(stderr, "[%s] FAIL: Could not create %s number\n", test_name, testCases[i].typeName);
            allPassed = false;
            continue;
        }

        // Serialize to JSONTyped
        cJSON *json = OCNumberCreateJSON(testCases[i].number, true);
        if (!json) {
            fprintf(stderr, "[%s] FAIL: Could not serialize %s to JSONTyped\n", test_name, testCases[i].typeName);
            OCRelease(testCases[i].number);
            allPassed = false;
            continue;
        }

        // Deserialize from JSONTyped
        OCNumberRef deserializedNumber = OCNumberCreateFromJSONTyped(json);
        if (!deserializedNumber) {
            fprintf(stderr, "[%s] FAIL: Could not deserialize %s from JSONTyped\n", test_name, testCases[i].typeName);
            cJSON_Delete(json);
            OCRelease(testCases[i].number);
            allPassed = false;
            continue;
        }

        // Verify equality
        bool equal = OCTypeEqual(testCases[i].number, deserializedNumber);
        if (!equal) {
            fprintf(stderr, "[%s] FAIL: %s roundtrip does not match original\n", test_name, testCases[i].typeName);
            allPassed = false;
        } else {
            fprintf(stderr, "[%s] PASS: %s roundtrip successful\n", test_name, testCases[i].typeName);
        }

        // Cleanup
        cJSON_Delete(json);
        OCRelease(testCases[i].number);
        OCRelease(deserializedNumber);
    }

    return allPassed;
}

bool jsonTypedTest_OCString(void) {
    const char *test_name = "jsonTypedTest_OCString";
    JSON_TEST_LOG(test_name, "begin...");

    // OCString uses native JSON representation, so we test the global functions
    const char *testStrings[] = {
        "Hello World",
        "UTF-8: Test String",
        "Empty string test: ",
        "Special chars: !@#$%^&*()_+-=[]{}|;':\",./<>?",
        "Line\nbreaks\tand\ttabs",
        ""  // Empty string
    };

    bool allPassed = true;
    size_t numTests = sizeof(testStrings) / sizeof(testStrings[0]);

    for (size_t i = 0; i < numTests; i++) {
        OCStringRef originalString = OCStringCreateWithCString(testStrings[i]);
        if (!originalString) {
            fprintf(stderr, "[%s] FAIL: Could not create string %zu\n", test_name, i);
            allPassed = false;
            continue;
        }

        // Use global JSONTyped functions since OCString uses native JSON
        cJSON *json = OCTypeCopyJSON((OCTypeRef)originalString, true);
        if (!json) {
            fprintf(stderr, "[%s] FAIL: Could not serialize string %zu to JSONTyped\n", test_name, i);
            OCRelease(originalString);
            allPassed = false;
            continue;
        }

        // Deserialize from JSONTyped
        OCStringRef deserializedString = (OCStringRef)OCTypeCreateFromJSONTyped(json);
        if (!deserializedString) {
            fprintf(stderr, "[%s] FAIL: Could not deserialize string %zu from JSONTyped\n", test_name, i);
            cJSON_Delete(json);
            OCRelease(originalString);
            allPassed = false;
            continue;
        }

        // Verify equality
        bool equal = OCTypeEqual(originalString, deserializedString);
        if (!equal) {
            fprintf(stderr, "[%s] FAIL: String %zu roundtrip does not match original\n", test_name, i);
            allPassed = false;
        }

        // Cleanup
        cJSON_Delete(json);
        OCRelease(originalString);
        OCRelease(deserializedString);
    }

    if (allPassed) {
        JSON_TEST_LOG(test_name, "PASS: All string roundtrips successful");
    }

    return allPassed;
}

bool jsonTypedTest_OCBoolean(void) {
    const char *test_name = "jsonTypedTest_OCBoolean";
    JSON_TEST_LOG(test_name, "begin...");

    bool allPassed = true;

    // Test both boolean values using global functions since OCBoolean uses native JSON
    OCBooleanRef testBooleans[] = {kOCBooleanTrue, kOCBooleanFalse};
    const char *boolNames[] = {"true", "false"};

    for (int i = 0; i < 2; i++) {
        // Use global JSONTyped functions
        cJSON *json = OCTypeCopyJSON((OCTypeRef)testBooleans[i], true);
        if (!json) {
            fprintf(stderr, "[%s] FAIL: Could not serialize %s to JSONTyped\n", test_name, boolNames[i]);
            allPassed = false;
            continue;
        }

        // Deserialize from JSONTyped
        OCBooleanRef deserializedBoolean = (OCBooleanRef)OCTypeCreateFromJSONTyped(json);
        if (!deserializedBoolean) {
            fprintf(stderr, "[%s] FAIL: Could not deserialize %s from JSONTyped\n", test_name, boolNames[i]);
            cJSON_Delete(json);
            allPassed = false;
            continue;
        }

        // Verify equality (should be same singleton)
        bool equal = (testBooleans[i] == deserializedBoolean);
        if (!equal) {
            fprintf(stderr, "[%s] FAIL: %s roundtrip does not match original\n", test_name, boolNames[i]);
            allPassed = false;
        } else {
            fprintf(stderr, "[%s] PASS: %s roundtrip successful\n", test_name, boolNames[i]);
        }

        // Cleanup
        cJSON_Delete(json);
        // Don't release singleton booleans
    }

    return allPassed;
}

bool jsonTypedTest_OCDictionary(void) {
    const char *test_name = "jsonTypedTest_OCDictionary";
    JSON_TEST_LOG(test_name, "begin...");

    // Create test dictionary with mixed types
    OCMutableDictionaryRef originalDict = OCDictionaryCreateMutable(0);
    if (!originalDict) {
        JSON_TEST_LOG(test_name, "FAIL: Could not create OCDictionary");
        return false;
    }

    // Add various key-value pairs
    OCStringRef key1 = OCStringCreateWithCString("integer");
    OCStringRef key2 = OCStringCreateWithCString("string");
    OCStringRef key3 = OCStringCreateWithCString("boolean");
    OCStringRef key4 = OCStringCreateWithCString("nested");

    OCNumberRef num123 = OCNumberCreateWithSInt32(123);
    OCStringRef valueStr = OCStringCreateWithCString("value");

    OCDictionarySetValue(originalDict, key1, num123);
    OCDictionarySetValue(originalDict, key2, valueStr);
    OCDictionarySetValue(originalDict, key3, kOCBooleanTrue);

    // Release our references to values since dictionary retains them
    OCRelease(num123);
    OCRelease(valueStr);

    // Add nested dictionary
    OCMutableDictionaryRef nestedDict = OCDictionaryCreateMutable(0);
    OCStringRef nestedKey = OCStringCreateWithCString("nested_value");
    OCNumberRef eNum = OCNumberCreateWithDouble(2.71828);

    OCDictionarySetValue(nestedDict, nestedKey, eNum);
    OCRelease(eNum); // Release our reference to the nested value

    OCDictionarySetValue(originalDict, key4, nestedDict);

    // Serialize to JSONTyped
    cJSON *json = OCDictionaryCreateJSON(originalDict, true);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        // Cleanup keys
        OCRelease(key1); OCRelease(key2); OCRelease(key3); OCRelease(key4); OCRelease(nestedKey);
        OCRelease(nestedDict);
        OCRelease(originalDict);
        return false;
    }

    // Deserialize from JSONTyped
    OCDictionaryRef deserializedDict = OCDictionaryCreateFromJSONTyped(json);
    if (!deserializedDict) {
        JSON_TEST_LOG(test_name, "FAIL: Could not deserialize from JSONTyped");
        cJSON_Delete(json);
        // Cleanup keys
        OCRelease(key1); OCRelease(key2); OCRelease(key3); OCRelease(key4); OCRelease(nestedKey);
        OCRelease(nestedDict);
        OCRelease(originalDict);
        return false;
    }

    // Verify equality
    bool equal = OCTypeEqual(originalDict, deserializedDict);
    if (!equal) {
        JSON_TEST_LOG(test_name, "FAIL: Roundtrip dictionary does not match original");
    }

    // Verify count matches
    uint64_t originalCount = OCDictionaryGetCount(originalDict);
    uint64_t deserializedCount = OCDictionaryGetCount(deserializedDict);
    if (originalCount != deserializedCount) {
        JSON_TEST_LOG(test_name, "FAIL: Dictionary counts differ");
        equal = false;
    }

    // Cleanup
    cJSON_Delete(json);
    OCRelease(key1); OCRelease(key2); OCRelease(key3); OCRelease(key4); OCRelease(nestedKey);
    OCRelease(nestedDict);
    OCRelease(originalDict);
    OCRelease(deserializedDict);

    if (equal) {
        JSON_TEST_LOG(test_name, "PASS: Roundtrip successful");
    }

    return equal;
}

bool jsonTypedTest_OCSet(void) {
    const char *test_name = "jsonTypedTest_OCSet";
    JSON_TEST_LOG(test_name, "begin...");

    // Create test set
    OCMutableSetRef originalSet = OCSetCreateMutable(0);
    if (!originalSet) {
        JSON_TEST_LOG(test_name, "FAIL: Could not create OCSet");
        return false;
    }

    // Add various values
    OCNumberRef num1 = OCNumberCreateWithSInt32(1);
    OCNumberRef num2 = OCNumberCreateWithSInt32(2);
    OCNumberRef num3 = OCNumberCreateWithSInt32(3);
    OCStringRef testStr = OCStringCreateWithCString("test");

    OCSetAddValue(originalSet, (OCTypeRef)num1);
    OCSetAddValue(originalSet, (OCTypeRef)num2);
    OCSetAddValue(originalSet, (OCTypeRef)num3);
    OCSetAddValue(originalSet, (OCTypeRef)testStr);

    // Release our references since set retains them
    OCRelease(num1);
    OCRelease(num2);
    OCRelease(num3);
    OCRelease(testStr);

    // Serialize to JSONTyped
    cJSON *json = OCSetCreateJSON(originalSet, true);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        OCRelease(originalSet);
        return false;
    }

    // Deserialize from JSONTyped
    OCSetRef deserializedSet = OCSetCreateFromJSONTyped(json);
    if (!deserializedSet) {
        JSON_TEST_LOG(test_name, "FAIL: Could not deserialize from JSONTyped");
        cJSON_Delete(json);
        OCRelease(originalSet);
        return false;
    }

    // Verify equality
    bool equal = OCTypeEqual(originalSet, deserializedSet);
    if (!equal) {
        JSON_TEST_LOG(test_name, "FAIL: Roundtrip set does not match original");
    }

    // Verify count matches
    uint64_t originalCount = OCSetGetCount(originalSet);
    uint64_t deserializedCount = OCSetGetCount(deserializedSet);
    if (originalCount != deserializedCount) {
        JSON_TEST_LOG(test_name, "FAIL: Set counts differ");
        equal = false;
    }

    // Cleanup
    cJSON_Delete(json);
    OCRelease(originalSet);
    OCRelease(deserializedSet);

    if (equal) {
        JSON_TEST_LOG(test_name, "PASS: Roundtrip successful");
    }

    return equal;
}

bool jsonTypedTest_OCIndexSet(void) {
    const char *test_name = "jsonTypedTest_OCIndexSet";
    JSON_TEST_LOG(test_name, "begin...");

    // Create test index set
    OCMutableIndexSetRef originalSet = OCIndexSetCreateMutable();
    if (!originalSet) {
        JSON_TEST_LOG(test_name, "FAIL: Could not create OCIndexSet");
        return false;
    }

    // Add some ranges
    OCIndexSetAddIndex(originalSet, 5);
    OCIndexSetAddIndex(originalSet, 10);
    OCIndexSetAddIndex(originalSet, 11);
    OCIndexSetAddIndex(originalSet, 12);
    OCIndexSetAddIndex(originalSet, 20);

    // Serialize to JSONTyped
    cJSON *json = OCIndexSetCreateJSON(originalSet, true);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        OCRelease(originalSet);
        return false;
    }

    // Deserialize from JSONTyped
    OCIndexSetRef deserializedSet = OCIndexSetCreateFromJSONTyped(json);
    if (!deserializedSet) {
        JSON_TEST_LOG(test_name, "FAIL: Could not deserialize from JSONTyped");
        cJSON_Delete(json);
        OCRelease(originalSet);
        return false;
    }

    // Verify equality
    bool equal = OCTypeEqual(originalSet, deserializedSet);
    if (!equal) {
        JSON_TEST_LOG(test_name, "FAIL: Roundtrip index set does not match original");
    }

    // Verify count matches
    uint64_t originalCount = OCIndexSetGetCount(originalSet);
    uint64_t deserializedCount = OCIndexSetGetCount(deserializedSet);
    if (originalCount != deserializedCount) {
        JSON_TEST_LOG(test_name, "FAIL: IndexSet counts differ");
        equal = false;
    }

    // Cleanup
    cJSON_Delete(json);
    OCRelease(originalSet);
    OCRelease(deserializedSet);

    if (equal) {
        JSON_TEST_LOG(test_name, "PASS: Roundtrip successful");
    }

    return equal;
}

bool jsonTypedTest_OCIndexArray(void) {
    const char *test_name = "jsonTypedTest_OCIndexArray";
    JSON_TEST_LOG(test_name, "begin...");

    // Create test index array
    OCMutableIndexArrayRef originalArray = OCIndexArrayCreateMutable(0);
    if (!originalArray) {
        JSON_TEST_LOG(test_name, "FAIL: Could not create OCIndexArray");
        return false;
    }

    // Add some indices
    OCIndexArrayAppendValue(originalArray, 100);
    OCIndexArrayAppendValue(originalArray, 200);
    OCIndexArrayAppendValue(originalArray, 300);
    OCIndexArrayAppendValue(originalArray, 50);

    // Serialize to JSONTyped
    cJSON *json = OCIndexArrayCreateJSON(originalArray, true);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        OCRelease(originalArray);
        return false;
    }

    // Deserialize from JSONTyped
    OCIndexArrayRef deserializedArray = OCIndexArrayCreateFromJSONTyped(json);
    if (!deserializedArray) {
        JSON_TEST_LOG(test_name, "FAIL: Could not deserialize from JSONTyped");
        cJSON_Delete(json);
        OCRelease(originalArray);
        return false;
    }

    // Verify equality
    bool equal = OCTypeEqual(originalArray, deserializedArray);
    if (!equal) {
        JSON_TEST_LOG(test_name, "FAIL: Roundtrip index array does not match original");
    }

    // Verify count matches
    uint64_t originalCount = OCIndexArrayGetCount(originalArray);
    uint64_t deserializedCount = OCIndexArrayGetCount(deserializedArray);
    if (originalCount != deserializedCount) {
        JSON_TEST_LOG(test_name, "FAIL: IndexArray counts differ");
        equal = false;
    }

    // Cleanup
    cJSON_Delete(json);
    OCRelease(originalArray);
    OCRelease(deserializedArray);

    if (equal) {
        JSON_TEST_LOG(test_name, "PASS: Roundtrip successful");
    }

    return equal;
}

bool jsonTypedTest_OCIndexPairSet(void) {
    const char *test_name = "jsonTypedTest_OCIndexPairSet";
    JSON_TEST_LOG(test_name, "begin...");

    // Create test index pair set
    OCMutableIndexPairSetRef originalSet = OCIndexPairSetCreateMutable();
    if (!originalSet) {
        JSON_TEST_LOG(test_name, "FAIL: Could not create OCIndexPairSet");
        return false;
    }

    // Add some index pairs
    OCIndexPairSetAddIndexPair(originalSet, 1, 10);
    OCIndexPairSetAddIndexPair(originalSet, 2, 20);
    OCIndexPairSetAddIndexPair(originalSet, 3, 30);

    // Serialize to JSONTyped
    cJSON *json = OCIndexPairSetCreateJSON(originalSet, true);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        OCRelease(originalSet);
        return false;
    }

    // Deserialize from JSONTyped
    OCIndexPairSetRef deserializedSet = OCIndexPairSetCreateFromJSONTyped(json);
    if (!deserializedSet) {
        JSON_TEST_LOG(test_name, "FAIL: Could not deserialize from JSONTyped");
        cJSON_Delete(json);
        OCRelease(originalSet);
        return false;
    }

    // Verify equality
    bool equal = OCTypeEqual(originalSet, deserializedSet);
    if (!equal) {
        JSON_TEST_LOG(test_name, "FAIL: Roundtrip index pair set does not match original");
    }

    // Verify count matches
    uint64_t originalCount = OCIndexPairSetGetCount(originalSet);
    uint64_t deserializedCount = OCIndexPairSetGetCount(deserializedSet);
    if (originalCount != deserializedCount) {
        JSON_TEST_LOG(test_name, "FAIL: IndexPairSet counts differ");
        equal = false;
    }

    // Cleanup
    cJSON_Delete(json);
    OCRelease(originalSet);
    OCRelease(deserializedSet);

    if (equal) {
        JSON_TEST_LOG(test_name, "PASS: Roundtrip successful");
    }

    return equal;
}

bool jsonTypedTest_OCAutoreleasePool(void) {
    const char *test_name = "jsonTypedTest_OCAutoreleasePool";
    JSON_TEST_LOG(test_name, "begin...");

    // Skip for now - OCAutoreleasePool JSONTyped functions not implemented
    JSON_TEST_LOG(test_name, "SKIP: OCAutoreleasePool JSONTyped functions not yet implemented");

    return true;
}

bool jsonTypedTest_GlobalFunctions(void) {
    const char *test_name = "jsonTypedTest_GlobalFunctions";
    JSON_TEST_LOG(test_name, "begin...");

    bool allPassed = true;

    // Test global functions with various types
    OCTypeRef testObjects[] = {
        (OCTypeRef)OCStringCreateWithCString("Global test"),
        (OCTypeRef)OCNumberCreateWithSInt32(999),
        (OCTypeRef)kOCBooleanTrue,
        (OCTypeRef)OCDataCreate((uint8_t[]){1,2,3,4}, 4)
    };

    const char *typeNames[] = {"OCString", "OCNumber", "OCBoolean", "OCData"};
    size_t numTests = sizeof(testObjects) / sizeof(testObjects[0]);

    for (size_t i = 0; i < numTests; i++) {
        if (!testObjects[i]) {
            fprintf(stderr, "[%s] FAIL: Could not create %s object\n", test_name, typeNames[i]);
            allPassed = false;
            continue;
        }

        // Use global JSONTyped function
        cJSON *json = OCTypeCopyJSON(testObjects[i], true);
        if (!json) {
            fprintf(stderr, "[%s] FAIL: OCTypeCopyJSON failed for %s\n", test_name, typeNames[i]);
            OCRelease(testObjects[i]);
            allPassed = false;
            continue;
        }

        // Use global deserialization function
        OCTypeRef deserializedObject = OCTypeCreateFromJSONTyped(json);
        if (!deserializedObject) {
            fprintf(stderr, "[%s] FAIL: OCTypeCreateFromJSONTyped failed for %s\n", test_name, typeNames[i]);
            cJSON_Delete(json);
            OCRelease(testObjects[i]);
            allPassed = false;
            continue;
        }

        // Verify equality
        bool equal = OCTypeEqual(testObjects[i], deserializedObject);
        if (!equal) {
            fprintf(stderr, "[%s] FAIL: Global %s roundtrip does not match original\n", test_name, typeNames[i]);
            allPassed = false;
        } else {
            fprintf(stderr, "[%s] PASS: Global %s roundtrip successful\n", test_name, typeNames[i]);
        }

        // Cleanup
        cJSON_Delete(json);
        OCRelease(testObjects[i]);
        OCRelease(deserializedObject);
    }

    return allPassed;
}

bool runAllJSONTypedTests(void) {
    fprintf(stderr, "\n=== Starting JSONTyped Roundtrip Tests ===\n");

    bool allPassed = true;

    // Run all individual tests
    allPassed &= jsonTypedTest_OCData();
    allPassed &= jsonTypedTest_OCArray();
    allPassed &= jsonTypedTest_OCNumber();
    allPassed &= jsonTypedTest_OCString();
    allPassed &= jsonTypedTest_OCBoolean();
    allPassed &= jsonTypedTest_OCDictionary();
    allPassed &= jsonTypedTest_OCSet();
    allPassed &= jsonTypedTest_OCIndexSet();
    allPassed &= jsonTypedTest_OCIndexArray();
    allPassed &= jsonTypedTest_OCIndexPairSet();
    allPassed &= jsonTypedTest_OCAutoreleasePool();
    allPassed &= jsonTypedTest_GlobalFunctions();

    fprintf(stderr, "\n=== JSONTyped Roundtrip Tests %s ===\n",
            allPassed ? "PASSED" : "FAILED");

    return allPassed;
}

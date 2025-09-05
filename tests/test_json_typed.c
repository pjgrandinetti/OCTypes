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
#define JSON_TEST_LOG(test_name, ...) \
    fprintf(stderr, "%s ", test_name); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")

bool jsonTypedTest_OCData(void) {
    const char *test_name = "jsonTypedTest_OCData";
    fprintf(stderr, "%s begin...", test_name);

    // Create test data
    uint8_t testBytes[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0x57, 0x6F, 0x72, 0x6C, 0x64};
    size_t dataLength = sizeof(testBytes);

    OCDataRef originalData = OCDataCreate(testBytes, dataLength);
    if (!originalData) {
        JSON_TEST_LOG(test_name, "FAIL: Could not create OCData");
        return false;
    }

    // Serialize to JSONTyped
    OCStringRef dataError = NULL;
    cJSON *json = OCDataCopyAsJSON(originalData, true, &dataError);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        if (dataError) {
            JSON_TEST_LOG(test_name, "Error: %s", OCStringGetCString(dataError));
        }
        OCRelease(originalData);
        return false;
    }

    // Deserialize from JSONTyped
    OCDataRef deserializedData = OCDataCreateFromJSON(json, NULL);
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
        fprintf(stderr, " passed\n");
    } else {
        JSON_TEST_LOG(test_name, "FAILED");
    }

    return equal;
}

bool jsonTypedTest_OCArray(void) {
    const char *test_name = "jsonTypedTest_OCArray";
    fprintf(stderr, "%s begin...", test_name);

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
    OCStringRef arrayError = NULL;
    cJSON *json = OCArrayCopyAsJSON(originalArray, true, &arrayError);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        if (arrayError) {
            JSON_TEST_LOG(test_name, "Error: %s", OCStringGetCString(arrayError));
        }
        OCRelease(originalArray);
        return false;
    }

    // Deserialize from JSONTyped
    OCArrayRef deserializedArray = OCArrayCreateFromJSONTyped(json, NULL);
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
        fprintf(stderr, " passed\n");
    } else {
        JSON_TEST_LOG(test_name, "FAILED");
    }

    return equal;
}

bool jsonTypedTest_OCNumber(void) {
    const char *test_name = "jsonTypedTest_OCNumber";
    fprintf(stderr, "%s begin...", test_name);

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
        OCStringRef jsonTypedError = NULL;
        cJSON *json = OCNumberCopyAsJSON(testCases[i].number, true, &jsonTypedError);
        if (!json) {
            fprintf(stderr, "[%s] FAIL: Could not serialize %s to JSONTyped\n", test_name, testCases[i].typeName);
            if (jsonTypedError) {
                fprintf(stderr, "[%s] Error: %s\n", test_name, OCStringGetCString(jsonTypedError));
            }
            OCRelease(testCases[i].number);
            allPassed = false;
            continue;
        }

        // Deserialize from JSONTyped
        OCNumberRef deserializedNumber = OCNumberCreateFromJSONTyped(json, NULL);
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
        }

        // Cleanup
        cJSON_Delete(json);
        OCRelease(testCases[i].number);
        OCRelease(deserializedNumber);
    }

    if (allPassed) {
        fprintf(stderr, " passed\n");
    } else {
        JSON_TEST_LOG(test_name, "FAILED");
    }

    return allPassed;
}

bool jsonTypedTest_OCString(void) {
    const char *test_name = "jsonTypedTest_OCString";
    fprintf(stderr, "%s begin...", test_name);

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
        OCStringRef stringError = NULL;
        cJSON *json = OCTypeCopyJSON((OCTypeRef)originalString, true, &stringError);
        if (!json) {
            fprintf(stderr, "[%s] FAIL: Could not serialize string %zu to JSONTyped\n", test_name, i);
            if (stringError) {
                fprintf(stderr, "[%s] Error: %s\n", test_name, OCStringGetCString(stringError));
            }
            OCRelease(originalString);
            allPassed = false;
            continue;
        }

        // Deserialize from JSONTyped
        OCStringRef deserializedString = (OCStringRef)OCTypeCreateFromJSONTyped(json, NULL);
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
        fprintf(stderr, " passed\n");
    } else {
        JSON_TEST_LOG(test_name, "FAILED");
    }

    return allPassed;
}

bool jsonTypedTest_OCBoolean(void) {
    const char *test_name = "jsonTypedTest_OCBoolean";
    fprintf(stderr, "%s begin...", test_name);

    bool allPassed = true;

    // Test both boolean values using global functions since OCBoolean uses native JSON
    OCBooleanRef testBooleans[] = {kOCBooleanTrue, kOCBooleanFalse};
    const char *boolNames[] = {"true", "false"};

    for (int i = 0; i < 2; i++) {
        // Use global JSONTyped functions
        OCStringRef boolError = NULL;
        cJSON *json = OCTypeCopyJSON((OCTypeRef)testBooleans[i], true, &boolError);
        if (!json) {
            fprintf(stderr, "[%s] FAIL: Could not serialize %s to JSONTyped\n", test_name, boolNames[i]);
            if (boolError) {
                fprintf(stderr, "[%s] Error: %s\n", test_name, OCStringGetCString(boolError));
            }
            allPassed = false;
            continue;
        }

        // Deserialize from JSONTyped
        OCBooleanRef deserializedBoolean = (OCBooleanRef)OCTypeCreateFromJSONTyped(json, NULL);
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
        }

        // Cleanup
        cJSON_Delete(json);
        // Don't release singleton booleans
    }

    if (allPassed) {
        fprintf(stderr, " passed\n");
    } else {
        JSON_TEST_LOG(test_name, "FAILED");
    }

    return allPassed;
}

bool jsonTypedTest_OCDictionary(void) {
    const char *test_name = "jsonTypedTest_OCDictionary";
    fprintf(stderr, "%s begin...", test_name);

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
    OCStringRef dictError = NULL;
    cJSON *json = OCDictionaryCopyAsJSON(originalDict, true, &dictError);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        if (dictError) {
            JSON_TEST_LOG(test_name, "Error: %s", OCStringGetCString(dictError));
        }
        // Cleanup keys
        OCRelease(key1); OCRelease(key2); OCRelease(key3); OCRelease(key4); OCRelease(nestedKey);
        OCRelease(nestedDict);
        OCRelease(originalDict);
        return false;
    }

    // Deserialize from JSONTyped
    OCDictionaryRef deserializedDict = OCDictionaryCreateFromJSONTyped(json, NULL);
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
        fprintf(stderr, " passed\n");
    } else {
        JSON_TEST_LOG(test_name, "FAILED");
    }

    return equal;
}

bool jsonTypedTest_OCSet(void) {
    const char *test_name = "jsonTypedTest_OCSet";
    fprintf(stderr, "%s begin...", test_name);

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
    OCStringRef error = NULL;
    cJSON *json = OCSetCopyAsJSON(originalSet, true, &error);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        if (error) {
            JSON_TEST_LOG(test_name, "Error: %s", OCStringGetCString(error));
        }
        OCRelease(originalSet);
        return false;
    }

    // Deserialize from JSONTyped
    OCSetRef deserializedSet = OCSetCreateFromJSONTyped(json, NULL);
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
        fprintf(stderr, " passed\n");
    } else {
        JSON_TEST_LOG(test_name, "FAILED");
    }

    return equal;
}

bool jsonTypedTest_OCIndexSet(void) {
    const char *test_name = "jsonTypedTest_OCIndexSet";
    fprintf(stderr, "%s begin...", test_name);

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
    OCStringRef error = NULL;
    cJSON *json = OCIndexSetCopyAsJSON(originalSet, true, &error);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        if (error) {
            JSON_TEST_LOG(test_name, "Error: %s", OCStringGetCString(error));
        }
        OCRelease(originalSet);
        return false;
    }

    // Deserialize from JSONTyped
    OCIndexSetRef deserializedSet = OCIndexSetCreateFromJSON(json, NULL);
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
        fprintf(stderr, " passed\n");
    } else {
        JSON_TEST_LOG(test_name, "FAILED");
    }

    return equal;
}

bool jsonTypedTest_OCIndexArray(void) {
    const char *test_name = "jsonTypedTest_OCIndexArray";
    fprintf(stderr, "%s begin...", test_name);

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
    OCStringRef error = NULL;
    cJSON *json = OCIndexArrayCopyAsJSON(originalArray, true, &error);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        if (error) {
            JSON_TEST_LOG(test_name, "Error: %s", OCStringGetCString(error));
        }
        OCRelease(originalArray);
        return false;
    }

    // Deserialize from JSONTyped
    OCIndexArrayRef deserializedArray = OCIndexArrayCreateFromJSON(json, NULL);
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
        fprintf(stderr, " passed\n");
    } else {
        JSON_TEST_LOG(test_name, "FAILED");
    }

    return equal;
}

bool jsonTypedTest_OCIndexPairSet(void) {
    const char *test_name = "jsonTypedTest_OCIndexPairSet";
    fprintf(stderr, "%s begin...", test_name);

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
    OCStringRef error = NULL;
    cJSON *json = OCIndexPairSetCopyAsJSON(originalSet, true, &error);
    if (!json) {
        JSON_TEST_LOG(test_name, "FAIL: Could not serialize to JSONTyped");
        if (error) {
            JSON_TEST_LOG(test_name, "Error: %s", OCStringGetCString(error));
        }
        OCRelease(originalSet);
        return false;
    }

    // Deserialize from JSONTyped
    OCIndexPairSetRef deserializedSet = OCIndexPairSetCreateFromJSON(json, NULL);
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
        fprintf(stderr, " passed\n");
    } else {
        JSON_TEST_LOG(test_name, "FAILED");
    }

    return equal;
}

bool jsonTypedTest_GlobalFunctions(void) {
    const char *test_name = "jsonTypedTest_GlobalFunctions";
    fprintf(stderr, "%s begin...", test_name);

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
        OCStringRef typeError = NULL;
        cJSON *json = OCTypeCopyJSON(testObjects[i], true, &typeError);
        if (!json) {
            fprintf(stderr, "[%s] FAIL: OCTypeCopyJSON failed for %s\n", test_name, typeNames[i]);
            if (typeError) {
                fprintf(stderr, "[%s] Error: %s\n", test_name, OCStringGetCString(typeError));
            }
            OCRelease(testObjects[i]);
            allPassed = false;
            continue;
        }

        // Use global deserialization function
        OCTypeRef deserializedObject = OCTypeCreateFromJSONTyped(json, NULL);
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
        }

        // Cleanup
        cJSON_Delete(json);
        OCRelease(testObjects[i]);
        OCRelease(deserializedObject);
    }

    if (allPassed) {
        fprintf(stderr, " passed\n");
    } else {
        JSON_TEST_LOG(test_name, "FAILED");
    }

    return allPassed;
}

bool runAllJSONTypedTests(void) {
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
    allPassed &= jsonTypedTest_GlobalFunctions();

    return allPassed;
}

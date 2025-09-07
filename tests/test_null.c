//
//  test_null.c
//  OCTypes
//
//  OCNull test implementations
//
#include "test_null.h"
#include "../src/OCTypes.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
bool test_OCNull_singleton(void) {
    const char* test_name = "test_OCNull_singleton";
    fprintf(stderr, "%s begin...", test_name);
    // Test singleton behavior
    OCNullRef null1 = kOCNull;
    OCNullRef null2 = kOCNull;
    if (null1 != null2) {
        fprintf(stderr, "OCNull singleton instances are not identical\n");
        return false;
    }
    if (!OCNullIsNull((OCTypeRef)null1)) {
        fprintf(stderr, "OCNullIsNull failed for kOCNull\n");
        return false;
    }
    if (!OCNullIsNull((OCTypeRef)null2)) {
        fprintf(stderr, "OCNullIsNull failed for second kOCNull reference\n");
        return false;
    }
    // Test OCNullGet() function
    OCNullRef null3 = OCNullGet();
    if (null3 != kOCNull) {
        fprintf(stderr, "OCNullGet() did not return kOCNull\n");
        return false;
    }
    // Test type ID
    OCTypeID nullTypeID = OCNullGetTypeID();
    if (OCGetTypeID((OCTypeRef)null1) != nullTypeID) {
        fprintf(stderr, "OCNull type ID mismatch\n");
        return false;
    }
    fprintf(stderr, " passed\n");
    return true;
}
bool test_OCNull_json_serialization(void) {
    const char* test_name = "test_OCNull_json_serialization";
    fprintf(stderr, "%s begin...", test_name);
    OCNullRef null_obj = kOCNull;
    // Test untyped serialization
    OCStringRef error = NULL;
    cJSON* json_untyped = OCNullCopyAsJSON(null_obj, false, &error);
    if (!json_untyped) {
        fprintf(stderr, "Failed to serialize OCNull in untyped mode\n");
        if (error) {
            fprintf(stderr, "Error: %s\n", OCStringGetCString(error));
        }
        return false;
    }
    if (!cJSON_IsNull(json_untyped)) {
        fprintf(stderr, "Untyped OCNull serialization did not produce JSON null\n");
        cJSON_Delete(json_untyped);
        return false;
    }
    // Test typed serialization (should be the same for OCNull)
    error = NULL;
    cJSON* json_typed = OCNullCopyAsJSON(null_obj, true, &error);
    if (!json_typed) {
        fprintf(stderr, "Failed to serialize OCNull in typed mode\n");
        if (error) {
            fprintf(stderr, "Error: %s\n", OCStringGetCString(error));
        }
        cJSON_Delete(json_untyped);
        return false;
    }
    if (!cJSON_IsNull(json_typed)) {
        fprintf(stderr, "Typed OCNull serialization did not produce JSON null\n");
        cJSON_Delete(json_untyped);
        cJSON_Delete(json_typed);
        return false;
    }
    // Test deserialization
    error = NULL;
    OCNullRef null_from_json = OCNullCreateFromJSON(json_untyped, &error);
    if (!null_from_json) {
        fprintf(stderr, "Failed to deserialize JSON null to OCNull\n");
        if (error) {
            fprintf(stderr, "Error: %s\n", OCStringGetCString(error));
        }
        cJSON_Delete(json_untyped);
        cJSON_Delete(json_typed);
        return false;
    }
    if (null_from_json != kOCNull) {
        fprintf(stderr, "Deserialized OCNull is not the singleton\n");
        cJSON_Delete(json_untyped);
        cJSON_Delete(json_typed);
        return false;
    }
    cJSON_Delete(json_untyped);
    cJSON_Delete(json_typed);
    fprintf(stderr, " passed\n");
    return true;
}
bool test_OCNull_in_arrays(void) {
    const char* test_name = "test_OCNull_in_arrays";
    fprintf(stderr, "%s begin...", test_name);
    // Test OCNull in heterogeneous arrays
    const char* json_array_str = "[1, null, \"hello\", true]";
    cJSON* json = cJSON_Parse(json_array_str);
    if (!json) {
        fprintf(stderr, "Failed to parse test JSON array\n");
        return false;
    }
    OCStringRef error = NULL;
    OCArrayRef array = OCArrayCreateFromJSON(json, &error);
    cJSON_Delete(json);
    if (!array) {
        fprintf(stderr, "Failed to create OCArray from JSON with null\n");
        if (error) {
            fprintf(stderr, "Error: %s\n", OCStringGetCString(error));
        }
        return false;
    }
    if (OCArrayGetCount(array) != 4) {
        fprintf(stderr, "Array count incorrect after JSON deserialization\n");
        OCRelease(array);
        return false;
    }
    // Check the null element
    OCNullRef null_elem = (OCNullRef)OCArrayGetValueAtIndex(array, 1);
    if (!OCNullIsNull((OCTypeRef)null_elem)) {
        fprintf(stderr, "Array element at index 1 is not OCNull\n");
        OCRelease(array);
        return false;
    }
    if (null_elem != kOCNull) {
        fprintf(stderr, "Array null element is not the singleton\n");
        OCRelease(array);
        return false;
    }
    // Test serialization back to JSON
    error = NULL;
    cJSON* json_out = OCArrayCopyAsJSON(array, false, &error);
    if (!json_out) {
        fprintf(stderr, "Failed to serialize array with OCNull back to JSON\n");
        if (error) {
            fprintf(stderr, "Error: %s\n", OCStringGetCString(error));
        }
        OCRelease(array);
        return false;
    }
    // Verify the output contains null
    char* json_str = cJSON_Print(json_out);
    if (!strstr(json_str, "null")) {
        fprintf(stderr, "Serialized JSON does not contain 'null'\n");
        free(json_str);
        cJSON_Delete(json_out);
        OCRelease(array);
        return false;
    }
    free(json_str);
    cJSON_Delete(json_out);
    OCRelease(array);
    fprintf(stderr, " passed\n");
    return true;
}
bool test_OCNull_in_dictionaries(void) {
    const char* test_name = "test_OCNull_in_dictionaries";
    fprintf(stderr, "%s begin...", test_name);
    // Test OCNull in dictionaries
    const char* json_dict_str = "{\"name\": \"test\", \"value\": null, \"count\": 42}";
    cJSON* json = cJSON_Parse(json_dict_str);
    if (!json) {
        fprintf(stderr, "Failed to parse test JSON dictionary\n");
        return false;
    }
    OCStringRef error = NULL;
    OCDictionaryRef dict = OCDictionaryCreateFromJSON(json, &error);
    cJSON_Delete(json);
    if (!dict) {
        fprintf(stderr, "Failed to create OCDictionary from JSON with null\n");
        if (error) {
            fprintf(stderr, "Error: %s\n", OCStringGetCString(error));
        }
        return false;
    }
    if (OCDictionaryGetCount(dict) != 3) {
        fprintf(stderr, "Dictionary count incorrect after JSON deserialization\n");
        OCRelease(dict);
        return false;
    }
    // Check the null value
    OCNullRef null_value = (OCNullRef)OCDictionaryGetValue(dict, STR("value"));
    if (!OCNullIsNull((OCTypeRef)null_value)) {
        fprintf(stderr, "Dictionary value for 'value' key is not OCNull\n");
        OCRelease(dict);
        return false;
    }
    if (null_value != kOCNull) {
        fprintf(stderr, "Dictionary null value is not the singleton\n");
        OCRelease(dict);
        return false;
    }
    // Verify other values are correct
    OCStringRef name = (OCStringRef)OCDictionaryGetValue(dict, STR("name"));
    if (!name || strcmp(OCStringGetCString(name), "test") != 0) {
        fprintf(stderr, "Dictionary 'name' value is incorrect\n");
        OCRelease(dict);
        return false;
    }
    OCNumberRef count = (OCNumberRef)OCDictionaryGetValue(dict, STR("count"));
    if (!count) {
        fprintf(stderr, "Dictionary 'count' value is missing\n");
        OCRelease(dict);
        return false;
    }
    double count_value;
    OCNumberGetValue(count, kOCNumberFloat64Type, &count_value);
    if (count_value != 42.0) {
        fprintf(stderr, "Dictionary 'count' value is incorrect\n");
        OCRelease(dict);
        return false;
    }
    OCRelease(dict);
    fprintf(stderr, " passed\n");
    return true;
}
bool test_OCNull_roundtrip(void) {
    const char* test_name = "test_OCNull_roundtrip";
    fprintf(stderr, "%s begin...", test_name);
    // Test complex nested structure with nulls
    const char* complex_json =
        "{"
        "\"users\": ["
        "{\"id\": 1, \"name\": \"Alice\", \"email\": null},"
        "{\"id\": 2, \"name\": null, \"email\": \"bob@example.com\"}"
        "],"
        "\"metadata\": null,"
        "\"total\": 2"
        "}";
    cJSON* json = cJSON_Parse(complex_json);
    if (!json) {
        fprintf(stderr, "Failed to parse complex JSON structure\n");
        return false;
    }
    OCStringRef error = NULL;
    OCDictionaryRef complex_dict = OCDictionaryCreateFromJSON(json, &error);
    cJSON_Delete(json);
    if (!complex_dict) {
        fprintf(stderr, "Failed to create OCDictionary from complex JSON\n");
        if (error) {
            fprintf(stderr, "Error: %s\n", OCStringGetCString(error));
        }
        return false;
    }
    // Check users array
    OCArrayRef users = (OCArrayRef)OCDictionaryGetValue(complex_dict, STR("users"));
    if (!users || OCArrayGetCount(users) != 2) {
        fprintf(stderr, "Users array is incorrect\n");
        OCRelease(complex_dict);
        return false;
    }
    // First user has null email
    OCDictionaryRef user1 = (OCDictionaryRef)OCArrayGetValueAtIndex(users, 0);
    OCNullRef email1 = (OCNullRef)OCDictionaryGetValue(user1, STR("email"));
    if (!OCNullIsNull((OCTypeRef)email1) || email1 != kOCNull) {
        fprintf(stderr, "First user email should be OCNull\n");
        OCRelease(complex_dict);
        return false;
    }
    // Second user has null name
    OCDictionaryRef user2 = (OCDictionaryRef)OCArrayGetValueAtIndex(users, 1);
    OCNullRef name2 = (OCNullRef)OCDictionaryGetValue(user2, STR("name"));
    if (!OCNullIsNull((OCTypeRef)name2) || name2 != kOCNull) {
        fprintf(stderr, "Second user name should be OCNull\n");
        OCRelease(complex_dict);
        return false;
    }
    // Metadata is null
    OCNullRef metadata = (OCNullRef)OCDictionaryGetValue(complex_dict, STR("metadata"));
    if (!OCNullIsNull((OCTypeRef)metadata) || metadata != kOCNull) {
        fprintf(stderr, "Metadata should be OCNull\n");
        OCRelease(complex_dict);
        return false;
    }
    OCRelease(complex_dict);
    fprintf(stderr, " passed\n");
    return true;
}
bool test_OCNull_comprehensive(void) {
    const char* test_name = "test_OCNull_comprehensive";
    fprintf(stderr, "%s begin...", test_name);
    bool all_passed = true;
    if (!test_OCNull_singleton()) all_passed = false;
    if (!test_OCNull_json_serialization()) all_passed = false;
    if (!test_OCNull_in_arrays()) all_passed = false;
    if (!test_OCNull_in_dictionaries()) all_passed = false;
    if (!test_OCNull_roundtrip()) all_passed = false;
    if (all_passed) {
        fprintf(stderr, " passed\n");
    } else {
        fprintf(stderr, "%s end...with problems\n", test_name);
    }
    return all_passed;
}

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../src/OCIndexArray.h"
#include "../src/OCIndexSet.h"
#include "../src/OCNumber.h"
#include "../src/OCType.h"
#include "test_utils.h"
bool OCIndexArrayCreateAndCount_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    bool success = true;
    OCIndex values[] = {1, 2, 3, 4};
    OCIndexArrayRef array = OCIndexArrayCreate(values, 4);
    success &= (array && (OCIndexArrayGetCount(array) == 4));
    if (array) OCRelease(array);
    OCIndexArrayRef bad = OCIndexArrayCreate(NULL, 5);
    success &= (bad == NULL);
    fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexArrayGetValueAtIndex_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    OCIndex values[] = {10, 20, 30};
    OCIndexArrayRef array = OCIndexArrayCreate(values, 3);
    if (!array) return false;
    bool success = (OCIndexArrayGetValueAtIndex(array, 0) == 10) &&
                   (OCIndexArrayGetValueAtIndex(array, 1) == 20) &&
                   (OCIndexArrayGetValueAtIndex(array, 2) == 30) &&
                   (OCIndexArrayGetValueAtIndex(array, 99) == kOCNotFound);
    OCRelease(array);
    fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexArraySetValueAtIndex_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    OCMutableIndexArrayRef array = OCIndexArrayCreateMutable(3);
    if (!array) return false;
    bool setOK = OCIndexArraySetValueAtIndex(array, 0, 99);
    bool setFail = !OCIndexArraySetValueAtIndex(array, 5, 42);  // Invalid index
    bool success = setOK && setFail &&
                   (OCIndexArrayGetValueAtIndex(array, 0) == 99);
    OCRelease(array);
    fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexArrayRemoveValueAtIndex_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    OCIndex values[] = {5, 10, 15, 20};
    OCIndexArrayRef orig = OCIndexArrayCreate(values, 4);
    if (!orig) return false;
    OCMutableIndexArrayRef array = OCIndexArrayCreateMutableCopy(orig);
    OCRelease(orig);
    if (!array) return false;
    bool removeOK = OCIndexArrayRemoveValueAtIndex(array, 1);
    bool removeFail = !OCIndexArrayRemoveValueAtIndex(array, 10);
    bool success = removeOK && removeFail &&
                   (OCIndexArrayGetCount(array) == 3) &&
                   (OCIndexArrayGetValueAtIndex(array, 1) == 15);
    OCRelease(array);
    fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexArrayRemoveValuesAtIndexes_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    OCIndex values[] = {10, 20, 30, 40, 50};
    OCIndexArrayRef orig = OCIndexArrayCreate(values, 5);
    if (!orig) return false;
    OCMutableIndexArrayRef array = OCIndexArrayCreateMutableCopy(orig);
    OCRelease(orig);
    if (!array) return false;
    OCMutableIndexSetRef indexSet = OCIndexSetCreateMutable();
    if (!indexSet) {
        OCRelease(array);
        return false;
    }
    OCIndexSetAddIndex(indexSet, 1);  // 20
    OCIndexSetAddIndex(indexSet, 3);  // 40
    OCIndexArrayRemoveValuesAtIndexes(array, indexSet);
    bool success = (OCIndexArrayGetCount(array) == 3) &&
                   (OCIndexArrayGetValueAtIndex(array, 0) == 10) &&
                   (OCIndexArrayGetValueAtIndex(array, 1) == 30) &&
                   (OCIndexArrayGetValueAtIndex(array, 2) == 50);
    OCRelease(indexSet);
    OCRelease(array);
    fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexArrayDeepCopy_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    OCIndex values[] = {1, 2, 3};
    OCIndexArrayRef orig = OCIndexArrayCreate(values, 3);
    if (!orig) return false;
    OCIndexArrayRef copy = (OCIndexArrayRef)OCTypeDeepCopy(orig);
    if (!copy) {
        OCRelease(orig);
        fprintf(stderr, "Deep copy returned NULL\n");
        return false;
    }
    bool equalContent = true;
    for (OCIndex i = 0; i < OCIndexArrayGetCount(orig); i++) {
        if (OCIndexArrayGetValueAtIndex(orig, i) != OCIndexArrayGetValueAtIndex(copy, i)) {
            equalContent = false;
            break;
        }
    }
    // Optional: If you can test identity — confirm not same object
    bool distinct = (orig != copy);
    OCRelease(orig);
    OCRelease(copy);
    bool success = equalContent && distinct;
    fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexArrayJSONEncoding_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    // Create test array
    OCIndex testIndexes[] = {10, 20, 30, 40, 50};
    size_t count = sizeof(testIndexes) / sizeof(testIndexes[0]);
    OCMutableIndexArrayRef original = OCIndexArrayCreateMutable(0);
    if (!original) {
        fprintf(stderr, "FAIL: Could not create mutable index array\n");
        return false;
    }
    for (size_t i = 0; i < count; i++) {
        OCIndexArrayAppendValue(original, testIndexes[i]);
    }
    // Test 1: Default encoding (should be OCJSONEncodingNone)
    OCJSONEncoding defaultEncoding = OCIndexArrayCopyEncoding(original);
    if (defaultEncoding != OCJSONEncodingNone) {
        fprintf(stderr, "FAIL: Default encoding should be OCJSONEncodingNone\n");
        OCRelease(original);
        return false;
    }
    // Test 2: Set encoding to base64 and test serialization
    OCIndexArraySetEncoding(original, OCJSONEncodingBase64);
    OCJSONEncoding newEncoding = OCIndexArrayCopyEncoding(original);
    if (newEncoding != OCJSONEncodingBase64) {
        fprintf(stderr, "FAIL: Could not set encoding to OCJSONEncodingBase64\n");
        OCRelease(original);
        return false;
    }
    // Test 3: Serialize with base64 encoding
    OCStringRef base64Error = NULL;
    cJSON *jsonBase64 = OCIndexArrayCopyAsJSON(original, true, &base64Error);
    if (!jsonBase64) {
        fprintf(stderr, "FAIL: Could not serialize OCIndexArray with base64 encoding");
        if (base64Error) {
            fprintf(stderr, ": %s", OCStringGetCString(base64Error));
        }
        fprintf(stderr, "\n");
        OCRelease(original);
        return false;
    }
    // Verify JSON structure
    cJSON *type = cJSON_GetObjectItem(jsonBase64, "type");
    cJSON *encoding = cJSON_GetObjectItem(jsonBase64, "encoding");
    cJSON *value = cJSON_GetObjectItem(jsonBase64, "value");
    if (!type || !cJSON_IsString(type) || strcmp(cJSON_GetStringValue(type), "OCIndexArray") != 0) {
        fprintf(stderr, "FAIL: Invalid type field\n");
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    if (!encoding || !cJSON_IsString(encoding) || strcmp(cJSON_GetStringValue(encoding), "base64") != 0) {
        fprintf(stderr, "FAIL: Invalid encoding field for base64\n");
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    if (!value || !cJSON_IsString(value)) {
        fprintf(stderr, "FAIL: Invalid value field for base64 encoding\n");
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    // Test 4: Roundtrip with base64 encoding
    OCStringRef error = NULL;
    OCIndexArrayRef deserializedBase64 = OCIndexArrayCreateFromJSON(jsonBase64, &error);
    if (!deserializedBase64) {
        fprintf(stderr, "FAIL: Could not deserialize OCIndexArray from base64: %s\n",
                error ? OCStringGetCString(error) : "unknown error");
        if (error) OCRelease(error);
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    // Verify array equality
    if (!OCTypeEqual(original, deserializedBase64)) {
        fprintf(stderr, "FAIL: Base64 roundtrip data does not match original\n");
        OCRelease(deserializedBase64);
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    // Verify encoding is preserved
    OCJSONEncoding deserializedEncoding = OCIndexArrayCopyEncoding(deserializedBase64);
    if (deserializedEncoding != OCJSONEncodingBase64) {
        fprintf(stderr, "FAIL: Base64 encoding not preserved in roundtrip\n");
        OCRelease(deserializedBase64);
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    // Test 5: Set encoding to none and test serialization
    OCIndexArraySetEncoding(original, OCJSONEncodingNone);
    OCStringRef noneError = NULL;
    cJSON *jsonNone = OCIndexArrayCopyAsJSON(original, true, &noneError);
    if (!jsonNone) {
        fprintf(stderr, "FAIL: Could not serialize OCIndexArray with none encoding");
        if (noneError) {
            fprintf(stderr, ": %s", OCStringGetCString(noneError));
        }
        fprintf(stderr, "\n");
        OCRelease(deserializedBase64);
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    // Verify JSON structure for none encoding
    cJSON *encodingNone = cJSON_GetObjectItem(jsonNone, "encoding");
    cJSON *valueNone = cJSON_GetObjectItem(jsonNone, "value");
    if (!encodingNone || !cJSON_IsString(encodingNone) || strcmp(cJSON_GetStringValue(encodingNone), "none") != 0) {
        fprintf(stderr, "FAIL: Invalid encoding field for none\n");
        cJSON_Delete(jsonNone);
        OCRelease(deserializedBase64);
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    if (!valueNone || !cJSON_IsArray(valueNone)) {
        fprintf(stderr, "FAIL: Invalid value field for none encoding (should be array)\n");
        cJSON_Delete(jsonNone);
        OCRelease(deserializedBase64);
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    // Verify array content
    int arraySize = cJSON_GetArraySize(valueNone);
    if (arraySize != (int)count) {
        fprintf(stderr, "FAIL: Array size mismatch in none encoding\n");
        cJSON_Delete(jsonNone);
        OCRelease(deserializedBase64);
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    for (int i = 0; i < arraySize; i++) {
        cJSON *item = cJSON_GetArrayItem(valueNone, i);
        if (!item || !cJSON_IsNumber(item) || (OCIndex)item->valuedouble != testIndexes[i]) {
            fprintf(stderr, "FAIL: Array content mismatch at index %d\n", i);
            cJSON_Delete(jsonNone);
            OCRelease(deserializedBase64);
            cJSON_Delete(jsonBase64);
            OCRelease(original);
            return false;
        }
    }
    // Test 6: Roundtrip with none encoding
    OCIndexArrayRef deserializedNone = OCIndexArrayCreateFromJSON(jsonNone, &error);
    if (!deserializedNone) {
        fprintf(stderr, "FAIL: Could not deserialize OCIndexArray from none: %s\n",
                error ? OCStringGetCString(error) : "unknown error");
        if (error) OCRelease(error);
        cJSON_Delete(jsonNone);
        OCRelease(deserializedBase64);
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    // Verify array equality
    if (!OCTypeEqual(original, deserializedNone)) {
        fprintf(stderr, "FAIL: None roundtrip data does not match original\n");
        OCRelease(deserializedNone);
        cJSON_Delete(jsonNone);
        OCRelease(deserializedBase64);
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    // Verify encoding is preserved
    OCJSONEncoding deserializedNoneEncoding = OCIndexArrayCopyEncoding(deserializedNone);
    if (deserializedNoneEncoding != OCJSONEncodingNone) {
        fprintf(stderr, "FAIL: None encoding not preserved in roundtrip\n");
        OCRelease(deserializedNone);
        cJSON_Delete(jsonNone);
        OCRelease(deserializedBase64);
        cJSON_Delete(jsonBase64);
        OCRelease(original);
        return false;
    }
    // Cleanup
    cJSON_Delete(jsonNone);
    cJSON_Delete(jsonBase64);
    OCRelease(deserializedNone);
    OCRelease(deserializedBase64);
    OCRelease(original);
    fprintf(stderr, " passed\n");
    return true;
}

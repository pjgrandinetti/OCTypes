
#include <stdio.h>
#include <string.h>
#include "../src/OCIndexSet.h"
#include "../src/OCNumber.h"
#include "../src/OCType.h"
#include "test_utils.h"
bool OCIndexSetCreateAndAccess_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    bool success = true;
    OCIndexSetRef s = OCIndexSetCreate();
    success &= (s && OCIndexSetGetCount(s) == 0);
    OCIndexSetRef single = OCIndexSetCreateWithIndex(42);
    success &= (single && OCIndexSetGetCount(single) == 1 && OCIndexSetFirstIndex(single) == 42);
    OCIndexSetRef range = OCIndexSetCreateWithIndexesInRange(10, 3);
    success &= (range && OCIndexSetGetCount(range) == 3);
    OCRelease(s);
    OCRelease(single);
    OCRelease(range);
    fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexSetAddAndContains_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    bool success = true;
    OCMutableIndexSetRef set = OCIndexSetCreateMutable();
    success &= (set != NULL);
    success &= OCIndexSetAddIndex(set, 10);
    success &= OCIndexSetAddIndex(set, 20);
    success &= !OCIndexSetAddIndex(set, 10);  // Duplicate
    success &= OCIndexSetContainsIndex(set, 10);
    success &= OCIndexSetContainsIndex(set, 20);
    success &= !OCIndexSetContainsIndex(set, 99);
    OCRelease(set);
    fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexSetRangeAndBounds_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    bool success = true;
    OCMutableIndexSetRef set = OCIndexSetCreateMutable();
    OCIndexSetAddIndex(set, 5);
    OCIndexSetAddIndex(set, 10);
    OCIndexSetAddIndex(set, 20);
    OCIndexSetAddIndex(set, 30);
    success &= (OCIndexSetFirstIndex(set) == 5);
    success &= (OCIndexSetLastIndex(set) == 30);
    success &= (OCIndexSetIndexLessThanIndex(set, 25) == 20);
    success &= (OCIndexSetIndexGreaterThanIndex(set, 25) == 30);
    OCRelease(set);
    fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexSetSerialization_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    bool success = true;
    OCMutableIndexSetRef set = OCIndexSetCreateMutable();
    OCIndexSetAddIndex(set, 100);
    OCIndexSetAddIndex(set, 200);
    OCDictionaryRef dict = OCIndexSetCreateDictionary(set);
    success &= (dict != NULL);
    OCIndexSetRef restored = OCIndexSetCreateFromDictionary(dict);
    success &= (restored && OCIndexSetEqual(set, restored));
    OCRelease(set);
    OCRelease(dict);
    OCRelease(restored);
    fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexSetDeepCopy_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    bool success = true;
    OCMutableIndexSetRef original = OCIndexSetCreateMutable();
    OCIndexSetAddIndex(original, 10);
    OCIndexSetAddIndex(original, 20);
    OCIndexSetRef copy = (OCIndexSetRef)OCTypeDeepCopy(original);
    success &= (copy != NULL);
    success &= OCIndexSetEqual(original, copy);
    success &= (original != copy);  // pointer inequality
    // Mutate the copy and check the original is unaffected
    OCIndexSetAddIndex((OCMutableIndexSetRef)copy, 30);
    success &= (OCIndexSetGetCount(copy) == 3);
    success &= (OCIndexSetGetCount(original) == 2);
    OCRelease(original);
    OCRelease(copy);
    fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexSetJSONEncoding_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    // Create test set
    OCMutableIndexSetRef original = OCIndexSetCreateMutable();
    if (!original) {
        fprintf(stderr, "FAIL: Could not create mutable index set\n");
        return false;
    }
    OCIndexSetAddIndex(original, 5);
    OCIndexSetAddIndex(original, 15);
    OCIndexSetAddIndex(original, 25);
    bool success = true;
    // Test encoding functionality
    success &= (OCIndexSetCopyEncoding(original) == OCJSONEncodingNone);
    // Test base64 encoding
    OCIndexSetSetEncoding(original, OCJSONEncodingBase64);
    success &= (OCIndexSetCopyEncoding(original) == OCJSONEncodingBase64);
    OCStringRef base64Error = NULL;
    cJSON *jsonBase64 = OCIndexSetCopyAsJSON(original, true, &base64Error);
    success &= (jsonBase64 != NULL);
    if (!jsonBase64 && base64Error) {
        fprintf(stderr, "Base64 serialization failed: %s\n", OCStringGetCString(base64Error));
    }
    if (jsonBase64) {
        cJSON *encoding = cJSON_GetObjectItem(jsonBase64, "encoding");
        success &= (encoding && cJSON_IsString(encoding) && strcmp(cJSON_GetStringValue(encoding), "base64") == 0);
        // Roundtrip test
        OCIndexSetRef deserializedBase64 = OCIndexSetCreateFromJSON(jsonBase64, NULL);
        success &= (deserializedBase64 != NULL);
        success &= OCTypeEqual(original, deserializedBase64);
        success &= (OCIndexSetCopyEncoding(deserializedBase64) == OCJSONEncodingBase64);
        if (deserializedBase64) OCRelease(deserializedBase64);
        cJSON_Delete(jsonBase64);
    }
    // Test none encoding
    OCIndexSetSetEncoding(original, OCJSONEncodingNone);
    success &= (OCIndexSetCopyEncoding(original) == OCJSONEncodingNone);
    OCStringRef noneError = NULL;
    cJSON *jsonNone = OCIndexSetCopyAsJSON(original, true, &noneError);
    success &= (jsonNone != NULL);
    if (!jsonNone && noneError) {
        fprintf(stderr, "None encoding serialization failed: %s\n", OCStringGetCString(noneError));
    }
    if (jsonNone) {
        cJSON *encoding = cJSON_GetObjectItem(jsonNone, "encoding");
        success &= (encoding && cJSON_IsString(encoding) && strcmp(cJSON_GetStringValue(encoding), "none") == 0);
        cJSON *value = cJSON_GetObjectItem(jsonNone, "value");
        success &= (value && cJSON_IsArray(value));
        success &= (cJSON_GetArraySize(value) == 3);
        // Roundtrip test
        OCIndexSetRef deserializedNone = OCIndexSetCreateFromJSON(jsonNone, NULL);
        success &= (deserializedNone != NULL);
        success &= OCTypeEqual(original, deserializedNone);
        success &= (OCIndexSetCopyEncoding(deserializedNone) == OCJSONEncodingNone);
        if (deserializedNone) OCRelease(deserializedNone);
        cJSON_Delete(jsonNone);
    }
    OCRelease(original);
    fprintf(stderr, " passed\n");
    return success;
}

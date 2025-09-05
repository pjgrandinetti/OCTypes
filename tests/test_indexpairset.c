#include <stdio.h>
#include <string.h>
#include "../src/OCIndexPairSet.h"
#include "../src/OCType.h"
#include "test_utils.h"
bool OCIndexPairSetCreation_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    bool success = true;
    OCIndexPairSetRef empty = OCIndexPairSetCreate();
    success &= empty && OCIndexPairSetGetCount(empty) == 0;
    if (empty) OCRelease(empty);
    OCIndexPair pairs[2] = {{1, 100}, {2, 200}};
    OCIndexPairSetRef set = OCIndexPairSetCreateWithIndexPairArray(pairs, 2);
    success &= set && OCIndexPairSetGetCount(set) == 2;
    success &= OCIndexPairSetValueForIndex(set, 1) == 100;
    success &= OCIndexPairSetValueForIndex(set, 2) == 200;
    success &= OCIndexPairSetValueForIndex(set, 3) == kOCNotFound;
    if (set) OCRelease(set);
    OCIndexPairSetRef one = OCIndexPairSetCreateWithIndexPair(5, 50);
    success &= one && OCIndexPairSetGetCount(one) == 1;
    success &= OCIndexPairSetValueForIndex(one, 5) == 50;
    if (one) OCRelease(one);
    // Test with NULL input and count = 0
    OCIndexPairSetRef nullSet = OCIndexPairSetCreateWithIndexPairArray(NULL, 0);
    success &= nullSet && OCIndexPairSetGetCount(nullSet) == 0;
    if (nullSet) OCRelease(nullSet);
        fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexPairSetAddAndContains_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    OCMutableIndexPairSetRef set = OCIndexPairSetCreateMutable();
    bool success = set != NULL;
    if (set) {
        success &= OCIndexPairSetAddIndexPair(set, 10, 100);
        success &= OCIndexPairSetAddIndexPair(set, 20, 200);
        success &= !OCIndexPairSetAddIndexPair(set, 10, 999);  // Duplicate
        success &= OCIndexPairSetContainsIndex(set, 10);
        success &= OCIndexPairSetContainsIndex(set, 20);
        success &= !OCIndexPairSetContainsIndex(set, 30);
        success &= OCIndexPairSetValueForIndex(set, 10) == 100;
        success &= OCIndexPairSetValueForIndex(set, 20) == 200;
        OCRelease(set);
    }
        fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexPairSetValueLookup_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    OCMutableIndexPairSetRef set = OCIndexPairSetCreateMutable();
    bool success = set != NULL;
    if (set) {
        OCIndexPairSetAddIndexPair(set, 5, 50);
        OCIndexPairSetAddIndexPair(set, 10, 100);
        success &= (OCIndexPairSetValueForIndex(set, 5) == 50);
        success &= (OCIndexPairSetValueForIndex(set, 10) == 100);
        success &= (OCIndexPairSetValueForIndex(set, 999) == kOCNotFound);
        OCRelease(set);
    }
        fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexPairSetEquality_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    OCIndexPairSetRef a = OCIndexPairSetCreateWithIndexPair(1, 100);
    OCIndexPairSetRef b = OCIndexPairSetCreateWithIndexPair(1, 100);
    OCIndexPairSetRef c = OCIndexPairSetCreateWithIndexPair(1, 999);
    bool success = a && b && c &&
                   OCTypeEqual(a, b) &&
                   !OCTypeEqual(a, c);
    if (a) OCRelease(a);
    if (b) OCRelease(b);
    if (c) OCRelease(c);
        fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexPairSetShow_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    OCMutableIndexPairSetRef set = OCIndexPairSetCreateMutable();
    bool success = set != NULL;
    if (set) {
        OCIndexPairSetAddIndexPair(set, 1, 10);
        OCIndexPairSetAddIndexPair(set, 2, 20);
        OCIndexPairSetAddIndexPair(set, 3, 30);
        OCIndexPairSetShow(set);
        OCRelease(set);
    }
        fprintf(stderr, " passed\n");
    return success;
}
bool OCIndexPairSetDeepCopy_test(void) {
    fprintf(stderr, "%s begin...", __func__);
    bool success = true;
    OCMutableIndexPairSetRef original = OCIndexPairSetCreateMutable();
    OCIndexPairSetAddIndexPair(original, 5, 50);
    OCIndexPairSetAddIndexPair(original, 10, 100);
    OCIndexPairSetRef copy = OCTypeDeepCopy(original);
    success &= copy != NULL;
    success &= OCTypeEqual(original, copy);
    // Shallow copy check — the internal pointers should not be identical
    success &= (original != copy);
    // Mutate the copy to ensure original is unaffected
    OCIndexPairSetAddIndexPair((OCMutableIndexPairSetRef)copy, 20, 200);
    success &= (OCIndexPairSetGetCount(copy) == 3);
    success &= (OCIndexPairSetGetCount(original) == 2);
    OCRelease(original);
    OCRelease(copy);
        fprintf(stderr, " passed\n");
    return success;
}

bool OCIndexPairSetJSONEncoding_test(void) {
    fprintf(stderr, "%s begin...", __func__);

    // Create test pair set
    OCMutableIndexPairSetRef original = OCIndexPairSetCreateMutable();
    if (!original) {
        fprintf(stderr, "FAIL: Could not create mutable index pair set\n");
        return false;
    }

    OCIndexPairSetAddIndexPair(original, 1, 10);
    OCIndexPairSetAddIndexPair(original, 2, 20);
    OCIndexPairSetAddIndexPair(original, 3, 30);

    bool success = true;

    // Test encoding functionality
    success &= (OCIndexPairSetCopyEncoding(original) == OCJSONEncodingNone);

    // Test base64 encoding
    OCIndexPairSetSetEncoding(original, OCJSONEncodingBase64);
    success &= (OCIndexPairSetCopyEncoding(original) == OCJSONEncodingBase64);

    OCStringRef base64Error = NULL;
    cJSON *jsonBase64 = OCIndexPairSetCopyAsJSON(original, true, &base64Error);
    success &= (jsonBase64 != NULL);
    if (!jsonBase64 && base64Error) {
        fprintf(stderr, "Base64 serialization failed: %s\n", OCStringGetCString(base64Error));
    }

    if (jsonBase64) {
        cJSON *encoding = cJSON_GetObjectItem(jsonBase64, "encoding");
        success &= (encoding && cJSON_IsString(encoding) && strcmp(cJSON_GetStringValue(encoding), "base64") == 0);

        // Roundtrip test
        OCIndexPairSetRef deserializedBase64 = OCIndexPairSetCreateFromJSON(jsonBase64, NULL);
        success &= (deserializedBase64 != NULL);
        success &= OCTypeEqual(original, deserializedBase64);
        success &= (OCIndexPairSetCopyEncoding(deserializedBase64) == OCJSONEncodingBase64);

        if (deserializedBase64) OCRelease(deserializedBase64);
        cJSON_Delete(jsonBase64);
    }

    // Test none encoding (CSDM flat array)
    OCIndexPairSetSetEncoding(original, OCJSONEncodingNone);
    success &= (OCIndexPairSetCopyEncoding(original) == OCJSONEncodingNone);

    OCStringRef noneError = NULL;
    cJSON *jsonNone = OCIndexPairSetCopyAsJSON(original, true, &noneError);
    success &= (jsonNone != NULL);
    if (!jsonNone && noneError) {
        fprintf(stderr, "None encoding serialization failed: %s\n", OCStringGetCString(noneError));
    }

    if (jsonNone) {
        cJSON *encoding = cJSON_GetObjectItem(jsonNone, "encoding");
        success &= (encoding && cJSON_IsString(encoding) && strcmp(cJSON_GetStringValue(encoding), "none") == 0);

        cJSON *value = cJSON_GetObjectItem(jsonNone, "value");
        success &= (value && cJSON_IsArray(value));
        // CSDM flat format: [index1,value1,index2,value2,index3,value3] = 6 elements
        success &= (cJSON_GetArraySize(value) == 6);

        // Verify CSDM flat array structure [1,10,2,20,3,30]
        if (success && cJSON_GetArraySize(value) == 6) {
            cJSON *items[6];
            for (int i = 0; i < 6; i++) {
                items[i] = cJSON_GetArrayItem(value, i);
                success &= (items[i] && cJSON_IsNumber(items[i]));
            }
            if (success) {
                // Check that pairs are preserved (order might vary due to set nature)
                bool foundPair1 = false, foundPair2 = false, foundPair3 = false;
                for (int i = 0; i < 6; i += 2) {
                    OCIndex index = (OCIndex)items[i]->valuedouble;
                    OCIndex value = (OCIndex)items[i+1]->valuedouble;
                    if (index == 1 && value == 10) foundPair1 = true;
                    else if (index == 2 && value == 20) foundPair2 = true;
                    else if (index == 3 && value == 30) foundPair3 = true;
                }
                success &= (foundPair1 && foundPair2 && foundPair3);
            }
        }

        // Roundtrip test
        OCIndexPairSetRef deserializedNone = OCIndexPairSetCreateFromJSON(jsonNone, NULL);
        success &= (deserializedNone != NULL);
        success &= OCTypeEqual(original, deserializedNone);
        success &= (OCIndexPairSetCopyEncoding(deserializedNone) == OCJSONEncodingNone);

        if (deserializedNone) OCRelease(deserializedNone);
        cJSON_Delete(jsonNone);
    }

    OCRelease(original);
        fprintf(stderr, " passed\n");
    return success;
}

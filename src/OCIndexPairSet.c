/*
 * OCIndexPairSet.c (cleaned version)
 *
 * Created by PhySy Ltd. Improved by Assistant, 2025
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OCTypes.h"
static OCTypeID kOCIndexPairSetID = kOCNotATypeID;
struct impl_OCIndexPairSet {
    OCBase base;
    OCDataRef indexPairs;
    OCJSONEncoding encoding;
};
OCTypeID OCIndexPairSetGetTypeID(void) {
    if (kOCIndexPairSetID == kOCNotATypeID) {
        kOCIndexPairSetID = OCRegisterType("OCIndexPairSet", (OCTypeRef (*)(cJSON *, OCStringRef *))OCIndexPairSetCreateFromJSON);
    }
    return kOCIndexPairSetID;
}
static bool impl_OCIndexPairSetEqual(const void *a_, const void *b_) {
    OCIndexPairSetRef a = (OCIndexPairSetRef)a_;
    OCIndexPairSetRef b = (OCIndexPairSetRef)b_;
    return OCTypeEqual(a->indexPairs, b->indexPairs);
}
static void impl_OCIndexPairSetFinalize(const void *obj) {
    OCIndexPairSetRef constSet = (OCIndexPairSetRef)obj;
    OCMutableIndexPairSetRef set = (OCMutableIndexPairSetRef)constSet;
    if (set->indexPairs) {
        OCRelease(set->indexPairs);
        set->indexPairs = NULL;  //
    }
}
static OCStringRef impl_OCIndexPairSetCopyFormattingDesc(OCTypeRef cf) {
    if (!cf) return NULL;
    return OCStringCreateWithCString("<OCIndexPairSet>");
}
static cJSON *
impl_OCIndexPairSetCopyJSON(const void *obj, bool typed) {
    return OCIndexPairSetCopyAsJSON((OCIndexPairSetRef)obj, typed);
}
static OCMutableIndexPairSetRef OCIndexPairSetAllocate(void);
static void *impl_OCIndexPairSetDeepCopy(const void *obj) {
    OCIndexPairSetRef src = (OCIndexPairSetRef)obj;
    if (!src || !src->indexPairs) return NULL;
    // Deep copy underlying OCDataRef
    OCDataRef copiedData = OCDataCreateCopy(src->indexPairs);
    if (!copiedData) return NULL;
    OCMutableIndexPairSetRef copy = OCIndexPairSetAllocate();
    if (!copy) {
        OCRelease(copiedData);
        return NULL;
    }
    copy->indexPairs = copiedData;
    return (void *)copy;
}
static void *impl_OCIndexPairSetDeepCopyMutable(const void *obj) {
    OCIndexPairSetRef src = (OCIndexPairSetRef)obj;
    if (!src || !src->indexPairs) return NULL;
    OCMutableDataRef copied = OCDataCreateMutableCopy(OCDataGetLength(src->indexPairs), src->indexPairs);
    if (!copied) return NULL;
    OCMutableIndexPairSetRef copy = OCIndexPairSetAllocate();
    if (!copy) {
        OCRelease(copied);
        return NULL;
    }
    copy->indexPairs = copied;
    return copy;
}
static OCMutableIndexPairSetRef OCIndexPairSetAllocate(void) {
    return (OCMutableIndexPairSetRef)OCTypeAlloc(
        struct impl_OCIndexPairSet,
        OCIndexPairSetGetTypeID(),
        impl_OCIndexPairSetFinalize,
        impl_OCIndexPairSetEqual,
        impl_OCIndexPairSetCopyFormattingDesc,
        impl_OCIndexPairSetCopyJSON,
        impl_OCIndexPairSetDeepCopy,
        impl_OCIndexPairSetDeepCopyMutable);
}
OCIndexPairSetRef OCIndexPairSetCreate(void) {
    OCMutableIndexPairSetRef s = OCIndexPairSetAllocate();
    s->indexPairs = NULL;
    return s;
}
OCMutableIndexPairSetRef OCIndexPairSetCreateMutable(void) {
    OCMutableIndexPairSetRef s = OCIndexPairSetAllocate();
    if (!s) return NULL;
    s->indexPairs = OCDataCreateMutable(0);
    return s;
}
OCIndexPairSetRef OCIndexPairSetCreateCopy(OCIndexPairSetRef source) {
    if (!source) return NULL;
    return (OCIndexPairSetRef)impl_OCIndexPairSetDeepCopy(source);
}
OCMutableIndexPairSetRef OCIndexPairSetCreateMutableCopy(OCIndexPairSetRef source) {
    if (!source) return NULL;
    return (OCMutableIndexPairSetRef)impl_OCIndexPairSetDeepCopyMutable(source);
}
OCIndexPairSetRef OCIndexPairSetCreateWithIndexPairArray(OCIndexPair *array, int count) {
    // Defensive check: invalid input if count > 0 but array is NULL
    if (count > 0 && array == NULL) return NULL;
    OCMutableIndexPairSetRef s = OCIndexPairSetAllocate();
    if (!s) return NULL;
    if (count == 0) {
        // Allocate empty OCData (not NULL, to allow valid GetCount and safety downstream)
        s->indexPairs = OCDataCreate(NULL, 0);
    } else {
        s->indexPairs = OCDataCreate((const uint8_t *)array, count * sizeof(OCIndexPair));
    }
    if (!s->indexPairs) {
        OCRelease(s);
        return NULL;
    }
    return s;
}
OCIndexPairSetRef OCIndexPairSetCreateWithIndexPair(OCIndex index, OCIndex value) {
    OCIndexPair pair = {index, value};
    return OCIndexPairSetCreateWithIndexPairArray(&pair, 1);
}
OCIndexPairSetRef OCIndexPairSetCreateWithTwoIndexPairs(OCIndex i1, OCIndex v1, OCIndex i2, OCIndex v2) {
    OCIndexPair pairs[2] = {{i1, v1}, {i2, v2}};
    return OCIndexPairSetCreateWithIndexPairArray(pairs, 2);
}
OCIndex OCIndexPairSetGetCount(OCIndexPairSetRef s) {
    return s && s->indexPairs ? OCDataGetLength(s->indexPairs) / sizeof(OCIndexPair) : 0;
}
OCIndexPair *OCIndexPairSetGetBytesPtr(OCIndexPairSetRef s) {
    return s && s->indexPairs ? (OCIndexPair *)OCDataGetBytesPtr(s->indexPairs) : NULL;
}
OCIndex OCIndexPairSetValueForIndex(OCIndexPairSetRef s, OCIndex index) {
    OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(s);
    OCIndex count = OCIndexPairSetGetCount(s);
    for (OCIndex i = 0; i < count; i++)
        if (pairs[i].index == index)
            return pairs[i].value;
    return kOCNotFound;
}
bool OCIndexPairSetAddIndexPair(OCMutableIndexPairSetRef s, OCIndex index, OCIndex value) {
    if (!s) return false;
    OCIndexPair newPair = {index, value};
    OCIndex count = OCIndexPairSetGetCount(s);
    OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(s);
    for (OCIndex i = 0; i < count; i++) {
        if (pairs[i].index == index) return false;
    }
    OCIndexPair *newArray = malloc((count + 1) * sizeof(OCIndexPair));
    if (!newArray) return false;
    int inserted = 0;
    for (OCIndex i = 0, j = 0; i <= count; i++) {
        if (!inserted && (i == count || pairs[i].index > index)) {
            newArray[j++] = newPair;
            inserted = 1;
        }
        if (i < count) newArray[j++] = pairs[i];
    }
    OCDataRef newData = OCDataCreate((const uint8_t *)newArray, (count + 1) * sizeof(OCIndexPair));
    free(newArray);
    if (!newData) return false;
    OCRelease(s->indexPairs);
    s->indexPairs = newData;
    return true;
}
bool OCIndexPairSetContainsIndex(OCIndexPairSetRef s, OCIndex index) {
    OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(s);
    OCIndex count = OCIndexPairSetGetCount(s);
    for (OCIndex i = 0; i < count; i++)
        if (pairs[i].index == index)
            return true;
    return false;
}
cJSON *OCIndexPairSetCopyAsJSON(OCIndexPairSetRef set, bool typed) {
    if (!set) return cJSON_CreateNull();

    OCIndex count = OCIndexPairSetGetCount(set);
    OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(set);
    
    if (typed) {
        // For typed serialization, check encoding preference
        OCJSONEncoding encoding = set->encoding;
        cJSON *entry = cJSON_CreateObject();
        if (!entry) return cJSON_CreateNull();
        
        cJSON_AddStringToObject(entry, "type", "OCIndexPairSet");
        
        if (encoding == OCJSONEncodingBase64) {
            // Use base64 encoding for compact binary representation
            cJSON_AddStringToObject(entry, "encoding", "base64");
            OCStringRef b64 = OCDataCreateBase64EncodedString(set->indexPairs, OCBase64EncodingOptionsNone);
            if (b64) {
                const char *b64Str = OCStringGetCString(b64);
                cJSON_AddStringToObject(entry, "value", b64Str ? b64Str : "");
                OCRelease(b64);
            } else {
                cJSON_AddStringToObject(entry, "value", "");
            }
        } else {
            // Default: use CSDM flat array format (encoding OCJSONEncodingNone)
            if (encoding == OCJSONEncodingNone) {
                cJSON_AddStringToObject(entry, "encoding", "none");
            }
            
            cJSON *arr = cJSON_CreateArray();
            if (!arr) {
                cJSON_Delete(entry);
                return cJSON_CreateNull();
            }
            
            if (pairs && count > 0) {
                // CSDM convention: flatten pairs into a single array [index1, value1, index2, value2, ...]
                for (OCIndex i = 0; i < count; i++) {
                    cJSON_AddItemToArray(arr, cJSON_CreateNumber((double)pairs[i].index));
                    cJSON_AddItemToArray(arr, cJSON_CreateNumber((double)pairs[i].value));
                }
            }
            cJSON_AddItemToObject(entry, "value", arr);
        }
        return entry;
    } else {
        // For untyped serialization, serialize as a plain JSON array following CSDM convention (no change)
        cJSON *arr = cJSON_CreateArray();
        if (!arr) return cJSON_CreateNull();
        
        if (pairs && count > 0) {
            // CSDM convention: flatten pairs into a single array [index1, value1, index2, value2, ...]
            for (OCIndex i = 0; i < count; i++) {
                cJSON_AddItemToArray(arr, cJSON_CreateNumber((double)pairs[i].index));
                cJSON_AddItemToArray(arr, cJSON_CreateNumber((double)pairs[i].value));
            }
        }
        return arr;
    }
}

OCIndexPairSetRef OCIndexPairSetCreateFromJSON(cJSON *json, OCStringRef *outError) {
    if (!json) {
        if (outError) *outError = STR("JSON input is NULL");
        return NULL;
    }
    
    cJSON *arrayToProcess = NULL;
    OCJSONEncoding encodingPreference = OCJSONEncodingNone;
    
    // Check if typed format (object with type/value structure)
    if (cJSON_IsObject(json)) {
        cJSON *type = cJSON_GetObjectItem(json, "type");
        cJSON *value = cJSON_GetObjectItem(json, "value");
        cJSON *encoding = cJSON_GetObjectItem(json, "encoding");

        if (!cJSON_IsString(type)) {
            if (outError) *outError = STR("Invalid typed JSON format: missing or invalid type field");
            return NULL;
        }

        const char *typeName = cJSON_GetStringValue(type);
        if (!typeName || strcmp(typeName, "OCIndexPairSet") != 0) {
            if (outError) *outError = STR("Invalid type: expected OCIndexPairSet");
            return NULL;
        }

        // Check for base64 encoding
        if (encoding && cJSON_IsString(encoding)) {
            const char *encodingStr = cJSON_GetStringValue(encoding);
            if (encodingStr && strcmp(encodingStr, "base64") == 0) {
                // Handle base64 encoded data
                if (!cJSON_IsString(value)) {
                    if (outError) *outError = STR("Invalid base64 format: value must be a string");
                    return NULL;
                }
                
                const char *b64Str = cJSON_GetStringValue(value);
                if (!b64Str) {
                    if (outError) *outError = STR("Invalid base64 string");
                    return NULL;
                }
                
                OCStringRef b64String = OCStringCreateWithCString(b64Str);
                if (!b64String) {
                    if (outError) *outError = STR("Failed to create base64 string");
                    return NULL;
                }
                
                OCDataRef data = OCDataCreateFromBase64EncodedString(b64String);
                OCRelease(b64String);
                
                if (!data) {
                    if (outError) *outError = STR("Failed to decode base64 data");
                    return NULL;
                }
                
                // Extract pairs from the data
                const OCIndexPair *pairs = (const OCIndexPair *)OCDataGetBytesPtr(data);
                OCIndex dataLength = OCDataGetLength(data);
                OCIndex pairCount = dataLength / sizeof(OCIndexPair);
                
                OCIndexPairSetRef result = OCIndexPairSetCreateWithIndexPairArray((OCIndexPair *)pairs, pairCount);
                OCRelease(data);
                
                if (result) {
                    // Set encoding preference
                    OCIndexPairSetSetEncoding((OCMutableIndexPairSetRef)result, OCJSONEncodingBase64);
                }
                
                if (!result && outError) {
                    *outError = STR("Failed to create OCIndexPairSet from base64 data");
                }
                return result;
            } else if (strcmp(encodingStr, "none") == 0) {
                encodingPreference = OCJSONEncodingNone;
            }
        }

        // Handle array format (default or explicit "none" encoding)
        if (!cJSON_IsArray(value)) {
            if (outError) *outError = STR("Invalid typed JSON format: value field must be an array for flat format");
            return NULL;
        }
        arrayToProcess = value;
    }
    // Handle untyped format (direct array)
    else if (cJSON_IsArray(json)) {
        arrayToProcess = json;
    }
    else {
        if (outError) *outError = STR("Invalid JSON: expected object or array");
        return NULL;
    }
    
    // Process the array - CSDM convention: flat array [index1, value1, index2, value2, ...]
    int arraySize = cJSON_GetArraySize(arrayToProcess);
    if (arraySize < 0) {
        if (outError) *outError = STR("Invalid array size");
        return NULL;
    }

    // Array size must be even for CSDM flat format
    if (arraySize % 2 != 0) {
        if (outError) *outError = STR("Array size must be even for CSDM flat format [index1, value1, index2, value2, ...]");
        return NULL;
    }

    int pairCount = arraySize / 2;
    if (pairCount == 0) {
        // Empty set - valid
        OCIndexPairSetRef result = OCIndexPairSetCreateWithIndexPairArray(NULL, 0);
        if (result) {
            OCIndexPairSetSetEncoding((OCMutableIndexPairSetRef)result, encodingPreference);
        }
        return result;
    }

    OCIndexPair *pairs = malloc(pairCount * sizeof(OCIndexPair));
    if (!pairs) {
        if (outError) *outError = STR("Failed to allocate memory for index pairs");
        return NULL;
    }

    // Parse flat array: [index1, value1, index2, value2, ...]
    for (int i = 0; i < pairCount; i++) {
        cJSON *indexNode = cJSON_GetArrayItem(arrayToProcess, i * 2);
        cJSON *valueNode = cJSON_GetArrayItem(arrayToProcess, i * 2 + 1);
        
        if (!cJSON_IsNumber(indexNode) || !cJSON_IsNumber(valueNode)) {
            if (outError) *outError = STR("Invalid array elements: expected numbers in flat format");
            free(pairs);
            return NULL;
        }
        
        pairs[i].index = (OCIndex)indexNode->valuedouble;
        pairs[i].value = (OCIndex)valueNode->valuedouble;
    }

    OCIndexPairSetRef result = OCIndexPairSetCreateWithIndexPairArray(pairs, pairCount);
    free(pairs);
    
    if (result) {
        OCIndexPairSetSetEncoding((OCMutableIndexPairSetRef)result, encodingPreference);
    }
    
    if (!result && outError) {
        *outError = STR("Failed to create OCIndexPairSet");
    }
    return result;
}

void OCIndexPairSetShow(OCIndexPairSetRef set) {
    if (!set) return;
    OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(set);
    OCIndex count = OCIndexPairSetGetCount(set);
    fprintf(stderr, "(");
    for (OCIndex i = 0; i < count; i++) {
        fprintf(stderr, "(%ld,%ld)%s", pairs[i].index, pairs[i].value, (i < count - 1 ? "," : ""));
    }
    fprintf(stderr, ")\n");
}
OCIndexArrayRef OCIndexPairSetCreateIndexArrayOfValues(OCIndexPairSetRef set) {
    if (!set) return NULL;
    OCIndex count = OCIndexPairSetGetCount(set);
    OCMutableIndexArrayRef result = OCIndexArrayCreateMutable(count);
    if (!result) return NULL;
    OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(set);
    if (!pairs) {
        OCRelease(result);
        return NULL;
    }
    for (OCIndex i = 0; i < count; ++i) {
        OCIndexArrayAppendValue(result, pairs[i].value);
    }
    return (OCIndexArrayRef)result;
}
OCIndexSetRef OCIndexPairSetCreateIndexSetOfIndexes(OCIndexPairSetRef set) {
    if (!set) return NULL;
    // 1) make an empty, mutable OCIndexSet
    OCMutableIndexSetRef indexes = OCIndexSetCreateMutable();
    // 2) grab the raw OCIndexPair buffer
    const OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(set);
    OCIndex cnt = OCIndexPairSetGetCount(set);
    // 3) add each .index into our OCIndexSet
    for (OCIndex i = 0; i < cnt; ++i) {
        OCIndexSetAddIndex(indexes, pairs[i].index);
    }
    // 4) return as immutable OCIndexSetRef
    return (OCIndexSetRef)indexes;
}
OCJSONEncoding OCIndexPairSetCopyEncoding(OCIndexPairSetRef set) {
    if (!set) return OCJSONEncodingNone;
    return set->encoding;
}
void OCIndexPairSetSetEncoding(OCMutableIndexPairSetRef set, OCJSONEncoding encoding) {
    if (!set) return;
    set->encoding = encoding;
}

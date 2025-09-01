/*
 * OCIndexArray.c – Improved Implementation
 *
 * Defines and implements OCIndexArray and OCMutableIndexArray types for managing
 * arrays of OCIndex values. Includes support for copying, mutation, serialization,
 * and Base64 conversion.
 */
#include "OCIndexArray.h"  // Own header first
#include <stdio.h>         // For printf, etc.
#include <stdlib.h>        // For malloc, free
#include <string.h>        // For memcpy, etc.
#include "OCArray.h"       // For OCArray functions
#include "OCData.h"        // For OCData functions
#include "OCDictionary.h"  // For OCDictionary functions
#include "OCIndexSet.h"    // For OCIndexSet functions
#include "OCNumber.h"      // For OCNumberType definition
#include "OCString.h"      // For OCString functions
static OCTypeID kOCIndexArrayID = kOCNotATypeID;
struct impl_OCIndexArray {
    OCBase base;
    OCMutableDataRef indexes;
};
OCTypeID OCIndexArrayGetTypeID(void) {
    if (kOCIndexArrayID == kOCNotATypeID) {
        kOCIndexArrayID = OCRegisterType("OCIndexArray", (OCTypeRef (*)(cJSON *))OCIndexArrayCreateFromJSONTyped);
    }
    return kOCIndexArrayID;
}
static bool impl_OCIndexArrayEqual(const void *a_, const void *b_) {
    OCIndexArrayRef a = (OCIndexArrayRef)a_;
    OCIndexArrayRef b = (OCIndexArrayRef)b_;
    if (OCDataGetLength(a->indexes) != OCDataGetLength(b->indexes)) return false;
    OCIndex count = OCIndexArrayGetCount(a);
    for (OCIndex i = 0; i < count; i++) {
        if (OCIndexArrayGetValueAtIndex(a, i) != OCIndexArrayGetValueAtIndex(b, i))
            return false;
    }
    return true;
}
static void *impl_OCIndexArrayDeepCopy(const void *obj) {
    OCIndexArrayRef src = (OCIndexArrayRef)obj;
    return src ? (void *)OCIndexArrayCreateCopy(src) : NULL;
}
static void *impl_OCIndexArrayDeepCopyMutable(const void *obj) {
    OCIndexArrayRef src = (OCIndexArrayRef)obj;
    return src ? (void *)OCIndexArrayCreateMutableCopy(src) : NULL;
}
static void impl_OCIndexArrayFinalize(const void *obj) {
    OCMutableIndexArrayRef array = (OCMutableIndexArrayRef)obj;
    if (array->indexes) OCRelease(array->indexes);
    array->indexes = NULL;
}
static OCStringRef impl_OCIndexArrayCopyFormattingDesc(OCTypeRef cf) {
    return OCStringCreateWithCString("<OCIndexArray>");
}
static cJSON *
impl_OCIndexArrayCopyJSON(const void *obj, bool typed) {
    return OCIndexArrayCopyAsJSON((OCIndexArrayRef)obj, typed);
}
static OCMutableIndexArrayRef OCIndexArrayAllocate(void) {
    return (OCMutableIndexArrayRef)OCTypeAlloc(
        struct impl_OCIndexArray,
        OCIndexArrayGetTypeID(),
        impl_OCIndexArrayFinalize,
        impl_OCIndexArrayEqual,
        impl_OCIndexArrayCopyFormattingDesc,
        impl_OCIndexArrayCopyJSON,
        impl_OCIndexArrayDeepCopy,
        impl_OCIndexArrayDeepCopyMutable);
}
OCIndexArrayRef OCIndexArrayCreate(OCIndex *values, OCIndex count) {
    if (count > 0 && values == NULL) return NULL;
    OCMutableIndexArrayRef array = OCIndexArrayAllocate();
    if (!array) return NULL;
    array->indexes = (OCMutableDataRef)OCDataCreate((const uint8_t *)values, count * sizeof(OCIndex));
    if (!array->indexes) {
        OCRelease(array);
        return NULL;
    }
    return array;
}
OCMutableIndexArrayRef OCIndexArrayCreateMutable(OCIndex capacity) {
    OCMutableIndexArrayRef array = OCIndexArrayAllocate();
    if (!array) return NULL;
    array->indexes = OCDataCreateMutable(capacity * sizeof(OCIndex));
    if (!array->indexes) {
        OCRelease(array);
        return NULL;
    }
    // Ensure the buffer is logically initialized
    if (!OCDataSetLength(array->indexes, capacity * sizeof(OCIndex))) {
        OCRelease(array);
        return NULL;
    }
    return array;
}
static OCIndexArrayRef OCIndexArrayCreateWithData(OCDataRef indexes) {
    if (!indexes) return NULL;
    OCMutableIndexArrayRef array = OCIndexArrayAllocate();
    if (!array) return NULL;
    array->indexes = (OCMutableDataRef)OCDataCreateCopy(indexes);
    if (!array->indexes) {
        OCRelease(array);
        return NULL;
    }
    return array;
}
static OCMutableIndexArrayRef OCIndexArrayCreateMutableWithData(OCMutableDataRef indexes) {
    if (!indexes) return NULL;
    OCMutableIndexArrayRef array = OCIndexArrayAllocate();
    if (!array) return NULL;
    OCIndex length = OCDataGetLength(indexes);
    array->indexes = OCDataCreateMutableCopy(length, indexes);
    if (!array->indexes) {
        OCRelease(array);
        return NULL;
    }
    return array;
}
OCIndexArrayRef OCIndexArrayCreateCopy(OCIndexArrayRef src) {
    return OCIndexArrayCreateWithData(src->indexes);
}
OCMutableIndexArrayRef OCIndexArrayCreateMutableCopy(OCIndexArrayRef src) {
    return OCIndexArrayCreateMutableWithData(src->indexes);
}
OCIndex OCIndexArrayGetCount(OCIndexArrayRef array) {
    return array && array->indexes ? OCDataGetLength(array->indexes) / sizeof(OCIndex) : 0;
}
OCIndex *OCIndexArrayGetMutableBytes(OCIndexArrayRef array) {
    return array && array->indexes ? (OCIndex *)OCDataGetMutableBytes(array->indexes) : NULL;
}
OCIndex OCIndexArrayGetValueAtIndex(OCIndexArrayRef array, OCIndex index) {
    if (!array || !array->indexes) return kOCNotFound;
    OCIndex count = OCIndexArrayGetCount(array);
    if (index >= count) return kOCNotFound;
    OCIndex *ptr = (OCIndex *)OCDataGetBytesPtr(array->indexes);
    return ptr ? ptr[index] : kOCNotFound;
}
bool OCIndexArraySetValueAtIndex(OCMutableIndexArrayRef array, OCIndex index, OCIndex value) {
    if (!array || !array->indexes) return false;
    OCIndex count = OCIndexArrayGetCount(array);
    if (index >= count) return false;
    OCIndex *ptr = (OCIndex *)OCDataGetMutableBytes(array->indexes);
    if (!ptr) return false;
    ptr[index] = value;
    return true;
}
bool OCIndexArrayRemoveValueAtIndex(OCMutableIndexArrayRef array, OCIndex index) {
    if (!array || !array->indexes) return false;
    OCIndex count = OCIndexArrayGetCount(array);
    if (index >= count) return false;
    OCIndex *old = (OCIndex *)OCDataGetBytesPtr(array->indexes);
    if (!old) return false;
    // Allocate a new buffer of capacity (count-1)*sizeof(OCIndex)
    OCMutableDataRef data = OCDataCreateMutable((count - 1) * sizeof(OCIndex));
    if (!data) return false;
    // Now explicitly set its length to match capacity:
    if (!OCDataSetLength(data, (count - 1) * sizeof(OCIndex))) {
        OCRelease(data);
        return false;
    }
    OCIndex *copy = (OCIndex *)OCDataGetMutableBytes(data);
    if (!copy) {
        OCRelease(data);
        return false;
    }
    // Copy everything except the “index”th element
    for (OCIndex i = 0, j = 0; i < count; i++) {
        if (i != index) {
            copy[j++] = old[i];
        }
    }
    OCRelease(array->indexes);
    array->indexes = data;
    return true;
}
void OCIndexArrayRemoveValuesAtIndexes(OCMutableIndexArrayRef array, OCIndexSetRef indexesToRemove) {
    if (!array || !indexesToRemove) return;
    OCIndex count = OCIndexSetGetCount(indexesToRemove);
    if (count == 0) return;
    // Copy indices to an array and sort descending
    OCIndex *indices = (OCIndex *)malloc(count * sizeof(OCIndex));
    if (!indices) return;
    OCIndex idx = OCIndexSetLastIndex(indexesToRemove);
    for (OCIndex i = 0; i < count && idx != kOCNotFound; ++i) {
        indices[i] = idx;
        idx = OCIndexSetIndexLessThanIndex(indexesToRemove, idx);
    }
    for (OCIndex i = 0; i < count; ++i) {
        OCIndexArrayRemoveValueAtIndex(array, indices[i]);
    }
    free(indices);
}
bool OCIndexArrayContainsIndex(OCIndexArrayRef array, OCIndex value) {
    OCIndex count = OCIndexArrayGetCount(array);
    OCIndex *ptr = (OCIndex *)OCDataGetBytesPtr(array->indexes);
    for (OCIndex i = 0; i < count; i++) {
        if (ptr[i] == value) return true;
    }
    return false;
}
bool OCIndexArrayAppendValue(OCMutableIndexArrayRef array, OCIndex value) {
    if (!array || !array->indexes) return false;
    if (!OCDataIncreaseLength(array->indexes, sizeof(OCIndex))) {
        return false;
    }
    OCIndex *ptr = (OCIndex *)OCDataGetMutableBytes(array->indexes);
    if (!ptr) return false;
    ptr[OCIndexArrayGetCount(array) - 1] = value;
    return true;
}
bool OCIndexArrayAppendValues(OCMutableIndexArrayRef dst, OCIndexArrayRef src) {
    if (!dst || !src) return false;
    OCMutableDataRef newData = OCDataCreateMutableCopy(0, dst->indexes);
    if (!newData) return false;
    if (!OCDataAppendBytes(newData,
                           OCDataGetBytesPtr(src->indexes),
                           OCDataGetLength(src->indexes))) {
        OCRelease(newData);
        return false;
    }
    OCRelease(dst->indexes);
    dst->indexes = newData;
    return true;
}
OCStringRef OCIndexArrayCreateBase64String(OCIndexArrayRef array, OCNumberType type) {
    if (!array) return NULL;
    OCIndex count = OCIndexArrayGetCount(array);
    OCIndex *src = (OCIndex *)OCDataGetBytesPtr(array->indexes);
    size_t size = 0;
    void *converted = NULL;
    switch (type) {
        case kOCNumberUInt8Type:
        case kOCNumberSInt8Type:
            size = sizeof(uint8_t);
            converted = malloc(count * size);
            for (OCIndex i = 0; i < count; i++) ((uint8_t *)converted)[i] = (uint8_t)src[i];
            break;
        case kOCNumberUInt16Type:
        case kOCNumberSInt16Type:
            size = sizeof(uint16_t);
            converted = malloc(count * size);
            for (OCIndex i = 0; i < count; i++) ((uint16_t *)converted)[i] = (uint16_t)src[i];
            break;
        case kOCNumberUInt32Type:
        case kOCNumberSInt32Type:
            size = sizeof(uint32_t);
            converted = malloc(count * size);
            for (OCIndex i = 0; i < count; i++) ((uint32_t *)converted)[i] = (uint32_t)src[i];
            break;
        case kOCNumberUInt64Type:
        case kOCNumberSInt64Type:
            size = sizeof(uint64_t);
            converted = malloc(count * size);
            for (OCIndex i = 0; i < count; i++) ((uint64_t *)converted)[i] = (uint64_t)src[i];
            break;
        default:
            return OCDataCreateBase64EncodedString(array->indexes, OCBase64EncodingOptionsNone);
    }
    OCDataRef data = OCDataCreate((const uint8_t *)converted, count * size);
    free(converted);
    OCStringRef result = OCDataCreateBase64EncodedString(data, OCBase64EncodingOptionsNone);
    OCRelease(data);
    return result;
}
OCArrayRef OCIndexArrayCreateCFNumberArray(OCIndexArrayRef array) {
    OCIndex count = OCIndexArrayGetCount(array);
    OCMutableArrayRef result = OCArrayCreateMutable(count, &kOCTypeArrayCallBacks);
    for (OCIndex i = 0; i < count; i++) {
        OCNumberRef num = OCNumberCreateWithOCIndex(OCIndexArrayGetValueAtIndex(array, i));
        OCArrayAppendValue(result, num);
        OCRelease(num);
    }
    return result;
}
OCDictionaryRef OCIndexArrayCreateDictionary(OCIndexArrayRef array) {
    OCMutableDictionaryRef dict = OCDictionaryCreateMutable(0);
    if (array && array->indexes)
        OCDictionarySetValue(dict, STR("indexes"), array->indexes);
    return dict;
}
OCIndexArrayRef OCIndexArrayCreateFromDictionary(OCDictionaryRef dict) {
    return dict ? OCIndexArrayCreateWithData(OCDictionaryGetValue(dict, STR("indexes"))) : NULL;
}
OCIndexArrayRef OCIndexArrayCreateFromJSON(cJSON *json) {
    if (!cJSON_IsArray(json)) return NULL;
    OCIndex count = cJSON_GetArraySize(json);
    OCIndex *values = malloc(sizeof(OCIndex) * count);
    if (!values) return NULL;
    for (OCIndex i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(json, (int)i);
        if (!cJSON_IsNumber(item)) {
            free(values);
            return NULL;
        }
        values[i] = (OCIndex)item->valuedouble;
    }
    OCIndexArrayRef result = OCIndexArrayCreate(values, count);
    free(values);
    return result;
}
cJSON *OCIndexArrayCopyAsJSON(OCIndexArrayRef array, bool typed) {
    if (!array) return cJSON_CreateNull();

    OCIndex count = OCIndexArrayGetCount(array);
    cJSON *arr = cJSON_CreateArray();
    for (OCIndex i = 0; i < count; i++) {
        OCIndex v = OCIndexArrayGetValueAtIndex(array, i);
        cJSON_AddItemToArray(arr, cJSON_CreateNumber((double)v));
    }

    if (typed) {
        // For typed serialization, OCIndexArray needs wrapping since it's not a native JSON type
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddStringToObject(entry, "type", "OCIndexArray");
        cJSON_AddItemToObject(entry, "value", arr);
        return entry;
    } else {
        // For untyped serialization, serialize as a plain JSON array
        return arr;
    }
}

OCIndexArrayRef OCIndexArrayCreateFromJSONTyped(cJSON *json) {
    if (!json || !cJSON_IsObject(json)) return NULL;

    cJSON *type = cJSON_GetObjectItem(json, "type");
    cJSON *value = cJSON_GetObjectItem(json, "value");

    if (!cJSON_IsString(type) || !cJSON_IsArray(value)) return NULL;

    const char *typeName = cJSON_GetStringValue(type);
    if (!typeName || strcmp(typeName, "OCIndexArray") != 0) return NULL;

    int count = cJSON_GetArraySize(value);
    if (count < 0) return NULL;

    OCIndex *values = malloc(sizeof(OCIndex) * count);
    if (!values) return NULL;

    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(value, i);
        if (!cJSON_IsNumber(item)) {
            fprintf(stderr, "OCIndexArrayCreateFromJSONTyped: Invalid index at position %d\n", i);
            free(values);
            return NULL;
        }
        values[i] = (OCIndex)item->valuedouble;
    }

    OCIndexArrayRef result = OCIndexArrayCreate(values, count);
    free(values);
    return result;
}
void OCIndexArrayShow(OCIndexArrayRef array) {
    fprintf(stderr, "(");
    OCIndex count = OCIndexArrayGetCount(array);
    for (OCIndex i = 0; i < count; i++) {
        fprintf(stderr, "%ld%s", OCIndexArrayGetValueAtIndex(array, i), (i < count - 1 ? "," : ""));
    }
    fprintf(stderr, ")\n");
}

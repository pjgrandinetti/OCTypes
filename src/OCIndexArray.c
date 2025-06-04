/*
 * OCIndexArray.c – Improved Implementation
 *
 * Defines and implements OCIndexArray and OCMutableIndexArray types for managing
 * arrays of OCIndex values. Includes support for copying, mutation, serialization,
 * and Base64 conversion.
 */

#include "OCLibrary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static OCTypeID kOCIndexArrayID = _kOCNotATypeID;

struct __OCIndexArray {
    OCBase _base;
    OCMutableDataRef indexes;
};

OCTypeID OCIndexArrayGetTypeID(void) {
    if (kOCIndexArrayID == _kOCNotATypeID)
        kOCIndexArrayID = OCRegisterType("OCIndexArray");
    return kOCIndexArrayID;
}

static bool __OCIndexArrayEqual(const void *a_, const void *b_) {
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

static void __OCIndexArrayFinalize(const void *obj) {
    OCMutableIndexArrayRef array = (OCMutableIndexArrayRef)obj;
    if (array->indexes) OCRelease(array->indexes);
    array->indexes = NULL;
}

static OCStringRef __OCIndexArrayCopyFormattingDesc(OCTypeRef cf) {
    return OCStringCreateWithCString("<OCIndexArray>");
}

static OCMutableIndexArrayRef OCIndexArrayAllocate(void) {
    return (OCMutableIndexArrayRef)OCTypeAlloc(
        struct __OCIndexArray,
        OCIndexArrayGetTypeID(),
        __OCIndexArrayFinalize,
        __OCIndexArrayEqual,
        __OCIndexArrayCopyFormattingDesc);
}

OCIndexArrayRef OCIndexArrayCreate(OCIndex *values, OCIndex count) {
    OCMutableIndexArrayRef array = OCIndexArrayAllocate();
    array->indexes = (OCMutableDataRef)OCDataCreate((const uint8_t *)values, count * sizeof(OCIndex));
    return array;
}

OCMutableIndexArrayRef OCIndexArrayCreateMutable(OCIndex capacity) {
    OCMutableIndexArrayRef array = OCIndexArrayAllocate();
    array->indexes = OCDataCreateMutable(capacity * sizeof(OCIndex));
    OCDataSetLength(array->indexes, capacity * sizeof(OCIndex));
    return array;
}

static OCIndexArrayRef OCIndexArrayCreateWithParameters(OCDataRef indexes) {
    OCMutableIndexArrayRef array = OCIndexArrayAllocate();
    array->indexes = (OCMutableDataRef)OCDataCreateCopy(indexes);
    return array;
}

static OCMutableIndexArrayRef OCIndexArrayCreateMutableWithParameters(OCMutableDataRef indexes) {
    OCMutableIndexArrayRef array = OCIndexArrayAllocate();
    array->indexes = OCDataCreateMutableCopy(OCDataGetLength(indexes), indexes);
    return array;
}

OCIndexArrayRef OCIndexArrayCreateCopy(OCIndexArrayRef src) {
    return OCIndexArrayCreateWithParameters(src->indexes);
}

OCMutableIndexArrayRef OCIndexArrayCreateMutableCopy(OCIndexArrayRef src) {
    return OCIndexArrayCreateMutableWithParameters(src->indexes);
}

OCIndex OCIndexArrayGetCount(OCIndexArrayRef array) {
    return array && array->indexes ? OCDataGetLength(array->indexes) / sizeof(OCIndex) : 0;
}

OCIndex *OCIndexArrayGetMutableBytePtr(OCIndexArrayRef array) {
    return array && array->indexes ? (OCIndex *)OCDataGetMutableBytePtr(array->indexes) : NULL;
}

OCIndex OCIndexArrayGetValueAtIndex(OCIndexArrayRef array, OCIndex index) {
    OCIndex *ptr = (OCIndex *)OCDataGetBytePtr(array->indexes);
    return ptr ? ptr[index] : kOCNotFound;
}

bool OCIndexArraySetValueAtIndex(OCMutableIndexArrayRef array, OCIndex index, OCIndex value) {
    OCIndex *ptr = (OCIndex *)OCDataGetBytePtr(array->indexes);
    if (!ptr) return false;
    ptr[index] = value;
    return true;
}

bool OCIndexArrayRemoveValueAtIndex(OCMutableIndexArrayRef array, OCIndex index) {
    OCIndex count = OCIndexArrayGetCount(array);
    if (index >= count) return false;
    OCIndex *old = (OCIndex *)OCDataGetBytePtr(array->indexes);
    OCMutableDataRef data = OCDataCreateMutable((count - 1) * sizeof(OCIndex));
    OCIndex *copy = (OCIndex *)OCDataGetMutableBytePtr(data);
    for (OCIndex i = 0, j = 0; i < count; i++) {
        if (i != index) copy[j++] = old[i];
    }
    OCRelease(array->indexes);
    array->indexes = data;
    return true;
}

void OCIndexArrayRemoveValuesAtIndexes(OCMutableIndexArrayRef array, OCIndexSetRef indexesToRemove) {
    if (!array || !indexesToRemove) return;
    OCIndex count = OCIndexSetGetCount(indexesToRemove);
    OCIndex prevIndex = kOCNotFound;
    for (OCIndex i = count; i-- > 0;) {
        OCIndex idx;
        if (i == count - 1)
            idx = OCIndexSetLastIndex(indexesToRemove);
        else
            idx = OCIndexSetIndexLessThanIndex(indexesToRemove, prevIndex);
        prevIndex = idx;
        if (idx != kOCNotFound)
            OCIndexArrayRemoveValueAtIndex(array, idx);
    }
}

bool OCIndexArrayContainsIndex(OCIndexArrayRef array, OCIndex value) {
    OCIndex count = OCIndexArrayGetCount(array);
    OCIndex *ptr = (OCIndex *)OCDataGetBytePtr(array->indexes);
    for (OCIndex i = 0; i < count; i++) {
        if (ptr[i] == value) return true;
    }
    return false;
}

bool OCIndexArrayAppendValue(OCMutableIndexArrayRef array, OCIndex value) {
    if (!array || !array->indexes) return false;
    OCDataIncreaseLength(array->indexes, sizeof(OCIndex));
    OCIndex *ptr = (OCIndex *)OCDataGetBytePtr(array->indexes);
    ptr[OCIndexArrayGetCount(array) - 1] = value;
    return true;
}

bool OCIndexArrayAppendValues(OCMutableIndexArrayRef dst, OCIndexArrayRef src) {
    if (!dst || !src) return false;
    OCMutableDataRef newData = OCDataCreateMutableCopy(0, dst->indexes);
    OCDataAppendBytes(newData, OCDataGetBytePtr(src->indexes), OCDataGetLength(src->indexes));
    OCRelease(dst->indexes);
    dst->indexes = newData;
    return true;
}

OCStringRef OCIndexArrayCreateBase64String(OCIndexArrayRef array, csdmNumericType type) {
    if (!array) return NULL;
    OCIndex count = OCIndexArrayGetCount(array);
    OCIndex *src = (OCIndex *)OCDataGetBytePtr(array->indexes);

    size_t size = 0;
    void *converted = NULL;

    switch (type) {
        case kCSDMNumberUInt8Type:
        case kCSDMNumberSInt8Type:
            size = sizeof(uint8_t);
            converted = malloc(count * size);
            for (OCIndex i = 0; i < count; i++) ((uint8_t *)converted)[i] = (uint8_t)src[i];
            break;
        case kCSDMNumberUInt16Type:
        case kCSDMNumberSInt16Type:
            size = sizeof(uint16_t);
            converted = malloc(count * size);
            for (OCIndex i = 0; i < count; i++) ((uint16_t *)converted)[i] = (uint16_t)src[i];
            break;
        case kCSDMNumberUInt32Type:
        case kCSDMNumberSInt32Type:
            size = sizeof(uint32_t);
            converted = malloc(count * size);
            for (OCIndex i = 0; i < count; i++) ((uint32_t *)converted)[i] = (uint32_t)src[i];
            break;
        case kCSDMNumberUInt64Type:
        case kCSDMNumberSInt64Type:
            size = sizeof(uint64_t);
            converted = malloc(count * size);
            for (OCIndex i = 0; i < count; i++) ((uint64_t *)converted)[i] = (uint64_t)src[i];
            break;
        default:
            return OCStringCreateBase64EncodedWithOptions(array->indexes, OCBase64EncodingOptionsNone);
    }

    OCDataRef data = OCDataCreate((const uint8_t *)converted, count * size);
    free(converted);
    OCStringRef result = OCStringCreateBase64EncodedWithOptions(data, OCBase64EncodingOptionsNone);
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

OCDictionaryRef OCIndexArrayCreatePList(OCIndexArrayRef array) {
    OCMutableDictionaryRef dict = OCDictionaryCreateMutable(0);
    if (array && array->indexes)
        OCDictionarySetValue(dict, STR("indexes"), array->indexes);
    return dict;
}

OCIndexArrayRef OCIndexArrayCreateWithPList(OCDictionaryRef dict) {
    return dict ? OCIndexArrayCreateWithParameters(OCDictionaryGetValue(dict, STR("indexes"))) : NULL;
}

OCDataRef OCIndexArrayCreateData(OCIndexArrayRef array) {
    return array ? OCRetain(array->indexes) : NULL;
}

OCIndexArrayRef OCIndexArrayCreateWithData(OCDataRef data) {
    return data ? OCIndexArrayCreateWithParameters(data) : NULL;
}

void OCIndexArrayShow(OCIndexArrayRef array) {
    fprintf(stderr, "(");
    OCIndex count = OCIndexArrayGetCount(array);
    for (OCIndex i = 0; i < count; i++) {
        fprintf(stderr, "%ld%s", OCIndexArrayGetValueAtIndex(array, i), (i < count - 1 ? "," : ""));
    }
    fprintf(stderr, ")\n");
}

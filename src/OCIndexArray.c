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

static void *__OCIndexArrayDeepCopy(const void *obj) {
    OCIndexArrayRef src = (OCIndexArrayRef)obj;
    return src ? (void *)OCIndexArrayCreateCopy(src) : NULL;
}

static void *__OCIndexArrayDeepCopyMutable(const void *obj) {
    OCIndexArrayRef src = (OCIndexArrayRef)obj;
    return src ? (void *)OCIndexArrayCreateMutableCopy(src) : NULL;
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
        __OCIndexArrayCopyFormattingDesc,
        __OCIndexArrayDeepCopy,
        __OCIndexArrayDeepCopyMutable
    );
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

static OCIndexArrayRef OCIndexArrayCreateWithParameters(OCDataRef indexes) {
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

static OCMutableIndexArrayRef OCIndexArrayCreateMutableWithParameters(OCMutableDataRef indexes) {
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
    if (!array || !array->indexes) return kOCNotFound;

    OCIndex count = OCIndexArrayGetCount(array);
    if (index >= count) return kOCNotFound;

    OCIndex *ptr = (OCIndex *)OCDataGetBytePtr(array->indexes);
    return ptr ? ptr[index] : kOCNotFound;
}

bool OCIndexArraySetValueAtIndex(OCMutableIndexArrayRef array, OCIndex index, OCIndex value) {
    if (!array || !array->indexes) return false;

    OCIndex count = OCIndexArrayGetCount(array);
    if (index >= count) return false;

    OCIndex *ptr = (OCIndex *)OCDataGetMutableBytePtr(array->indexes);
    if (!ptr) return false;

    ptr[index] = value;
    return true;
}


bool OCIndexArrayRemoveValueAtIndex(OCMutableIndexArrayRef array, OCIndex index) {
    if (!array || !array->indexes) return false;

    OCIndex count = OCIndexArrayGetCount(array);
    if (index >= count) return false;

    OCIndex *old = (OCIndex *)OCDataGetBytePtr(array->indexes);
    if (!old) return false;

    // Allocate a new buffer of capacity (count-1)*sizeof(OCIndex)
    OCMutableDataRef data = OCDataCreateMutable((count - 1) * sizeof(OCIndex));
    if (!data) return false;

    // Now explicitly set its length to match capacity:
    if (!OCDataSetLength(data, (count - 1) * sizeof(OCIndex))) {
        OCRelease(data);
        return false;
    }

    OCIndex *copy = (OCIndex *)OCDataGetMutableBytePtr(data);
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
    OCIndex *ptr = (OCIndex *)OCDataGetBytePtr(array->indexes);
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

    OCIndex *ptr = (OCIndex *)OCDataGetMutableBytePtr(array->indexes);
    if (!ptr) return false;

    ptr[OCIndexArrayGetCount(array) - 1] = value;
    return true;
}

bool OCIndexArrayAppendValues(OCMutableIndexArrayRef dst, OCIndexArrayRef src) {
    if (!dst || !src) return false;

    OCMutableDataRef newData = OCDataCreateMutableCopy(0, dst->indexes);
    if (!newData) return false;

    if (!OCDataAppendBytes(newData,
                           OCDataGetBytePtr(src->indexes),
                           OCDataGetLength(src->indexes))) {
        OCRelease(newData);
        return false;
    }

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

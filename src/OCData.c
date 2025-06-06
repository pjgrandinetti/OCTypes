/*
 * OCData.c – Improved Implementation with Efficient Allocation
 *
 * Provides immutable and mutable binary buffers. Includes support for copying,
 * mutation, serialization, and formatting.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OCLibrary.h"

static OCTypeID kOCDataID = _kOCNotATypeID;

struct __OCData {
    OCBase _base;
    uint8_t *bytes;
    uint64_t length;
    uint64_t capacity;
};

OCTypeID OCDataGetTypeID(void) {
    if (kOCDataID == _kOCNotATypeID) kOCDataID = OCRegisterType("OCData");
    return kOCDataID;
}

static bool __OCDataEqual(const void *a_, const void *b_) {
    OCDataRef a = (OCDataRef)a_;
    OCDataRef b = (OCDataRef)b_;
    if (a->_base.typeID != b->_base.typeID) return false;
    if (a == b) return true;
    if (a->length != b->length) return false;
    return memcmp(a->bytes, b->bytes, a->length) == 0;
}

OCStringRef OCDataCopyFormattingDesc(OCTypeRef cf) {
    if (!cf) return NULL;
    OCDataRef data = (OCDataRef)cf;
    const size_t maxPreview = 16;
    size_t previewLen = data->length < maxPreview ? data->length : maxPreview;
    size_t bufferSize = 64 + previewLen * 3;
    char *buffer = calloc(1, bufferSize);
    if (!buffer) return NULL;
    int offset = snprintf(buffer, bufferSize, "<OCData: %llu bytes, preview: ",
                          (unsigned long long)data->length);
    for (size_t i = 0; i < previewLen && offset < (int)(bufferSize - 4); ++i)
        offset += snprintf(buffer + offset, bufferSize - offset, "%02X ", data->bytes[i]);
    snprintf(buffer + offset, bufferSize - offset, data->length > maxPreview ? "…>" : ">");
    OCStringRef result = OCStringCreateWithCString(buffer);
    free(buffer);
    return result;
}

static void __OCDataFinalize(const void *obj) {
    OCDataRef data = (OCDataRef)obj;
    if (data->bytes) free(data->bytes);
}

static struct __OCData *OCDataAllocate() {
    return OCTypeAlloc(struct __OCData,
                       OCDataGetTypeID(),
                       __OCDataFinalize,
                       __OCDataEqual,
                       OCDataCopyFormattingDesc);
}

OCDataRef OCDataCreate(const uint8_t *bytes, uint64_t length) {
    if (!bytes || length == 0) return NULL;
    struct __OCData *data = OCDataAllocate();
    if (!data) return NULL;
    data->bytes = malloc(length);
    if (!data->bytes) {
        fprintf(stderr, "OCDataCreate: malloc failed\n");
        OCRelease(data);
        return NULL;
    }
    memcpy(data->bytes, bytes, length);
    data->length = length;
    data->capacity = length;
    return data;
}

OCDataRef OCDataCreateWithBytesNoCopy(const uint8_t *bytes, uint64_t length) {
    if (!bytes) return NULL;
    struct __OCData *data = OCDataAllocate();
    data->bytes = (uint8_t *)bytes;
    data->length = length;
    data->capacity = length;
    return data;
}

OCDataRef OCDataCreateCopy(OCDataRef source) {
    return source ? OCDataCreate(source->bytes, source->length) : NULL;
}

OCMutableDataRef OCDataCreateMutable(uint64_t capacity) {
    struct __OCData *data = OCDataAllocate();
    if (!data) return NULL;
    data->bytes = (capacity > 0) ? calloc(capacity, sizeof(uint8_t)) : NULL;
    if (capacity > 0 && !data->bytes) {
        fprintf(stderr, "OCDataCreateMutable: calloc failed\n");
        OCRelease(data);
        return NULL;
    }
    data->length = 0;
    data->capacity = capacity;
    return data;
}

OCMutableDataRef OCDataCreateMutableCopy(uint64_t capacity, OCDataRef source) {
    return source ? (OCMutableDataRef)OCDataCreate(source->bytes, source->length) : NULL;
}

uint64_t OCDataGetLength(OCDataRef data) {
    return data ? data->length : 0;
}

const uint8_t *OCDataGetBytePtr(OCDataRef data) {
    return data ? data->bytes : NULL;
}

uint8_t *OCDataGetMutableBytePtr(OCMutableDataRef data) {
    return data ? data->bytes : NULL;
}

bool OCDataGetBytes(OCDataRef data, OCRange range, uint8_t *buffer) {
    if (!data || !buffer || !data->bytes) return false;
    if (range.location + range.length > data->length) return false;

    memcpy(buffer, data->bytes + range.location, range.length);
    return true;
}

bool OCDataSetLength(OCMutableDataRef data, uint64_t length) {
    if (!data) return false;
    if (length == data->length) return true;

    if (length > data->capacity) {
        void *newBytes = realloc(data->bytes, length);
        if (!newBytes) {
            fprintf(stderr, "OCDataSetLength: realloc failed\n");
            return false;
        }
        data->bytes = newBytes;
        data->capacity = length;
    }

    if (length > data->length) {
        memset((uint8_t *)data->bytes + data->length, 0, length - data->length);
    }

    data->length = length;
    return true;
}

bool OCDataIncreaseLength(OCMutableDataRef data, uint64_t extraLength) {
    if (!data) return false;
    return OCDataSetLength(data, data->length + extraLength);
}

bool OCDataAppendBytes(OCMutableDataRef data, const uint8_t *bytes, uint64_t length) {
    if (!data || !bytes || length == 0) return false;

    uint64_t oldLength = data->length;
    if (!OCDataSetLength(data, oldLength + length)) return false;

    memcpy((uint8_t *)data->bytes + oldLength, bytes, length);
    return true;
}

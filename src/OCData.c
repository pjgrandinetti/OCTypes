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

static OCTypeID kOCDataID = kOCNotATypeID;

struct impl_OCData {
    OCBase base;
    uint8_t *bytes;
    uint64_t length;
    uint64_t capacity;
};

OCTypeID OCDataGetTypeID(void) {
    if (kOCDataID == kOCNotATypeID) kOCDataID = OCRegisterType("OCData");
    return kOCDataID;
}

static bool impl_OCDataEqual(const void *a_, const void *b_) {
    OCDataRef a = (OCDataRef)a_;
    OCDataRef b = (OCDataRef)b_;
    if (a->base.typeID != b->base.typeID) return false;
    if (a == b) return true;
    if (a->length != b->length) return false;
    return memcmp(a->bytes, b->bytes, a->length) == 0;
}

static void *impl_OCDataDeepCopy(const void *obj) {
    OCDataRef source = (OCDataRef)obj;
    if (!source) return NULL;
    return (void *)OCDataCreate(source->bytes, source->length);
}

static void *impl_OCDataDeepCopyMutable(const void *obj) {
    OCDataRef source = (OCDataRef)obj;
    if (!source) return NULL;
    return (void *)OCDataCreateMutableCopy(source->length, source);
}


OCDataRef OCDataCreateCopy(OCDataRef source) {
    return source ? OCDataCreate(source->bytes, source->length) : NULL;
}

OCMutableDataRef OCDataCreateMutableCopy(uint64_t capacity, OCDataRef source) {
    if (!source) return NULL;
    uint64_t actualCap = (capacity > source->length) ? capacity : source->length;
    OCMutableDataRef result = OCDataCreateMutable(actualCap);
    if (!result) return NULL;

    if (source->length > 0) {
        if (!OCDataAppendBytes(result, source->bytes, source->length)) {
            OCRelease(result);
            return NULL;
        }
    }

    return result;
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

    if (data->bytes) {
        for (size_t i = 0; i < previewLen && offset < (int)(bufferSize - 4); ++i) {
            offset += snprintf(buffer + offset, bufferSize - offset, "%02X ", data->bytes[i]);
        }
    }

    snprintf(buffer + offset, bufferSize - offset, data->length > maxPreview ? "…>" : ">");
    OCStringRef result = OCStringCreateWithCString(buffer);
    free(buffer);
    return result;
}

static void impl_OCDataFinalize(const void *obj) {
    OCDataRef data = (OCDataRef)obj;
    if (data->bytes) free(data->bytes);
}

static struct impl_OCData *OCDataAllocate() {
    return OCTypeAlloc(struct impl_OCData,
                   OCDataGetTypeID(),
                   impl_OCDataFinalize,
                   impl_OCDataEqual,
                   OCDataCopyFormattingDesc,
                   impl_OCDataDeepCopy,   
                   impl_OCDataDeepCopyMutable);           

}

OCDataRef OCDataCreate(const uint8_t *bytes, uint64_t length) {
    struct impl_OCData *data = OCDataAllocate();
    if (!data) return NULL;

    if (length > 0) {
        data->bytes = malloc(length);
        if (!data->bytes) {
            fprintf(stderr, "OCDataCreate: malloc failed\n");
            OCRelease(data);
            return NULL;
        }
        memcpy(data->bytes, bytes, length);
    } else {
        data->bytes = NULL;
    }

    data->length = length;
    data->capacity = length;
    return data;
}


OCDataRef OCDataCreateWithBytesNoCopy(const uint8_t *bytes, uint64_t length) {
    if (!bytes) return NULL;
    struct impl_OCData *data = OCDataAllocate();
    data->bytes = (uint8_t *)bytes;
    data->length = length;
    data->capacity = length;
    return data;
}

OCMutableDataRef OCDataCreateMutable(uint64_t capacity) {
    struct impl_OCData *data = OCDataAllocate();
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

uint64_t OCDataGetLength(OCDataRef data) {
    return data ? data->length : 0;
}

const uint8_t *OCDataGetBytesPtr(OCDataRef data) {
    return data ? data->bytes : NULL;
}

uint8_t *OCDataGetMutableBytesPtr(OCMutableDataRef data) {
    return data ? data->bytes : NULL;
}

bool OCDataGetBytes(OCDataRef data, OCRange range, uint8_t *buffer) {
    if (!data || !buffer || !data->bytes) return false;
    if (range.location + range.length > data->length) return false;

    memcpy(buffer, data->bytes + range.location, range.length);
    return true;
}

bool OCDataSetLength(OCMutableDataRef data, uint64_t newLength) {
    if (!data) {
        fprintf(stderr, "OCDataSetLength: data is NULL\n");
        return false;
    }

    if (newLength == data->length) {
        return true; // No change needed
    }

    // If expanding, reallocate if necessary
    if (newLength > data->capacity) {
        uint8_t *newBytes = realloc(data->bytes, newLength);
        if (!newBytes) {
            fprintf(stderr, "OCDataSetLength: realloc failed for newLength = %llu\n",
                    (unsigned long long)newLength);
            return false;
        }
        data->bytes = newBytes;
        data->capacity = newLength;
    }

    // Zero new region if expanding
    if (newLength > data->length && data->bytes) {
        memset(data->bytes + data->length, 0, newLength - data->length);
    }

    // Shrink or grow the logical length
    data->length = newLength;
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

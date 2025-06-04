// OCData.c – Updated version with OCTypeAlloc integration and finalize refactor
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OCLibrary.h"

static OCTypeID kOCDataID = _kOCNotATypeID;

// OCData Opaque Type
struct __OCData {
    OCBase _base;
    uint8_t *bytes;
    uint64_t length;
    uint64_t capacity;
};

OCTypeID OCDataGetTypeID(void)
{
    if (kOCDataID == _kOCNotATypeID) kOCDataID = OCRegisterType("OCData");
    return kOCDataID;
}

static bool __OCDataEqual(const void * theType1, const void * theType2)
{
    OCDataRef theData1 = (OCDataRef) theType1;
    OCDataRef theData2 = (OCDataRef) theType2;
    if (theData1->_base.typeID != theData2->_base.typeID) return false;
    if (theData1 == theData2) return true;
    if (theData1->length != theData2->length) return false;
    return memcmp(theData1->bytes, theData2->bytes, theData1->length) == 0;
}

OCStringRef OCDataCopyFormattingDesc(OCTypeRef cf)
{
    if (!cf) return NULL;
    OCDataRef data = (OCDataRef)cf;

    const size_t maxPreview = 16;
    size_t previewLen = data->length < maxPreview ? data->length : maxPreview;

    // Estimate: 2 chars per byte + "…" + static text overhead
    size_t bufferSize = 64 + previewLen * 3;
    char *buffer = calloc(1, bufferSize);
    if (!buffer) return NULL;

    int offset = snprintf(buffer, bufferSize, "<OCData: %llu bytes, preview: ",
                          (unsigned long long)data->length);

    for (size_t i = 0; i < previewLen && offset < (int)(bufferSize - 4); ++i)
        offset += snprintf(buffer + offset, bufferSize - offset, "%02X ", data->bytes[i]);

    if (data->length > maxPreview)
        snprintf(buffer + offset, bufferSize - offset, "…>");

    else
        snprintf(buffer + offset, bufferSize - offset, ">");

    OCStringRef result = OCStringCreateWithCString(buffer);
    free(buffer);
    return result;
}

static void __OCDataFinalize(const void * theType)
{
    if (NULL == theType) return;
    OCDataRef theData = (OCDataRef) theType;
    if (theData->bytes) {
        free(theData->bytes);
    }
}

static struct __OCData *OCDataAllocate()
{
    return OCTypeAlloc(struct __OCData,
                       OCDataGetTypeID(),
                       __OCDataFinalize,
                       __OCDataEqual,
                       OCDataCopyFormattingDesc);
}

OCDataRef OCDataCreate(const uint8_t *bytes, uint64_t length)
{
    if (NULL == bytes) return NULL;
    struct __OCData *newData = OCDataAllocate();
    if (NULL == newData) return NULL;

    newData->bytes = calloc(length, sizeof(uint8_t));
    if (NULL == newData->bytes) {
        fprintf(stderr, "OCDataCreate: Memory allocation for bytes failed.\n");
        OCRelease(newData);
        return NULL;
    }
    memcpy(newData->bytes, bytes, length);
    newData->length = length;
    newData->capacity = length;
    return newData;
}

OCDataRef OCDataCreateWithBytesNoCopy(const uint8_t *bytes, uint64_t length)
{
    if (NULL == bytes) return NULL;
    struct __OCData *newData = OCDataAllocate();
    if (NULL == newData) return NULL;
    newData->bytes = (uint8_t *)bytes;
    newData->length = length;
    newData->capacity = length;
    return newData;
}

OCDataRef OCDataCreateCopy(OCDataRef theData)
{
    if (NULL == theData) return NULL;
    return OCDataCreate(theData->bytes, theData->length);
}

OCMutableDataRef OCDataCreateMutable(uint64_t capacity)
{
    struct __OCData *newData = OCDataAllocate();
    if (NULL == newData) return NULL;
    if (capacity > 0) {
        newData->bytes = calloc(capacity, sizeof(uint8_t));
        if (NULL == newData->bytes) {
            fprintf(stderr, "OCDataCreateMutable: Memory allocation for bytes failed.\n");
            OCRelease(newData);
            return NULL;
        }
    }
    newData->length = 0;
    newData->capacity = capacity;
    return newData;
}

OCMutableDataRef OCDataCreateMutableCopy(uint64_t capacity, OCDataRef theData)
{
    if (NULL == theData) return NULL;
    return (OCMutableDataRef)OCDataCreate(theData->bytes, theData->length);
}

uint64_t OCDataGetLength(OCDataRef theData)
{
    return theData ? theData->length : 0;
}

const uint8_t *OCDataGetBytePtr(OCDataRef theData)
{
    return theData ? theData->bytes : NULL;
}

uint8_t *OCDataGetMutableBytePtr(OCMutableDataRef theData)
{
    return theData ? theData->bytes : NULL;
}

void OCDataGetBytes(OCDataRef theData, OCRange range, uint8_t *buffer)
{
    if (theData && buffer && theData->bytes)
        memmove(buffer, theData->bytes + range.location, range.length);
}

void OCDataSetLength(OCMutableDataRef theData, uint64_t length)
{
    if (NULL == theData) return;
    uint64_t oldLength = theData->length;
    if (length < oldLength) {
        theData->length = length;
        return;
    }
    if (length > theData->capacity) {
        void *newBytes = realloc(theData->bytes, length);
        if (newBytes == NULL) {
            fprintf(stderr, "OCDataSetLength: Memory allocation failed.\n");
            return;
        }
        theData->bytes = newBytes;
        theData->capacity = length;
    }
    memset(theData->bytes + oldLength, 0, length - oldLength);
    theData->length = length;
}

void OCDataIncreaseLength(OCMutableDataRef theData, uint64_t extraLength)
{
    OCDataSetLength(theData, theData->length + extraLength);
}

void OCDataAppendBytes(OCMutableDataRef theData, const uint8_t *bytes, uint64_t length)
{
    if (NULL == theData || NULL == bytes) return;
    uint64_t oldLength = theData->length;
    OCDataSetLength(theData, oldLength + length);
    memcpy(theData->bytes + oldLength, bytes, length);
}

//
//  OCData.c
//  OCTypes
//
//  Created by philip on 6/28/17.
//

#include <stdlib.h>   // for malloc, free, realloc
#include <string.h>   // for memcmp, memcpy, memmove
#include "OCLibrary.h"

static OCTypeID kOCDataID = _kOCNotATypeID;

// OCData Opaque Type
struct __OCData {
    OCBase _base;

    // OCData Type attributes
    uint8_t *bytes;
    uint64_t length;
    uint64_t capacity;
};

static bool __OCDataEqual(const void * theType1, const void * theType2)
{
    OCDataRef theData1 = (OCDataRef) theType1;
    OCDataRef theData2 = (OCDataRef) theType2;
    if(theData1->_base.typeID != theData2->_base.typeID) return false;
    
    if(NULL == theData1 || NULL == theData2) return false;
    if(theData1 == theData2) return true;
    if(theData1->length != theData2->length) return false;
    return 0 == memcmp(theData1->bytes, theData2->bytes, theData1->length);
    return true;
}

static void __OCDataFinalize(const void * theType)
{
    if(NULL == theType) return;
    OCDataRef theData = (OCDataRef) theType;
    free(theData->bytes);
    free((void *)theData);
}

OCTypeID OCDataGetTypeID(void)
{
    if(kOCDataID == _kOCNotATypeID) kOCDataID = OCRegisterType("OCData");
    return kOCDataID;
}

static struct __OCData *OCDataAllocate()
{
    struct __OCData *theData = malloc(sizeof(struct __OCData));
    if(NULL == theData) return NULL;
    theData->_base.typeID = OCDataGetTypeID();
    theData->_base.retainCount = 1;
    theData->_base.finalize = __OCDataFinalize;
    theData->_base.equal = __OCDataEqual;
    theData->_base.copyFormattingDesc = NULL;
    
    return theData;
}

OCDataRef OCDataCreate(const uint8_t *bytes, uint64_t length)
{
    if(NULL==bytes) return NULL;
    struct __OCData *newData = OCDataAllocate();
    if(NULL == newData) return NULL;
    newData->bytes = (uint8_t *) malloc(length * sizeof(uint8_t));
    memcpy((void *) newData->bytes, (const void *) bytes, length * sizeof(uint8_t));
    newData->length = length;
    newData->capacity = length;
    
    return newData;
}

OCDataRef OCDataCreateWithBytesNoCopy(const uint8_t *bytes, uint64_t length)
{
    if(NULL==bytes) return NULL;
    struct __OCData *newData = OCDataAllocate();
    if(NULL == newData) return NULL;
    newData->bytes = (uint8_t *) bytes;
    newData->length = length;
    newData->capacity = length;
    
    return newData;
}

OCDataRef OCDataCreateCopy(OCDataRef theData)
{
    return OCDataCreate(theData->bytes, theData->length);
}

OCMutableDataRef OCDataCreateMutable(uint64_t capacity)
{
    struct __OCData *newData = OCDataAllocate();
    if(NULL == newData) return NULL;
    if(capacity>0) newData->bytes = (uint8_t *) malloc(capacity * sizeof(uint8_t));
    else newData->bytes = NULL;
    
    newData->length = 0;
    newData->capacity = capacity;
    
    return newData;
}

OCMutableDataRef OCDataCreateMutableCopy(uint64_t capacity, OCDataRef theData)
{
    return (OCMutableDataRef) OCDataCreate(theData->bytes, theData->length);
}

uint64_t OCDataGetLength(OCDataRef theData)
{
    return theData->length;
}

const uint8_t *OCDataGetBytePtr(OCDataRef theData)
{
    return theData->bytes;
}

uint8_t *OCDataGetMutableBytePtr(OCMutableDataRef theData)
{
    return theData->bytes;
}

void OCDataGetBytes(OCDataRef theData, OCRange range, uint8_t *buffer)
{
    memmove(buffer, theData->bytes + range.location, range.length);
}

void OCDataSetLength(OCMutableDataRef theData, uint64_t length)
{
    if(length<theData->length) {
        theData->length = length;
        return;
    }
    if(theData->capacity==0) {
        if(NULL==theData->bytes) {
            theData->bytes = (uint8_t *) malloc(length*sizeof(uint8_t));
        }
        else theData->bytes = (uint8_t *) realloc(theData->bytes, length * sizeof(uint8_t));
        for(uint64_t index=theData->length;index<length;index++) {
            theData->bytes[index] = 0;
        }
        theData->length = length;
        return;
    }
    if(length>theData->length && length<theData->capacity) {
        for(uint64_t index=theData->length;index<length;index++) {
            theData->bytes[index] = 0;
        }
        theData->length = length;
        return;
    }
}

void OCDataIncreaseLength(OCMutableDataRef theData, uint64_t extraLength)
{
    OCDataSetLength(theData, theData->length+extraLength);
}

void OCDataAppendBytes(OCMutableDataRef theData, const uint8_t *bytes, uint64_t length)
{
    uint64_t oldLength = theData->length;
    OCDataIncreaseLength(theData, length);
    
    for(uint64_t index=0;index<length;index++) {
        theData->bytes[oldLength+index] = bytes[index];
    }
}



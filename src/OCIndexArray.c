//
//  OCIndexPairSet.c
//
//  Created by PhySy Ltd on 10/21/11.
//  Copyright (c) 2008-2014 PhySy Ltd. All rights reserved.
//

#include "OCLibrary.h"

static OCTypeID kOCIndexArrayID = _kOCNotATypeID;

struct __OCIndexArray {
    OCBase _base;

    OCMutableDataRef indexes;
};


OCTypeID OCIndexArrayGetTypeID(void)
{
    if (kOCIndexArrayID == _kOCNotATypeID)
        kOCIndexArrayID = OCRegisterType("OCIndexArray");
    return kOCIndexArrayID;
}

bool __OCIndexArrayEqual(OCIndexArrayRef input1, OCIndexArrayRef input2)
{
	IF_NO_OBJECT_EXISTS_RETURN(input1,false);
	IF_NO_OBJECT_EXISTS_RETURN(input2,false);
    
    if(OCDataGetLength(input1->indexes) != OCDataGetLength(input2->indexes)) return false;
    long count = OCIndexArrayGetCount(input1);
    for(long index = 0; index<count; index++) {
        if(OCIndexArrayGetValueAtIndex(input1, index) != OCIndexArrayGetValueAtIndex(input2, index)) return false;
    }
	return true;
}

static
static void __OCIndexArrayFinalize(const void *theType)
{
    OCMutableIndexArrayRef input = (OCMutableIndexArrayRef)theType;

    if(input->indexes) OCRelease(input->indexes);
    input->indexes = NULL;
}

static OCStringRef __OCIndexArrayCopyFormattingDesc(const void *theType)
{
    return NULL;
}

static struct __OCIndexArray *OCIndexArrayAllocate(void)
{
    return OCTypeAlloc(
        struct __OCIndexArray,
        OCIndexArrayGetTypeID(),
        __OCIndexArrayFinalize,
        __OCIndexArrayEqual,
        __OCIndexArrayCopyFormattingDesc);
}

OCIndexArrayRef OCIndexArrayCreate(long *indexes, long numValues)
{
    // Initialize object
    
    OCIndexArray *newIndexArray = [OCIndexArray alloc];
    
    // *** Setup attributes ***
    
    newIndexArray->indexes = (CFMutableDataRef) OCDataCreate(kCFAllocatorDefault, (const UInt8 *) indexes, numValues*sizeof(long));
    return (OCIndexArrayRef) newIndexArray;
}

PSMutableIndexArrayRef OCIndexArrayCreateMutable(long capacity)
{
    // Initialize object
    
    OCIndexArray *newIndexArray = [OCIndexArray alloc];
    
    // *** Setup attributes ***
    
    newIndexArray->indexes = OCDataCreateMutable(kCFAllocatorDefault, capacity*sizeof(long));
    OCDataSetLength(newIndexArray->indexes, capacity*sizeof(long));
    return (PSMutableIndexArrayRef) newIndexArray;
}

static OCIndexArrayRef OCIndexArrayCreateWithParameters(OCDataRef indexes)
{
    OCIndexArray *newIndexArray = [OCIndexArray alloc];
    
    // *** Setup attributes ***
    
    newIndexArray->indexes = (CFMutableDataRef) OCDataCreateCopy(kCFAllocatorDefault, indexes);
    return (OCIndexArrayRef) newIndexArray;
}

static PSMutableIndexArrayRef OCIndexArrayCreateMutableWithParameters(CFMutableDataRef indexes)
{
    OCIndexArray *newIndexArray = [OCIndexArray alloc];
    
    // *** Setup attributes ***
    
    newIndexArray->indexes = OCDataCreateMutableCopy(kCFAllocatorDefault,OCDataGetLength(indexes),indexes);
    return (PSMutableIndexArrayRef) newIndexArray;
}

OCIndexArrayRef OCIndexArrayCreateCopy(OCIndexArrayRef theIndexArray)
{
	return OCIndexArrayCreateWithParameters(theIndexArray->indexes);
}

PSMutableIndexArrayRef OCIndexArrayCreateMutableCopy(OCIndexArrayRef theIndexArray)
{
	return OCIndexArrayCreateMutableWithParameters(theIndexArray->indexes);
}

long OCIndexArrayGetCount(OCIndexArrayRef theIndexArray)
{
    if(theIndexArray->indexes) return OCDataGetLength(theIndexArray->indexes)/sizeof(long);
    return 0;
}

long *OCIndexArrayGetMutableBytePtr(OCIndexArrayRef theIndexArray) {
    if(theIndexArray->indexes)
        return (long *) OCDataGetMutableBytePtr(theIndexArray->indexes);
    return NULL;
}

long OCIndexArrayGetValueAtIndex(OCIndexArrayRef theIndexArray, long index)
{
    if(theIndexArray->indexes) {
        long *indexes = (long *) OCDataGetBytePtr(theIndexArray->indexes);
        return indexes[index];
    }
    return kCFNotFound;
}

bool OCIndexArraySetValueAtIndex(PSMutableIndexArrayRef theIndexArray, long index, long value)
{
    if(theIndexArray->indexes) {
        long *indexes = (long *) OCDataGetBytePtr(theIndexArray->indexes);
        indexes[index] = value;
        return true;
    }
    return false;
}

bool OCIndexArrayRemoveValueAtIndex(PSMutableIndexArrayRef theIndexArray, long index)
{
    if(theIndexArray->indexes) {
        long *oldIndexes = (long *) OCDataGetBytePtr(theIndexArray->indexes);
        long count = OCIndexArrayGetCount(theIndexArray);
        CFMutableDataRef data = OCDataCreateMutable(kCFAllocatorDefault, (count-1)*sizeof(long));
        OCDataSetLength(data, (count-1)*sizeof(long));
        long *newIndexes = (long *) OCDataGetMutableBytePtr(data);
        long j = 0;
        for(long i =0; i<count; i++) {
            if(i != index)
                newIndexes[j++] = oldIndexes[i];
        }
        OCRelease(theIndexArray->indexes);
        theIndexArray->indexes = data;
        return true;
    }
    return false;
 
}

void OCIndexArrayRemoveValuesAtIndexes(PSMutableIndexArrayRef theIndexArray, OCIndexSetRef theIndexSet)
{
    if(theIndexArray==NULL) return;
    if(theIndexSet==NULL) return;
    
long count = OCIndexSetGetCount(theIndexSet);
    if(count) {
        long index = OCIndexSetLastIndex(theIndexSet);
        OCIndexArrayRemoveValueAtIndex(theIndexArray, index);
        for(long i=0; i<count-1; i++) {
            index = OCIndexSetIndexLessThanIndex(theIndexSet, index);
            if(index==kCFNotFound) return;
            OCIndexArrayRemoveValueAtIndex(theIndexArray, index);
        }
    }
}


bool OCIndexArrayContainsIndex(OCIndexArrayRef theIndexArray, long index)
{
    if(theIndexArray->indexes) {
        long count = OCIndexArrayGetCount(theIndexArray);
        long *indexes = (long *) OCDataGetBytePtr(theIndexArray->indexes);
        for(int32_t i=0;i<count;i++) if(indexes[i] == index) return true;
    }
    return false;
}

bool OCIndexArrayAppendValue(PSMutableIndexArrayRef theIndexArray, long index)
{
    if(theIndexArray->indexes) {
        long count = OCIndexArrayGetCount(theIndexArray);
        OCDataIncreaseLength(theIndexArray->indexes, sizeof(long));
        long *indexes = (long *) OCDataGetBytePtr(theIndexArray->indexes);
        indexes[count] = index;
        return true;
    }
    return false;
}

CFStringRef OCIndexArrayCreateBase64String(OCIndexArrayRef theIndexArray, csdmNumericType integerType)
{
    long *indexes = (long *) OCDataGetBytePtr(theIndexArray->indexes);
    if(integerType == kCSDMNumberUInt8Type || integerType == kCSDMNumberSInt8Type) {
        long count = OCIndexArrayGetCount(theIndexArray);
        uint8_t *theIndexes = malloc(count*sizeof(uint8_t));
        for(long i=0;i<count;i++) theIndexes[i] = indexes[i];
        OCDataRef theData = OCDataCreate(kCFAllocatorDefault, theIndexes, sizeof(uint8_t)*count);
        CFStringRef result = (CFStringRef) [[(NSData *) theData base64EncodedStringWithOptions:0] retain];
        OCRelease(theData);
        free(theIndexes);
        return result;
    }
    if(integerType == kCSDMNumberUInt16Type || integerType == kCSDMNumberSInt16Type) {
        long count = OCIndexArrayGetCount(theIndexArray);
        uint16_t *theIndexes = malloc(count*sizeof(uint16_t));
        for(long i=0;i<count;i++) theIndexes[i] = indexes[i];
        OCDataRef theData = OCDataCreate(kCFAllocatorDefault, (const unsigned char *) theIndexes, sizeof(uint16_t)*count);
        CFStringRef result = (CFStringRef) [[(NSData *) theData base64EncodedStringWithOptions:0] retain];
        OCRelease(theData);
        free(theIndexes);
        return result;
    }
    if(integerType == kCSDMNumberUInt32Type || integerType == kCSDMNumberSInt32Type) {
        long count = OCIndexArrayGetCount(theIndexArray);
        uint32_t *theIndexes = malloc(count*sizeof(uint32_t));
        for(long i=0;i<count;i++) theIndexes[i] = (uint32_t) indexes[i];
        OCDataRef theData = OCDataCreate(kCFAllocatorDefault, (const unsigned char *) theIndexes, sizeof(uint32_t)*count);
        CFStringRef result = (CFStringRef) [[(NSData *) theData base64EncodedStringWithOptions:0] retain];
        OCRelease(theData);
        free(theIndexes);
        return result;
    }
    if(integerType == kCSDMNumberUInt64Type || integerType == kCSDMNumberSInt64Type) {
        long count = OCIndexArrayGetCount(theIndexArray);
        uint64_t *theIndexes = malloc(count*sizeof(uint64_t));
        for(long i=0;i<count;i++) theIndexes[i] = indexes[i];
        OCDataRef theData = OCDataCreate(kCFAllocatorDefault, (const unsigned char *) theIndexes, sizeof(uint64_t)*count);
        CFStringRef result = (CFStringRef) [[(NSData *) theData base64EncodedStringWithOptions:0] retain];
        OCRelease(theData);
        free(theIndexes);
        return result;
    }
    return (CFStringRef) [[(NSData *) theIndexArray->indexes base64EncodedStringWithOptions:0] retain];
}

bool OCIndexArrayAppendValues(PSMutableIndexArrayRef theIndexArray, OCIndexArrayRef arrayToAppend)
{
    if(theIndexArray->indexes) {
        CFMutableDataRef newIndexes = OCDataCreateMutableCopy(kCFAllocatorDefault, 0, theIndexArray->indexes);
        OCDataAppendBytes(newIndexes, OCDataGetBytePtr(arrayToAppend->indexes), OCDataGetLength(arrayToAppend->indexes));
        OCRelease(theIndexArray->indexes);
        theIndexArray->indexes = newIndexes;
        return true;
    }
    return false;
}



CFArrayRef OCIndexArrayCreateCFNumberArray(OCIndexArrayRef theIndexArray)
{
    long count = OCIndexArrayGetCount(theIndexArray);
    CFMutableArrayRef theArray = CFArrayCreateMutable(kCFAllocatorDefault, count, &kCFTypeArrayCallBacks);
    for(long i = 0;i<count;i++) {
        long index = OCIndexArrayGetValueAtIndex(theIndexArray, i);
        CFNumberRef number = PSCFNumberCreateWithlong(index);
        CFArrayAppendValue(theArray, number);
        OCRelease(number);
    }
    return theArray;
}

CFDictionaryRef OCIndexArrayCreatePList(OCIndexArrayRef theIndexArray)
{
    CFMutableDictionaryRef dictionary = nil;
    if(theIndexArray) {
        dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault,0,&kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
        CFDictionarySetValue( dictionary, CFSTR("indexes"), theIndexArray->indexes);
        
	}
	return dictionary;
}

OCIndexArrayRef OCIndexArrayCreateWithPList(CFDictionaryRef dictionary)
{
	if(dictionary==NULL) return NULL;
	return OCIndexArrayCreateWithParameters(CFDictionaryGetValue(dictionary, CFSTR("indexes")));
}


OCDataRef OCIndexArrayCreateData(OCIndexArrayRef theIndexArray)
{
    if(theIndexArray == NULL) return NULL;
    return CFRetain(theIndexArray->indexes);
}

OCIndexArrayRef OCIndexArrayCreateWithData(OCDataRef data)
{
    if(data==nil) return nil;
    return OCIndexArrayCreateWithParameters(data);
}


void OCIndexArrayShow(OCIndexArrayRef theIndexArray)
{
    long count = OCIndexArrayGetCount(theIndexArray);
    fprintf(stderr, "(");
    for(long index = 0; index<count; index++)
    {
        fprintf(stderr, "%ld", OCIndexArrayGetValueAtIndex(theIndexArray, index));
        if(index<count-1) fprintf(stderr, ",");
    }
    fprintf(stderr, ")\n");
}




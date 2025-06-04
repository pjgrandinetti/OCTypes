//
//  OCIndexPairSet.c
//
//  Created by PhySy Ltd on 10/21/11.
//  Copyright (c) 2008-2014 PhySy Ltd. All rights reserved.
//

#include "OCLibrary.h"

#include "OCLibrary.h"

static OCTypeID kOCIndexPairSetID = _kOCNotATypeID;

struct __OCIndexPairSet {
    OCBase _base;

    OCDataRef indexPairs;
};

OCTypeID OCIndexPairSetGetTypeID(void)
{
    if (kOCIndexPairSetID == _kOCNotATypeID)
        kOCIndexPairSetID = OCRegisterType("OCIndexPairSet");
    return kOCIndexPairSetID;
}

bool __OCIndexPairSetEqual(const void *a_, const void *b_)
{
    OCIndexPairSetRef input1 = (OCIndexPairSetRef)a_;
    OCIndexPairSetRef input2 = (OCIndexPairSetRef)b_;

    if(OCDataGetLength(input1->indexPairs) != OCDataGetLength(input2->indexPairs)) return false;
    if(!OCTypeEqual(input1->indexPairs, input2->indexPairs)) return false;
	return true;
}

static void __OCIndexPairSetFinalize(const void *theType)
{
    OCIndexPairSetRef input = (OCIndexPairSetRef)theType;

    if(input->indexPairs) OCRelease(input->indexPairs);
    input->indexPairs = NULL;
}

static OCStringRef __OCIndexPairSetCopyFormattingDesc(const void *theType)
{
    return NULL;
}

static struct __OCIndexPairSet *OCIndexPairSetAllocate(void)
{
    return OCTypeAlloc(
        struct __OCIndexPairSet,
        OCIndexPairSetGetTypeID(),
        __OCIndexPairSetFinalize,
        __OCIndexPairSetEqual,
        __OCIndexPairSetCopyFormattingDesc);
}

OCIndexPairSetRef OCIndexPairSetCreate(void)
{
    OCIndexPairSet *newIndexSet = OCIndexPairSetAllocate();
    newIndexSet->indexPairs = NULL;
    return (OCIndexPairSetRef) newIndexSet;
}

OCMutableIndexPairSetRef OCIndexPairSetCreateMutable(void)
{
    OCIndexPairSet *newIndexSet = [OCIndexPairSet alloc];
    newIndexSet->indexPairs = NULL;
    return (OCMutableIndexPairSetRef) newIndexSet;
}

static OCIndexPairSetRef OCIndexPairSetCreateWithParameters(OCDataRef indexPairs)
{
    OCIndexPairSet *newIndexSet = [OCIndexPairSet alloc];
    newIndexSet->indexPairs = OCDataCreateCopy(kCFAllocatorDefault, indexPairs);
    return (OCIndexPairSetRef) newIndexSet;
}

static OCMutableIndexPairSetRef OCIndexPairSetCreateMutableWithParameters(OCDataRef indexPairs)
{
    OCIndexPairSet *newIndexSet = [OCIndexPairSet alloc];
    newIndexSet->indexPairs = OCDataCreateCopy(kCFAllocatorDefault,indexPairs);
    return (OCMutableIndexPairSetRef) newIndexSet;
}

OCIndexPairSetRef OCIndexPairSetCreateCopy(OCIndexPairSetRef theIndexSet)
{
	return OCIndexPairSetCreateWithParameters(theIndexSet->indexPairs);
}

OCMutableIndexPairSetRef OCIndexPairSetCreateMutableCopy(OCIndexPairSetRef theIndexSet)
{
	return OCIndexPairSetCreateMutableWithParameters(theIndexSet->indexPairs);
}

OCMutableIndexPairSetRef OCIndexPairSetCreateMutableWithIndexArray(PSIndexArrayRef indexArray)
{
    OCMutableIndexPairSetRef indexPairSet = OCIndexPairSetCreateMutable();
    for(long index = 0; index<PSIndexArrayGetCount(indexArray); index++) {
        OCIndexPairSetAddIndexPair(indexPairSet, index, PSIndexArrayGetValueAtIndex(indexArray, index));
    }
    return indexPairSet;
}

OCIndexPairSetRef OCIndexPairSetCreateWithIndexPairArray(PSIndexPair *array, int numValues)
{
    OCIndexPairSet *newIndexSet = [OCIndexPairSet alloc];
    newIndexSet->indexPairs = OCDataCreate((const UInt8 *) array, numValues*sizeof(PSIndexPair));
    return (OCIndexPairSetRef) newIndexSet;

}

OCIndexPairSetRef OCIndexPairSetCreateWithIndexPair(long index, long value)
{
    OCIndexPairSet *newIndexSet = [OCIndexPairSet alloc];
    PSIndexPair indexPair = {index,value};
    // *** Setup attributes ***
    
    newIndexSet->indexPairs = OCDataCreate((const UInt8 *) &indexPair, sizeof(PSIndexPair));
    return (OCIndexPairSetRef) newIndexSet;
}

OCIndexPairSetRef OCIndexPairSetCreateWithTwoIndexPairs(long index1, long value1, long index2, long value2)
{
    OCIndexPairSet *newIndexSet = [OCIndexPairSet alloc];
    PSIndexPair indexPair[2] = {index1,value1,index2,value2};
    // *** Setup attributes ***
    
    newIndexSet->indexPairs = OCDataCreate((const UInt8 *) &indexPair, sizeof(PSIndexPair));
    return (OCIndexPairSetRef) newIndexSet;
}

OCDataRef OCIndexPairSetGetIndexPairs(OCIndexPairSetRef theIndexSet)
{
    return theIndexSet->indexPairs;
}

long OCIndexPairSetValueForIndex(OCIndexPairSetRef theIndexSet, long index)
{
    long count = OCIndexPairSetGetCount(theIndexSet);
    PSIndexPair *indexPairs = (PSIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
    for(long i=0;i<count; i++) {
        if(indexPairs[i].index == index) return indexPairs[i].value;
    }
    return kCFNotFound;
}

PSIndexArrayRef OCIndexPairSetCreateIndexArrayOfValues(OCIndexPairSetRef theIndexSet)
{
    long count = OCIndexPairSetGetCount(theIndexSet);
    OCMutableIndexArrayRef values = PSIndexArrayCreateMutable(count);
    PSIndexPair *indexPairs = (PSIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
    for(long index=0;index<OCIndexPairSetGetCount(theIndexSet); index++) {
        PSIndexArraySetValueAtIndex(values, index, indexPairs[index].value);
    }
    return values;
}

PSIndexSetRef OCIndexPairSetCreateIndexSetOfIndexes(OCIndexPairSetRef theIndexSet)
{
    OCMutableIndexSetRef indexes = PSIndexSetCreateMutable();
    PSIndexPair *indexPairs = (PSIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
    for(long index=0;index<OCIndexPairSetGetCount(theIndexSet); index++) {
        PSIndexSetAddIndex(indexes, indexPairs[index].index);
    }
    return indexes;
}

PSIndexPair *OCIndexPairSetGetBytePtr(OCIndexPairSetRef theIndexSet)
{
    if(theIndexSet->indexPairs)
        return (PSIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
    return NULL;
}

long OCIndexPairSetGetCount(OCIndexPairSetRef theIndexSet)
{
    if(theIndexSet->indexPairs) return OCDataGetLength(theIndexSet->indexPairs)/sizeof(PSIndexPair);
    return 0;
}

PSIndexPair OCIndexPairSetFirstIndex(OCIndexPairSetRef theIndexSet)
{
    if(theIndexSet->indexPairs) {
        PSIndexPair *indexPairs = (PSIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
        return indexPairs[0];
    }
    PSIndexPair result = {kCFNotFound,0};
    return result;
}

PSIndexPair OCIndexPairSetLastIndex(OCIndexPairSetRef theIndexSet)
{
    if(theIndexSet->indexPairs) {
        long count = OCIndexPairSetGetCount(theIndexSet);
        PSIndexPair *indexPairs = (PSIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
        return indexPairs[count-1];
    }
    PSIndexPair result = {kCFNotFound,0};
    return result;
}

PSIndexPair OCIndexPairSetIndexPairLessThanIndexPair(OCIndexPairSetRef theIndexSet, PSIndexPair indexPair)
{
    PSIndexPair noResult = {kCFNotFound,0};
    if(theIndexSet==NULL || theIndexSet->indexPairs == NULL) return noResult;
    long count = OCIndexPairSetGetCount(theIndexSet);
    PSIndexPair *indexPairs = (PSIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
    for(long i=count-1; i>=0;i--) {
        if(indexPairs[i].index<indexPair.index) return indexPairs[i];
    }
    return noResult;
}

bool OCIndexPairSetContainsIndex(OCIndexPairSetRef theIndexSet, long index)
{
    if(theIndexSet->indexPairs) {
        long count = OCIndexPairSetGetCount(theIndexSet);
        PSIndexPair *indexPairs = (PSIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
        for(int32_t i=0;i<count;i++)
            if(indexPairs[i].index == index) return true;
    }
    return false;
}

bool OCIndexPairSetContainsIndexPair(OCIndexPairSetRef theIndexSet, PSIndexPair indexPair)
{
    if(theIndexSet->indexPairs) {
        long count = OCIndexPairSetGetCount(theIndexSet);
        PSIndexPair *indexPairs = (PSIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
        for(int32_t i=0;i<count;i++)
            if(indexPairs[i].index == indexPair.index && indexPairs[i].value == indexPair.value) return true;
    }
    return false;
}

bool OCIndexPairSetRemoveIndexPairWithIndex(OCMutableIndexPairSetRef theIndexSet, long index)
{
    if(!OCIndexPairSetContainsIndex(theIndexSet, index)) return false;

    long count = OCIndexPairSetGetCount(theIndexSet);
    if(count<1) return false;
    
    PSIndexPair *indexPairs = (PSIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
    
    PSIndexPair newIndexPairs[count-1];
    long i = 0;
    for(long j = 0; j < count; j++) {
        if(indexPairs[j].index != index) {
            newIndexPairs[i++] = indexPairs[j];
        }
    }
    OCRelease(theIndexSet->indexPairs);
    
    theIndexSet->indexPairs = OCDataCreate((const UInt8 *) newIndexPairs, (count-1)*sizeof(PSIndexPair));
    return true;
}


bool OCIndexPairSetAddIndexPair(OCMutableIndexPairSetRef theIndexSet, long index, long value)
{
    int32_t i = 0;
    long count = 0;
    PSIndexPair indexPair = {index, value};

    if(theIndexSet->indexPairs) {
        count = OCIndexPairSetGetCount(theIndexSet);
        PSIndexPair *indexPairs = (PSIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
        for(i=0;i<count;i++) {
            if(indexPairs[i].index == indexPair.index) return false;
            if(indexPairs[i].index>indexPair.index) break;
        }
        PSIndexPair *newIndexPairs = malloc((count+1)*sizeof(PSIndexPair));
        for(int32_t j = 0;j<count+1; j++) {
            if(j<i) newIndexPairs[j] = indexPairs[j];
            else if(j==i) newIndexPairs[j] = indexPair;
            else newIndexPairs[j] = indexPairs[j-1];
        }
        OCRelease(theIndexSet->indexPairs);
        theIndexSet->indexPairs = OCDataCreate(CFGetAllocator(theIndexSet), (const UInt8 *) newIndexPairs,(count+1)*sizeof(PSIndexPair));
        free(newIndexPairs);
        return true;
    }
    theIndexSet->indexPairs = OCDataCreate(CFGetAllocator(theIndexSet), (const UInt8 *) &indexPair, sizeof(PSIndexPair));
    return true;
}


OCDictionaryRef OCIndexPairSetCreatePList(OCIndexPairSetRef theIndexSet)
{
    CFMutableDictionaryRef dictionary = nil;
    if(theIndexSet) {
        dictionary = OCDictionaryCreateMutable(&kOCTypeDictionaryKeyCallBacks,&kOCTypeDictionaryValueCallBacks);
        OCDictionarySetValue( dictionary, CFSTR("indexPairs"), theIndexSet->indexPairs);
        
	}
	return dictionary;
}

OCIndexPairSetRef OCIndexPairSetCreateWithPList(OCDictionaryRef dictionary)
{
	if(dictionary==NULL) return NULL;
	return OCIndexPairSetCreateWithParameters(OCDictionaryGetValue(dictionary, CFSTR("indexPairs")));
}


OCDataRef OCIndexPairSetCreateData(OCIndexPairSetRef theIndexSet)
{
    if(theIndexSet == NULL) return NULL;
    return CFRetain(theIndexSet->indexPairs);
}

OCIndexPairSetRef OCIndexPairSetCreateWithData(OCDataRef data)
{
    if(data==nil) return nil;
    return OCIndexPairSetCreateWithParameters(data);
}

void OCIndexPairSetShow(OCIndexPairSetRef theIndexPairSet)
{
    PSIndexPair *indexPairs = (PSIndexPair *) OCDataGetBytePtr(theIndexPairSet->indexPairs);
    long count = OCIndexPairSetGetCount(theIndexPairSet);
    fprintf(stderr,"(");
    for(long index=0; index<count; index++) {
        fprintf(stderr, "(%ld,%ld)",indexPairs[index].index, indexPairs[index].value);
        if(index!=count-1)     fprintf(stderr,",");
    }
    fprintf(stderr,")\n");
}



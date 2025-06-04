//
//  OCIndexPairSet.c
//
//  Created by PhySy Ltd on 10/21/11.
//  Copyright (c) 2008-2014 PhySy Ltd. All rights reserved.
//

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
    OCMutableIndexPairSetRef input = (OCMutableIndexPairSetRef)theType;

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
    OCMutableIndexPairSetRef newIndexSet = (OCMutableIndexPairSetRef) OCIndexPairSetAllocate();
    newIndexSet->indexPairs = NULL;
    return (OCIndexPairSetRef) newIndexSet;
}

OCMutableIndexPairSetRef OCIndexPairSetCreateMutable(void)
{
    OCMutableIndexPairSetRef newIndexSet = (OCMutableIndexPairSetRef) OCIndexPairSetAllocate();
    newIndexSet->indexPairs = NULL;
    return newIndexSet;
}

static OCIndexPairSetRef OCIndexPairSetCreateWithParameters(OCDataRef indexPairs)
{
    OCMutableIndexPairSetRef newIndexSet = (OCMutableIndexPairSetRef) OCIndexPairSetAllocate();
    newIndexSet->indexPairs = OCDataCreateCopy(indexPairs);
    return (OCIndexPairSetRef) newIndexSet;
}

static OCMutableIndexPairSetRef OCIndexPairSetCreateMutableWithParameters(OCDataRef indexPairs)
{
    OCMutableIndexPairSetRef newIndexSet = (OCMutableIndexPairSetRef) OCIndexPairSetAllocate();
    newIndexSet->indexPairs = OCDataCreateCopy(indexPairs);
    return newIndexSet;
}

OCIndexPairSetRef OCIndexPairSetCreateCopy(OCIndexPairSetRef theIndexSet)
{
	return OCIndexPairSetCreateWithParameters(theIndexSet->indexPairs);
}

OCMutableIndexPairSetRef OCIndexPairSetCreateMutableCopy(OCIndexPairSetRef theIndexSet)
{
	return OCIndexPairSetCreateMutableWithParameters(theIndexSet->indexPairs);
}

OCMutableIndexPairSetRef OCIndexPairSetCreateMutableWithIndexArray(OCIndexArrayRef indexArray)
{
    OCMutableIndexPairSetRef indexPairSet = OCIndexPairSetCreateMutable();
    for(long index = 0; index<OCIndexArrayGetCount(indexArray); index++) {
        OCIndexPairSetAddIndexPair(indexPairSet, index, OCIndexArrayGetValueAtIndex(indexArray, index));
    }
    return indexPairSet;
}

OCIndexPairSetRef OCIndexPairSetCreateWithIndexPairArray(OCIndexPair *array, int numValues)
{
    OCMutableIndexPairSetRef newIndexSet = (OCMutableIndexPairSetRef) OCIndexPairSetAllocate();
    newIndexSet->indexPairs = OCDataCreate((const UInt8 *) array, numValues*sizeof(OCIndexPair));
    return (OCIndexPairSetRef) newIndexSet;

}

OCIndexPairSetRef OCIndexPairSetCreateWithIndexPair(long index, long value)
{
    OCMutableIndexPairSetRef newIndexSet = (OCMutableIndexPairSetRef) OCIndexPairSetAllocate();
    OCIndexPair indexPair = {index,value};
    // *** Setup attributes ***
    
    newIndexSet->indexPairs = OCDataCreate((const UInt8 *) &indexPair, sizeof(OCIndexPair));
    return (OCIndexPairSetRef) newIndexSet;
}

OCIndexPairSetRef OCIndexPairSetCreateWithTwoIndexPairs(long index1, long value1, long index2, long value2)
{
    OCMutableIndexPairSetRef newIndexSet = (OCMutableIndexPairSetRef) OCIndexPairSetAllocate();
    OCIndexPair indexPair[2] = {index1,value1,index2,value2};
    // *** Setup attributes ***
    
    newIndexSet->indexPairs = OCDataCreate((const UInt8 *) &indexPair, sizeof(OCIndexPair));
    return (OCIndexPairSetRef) newIndexSet;
}

OCDataRef OCIndexPairSetGetIndexPairs(OCIndexPairSetRef theIndexSet)
{
    return theIndexSet->indexPairs;
}

long OCIndexPairSetValueForIndex(OCIndexPairSetRef theIndexSet, long index)
{
    long count = OCIndexPairSetGetCount(theIndexSet);
    OCIndexPair *indexPairs = (OCIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
    for(long i=0;i<count; i++) {
        if(indexPairs[i].index == index) return indexPairs[i].value;
    }
    return kCFNotFound;
}

OCIndexArrayRef OCIndexPairSetCreateIndexArrayOfValues(OCIndexPairSetRef theIndexSet)
{
    long count = OCIndexPairSetGetCount(theIndexSet);
    OCMutableIndexArrayRef values = OCIndexArrayCreateMutable(count);
    OCIndexPair *indexPairs = (OCIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
    for(long index=0;index<OCIndexPairSetGetCount(theIndexSet); index++) {
        OCIndexArraySetValueAtIndex(values, index, indexPairs[index].value);
    }
    return values;
}

OCIndexSetRef OCIndexPairSetCreateIndexSetOfIndexes(OCIndexPairSetRef theIndexSet)
{
    OCMutableIndexSetRef indexes = OCIndexSetCreateMutable();
    OCIndexPair *indexPairs = (OCIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
    for(long index=0;index<OCIndexPairSetGetCount(theIndexSet); index++) {
        OCIndexSetAddIndex(indexes, indexPairs[index].index);
    }
    return indexes;
}

OCIndexPair *OCIndexPairSetGetBytePtr(OCIndexPairSetRef theIndexSet)
{
    if(theIndexSet->indexPairs)
        return (OCIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
    return NULL;
}

long OCIndexPairSetGetCount(OCIndexPairSetRef theIndexSet)
{
    if(theIndexSet->indexPairs) return OCDataGetLength(theIndexSet->indexPairs)/sizeof(OCIndexPair);
    return 0;
}

OCIndexPair OCIndexPairSetFirstIndex(OCIndexPairSetRef theIndexSet)
{
    if(theIndexSet->indexPairs) {
        OCIndexPair *indexPairs = (OCIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
        return indexPairs[0];
    }
    OCIndexPair result = {kCFNotFound,0};
    return result;
}

OCIndexPair OCIndexPairSetLastIndex(OCIndexPairSetRef theIndexSet)
{
    if(theIndexSet->indexPairs) {
        long count = OCIndexPairSetGetCount(theIndexSet);
        OCIndexPair *indexPairs = (OCIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
        return indexPairs[count-1];
    }
    OCIndexPair result = {kCFNotFound,0};
    return result;
}

OCIndexPair OCIndexPairSetIndexPairLessThanIndexPair(OCIndexPairSetRef theIndexSet, OCIndexPair indexPair)
{
    OCIndexPair noResult = {kCFNotFound,0};
    if(theIndexSet==NULL || theIndexSet->indexPairs == NULL) return noResult;
    long count = OCIndexPairSetGetCount(theIndexSet);
    OCIndexPair *indexPairs = (OCIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
    for(long i=count-1; i>=0;i--) {
        if(indexPairs[i].index<indexPair.index) return indexPairs[i];
    }
    return noResult;
}

bool OCIndexPairSetContainsIndex(OCIndexPairSetRef theIndexSet, long index)
{
    if(theIndexSet->indexPairs) {
        long count = OCIndexPairSetGetCount(theIndexSet);
        OCIndexPair *indexPairs = (OCIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
        for(int32_t i=0;i<count;i++)
            if(indexPairs[i].index == index) return true;
    }
    return false;
}

bool OCIndexPairSetContainsIndexPair(OCIndexPairSetRef theIndexSet, OCIndexPair indexPair)
{
    if(theIndexSet->indexPairs) {
        long count = OCIndexPairSetGetCount(theIndexSet);
        OCIndexPair *indexPairs = (OCIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
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
    
    OCIndexPair *indexPairs = (OCIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
    
    OCIndexPair newIndexPairs[count-1];
    long i = 0;
    for(long j = 0; j < count; j++) {
        if(indexPairs[j].index != index) {
            newIndexPairs[i++] = indexPairs[j];
        }
    }
    OCRelease(theIndexSet->indexPairs);
    
    theIndexSet->indexPairs = OCDataCreate((const UInt8 *) newIndexPairs, (count-1)*sizeof(OCIndexPair));
    return true;
}


bool OCIndexPairSetAddIndexPair(OCMutableIndexPairSetRef theIndexSet, long index, long value)
{
    int32_t i = 0;
    long count = 0;
    OCIndexPair indexPair = {index, value};

    if(theIndexSet->indexPairs) {
        count = OCIndexPairSetGetCount(theIndexSet);
        OCIndexPair *indexPairs = (OCIndexPair *) OCDataGetBytePtr(theIndexSet->indexPairs);
        for(i=0;i<count;i++) {
            if(indexPairs[i].index == indexPair.index) return false;
            if(indexPairs[i].index>indexPair.index) break;
        }
        OCIndexPair *newIndexPairs = malloc((count+1)*sizeof(OCIndexPair));
        for(int32_t j = 0;j<count+1; j++) {
            if(j<i) newIndexPairs[j] = indexPairs[j];
            else if(j==i) newIndexPairs[j] = indexPair;
            else newIndexPairs[j] = indexPairs[j-1];
        }
        OCRelease(theIndexSet->indexPairs);
        theIndexSet->indexPairs = OCDataCreate(CFGetAllocator(theIndexSet), (const UInt8 *) newIndexPairs,(count+1)*sizeof(OCIndexPair));
        free(newIndexPairs);
        return true;
    }
    theIndexSet->indexPairs = OCDataCreate(CFGetAllocator(theIndexSet), (const UInt8 *) &indexPair, sizeof(OCIndexPair));
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
    OCIndexPair *indexPairs = (OCIndexPair *) OCDataGetBytePtr(theIndexPairSet->indexPairs);
    long count = OCIndexPairSetGetCount(theIndexPairSet);
    fprintf(stderr,"(");
    for(long index=0; index<count; index++) {
        fprintf(stderr, "(%ld,%ld)",indexPairs[index].index, indexPairs[index].value);
        if(index!=count-1)     fprintf(stderr,",");
    }
    fprintf(stderr,")\n");
}



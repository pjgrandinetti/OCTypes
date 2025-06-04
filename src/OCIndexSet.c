//
//  PSIndexSet.c
//
//  Created by PhySy Ltd on 10/21/11.
//  Copyright (c) 2008-2014 PhySy Ltd. All rights reserved.
//

#include "OCLibrary.h"

@interface PSIndexSet ()
{
@private
    CFDataRef indexes;
}
@end


@implementation PSIndexSet

- (void) dealloc
{
    if(indexes) CFRelease(indexes);
    [super dealloc];
}

PSIndexSetRef PSIndexSetCreate()
{
    // Initialize object
    
    PSIndexSet *newIndexSet = [PSIndexSet alloc];
    
    // *** Setup attributes ***
    
    newIndexSet->indexes = NULL;
    return (PSIndexSetRef) newIndexSet;
}

PSMutableIndexSetRef PSIndexSetCreateMutable(void)
{
    // Initialize object
    
    PSIndexSet *newIndexSet = [PSIndexSet alloc];
    
    // *** Setup attributes ***
    
    newIndexSet->indexes = NULL;
    return (PSMutableIndexSetRef) newIndexSet;
}

static PSIndexSetRef PSIndexSetCreateWithParameters(CFDataRef indexes)
{
    PSIndexSet *newIndexSet = [PSIndexSet alloc];
    
    // *** Setup attributes ***
    
    newIndexSet->indexes = CFDataCreateCopy(kCFAllocatorDefault, indexes);
    return (PSIndexSetRef) newIndexSet;
}

static PSMutableIndexSetRef PSIndexSetCreateMutableWithParameters(CFDataRef indexes)
{
    PSIndexSet *newIndexSet = [PSIndexSet alloc];
    
    // *** Setup attributes ***
    
    newIndexSet->indexes = CFDataCreateCopy(kCFAllocatorDefault,indexes);
    return (PSMutableIndexSetRef) newIndexSet;
}

PSIndexSetRef PSIndexSetCreateCopy(PSIndexSetRef theIndexSet)
{
	return PSIndexSetCreateWithParameters(theIndexSet->indexes);
}

PSMutableIndexSetRef PSIndexSetCreateMutableCopy(PSIndexSetRef theIndexSet)
{
	return PSIndexSetCreateMutableWithParameters(theIndexSet->indexes);
}

PSIndexSetRef PSIndexSetCreateWithIndex(CFIndex index)
{
    PSIndexSet *newIndexSet = [PSIndexSet alloc];
    
    // *** Setup attributes ***
    
    newIndexSet->indexes = CFDataCreate(kCFAllocatorDefault, (const UInt8 *) &index, sizeof(CFIndex));
    return (PSIndexSetRef) newIndexSet;
}

PSIndexSetRef PSIndexSetCreateWithIndexesInRange(CFRange indexRange)
{
    PSIndexSet *newIndexSet = [PSIndexSet alloc];
    
    // *** Setup attributes ***
    CFIndex *indexes = malloc(indexRange.length*sizeof(CFIndex));
    for(int32_t index = 0;index<indexRange.length; index++) indexes[index] = indexRange.location + index;
    newIndexSet->indexes = CFDataCreate(kCFAllocatorDefault, (const UInt8 *) indexes, indexRange.length*sizeof(CFIndex));
    free(indexes);
    return (PSIndexSetRef) newIndexSet;
}

CFDataRef PSIndexSetGetIndexes(PSIndexSetRef theIndexSet)
{
    return theIndexSet->indexes;
}

CFIndex *PSIndexSetGetBytePtr(PSIndexSetRef theIndexSet)
{
    if(theIndexSet->indexes)
        return (CFIndex *) CFDataGetBytePtr(theIndexSet->indexes);
    return NULL;
}

CFIndex PSIndexSetGetCount(PSIndexSetRef theIndexSet)
{
    if(theIndexSet->indexes) return CFDataGetLength(theIndexSet->indexes)/sizeof(CFIndex);
    return 0;
}

CFIndex PSIndexSetFirstIndex(PSIndexSetRef theIndexSet)
{
    if(theIndexSet->indexes) {
        CFIndex *indexes = (CFIndex *) CFDataGetBytePtr(theIndexSet->indexes);
        return indexes[0];
    }
    return kCFNotFound;
}

CFIndex PSIndexSetLastIndex(PSIndexSetRef theIndexSet)
{
    if(theIndexSet->indexes) {
        CFIndex count = PSIndexSetGetCount(theIndexSet);
        CFIndex *indexes = (CFIndex *) CFDataGetBytePtr(theIndexSet->indexes);
        return indexes[count-1];
    }
    return kCFNotFound;
}

CFIndex PSIndexSetIndexLessThanIndex(PSIndexSetRef theIndexSet, CFIndex index)
{
    if(theIndexSet==NULL || theIndexSet->indexes == NULL) return kCFNotFound;
    CFIndex count = PSIndexSetGetCount(theIndexSet);
    CFIndex *indexes = (CFIndex *) CFDataGetBytePtr(theIndexSet->indexes);
    for(CFIndex i=count-1; i>=0;i--) {
        if(indexes[i]<index) return indexes[i];
    }
    return kCFNotFound;
}

CFIndex PSIndexSetIndexGreaterThanIndex(PSIndexSetRef theIndexSet, CFIndex index)
{
    if(theIndexSet==NULL || theIndexSet->indexes == NULL) return kCFNotFound;
    CFIndex count = PSIndexSetGetCount(theIndexSet);
    CFIndex *indexes = (CFIndex *) CFDataGetBytePtr(theIndexSet->indexes);
    for(CFIndex i=count-1; i>=0;i--) {
        if(indexes[i]>index) return indexes[i];
    }
    return kCFNotFound;
}

bool PSIndexSetContainsIndex(PSIndexSetRef theIndexSet, CFIndex index)
{
    if(theIndexSet->indexes) {
        CFIndex count = PSIndexSetGetCount(theIndexSet);
        CFIndex *indexes = (CFIndex *) CFDataGetBytePtr(theIndexSet->indexes);
        for(int32_t i=0;i<count;i++) if(indexes[i] == index) return true;
    }
    return false;
}

bool PSIndexSetAddIndex(PSMutableIndexSetRef theIndexSet, CFIndex index)
{
    int32_t i = 0;
    CFIndex count = 0;
    if(theIndexSet->indexes) {
        count = PSIndexSetGetCount(theIndexSet);
        CFIndex *indexes = (CFIndex *) CFDataGetBytePtr(theIndexSet->indexes);
        for(i=0;i<count;i++) {
            if(indexes[i] == index) return false;
            if(indexes[i]>index) break;
        }
        CFIndex *newIndexes = malloc((count+1)*sizeof(CFIndex));
        for(int32_t j = 0;j<count+1; j++) {
            if(j<i) newIndexes[j] = indexes[j];
            else if(j==i) newIndexes[j] = index;
            else newIndexes[j] = indexes[j-1];
        }
        CFRelease(theIndexSet->indexes);
        theIndexSet->indexes = CFDataCreate(CFGetAllocator(theIndexSet), (const UInt8 *) newIndexes,(count+1)*sizeof(CFIndex));
        free(newIndexes);
        return true;
    }
    theIndexSet->indexes = CFDataCreate(CFGetAllocator(theIndexSet), (const UInt8 *) &index, sizeof(CFIndex));
    return true;
}

bool PSIndexSetEqual(PSIndexSetRef input1, PSIndexSetRef input2)
{
	IF_NO_OBJECT_EXISTS_RETURN(input1,false);
	IF_NO_OBJECT_EXISTS_RETURN(input2,false);
    
    if(CFDataGetLength(input1->indexes) != CFDataGetLength(input2->indexes)) return false;
	if(!memcmp(CFDataGetBytePtr(input1->indexes), CFDataGetBytePtr(input2->indexes), CFDataGetLength(input1->indexes))) return false;
	return true;
}


CFArrayRef PSIndexSetCreateCFNumberArray(PSIndexSetRef theIndexSet)
{
    CFIndex count = PSIndexSetGetCount(theIndexSet);
    CFMutableArrayRef theArray = CFArrayCreateMutable(kCFAllocatorDefault, count, &kCFTypeArrayCallBacks);
    CFIndex *indexes = PSIndexSetGetBytePtr(theIndexSet);

    for(CFIndex i = 0;i<count;i++) {
        CFNumberRef number = PSCFNumberCreateWithCFIndex(indexes[i]);
        CFArrayAppendValue(theArray, number);
        CFRelease(number);
    }
    return theArray;
}

CFDictionaryRef PSIndexSetCreatePList(PSIndexSetRef theIndexSet)
{
    CFMutableDictionaryRef dictionary = nil;
    if(theIndexSet) {
        dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault,0,&kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
        CFDictionarySetValue( dictionary, CFSTR("indexes"), theIndexSet->indexes);
        
	}
	return dictionary;
}

PSIndexSetRef PSIndexSetCreateWithPList(CFDictionaryRef dictionary)
{
	if(dictionary==NULL) return NULL;
	return PSIndexSetCreateWithParameters(CFDictionaryGetValue(dictionary, CFSTR("indexes")));
}


CFDataRef PSIndexSetCreateData(PSIndexSetRef theIndexSet)
{
    if(theIndexSet == NULL) return NULL;
    return CFRetain(theIndexSet->indexes);
}

PSIndexSetRef PSIndexSetCreateWithData(CFDataRef data)
{
    if(data==nil) return nil;
    return PSIndexSetCreateWithParameters(data);
}



@end



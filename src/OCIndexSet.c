#include "OCLibrary.h"
#include <stdio.h>  
#include <stdlib.h> 
#include <string.h> 


//-----------------------------------------------------------------------------
// Static TypeID
//-----------------------------------------------------------------------------
static OCTypeID kOCIndexSetID = _kOCNotATypeID;

struct __OCIndexSet {
    OCBase _base;

    OCDataRef indexes;
};

//-----------------------------------------------------------------------------
// Equality Callback
//-----------------------------------------------------------------------------
bool __OCIndexSetEqual(const void *a_, const void *b_)
{
    OCIndexSetRef input1 = (OCIndexSetRef)a_;
    OCIndexSetRef input2 = (OCIndexSetRef)b_;

    IF_NO_OBJECT_EXISTS_RETURN(input1, false);
    IF_NO_OBJECT_EXISTS_RETURN(input2, false);

    // Compare lengths
    size_t len1 = input1->indexes ? OCDataGetLength(input1->indexes) : 0;
    size_t len2 = input2->indexes ? OCDataGetLength(input2->indexes) : 0;
    if (len1 != len2) return false;

    // If both are NULL or both lengths are zero, they’re equal
    if (len1 == 0 && len2 == 0) return true;

    // Byte-for-byte comparison
    const void *bytes1 = OCDataGetBytePtr(input1->indexes);
    const void *bytes2 = OCDataGetBytePtr(input2->indexes);
    return (memcmp(bytes1, bytes2, len1) == 0);
}


//-----------------------------------------------------------------------------
// Finalizer Callback
//-----------------------------------------------------------------------------
void __OCIndexSetFinalize(const void *theType)
{
    OCMutableIndexSetRef obj = (OCMutableIndexSetRef)theType;
    if (obj->indexes) {
        OCRelease(obj->indexes);
        obj->indexes = NULL;
    }
}


//-----------------------------------------------------------------------------
// Copy-Formatting Callback (unused here)
//-----------------------------------------------------------------------------
OCStringRef __OCIndexSetCopyFormattingDesc(const void *theType)
{
    (void)theType;
    return NULL;
}


//-----------------------------------------------------------------------------
// Allocate a new object via OCTypeAlloc
//-----------------------------------------------------------------------------
OCMutableIndexSetRef OCIndexSetAllocate(void)
{
    return (OCMutableIndexSetRef)
        OCTypeAlloc(
            struct __OCIndexSet,
            OCIndexSetGetTypeID(),
            __OCIndexSetFinalize,
            __OCIndexSetEqual,
            __OCIndexSetCopyFormattingDesc
        );
}


//-----------------------------------------------------------------------------
// TypeID Registration
//-----------------------------------------------------------------------------
OCTypeID OCIndexSetGetTypeID(void)
{
    if (kOCIndexSetID == _kOCNotATypeID) {
        kOCIndexSetID = OCRegisterType("OCIndexSet");
    }
    return kOCIndexSetID;
}


//-----------------------------------------------------------------------------
// Public Constructors
//-----------------------------------------------------------------------------
OCIndexSetRef OCIndexSetCreate(void)
{
    OCMutableIndexSetRef newSet = (OCMutableIndexSetRef) OCIndexSetAllocate();
    newSet->indexes = NULL;
    return (OCIndexSetRef)newSet;
}

OCMutableIndexSetRef OCIndexSetCreateMutable(void)
{
    OCMutableIndexSetRef newSet = OCIndexSetAllocate();
    newSet->indexes = NULL;
    return newSet;
}

OCIndexSetRef OCIndexSetCreateCopy(OCIndexSetRef theIndexSet)
{
    if (!theIndexSet) return NULL;
    // Internally, just copy the OCDataRef buffer if it exists
    OCDataRef oldData = theIndexSet->indexes;
    return (oldData
            ? OCIndexSetCreateWithData(oldData)
            : OCIndexSetCreate());
}

OCMutableIndexSetRef OCIndexSetCreateMutableCopy(OCIndexSetRef theIndexSet)
{
    if (!theIndexSet) return NULL;
    OCDataRef oldData = theIndexSet->indexes;
    return (oldData
            ? (OCMutableIndexSetRef) OCIndexSetCreateWithData(oldData)
            : OCIndexSetCreateMutable());
}

OCIndexSetRef OCIndexSetCreateWithIndex(long index)
{
    OCMutableIndexSetRef newSet = OCIndexSetAllocate();
    // Create a single-element data buffer
    newSet->indexes = OCDataCreate((const unsigned char *)&index,
                                   sizeof(long));
    return (OCIndexSetRef)newSet;
}

OCIndexSetRef OCIndexSetCreateWithIndexesInRange(long location, long length)
{
    OCMutableIndexSetRef newSet = OCIndexSetAllocate();
    if (length <= 0) {
        newSet->indexes = NULL;
        return (OCIndexSetRef)newSet;
    }

    // Build a temporary C array of longs: { location, location+1, …, location+length−1 }
    long *temp = (long *)malloc(sizeof(long) * length);
    for (long i = 0; i < length; i++) {
        temp[i] = location + i;
    }

    // Create the OCDataRef from that buffer
    newSet->indexes = OCDataCreate((const unsigned char *)temp,
                                   sizeof(long) * length);
    free(temp);
    return (OCIndexSetRef)newSet;
}


//-----------------------------------------------------------------------------
// Accessors
//-----------------------------------------------------------------------------
OCDataRef OCIndexSetGetIndexes(OCIndexSetRef theIndexSet)
{
    if (!theIndexSet) return NULL;
    return theIndexSet->indexes;
}

long *OCIndexSetGetBytePtr(OCIndexSetRef theIndexSet)
{
    if (!theIndexSet || !theIndexSet->indexes) return NULL;
    return (long *)OCDataGetBytePtr(theIndexSet->indexes);
}

long OCIndexSetGetCount(OCIndexSetRef theIndexSet)
{
    if (!theIndexSet || !theIndexSet->indexes) return 0;
    return (long)(OCDataGetLength(theIndexSet->indexes) / sizeof(long));
}

long OCIndexSetFirstIndex(OCIndexSetRef theIndexSet)
{
    if (!theIndexSet || !theIndexSet->indexes) return kOCNotFound;
    long *buf = (long *)OCDataGetBytePtr(theIndexSet->indexes);
    return buf ? buf[0] : kOCNotFound;
}

long OCIndexSetLastIndex(OCIndexSetRef theIndexSet)
{
    if (!theIndexSet || !theIndexSet->indexes) return kOCNotFound;
    long count = OCIndexSetGetCount(theIndexSet);
    if (count == 0) return kOCNotFound;
    long *buf = (long *)OCDataGetBytePtr(theIndexSet->indexes);
    return buf[count - 1];
}

long OCIndexSetIndexLessThanIndex(OCIndexSetRef theIndexSet, long index)
{
    if (!theIndexSet || !theIndexSet->indexes) return kOCNotFound;
    long count = OCIndexSetGetCount(theIndexSet);
    long *buf = (long *)OCDataGetBytePtr(theIndexSet->indexes);
    for (long i = count - 1; i >= 0; i--) {
        if (buf[i] < index) return buf[i];
    }
    return kOCNotFound;
}

long OCIndexSetIndexGreaterThanIndex(OCIndexSetRef theIndexSet, long index)
{
    if (!theIndexSet || !theIndexSet->indexes) return kOCNotFound;
    long count = OCIndexSetGetCount(theIndexSet);
    long *buf = (long *)OCDataGetBytePtr(theIndexSet->indexes);
    // Since they are sorted, we could scan forward—but we’ll do a simple linear scan:
    for (long i = 0; i < count; i++) {
        if (buf[i] > index) return buf[i];
    }
    return kOCNotFound;
}

bool OCIndexSetContainsIndex(OCIndexSetRef theIndexSet, long index)
{
    if (!theIndexSet || !theIndexSet->indexes) return false;
    long count = OCIndexSetGetCount(theIndexSet);
    long *buf = (long *)OCDataGetBytePtr(theIndexSet->indexes);
    for (long i = 0; i < count; i++) {
        if (buf[i] == index) return true;
    }
    return false;
}


//-----------------------------------------------------------------------------
// Mutator: insert one index into the sorted buffer
//-----------------------------------------------------------------------------
bool OCIndexSetAddIndex(OCMutableIndexSetRef theIndexSet, long index)
{
    if (!theIndexSet) return false;

    // If no data exists yet, simply create a single‐element buffer:
    if (!theIndexSet->indexes) {
        theIndexSet->indexes = OCDataCreate((const unsigned char *)&index,
                                            sizeof(long));
        return true;
    }

    long count = OCIndexSetGetCount(theIndexSet);
    long *buf = (long *)OCDataGetBytePtr(theIndexSet->indexes);

    // Find insertion point (or detect duplicate):
    long i;
    for (i = 0; i < count; i++) {
        if (buf[i] == index) {
            // Already present → do nothing
            return false;
        }
        if (buf[i] > index) {
            break;
        }
    }

    // Allocate a new C array with one extra slot
    long *newBuf = (long *)malloc(sizeof(long) * (count + 1));
    for (long j = 0; j < count + 1; j++) {
        if (j < i) {
            newBuf[j] = buf[j];
        }
        else if (j == i) {
            newBuf[j] = index;
        }
        else {
            newBuf[j] = buf[j - 1];
        }
    }

    // Release old OCDataRef, create a new one, free the temporary
    OCRelease(theIndexSet->indexes);
    theIndexSet->indexes = OCDataCreate((const unsigned char *)newBuf,
                                        sizeof(long) * (count + 1));
    free(newBuf);
    return true;
}


//-----------------------------------------------------------------------------
// Equality (public wrapper)
//-----------------------------------------------------------------------------
bool OCIndexSetEqual(OCIndexSetRef input1, OCIndexSetRef input2)
{
    return __OCIndexSetEqual(input1, input2);
}


//-----------------------------------------------------------------------------
// Convert to OCNumber-backed OCArray
//-----------------------------------------------------------------------------
OCArrayRef OCIndexSetCreateOCNumberArray(OCIndexSetRef theIndexSet)
{
    if (!theIndexSet || !theIndexSet->indexes) {
        // Return empty array
        return OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    }

    long count = OCIndexSetGetCount(theIndexSet);
    OCMutableArrayRef arr = OCArrayCreateMutable(count, &kOCTypeArrayCallBacks);
    long *buf = (long *)OCDataGetBytePtr(theIndexSet->indexes);

    for (long i = 0; i < count; i++) {
        OCNumberRef num = OCNumberCreateWithLong(buf[i]);
        OCArrayAppendValue(arr, num);
        OCRelease(num);
    }
    return arr;
}


//-----------------------------------------------------------------------------
// Plist Serialization
//-----------------------------------------------------------------------------
OCDictionaryRef OCIndexSetCreatePList(OCIndexSetRef theIndexSet)
{
    if (!theIndexSet) return NULL;
    OCDictionaryRef dict = OCDictionaryCreateMutable(0);
    if (theIndexSet->indexes) {
        OCDictionarySetValue(dict, STR("indexes"), theIndexSet->indexes);
    }
    return dict;
}

OCIndexSetRef OCIndexSetCreateWithPList(OCDictionaryRef dictionary)
{
    if (!dictionary) return NULL;
    OCDataRef data = (OCDataRef)OCDictionaryGetValue(dictionary, STR("indexes"));
    if (!data) return OCIndexSetCreate();
    return OCIndexSetCreateWithData(data);
}


OCDataRef OCIndexSetCreateData(OCIndexSetRef theIndexSet)
{
    if (!theIndexSet || !theIndexSet->indexes) return NULL;
    return OCRetain(theIndexSet->indexes);
}

OCIndexSetRef OCIndexSetCreateWithData(OCDataRef data)
{
    if (!data) return NULL;
    // Copy the underlying OCDataRef
    OCMutableIndexSetRef newSet = OCIndexSetAllocate();
    newSet->indexes = (OCMutableDataRef)OCDataCreateCopy(data);
    return (OCIndexSetRef)newSet;
}


//-----------------------------------------------------------------------------
// Debug Print
//-----------------------------------------------------------------------------
void OCIndexSetShow(OCIndexSetRef theIndexSet)
{
    if (!theIndexSet || !theIndexSet->indexes) {
        fprintf(stderr, "( )\n");
        return;
    }
    long count = OCIndexSetGetCount(theIndexSet);
    long *buf = (long *)OCDataGetBytePtr(theIndexSet->indexes);

    fprintf(stderr, "(");
    for (long i = 0; i < count; i++) {
        fprintf(stderr, "%ld", buf[i]);
        if (i < count - 1) {
            fprintf(stderr, ",");
        }
    }
    fprintf(stderr, ")\n");
}

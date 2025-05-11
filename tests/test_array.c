#include "test_array.h"
#include "../src/OCString.h" // For OCStringRef, OCStringCreateWithCString, OCStringEqual, OCRelease
#include "../src/OCArray.h"  // For OCArray specific functions like OCArrayCreateMutable, OCArrayAppendValue etc.

// Adapter for OCArraySortValues on OCStringRefs:
OCComparisonResult OCStringCompareAdapter(const void *a, const void *b, void *ctx) {
    (void)ctx; // Context not used in this adapter
    return OCStringCompare((OCStringRef)a, (OCStringRef)b, 0);
}

// Original arrayTest0 implementation
bool arrayTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCMutableArrayRef arr = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    if (!arr) PRINTERROR; // Check for null after creation

    const char *in[]   = {"e","b","a","c","d"};
    const char *want[] = {"a","b","c","d","e"};
    OCStringRef temp[5];

    for(int i=0;i<5;i++){
        temp[i] = OCStringCreateWithCString(in[i]);
        if (!temp[i]) { // Check for null after creation
            OCRelease(arr);
            for (int j = 0; j < i; ++j) OCRelease(temp[j]);
            PRINTERROR;
        }
        OCArrayAppendValue(arr, temp[i]);
    }

    // verify insertion order
    for(int i=0;i<5;i++){
        OCStringRef got = (OCStringRef)OCArrayGetValueAtIndex(arr,i);
        // No need to create a new OCString for comparison if OCStringEqual can take char*
        // Assuming OCStringEqual takes two OCStringRef, so chk is needed.
        OCStringRef chk = OCStringCreateWithCString(in[i]);
        if (!chk) { /* Handle allocation failure for chk */ PRINTERROR; }
        if (!OCStringEqual(got,chk)) { 
            OCRelease(chk); 
            // Release previously created temp strings and array
            for(int k=0;k<5;k++) OCRelease(temp[k]);
            OCRelease(arr);
            PRINTERROR; 
        }
        OCRelease(chk);
    }

    // sort and verify
    OCArraySortValues(arr, OCRangeMake(0,5), OCStringCompareAdapter, NULL);
    for(int i=0;i<5;i++){
        OCStringRef got = (OCStringRef)OCArrayGetValueAtIndex(arr,i);
        OCStringRef chk = OCStringCreateWithCString(want[i]);
        if (!chk) { /* Handle allocation failure for chk */ PRINTERROR; }
        if (!OCStringEqual(got,chk)) { 
            OCRelease(chk); 
            // Release previously created temp strings and array
            for(int k=0;k<5;k++) OCRelease(temp[k]);
            OCRelease(arr);
            PRINTERROR; 
        }
        OCRelease(chk);
    }

    fprintf(stderr, "%s end...without problems\n", __func__);
    for(int i=0;i<5;i++) OCRelease(temp[i]);
    OCRelease(arr);
    return true;
}

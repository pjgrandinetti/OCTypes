/**
 * OCSet.c
 * 
 * OCSet implementation using OCArray internally.
 * Provides unordered, unique collection of OCTypeRef values.
 */

#include "OCLibrary.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>

static OCTypeID kOCSetID = _kOCNotATypeID;

struct __OCSet {
    OCBase _base;
    OCMutableArrayRef elements;
};

static bool __OCSetEqual(const void *a, const void *b)
{
    OCSetRef set1 = (OCSetRef)a;
    OCSetRef set2 = (OCSetRef)b;
    if (set1 == set2) return true;
    if (!set1 || !set2) return false;

    OCIndex count1 = OCArrayGetCount(set1->elements);
    OCIndex count2 = OCArrayGetCount(set2->elements);
    if (count1 != count2) return false;

    for (OCIndex i = 0; i < count1; ++i) {
        OCTypeRef v = (OCTypeRef)OCArrayGetValueAtIndex(set1->elements, i);
        if (!OCArrayContainsValue(set2->elements, v)) return false;
    }
    return true;
}

static void __OCSetFinalize(const void *obj)
{
    OCSetRef set = (OCSetRef)obj;
    if (set && set->elements) {
        OCRelease(set->elements);
    }
}

static OCStringRef __OCSetCopyFormattingDesc(OCTypeRef cf)
{
    OCSetRef set = (OCSetRef)cf;
    if (!set) return OCStringCreateWithCString("<OCSet: NULL>");

    OCIndex count = OCArrayGetCount(set->elements);
    OCMutableStringRef result = OCStringCreateMutable(0);
    OCStringAppendFormat(result, STR("<OCSet: %u value%s {"), count, count == 1 ? "" : "s");

    for (OCIndex i = 0; i < count; ++i) {
        OCTypeRef val = (OCTypeRef)OCArrayGetValueAtIndex(set->elements, i);
        OCStringRef desc = OCTypeCopyFormattingDesc(val);
        OCStringAppend(result, desc);
        OCRelease(desc);
        if (i + 1 < count)
            OCStringAppendCString(result, ", ");
    }

    OCStringAppendCString(result, "}>");
    return result;
}

static void *__OCSetDeepCopy(const void *obj) {
    OCSetRef src = (OCSetRef)obj;
    if (!src || !src->elements) return NULL;

    OCMutableSetRef copy = OCSetCreateMutable(0);
    if (!copy) return NULL;

    OCIndex count = OCArrayGetCount(src->elements);
    for (OCIndex i = 0; i < count; i++) {
        OCTypeRef original = OCArrayGetValueAtIndex(src->elements, i);
        void *cloned = OCTypeDeepCopy(original);
        if (!cloned) {
            OCRelease(copy);
            return NULL;
        }
        OCSetAddValue(copy, cloned);
        OCRelease(cloned); // AddValue retains it
    }

    return (void *)copy;
}

static void *__OCSetDeepCopyMutable(const void *obj) {
    // OCSet is mutable, so same as deep copy
    return __OCSetDeepCopy(obj);
}

OCTypeID OCSetGetTypeID(void)
{
    if (kOCSetID == _kOCNotATypeID) {
        kOCSetID = OCRegisterType("OCSet");
    }
    return kOCSetID;
}

static struct __OCSet *OCSetAllocate(void)
{
    struct __OCSet *set = OCTypeAlloc(
        struct __OCSet,
        OCSetGetTypeID(),
        __OCSetFinalize,
        __OCSetEqual,
        __OCSetCopyFormattingDesc,
        __OCSetDeepCopy,
        __OCSetDeepCopyMutable
    );

    set->elements = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    return set;
}

OCSetRef OCSetCreate(void)
{
    return (OCSetRef) OCSetAllocate();
}

OCMutableSetRef OCSetCreateMutable(OCIndex capacity)
{
    return (OCMutableSetRef)OCSetAllocate();
}

OCSetRef OCSetCreateCopy(OCSetRef theSet)
{
    if (!theSet) return NULL;
    OCSetRef copy = OCSetCreate();
    OCIndex count = OCArrayGetCount(theSet->elements);
    for (OCIndex i = 0; i < count; i++) {
        OCTypeRef value = OCArrayGetValueAtIndex(theSet->elements, i);
        OCSetAddValue((OCMutableSetRef)copy, value);
    }
    return copy;
}

OCMutableSetRef OCSetCreateMutableCopy(OCSetRef theSet)
{
    return (OCMutableSetRef)OCSetCreateCopy(theSet);
}

OCIndex OCSetGetCount(OCSetRef theSet)
{
    if (!theSet) return 0;
    return OCArrayGetCount(theSet->elements);
}

bool OCSetContainsValue(OCSetRef theSet, OCTypeRef value)
{
    if (!theSet || !value) return false;
    return OCArrayContainsValue(theSet->elements, value);
}

OCArrayRef OCSetCreateValueArray(OCSetRef theSet)
{
    if (!theSet) return NULL;
    return OCArrayCreateCopy(theSet->elements);
}

bool OCSetAddValue(OCMutableSetRef theSet, OCTypeRef value)
{
    if (!theSet || !value) return false;
    if (OCSetContainsValue(theSet, value)) return true;
    OCArrayAppendValue(theSet->elements, value);
    return true;
}

bool OCSetRemoveValue(OCMutableSetRef theSet, OCTypeRef value)
{
    if (!theSet || !value) return false;
    OCIndex idx = OCArrayGetFirstIndexOfValue(theSet->elements, value);
    if (idx == kOCNotFound) return false;
    OCArrayRemoveValueAtIndex(theSet->elements, idx);
    return true;
}

void OCSetRemoveAllValues(OCMutableSetRef theSet)
{
    if (!theSet) return;
    while (OCSetGetCount(theSet) > 0) {
        OCArrayRemoveValueAtIndex(theSet->elements, 0);
    }
}

bool OCSetEqual(OCSetRef a, OCSetRef b)
{
    return __OCSetEqual(a, b);
}

void OCSetShow(OCSetRef theSet)
{
    if (!theSet) return;
    OCStringRef desc = OCTypeCopyFormattingDesc(theSet);
    if (desc) {
        fprintf(stderr, "%s\n", OCStringGetCString(desc));
        OCRelease(desc);
    }
}


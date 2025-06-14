/*
 * OCIndexPairSet.c (cleaned version)
 *
 * Created by PhySy Ltd. Improved by Assistant, 2025
 */

#include "OCLibrary.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static OCTypeID kOCIndexPairSetID = kOCNotATypeID;

struct impl_OCIndexPairSet {
    OCBase base;
    OCDataRef indexPairs;
};

OCTypeID OCIndexPairSetGetTypeID(void) {
    if (kOCIndexPairSetID == kOCNotATypeID)
        kOCIndexPairSetID = OCRegisterType("OCIndexPairSet");
    return kOCIndexPairSetID;
}

static bool impl_OCIndexPairSetEqual(const void *a_, const void *b_) {
    OCIndexPairSetRef a = (OCIndexPairSetRef)a_;
    OCIndexPairSetRef b = (OCIndexPairSetRef)b_;
    return OCTypeEqual(a->indexPairs, b->indexPairs);
}

static void impl_OCIndexPairSetFinalize(const void *obj) {
    OCIndexPairSetRef constSet = (OCIndexPairSetRef)obj;
    OCMutableIndexPairSetRef set = (OCMutableIndexPairSetRef)constSet;

    if (set->indexPairs) {
        OCRelease(set->indexPairs);
        set->indexPairs = NULL; // 
    }
}

static OCStringRef impl_OCIndexPairSetCopyFormattingDesc(OCTypeRef cf) {
    if (!cf) return NULL;
    return OCStringCreateWithCString("<OCIndexPairSet>");
}

static OCMutableIndexPairSetRef OCIndexPairSetAllocate(void);

static void *impl_OCIndexPairSetDeepCopy(const void *obj) {
    OCIndexPairSetRef src = (OCIndexPairSetRef)obj;
    if (!src || !src->indexPairs) return NULL;

    // Deep copy underlying OCDataRef
    OCDataRef copiedData = OCDataCreateCopy(src->indexPairs);
    if (!copiedData) return NULL;

    OCMutableIndexPairSetRef copy = OCIndexPairSetAllocate();
    if (!copy) {
        OCRelease(copiedData);
        return NULL;
    }

    copy->indexPairs = copiedData;
    return (void *) copy;
}

static void *impl_OCIndexPairSetDeepCopyMutable(const void *obj) {
    OCIndexPairSetRef src = (OCIndexPairSetRef)obj;
    if (!src || !src->indexPairs) return NULL;

    OCMutableDataRef copied = OCDataCreateMutableCopy(OCDataGetLength(src->indexPairs), src->indexPairs);
    if (!copied) return NULL;

    OCMutableIndexPairSetRef copy = OCIndexPairSetAllocate();
    if (!copy) {
        OCRelease(copied);
        return NULL;
    }

    copy->indexPairs = copied;
    return copy;
}

static OCMutableIndexPairSetRef OCIndexPairSetAllocate(void) {
    return (OCMutableIndexPairSetRef)OCTypeAlloc(
        struct impl_OCIndexPairSet,
        OCIndexPairSetGetTypeID(),
        impl_OCIndexPairSetFinalize,
        impl_OCIndexPairSetEqual,
        impl_OCIndexPairSetCopyFormattingDesc,
        impl_OCIndexPairSetDeepCopy,
        impl_OCIndexPairSetDeepCopyMutable
    );
}



OCIndexPairSetRef OCIndexPairSetCreate(void) {
    OCMutableIndexPairSetRef s = OCIndexPairSetAllocate();
    s->indexPairs = NULL;
    return s;
}

OCMutableIndexPairSetRef OCIndexPairSetCreateMutable(void) {
    OCMutableIndexPairSetRef s = OCIndexPairSetAllocate();
    if (!s) return NULL;
    s->indexPairs = OCDataCreateMutable(0);
    return s;
}

OCIndexPairSetRef OCIndexPairSetCreateWithIndexPairArray(OCIndexPair *array, int count) {
    // Defensive check: invalid input if count > 0 but array is NULL
    if (count > 0 && array == NULL) return NULL;

    OCMutableIndexPairSetRef s = OCIndexPairSetAllocate();
    if (!s) return NULL;

    if (count == 0) {
        // Allocate empty OCData (not NULL, to allow valid GetCount and safety downstream)
        s->indexPairs = OCDataCreate(NULL, 0);
    } else {
        s->indexPairs = OCDataCreate((const uint8_t *)array, count * sizeof(OCIndexPair));
    }

    if (!s->indexPairs) {
        OCRelease(s);
        return NULL;
    }

    return s;
}

OCIndexPairSetRef OCIndexPairSetCreateWithIndexPair(OCIndex index, OCIndex value) {
    OCIndexPair pair = {index, value};
    return OCIndexPairSetCreateWithIndexPairArray(&pair, 1);
}

OCIndexPairSetRef OCIndexPairSetCreateWithTwoIndexPairs(OCIndex i1, OCIndex v1, OCIndex i2, OCIndex v2) {
    OCIndexPair pairs[2] = { {i1, v1}, {i2, v2} };
    return OCIndexPairSetCreateWithIndexPairArray(pairs, 2);
}

OCIndex OCIndexPairSetGetCount(OCIndexPairSetRef s) {
    return s && s->indexPairs ? OCDataGetLength(s->indexPairs) / sizeof(OCIndexPair) : 0;
}

OCIndexPair *OCIndexPairSetGetBytesPtr(OCIndexPairSetRef s) {
    return s && s->indexPairs ? (OCIndexPair *)OCDataGetBytesPtr(s->indexPairs) : NULL;
}

OCIndex OCIndexPairSetValueForIndex(OCIndexPairSetRef s, OCIndex index) {
    OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(s);
    OCIndex count = OCIndexPairSetGetCount(s);
    for (OCIndex i = 0; i < count; i++)
        if (pairs[i].index == index)
            return pairs[i].value;
    return kOCNotFound;
}

bool OCIndexPairSetAddIndexPair(OCMutableIndexPairSetRef s, OCIndex index, OCIndex value) {
    if (!s) return false;
    OCIndexPair newPair = {index, value};
    OCIndex count = OCIndexPairSetGetCount(s);
    OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(s);

    for (OCIndex i = 0; i < count; i++) {
        if (pairs[i].index == index) return false;
    }

    OCIndexPair *newArray = malloc((count + 1) * sizeof(OCIndexPair));
    if (!newArray) return false;

    int inserted = 0;
    for (OCIndex i = 0, j = 0; i <= count; i++) {
        if (!inserted && (i == count || pairs[i].index > index)) {
            newArray[j++] = newPair;
            inserted = 1;
        }
        if (i < count) newArray[j++] = pairs[i];
    }

    OCDataRef newData = OCDataCreate((const uint8_t *)newArray, (count + 1) * sizeof(OCIndexPair));
    free(newArray);
    if (!newData) return false;

    OCRelease(s->indexPairs);
    s->indexPairs = newData;
    return true;
}

bool OCIndexPairSetContainsIndex(OCIndexPairSetRef s, OCIndex index) {
    OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(s);
    OCIndex count = OCIndexPairSetGetCount(s);
    for (OCIndex i = 0; i < count; i++)
        if (pairs[i].index == index)
            return true;
    return false;
}

void OCIndexPairSetShow(OCIndexPairSetRef s) {
    if (!s) return;
    OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(s);
    OCIndex count = OCIndexPairSetGetCount(s);
    fprintf(stderr, "(");
    for (OCIndex i = 0; i < count; i++) {
        fprintf(stderr, "(%ld,%ld)%s", pairs[i].index, pairs[i].value, (i < count - 1 ? "," : ""));
    }
    fprintf(stderr, ")\n");
}

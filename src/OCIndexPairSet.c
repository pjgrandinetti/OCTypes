/*
 * OCIndexPairSet.c (cleaned version)
 *
 * Created by PhySy Ltd. Improved by Assistant, 2025
 */

#include "OCLibrary.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static OCTypeID kOCIndexPairSetID = _kOCNotATypeID;

struct __OCIndexPairSet {
    OCBase _base;
    OCDataRef indexPairs;
};

OCTypeID OCIndexPairSetGetTypeID(void) {
    if (kOCIndexPairSetID == _kOCNotATypeID)
        kOCIndexPairSetID = OCRegisterType("OCIndexPairSet");
    return kOCIndexPairSetID;
}

static bool __OCIndexPairSetEqual(const void *a_, const void *b_) {
    OCIndexPairSetRef a = (OCIndexPairSetRef)a_;
    OCIndexPairSetRef b = (OCIndexPairSetRef)b_;
    return OCTypeEqual(a->indexPairs, b->indexPairs);
}

static void __OCIndexPairSetFinalize(const void *obj) {
    OCIndexPairSetRef set = (OCIndexPairSetRef)obj;
    if (set->indexPairs) OCRelease(set->indexPairs);
}

static OCStringRef __OCIndexPairSetCopyFormattingDesc(OCTypeRef cf) {
    if (!cf) return NULL;
    return OCStringCreateWithCString("<OCIndexPairSet>");
}

static OCMutableIndexPairSetRef OCIndexPairSetAllocate(void) {
    return (OCMutableIndexPairSetRef)OCTypeAlloc(
        struct __OCIndexPairSet,
        OCIndexPairSetGetTypeID(),
        __OCIndexPairSetFinalize,
        __OCIndexPairSetEqual,
        __OCIndexPairSetCopyFormattingDesc);
}

OCIndexPairSetRef OCIndexPairSetCreate(void) {
    OCMutableIndexPairSetRef s = OCIndexPairSetAllocate();
    s->indexPairs = NULL;
    return s;
}

OCMutableIndexPairSetRef OCIndexPairSetCreateMutable(void) {
    OCMutableIndexPairSetRef s = OCIndexPairSetAllocate();
    s->indexPairs = NULL;
    return s;
}

OCIndexPairSetRef OCIndexPairSetCreateWithIndexPairArray(OCIndexPair *array, int count) {
    OCMutableIndexPairSetRef s = OCIndexPairSetAllocate();
    s->indexPairs = OCDataCreate((const uint8_t *)array, count * sizeof(OCIndexPair));
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

OCIndexPair *OCIndexPairSetGetBytePtr(OCIndexPairSetRef s) {
    return s && s->indexPairs ? (OCIndexPair *)OCDataGetBytePtr(s->indexPairs) : NULL;
}

OCIndex OCIndexPairSetValueForIndex(OCIndexPairSetRef s, OCIndex index) {
    OCIndexPair *pairs = OCIndexPairSetGetBytePtr(s);
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
    OCIndexPair *pairs = OCIndexPairSetGetBytePtr(s);

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
    OCIndexPair *pairs = OCIndexPairSetGetBytePtr(s);
    OCIndex count = OCIndexPairSetGetCount(s);
    for (OCIndex i = 0; i < count; i++)
        if (pairs[i].index == index)
            return true;
    return false;
}

void OCIndexPairSetShow(OCIndexPairSetRef s) {
    if (!s) return;
    OCIndexPair *pairs = OCIndexPairSetGetBytePtr(s);
    OCIndex count = OCIndexPairSetGetCount(s);
    fprintf(stderr, "(");
    for (OCIndex i = 0; i < count; i++) {
        fprintf(stderr, "(%ld,%ld)%s", pairs[i].index, pairs[i].value, (i < count - 1 ? "," : ""));
    }
    fprintf(stderr, ")\n");
}

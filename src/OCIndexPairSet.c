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
static cJSON *
impl_OCIndexPairSetCopyJSON(const void *obj)
{
    return OCIndexPairSetCreateJSON((OCIndexPairSetRef)obj);
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
        impl_OCIndexPairSetCopyJSON,
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

OCIndexPairSetRef OCIndexPairSetCreateCopy(OCIndexPairSetRef source) {
    if (!source) return NULL;
    return (OCIndexPairSetRef)impl_OCIndexPairSetDeepCopy(source);
}

OCMutableIndexPairSetRef OCIndexPairSetCreateMutableCopy(OCIndexPairSetRef source) {
    if (!source) return NULL;
    return (OCMutableIndexPairSetRef)impl_OCIndexPairSetDeepCopyMutable(source);
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

cJSON *OCIndexPairSetCreateJSON(OCIndexPairSetRef set) {
    if (!set) return cJSON_CreateNull();

    OCIndex count = OCIndexPairSetGetCount(set);
    OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(set);
    if (!pairs) return cJSON_CreateNull();

    cJSON *arr = cJSON_CreateArray();
    for (OCIndex i = 0; i < count; i++) {
        cJSON *pair = cJSON_CreateArray();
        cJSON_AddItemToArray(pair, cJSON_CreateNumber((double)pairs[i].index));
        cJSON_AddItemToArray(pair, cJSON_CreateNumber((double)pairs[i].value));
        cJSON_AddItemToArray(arr, pair);
    }
    return arr;
}

OCIndexPairSetRef OCIndexPairSetCreateFromJSON(cJSON *json) {
    if (!json || !cJSON_IsArray(json)) return NULL;

    OCIndex count = cJSON_GetArraySize(json);
    OCIndexPair *pairs = malloc(count * sizeof(OCIndexPair));
    if (!pairs) return NULL;

    for (OCIndex i = 0; i < count; i++) {
        cJSON *pair = cJSON_GetArrayItem(json, (int) i);
        if (!cJSON_IsArray(pair) || cJSON_GetArraySize(pair) != 2) {
            free(pairs);
            return NULL;
        }

        cJSON *indexNode = cJSON_GetArrayItem(pair, 0);
        cJSON *valueNode = cJSON_GetArrayItem(pair, 1);
        if (!cJSON_IsNumber(indexNode) || !cJSON_IsNumber(valueNode)) {
            free(pairs);
            return NULL;
        }

        pairs[i].index = (OCIndex)indexNode->valuedouble;
        pairs[i].value = (OCIndex)valueNode->valuedouble;
    }

    OCIndexPairSetRef result = OCIndexPairSetCreateWithIndexPairArray(pairs, (int) count);
    free(pairs);
    return result;
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

OCIndexArrayRef OCIndexPairSetCreateIndexArrayOfValues(OCIndexPairSetRef set) {
    if (!set) return NULL;

    OCIndex count = OCIndexPairSetGetCount(set);
    OCMutableIndexArrayRef result = OCIndexArrayCreateMutable(count);
    if (!result) return NULL;

    OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(set);
    if (!pairs) {
        OCRelease(result);
        return NULL;
    }

    for (OCIndex i = 0; i < count; ++i) {
        OCIndexArrayAppendValue(result, pairs[i].value);
    }

    return (OCIndexArrayRef)result;
}

OCIndexSetRef OCIndexPairSetCreateIndexSetOfIndexes(OCIndexPairSetRef set)
{
    if (!set) return NULL;

    // 1) make an empty, mutable OCIndexSet
    OCMutableIndexSetRef indexes = OCIndexSetCreateMutable();

    // 2) grab the raw OCIndexPair buffer
    const OCIndexPair *pairs = OCIndexPairSetGetBytesPtr(set);
    OCIndex cnt   = OCIndexPairSetGetCount(set);

    // 3) add each .index into our OCIndexSet
    for (OCIndex i = 0; i < cnt; ++i) {
        OCIndexSetAddIndex(indexes, pairs[i].index);
    }

    // 4) return as immutable OCIndexSetRef
    return (OCIndexSetRef)indexes;
}

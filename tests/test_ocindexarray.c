#include "test_utils.h"
#include <stdint.h>
#include <complex.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include "../src/OCNumber.h"
#include "../src/OCType.h"
#include "../src/OCString.h"
#include "../src/OCIndexSet.h"
#include "../src/OCIndexArray.h"

bool OCIndexArrayCreateAndCount_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);

    // Normal creation
    OCIndex values[] = {1, 2, 3, 4};
    OCIndexArrayRef array = OCIndexArrayCreate(values, 4);
    bool success = array && (OCIndexArrayGetCount(array) == 4);
    if (array) OCRelease(array);

    // Invalid call with NULL values but non-zero count
    OCIndexArrayRef bad = OCIndexArrayCreate(NULL, 5);
    success &= (bad == NULL);

    fprintf(stderr, "%s end...without problems\n", __func__);
    return success;
}

bool OCIndexArrayGetValueAtIndex_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);

    OCIndex values[] = {10, 20, 30};
    OCIndexArrayRef array = OCIndexArrayCreate(values, 3);
    if (!array) return false;

    bool success = (OCIndexArrayGetValueAtIndex(array, 0) == 10) &&
                   (OCIndexArrayGetValueAtIndex(array, 1) == 20) &&
                   (OCIndexArrayGetValueAtIndex(array, 2) == 30) &&
                   (OCIndexArrayGetValueAtIndex(array, 99) == kOCNotFound);

    OCRelease(array);
    fprintf(stderr, "%s end...without problems\n", __func__);
    return success;
}

bool OCIndexArraySetValueAtIndex_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);

    OCMutableIndexArrayRef array = OCIndexArrayCreateMutable(3);
    if (!array) return false;

    bool setOK = OCIndexArraySetValueAtIndex(array, 0, 99);
    bool setFail = !OCIndexArraySetValueAtIndex(array, 5, 42); // Invalid index
    bool success = setOK && setFail &&
                   (OCIndexArrayGetValueAtIndex(array, 0) == 99);

    OCRelease(array);
    fprintf(stderr, "%s end...without problems\n", __func__);
    return success;
}

bool OCIndexArrayRemoveValueAtIndex_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);

    OCIndex values[] = {5, 10, 15, 20};
    OCIndexArrayRef orig = OCIndexArrayCreate(values, 4);
    if (!orig) return false;

    OCMutableIndexArrayRef array = OCIndexArrayCreateMutableCopy(orig);
    OCRelease(orig);
    if (!array) return false;

    bool removeOK = OCIndexArrayRemoveValueAtIndex(array, 1);
    bool removeFail = !OCIndexArrayRemoveValueAtIndex(array, 10);
    bool success = removeOK && removeFail &&
                   (OCIndexArrayGetCount(array) == 3) &&
                   (OCIndexArrayGetValueAtIndex(array, 1) == 15);

    OCRelease(array);
    fprintf(stderr, "%s end...without problems\n", __func__);
    return success;
}

bool OCIndexArrayRemoveValuesAtIndexes_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);


    OCIndex values[] = {10, 20, 30, 40, 50};
    OCIndexArrayRef orig = OCIndexArrayCreate(values, 5);
    if (!orig) {
        fprintf(stderr, "Failed to create original array.\n");
        return false;
    }

    OCMutableIndexArrayRef array = OCIndexArrayCreateMutableCopy(orig);
    OCRelease(orig);
    if (!array) {
        fprintf(stderr, "Failed to create mutable copy.\n");
        return false;
    }

    OCMutableIndexSetRef indexSet = OCIndexSetCreateMutable();
    if (!indexSet) {
        OCRelease(array);
        fprintf(stderr, "Failed to create index set.\n");
        return false;
    }

    OCIndexSetAddIndex(indexSet, 1); // 20
    OCIndexSetAddIndex(indexSet, 3); // 40

    OCIndexArrayRemoveValuesAtIndexes(array, indexSet);

    bool success = (OCIndexArrayGetCount(array) == 3) &&
                   (OCIndexArrayGetValueAtIndex(array, 0) == 10) &&
                   (OCIndexArrayGetValueAtIndex(array, 1) == 30) &&
                   (OCIndexArrayGetValueAtIndex(array, 2) == 50);

    OCRelease(indexSet);
    OCRelease(array);
    fprintf(stderr, "%s end...%s\n", __func__, success ? "without problems" : "with errors");
    return success;
}

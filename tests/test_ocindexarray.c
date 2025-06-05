#include "test_utils.h"
#include <stdint.h>
#include <complex.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include "../src/OCNumber.h"
#include "../src/OCType.h"
#include "../src/OCString.h"

bool test_OCIndexArrayCreateAndCount(void) {
    OCIndex values[] = {1, 2, 3, 4};
    OCIndexArrayRef array = OCIndexArrayCreate(values, 4);
    if (!array) return false;
    bool success = OCIndexArrayGetCount(array) == 4;
    OCRelease(array);
    return success;
}

bool test_OCIndexArrayGetValueAtIndex(void) {
    OCIndex values[] = {10, 20, 30};
    OCIndexArrayRef array = OCIndexArrayCreate(values, 3);
    if (!array) return false;
    bool success = OCIndexArrayGetValueAtIndex(array, 0) == 10 &&
                   OCIndexArrayGetValueAtIndex(array, 1) == 20 &&
                   OCIndexArrayGetValueAtIndex(array, 2) == 30;
    OCRelease(array);
    return success;
}

bool test_OCIndexArraySetValueAtIndex(void) {
    OCMutableIndexArrayRef array = OCIndexArrayCreateMutable(3);
    if (!array) return false;
    OCIndexArraySetValueAtIndex(array, 0, 99);
    bool success = OCIndexArrayGetValueAtIndex(array, 0) == 99;
    OCRelease(array);
    return success;
}

bool test_OCIndexArrayRemoveValueAtIndex(void) {
    OCIndex values[] = {5, 10, 15, 20};
    OCIndexArrayRef orig = OCIndexArrayCreate(values, 4);
    if (!orig) return false;
    OCMutableIndexArrayRef array = OCIndexArrayCreateMutableCopy(orig);
    OCRelease(orig);
    if (!array) return false;
    OCIndexArrayRemoveValueAtIndex(array, 1);
    bool success = OCIndexArrayGetCount(array) == 3 &&
                   OCIndexArrayGetValueAtIndex(array, 1) == 15;
    OCRelease(array);
    return success;
}

bool test_OCIndexArrayRemoveValuesAtIndexes(void) {
    OCIndex values[] = {10, 20, 30, 40, 50};
    OCIndexArrayRef orig = OCIndexArrayCreate(values, 5);
    if (!orig) return false;
    OCMutableIndexArrayRef array = OCIndexArrayCreateMutableCopy(orig);
    OCRelease(orig);
    if (!array) return false;
    OCMutableIndexSetRef indexSet = OCIndexSetCreateMutable();
    if (!indexSet) {
        OCRelease(array);
        return false;
    }
    OCIndexSetAddIndex(indexSet, 1); // 20
    OCIndexSetAddIndex(indexSet, 3); // 40
    OCIndexArrayRemoveValuesAtIndexes(array, indexSet);

    bool success = OCIndexArrayGetCount(array) == 3 &&
                   OCIndexArrayGetValueAtIndex(array, 0) == 10 &&
                   OCIndexArrayGetValueAtIndex(array, 1) == 30 &&
                   OCIndexArrayGetValueAtIndex(array, 2) == 50;

    OCRelease(indexSet);
    OCRelease(array);
    return success;
}

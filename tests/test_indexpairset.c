#include <stdio.h>
#include "../src/OCIndexPairSet.h"
#include "../src/OCType.h"
#include "test_utils.h"
bool OCIndexPairSetCreation_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = true;
    OCIndexPairSetRef empty = OCIndexPairSetCreate();
    success &= empty && OCIndexPairSetGetCount(empty) == 0;
    if (empty) OCRelease(empty);
    OCIndexPair pairs[2] = {{1, 100}, {2, 200}};
    OCIndexPairSetRef set = OCIndexPairSetCreateWithIndexPairArray(pairs, 2);
    success &= set && OCIndexPairSetGetCount(set) == 2;
    success &= OCIndexPairSetValueForIndex(set, 1) == 100;
    success &= OCIndexPairSetValueForIndex(set, 2) == 200;
    success &= OCIndexPairSetValueForIndex(set, 3) == kOCNotFound;
    if (set) OCRelease(set);
    OCIndexPairSetRef one = OCIndexPairSetCreateWithIndexPair(5, 50);
    success &= one && OCIndexPairSetGetCount(one) == 1;
    success &= OCIndexPairSetValueForIndex(one, 5) == 50;
    if (one) OCRelease(one);
    // Test with NULL input and count = 0
    OCIndexPairSetRef nullSet = OCIndexPairSetCreateWithIndexPairArray(NULL, 0);
    success &= nullSet && OCIndexPairSetGetCount(nullSet) == 0;
    if (nullSet) OCRelease(nullSet);
    fprintf(stderr, "%s end...%s\n", __func__, success ? "without problems" : "with errors ***");
    return success;
}
bool OCIndexPairSetAddAndContains_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCMutableIndexPairSetRef set = OCIndexPairSetCreateMutable();
    bool success = set != NULL;
    if (set) {
        success &= OCIndexPairSetAddIndexPair(set, 10, 100);
        success &= OCIndexPairSetAddIndexPair(set, 20, 200);
        success &= !OCIndexPairSetAddIndexPair(set, 10, 999);  // Duplicate
        success &= OCIndexPairSetContainsIndex(set, 10);
        success &= OCIndexPairSetContainsIndex(set, 20);
        success &= !OCIndexPairSetContainsIndex(set, 30);
        success &= OCIndexPairSetValueForIndex(set, 10) == 100;
        success &= OCIndexPairSetValueForIndex(set, 20) == 200;
        OCRelease(set);
    }
    fprintf(stderr, "%s end...%s\n", __func__, success ? "without problems" : "with errors ***");
    return success;
}
bool OCIndexPairSetValueLookup_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCMutableIndexPairSetRef set = OCIndexPairSetCreateMutable();
    bool success = set != NULL;
    if (set) {
        OCIndexPairSetAddIndexPair(set, 5, 50);
        OCIndexPairSetAddIndexPair(set, 10, 100);
        success &= (OCIndexPairSetValueForIndex(set, 5) == 50);
        success &= (OCIndexPairSetValueForIndex(set, 10) == 100);
        success &= (OCIndexPairSetValueForIndex(set, 999) == kOCNotFound);
        OCRelease(set);
    }
    fprintf(stderr, "%s end...%s\n", __func__, success ? "without problems" : "with errors ***");
    return success;
}
bool OCIndexPairSetEquality_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCIndexPairSetRef a = OCIndexPairSetCreateWithIndexPair(1, 100);
    OCIndexPairSetRef b = OCIndexPairSetCreateWithIndexPair(1, 100);
    OCIndexPairSetRef c = OCIndexPairSetCreateWithIndexPair(1, 999);
    bool success = a && b && c &&
                   OCTypeEqual(a, b) &&
                   !OCTypeEqual(a, c);
    if (a) OCRelease(a);
    if (b) OCRelease(b);
    if (c) OCRelease(c);
    fprintf(stderr, "%s end...%s\n", __func__, success ? "without problems" : "with errors ***");
    return success;
}
bool OCIndexPairSetShow_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCMutableIndexPairSetRef set = OCIndexPairSetCreateMutable();
    bool success = set != NULL;
    if (set) {
        OCIndexPairSetAddIndexPair(set, 1, 10);
        OCIndexPairSetAddIndexPair(set, 2, 20);
        OCIndexPairSetAddIndexPair(set, 3, 30);
        fprintf(stderr, "Set contents: ");
        OCIndexPairSetShow(set);
        OCRelease(set);
    }
    fprintf(stderr, "%s end...%s\n", __func__, success ? "without problems" : "with errors ***");
    return success;
}
bool OCIndexPairSetDeepCopy_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = true;
    OCMutableIndexPairSetRef original = OCIndexPairSetCreateMutable();
    OCIndexPairSetAddIndexPair(original, 5, 50);
    OCIndexPairSetAddIndexPair(original, 10, 100);
    OCIndexPairSetRef copy = OCTypeDeepCopy(original);
    success &= copy != NULL;
    success &= OCTypeEqual(original, copy);
    // Shallow copy check — the internal pointers should not be identical
    success &= (original != copy);
    // Mutate the copy to ensure original is unaffected
    OCIndexPairSetAddIndexPair((OCMutableIndexPairSetRef)copy, 20, 200);
    success &= (OCIndexPairSetGetCount(copy) == 3);
    success &= (OCIndexPairSetGetCount(original) == 2);
    OCRelease(original);
    OCRelease(copy);
    fprintf(stderr, "%s end...%s\n", __func__, success ? "without problems" : "with errors ***");
    return success;
}

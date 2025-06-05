
#include "test_utils.h"
#include "../src/OCType.h"
#include "../src/OCIndexSet.h"
#include "../src/OCNumber.h"
#include <stdio.h>

bool OCIndexSetCreateAndAccess_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = true;

    OCIndexSetRef s = OCIndexSetCreate();
    success &= (s && OCIndexSetGetCount(s) == 0);

    OCIndexSetRef single = OCIndexSetCreateWithIndex(42);
    success &= (single && OCIndexSetGetCount(single) == 1 && OCIndexSetFirstIndex(single) == 42);

    OCIndexSetRef range = OCIndexSetCreateWithIndexesInRange(10, 3);
    success &= (range && OCIndexSetGetCount(range) == 3);

    OCRelease(s);
    OCRelease(single);
    OCRelease(range);

    fprintf(stderr, "%s end...%s\n", __func__, success ? "without problems" : "with errors");
    return success;
}

bool OCIndexSetAddAndContains_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = true;

    OCMutableIndexSetRef set = OCIndexSetCreateMutable();
    success &= (set != NULL);

    success &= OCIndexSetAddIndex(set, 10);
    success &= OCIndexSetAddIndex(set, 20);
    success &= !OCIndexSetAddIndex(set, 10); // Duplicate

    success &= OCIndexSetContainsIndex(set, 10);
    success &= OCIndexSetContainsIndex(set, 20);
    success &= !OCIndexSetContainsIndex(set, 99);

    OCRelease(set);
    fprintf(stderr, "%s end...%s\n", __func__, success ? "without problems" : "with errors");
    return success;
}

bool OCIndexSetRangeAndBounds_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = true;

    OCMutableIndexSetRef set = OCIndexSetCreateMutable();
    OCIndexSetAddIndex(set, 5);
    OCIndexSetAddIndex(set, 10);
    OCIndexSetAddIndex(set, 20);
    OCIndexSetAddIndex(set, 30);

    success &= (OCIndexSetFirstIndex(set) == 5);
    success &= (OCIndexSetLastIndex(set) == 30);
    success &= (OCIndexSetIndexLessThanIndex(set, 25) == 20);
    success &= (OCIndexSetIndexGreaterThanIndex(set, 25) == 30);

    OCRelease(set);
    fprintf(stderr, "%s end...%s\n", __func__, success ? "without problems" : "with errors");
    return success;
}

bool OCIndexSetSerialization_test(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = true;

    OCMutableIndexSetRef set = OCIndexSetCreateMutable();
    OCIndexSetAddIndex(set, 100);
    OCIndexSetAddIndex(set, 200);

    OCDictionaryRef dict = OCIndexSetCreatePList(set);
    success &= (dict != NULL);

    OCIndexSetRef restored = OCIndexSetCreateWithPList(dict);
    success &= (restored && OCIndexSetEqual(set, restored));

    OCRelease(set);
    OCRelease(dict);
    OCRelease(restored);

    fprintf(stderr, "%s end...%s\n", __func__, success ? "without problems" : "with errors");
    return success;
}
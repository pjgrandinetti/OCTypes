// tests/test_dictionary.c

#include "test_utils.h"
#include <stdio.h>
#include <stdint.h>
#include <complex.h>       // for float complex / double complex
#include "../src/OCLibrary.h"
#include "../src/OCType.h"             // for OCTypeEqual
#include "../src/OCString.h"
#include "../src/OCNumber.h"
#include "../src/OCAutoreleasePool.h"
#include "../src/OCArray.h"
#include "../src/OCDictionary.h"

// pull in the getter for numbers (was missing from OCNumber.h)
extern void OCNumberGetValue(OCNumberRef number, OCNumberType type, void *outValue);

bool dictionaryTest0(void) {
    OCAutoreleasePoolRef pool = OCAutoreleasePoolCreate();

    // Test 1: empty mutable dict
    OCMutableDictionaryRef dict1_mut = OCDictionaryCreateMutable(0);
    ASSERT_NOT_NULL(dict1_mut, "Test 1.1: dict1_mut should not be NULL");
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict1_mut), 0, "Test 1.2: dict1_mut should be empty");
    OCRelease(dict1_mut);

    // Test 2: add two key/value pairs
    OCStringRef key1 = OCStringCreateWithCString("key1");
    OCNumberRef val1 = OCNumberCreateWithSInt32(100);
    OCStringRef key2 = OCStringCreateWithCString("key2");
    OCNumberRef val2 = OCNumberCreateWithSInt32(200);

    OCMutableDictionaryRef dict2 = OCDictionaryCreateMutable(2);
    ASSERT_NOT_NULL(dict2, "Test 2.1: dict2 should not be NULL");
    OCDictionaryAddValue(dict2, key1, val1);
    OCDictionaryAddValue(dict2, key2, val2);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict2), 2, "Test 2.2: dict2 should have 2 items");
    ASSERT_TRUE(OCTypeEqual(OCDictionaryGetValue((OCDictionaryRef)dict2, key1), val1), "Test 2.3: value for key1");
    ASSERT_TRUE(OCTypeEqual(OCDictionaryGetValue((OCDictionaryRef)dict2, key2), val2), "Test 2.4: value for key2");

    // Test 3: remove and contains
    OCDictionaryRemoveValue(dict2, key1);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict2), 1, "Test 3.1: dict2 should have 1 item after removal");
    ASSERT_NULL(OCDictionaryGetValue((OCDictionaryRef)dict2, key1), "Test 3.2: key1 no longer present");
    ASSERT_TRUE(OCDictionaryContainsKey((OCDictionaryRef)dict2, key2), "Test 3.3: key2 still present");

    // Test 4: retrieve & unwrap number
    OCNumberRef got = (OCNumberRef)OCDictionaryGetValue((OCDictionaryRef)dict2, key2);
    ASSERT_NOT_NULL(got, "Test 4.1: retrieved val2 not NULL");
    ASSERT_TRUE(OCTypeEqual(got, val2),   "Test 4.2: retrieved val2 equals original");
    int out;
    OCNumberGetValue(got, kOCNumberSInt32Type, &out);
    ASSERT_EQUAL(out, 200, "Test 4.3: integer unwrapped correctly");

    // Test 5: copy dictionary stays independent
    OCDictionaryAddValue(dict2, key1, val1);
    OCDictionaryRef copy = OCDictionaryCreateCopy((OCDictionaryRef)dict2);
    ASSERT_NOT_NULL(copy, "Test 5.1: copy not NULL");
    ASSERT_EQUAL(OCDictionaryGetCount(copy), OCDictionaryGetCount((OCDictionaryRef)dict2), "Test 5.2: counts match");
    // mutate original
    OCNumberRef z = OCNumberCreateWithSInt32(300);
    OCStringRef key3 = OCStringCreateWithCString("key3"); // Store the key
    OCDictionaryAddValue(dict2, key3, z);
    ASSERT_EQUAL(OCDictionaryGetCount((OCDictionaryRef)dict2), 3, "Test 5.3: original now 3");
    ASSERT_EQUAL(OCDictionaryGetCount(copy), 2, "Test 5.4: copy still 2");
    OCRelease(z);
    OCRelease(copy);

    // cleanup
    OCRelease(key1);
    OCRelease(key2);
    OCRelease(key3); // Release the stored key3
    OCRelease(val1);
    OCRelease(val2);
    OCRelease(dict2);
    OCRelease(pool);

    printf("dictionaryTest0 completed.\n");
    return true;
}

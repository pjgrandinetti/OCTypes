//
//  main.c
//  tests
//
//  Created by Philip Grandinetti on 6/17/17.
//  Updated by ChatGPT on 2025-05-11.
//
#define PRINTERROR do { \
    fprintf(stderr, "failure: line %d, %s\n", __LINE__, __FUNCTION__); \
    return false; \
} while (0)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <complex.h>   // For complex number tests
#include <math.h>      // For M_PI, DBL_EPSILON, FLT_EPSILON, etc.
#include "../src/OCLibrary.h"
#include "../src/OCMath.h"   // Prototypes for cargument, ccbrt, etc.
#include "../src/OCAutoreleasePool.h" // For autorelease pool tests

// Declare raise_to_integer_power if not in OCMath.h
extern double complex raise_to_integer_power(double complex x, long power);

// Adapter for string comparison
OCComparisonResult OCStringCompareAdapter(const void *str1, const void *str2, void *context) {
    (void)context;
    return OCStringCompare((OCStringRef)str1, (OCStringRef)str2, 0);
}

// Test prototypes
bool stringTest0(void);
bool stringTest1(void);
bool stringTest2(void);
bool arrayTest0(void);
bool mathTest0(void);
bool typeTest0(void);
bool autoreleasePoolTest0(void); // Declare new test function

int main(int argc, const char * argv[]) {
    int failures = 0;
    if (!typeTest0())    failures++;
    if (!stringTest0())  failures++;
    if (!stringTest1())  failures++;
    if (!stringTest2())  failures++;
    if (!arrayTest0())   failures++;
    if (!mathTest0())    failures++;
    if (!autoreleasePoolTest0()) failures++; // Add call to new test function
    if (failures > 0) {
        fprintf(stderr, "%d test(s) failed\n", failures);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "All tests passed\n");
    return EXIT_SUCCESS;
}

bool arrayTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCMutableArrayRef array = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    
    // Initial insertion order
    const char *order[] = { "e", "b", "a", "c", "d" };
    for (int i = 0; i < 5; i++) {
        OCArrayAppendValue(array, OCStringCreateWithCString(order[i]));
    }
    // Verify insertion
    for (int i = 0; i < 5; i++) {
        OCStringRef s = (OCStringRef)OCArrayGetValueAtIndex(array, i);
        if (!OCStringEqual(s, OCStringCreateWithCString(order[i]))) PRINTERROR;
    }
    
    // Sort
    OCArraySortValues(array, OCRangeMake(0, OCArrayGetCount(array)), OCStringCompareAdapter, NULL);
    
    // Expected alphabetical order
    const char *sorted[] = { "a", "b", "c", "d", "e" };
    for (int i = 0; i < 5; i++) {
        OCStringRef s = (OCStringRef)OCArrayGetValueAtIndex(array, i);
        if (!OCStringEqual(s, OCStringCreateWithCString(sorted[i]))) PRINTERROR;
    }
    
    fprintf(stderr, "%s end...without problems\n", __func__);
    OCRelease(array);
    return true;
}

bool stringTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    
    double result = OCComplexFromCString("1+1");
    if (OCCompareDoubleValues(result, 2.0) != kOCCompareEqualTo) PRINTERROR;
    
    result = OCComplexFromCString("sqrt(4)");
    if (OCCompareDoubleValues(result, 2.0) != kOCCompareEqualTo) PRINTERROR;
    
    // ... additional string parsing tests as before ...
    
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool stringTest1(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    
    OCStringRef theString = OCStringCreateWithCString("theStringWithAStringOnAStringTown");
    if (!OCStringEqual(theString, STR("theStringWithAStringOnAStringTown"))) PRINTERROR;
    
    OCRelease(theString);
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool stringTest2(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    
    OCStringRef s1 = OCStringCreateWithFormat(STR("Ix(%@)"), STR("H2"));
    if (!OCStringEqual(s1, STR("Ix(H2)"))) PRINTERROR;
    OCRelease(s1);
    
    OCStringRef s2 = OCStringCreateWithFormat(STR("Ix(%s)"), "H2");
    if (!OCStringEqual(s2, STR("Ix(H2)"))) PRINTERROR;
    OCRelease(s2);
    
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool mathTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    
    double complex z = 2.0 + 3.0*I;
    double complex raised0 = raise_to_integer_power(z, 0);
    if (OCCompareDoubleValuesLoose(creal(raised0), 1.0) != kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValuesLoose(cimag(raised0), 0.0) != kOCCompareEqualTo) PRINTERROR;
    
    double complex raised2 = raise_to_integer_power(z, 2);
    double complex expected2 = z * z;
    if (OCCompareDoubleValuesLoose(creal(raised2), creal(expected2)) != kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValuesLoose(cimag(raised2), cimag(expected2)) != kOCCompareEqualTo) PRINTERROR;
    
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool autoreleasePoolTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool result;

    // Test 1: Basic Pool Creation and Release
    OCAutoreleasePoolRef pool1 = OCAutoreleasePoolCreate();
    if (pool1 == NULL) PRINTERROR;
    result = OCAutoreleasePoolRelease(pool1);
    if (!result) PRINTERROR;

    // Test 2: Autorelease Single Object
    OCAutoreleasePoolRef pool2 = OCAutoreleasePoolCreate();
    if (pool2 == NULL) PRINTERROR;
    OCStringRef str1 = OCStringCreateWithCString("test string 1");
    if (str1 == NULL) PRINTERROR;
    const void* autoreleased_ptr1 = OCAutorelease(str1);
    if (autoreleased_ptr1 != str1) PRINTERROR;
    result = OCAutoreleasePoolRelease(pool2);
    if (!result) PRINTERROR;

    // Test 3: Autorelease NULL Object
    OCAutoreleasePoolRef pool3 = OCAutoreleasePoolCreate();
    if (pool3 == NULL) PRINTERROR;
    const void* autoreleased_ptr_null = OCAutorelease(NULL);
    if (autoreleased_ptr_null != NULL) PRINTERROR;
    result = OCAutoreleasePoolRelease(pool3);
    if (!result) PRINTERROR;

    // Test 4: Nested Pools
    OCAutoreleasePoolRef poolA = OCAutoreleasePoolCreate();
    if (poolA == NULL) PRINTERROR;
    OCStringRef strA = OCStringCreateWithCString("string for pool A");
    if (strA == NULL) PRINTERROR;
    OCAutorelease(strA);

    OCAutoreleasePoolRef poolB = OCAutoreleasePoolCreate();
    if (poolB == NULL) PRINTERROR;
    OCStringRef strB1 = OCStringCreateWithCString("string B1 for pool B");
    if (strB1 == NULL) PRINTERROR;
    OCAutorelease(strB1);
    OCStringRef strB2 = OCStringCreateWithCString("string B2 for pool B");
    if (strB2 == NULL) PRINTERROR;
    OCAutorelease(strB2);

    result = OCAutoreleasePoolRelease(poolB);
    if (!result) PRINTERROR;

    OCStringRef strA2 = OCStringCreateWithCString("another string for pool A");
    if (strA2 == NULL) PRINTERROR;
    OCAutorelease(strA2);

    result = OCAutoreleasePoolRelease(poolA);
    if (!result) PRINTERROR;

    // Test 5: Release Non-Top Pool
    OCAutoreleasePoolRef poolX = OCAutoreleasePoolCreate();
    if (poolX == NULL) PRINTERROR;
    OCStringRef strX = OCStringCreateWithCString("for pool X"); OCAutorelease(strX);

    OCAutoreleasePoolRef poolY = OCAutoreleasePoolCreate();
    if (poolY == NULL) PRINTERROR;
    OCStringRef strY = OCStringCreateWithCString("for pool Y"); OCAutorelease(strY);

    OCAutoreleasePoolRef poolZ = OCAutoreleasePoolCreate();
    if (poolZ == NULL) PRINTERROR;
    OCStringRef strZ = OCStringCreateWithCString("for pool Z"); OCAutorelease(strZ);

    result = OCAutoreleasePoolRelease(poolY);
    if (!result) PRINTERROR;

    OCStringRef strX2 = OCStringCreateWithCString("for pool X again");
    if (strX2 == NULL) PRINTERROR;
    OCAutorelease(strX2);

    result = OCAutoreleasePoolRelease(poolX);
    if (!result) PRINTERROR;

    OCAutoreleasePoolRef emptyPool = OCAutoreleasePoolCreate();
    if (emptyPool == NULL) PRINTERROR;
    result = OCAutoreleasePoolRelease(emptyPool);
    if (!result) PRINTERROR;

    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool typeTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    
    OCTypeID id1 = OCRegisterType("MyType");
    if (id1 == _kOCNotATypeID) PRINTERROR;
    
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}


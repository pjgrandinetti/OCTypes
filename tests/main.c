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
bool autoreleasePoolTest0(void);

int main(int argc, const char * argv[]) {
    int failures = 0;
    if (!typeTest0())     failures++;
    if (!stringTest0())   failures++;
    if (!stringTest1())   failures++;
    if (!stringTest2())   failures++;
    if (!arrayTest0())    failures++;
    if (!mathTest0())     failures++;
    if (!autoreleasePoolTest0()) failures++;
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
    
    // Create and append initial strings
    const char *order[] = { "e", "b", "a", "c", "d" };
    OCStringRef created[5];
    for (int i = 0; i < 5; i++) {
        created[i] = OCStringCreateWithCString(order[i]);
        OCArrayAppendValue(array, created[i]);
    }
    // Verify insertion
    for (int i = 0; i < 5; i++) {
        OCStringRef s = (OCStringRef)OCArrayGetValueAtIndex(array, i);
        OCStringRef temp = OCStringCreateWithCString(order[i]);
        bool eq = OCStringEqual(s, temp);
        OCRelease(temp);
        if (!eq) PRINTERROR;
    }
    // Sort
    OCRange full = OCRangeMake(0, OCArrayGetCount(array));
    OCArraySortValues(array, full, OCStringCompareAdapter, NULL);
    // Check sorted
    const char *sorted[] = { "a", "b", "c", "d", "e" };
    for (int i = 0; i < 5; i++) {
        OCStringRef s = (OCStringRef)OCArrayGetValueAtIndex(array, i);
        OCStringRef temp = OCStringCreateWithCString(sorted[i]);
        bool eq = OCStringEqual(s, temp);
        OCRelease(temp);
        if (!eq) PRINTERROR;
    }
    fprintf(stderr, "%s end...without problems\n", __func__);
    OCRelease(array);
    // Release our created strings
    for (int i = 0; i < 5; i++) OCRelease(created[i]);
    return true;
}

bool stringTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    
    double result;
    result = OCComplexFromCString("1+1");
    if (OCCompareDoubleValues(result, 2.0) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("42/2");
    if (OCCompareDoubleValues(result, 21.0) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("(42/2+10)*2");
    if (OCCompareDoubleValues(result, 62.0) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("sqrt(4)");
    if (OCCompareDoubleValues(result, 2.0) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("cbrt(8)");
    if (OCCompareDoubleValues(result, 2.0) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("qtrt(16)");
    if (OCCompareDoubleValues(result, 2.0) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("exp(16)");
    if (OCCompareDoubleValues(result, exp(16)) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("log(16)");
    if (OCCompareDoubleValues(result, log(16)) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("acos(0.5)");
    if (OCCompareDoubleValues(result, acos(0.5)) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("asin(0.5)");
    if (OCCompareDoubleValues(result, asin(0.5)) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("cos(123)");
    if (OCCompareDoubleValues(result, cos(123)) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("sin(123)");
    if (OCCompareDoubleValues(result, sin(123)) != kOCCompareEqualTo) PRINTERROR;

    double complex compl = OCComplexFromCString("conj(1+I)");
    if (OCCompareDoubleValues(creal(compl), creal(conj(1+I))) != kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValues(cimag(compl), cimag(conj(1+I))) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("creal(conj(1+I))");
    if (OCCompareDoubleValues(result, creal(conj(1+I))) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("cimag(conj(1+I))");
    if (OCCompareDoubleValues(result, cimag(conj(1+I))) != kOCCompareEqualTo) PRINTERROR;

    result = OCComplexFromCString("carg(1+I)");
    if (OCCompareDoubleValues(result, cargument(1+I)) != kOCCompareEqualTo) PRINTERROR;

    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool stringTest1(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    
    OCStringRef sTown = OCStringCreateWithCString("theStringWithAStringOnAStringTown");
    OCStringRef constStr = STR("theStringWithAStringOnAStringTown");
    if (!OCStringEqual(sTown, constStr)) PRINTERROR;
    OCRelease(sTown);
    OCRelease(constStr);
    
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool stringTest2(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    
    OCStringRef fmt1 = STR("Ix(%@)");
    OCStringRef arg1 = STR("H2");
    OCStringRef s1  = OCStringCreateWithFormat(fmt1, arg1);
    if (!OCStringEqual(s1, STR("Ix(H2)"))) PRINTERROR;
    OCRelease(s1);
    
    OCStringRef fmt2 = STR("Ix(%s)");
    OCStringRef s2   = OCStringCreateWithFormat(fmt2, "H2");
    if (!OCStringEqual(s2, STR("Ix(H2)"))) PRINTERROR;
    OCRelease(s2);
    
    // release format & arg constants
    OCRelease(fmt1);
    OCRelease(arg1);
    OCRelease(fmt2);
    
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool mathTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    
    double complex z = 2.0 + 3.0*I;
    double complex r0 = raise_to_integer_power(z, 0);
    if (OCCompareDoubleValuesLoose(creal(r0), 1.0) != kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValuesLoose(cimag(r0), 0.0) != kOCCompareEqualTo) PRINTERROR;

    double complex r2 = raise_to_integer_power(z, 2);
    double complex e2 = z * z;
    if (OCCompareDoubleValuesLoose(creal(r2), creal(e2)) != kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValuesLoose(cimag(r2), cimag(e2)) != kOCCompareEqualTo) PRINTERROR;

    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool autoreleasePoolTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool res;

    OCAutoreleasePoolRef p1 = OCAutoreleasePoolCreate(); if (!p1) PRINTERROR;
    res = OCAutoreleasePoolRelease(p1); if (!res) PRINTERROR;

    OCAutoreleasePoolRef p2 = OCAutoreleasePoolCreate(); if (!p2) PRINTERROR;
    OCStringRef st1 = OCStringCreateWithCString("test string 1"); if (!st1) PRINTERROR;
    if (OCAutorelease(st1) != st1) PRINTERROR;
    res = OCAutoreleasePoolRelease(p2); if (!res) PRINTERROR;

    OCAutoreleasePoolRef p3 = OCAutoreleasePoolCreate(); if (!p3) PRINTERROR;
    if (OCAutorelease(NULL) != NULL) PRINTERROR;
    res = OCAutoreleasePoolRelease(p3); if (!res) PRINTERROR;

    OCAutoreleasePoolRef pa = OCAutoreleasePoolCreate(); if (!pa) PRINTERROR;
    OCStringRef sa = OCStringCreateWithCString("string for pool A"); OCAutorelease(sa);
    OCAutoreleasePoolRef pb = OCAutoreleasePoolCreate(); if (!pb) PRINTERROR;
    OCStringRef sb1 = OCStringCreateWithCString("string B1 for pool B"); OCAutorelease(sb1);
    OCStringRef sb2 = OCStringCreateWithCString("string B2 for pool B"); OCAutorelease(sb2);
    res = OCAutoreleasePoolRelease(pb); if (!res) PRINTERROR;
    OCStringRef sa2 = OCStringCreateWithCString("another string for pool A"); OCAutorelease(sa2);
    res = OCAutoreleasePoolRelease(pa); if (!res) PRINTERROR;

    OCAutoreleasePoolRef px = OCAutoreleasePoolCreate(); if (!px) PRINTERROR;
    OCAutorelease(OCStringCreateWithCString("for pool X"));
    OCAutoreleasePoolRef py = OCAutoreleasePoolCreate(); if (!py) PRINTERROR;
    OCAutorelease(OCStringCreateWithCString("for pool Y"));
    OCAutoreleasePoolRef pz = OCAutoreleasePoolCreate(); if (!pz) PRINTERROR;
    OCAutorelease(OCStringCreateWithCString("for pool Z"));
    res = OCAutoreleasePoolRelease(py); if (!res) PRINTERROR;
    OCAutorelease(OCStringCreateWithCString("for pool X again"));
    res = OCAutoreleasePoolRelease(px); if (!res) PRINTERROR;
    OCAutoreleasePoolRef empty = OCAutoreleasePoolCreate(); if (!empty) PRINTERROR;
    res = OCAutoreleasePoolRelease(empty); if (!res) PRINTERROR;

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


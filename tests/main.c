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
#include <stdint.h>
#include "../src/OCLibrary.h"
#include "../src/OCMath.h"             // Prototypes for cargument, ccbrt, etc.
#include "../src/OCAutoreleasePool.h"  // For autorelease‐pool tests
#include "../src/OCData.h"             // For OCData tests

// If raise_to_integer_power isn't in OCMath.h, declare it:
extern double complex raise_to_integer_power(double complex x, long power);

// Adapter for OCArraySortValues on OCStringRefs:
OCComparisonResult OCStringCompareAdapter(const void *a, const void *b, void *ctx) {
    (void)ctx;
    return OCStringCompare((OCStringRef)a, (OCStringRef)b, 0);
}

// Test prototypes
bool stringTest0(void);
bool stringTest1(void);
bool stringTest2(void);
bool arrayTest0(void);
bool mathTest0(void);
bool autoreleasePoolTest0(void);
bool dataTest0(void);
bool typeTest0(void);

int main(int argc, const char * argv[]) {
    int failures = 0;
    if (!typeTest0())            failures++;
    if (!stringTest0())          failures++;
    if (!stringTest1())          failures++;
    if (!stringTest2())          failures++;
    if (!arrayTest0())           failures++;
    if (!mathTest0())            failures++;
    if (!autoreleasePoolTest0()) failures++;
    if (!dataTest0())            failures++;
    if (failures) {
        fprintf(stderr, "%d test(s) failed\n", failures);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "All tests passed\n");
    return EXIT_SUCCESS;
}

bool stringTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    double r;
    r = OCComplexFromCString("1+1");
    if (OCCompareDoubleValues(r, 2.0)!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("42/2");
    if (OCCompareDoubleValues(r, 21.0)!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("(42/2+10)*2");
    if (OCCompareDoubleValues(r, 62.0)!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("sqrt(4)");
    if (OCCompareDoubleValues(r, 2.0)!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("cbrt(8)");
    if (OCCompareDoubleValues(r, 2.0)!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("qtrt(16)");
    if (OCCompareDoubleValues(r, 2.0)!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("exp(16)");
    if (OCCompareDoubleValues(r, exp(16))!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("log(16)");
    if (OCCompareDoubleValues(r, log(16))!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("acos(0.5)");
    if (OCCompareDoubleValues(r, acos(0.5))!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("asin(0.5)");
    if (OCCompareDoubleValues(r, asin(0.5))!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("cos(123)");
    if (OCCompareDoubleValues(r, cos(123))!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("sin(123)");
    if (OCCompareDoubleValues(r, sin(123))!=kOCCompareEqualTo) PRINTERROR;

    double complex cz = OCComplexFromCString("conj(1+I)");
    if (OCCompareDoubleValues(creal(cz), creal(conj(1+I)))!=kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValues(cimag(cz), cimag(conj(1+I)))!=kOCCompareEqualTo) PRINTERROR;

    r = OCComplexFromCString("creal(conj(1+I))");
    if (OCCompareDoubleValues(r, creal(conj(1+I)))!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("cimag(conj(1+I))");
    if (OCCompareDoubleValues(r, cimag(conj(1+I)))!=kOCCompareEqualTo) PRINTERROR;
    r = OCComplexFromCString("carg(1+I)");
    if (OCCompareDoubleValues(r, cargument(1+I))!=kOCCompareEqualTo) PRINTERROR;

    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool stringTest1(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCStringRef s1 = OCStringCreateWithCString("theStringWithAStringOnAStringTown");
    if (!OCStringEqual(s1, STR("theStringWithAStringOnAStringTown"))) PRINTERROR;
    OCRelease(s1);
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool stringTest2(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCStringRef fmt1 = STR("Ix(%@)");
    OCStringRef arg1 = STR("H2");
    OCStringRef out1 = OCStringCreateWithFormat(fmt1, arg1);
    if (!OCStringEqual(out1, STR("Ix(H2)"))) PRINTERROR;
    OCRelease(out1);

    OCStringRef fmt2 = STR("Ix(%s)");
    OCStringRef out2 = OCStringCreateWithFormat(fmt2, "H2");
    if (!OCStringEqual(out2, STR("Ix(H2)"))) PRINTERROR;
    OCRelease(out2);

    OCRelease(fmt1);
    OCRelease(arg1);
    OCRelease(fmt2);

    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool arrayTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCMutableArrayRef arr = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    const char *in[]   = {"e","b","a","c","d"};
    const char *want[] = {"a","b","c","d","e"};
    OCStringRef temp[5];
    for(int i=0;i<5;i++){
        temp[i] = OCStringCreateWithCString(in[i]);
        OCArrayAppendValue(arr, temp[i]);
    }
    // verify
    for(int i=0;i<5;i++){
        OCStringRef got = (OCStringRef)OCArrayGetValueAtIndex(arr,i);
        OCStringRef chk = OCStringCreateWithCString(in[i]);
        if (!OCStringEqual(got,chk)) { OCRelease(chk); PRINTERROR; }
        OCRelease(chk);
    }
    OCArraySortValues(arr, OCRangeMake(0,5), OCStringCompareAdapter, NULL);
    for(int i=0;i<5;i++){
        OCStringRef got = (OCStringRef)OCArrayGetValueAtIndex(arr,i);
        OCStringRef chk = OCStringCreateWithCString(want[i]);
        if (!OCStringEqual(got,chk)) { OCRelease(chk); PRINTERROR; }
        OCRelease(chk);
    }
    fprintf(stderr, "%s end...without problems\n", __func__);
    for(int i=0;i<5;i++) OCRelease(temp[i]);
    OCRelease(arr);
    return true;
}

bool mathTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    double complex z = 2.0 + 3.0*I;
    double complex r0 = raise_to_integer_power(z,0);
    if (OCCompareDoubleValuesLoose(creal(r0),1.0)!=kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValuesLoose(cimag(r0),0.0)!=kOCCompareEqualTo) PRINTERROR;
    double complex r2 = raise_to_integer_power(z,2);
    double complex e2 = z*z;
    if (OCCompareDoubleValuesLoose(creal(r2),creal(e2))!=kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValuesLoose(cimag(r2),cimag(e2))!=kOCCompareEqualTo) PRINTERROR;
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool autoreleasePoolTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool ok;
    // simple create/release
    OCAutoreleasePoolRef p1 = OCAutoreleasePoolCreate(); if(!p1) PRINTERROR;
    ok = OCAutoreleasePoolRelease(p1); if(!ok) PRINTERROR;
    // autorelease one object
    OCAutoreleasePoolRef p2 = OCAutoreleasePoolCreate(); 
    OCStringRef s2 = OCStringCreateWithCString("t"); 
    if (OCAutorelease(s2)!=s2) PRINTERROR;
    ok = OCAutoreleasePoolRelease(p2); if(!ok) PRINTERROR;
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool dataTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    // Test1
    uint8_t b1[] = {1,2,3,4,5};
    OCDataRef d1 = OCDataCreate(b1,sizeof b1);
    if(!d1) PRINTERROR;
    if(OCDataGetLength(d1)!=sizeof b1) PRINTERROR;
    const uint8_t *p1 = OCDataGetBytePtr(d1);
    for(size_t i=0;i<sizeof b1;i++) if(p1[i]!=b1[i]) PRINTERROR;

    // Test2: no-copy on heap
    size_t L2 = 3;
    uint8_t *b2 = malloc(L2);
    if(!b2) { OCRelease(d1); PRINTERROR; }
    b2[0]=0x0A; b2[1]=0x0B; b2[2]=0x0C;
    OCDataRef d2 = OCDataCreateWithBytesNoCopy(b2,L2);
    if(!d2){ free(b2); OCRelease(d1); PRINTERROR; }
    if(OCDataGetLength(d2)!=L2){ OCRelease(d2); OCRelease(d1); PRINTERROR; }
    const uint8_t *p2 = OCDataGetBytePtr(d2);
    for(size_t i=0;i<L2;i++) if(p2[i]!=b2[i]){ OCRelease(d2); OCRelease(d1); PRINTERROR; }
    // release d2 → must free(b2)
    OCRelease(d2);

    // Test3: copy
    OCDataRef d3 = OCDataCreateCopy(d1);
    if(!d3) PRINTERROR;
    if(OCDataGetLength(d3)!=OCDataGetLength(d1)) PRINTERROR;
    const uint8_t *p3 = OCDataGetBytePtr(d3);
    if(p3==p1) PRINTERROR;
    for(size_t i=0;i<OCDataGetLength(d3);i++) if(p3[i]!=p1[i]) PRINTERROR;
    OCRelease(d3);

    OCRelease(d1);
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool typeTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCTypeID tid = OCRegisterType("MyType");
    if(tid==_kOCNotATypeID) PRINTERROR;
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

#include "test_string.h"
#include "../src/OCString.h"
#include "../src/OCMath.h" // For OCComplexFromCString, OCCompareDoubleValues
#include <math.h> // For standard math functions like exp, log, acos, asin, cos, sin
#include <complex.h> // For creal, cimag, conj

// Original stringTest0 implementation
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

// Original stringTest1 implementation
bool stringTest1(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCStringRef s1 = OCStringCreateWithCString("theStringWithAStringOnAStringTown");
    if (!OCStringEqual(s1, STR("theStringWithAStringOnAStringTown"))) PRINTERROR;
    OCRelease(s1);
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

// Original stringTest2 implementation
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

    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

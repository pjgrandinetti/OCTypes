#include "test_string.h"
#include "../src/OCString.h"
#include "../src/OCMath.h"      // for OCComplexFromCString, OCCompareDoubleValues
#include <math.h>              // exp, log, acos, asin, cos, sin
#include <complex.h>           // creal, cimag, conj
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h> // Added for strcmp

// ————————— existing tests —————————

bool complex_parser_Test0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = false;
    double r;

    r = OCComplexFromCString("1+1");
    if (OCCompareDoubleValues(r, 2.0) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("42/2");
    if (OCCompareDoubleValues(r, 21.0) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("(42/2+10)*2");
    if (OCCompareDoubleValues(r, 62.0) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("sqrt(4)");
    if (OCCompareDoubleValues(r, 2.0) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("cbrt(8)");
    if (OCCompareDoubleValues(r, 2.0) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("qtrt(16)");
    if (OCCompareDoubleValues(r, 2.0) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("exp(16)");
    if (OCCompareDoubleValues(r, exp(16)) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("log(16)");
    if (OCCompareDoubleValues(r, log(16)) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("acos(0.5)");
    if (OCCompareDoubleValues(r, acos(0.5)) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("asin(0.5)");
    if (OCCompareDoubleValues(r, asin(0.5)) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("cos(123)");
    if (OCCompareDoubleValues(r, cos(123)) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("sin(123)");
    if (OCCompareDoubleValues(r, sin(123)) != kOCCompareEqualTo) goto cleanup;

    double complex cz = OCComplexFromCString("conj(1+I)");
    if (OCCompareDoubleValues(creal(cz), creal(conj(1+I))) != kOCCompareEqualTo) goto cleanup;
    if (OCCompareDoubleValues(cimag(cz), cimag(conj(1+I))) != kOCCompareEqualTo) goto cleanup;

    r = OCComplexFromCString("creal(conj(1+I))");
    if (OCCompareDoubleValues(r, creal(conj(1+I))) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("cimag(conj(1+I))");
    if (OCCompareDoubleValues(r, cimag(conj(1+I))) != kOCCompareEqualTo) goto cleanup;
    r = OCComplexFromCString("carg(1+I)");
    if (OCCompareDoubleValues(r, carg(1+I)) != kOCCompareEqualTo) goto cleanup;

    success = true;
cleanup:
    if (!success) fprintf(stderr, "Test %s FAILED.\n", __func__);
    else         fprintf(stderr, "%s passed.\n", __func__);
    return success;
}


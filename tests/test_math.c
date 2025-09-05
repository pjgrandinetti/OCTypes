#include "test_math.h"
#include "../src/OCMath.h"  // For OCCompareDoubleValuesLoose and other math functions if used directly
// Define _USE_MATH_DEFINES before including math.h to get M_PI_4 on Windows
#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <complex.h>  // For double complex, creal, cimag, I
#include <math.h>     // For mathematical constants and functions
// Fallback definitions for mathematical constants if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_4
#define M_PI_4 (M_PI / 4.0)
#endif
// Definition for raise_to_integer_power if it's not part of the library
// This is here because it was extern in the original main.c
// Ideally, this function should be part of OCMath.c and declared in OCMath.h
/* extern double complex raise_to_integer_power(double complex x, long power); */
// Assuming raise_to_integer_power is available from linking with OCMath.o or similar
// Original mathTest0 implementation
bool mathTest0(void) {
    fprintf(stderr, "%s begin...", __func__);
    double complex z = 2.0 + 3.0 * I;
    double complex r0 = raise_to_integer_power(z, 0);
    if (OCCompareDoubleValuesLoose(creal(r0), 1.0) != kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValuesLoose(cimag(r0), 0.0) != kOCCompareEqualTo) PRINTERROR;
    double complex r2 = raise_to_integer_power(z, 2);
    double complex e2 = z * z;
    if (OCCompareDoubleValuesLoose(creal(r2), creal(e2)) != kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValuesLoose(cimag(r2), cimag(e2)) != kOCCompareEqualTo) PRINTERROR;
    fprintf(stderr, " passed\n");
    return true;
}
// Consolidated mathTest1 from test_math_extra.c
bool mathTest1(void) {
    fprintf(stderr, "%s begin...", __func__);
    // Test cargument
    double complex z = 1.0 + 1.0 * I;
    double arg = cargument(z);
    if (OCCompareDoubleValuesLoose(arg, M_PI_4) != kOCCompareEqualTo) PRINTERROR;
    // Test ccbrt
    double complex cbrtRes = ccbrt(8.0 + 0.0 * I);
    if (OCCompareDoubleValuesLoose(creal(cbrtRes), 2.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(cbrtRes), 0.0) != kOCCompareEqualTo) PRINTERROR;
    // Test cqtrt
    double complex qtrtRes = cqtrt(16.0 + 0.0 * I);
    if (OCCompareDoubleValuesLoose(creal(qtrtRes), 2.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(qtrtRes), 0.0) != kOCCompareEqualTo) PRINTERROR;
    // Test compare values
    if (OCCompareFloatValues(1.0f, 1.0f) != kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValues(2.0, 3.0) != kOCCompareLessThan) PRINTERROR;
    if (OCCompareDoubleValuesLoose(0.10000001, 0.10000002) != kOCCompareEqualTo) PRINTERROR;
    // Test floor/ceil
    if (OCDoubleFloor(3.7) != 3.0) PRINTERROR;
    if (OCDoubleCeil(3.1) != 4.0) PRINTERROR;
    // Test trig
    double complex angle = M_PI_4 + 0.0 * I;
    if (OCCompareDoubleValuesLoose(creal(complex_sine(angle)), sin(M_PI_4)) != kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValuesLoose(creal(complex_cosine(angle)), cos(M_PI_4)) != kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValuesLoose(creal(complex_tangent(angle)), tan(M_PI_4)) != kOCCompareEqualTo) PRINTERROR;
    fprintf(stderr, " passed\n");
    return true;
}

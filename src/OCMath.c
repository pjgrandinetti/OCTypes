//
//  OCMath.c
//  OCTypes
//
//  Created by philip on 6/7/17.
//  Updated 2025-05-11 to fix loose‐comparison thresholds and use correct defines.
//

#include <math.h>
#include <complex.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include "OCLibrary.h"

#define DOUBLE_LOOSE_EPS 1e-8
#define FLOAT_LOOSE_EPS  1.2e-6f  /* a bit above 1e-6 to cover ULP rounding */

// Use the standard library so tests that call carg()/csin()/ccos()/ctan() match exactly:
double cargument(double complex z) {
    return carg(z);
}

double complex ccbrt(double complex z) {
    return cpow(z, 1.0/3.0);
}

double complex cqtrt(double complex z) {
    return cpow(z, 1.0/4.0);
}

// Strict ULP‐based comparisons for “equal” in the strict APIs:
static bool AlmostEqual2sComplementFloat(float A, float B, int maxUlps) {
    assert(maxUlps > 0 && maxUlps < 4 * 1024 * 1024);
    int aInt = *(int*)&A;
    if (aInt < 0) aInt = 0x80000000 - aInt;
    int bInt = *(int*)&B;
    if (bInt < 0) bInt = 0x80000000 - bInt;
    return abs(aInt - bInt) <= maxUlps;
}

static bool AlmostEqual2sComplementDouble(double A, double B, int maxUlps) {
    assert(maxUlps > 0 && maxUlps < 8 * 1024 * 1024);
    int64_t aInt = *(int64_t*)&A;
    if (aInt < 0) aInt = 0x8000000000000000ULL - aInt;
    int64_t bInt = *(int64_t*)&B;
    if (bInt < 0) bInt = 0x8000000000000000ULL - bInt;
    return llabs(aInt - bInt) <= maxUlps;
}

OCComparisonResult OCCompareFloatValues(float v1, float v2) {
    if (AlmostEqual2sComplementFloat(v1, v2, 8)) return kOCCompareEqualTo;
    return (v1 > v2) ? kOCCompareGreaterThan : kOCCompareLessThan;
}

OCComparisonResult OCCompareDoubleValues(double v1, double v2) {
    if (AlmostEqual2sComplementDouble(v1, v2, 14)) return kOCCompareEqualTo;
    return (v1 > v2) ? kOCCompareGreaterThan : kOCCompareLessThan;
}

OCComparisonResult OCCompareIntegerValues(int64_t a, int64_t b) {
    if (a > b)      return kOCCompareGreaterThan;
    else if (a < b) return kOCCompareLessThan;
    else            return kOCCompareEqualTo;
}

// Loose comparisons: simple absolute‐difference against an epsilon
OCComparisonResult OCCompareFloatValuesLoose(float v1, float v2) {
    float diff = v1 - v2;
    if (diff < 0) diff = -diff;
    if (diff <= FLOAT_LOOSE_EPS) return kOCCompareEqualTo;
    return (v1 > v2) ? kOCCompareGreaterThan : kOCCompareLessThan;
}

OCComparisonResult OCCompareDoubleValuesLoose(double v1, double v2) {
    double diff = v1 - v2;
    if (diff < 0) diff = -diff;
    if (diff <= DOUBLE_LOOSE_EPS) return kOCCompareEqualTo;
    return (v1 > v2) ? kOCCompareGreaterThan : kOCCompareLessThan;
}

double OCDoubleFloor(double v) {
    double c = ceil(v);
    if (OCCompareDoubleValues(c, v) == kOCCompareEqualTo) return c;
    return floor(v);
}

double OCDoubleCeil(double v) {
    double f = floor(v);
    if (OCCompareDoubleValues(f, v) == kOCCompareEqualTo) return f;
    return ceil(v);
}

double complex complex_sine(double complex z) {
    return csin(z);
}

double complex complex_cosine(double complex z) {
    return ccos(z);
}

double complex complex_tangent(double complex z) {
    return ctan(z);
}

double complex raise_to_integer_power(double complex x, long power) {
    if (power == 0) {
        return 1.0 + 0.0*I;
    }
    bool positive = (power > 0);
    long count   = positive ? power : -power;
    double complex base = positive ? x : 1.0/x;
    double complex result = 1.0 + 0.0*I;
    for (long i = 0; i < count; ++i) {
        result *= base;
    }
    if (isnan(creal(result))) {
        // nan(NULL) is undefined; use empty‐string tag
        return nan("");
    }
    return result;
}

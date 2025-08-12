#include "../src/OCMath.h"  // for OCComplexFromCString, OCCompareDoubleValuesLoose
#include "../src/OCString.h"
#include "test_string.h"
// Define _USE_MATH_DEFINES before including math.h to get M_PI on Windows
#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <complex.h>  // creal, cimag, conj, cexp, clog, csin, ccos, casin, cacos, cargument
#include <math.h>     // exp, log, acos, asin, cos, sin, sqrt, isinf, isnan
#include <stdio.h>
#include <stdlib.h>
// Fallback definition for M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define PRINTFAILURE(msg)                          \
    do {                                           \
        fprintf(stderr, "TEST FAILED: %s\n", msg); \
        success = false;                           \
    } while (0)
#define PRINTFAILURE_WITH_VALUES(msg, actual, expected)                                        \
    do {                                                                                       \
        fprintf(stderr, "TEST FAILED: %s\nActual: %f\nExpected: %f\n", msg, actual, expected); \
        success = false;                                                                       \
    } while (0)
bool complex_parser_Test0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = true;  // Initialize success to true
    double complex z;
    // ——— Simple real and imaginary constants ———
    z = OCComplexFromCString("0");
    if (OCCompareDoubleValuesLoose(creal(z), 0.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), 0.0) != kOCCompareEqualTo)
        PRINTFAILURE("0");
    z = OCComplexFromCString("12345");
    if (OCCompareDoubleValuesLoose(creal(z), 12345.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), 0.0) != kOCCompareEqualTo)
        PRINTFAILURE("12345");
    z = OCComplexFromCString("I");
    if (OCCompareDoubleValuesLoose(creal(z), 0.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), 1.0) != kOCCompareEqualTo)
        PRINTFAILURE("I");
    z = OCComplexFromCString("-I");
    if (OCCompareDoubleValuesLoose(creal(z), 0.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), -1.0) != kOCCompareEqualTo)
        PRINTFAILURE("-I");
    z = OCComplexFromCString("pi");
    if (OCCompareDoubleValuesLoose(creal(z), M_PI) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), 0.0) != kOCCompareEqualTo)
        PRINTFAILURE("pi");
    // ——— Basic arithmetic ———
    z = OCComplexFromCString("1+2");
    if (OCCompareDoubleValuesLoose(creal(z), 3.0) != kOCCompareEqualTo) PRINTFAILURE("1+2");
    z = OCComplexFromCString("6*7");
    if (OCCompareDoubleValuesLoose(creal(z), 42.0) != kOCCompareEqualTo) PRINTFAILURE("6*7");
    z = OCComplexFromCString("84/2");
    if (OCCompareDoubleValuesLoose(creal(z), 42.0) != kOCCompareEqualTo) PRINTFAILURE("84/2");
    // ——— Operator precedence and grouping ———
    z = OCComplexFromCString("2+3*4");
    if (OCCompareDoubleValuesLoose(creal(z), 14.0) != kOCCompareEqualTo) PRINTFAILURE("2+3*4");
    z = OCComplexFromCString("(2+3)*4");
    if (OCCompareDoubleValuesLoose(creal(z), 20.0) != kOCCompareEqualTo) PRINTFAILURE("(2+3)*4");
    z = OCComplexFromCString("-3+4*I");
    if (OCCompareDoubleValuesLoose(creal(z), -3.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), 4.0) != kOCCompareEqualTo) {
        fprintf(stderr, "Test failed: -3+4*I\n");
        fprintf(stderr, "Actual: creal=%f, cimag=%f\n", creal(z), cimag(z));
        fprintf(stderr, "Expected: creal=-3.0, cimag=4.0\n");
        success = false;  // Record failure and continue
    }
    // ——— Absolute value ———
    z = OCComplexFromCString("|(-3+4*I)|");
    if (OCCompareDoubleValuesLoose(creal(z), 5.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), 0.0) != kOCCompareEqualTo) {
        fprintf(stderr, "Test failed: |(-3+4*I)|\n");
        fprintf(stderr, "Actual: creal=%f, cimag=%f\n", creal(z), cimag(z));
        fprintf(stderr, "Expected: creal=5.0, cimag=0.0\n");
        success = false;  // Record failure and continue
    }
    z = OCComplexFromCString("5-7");
    if (OCCompareDoubleValuesLoose(creal(z), -2.0) != kOCCompareEqualTo) PRINTFAILURE("5-7");
    z = OCComplexFromCString("||-3+4*I| - 5|");
    if (OCCompareDoubleValuesLoose(creal(z), 0.0) != kOCCompareEqualTo) PRINTFAILURE("||-3+4*I| - 5|");
    // ——— Decimal and scientific notation ———
    z = OCComplexFromCString("3.14+2.72*I");
    if (OCCompareDoubleValuesLoose(creal(z), 3.14) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), 2.72) != kOCCompareEqualTo)
        PRINTFAILURE("3.14+2.72*I");
    z = OCComplexFromCString("1e3-5E-1*I");
    if (OCCompareDoubleValuesLoose(creal(z), 1000.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), -0.5) != kOCCompareEqualTo)
        PRINTFAILURE("1e3-5E-1*I");
    // ——— Exponentiation and associativity ———
    z = OCComplexFromCString("2^3");
    if (OCCompareDoubleValuesLoose(creal(z), 8.0) != kOCCompareEqualTo) PRINTFAILURE("2^3");
    z = OCComplexFromCString("2^3^2");
    if (OCCompareDoubleValuesLoose(creal(z), 512.0) != kOCCompareEqualTo)
        PRINTFAILURE_WITH_VALUES("2^3^2", creal(z), 512.0);
    z = OCComplexFromCString("-2^2");
    if (OCCompareDoubleValuesLoose(creal(z), -4.0) != kOCCompareEqualTo) PRINTFAILURE("-2^2");
    z = OCComplexFromCString("(-2)^2");
    if (OCCompareDoubleValuesLoose(creal(z), 4.0) != kOCCompareEqualTo) PRINTFAILURE("(-2)^2");
    // ——— Built-in functions, real-valued ———
    z = OCComplexFromCString("sqrt(16)");
    if (OCCompareDoubleValuesLoose(creal(z), 4.0) != kOCCompareEqualTo) PRINTFAILURE("sqrt(16)");
    z = OCComplexFromCString("cbrt(27)");
    if (OCCompareDoubleValuesLoose(creal(z), 3.0) != kOCCompareEqualTo) PRINTFAILURE("cbrt(27)");
    z = OCComplexFromCString("qtrt(81)");
    if (OCCompareDoubleValuesLoose(creal(z), 3.0) != kOCCompareEqualTo) PRINTFAILURE("qtrt(81)");
    z = OCComplexFromCString("exp(1)");
    if (OCCompareDoubleValuesLoose(creal(z), exp(1.0)) != kOCCompareEqualTo) PRINTFAILURE("exp(1)");
    z = OCComplexFromCString("log(exp(2))");
    if (OCCompareDoubleValuesLoose(creal(z), 2.0) != kOCCompareEqualTo) PRINTFAILURE("log(exp(2))");
    z = OCComplexFromCString("sin(pi/2)");
    if (OCCompareDoubleValuesLoose(creal(z), 1.0) != kOCCompareEqualTo) PRINTFAILURE("sin(pi/2)");
    z = OCComplexFromCString("cos(0)");
    if (OCCompareDoubleValuesLoose(creal(z), 1.0) != kOCCompareEqualTo) PRINTFAILURE("cos(0)");
    z = OCComplexFromCString("asin(0.5)");
    if (OCCompareDoubleValuesLoose(creal(z), asin(0.5)) != kOCCompareEqualTo) PRINTFAILURE("asin(0.5)");
    z = OCComplexFromCString("acos(0.5)");
    if (OCCompareDoubleValuesLoose(creal(z), acos(0.5)) != kOCCompareEqualTo) PRINTFAILURE("acos(0.5)");
    // ——— Nested single-arg functions ———
    z = OCComplexFromCString("sin(cos(0))");
    if (OCCompareDoubleValuesLoose(creal(z), sin(cos(0))) != kOCCompareEqualTo) PRINTFAILURE("sin(cos(0))");
    z = OCComplexFromCString("exp(log(5+2*I))");
    if (OCCompareDoubleValuesLoose(creal(z), 5.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), 2.0) != kOCCompareEqualTo)
        PRINTFAILURE("exp(log(5+2*I))");
    z = OCComplexFromCString("conj(conj(3+4*I))");
    if (OCCompareDoubleValuesLoose(creal(z), 3.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), 4.0) != kOCCompareEqualTo)
        PRINTFAILURE("conj(conj(3+4*I))");
    z = OCComplexFromCString("conj(sqrt(9+16*I))");
    {
        double complex expected = conj(csqrt(9.0 + 16.0 * I));
        if (OCCompareDoubleValuesLoose(creal(z), creal(expected)) != kOCCompareEqualTo ||
            OCCompareDoubleValuesLoose(cimag(z), cimag(expected)) != kOCCompareEqualTo)
            PRINTFAILURE("conj(sqrt(9+16*I))");
    }
    // ——— Complex exponentiation ———
    z = OCComplexFromCString("(1+I)^(2+3*I)");
    {
        double complex expected = cpow(1.0 + 1.0 * I, 2.0 + 3.0 * I);
        if (OCCompareDoubleValuesLoose(creal(z), creal(expected)) != kOCCompareEqualTo ||
            OCCompareDoubleValuesLoose(cimag(z), cimag(expected)) != kOCCompareEqualTo)
            PRINTFAILURE("(1+I)^(2+3*I)");
    }
    // ——— Whitespace and linebreak tolerance ———
    z = OCComplexFromCString("  - 2 ^  2  ");
    if (OCCompareDoubleValuesLoose(creal(z), -4.0) != kOCCompareEqualTo) PRINTFAILURE("  - 2 ^  2  ");
    z = OCComplexFromCString("1 +\n 2 *\t3");
    if (OCCompareDoubleValuesLoose(creal(z), 7.0) != kOCCompareEqualTo) PRINTFAILURE("1 +\\n2 *\\t3");
    // ——— Complicated nested parentheses and operations ———
    z = OCComplexFromCString("(54.8+cos(3.1))(45/4+log(3))-exp(-34)");
    {
        double complex expected =
            (54.8 + cos(3.1))               /* first group */
                * ((45.0 / 4.0) + log(3.0)) /* second group */
            - cexp(-34.0);                  /* subtraction of exp */
        if (OCCompareDoubleValuesLoose(creal(z), creal(expected)) != kOCCompareEqualTo ||
            OCCompareDoubleValuesLoose(cimag(z), cimag(expected)) != kOCCompareEqualTo) {
            fprintf(stderr, "Test failed: (54.8+cos(3.1))(45/4+log(3))-exp(-34)\n");
            fprintf(stderr, "Actual: creal=%f, cimag=%f\n", creal(z), cimag(z));
            fprintf(stderr, "Expected: creal=%f, cimag=%f\n", creal(expected), cimag(expected));
            success = false;  // Record failure and continue
        }
    }
    z = OCComplexFromCString("|-5|");
    if (OCCompareDoubleValuesLoose(creal(z), 5.0) != kOCCompareEqualTo) PRINTFAILURE("|-5|");
    z = OCComplexFromCString("||3-4*I||");
    if (OCCompareDoubleValuesLoose(creal(z), 5.0) != kOCCompareEqualTo) PRINTFAILURE("||3-4*I||");
    z = OCComplexFromCString("2-3-4");
    if (OCCompareDoubleValuesLoose(creal(z), -5.0) != kOCCompareEqualTo) PRINTFAILURE("2-3-4");
    z = OCComplexFromCString("2(3+4)");
    if (OCCompareDoubleValuesLoose(creal(z), 14.0) != kOCCompareEqualTo) PRINTFAILURE("2(3+4)");
    z = OCComplexFromCString("|sin(pi)|");
    if (OCCompareDoubleValuesLoose(creal(z), fabs(sin(M_PI))) != kOCCompareEqualTo) PRINTFAILURE("|sin(pi)|");
    z = OCComplexFromCString("sin(cos(sin(1)))");
    if (OCCompareDoubleValuesLoose(creal(z), sin(cos(sin(1.0)))) != kOCCompareEqualTo) PRINTFAILURE("sin(cos(sin(1)))");
    z = OCComplexFromCString("creal(3+4*I)");
    if (OCCompareDoubleValuesLoose(creal(z), 3.0) != kOCCompareEqualTo) PRINTFAILURE("creal(3+4*I)");
    z = OCComplexFromCString("cimag(3+4*I)");
    if (OCCompareDoubleValuesLoose(creal(z), 4.0) != kOCCompareEqualTo) PRINTFAILURE("cimag(3+4*I)");
    z = OCComplexFromCString("-2^2");
    if (OCCompareDoubleValuesLoose(creal(z), -4.0) != kOCCompareEqualTo) PRINTFAILURE("-2^2");
    // (This is already included, but test with whitespace.)
    z = OCComplexFromCString("- (2^2)");
    if (OCCompareDoubleValuesLoose(creal(z), -4.0) != kOCCompareEqualTo) PRINTFAILURE("- (2^2)");
    z = OCComplexFromCString("exp(I*pi)+1");
    if (OCCompareDoubleValuesLoose(creal(z), 0.0) != kOCCompareEqualTo) PRINTFAILURE("exp(I*pi)+1");
    // Euler's identity
    z = OCComplexFromCString("conj(|3+4*I|+I)");
    double complex expected = conj(5.0 + I);
    if (OCCompareDoubleValuesLoose(creal(z), creal(expected)) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), cimag(expected)) != kOCCompareEqualTo)
        PRINTFAILURE("conj(|3+4*I|+I)");
    z = OCComplexFromCString("(1+2*I)*(3+4*I)");
    if (OCCompareDoubleValuesLoose(creal(z), -5.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), 10.0) != kOCCompareEqualTo)
        PRINTFAILURE("(1+2*I)*(3+4*I)");
    z = OCComplexFromCString("(1+2)(3+4)");
    if (OCCompareDoubleValuesLoose(creal(z), 21.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), 0.0) != kOCCompareEqualTo)
        PRINTFAILURE("(1+2)(3+4)");
    z = OCComplexFromCString("log(-1)");
    if (OCCompareDoubleValuesLoose(creal(z), 0.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), M_PI) != kOCCompareEqualTo)
        PRINTFAILURE("log(-1)");
    z = OCComplexFromCString("sqrt(-1)");
    if (OCCompareDoubleValuesLoose(creal(z), 0.0) != kOCCompareEqualTo ||
        OCCompareDoubleValuesLoose(cimag(z), 1.0) != kOCCompareEqualTo)
        PRINTFAILURE("sqrt(-1)");

    // ——— Infinity tests ———
    z = OCComplexFromCString("inf");
    if (!isinf(creal(z)) || cimag(z) != 0.0)
        PRINTFAILURE("inf");

    z = OCComplexFromCString("∞");
    if (!isinf(creal(z)) || cimag(z) != 0.0)
        PRINTFAILURE("∞");

    z = OCComplexFromCString("-inf");
    if (!isinf(creal(z)) || creal(z) > 0 || cimag(z) != 0.0)
        PRINTFAILURE("-inf");

    z = OCComplexFromCString("-∞");
    if (!isinf(creal(z)) || creal(z) > 0 || cimag(z) != 0.0)
        PRINTFAILURE("-∞");

    // Infinity arithmetic
    z = OCComplexFromCString("inf + 5");
    if (!isinf(creal(z)))
        PRINTFAILURE("inf + 5");

    z = OCComplexFromCString("2 * inf");
    if (!isinf(creal(z)))
        PRINTFAILURE("2 * inf");

    z = OCComplexFromCString("∞ - 100");
    if (!isinf(creal(z)))
        PRINTFAILURE("∞ - 100");

    z = OCComplexFromCString("5 / inf");
    if (creal(z) != 0.0 || cimag(z) != 0.0)
        PRINTFAILURE("5 / inf");

    // Infinity in complex expressions
    z = OCComplexFromCString("inf + 3*I");
    if (!isinf(creal(z)) || cimag(z) != 3.0)
        PRINTFAILURE("inf + 3*I");

    z = OCComplexFromCString("2 + inf*I");
    if (!isnan(creal(z)) || !isinf(cimag(z)))
        PRINTFAILURE("2 + inf*I");

    // Infinity with functions
    z = OCComplexFromCString("|inf|");
    if (!isinf(creal(z)) || cimag(z) != 0.0)
        PRINTFAILURE("|inf|");

    z = OCComplexFromCString("sin(inf)");
    if (!isnan(creal(z))) // sin(inf) should be NaN
        PRINTFAILURE("sin(inf)");

    z = OCComplexFromCString("exp(inf)");
    if (!isinf(creal(z)))
        PRINTFAILURE("exp(inf)");

    // Infinity with parentheses and precedence
    z = OCComplexFromCString("(inf + 5) * 2");
    if (!isinf(creal(z)))
        PRINTFAILURE("(inf + 5) * 2");

    z = OCComplexFromCString("inf^2");
    if (!isinf(creal(z)))
        PRINTFAILURE("inf^2");

    if (!success)
        fprintf(stderr, "%s FAILED.\n", __func__);
    else
        fprintf(stderr, "%s passed.\n", __func__);
    return success;
}

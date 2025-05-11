#include "test_string.h"
#include "../src/OCString.h"
#include "../src/OCMath.h" // For OCComplexFromCString, OCCompareDoubleValues
#include <math.h> // For standard math functions like exp, log, acos, asin, cos, sin
#include <complex.h> // For creal, cimag, conj

// Original stringTest0 implementation
bool stringTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = false; // Initialize success flag
    double r;
    // Note: OCComplexFromCString returns a double, not an OCTypeRef.
    // No OCRelease is needed for 'r' or 'cz' themselves.
    // If OCComplexFromCString internally leaks OCStrings, that's an issue within OCComplexFromCString.

    r = OCComplexFromCString("1+1");
    if (OCCompareDoubleValues(r, 2.0) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"1+1\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("42/2");
    if (OCCompareDoubleValues(r, 21.0) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"42/2\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("(42/2+10)*2");
    if (OCCompareDoubleValues(r, 62.0) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"(42/2+10)*2\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("sqrt(4)");
    if (OCCompareDoubleValues(r, 2.0) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"sqrt(4)\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("cbrt(8)");
    if (OCCompareDoubleValues(r, 2.0) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"cbrt(8)\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("qtrt(16)");
    if (OCCompareDoubleValues(r, 2.0) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"qtrt(16)\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("exp(16)");
    if (OCCompareDoubleValues(r, exp(16)) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"exp(16)\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("log(16)");
    if (OCCompareDoubleValues(r, log(16)) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"log(16)\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("acos(0.5)");
    if (OCCompareDoubleValues(r, acos(0.5)) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"acos(0.5)\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("asin(0.5)");
    if (OCCompareDoubleValues(r, asin(0.5)) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"asin(0.5)\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("cos(123)");
    if (OCCompareDoubleValues(r, cos(123)) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"cos(123)\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("sin(123)");
    if (OCCompareDoubleValues(r, sin(123)) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"sin(123)\") failed in %s\n", __func__);
        goto cleanup;
    }

    double complex cz = OCComplexFromCString("conj(1+I)");
    if (OCCompareDoubleValues(creal(cz), creal(conj(1 + I))) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"conj(1+I)\") real part failed in %s\n", __func__);
        goto cleanup;
    }
    if (OCCompareDoubleValues(cimag(cz), cimag(conj(1 + I))) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"conj(1+I)\") imag part failed in %s\n", __func__);
        goto cleanup;
    }

    r = OCComplexFromCString("creal(conj(1+I))");
    if (OCCompareDoubleValues(r, creal(conj(1 + I))) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"creal(conj(1+I))\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("cimag(conj(1+I))");
    if (OCCompareDoubleValues(r, cimag(conj(1 + I))) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"cimag(conj(1+I))\") failed in %s\n", __func__);
        goto cleanup;
    }
    r = OCComplexFromCString("carg(1+I)");
    if (OCCompareDoubleValues(r, cargument(1 + I)) != kOCCompareEqualTo) {
        fprintf(stderr, "Error: OCComplexFromCString(\"carg(1+I)\") failed in %s\n", __func__);
        goto cleanup;
    }

    fprintf(stderr, "%s end...without problems\n", __func__);
    success = true; // Mark as successful

cleanup:
    // No OCStringRef objects were created directly in this function's scope to be released here.
    if (!success) {
        fprintf(stderr, "Test %s FAILED.\n", __func__);
    }
    return success;
}

// Original stringTest1 implementation
bool stringTest1(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = false; // Initialize success flag
    OCStringRef s1 = NULL;
    OCStringRef s_check = NULL;

    s1 = OCStringCreateWithCString("theStringWithAStringOnAStringTown");
    if (!s1) {
        fprintf(stderr, "Error: OCStringCreateWithCString failed in %s\n", __func__);
        goto cleanup;
    }
    
    s_check = STR("theStringWithAStringOnAStringTown"); // STR likely creates an OCStringRef
    if (!s_check) {
        fprintf(stderr, "Error: STR(\"theStringWithAStringOnAStringTown\") failed in %s\n", __func__);
        goto cleanup;
    }

    if (!OCStringEqual(s1, s_check)) {
        fprintf(stderr, "Error: OCStringEqual failed in %s\n", __func__);
        goto cleanup;
    }
    
    fprintf(stderr, "%s end...without problems\n", __func__);
    success = true; // Mark as successful

cleanup:
    if (s1) OCRelease(s1);
    if (!success) {
        fprintf(stderr, "Test %s FAILED.\n", __func__);
    }
    return success;
}

// Original stringTest2 implementation
bool stringTest2(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = false; // Initialize success flag
    OCStringRef fmt1 = NULL;
    OCStringRef arg1 = NULL;
    OCStringRef out1 = NULL;
    OCStringRef s_check1 = NULL;

    OCStringRef fmt2 = NULL;
    OCStringRef out2 = NULL;
    OCStringRef s_check2 = NULL;

    fmt1 = STR("Ix(%@)"); // STR likely creates an OCStringRef
    if (!fmt1) { 
        fprintf(stderr, "Error: STR(\"Ix(%%@)\") failed for fmt1 in %s\n", __func__); // Escaped %
        goto cleanup; 
    }
    arg1 = STR("H2"); // STR likely creates an OCStringRef
    if (!arg1) { 
        fprintf(stderr, "Error: STR(\"H2\") failed for arg1 in %s\n", __func__);
        goto cleanup; 
    }
    out1 = OCStringCreateWithFormat(fmt1, arg1);
    if (!out1) { 
        fprintf(stderr, "Error: OCStringCreateWithFormat for out1 failed in %s\n", __func__);
        goto cleanup; 
    }
    s_check1 = STR("Ix(H2)"); // STR likely creates an OCStringRef
    if (!s_check1) { 
        fprintf(stderr, "Error: STR(\"Ix(H2)\") failed for s_check1 in %s\n", __func__);
        goto cleanup; 
    }
    if (!OCStringEqual(out1, s_check1)) {
        fprintf(stderr, "Error: OCStringEqual for out1 failed in %s\n", __func__);
        goto cleanup;
    }

    fmt2 = STR("Ix(%s)"); // STR likely creates an OCStringRef
    if (!fmt2) { 
        fprintf(stderr, "Error: STR(\"Ix(%%s)\") failed for fmt2 in %s\n", __func__); // Escaped %
        goto cleanup; 
    }
    // For %s format specifier, OCStringCreateWithFormat expects a C string, not OCStringRef
    out2 = OCStringCreateWithFormat(fmt2, "H2"); 
    if (!out2) { 
        fprintf(stderr, "Error: OCStringCreateWithFormat for out2 failed in %s\n", __func__);
        goto cleanup; 
    }
    s_check2 = STR("Ix(H2)"); // STR likely creates an OCStringRef
    if (!s_check2) { 
        fprintf(stderr, "Error: STR(\"Ix(H2)\") failed for s_check2 in %s\n", __func__);
        goto cleanup; 
    }
    if (!OCStringEqual(out2, s_check2)) {
        fprintf(stderr, "Error: OCStringEqual for out2 failed in %s\n", __func__);
        goto cleanup;
    }

    fprintf(stderr, "%s end...without problems\n", __func__);
    success = true; // Mark as successful

cleanup:
    if (out1) OCRelease(out1); // out1 is from OCStringCreateWithFormat, needs release
    if (out2) OCRelease(out2); // out2 is from OCStringCreateWithFormat, needs release

    if (!success) {
        fprintf(stderr, "Test %s FAILED.\n", __func__);
    }
    return success;
}

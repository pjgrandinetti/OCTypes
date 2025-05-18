#include "test_string.h"
#include "../src/OCString.h"
#include "../src/OCMath.h"      // for OCComplexFromCString, OCCompareDoubleValues
#include <math.h>              // exp, log, acos, asin, cos, sin
#include <complex.h>           // creal, cimag, conj
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// ————————— existing tests —————————

bool stringTest0(void) {
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

bool stringTest1(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = false;
    OCStringRef s1 = OCStringCreateWithCString("theStringWithAStringOnAStringTown");
    OCStringRef s_check = STR("theStringWithAStringOnAStringTown");
    if (!s1 || !s_check)                           goto cleanup;
    if (!OCStringEqual(s1, s_check))               goto cleanup;
    success = true;
cleanup:
    if (s1) OCRelease(s1);
    if (!success) fprintf(stderr, "Test %s FAILED.\n", __func__);
    else         fprintf(stderr, "%s passed.\n", __func__);
    return success;
}

bool stringTest2(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = false;
    // Test OCStringCreateWithFormat with OCStringRef argument
    OCStringRef fmt1 = STR("Ix(%@)");
    OCStringRef out1 = OCStringCreateWithFormat(fmt1, STR("H2"));
    OCStringRef chk1 = STR("Ix(H2)");
    
    // Initialize out2 to NULL before potentially jumping to cleanup
    OCStringRef out2 = NULL;
    
    if (!OCStringEqual(out1, chk1)) goto cleanup;

    // Test OCStringCreateWithFormat with C-string %s
    OCStringRef fmt2 = STR("Ix(%s)");
    out2 = OCStringCreateWithFormat(fmt2, "H2");
    OCStringRef chk2 = STR("Ix(H2)");
    if (!OCStringEqual(out2, chk2)) goto cleanup;

    success = true;
cleanup:
    if (out1) OCRelease(out1);
    if (out2) OCRelease(out2);
    if (!success) fprintf(stderr, "Test %s FAILED.\n", __func__);
    else         fprintf(stderr, "%s passed.\n", __func__);
    return success;
}

// ————————— new UTF-8–aware tests —————————

bool stringTest3(void) {
    fprintf(stderr, "%s begin…\n", __func__);
    bool ok = true;
    // Contains: 'héllo • 世界'
    OCStringRef s = OCStringCreateWithCString("héllo • 世界");
    // Code-point length: h(1) é(1) l(1) l(1) o(1) space(1) •(1) space(1) 世(1) 界(1) = 10
    if (OCStringGetLength(s) != 10) ok = false;
    // Check a few code-points
    // index 1 → 'é' (0xE9)
    if ((unsigned char)OCStringGetCharacterAtIndex(s,1) != 0xE9) ok = false;
    // index 6 → '•' (0x2022)
    // full code-point, so returned low‐byte is 0xA2
    if (((uint8_t)OCStringGetCharacterAtIndex(s,6)) != 0xA2) ok = false;
    OCRelease(s);
    fprintf(stderr, "%s %s.\n", __func__, ok?"passed":"FAILED");
    return ok;
}

bool stringTest4(void) {
    fprintf(stderr, "%s begin…\n", __func__);
    bool ok = true;
    OCStringRef s = OCStringCreateWithCString("foo•bar•baz");
    // substring between first and second bullet
    OCRange r = OCRangeMake(4,1); 
    OCStringRef sub = OCStringCreateWithSubstring(s, r);
    if (!OCStringEqual(sub, STR("•"))) ok = false;
    OCRelease(sub);
    OCRelease(s);
    fprintf(stderr, "%s %s.\n", __func__, ok?"passed":"FAILED");
    return ok;
}

bool stringTest5(void) {
    fprintf(stderr, "%s begin…\n", __func__);
    bool ok = true;
    OCMutableStringRef m = OCStringCreateMutable(0);
    OCStringAppendCString(m, "α"); // Greek alpha (2 bytes in UTF-8)
    if (OCStringGetLength((OCStringRef)m) != 1) ok = false;
    OCStringAppend(m, STR("β"));   // Greek beta
    if (OCStringGetLength((OCStringRef)m) != 2) ok = false;
    // Final should be "αβ"
    if (!OCStringEqual((OCStringRef)m, STR("αβ"))) ok = false;
    OCRelease(m);
    fprintf(stderr, "%s %s.\n", __func__, ok?"passed":"FAILED");
    return ok;
}

bool stringTest6(void) {
    fprintf(stderr, "%s begin…\n", __func__);
    bool ok = true;
    // Start with "a•b•c"
    OCMutableStringRef m = OCStringCreateMutableCopy(STR("a•b•c"));
    // Delete the middle bullet
    OCStringDelete(m, OCRangeMake(1,1));
    if (!OCStringEqual((OCStringRef)m, STR("abc"))) ok = false;
    // Insert a unicode snowman
    OCStringInsert(m, 1, STR("☃"));
    if (!OCStringEqual((OCStringRef)m, STR("a☃bc"))) ok = false;
    // Replace ☃ with "🌟" (star)
    OCStringReplace(m, OCRangeMake(1,1), STR("🌟"));
    if (!OCStringEqual((OCStringRef)m, STR("a🌟bc"))) ok = false;
    // replaceAll 'b'→'B'
    OCStringReplaceAll(m, STR("B"));
    if (!OCStringEqual((OCStringRef)m, STR("a🌟Bc"))) ok = false;
    OCRelease(m);
    fprintf(stderr, "%s %s.\n", __func__, ok?"passed":"FAILED");
    return ok;
}

bool stringTest7(void) {
    fprintf(stderr, "%s begin…\n", __func__);
    bool ok = true;
    OCStringRef s = OCStringCreateWithCString("one•two•three");
    // find ranges
    OCArrayRef ranges = OCStringCreateArrayWithFindResults(s, STR("•"), OCRangeMake(0, OCStringGetLength(s)), 0);
    if (!ranges || OCArrayGetCount(ranges)!=2) ok = false;
    // split on bullet
    OCArrayRef parts = OCStringCreateArrayBySeparatingStrings(s, STR("•"));
    if (!parts || OCArrayGetCount(parts)!=3) ok = false;
    // check content
    if (!OCStringEqual((OCStringRef)OCArrayGetValueAtIndex(parts,0), STR("one"))) ok = false;
    if (!OCStringEqual((OCStringRef)OCArrayGetValueAtIndex(parts,1), STR("two"))) ok = false;
    if (!OCStringEqual((OCStringRef)OCArrayGetValueAtIndex(parts,2), STR("three"))) ok = false;
    OCRelease(ranges);
    OCRelease(parts);
    OCRelease(s);
    fprintf(stderr, "%s %s.\n", __func__, ok?"passed":"FAILED");
    return ok;
}


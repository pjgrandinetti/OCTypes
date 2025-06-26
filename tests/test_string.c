#include "test_string.h"
#include "../src/OCString.h"
#include "../src/OCMath.h"      // for OCComplexFromCString, OCCompareDoubleValues
#include <math.h>              // exp, log, acos, asin, cos, sin
#include <complex.h>           // creal, cimag, conj
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h> // Added for strcmp
#include <inttypes.h> // Provides PRIu64 and related macros

// ————————— existing tests —————————

// Test equality, immutability, and creation from C-string and STR
bool stringTest1(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = false;
    OCStringRef s1 = OCStringCreateWithCString("theStringWithAStringOnAStringTown");
    OCStringRef s_check = STR("theStringWithAStringOnAStringTown");

    // Test length, equality, and roundtrip
    if (!s1 || !s_check)                           goto cleanup;
    if (!OCStringEqual(s1, s_check)) {
        fprintf(stderr, "OCStringEqual failed on identical strings\n");
        goto cleanup;
    }
    if (OCStringGetLength(s1) != OCStringGetLength(s_check)) {
        fprintf(stderr, "Length mismatch: %llu != %llu\n",
                (unsigned long long)OCStringGetLength(s1),
                (unsigned long long)OCStringGetLength(s_check));
        goto cleanup;
    }
    // Round-trip copy and compare
    OCStringRef copy = OCStringCreateCopy(s1);
    if (!OCStringEqual(s1, copy)) {
        fprintf(stderr, "Copy is not equal to original!\n");
        OCRelease(copy);
        goto cleanup;
    }
    OCRelease(copy);

    // Test empty string
    OCStringRef s_empty = OCStringCreateWithCString("");
    OCStringRef s_empty2 = STR("");
    if (!OCStringEqual(s_empty, s_empty2)) {
        fprintf(stderr, "Empty string comparison failed\n");
        OCRelease(s_empty);
        OCRelease(s_empty2);
        goto cleanup;
    }
    OCRelease(s_empty);
    // s_empty2 is static, don't release

    success = true;
cleanup:
    if (s1) OCRelease(s1);
    if (!success) fprintf(stderr, "Test %s FAILED.\n", __func__);
    else         fprintf(stderr, "%s passed.\n", __func__);
    return success;
}

// Test format features, including %d, %f, %s, %@, and edge cases
bool stringTest2(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = false;
    OCStringRef out = NULL;

    // --- Test %@ with OCStringRef
    OCStringRef fmt1 = STR("Ix(%@)");
    OCStringRef out1 = OCStringCreateWithFormat(fmt1, STR("H2"));
    OCStringRef chk1 = STR("Ix(H2)");
    if (!OCStringEqual(out1, chk1)) {
        fprintf(stderr, "Format with %%@ failed: got [%s] expected [%s]\n",
                OCStringGetCString(out1), OCStringGetCString(chk1));
        goto cleanup;
    }

    // --- Test %s with C string
    OCStringRef fmt2 = STR("Ix(%s)");
    out = OCStringCreateWithFormat(fmt2, "H2");
    if (!OCStringEqual(out, chk1)) {
        fprintf(stderr, "Format with %%s failed: got [%s] expected [%s]\n",
                OCStringGetCString(out), OCStringGetCString(chk1));
        goto cleanup;
    }
    OCRelease(out); out = NULL;

    // --- Test mixed specifiers (%@, %d, %f)
    OCStringRef fmt3 = STR("Atom:%@, Index:%d, Mass:%.2f");
    out = OCStringCreateWithFormat(fmt3, STR("H2"), 42, 1.0079);
    OCStringRef chk3 = OCStringCreateWithCString("Atom:H2, Index:42, Mass:1.01");
    if (!OCStringEqual(out, chk3)) {
        fprintf(stderr, "Mixed format failed: got [%s] expected [%s]\n",
                OCStringGetCString(out), OCStringGetCString(chk3));
        OCRelease(chk3);
        goto cleanup;
    }
    OCRelease(chk3);
    OCRelease(out); out = NULL;

    // --- Test Unicode/UTF-8
    OCStringRef fmt4 = STR("Prefix-%@-Suffix");
    OCStringRef out4 = OCStringCreateWithFormat(fmt4, STR("é💧"));
    OCStringRef chk4 = OCStringCreateWithCString("Prefix-é💧-Suffix");
    if (!OCStringEqual(out4, chk4)) {
        fprintf(stderr, "Unicode format failed: got [%s] expected [%s]\n",
                OCStringGetCString(out4), OCStringGetCString(chk4));
        OCRelease(out4);
        OCRelease(chk4);
        goto cleanup;
    }
    OCRelease(out4); OCRelease(chk4);

    // --- Test edge case: empty format string
    OCStringRef emptyFmt = STR("");
    OCStringRef outEmpty = OCStringCreateWithFormat(emptyFmt);
    if (!OCStringEqual(outEmpty, emptyFmt)) {
        fprintf(stderr, "Empty format string failed\n");
        OCRelease(outEmpty);
        goto cleanup;
    }
    OCRelease(outEmpty);

    // --- Test error case: NULL OCStringRef as argument to %@
    OCStringRef fmt5 = STR("NullCheck:%@");
    out = OCStringCreateWithFormat(fmt5, (OCStringRef)NULL);
    OCStringRef chk5 = OCStringCreateWithCString("NullCheck:");
    if (!OCStringEqual(out, chk5)) {
        fprintf(stderr, "NULL OCStringRef in %%@ failed: got [%s] expected [%s]\n",
                OCStringGetCString(out), OCStringGetCString(chk5));
        OCRelease(chk5);
        goto cleanup;
    }
    OCRelease(chk5);
    OCRelease(out); out = NULL;

    // --- Test double %@
    OCStringRef fmt6 = STR("%@-%@");
    OCStringRef out6 = OCStringCreateWithFormat(fmt6, STR("A"), STR("B"));
    OCStringRef chk6 = STR("A-B");
    if (!OCStringEqual(out6, chk6)) {
        fprintf(stderr, "Double %%@ failed: got [%s] expected [%s]\n",
                OCStringGetCString(out6), OCStringGetCString(chk6));
        OCRelease(out6);
        goto cleanup;
    }
    OCRelease(out6);

    // --- Test mismatched argument types (should NOT crash, but should warn)
    // The following should be detected and handled gracefully (if you've added runtime checks):
    // out = OCStringCreateWithFormat(fmt1, 1234); // Invalid, would crash with current implementation

    success = true;
cleanup:
    if (out1) OCRelease(out1);
    if (out) OCRelease(out);
    if (!success) fprintf(stderr, "Test %s FAILED.\n", __func__);
    else         fprintf(stderr, "%s passed.\n", __func__);
    return success;
}

// --- More rigorous, order-mixed format argument tests ---

bool stringTest_mixed_format_specifiers(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool ok = false;
    OCStringRef fmt = NULL;
    OCStringRef out = NULL;
    OCStringRef chk = NULL;

    // Test interleaved %@ and printf specifiers (the dangerous case)
    fmt = STR("%@:%d:%@:%s:%@");
    out = OCStringCreateWithFormat(fmt, STR("A"), 11, STR("B"), "X", STR("C"));
    chk = OCStringCreateWithCString("A:11:B:X:C");
    if (!OCStringEqual(out, chk)) {
        fprintf(stderr, "Mixed (%%@ and printf) format failed: got [%s], expected [%s]\n",
                OCStringGetCString(out), OCStringGetCString(chk));
        goto cleanup;
    }
    OCRelease(out); out = NULL;
    OCRelease(chk); chk = NULL;

    // Test a chunk with multiple printf specifiers between %@ (will break if not parsed right)
    fmt = STR("%@:%d-%0.2f:%@");
    out = OCStringCreateWithFormat(fmt, STR("Q"), 99, 2.71, STR("W"));
    chk = OCStringCreateWithCString("Q:99-2.71:W");
    if (!OCStringEqual(out, chk)) {
        fprintf(stderr, "Multiple printf specifiers between %%@ failed: got [%s], expected [%s]\n",
                OCStringGetCString(out), OCStringGetCString(chk));
        goto cleanup;
    }
    OCRelease(out); out = NULL;
    OCRelease(chk); chk = NULL;

    // Test sequence with only printf specifiers, no %@ (should still work)
    fmt = STR("Val1:%d,Val2:%.1f,Val3:%s");
    out = OCStringCreateWithFormat(fmt, 123, 4.5, "hi");
    chk = OCStringCreateWithCString("Val1:123,Val2:4.5,Val3:hi");
    if (!OCStringEqual(out, chk)) {
        fprintf(stderr, "Printf-only format failed: got [%s], expected [%s]\n",
                OCStringGetCString(out), OCStringGetCString(chk));
        goto cleanup;
    }
    OCRelease(out); out = NULL;
    OCRelease(chk); chk = NULL;

    // Test out-of-order usage: printf, %@, printf, %@, printf
    fmt = STR("%d:%@:%f:%@:%s");
    out = OCStringCreateWithFormat(fmt, 1, STR("M"), 3.14, STR("N"), "str");
    chk = OCStringCreateWithCString("1:M:3.140000:N:str");
    if (!OCStringEqual(out, chk)) {
        fprintf(stderr, "Out-of-order %%@ and printf failed: got [%s], expected [%s]\n",
                OCStringGetCString(out), OCStringGetCString(chk));
        goto cleanup;
    }
    OCRelease(out); out = NULL;
    OCRelease(chk); chk = NULL;

    // Test %% and %@ next to each other
    fmt = STR("100%% %@!");
    out = OCStringCreateWithFormat(fmt, STR("bonus"));
    chk = OCStringCreateWithCString("100% bonus!");
    if (!OCStringEqual(out, chk)) {
        fprintf(stderr, "Percent/%%@ adjacent failed: got [%s], expected [%s]\n",
                OCStringGetCString(out), OCStringGetCString(chk));
        goto cleanup;
    }
    OCRelease(out); out = NULL;
    OCRelease(chk); chk = NULL;

    // Test many %@ in a row (should consume all)
    fmt = STR("%@%@%@");
    out = OCStringCreateWithFormat(fmt, STR("1"), STR("2"), STR("3"));
    chk = OCStringCreateWithCString("123");
    if (!OCStringEqual(out, chk)) {
        fprintf(stderr, "Consecutive %%@ failed: got [%s], expected [%s]\n",
                OCStringGetCString(out), OCStringGetCString(chk));
        goto cleanup;
    }
    OCRelease(out); out = NULL;
    OCRelease(chk); chk = NULL;

    // Test if passing NULL as OCStringRef to %@ doesn't crash, just skips
    fmt = STR("%@|%@|%@");
    out = OCStringCreateWithFormat(fmt, STR("X"), (OCStringRef)NULL, STR("Y"));
    chk = OCStringCreateWithCString("X| |Y");
    // Our logic expects "X||Y", so just ensure NULL does not crash and prints nothing for that slot.
    if (!OCStringEqual(out, STR("X||Y"))) {
        fprintf(stderr, "NULL to %%@ should print nothing: got [%s]\n", OCStringGetCString(out));
        goto cleanup;
    }
    OCRelease(out); out = NULL;
    OCRelease(chk); chk = NULL;

    // Test more arguments than specifiers (extra args ignored)
    fmt = STR("%@-%d-%@");
    out = OCStringCreateWithFormat(fmt, STR("A"), 8, STR("B"), STR("SHOULD_NOT_BE_USED"), 999);
    chk = OCStringCreateWithCString("A-8-B");
    if (!OCStringEqual(out, chk)) {
        fprintf(stderr, "Extra args present failed: got [%s], expected [%s]\n",
                OCStringGetCString(out), OCStringGetCString(chk));
        goto cleanup;
    }
    OCRelease(out); out = NULL;
    OCRelease(chk); chk = NULL;

    // Test fewer arguments than needed (should not crash, might print garbage)
    fmt = STR("%@-%d-%@");
    out = OCStringCreateWithFormat(fmt, STR("A"));
    if (out) { OCRelease(out); out = NULL; }

    fprintf(stderr, "stringTest_mixed_format_specifiers passed (if no crash).\n");
    ok = true;
cleanup:
    if (out) OCRelease(out);
    if (chk) OCRelease(chk);
    return ok;
}

bool test_OCStringAppendFormat(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool ok = false;
    OCMutableStringRef str = NULL;
    OCStringRef fmt = NULL;
    OCStringRef verify = NULL;

    // Start with an initial string
    str = OCStringCreateMutable(0);
    OCStringAppendCString(str, "START|");

    // Append an OCStringRef using %@ and test append behavior
    fmt = STR("%@|");
    OCStringAppendFormat(str, fmt, STR("SEG1"));
    verify = OCStringCreateWithCString("START|SEG1|");
    if (!OCStringEqual((OCStringRef)str, verify)) {
        fprintf(stderr, "OCStringAppendFormat (single %%@) failed: got [%s], expected [%s]\n",
                OCStringGetCString((OCStringRef)str), OCStringGetCString(verify));
        goto cleanup;
    }
    OCRelease(verify);

    // Append with multiple types: %d, %f, %s, %c, %% and pointer
    fmt = STR("%d-%0.1f-%s-%c-%%-%p|");
    int intval = 42;
    double fval = 3.14;
    const char *sval = "sVal";
    char cval = 'X';
    void *pval = (void*)0x1234;

    OCStringAppendFormat(str, fmt, intval, fval, sval, cval, pval);

    // Construct expected prefix (don't assume pointer formatting, only compare prefix up to pointer)
    char buf[256];
    snprintf(buf, sizeof(buf), "START|SEG1|42-3.1-sVal-X-%%-");
    const char *str_c = OCStringGetCString((OCStringRef)str);
    if (strncmp(str_c, buf, strlen(buf)) != 0) {
        fprintf(stderr, "OCStringAppendFormat (multi type) failed:\n  got:      [%s]\n  expected: [%s...]\n",
                str_c, buf);
        goto cleanup;
    }

    // Append with double %@ and check append order (only check trailing pattern)
    fmt = STR("%@-%@|");
    OCStringAppendFormat(str, fmt, STR("A"), STR("B"));
    str_c = OCStringGetCString((OCStringRef)str);
    if (!strstr(str_c, "A-B|")) {
        fprintf(stderr, "OCStringAppendFormat (double %%@) failed: missing [A-B|] in [%s]\n", str_c);
        goto cleanup;
    }

    // Append with Unicode
    fmt = STR("💧%@💧|");
    OCStringAppendFormat(str, fmt, STR("Z"));
    str_c = OCStringGetCString((OCStringRef)str);
    if (!strstr(str_c, "💧Z💧|")) {
        fprintf(stderr, "OCStringAppendFormat (unicode) failed: missing [💧Z💧|] in [%s]\n", str_c);
        goto cleanup;
    }

    // Append with literal percent
    fmt = STR("100%%|");
    OCStringAppendFormat(str, fmt);
    str_c = OCStringGetCString((OCStringRef)str);
    if (!strstr(str_c, "100%|")) {
        fprintf(stderr, "OCStringAppendFormat (literal %%) failed: missing [100%%|] in [%s]\n", str_c);
        goto cleanup;
    }

    // Append with NULL OCStringRef for %@ (should append nothing, not crash)
    fmt = STR("NULL-%@|");
    OCStringAppendFormat(str, fmt, (OCStringRef)NULL);
    str_c = OCStringGetCString((OCStringRef)str);
    if (!strstr(str_c, "NULL-|")) {
        fprintf(stderr, "OCStringAppendFormat (NULL %%@) failed: missing [NULL-|] in [%s]\n", str_c);
        goto cleanup;
    }

    // Final check: count '|' pieces to ensure string grew as expected (at least 7 pieces)
    size_t expected_pieces = 7;
    size_t found = 0;
    for (const char *p = str_c; (p = strchr(p, '|')) != NULL; ++p, ++found) { }
    if (found < expected_pieces) {
        fprintf(stderr, "OCStringAppendFormat (pieces) mismatch: expected >=%zu, found %zu\n", expected_pieces, found);
        goto cleanup;
    }

    ok = true;
cleanup:
    if (str) OCRelease(str);
    if (!ok) fprintf(stderr, "Test %s FAILED.\n", __func__);
    else     fprintf(stderr, "%s passed.\n", __func__);
    return ok;
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
    // index 6 → '•' (U+2022)
    uint32_t char_at_6 = OCStringGetCharacterAtIndex(s,6);
    if (char_at_6 != 0x2022) { // Check full Unicode code point for '•'
        fprintf(stderr, "stringTest3: Character at index 6 is not U+2022. Expected 0x2022, Got 0x%X\n", char_at_6);
        ok = false;
    }
    OCRelease(s);
    fprintf(stderr, "%s %s.\n", __func__, ok?"passed":"FAILED");
    return ok;
}

bool stringTest4(void) {
    fprintf(stderr, "%s begin…\n", __func__);
    bool ok = true;
    OCStringRef s = OCStringCreateWithCString("foo•bar•baz");
    // substring for the first bullet '•' which is at index 3
    OCRange r = OCRangeMake(3,1); // Index of '•' in "foo•bar•baz"
    OCStringRef sub = OCStringCreateWithSubstring(s, r);
    if (!OCStringEqual(sub, STR("•"))) {
        fprintf(stderr, "stringTest4: Substring did not extract '•'. Got '%s'\n", OCStringGetCString(sub));
        ok = false;
    }
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
    if (!m) {
        fprintf(stderr, "stringTest6: OCStringCreateMutableCopy failed.\n");
        return false; // nothing to release here
    }

    // Delete the middle bullet (at index 1: '•')
    OCStringDelete(m, OCRangeMake(1,1));
    if (!OCStringEqual((OCStringRef)m, STR("ab•c"))) {
        fprintf(stderr, "stringTest6: Delete failed. Expected 'ab•c', got '%s'\n",
                OCStringGetCString((OCStringRef)m));
        ok = false;
    }

    // Insert a unicode snowman at index 1
    OCStringInsert(m, 1, STR("☃"));
    if (!OCStringEqual((OCStringRef)m, STR("a☃b•c"))) {
        fprintf(stderr, "stringTest6: Insert failed. Expected 'a☃b•c', got '%s'\n",
                OCStringGetCString((OCStringRef)m));
        ok = false;
    }

    // Replace "☃" with "🌟"
    OCStringReplace(m, OCRangeMake(1,1), STR("🌟"));
    if (!OCStringEqual((OCStringRef)m, STR("a🌟b•c"))) {
        fprintf(stderr, "stringTest6: Replace failed. Expected 'a🌟b•c', got '%s'\n",
                OCStringGetCString((OCStringRef)m));
        ok = false;
    }

    // Replace all contents with "B"
    OCStringReplaceAll(m, STR("B"));
    if (!OCStringEqual((OCStringRef)m, STR("B"))) {
        fprintf(stderr, "stringTest6: ReplaceAll failed. Expected 'B', got '%s'\n",
                OCStringGetCString((OCStringRef)m));
        ok = false;
    }

    // Now finally release the mutable copy
    OCRelease(m);

    fprintf(stderr, "%s %s.\n", __func__, ok ? "passed" : "FAILED");
    return ok;
}

bool stringTest7(void) {
    fprintf(stderr, "%s begin…\n", __func__);
    bool ok = true;
    OCStringRef s = OCStringCreateWithCString("one•two•three");
    if (!s) {
        fprintf(stderr, "stringTest7: OCStringCreateWithCString failed to create 's'.\n");
        return false; // Cannot proceed
    }

    // find ranges
    fprintf(stderr, "  stringTest7: Testing OCStringCreateArrayWithFindResults...\n");
    OCArrayRef ranges = OCStringCreateArrayWithFindResults(s, STR("•"), OCRangeMake(0, OCStringGetLength(s)), 0);
    if (!ranges) {
        fprintf(stderr, "stringTest7: OCStringCreateArrayWithFindResults returned NULL for ranges.\n");
        ok = false;
    } else if (OCArrayGetCount(ranges) != 2) {
        fprintf(stderr, "stringTest7: Expected 2 ranges, got  %" PRIu64 ".\n", OCArrayGetCount(ranges));
        ok = false;
    } else {
        // Optional: Check the actual ranges if needed
        // OCRange* r0 = (OCRange*)OCArrayGetValueAtIndex(ranges, 0);
        // OCRange* r1 = (OCRange*)OCArrayGetValueAtIndex(ranges, 1);
        // if (!(r0 && r0->location == 3 && r0->length == 1)) { fprintf(stderr, "Range 0 incorrect\n"); ok = false; }
        // if (!(r1 && r1->location == 7 && r1->length == 1)) { fprintf(stderr, "Range 1 incorrect\n"); ok = false; }
        fprintf(stderr, "  stringTest7: OCStringCreateArrayWithFindResults seems OK (count =  %" PRIu64 ").\n", OCArrayGetCount(ranges));
    }

    // split on bullet
    fprintf(stderr, "  stringTest7: Testing OCStringCreateArrayBySeparatingStrings...\n");
    OCArrayRef parts = OCStringCreateArrayBySeparatingStrings(s, STR("•"));
    if (!parts) {
        fprintf(stderr, "stringTest7: OCStringCreateArrayBySeparatingStrings returned NULL for parts.\n");
        ok = false;
    } else if (OCArrayGetCount(parts) != 3) {
        fprintf(stderr, "stringTest7: Expected 3 parts, got  %" PRIu64 ".\n", OCArrayGetCount(parts));
        ok = false;
    } else {
        fprintf(stderr, "  stringTest7: OCStringCreateArrayBySeparatingStrings count seems OK (count =  %" PRIu64 ").\n", OCArrayGetCount(parts));
        // check content
        OCStringRef part0 = (OCStringRef)OCArrayGetValueAtIndex(parts, 0);
        OCStringRef part1 = (OCStringRef)OCArrayGetValueAtIndex(parts, 1);
        OCStringRef part2 = (OCStringRef)OCArrayGetValueAtIndex(parts, 2);

        if (!part0 || !OCStringEqual(part0, STR("one"))) {
            fprintf(stderr, "stringTest7: Part 0 mismatch. Expected 'one', got '%s'.\n", part0 ? OCStringGetCString(part0) : "NULL");
            ok = false;
        }
        if (!part1 || !OCStringEqual(part1, STR("two"))) {
            fprintf(stderr, "stringTest7: Part 1 mismatch. Expected 'two', got '%s'.\n", part1 ? OCStringGetCString(part1) : "NULL");
            ok = false;
        }
        if (!part2 || !OCStringEqual(part2, STR("three"))) {
            fprintf(stderr, "stringTest7: Part 2 mismatch. Expected 'three', got '%s'.\n", part2 ? OCStringGetCString(part2) : "NULL");
            ok = false;
        }
    }

    if (ranges) OCRelease(ranges);
    if (parts) OCRelease(parts);
    OCRelease(s);
    fprintf(stderr, "%s %s.\n", __func__, ok?"passed":"FAILED");
    return ok;
}



// Helper sub-tests for stringTest8
static bool test_CreateAndGetCString(void);
static bool test_MutableAppend(void);
static bool test_MutableCopy(void);
static bool test_CompareAndCase(void);
static bool test_TrimFunctions(void);
static bool test_FindOperations(void);
static bool test_FindAndReplace(void);
static bool test_CharUtilities(void);
static bool test_FloatStringifiers(void);
static bool test_ComplexStringifiers(void);

static bool test_CreateAndGetCString(void) {
    fprintf(stderr, "  [Create/GetCString]...\n");
    bool ok = true;
    const char* input = "Hello, C-String!";
    OCStringRef s = OCStringCreateWithCString(input);
    if (!s) { fprintf(stderr, "    ERROR: createWithCString failed\n"); return false; }
    if (strcmp(input, OCStringGetCString(s)) != 0) {
        fprintf(stderr, "    ERROR: round-trip C-string mismatch\n");
        ok = false;
    }
    OCRelease(s);
    return ok;
}

static bool test_MutableAppend(void) {
    fprintf(stderr, "  [MutableAppend]...\n");
    bool ok = true;
    OCMutableStringRef m = OCStringCreateMutable(10);
    if (!m) { fprintf(stderr, "    ERROR: createMutable failed\n"); return false; }
    OCStringAppendCString(m, "Part1");
    if (OCStringGetLength((OCStringRef)m) != 5) {
        fprintf(stderr, "    ERROR: length after Part1\n"); ok = false;
    }
    OCStringAppendCString(m, " Part2");
    if (strcmp(OCStringGetCString((OCStringRef)m), "Part1 Part2") != 0) {
        fprintf(stderr, "    ERROR: content after append\n"); ok = false;
    }
    OCRelease(m);
    return ok;
}

static bool test_MutableCopy(void) {
    fprintf(stderr, "  [MutableCopy]...\n");
    bool ok = true;
    OCStringRef original = STR("Original");
    OCMutableStringRef copy = OCStringCreateMutableCopy(original);
    if (!copy) { fprintf(stderr, "    ERROR: createMutableCopy failed\n"); return false; }
    if (!OCStringEqual((OCStringRef)copy, original)) {
        fprintf(stderr, "    ERROR: copy != original\n"); ok = false;
    }
    OCStringAppendCString(copy, " Modified");
    if (OCStringEqual((OCStringRef)copy, original)) {
        fprintf(stderr, "    ERROR: modification affected original\n"); ok = false;
    }
    OCRelease(copy);
    return ok;
}

static bool test_CompareAndCase(void) {
    fprintf(stderr, "  [Compare/Case]...\n");
    bool ok = true;
    OCStringRef lower = STR("apple"), upper = STR("APPLE"), banana = STR("banana");
    if (OCStringCompare(lower, upper, 0) == kOCCompareEqualTo) {
        fprintf(stderr, "    ERROR: case-sensitive compare equal\n"); ok = false;
    }
    if (OCStringCompare(lower, upper, kOCCompareCaseInsensitive) != kOCCompareEqualTo) {
        fprintf(stderr, "    ERROR: case-insensitive compare\n"); ok = false;
    }
    if (OCStringCompare(lower, banana, 0) != kOCCompareLessThan) {
        fprintf(stderr, "    ERROR: apple vs banana ordering\n"); ok = false;
    }
    return ok;
}

static bool test_TrimFunctions(void) {
    fprintf(stderr, "  [TrimFunctions]...\n");
    bool ok = true;

    OCMutableStringRef ws = OCMutableStringCreateWithCString("  MiXeD CaSe & Spaces  ");
    if (!ws) { fprintf(stderr, "    ERROR: createMutableCString failed\n"); return false; }
    OCStringLowercase(ws);
    OCStringUppercase(ws);
    OCStringTrimWhitespace(ws);
    if (!OCStringEqual((OCStringRef)ws, STR("MIXED CASE & SPACES"))) {
        fprintf(stderr, "    ERROR: trim/format functions failed\n"); ok = false;
    }
    OCRelease(ws);

    OCMutableStringRef p1 = OCMutableStringCreateWithCString("(content)");
    OCMutableStringRef p2 = OCMutableStringCreateWithCString("no parens");
    OCMutableStringRef p3 = OCMutableStringCreateWithCString("(mismatched");
    OCMutableStringRef p4 = OCMutableStringCreateWithCString("mismatched)");
    OCMutableStringRef p5 = OCMutableStringCreateWithCString("((double))");
    if (!OCStringTrimMatchingParentheses(p1) ||
        !OCStringEqual((OCStringRef)p1, STR("content"))) {
        fprintf(stderr, "    ERROR: trimMatching fail p1\n"); ok = false;
    }
    if (OCStringTrimMatchingParentheses(p2) ||
        !OCStringEqual((OCStringRef)p2, STR("no parens"))) {
        fprintf(stderr, "    ERROR: trimMatching fail p2\n"); ok = false;
    }
    if (OCStringTrimMatchingParentheses(p3) ||
        !OCStringEqual((OCStringRef)p3, STR("(mismatched"))) {
        fprintf(stderr, "    ERROR: trimMatching fail p3\n"); ok = false;
    }
    if (OCStringTrimMatchingParentheses(p4) ||
        !OCStringEqual((OCStringRef)p4, STR("mismatched)"))) {
        fprintf(stderr, "    ERROR: trimMatching fail p4\n"); ok = false;
    }
    if (!OCStringTrimMatchingParentheses(p5) ||
        !OCStringEqual((OCStringRef)p5, STR("(double)"))) {
        fprintf(stderr, "    ERROR: trimMatching fail p5\n"); ok = false;
    }
    OCRelease(p1); OCRelease(p2); OCRelease(p3); OCRelease(p4); OCRelease(p5);

    return ok;
}

static bool test_FindOperations(void) {
    fprintf(stderr, "  [FindOperations]...\n");
    bool ok = true;
    OCStringRef hay = STR("One two three two one"), needle = STR("two");
    OCRange r1 = OCStringFind(hay, needle, 0);
    if (r1.location != 4 || r1.length != 3) {
        fprintf(stderr, "    ERROR: first find\n"); ok = false;
    }
    OCStringRef sub = OCStringCreateWithSubstring(hay, (OCRange){
        r1.location + r1.length,
        OCStringGetLength(hay) - (r1.location + r1.length)
    });
    OCRange r2 = OCStringFind(sub, needle, 0);
    if (r2.location != 7 || r2.length != 3) {
        fprintf(stderr, "    ERROR: second find\n"); ok = false;
    }
    OCRelease(sub);
    OCRange rc = OCStringFind(hay, STR("TWO"), kOCCompareCaseInsensitive);
    if (rc.location != 4 || rc.length != 3) {
        fprintf(stderr, "    ERROR: ci find\n"); ok = false;
    }
    OCRange rf = OCStringFind(hay, STR("four"), 0);
    if (rf.location != kOCNotFound || rf.length != 0) {
        fprintf(stderr, "    ERROR: not found\n"); ok = false;
    }
    return ok;
}

static bool test_FindAndReplace(void) {
    fprintf(stderr, "  [FindAndReplace]...\n");
    bool ok = true;
    OCMutableStringRef s = OCMutableStringCreateWithCString("cat dog cat bird cat");
    int64_t cnt = OCStringFindAndReplace2(s, STR("cat"), STR("mouse"));
    if (cnt != 3 || !OCStringEqual((OCStringRef)s, STR("mouse dog mouse bird mouse"))) {
        fprintf(stderr, "    ERROR: findAndReplace2\n"); ok = false;
    }
    OCRelease(s);
    return ok;
}

static bool test_CharUtilities(void) {
    fprintf(stderr, "  [CharUtils]...\n");
    bool ok = true;
    if (!characterIsUpperCaseLetter('A') || characterIsUpperCaseLetter('a')) {
        fprintf(stderr, "    ERROR: upperCase\n"); ok = false;
    }
    if (!characterIsLowerCaseLetter('z') || characterIsLowerCaseLetter('Z')) {
        fprintf(stderr, "    ERROR: lowerCase\n"); ok = false;
    }
    if (!characterIsDigitOrDecimalPoint('5') || !characterIsDigitOrDecimalPoint('.')) {
        fprintf(stderr, "    ERROR: digitOrPoint\n"); ok = false;
    }
    if (characterIsDigitOrDecimalPoint('x')) {
        fprintf(stderr, "    ERROR: digitOrPoint wrong\n"); ok = false;
    }
    if (!characterIsDigitOrDecimalPointOrSpace(' ') ||
        characterIsDigitOrDecimalPointOrSpace('\t')) {
        fprintf(stderr, "    ERROR: digitOrSpace\n"); ok = false;
    }
    return ok;
}

static bool test_FloatStringifiers(void) {
    fprintf(stderr, "  [FloatStrings]...\n");
    bool ok = true;
    OCStringRef f = OCFloatCreateStringValue(3.14159f);
    if (OCStringFind(f, STR("3.14"), 0).length == 0) {
        fprintf(stderr, "    ERROR: floatStr\n"); ok = false;
    }
    OCRelease(f);
    OCStringRef d = OCDoubleCreateStringValue(2.718281828);
    if (OCStringFind(d, STR("2.71828"), 0).length == 0) {
        fprintf(stderr, "    ERROR: doubleStr\n"); ok = false;
    }
    OCRelease(d);
    return ok;
}

static bool test_ComplexStringifiers(void) {
    fprintf(stderr, "  [ComplexStrings]...\n");
    bool ok = true;
    float complex fc = 1.0f + 2.0f * I;
    OCStringRef s1 = OCFloatComplexCreateStringValue(fc, STR("%.1f%+.1fi"));
    if (!OCStringEqual(s1, STR("1.0+2.0i"))) {
        fprintf(stderr, "    ERROR: floatComplex\n"); ok = false;
    }
    OCRelease(s1);
    double complex dc = 3.0 - 4.0 * I;
    OCStringRef s2 = OCDoubleComplexCreateStringValue(dc, STR("%.1f%+.1fi"));
    if (!OCStringEqual(s2, STR("3.0-4.0i"))) {
        fprintf(stderr, "    ERROR: doubleComplex\n"); ok = false;
    }
    OCRelease(s2);
    return ok;
}

bool stringTest8(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool ok = true;
    ok &= test_CreateAndGetCString();
    ok &= test_MutableAppend();
    ok &= test_MutableCopy();
    ok &= test_CompareAndCase();
    ok &= test_TrimFunctions();
    ok &= test_FindOperations();
    ok &= test_FindAndReplace();
    ok &= test_CharUtilities();
    ok &= test_FloatStringifiers();
    ok &= test_ComplexStringifiers();
    fprintf(stderr, "%s %s.\n", __func__, ok ? "passed" : "FAILED");
    return ok;
}

bool stringTest9(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool ok = true;
    
    // ------ Part 1: Advanced UTF-8 handling tests ------
    fprintf(stderr, "  Testing advanced UTF-8 handling...\n");
    
    // Test string with multiple complex Unicode code points: Emojis, combining marks, etc.
    const char* complex_utf8 = "😀👨‍👩‍👧‍👦🇺🇸é̀́";
    OCStringRef utf8_str = OCStringCreateWithCString(complex_utf8);
    if (!utf8_str) {
        fprintf(stderr, "stringTest9: OCStringCreateWithCString failed for complex UTF-8 string.\n");
        return false;
    }
    
    // Verify correct code point count (hard-coded expected value)
    // 😀(1) 👨‍👩‍👧‍👦(1 since it's a ZWJ sequence) 🇺🇸(1) é̀́(1) = 4 code points
    uint64_t expected_code_points = 4;
    if (OCStringGetLength(utf8_str) != expected_code_points) {
        fprintf(stderr, "stringTest9: Incorrect code point count. Expected  %" PRIu64 ", got  %" PRIu64 ".\n", 
                expected_code_points, OCStringGetLength(utf8_str));
        ok = false;
    }
    
    // Test Unicode code point access - verify emoji code point (U+1F600)
    uint32_t emoji_cp = OCStringGetCharacterAtIndex(utf8_str, 0);
    if (emoji_cp != 0x1F600) {
        fprintf(stderr, "stringTest9: Incorrect emoji code point. Expected 0x1F600, got 0x%X.\n", emoji_cp);
        ok = false;
    }
    
    OCRelease(utf8_str);
    
    // ------ Part 2: Edge cases for string manipulation ------
    fprintf(stderr, "  Testing edge cases for string manipulation...\n");
    
    // Empty string tests
    OCStringRef empty_str = OCStringCreateWithCString("");
    if (OCStringGetLength(empty_str) != 0) {
        fprintf(stderr, "stringTest9: Empty string should have length 0, got  %" PRIu64 ".\n", OCStringGetLength(empty_str));
        ok = false;
    }
    
    // Test substring of empty string
    OCStringRef empty_substring = OCStringCreateWithSubstring(empty_str, OCRangeMake(0, 0));
    if (!empty_substring || OCStringGetLength(empty_substring) != 0) {
        fprintf(stderr, "stringTest9: Substring of empty string failed.\n");
        ok = false;
    }
    
    // Test edge cases for string splitting
    OCArrayRef empty_parts = OCStringCreateArrayBySeparatingStrings(empty_str, STR(","));
    if (!empty_parts) {
        fprintf(stderr, "stringTest9: OCStringCreateArrayBySeparatingStrings failed for empty string.\n");
        ok = false;
    } else {
        // Should get array with one empty string
        uint64_t count = OCArrayGetCount(empty_parts);
        if (count != 1) {
            fprintf(stderr, "stringTest9: Splitting empty string should result in array with 1 element, got  %" PRIu64 ".\n", count);
            ok = false;
        }
        OCRelease(empty_parts);
    }
    
    // Test splitting with empty separator (should return array with single element)
    OCArrayRef parts_empty_sep = OCStringCreateArrayBySeparatingStrings(STR("test"), STR(""));
    if (!parts_empty_sep) {
        fprintf(stderr, "stringTest9: OCStringCreateArrayBySeparatingStrings failed for empty separator.\n");
        ok = false;
    } else {
        uint64_t count = OCArrayGetCount(parts_empty_sep);
        if (count != 1) {
            fprintf(stderr, "stringTest9: Splitting with empty separator should return array with 1 element, got  %" PRIu64 ".\n", count);
            ok = false;
        }
        OCRelease(parts_empty_sep);
    }
    
    OCRelease(empty_substring);
    OCRelease(empty_str);
    
    // ------ Part 3: Separator at beginning/end edge cases ------
    fprintf(stderr, "  Testing separators at beginning/end edge cases...\n");
    
    // String with separator at beginning and end: ";one;two;"
    OCStringRef edge_str = OCStringCreateWithCString(";one;two;");
    OCArrayRef edge_parts = OCStringCreateArrayBySeparatingStrings(edge_str, STR(";"));
    
    if (!edge_parts) {
        fprintf(stderr, "stringTest9: OCStringCreateArrayBySeparatingStrings failed for edge_str.\n");
        ok = false;
    } else {
        // Should have 4 parts: "", "one", "two", ""
        uint64_t count = OCArrayGetCount(edge_parts);
        if (count != 4) {
            fprintf(stderr, "stringTest9: Expected 4 parts from ';one;two;', got  %" PRIu64 ".\n", count);
            ok = false;
        } else {
            OCStringRef part0 = (OCStringRef)OCArrayGetValueAtIndex(edge_parts, 0);
            OCStringRef part1 = (OCStringRef)OCArrayGetValueAtIndex(edge_parts, 1);
            OCStringRef part2 = (OCStringRef)OCArrayGetValueAtIndex(edge_parts, 2);
            OCStringRef part3 = (OCStringRef)OCArrayGetValueAtIndex(edge_parts, 3);
            
            if (!OCStringEqual(part0, STR("")) || 
                !OCStringEqual(part1, STR("one")) || 
                !OCStringEqual(part2, STR("two")) || 
                !OCStringEqual(part3, STR(""))) {
                fprintf(stderr, "stringTest9: Incorrect parts for ';one;two;'\n");
                ok = false;
            }
        }
        OCRelease(edge_parts);
    }
    OCRelease(edge_str);
    
    // ------ Part 4: Boundary/Large string tests ------
    fprintf(stderr, "  Testing with large strings...\n");
    
    // Test with larger string
    const size_t LARGE_SIZE = 1000;
    char* large_buf = malloc(LARGE_SIZE + 1);
    if (large_buf) {
        // Fill with repeating pattern
        for (size_t i = 0; i < LARGE_SIZE; i++) {
            large_buf[i] = 'a' + (i % 26);
        }
        large_buf[LARGE_SIZE] = '\0';
        
        OCStringRef large_str = OCStringCreateWithCString(large_buf);
        if (!large_str) {
            fprintf(stderr, "stringTest9: OCStringCreateWithCString failed for large string.\n");
            ok = false;
        } else {
            if (OCStringGetLength(large_str) != LARGE_SIZE) {
                fprintf(stderr, "stringTest9: Incorrect length for large string. Expected %zu, got  %" PRIu64 ".\n", 
                        LARGE_SIZE, OCStringGetLength(large_str));
                ok = false;
            }
            
            // Test substring with boundary conditions
            OCStringRef sub_mid = OCStringCreateWithSubstring(large_str, OCRangeMake(500, 100));
            if (!sub_mid) {
                fprintf(stderr, "stringTest9: Substring creation failed for large string.\n");
                ok = false;
            } else {
                if (OCStringGetLength(sub_mid) != 100) {
                    fprintf(stderr, "stringTest9: Incorrect substring length. Expected 100, got  %" PRIu64 ".\n", 
                            OCStringGetLength(sub_mid));
                    ok = false;
                }
                OCRelease(sub_mid);
            }
            
            OCRelease(large_str);
        }
        
        free(large_buf);
    } else {
        fprintf(stderr, "stringTest9: Failed to allocate buffer for large string test.\n");
        ok = false;
    }
    
    fprintf(stderr, "%s %s.\n", __func__, ok ? "passed" : "FAILED");
    return ok;
}


bool stringTest10(void) {
    printf("Running test_string_split_no_separator...\n");

    OCArrayRef arr = OCStringCreateArrayBySeparatingStrings(STR("8"), STR(".."));
    assert(arr != NULL);

    // Should only get back one element, the original string "8"
    assert(OCArrayGetCount(arr) == 1);

    OCStringRef s0 = OCArrayGetValueAtIndex(arr, 0);
    // Compare the returned OCStringRef to "8" with case-sensitive comparison
    assert(OCStringCompare(s0, STR("8"), 0) == kOCCompareEqualTo);

    OCRelease(arr);
    printf("test_string_split_no_separator passed\n");
    return true;
}

bool stringTest11(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool ok = true;
    
    OCMutableStringRef mutString = OCStringCreateMutableCopy(STR("•×÷−\n+μγºh_pɣ√∛∜ "));
    // Each replacement uses the current full range of the string
    OCRange range;
    range.location = 0;
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("ɣ"), STR("𝛾"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("\n"), STR(""), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("•"), STR("*"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("×"), STR("*"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("÷"), STR("/"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("−"), STR("-"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("+"), STR("+"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("μ"), STR("µ"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("γ"), STR("𝛾"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("º"), STR("°"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("h_p"), STR("h_P"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("√"), STR("sqrt"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("∛"), STR("cbrt"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR("∜"), STR("qtrt"), range, 0);
    range.length = OCStringGetLength((OCStringRef)mutString);
    OCStringFindAndReplace(mutString, STR(" "), STR(""), range, 0);

    OCStringRef result = (OCStringRef)mutString;
    OCStringShow(result);printf("\n");
    assert(OCStringCompare(result, STR("**/-+µ𝛾°h_P𝛾sqrtcbrtqtrt"), 0) == kOCCompareEqualTo);

    OCRelease(mutString);
    fprintf(stderr, "%s %s.\n", __func__, ok ? "passed" : "FAILED");
    return ok;
}


bool stringTest_deepcopy(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool ok = true;

    OCStringRef original = STR("deepcopy test ✓");
    OCStringRef copy = (OCStringRef)OCTypeDeepCopy(original);

    if (!copy) {
        fprintf(stderr, "ERROR: OCTypeDeepCopy returned NULL\n");
        return false;
    }

    // Equal in value but not pointer
    if (!OCTypeEqual(original, copy)) {
        fprintf(stderr, "ERROR: Deep copy not equal to original\n");
        ok = false;
    }
    if (original == copy) {
        fprintf(stderr, "ERROR: Deep copy is pointer-identical (shallow)\n");
        ok = false;
    }

    OCRelease(copy);

    // Test mutable copy
    OCMutableStringRef mut_copy = (OCMutableStringRef)OCTypeDeepCopyMutable(original);
    if (!mut_copy) {
        fprintf(stderr, "ERROR: OCTypeDeepCopyMutable returned NULL\n");
        return false;
    }

    if (!OCTypeEqual(original, mut_copy)) {
        fprintf(stderr, "ERROR: Mutable deep copy not equal to original\n");
        ok = false;
    }

    // Modify the mutable copy and ensure original is unaffected
    OCStringAppendCString(mut_copy, "!");
    if (OCTypeEqual(original, mut_copy)) {
        fprintf(stderr, "ERROR: Original affected by mutable copy modification\n");
        ok = false;
    }

    OCRelease(mut_copy);
    fprintf(stderr, "%s %s.\n", __func__, ok ? "passed" : "FAILED");
    return ok;
}

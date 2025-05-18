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
        return false; // Cannot proceed
    }

    // Delete the middle bullet (at index 1: '•')
    // String: "a•b•c" -> delete char at index 1 -> "ab•c"
    OCStringDelete(m, OCRangeMake(1,1));
    if (!OCStringEqual((OCStringRef)m, STR("ab•c"))) {
        fprintf(stderr, "stringTest6: Delete failed. Expected 'ab•c', got '%s'\n", OCStringGetCString((OCStringRef)m));
        ok = false;
    }

    // Insert a unicode snowman "☃" at index 1
    // String: "ab•c" -> insert "☃" at index 1 -> "a☃b•c"
    OCStringInsert(m, 1, STR("☃"));
    if (!OCStringEqual((OCStringRef)m, STR("a☃b•c"))) {
        fprintf(stderr, "stringTest6: Insert failed. Expected 'a☃b•c', got '%s'\n", OCStringGetCString((OCStringRef)m));
        ok = false;
    }

    // Replace "☃" (at index 1, length 1) with "🌟" (star)
    // String: "a☃b•c" -> replace "☃" with "🌟" -> "a🌟b•c"
    OCStringReplace(m, OCRangeMake(1,1), STR("🌟"));
    if (!OCStringEqual((OCStringRef)m, STR("a🌟b•c"))) {
        fprintf(stderr, "stringTest6: Replace failed. Expected 'a🌟b•c', got '%s'\n", OCStringGetCString((OCStringRef)m));
        ok = false;
    }

    // ReplaceAll contents of m with "B"
    // String: "a🌟b•c" -> replace all with "B" -> "B"
    OCStringReplaceAll(m, STR("B"));
    if (!OCStringEqual((OCStringRef)m, STR("B"))) {
        fprintf(stderr, "stringTest6: ReplaceAll failed. Expected 'B', got '%s'\n", OCStringGetCString((OCStringRef)m));
        ok = false;
    }
    OCRelease(m);
    fprintf(stderr, "%s %s.\n", __func__, ok?"passed":"FAILED");
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
        fprintf(stderr, "stringTest7: Expected 2 ranges, got %llu.\n", OCArrayGetCount(ranges));
        ok = false;
    } else {
        // Optional: Check the actual ranges if needed
        // OCRange* r0 = (OCRange*)OCArrayGetValueAtIndex(ranges, 0);
        // OCRange* r1 = (OCRange*)OCArrayGetValueAtIndex(ranges, 1);
        // if (!(r0 && r0->location == 3 && r0->length == 1)) { fprintf(stderr, "Range 0 incorrect\n"); ok = false; }
        // if (!(r1 && r1->location == 7 && r1->length == 1)) { fprintf(stderr, "Range 1 incorrect\n"); ok = false; }
        fprintf(stderr, "  stringTest7: OCStringCreateArrayWithFindResults seems OK (count = %llu).\n", OCArrayGetCount(ranges));
    }

    // split on bullet
    fprintf(stderr, "  stringTest7: Testing OCStringCreateArrayBySeparatingStrings...\n");
    OCArrayRef parts = OCStringCreateArrayBySeparatingStrings(s, STR("•"));
    if (!parts) {
        fprintf(stderr, "stringTest7: OCStringCreateArrayBySeparatingStrings returned NULL for parts.\n");
        ok = false;
    } else if (OCArrayGetCount(parts) != 3) {
        fprintf(stderr, "stringTest7: Expected 3 parts, got %llu.\n", OCArrayGetCount(parts));
        ok = false;
    } else {
        fprintf(stderr, "  stringTest7: OCStringCreateArrayBySeparatingStrings count seems OK (count = %llu).\n", OCArrayGetCount(parts));
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

bool stringTest8(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool ok = true;

    fprintf(stderr, "  Testing OCStringCreateWithCString and OCStringGetCString...\n");
    // Test OCStringCreateWithCString and OCStringGetCString
    const char* cstr_in = "Hello, C-String!";
    OCStringRef s_from_cstr = OCStringCreateWithCString(cstr_in);
    if (!s_from_cstr) {
        fprintf(stderr, "stringTest8: OCStringCreateWithCString failed.\n");
        return false;
    }
    const char* cstr_out = OCStringGetCString(s_from_cstr);
    if (strcmp(cstr_in, cstr_out) != 0) {
        fprintf(stderr, "stringTest8: OCStringGetCString did not return the original C-string.\n");
        ok = false;
    }
    OCRelease(s_from_cstr);

    fprintf(stderr, "  Testing OCStringCreateMutable and OCStringAppendCString...\n");
    // Test OCStringCreateMutable and OCStringAppendCString
    OCMutableStringRef mut_s = OCStringCreateMutable(10);
    if (!mut_s) {
        fprintf(stderr, "stringTest8: OCStringCreateMutable failed.\n");
        return false;
    }
    OCStringAppendCString(mut_s, "Part1");
    if (OCStringGetLength((OCStringRef)mut_s) != 5) { // "Part1" has length 5
        fprintf(stderr, "stringTest8: OCStringAppendCString (Part1) length check failed. Expected 5, got %llu\n", OCStringGetLength((OCStringRef)mut_s));
        ok = false;
    }
    OCStringAppendCString(mut_s, " Part2");
    const char* appended_cstr = OCStringGetCString((OCStringRef)mut_s);
    if (strcmp(appended_cstr, "Part1 Part2") != 0) {
        fprintf(stderr, "stringTest8: OCStringAppendCString failed. Expected 'Part1 Part2', got '%s'\n", appended_cstr);
        ok = false;
    }
    if (OCStringGetLength((OCStringRef)mut_s) != 11) { // "Part1 Part2" has length 11
        fprintf(stderr, "stringTest8: OCStringAppendCString (Part1 Part2) length check failed. Expected 11, got %llu\n", OCStringGetLength((OCStringRef)mut_s));
        ok = false;
    }
    OCRelease(mut_s);

    fprintf(stderr, "  Testing OCStringCreateMutableCopy...\n");
    // Test OCStringCreateMutableCopy
    OCStringRef original_s = STR("Original");
    OCMutableStringRef mutable_copy = OCStringCreateMutableCopy(original_s);
    if (!mutable_copy) {
        fprintf(stderr, "stringTest8: OCStringCreateMutableCopy failed.\n");
        // No OCRelease(original_s) as STR() creates a constant string
        return false;
    }
    if (!OCStringEqual((OCStringRef)mutable_copy, original_s)) {
        fprintf(stderr, "stringTest8: Mutable copy is not equal to original.\n");
        ok = false;
    }
    OCStringAppendCString(mutable_copy, " Modified");
    if (OCStringEqual((OCStringRef)mutable_copy, original_s)) {
        fprintf(stderr, "stringTest8: Mutable copy modification affected original (should not happen).\n");
        ok = false;
    }
    OCRelease(mutable_copy);
    // No OCRelease(original_s)

    fprintf(stderr, "  Testing OCStringCompare...\n");
    // Test OCStringCompare
    OCStringRef s_apple_lower = STR("apple");
    OCStringRef s_apple_upper = STR("APPLE");
    OCStringRef s_banana = STR("banana");

    if (OCStringCompare(s_apple_lower, s_apple_upper, 0) == kOCCompareEqualTo) {
        fprintf(stderr, "stringTest8: Case-sensitive compare failed (apple vs APPLE should not be equal).\n");
        ok = false;
    }
    if (OCStringCompare(s_apple_lower, s_apple_upper, kOCCompareCaseInsensitive) != kOCCompareEqualTo) {
        fprintf(stderr, "stringTest8: Case-insensitive compare failed (apple vs APPLE should be equal).\n");
        ok = false;
    }
    if (OCStringCompare(s_apple_lower, s_banana, 0) != kOCCompareLessThan) {
        fprintf(stderr, "stringTest8: Compare failed (apple vs banana).\n");
        ok = false;
    }
    // No OCRelease for STR() strings

    fprintf(stderr, "  Testing OCStringLowercase, OCStringUppercase, OCStringTrimWhitespace...\n");
    // Test OCStringLowercase, OCStringUppercase, OCStringTrimWhitespace
    OCMutableStringRef mixed_case_str = OCMutableStringCreateWithCString("  MiXeD CaSe & Spaces  ");
    if (!mixed_case_str) {
        fprintf(stderr, "stringTest8: OCMutableStringCreateWithCString for mixed_case_str failed.\n");
        return false;
    }
    OCStringLowercase(mixed_case_str);
    if (!OCStringEqual((OCStringRef)mixed_case_str, STR("  mixed case & spaces  "))) {
        fprintf(stderr, "stringTest8: OCStringLowercase failed.\n");
        ok = false;
    }
    OCStringUppercase(mixed_case_str);
    if (!OCStringEqual((OCStringRef)mixed_case_str, STR("  MIXED CASE & SPACES  "))) {
        fprintf(stderr, "stringTest8: OCStringUppercase failed.\n");
        ok = false;
    }
    OCStringTrimWhitespace(mixed_case_str);
    if (!OCStringEqual((OCStringRef)mixed_case_str, STR("MIXED CASE & SPACES"))) {
        fprintf(stderr, "stringTest8: OCStringTrimWhitespace failed.\n");
        ok = false;
    }
    OCRelease(mixed_case_str);
    
    fprintf(stderr, "  Testing OCStringTrimMatchingParentheses...\n");
    // Test OCStringTrimMatchingParentheses
    OCMutableStringRef paren_str1 = OCMutableStringCreateWithCString("(content)");
    OCMutableStringRef paren_str2 = OCMutableStringCreateWithCString("no parens");
    OCMutableStringRef paren_str3 = OCMutableStringCreateWithCString("(mismatched");
    OCMutableStringRef paren_str4 = OCMutableStringCreateWithCString("mismatched)");
    OCMutableStringRef paren_str5 = OCMutableStringCreateWithCString("((double))");


    if (!OCStringTrimMatchingParentheses(paren_str1) || !OCStringEqual((OCStringRef)paren_str1, STR("content"))) {
        fprintf(stderr, "stringTest8: OCStringTrimMatchingParentheses failed for '(content)'.\n");
        ok = false;
    }
    if (OCStringTrimMatchingParentheses(paren_str2) || !OCStringEqual((OCStringRef)paren_str2, STR("no parens"))) {
        fprintf(stderr, "stringTest8: OCStringTrimMatchingParentheses failed for 'no parens'.\n");
        ok = false;
    }
     if (OCStringTrimMatchingParentheses(paren_str3) || !OCStringEqual((OCStringRef)paren_str3, STR("(mismatched"))) {
        fprintf(stderr, "stringTest8: OCStringTrimMatchingParentheses failed for '(mismatched'.\n");
        ok = false;
    }
    if (OCStringTrimMatchingParentheses(paren_str4) || !OCStringEqual((OCStringRef)paren_str4, STR("mismatched)"))) {
        fprintf(stderr, "stringTest8: OCStringTrimMatchingParentheses failed for 'mismatched)'.\n");
        ok = false;
    }
    if (!OCStringTrimMatchingParentheses(paren_str5) || !OCStringEqual((OCStringRef)paren_str5, STR("(double)"))) {
        fprintf(stderr, "stringTest8: OCStringTrimMatchingParentheses failed for '((double))'.\n");
        ok = false;
    }
    OCRelease(paren_str1);
    OCRelease(paren_str2);
    OCRelease(paren_str3);
    OCRelease(paren_str4);
    OCRelease(paren_str5);

    fprintf(stderr, "  Testing OCStringFind...\n");
    // Test OCStringFind
    OCStringRef find_haystack = STR("One two three two one");
    OCStringRef find_needle = STR("two");
    OCRange found_range1 = OCStringFind(find_haystack, find_needle, 0);
    if (found_range1.location != 4 || found_range1.length != 3) {
        fprintf(stderr, "stringTest8: OCStringFind (1st occurrence of 'two') failed. Expected {4,3}, got {%llu,%llu}\n", found_range1.location, found_range1.length);
        ok = false;
    }
    
    // Test finding the second occurrence by searching in a subrange of the original string
    // This requires creating a temporary substring or adjusting how OCStringFind is used if it supports a search range directly.
    // The current OCStringFind takes the full string and stringToFind. The example in OCString.h for OCStringFind
    // actually demonstrates OCStringCreateArrayWithFindResults for multiple occurrences.
    // For a direct second find, one might do:
    OCStringRef sub_haystack = OCStringCreateWithSubstring(find_haystack, OCRangeMake(found_range1.location + found_range1.length, OCStringGetLength(find_haystack) - (found_range1.location + found_range1.length)));
    OCRange found_range_in_sub = OCStringFind(sub_haystack, find_needle, 0);
    if (found_range_in_sub.location != 7 || found_range_in_sub.length != 3) { // "two" is at index 7 of " three two one"
         fprintf(stderr, "stringTest8: OCStringFind (2nd occurrence of 'two') failed. Expected {7,3} in substring, got {%llu,%llu}\n", found_range_in_sub.location, found_range_in_sub.length);
        ok = false;
    }
    OCRelease(sub_haystack);

    OCStringRef find_needle_upper = STR("TWO");
    OCRange found_range_case_insensitive = OCStringFind(find_haystack, find_needle_upper, kOCCompareCaseInsensitive);
     if (found_range_case_insensitive.location != 4 || found_range_case_insensitive.length != 3) {
        fprintf(stderr, "stringTest8: OCStringFind (case-insensitive 'TWO') failed. Expected {4,3}, got {%llu,%llu}\n", found_range_case_insensitive.location, found_range_case_insensitive.length);
        ok = false;
    }
    OCRange not_found_range = OCStringFind(find_haystack, STR("four"), 0);
    if (not_found_range.location != kOCNotFound || not_found_range.length != 0) { // kOCNotFound is typically (uint64_t)-1 or similar for location
        fprintf(stderr, "stringTest8: OCStringFind (not found 'four') failed. Expected {%llu,0}, got {%llu,%llu}\n", (uint64_t)kOCNotFound, not_found_range.location, not_found_range.length);
        ok = false;
    }

    fprintf(stderr, "  Testing OCStringFindAndReplace2...\n");
    // Test OCStringFindAndReplace2
    OCMutableStringRef replace_str = OCMutableStringCreateWithCString("cat dog cat bird cat");
    OCStringRef find_cat = STR("cat");
    OCStringRef replace_mouse = STR("mouse");
    int64_t replacements = OCStringFindAndReplace2(replace_str, find_cat, replace_mouse);
    if (replacements != 3 || !OCStringEqual((OCStringRef)replace_str, STR("mouse dog mouse bird mouse"))) {
        fprintf(stderr, "stringTest8: OCStringFindAndReplace2 failed.\n");
        ok = false;
    }
    OCRelease(replace_str);

    fprintf(stderr, "  Testing OCStringFindAndReplace (with range and options)...\n");
    // Test OCStringFindAndReplace (with range and options)
    OCMutableStringRef replace_range_str = OCMutableStringCreateWithCString("Alpha Beta Gamma Alpha Delta");
    OCStringRef find_alpha = STR("Alpha");
    OCStringRef replace_omega = STR("Omega");
    OCRange search_r = {0, 15}; // "Alpha Beta Gamm"
    replacements = OCStringFindAndReplace(replace_range_str, find_alpha, replace_omega, search_r, 0);
    if (replacements != 1 || !OCStringEqual((OCStringRef)replace_range_str, STR("Omega Beta Gamma Alpha Delta"))) {
        fprintf(stderr, "stringTest8: OCStringFindAndReplace (first part) failed.\n");
        ok = false;
    }
    OCStringRef find_ALPHA_case = STR("ALPHA");
    search_r.location = OCStringGetLength(STR("Omega Beta Gamma ")); // Start after "Omega Beta Gamma "
    search_r.length = OCStringGetLength((OCStringRef)replace_range_str) - search_r.location;
    replacements = OCStringFindAndReplace(replace_range_str, find_ALPHA_case, replace_omega, search_r, kOCCompareCaseInsensitive);
     if (replacements != 1 || !OCStringEqual((OCStringRef)replace_range_str, STR("Omega Beta Gamma Omega Delta"))) {
        fprintf(stderr, "stringTest8: OCStringFindAndReplace (second part, case-insensitive) failed.\n");
        ok = false;
    }
    OCRelease(replace_range_str);

    fprintf(stderr, "  Testing character utility functions...\n");
    // Test characterIsUpperCaseLetter, characterIsLowerCaseLetter, characterIsDigitOrDecimalPoint, characterIsDigitOrDecimalPointOrSpace
    if (!characterIsUpperCaseLetter('A') || characterIsUpperCaseLetter('a')) { fprintf(stderr, "char test A failed\n"); ok = false; }
    if (!characterIsLowerCaseLetter('z') || characterIsLowerCaseLetter('Z')) { fprintf(stderr, "char test z failed\n"); ok = false; }
    if (!characterIsDigitOrDecimalPoint('5') || !characterIsDigitOrDecimalPoint('.')) { fprintf(stderr, "char test 5 . failed\n"); ok = false; }
    if (characterIsDigitOrDecimalPoint('x')) { fprintf(stderr, "char test x failed\n"); ok = false; }
    if (!characterIsDigitOrDecimalPointOrSpace('9') || !characterIsDigitOrDecimalPointOrSpace('.') || !characterIsDigitOrDecimalPointOrSpace(' ')) { fprintf(stderr, "char test 9 . space failed\n"); ok = false; }
    if (characterIsDigitOrDecimalPointOrSpace('\t') || characterIsDigitOrDecimalPointOrSpace('A')) { fprintf(stderr, "char test tab A failed\n"); ok = false; }
    if (!ok) {
        fprintf(stderr, "stringTest8: Character check functions failed.\n");
    }
    
    fprintf(stderr, "  Testing OCFloatCreateStringValue, OCDoubleCreateStringValue...\n");
    // Test OCFloatCreateStringValue, OCDoubleCreateStringValue
    OCStringRef float_str = OCFloatCreateStringValue(3.14159f);
    // Precision might vary, so check for presence of key parts
    if (OCStringFind(float_str, STR("3.14"), 0).length == 0) {
         fprintf(stderr, "stringTest8: OCFloatCreateStringValue check failed.\n");
        ok = false;
    }
    OCRelease(float_str);

    OCStringRef double_str = OCDoubleCreateStringValue(2.718281828);
     if (OCStringFind(double_str, STR("2.71828"), 0).length == 0) {
        fprintf(stderr, "stringTest8: OCDoubleCreateStringValue check failed.\n");
        ok = false;
    }
    OCRelease(double_str);

    fprintf(stderr, "  Testing OCFloatComplexCreateStringValue, OCDoubleComplexCreateStringValue...\n");
    // Test OCFloatComplexCreateStringValue, OCDoubleComplexCreateStringValue
    float complex fc = 1.0f + 2.0f * I;
    OCStringRef fc_str = OCFloatComplexCreateStringValue(fc, STR("%.1f%+.1fi"));
    if (!OCStringEqual(fc_str, STR("1.0+2.0i"))) {
        fprintf(stderr, "stringTest8: OCFloatComplexCreateStringValue failed. Expected '1.0+2.0i', got '%s'\n", OCStringGetCString(fc_str));
        ok = false;
    }
    OCRelease(fc_str);

    double complex dc = 3.0 - 4.0 * I;
    OCStringRef dc_str = OCDoubleComplexCreateStringValue(dc, STR("%.1f%+.1fi"));
     if (!OCStringEqual(dc_str, STR("3.0-4.0i"))) {
        fprintf(stderr, "stringTest8: OCDoubleComplexCreateStringValue failed. Expected '3.0-4.0i', got '%s'\n", OCStringGetCString(dc_str));
        ok = false;
    }
    OCRelease(dc_str);


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
        fprintf(stderr, "stringTest9: Incorrect code point count. Expected %llu, got %llu.\n", 
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
        fprintf(stderr, "stringTest9: Empty string should have length 0, got %llu.\n", OCStringGetLength(empty_str));
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
            fprintf(stderr, "stringTest9: Splitting empty string should result in array with 1 element, got %llu.\n", count);
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
            fprintf(stderr, "stringTest9: Splitting with empty separator should return array with 1 element, got %llu.\n", count);
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
            fprintf(stderr, "stringTest9: Expected 4 parts from ';one;two;', got %llu.\n", count);
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
                fprintf(stderr, "stringTest9: Incorrect length for large string. Expected %zu, got %llu.\n", 
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
                    fprintf(stderr, "stringTest9: Incorrect substring length. Expected 100, got %llu.\n", 
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


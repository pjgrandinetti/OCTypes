#include "test_array.h"
#include "../src/OCString.h" // For OCStringRef, OCStringCreateWithCString, OCStringEqual, OCRelease
#include "../src/OCArray.h"  // For OCArray specific functions like OCArrayCreateMutable, OCArrayAppendValue etc.
#include <inttypes.h>   // For PRIu64

// Adapter for OCArraySortValues on OCStringRefs:
OCComparisonResult OCStringCompareAdapter(const void *a, const void *b, void *ctx) {
    (void)ctx; // Context not used in this adapter
    return OCStringCompare((OCStringRef)a, (OCStringRef)b, 0);
}

// Original arrayTest0 implementation
bool arrayTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCMutableArrayRef arr = NULL;
    OCStringRef temp[5] = {NULL, NULL, NULL, NULL, NULL};
    OCStringRef chk = NULL;
    bool success = false;

    arr = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    if (!arr) {
        fprintf(stderr, "Error: OCArrayCreateMutable failed in %s\n", __func__);
        goto cleanup;
    }

    const char *in[]   = {"e","b","a","c","d"};
    const char *want[] = {"a","b","c","d","e"};

    for (int i = 0; i < 5; i++) {
        temp[i] = OCStringCreateWithCString(in[i]);
        if (!temp[i]) {
            fprintf(stderr, "Error: OCStringCreateWithCString failed for in[%d] in %s\n", i, __func__);
            goto cleanup;
        }
        OCArrayAppendValue(arr, temp[i]);
    }

    // verify insertion order
    for (int i = 0; i < 5; i++) {
        OCStringRef got = (OCStringRef)OCArrayGetValueAtIndex(arr, i);
        chk = OCStringCreateWithCString(in[i]);
        if (!chk) {
            fprintf(stderr, "Error: OCStringCreateWithCString failed for chk (in) in %s\n", __func__);
            goto cleanup;
        }
        if (!OCStringEqual(got, chk)) {
            fprintf(stderr, "Error: OCStringEqual failed for in[%d] in %s\n", i, __func__);
            goto cleanup;
        }
        OCRelease(chk);
        chk = NULL; // Prevent double release in cleanup
    }

    // sort and verify
    OCArraySortValues(arr, OCRangeMake(0, 5), OCStringCompareAdapter, NULL);
    for (int i = 0; i < 5; i++) {
        OCStringRef got = (OCStringRef)OCArrayGetValueAtIndex(arr, i);
        chk = OCStringCreateWithCString(want[i]);
        if (!chk) {
            fprintf(stderr, "Error: OCStringCreateWithCString failed for chk (want) in %s\n", __func__);
            goto cleanup;
        }
        if (!OCStringEqual(got, chk)) {
            fprintf(stderr, "Error: OCStringEqual failed for want[%d] in %s\n", i, __func__);
            goto cleanup;
        }
        OCRelease(chk);
        chk = NULL; // Prevent double release in cleanup
    }

    fprintf(stderr, "%s end...without problems\n", __func__);
    success = true;

cleanup:
    if (chk) OCRelease(chk);
    for (int i = 0; i < 5; i++) {
        if (temp[i]) OCRelease(temp[i]);
    }
    if (arr) OCRelease(arr);
    
    if (!success) {
      // This replaces the PRINTERROR macro behavior for this function
      fprintf(stderr, "Test %s FAILED at %s:%d\n", __func__, __FILE__, __LINE__-3); // approx line
    }
    return success;
}

bool arrayTest1_creation(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = false;

    OCStringRef s1 = NULL;
    OCStringRef s2 = NULL;
    OCArrayRef arr1 = NULL;
    OCArrayRef arr_empty_explicit_null = NULL;
    OCArrayRef arr_empty_non_null_ptr = NULL;
    OCArrayRef arr1_copy = NULL;
    OCMutableArrayRef marr1 = NULL;
    OCMutableArrayRef marr1_copy = NULL;
    OCMutableArrayRef marr1_mcopy = NULL;
    OCArrayRef arr_empty = NULL;
    OCArrayRef arr_empty_copy = NULL;
    OCMutableArrayRef marr_zero_cap = NULL;
    OCMutableArrayRef marr_empty_copy2 = NULL;

    s1 = OCStringCreateWithCString("hello");
    if (!s1) { fprintf(stderr, "Error: OCStringCreateWithCString(\"hello\") failed in %s\n", __func__); goto cleanup; }
    s2 = OCStringCreateWithCString("world");
    if (!s2) { fprintf(stderr, "Error: OCStringCreateWithCString(\"world\") failed in %s\n", __func__); goto cleanup; }

    const void *values[] = {s1, s2};
    arr1 = OCArrayCreate(values, 2, &kOCTypeArrayCallBacks);
    if (!arr1) { fprintf(stderr, "Error: OCArrayCreate failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetCount(arr1) == 2)) { fprintf(stderr, "Assertion failed: OCArrayCreate count mismatch in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(arr1, 0), s1))) { fprintf(stderr, "Assertion failed: OCArrayCreate value mismatch at index 0 in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(arr1, 1), s2))) { fprintf(stderr, "Assertion failed: OCArrayCreate value mismatch at index 1 in %s\n", __func__); goto cleanup; }

    arr_empty_explicit_null = OCArrayCreate(NULL, 0, &kOCTypeArrayCallBacks);
    if (!arr_empty_explicit_null) { fprintf(stderr, "Error: OCArrayCreate with NULL values and count 0 failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetCount(arr_empty_explicit_null) == 0)) { fprintf(stderr, "Assertion failed: OCArrayCreate with NULL values and count 0: count mismatch in %s\n", __func__); goto cleanup; }

    const void *empty_values_ptr[] = {NULL};
    arr_empty_non_null_ptr = OCArrayCreate(empty_values_ptr, 0, &kOCTypeArrayCallBacks);
    if (!arr_empty_non_null_ptr) { fprintf(stderr, "Error: OCArrayCreate with non-NULL values pointer and count 0 failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetCount(arr_empty_non_null_ptr) == 0)) { fprintf(stderr, "Assertion failed: OCArrayCreate with non-NULL values pointer and count 0: count mismatch in %s\n", __func__); goto cleanup; }
    OCRelease(arr_empty_non_null_ptr); arr_empty_non_null_ptr = NULL;

    arr1_copy = OCArrayCreateCopy(arr1);
    if (!arr1_copy) { fprintf(stderr, "Error: OCArrayCreateCopy failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetCount(arr1_copy) == 2)) { fprintf(stderr, "Assertion failed: OCArrayCreateCopy count mismatch in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(arr1_copy, 0), s1))) { fprintf(stderr, "Assertion failed: OCArrayCreateCopy value mismatch at index 0 in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(arr1_copy, 1), s2))) { fprintf(stderr, "Assertion failed: OCArrayCreateCopy value mismatch at index 1 in %s\n", __func__); goto cleanup; }
    if (!((void*)arr1 != (void*)arr1_copy)) { fprintf(stderr, "Assertion failed: OCArrayCreateCopy should create a new instance in %s\n", __func__); goto cleanup; } // Cast to void* for pointer comparison if necessary
    OCRelease(arr1_copy); arr1_copy = NULL;

    marr1 = OCArrayCreateMutable(5, &kOCTypeArrayCallBacks);
    if (!marr1) { fprintf(stderr, "Error: OCArrayCreateMutable failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetCount(marr1) == 0)) { fprintf(stderr, "Assertion failed: OCArrayCreateMutable initial count should be 0 in %s\n", __func__); goto cleanup; }

    marr1_copy = OCArrayCreateMutableCopy(arr1);
    if (!marr1_copy) { fprintf(stderr, "Error: OCArrayCreateMutableCopy from immutable failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetCount(marr1_copy) == 2)) { fprintf(stderr, "Assertion failed: OCArrayCreateMutableCopy (from immutable) count mismatch in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr1_copy, 0), s1))) { fprintf(stderr, "Assertion failed: OCArrayCreateMutableCopy (from immutable) value mismatch at index 0 in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr1_copy, 1), s2))) { fprintf(stderr, "Assertion failed: OCArrayCreateMutableCopy (from immutable) value mismatch at index 1 in %s\n", __func__); goto cleanup; }
    OCRelease(marr1_copy); marr1_copy = NULL;

    OCArrayAppendValue(marr1, s1); // s1 is already checked for NULL
    marr1_mcopy = OCArrayCreateMutableCopy(marr1);
    if (!marr1_mcopy) { fprintf(stderr, "Error: OCArrayCreateMutableCopy from mutable failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetCount(marr1_mcopy) == 1)) { fprintf(stderr, "Assertion failed: OCArrayCreateMutableCopy (from mutable) count mismatch in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr1_mcopy, 0), s1))) { fprintf(stderr, "Assertion failed: OCArrayCreateMutableCopy (from mutable) value mismatch at index 0 in %s\n", __func__); goto cleanup; }
    OCRelease(marr1_mcopy); marr1_mcopy = NULL;

    arr_empty = OCArrayCreate(NULL, 0, &kOCTypeArrayCallBacks);
    if (!arr_empty) { fprintf(stderr, "Error: OCArrayCreate with 0 values failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetCount(arr_empty) == 0)) { fprintf(stderr, "Assertion failed: OCArrayCreate with 0 values count mismatch in %s\n", __func__); goto cleanup; }
    OCRelease(arr_empty); arr_empty = NULL;

    arr_empty_copy = OCArrayCreateCopy(arr_empty_explicit_null);
    if (!arr_empty_copy) { fprintf(stderr, "Error: OCArrayCreateCopy with empty array failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetCount(arr_empty_copy) == 0)) { fprintf(stderr, "Assertion failed: OCArrayCreateCopy with empty array count mismatch in %s\n", __func__); goto cleanup; }
    OCRelease(arr_empty_copy); arr_empty_copy = NULL;

    marr_zero_cap = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    if (!marr_zero_cap) { fprintf(stderr, "Error: OCArrayCreateMutable with 0 capacity failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetCount(marr_zero_cap) == 0)) { fprintf(stderr, "Assertion failed: OCArrayCreateMutable with 0 capacity count mismatch in %s\n", __func__); goto cleanup; }
    OCArrayAppendValue(marr_zero_cap, s1);
    if (!(OCArrayGetCount(marr_zero_cap) == 1)) { fprintf(stderr, "Assertion failed: OCArrayCreateMutable with 0 capacity: count after append mismatch in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr_zero_cap, 0), s1))) { fprintf(stderr, "Assertion failed: OCArrayCreateMutable with 0 capacity: value after append mismatch in %s\n", __func__); goto cleanup; }
    OCRelease(marr_zero_cap); marr_zero_cap = NULL;

    marr_empty_copy2 = OCArrayCreateMutableCopy(arr_empty_explicit_null);
    if (!marr_empty_copy2) { fprintf(stderr, "Error: OCArrayCreateMutableCopy with empty array failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetCount(marr_empty_copy2) == 0)) { fprintf(stderr, "Assertion failed: OCArrayCreateMutableCopy with empty array count mismatch in %s\n", __func__); goto cleanup; }
    OCRelease(marr_empty_copy2); marr_empty_copy2 = NULL;

    fprintf(stderr, "%s end...without problems\n", __func__);
    success = true;

cleanup:
    if (s1) OCRelease(s1);
    if (s2) OCRelease(s2);
    if (arr1) OCRelease(arr1);
    if (arr_empty_explicit_null) OCRelease(arr_empty_explicit_null);
    if (arr_empty_non_null_ptr) OCRelease(arr_empty_non_null_ptr); // Should be NULL if released mid-function
    if (arr1_copy) OCRelease(arr1_copy); // Should be NULL if released mid-function
    if (marr1) OCRelease(marr1);
    if (marr1_copy) OCRelease(marr1_copy); // Should be NULL if released mid-function
    if (marr1_mcopy) OCRelease(marr1_mcopy); // Should be NULL if released mid-function
    if (arr_empty) OCRelease(arr_empty); // Should be NULL if released mid-function
    if (arr_empty_copy) OCRelease(arr_empty_copy); // Should be NULL if released mid-function
    if (marr_zero_cap) OCRelease(marr_zero_cap); // Should be NULL if released mid-function
    if (marr_empty_copy2) OCRelease(marr_empty_copy2); // Should be NULL if released mid-function

    if (!success) {
      fprintf(stderr, "Test %s FAILED (due to prior error or assertion failure) in file %s\n", __func__, __FILE__);
    }
    return success;
}

bool arrayTest2_access(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = false;

    OCStringRef s1 = NULL;
    OCStringRef s2 = NULL;
    OCStringRef s3 = NULL;
    OCArrayRef arr = NULL;
    OCArrayRef empty_arr = NULL;
    OCNumberRef n1 = NULL;
    OCNumberRef n2 = NULL;
    OCNumberRef n3 = NULL;
    OCNumberRef n4 = NULL;
    OCArrayRef num_arr = NULL;

    s1 = OCStringCreateWithCString("apple");
    if (!s1) { fprintf(stderr, "Error: OCStringCreateWithCString(\"apple\") failed in %s\n", __func__); goto cleanup; }
    s2 = OCStringCreateWithCString("banana");
    if (!s2) { fprintf(stderr, "Error: OCStringCreateWithCString(\"banana\") failed in %s\n", __func__); goto cleanup; }
    s3 = OCStringCreateWithCString("cherry");
    if (!s3) { fprintf(stderr, "Error: OCStringCreateWithCString(\"cherry\") failed in %s\n", __func__); goto cleanup; }

    const void *values[] = {s1, s2, s3};
    arr = OCArrayCreate((const void **)values, 3, &kOCTypeArrayCallBacks);
    if (!arr) { fprintf(stderr, "Error: Failed to create array for access tests in %s\n", __func__); goto cleanup; }

    if (!(OCArrayGetCount(arr) == 3)) { fprintf(stderr, "Assertion failed: OCArrayGetCount basic test failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(arr, 0), s1))) { fprintf(stderr, "Assertion failed: OCArrayGetValueAtIndex(0) failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(arr, 1), s2))) { fprintf(stderr, "Assertion failed: OCArrayGetValueAtIndex(1) failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(arr, 2), s3))) { fprintf(stderr, "Assertion failed: OCArrayGetValueAtIndex(2) failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetValueAtIndex(arr, 3) == NULL)) { fprintf(stderr, "Assertion failed: OCArrayGetValueAtIndex out of bounds should return NULL in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetValueAtIndex(arr, (uint64_t)-1) == NULL)) { fprintf(stderr, "Assertion failed: OCArrayGetValueAtIndex large index should return NULL in %s\n", __func__); goto cleanup; }

    empty_arr = OCArrayCreate(NULL, 0, &kOCTypeArrayCallBacks);
    if (!empty_arr) { fprintf(stderr, "Error: Failed to create empty array in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetCount(empty_arr) == 0)) { fprintf(stderr, "Assertion failed: OCArrayGetCount for empty array failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetValueAtIndex(empty_arr, 0) == NULL)) { fprintf(stderr, "Assertion failed: OCArrayGetValueAtIndex on empty array should return NULL in %s\n", __func__); goto cleanup; }

    n1 = OCNumberCreateWithSInt32(10);
    if (!n1) { fprintf(stderr, "Error: OCNumberCreateWithSInt32(10) failed in %s\n", __func__); goto cleanup; }
    n2 = OCNumberCreateWithSInt32(20);
    if (!n2) { fprintf(stderr, "Error: OCNumberCreateWithSInt32(20) failed in %s\n", __func__); goto cleanup; }
    n3 = OCNumberCreateWithSInt32(10); // Same value as n1 for OCEqual test
    if (!n3) { fprintf(stderr, "Error: OCNumberCreateWithSInt32(10) for n3 failed in %s\n", __func__); goto cleanup; }
    n4 = OCNumberCreateWithSInt32(30);
    if (!n4) { fprintf(stderr, "Error: OCNumberCreateWithSInt32(30) failed in %s\n", __func__); goto cleanup; }

    const void *num_values[] = {n1, n2, n3};
    num_arr = OCArrayCreate((const void **)num_values, 3, &kOCTypeArrayCallBacks);
    if (!num_arr) { fprintf(stderr, "Error: Failed to create number array for access tests in %s\n", __func__); goto cleanup; }

    if (!(OCArrayGetFirstIndexOfValue(num_arr, n1) == 0)) { fprintf(stderr, "Assertion failed: FirstIndexOfValue (n1) failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetFirstIndexOfValue(num_arr, n2) == 1)) { fprintf(stderr, "Assertion failed: FirstIndexOfValue (n2) failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetFirstIndexOfValue(num_arr, n3) == 0)) { fprintf(stderr, "Assertion failed: FirstIndexOfValue (n3) due to OCEqual failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetFirstIndexOfValue(num_arr, n4) == kOCNotFound)) { fprintf(stderr, "Assertion failed: FirstIndexOfValue (n4 not present) failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayGetFirstIndexOfValue(empty_arr, n1) == kOCNotFound)) { fprintf(stderr, "Assertion failed: FirstIndexOfValue on empty array failed in %s\n", __func__); goto cleanup; }

    if (!(OCArrayContainsValue(num_arr, n1))) { fprintf(stderr, "Assertion failed: ContainsValue (n1) failed in %s\n", __func__); goto cleanup; }
    if (!(OCArrayContainsValue(num_arr, n3))) { fprintf(stderr, "Assertion failed: ContainsValue (n3) due to OCEqual failed in %s\n", __func__); goto cleanup; }
    if (!(!OCArrayContainsValue(num_arr, n4))) { fprintf(stderr, "Assertion failed: ContainsValue (n4 not present) failed in %s\n", __func__); goto cleanup; }
    if (!(!OCArrayContainsValue(empty_arr, n1))) { fprintf(stderr, "Assertion failed: ContainsValue on empty array failed in %s\n", __func__); goto cleanup; }

    fprintf(stderr, "%s end...without problems\n", __func__);
    success = true;

cleanup:
    if (s1) OCRelease(s1);
    if (s2) OCRelease(s2);
    if (s3) OCRelease(s3);
    if (arr) OCRelease(arr);
    if (n1) OCRelease(n1);
    if (n2) OCRelease(n2);
    if (n3) OCRelease(n3);
    if (n4) OCRelease(n4);
    if (num_arr) OCRelease(num_arr);
    if (empty_arr) OCRelease(empty_arr);

    if (!success) {
        fprintf(stderr, "Test %s FAILED (due to prior error or assertion failure) in file %s\n", __func__, __FILE__);
    }
    return success;
}

bool arrayTest3_modification(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = false;

    OCStringRef s1 = NULL;
    OCStringRef s2 = NULL;
    OCStringRef s3 = NULL;
    OCStringRef s4 = NULL;
    OCStringRef s_extra = NULL;
    OCMutableArrayRef marr = NULL;
    OCStringRef s_insert = NULL;
    OCStringRef s_another = NULL;
    OCArrayRef arr_to_append = NULL;

    s1 = OCStringCreateWithCString("a");
    if (!s1) { fprintf(stderr, "Error: OCStringCreateWithCString(\"a\") failed in %s\n", __func__); goto cleanup; }
    s2 = OCStringCreateWithCString("b");
    if (!s2) { fprintf(stderr, "Error: OCStringCreateWithCString(\"b\") failed in %s\n", __func__); goto cleanup; }
    s3 = OCStringCreateWithCString("c");
    if (!s3) { fprintf(stderr, "Error: OCStringCreateWithCString(\"c\") failed in %s\n", __func__); goto cleanup; }
    s4 = OCStringCreateWithCString("d");
    if (!s4) { fprintf(stderr, "Error: OCStringCreateWithCString(\"d\") failed in %s\n", __func__); goto cleanup; }
    s_extra = OCStringCreateWithCString("extra");
    if (!s_extra) { fprintf(stderr, "Error: OCStringCreateWithCString(\"extra\") failed in %s\n", __func__); goto cleanup; }

    marr = OCArrayCreateMutable(2, &kOCTypeArrayCallBacks);
    if (!marr) { fprintf(stderr, "Error: Failed to create mutable array for modification tests in %s\n", __func__); goto cleanup; }

    OCArrayAppendValue(marr, s1);
    if (!(OCArrayGetCount(marr) == 1)) { fprintf(stderr, "Assertion failed: AppendValue count failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr, 0), s1))) { fprintf(stderr, "Assertion failed: AppendValue value failed in %s\n", __func__); goto cleanup; }

    s_insert = OCStringCreateWithCString("insert_me");
    if (!s_insert) { fprintf(stderr, "Error: OCStringCreateWithCString(\"insert_me\") failed in %s\n", __func__); goto cleanup; }
    OCArrayInsertValueAtIndex(marr, 0, s_insert); // marr: [insert_me, a]
    if (!(OCArrayGetCount(marr) == 2)) { fprintf(stderr, "Assertion failed: InsertValueAtIndex count failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr, 0), s_insert))) { fprintf(stderr, "Assertion failed: InsertValueAtIndex(0) value failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr, 1), s1))) { fprintf(stderr, "Assertion failed: InsertValueAtIndex(1) (original s1) value failed in %s\n", __func__); goto cleanup; }

    OCArrayAppendValue(marr, s2); // marr: [insert_me, a, b]
    if (!(OCArrayGetCount(marr) == 3)) { fprintf(stderr, "Assertion failed: AppendValue (s2) count failed in %s\n", __func__); goto cleanup; }

    OCArrayRemoveValueAtIndex(marr, 1); // marr: [insert_me, b]
    if (!(OCArrayGetCount(marr) == 2)) { fprintf(stderr, "Assertion failed: RemoveValueAtIndex count failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr, 0), s_insert))) { fprintf(stderr, "Assertion failed: RemoveValueAtIndex(0) (s_insert) value failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr, 1), s2))) { fprintf(stderr, "Assertion failed: RemoveValueAtIndex(1) (s2) value failed in %s\n", __func__); goto cleanup; }

    OCArrayRemoveValueAtIndex(marr, 0); 
    OCArrayInsertValueAtIndex(marr, 0, s3);
    if (!(OCArrayGetCount(marr) == 2)) { fprintf(stderr, "Assertion failed: ReplaceValueAtIndex count failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr, 0), s3))) { fprintf(stderr, "Assertion failed: ReplaceValueAtIndex(0) (s3) value failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr, 1), s2))) { fprintf(stderr, "Assertion failed: ReplaceValueAtIndex(1) (s2) value failed in %s\n", __func__); goto cleanup; }

    s_another = OCStringCreateWithCString("another");
    if (!s_another) { fprintf(stderr, "Error: OCStringCreateWithCString(\"another\") failed in %s\n", __func__); goto cleanup; }
    const void* other_values[] = {s4, s_another};
    arr_to_append = OCArrayCreate((const void **)other_values, 2, &kOCTypeArrayCallBacks);
    if (!arr_to_append) { fprintf(stderr, "Error: OCArrayCreate for arr_to_append failed in %s\n", __func__); goto cleanup; }
    OCArrayAppendArray(marr, arr_to_append, OCRangeMake(0, OCArrayGetCount(arr_to_append))); // marr: [c, b, d, another]
    if (!(OCArrayGetCount(marr) == 4)) { fprintf(stderr, "Assertion failed: AppendArray count failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr, 2), s4))) { fprintf(stderr, "Assertion failed: AppendArray value s4 failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr, 3), s_another))) { fprintf(stderr, "Assertion failed: AppendArray value s_another failed in %s\n", __func__); goto cleanup; }

    while(OCArrayGetCount(marr) > 0) {
        OCArrayRemoveValueAtIndex(marr, 0);
    }
    if (!(OCArrayGetCount(marr) == 0)) { fprintf(stderr, "Assertion failed: RemoveAllValues count failed in %s\n", __func__); goto cleanup; }
    OCArrayAppendValue(marr, s_extra); // Check if it's usable after removing all
    if (!(OCArrayGetCount(marr) == 1)) { fprintf(stderr, "Assertion failed: Append after RemoveAllValues count failed in %s\n", __func__); goto cleanup; }
    if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr, 0), s_extra))) { fprintf(stderr, "Assertion failed: Append after RemoveAllValues value failed in %s\n", __func__); goto cleanup; }

    fprintf(stderr, "%s end...without problems\n", __func__);
    success = true;

cleanup:
    if (s1) OCRelease(s1);
    if (s2) OCRelease(s2);
    if (s3) OCRelease(s3);
    if (s4) OCRelease(s4);
    if (s_extra) OCRelease(s_extra);
    if (s_insert) OCRelease(s_insert);
    if (s_another) OCRelease(s_another);
    if (arr_to_append) OCRelease(arr_to_append);
    if (marr) OCRelease(marr);

    if (!success) {
        fprintf(stderr, "Test %s FAILED (due to prior error or assertion failure) in file %s\n", __func__, __FILE__);
    }
    return success;
}

bool arrayTest4_search_sort(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    bool success = false;

    OCStringRef s_e = NULL;
    OCStringRef s_b = NULL;
    OCStringRef s_a = NULL;
    OCStringRef s_c = NULL;
    OCStringRef s_d = NULL;
    OCStringRef s_f = NULL; // Not in initial array
    OCMutableArrayRef marr = NULL;
    OCStringRef temp_str = NULL; // For temporary string creation during verification
    OCMutableArrayRef empty_marr = NULL;

    s_e = OCStringCreateWithCString("e");
    if (!s_e) { fprintf(stderr, "Error: OCStringCreateWithCString(\"e\") failed in %s\n", __func__); goto cleanup; }
    s_b = OCStringCreateWithCString("b");
    if (!s_b) { fprintf(stderr, "Error: OCStringCreateWithCString(\"b\") failed in %s\n", __func__); goto cleanup; }
    s_a = OCStringCreateWithCString("a");
    if (!s_a) { fprintf(stderr, "Error: OCStringCreateWithCString(\"a\") failed in %s\n", __func__); goto cleanup; }
    s_c = OCStringCreateWithCString("c");
    if (!s_c) { fprintf(stderr, "Error: OCStringCreateWithCString(\"c\") failed in %s\n", __func__); goto cleanup; }
    s_d = OCStringCreateWithCString("d");
    if (!s_d) { fprintf(stderr, "Error: OCStringCreateWithCString(\"d\") failed in %s\n", __func__); goto cleanup; }
    s_f = OCStringCreateWithCString("f");
    if (!s_f) { fprintf(stderr, "Error: OCStringCreateWithCString(\"f\") failed in %s\n", __func__); goto cleanup; }

    marr = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    if (!marr) { fprintf(stderr, "Error: OCArrayCreateMutable failed in %s\n", __func__); goto cleanup; }

    OCArrayAppendValue(marr, s_e);
    OCArrayAppendValue(marr, s_b);
    OCArrayAppendValue(marr, s_a);
    OCArrayAppendValue(marr, s_c);
    OCArrayAppendValue(marr, s_d);

    OCArraySortValues(marr, OCRangeMake(0, OCArrayGetCount(marr)), (OCComparatorFunction)OCStringCompareAdapter, NULL);
    const char *sorted_expected[] = {"a", "b", "c", "d", "e"};
    if (!(OCArrayGetCount(marr) == 5)) { fprintf(stderr, "Assertion failed: OCArraySortValues count mismatch in %s\n", __func__); goto cleanup; }
    for (uint64_t i = 0; i < 5; ++i) {
        temp_str = OCStringCreateWithCString(sorted_expected[i]);
        if (!temp_str) { fprintf(stderr, "Error: OCStringCreateWithCString for sorted_expected failed in %s\n", __func__); goto cleanup; }
        if (!(OCTypeEqual(OCArrayGetValueAtIndex(marr, i), temp_str))) {
            fprintf(stderr, "Assertion failed: OCArraySortValues element %" PRIu64 " mismatch in %s\n", i, __func__);
            goto cleanup; // temp_str will be released in cleanup
        }
        OCRelease(temp_str); temp_str = NULL;
    }

    uint64_t found_idx; // OCArrayBSearchValues returns int64_t, but kOCNotFound is -1 (long). Using uint64_t and checking against kOCNotFound is fine.
    found_idx = OCArrayBSearchValues(marr, OCRangeMake(0, OCArrayGetCount(marr)), s_c, (OCComparatorFunction)OCStringCompareAdapter, NULL);
    if (!(found_idx == 2)) { fprintf(stderr, "Assertion failed: OCArrayBSearchValues for s_c (expected 2, got %" PRIu64 ") failed in %s\n", found_idx, __func__); goto cleanup; }

    found_idx = OCArrayBSearchValues(marr, OCRangeMake(0, OCArrayGetCount(marr)), s_a, (OCComparatorFunction)OCStringCompareAdapter, NULL);
    if (!(found_idx == 0)) { fprintf(stderr, "Assertion failed: OCArrayBSearchValues for s_a (expected 0, got %" PRIu64 ") failed in %s\n", found_idx, __func__); goto cleanup; }

    found_idx = OCArrayBSearchValues(marr, OCRangeMake(0, OCArrayGetCount(marr)), s_e, (OCComparatorFunction)OCStringCompareAdapter, NULL);
    if (!(found_idx == 4)) { fprintf(stderr, "Assertion failed: OCArrayBSearchValues for s_e (expected 4, got %" PRIu64 ") failed in %s\n", found_idx, __func__); goto cleanup; }

    found_idx = OCArrayBSearchValues(marr, OCRangeMake(0, OCArrayGetCount(marr)), s_f, (OCComparatorFunction)OCStringCompareAdapter, NULL);
    if (!(found_idx == kOCNotFound)) { fprintf(stderr, "Assertion failed: OCArrayBSearchValues for s_f (not found, expected %ld, got %" PRIu64 ") failed in %s\n", (long)kOCNotFound, found_idx, __func__); goto cleanup; }

    found_idx = OCArrayBSearchValues(marr, OCRangeMake(0, 0), s_a, (OCComparatorFunction)OCStringCompareAdapter, NULL);
    if (!(found_idx == kOCNotFound)) { fprintf(stderr, "Assertion failed: OCArrayBSearchValues with empty range (expected %ld, got %" PRIu64 ") failed in %s\n", (long)kOCNotFound, found_idx, __func__); goto cleanup; }

    // marr: [a, b, c, d, e]
    found_idx = OCArrayBSearchValues(marr, OCRangeMake(2, 3), s_d, (OCComparatorFunction)OCStringCompareAdapter, NULL); // Search [c, d, e]
    if (!(found_idx == 3)) { fprintf(stderr, "Assertion failed: OCArrayBSearchValues for s_d in sub-range [2,3] (expected 3, got %" PRIu64 ") failed in %s\n", found_idx, __func__); goto cleanup; }

    found_idx = OCArrayBSearchValues(marr, OCRangeMake(0, 2), s_b, (OCComparatorFunction)OCStringCompareAdapter, NULL); // Search [a, b]
    if (!(found_idx == 1)) { fprintf(stderr, "Assertion failed: OCArrayBSearchValues for s_b in sub-range [0,2] (expected 1, got %" PRIu64 ") failed in %s\n", found_idx, __func__); goto cleanup; }

    found_idx = OCArrayBSearchValues(marr, OCRangeMake(4, 1), s_e, (OCComparatorFunction)OCStringCompareAdapter, NULL); // Search [e]
    if (!(found_idx == 4)) { fprintf(stderr, "Assertion failed: OCArrayBSearchValues for s_e in sub-range [4,1] (expected 4, got %" PRIu64 ") failed in %s\n", found_idx, __func__); goto cleanup; }

    found_idx = OCArrayBSearchValues(marr, OCRangeMake(1, 3), s_a, (OCComparatorFunction)OCStringCompareAdapter, NULL); // Search [b, c, d] -> s_a not found
    if (!(found_idx == kOCNotFound)) { fprintf(stderr, "Assertion failed: OCArrayBSearchValues for s_a (not found) in sub-range [1,3] (expected %ld, got %" PRIu64 ") failed in %s\n", (long)kOCNotFound, found_idx, __func__); goto cleanup; }

    empty_marr = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    if (!empty_marr) { fprintf(stderr, "Error: OCArrayCreateMutable for empty_marr failed in %s\n", __func__); goto cleanup; }
    found_idx = OCArrayBSearchValues(empty_marr, OCRangeMake(0, OCArrayGetCount(empty_marr)), s_a, (OCComparatorFunction)OCStringCompareAdapter, NULL);
    if (!(found_idx == kOCNotFound)) { fprintf(stderr, "Assertion failed: OCArrayBSearchValues on empty array (expected %ld, got %" PRIu64 ") failed in %s\n", (long)kOCNotFound, found_idx, __func__); goto cleanup; }
    OCRelease(empty_marr); empty_marr = NULL;

    fprintf(stderr, "%s end...without problems\n", __func__);
    success = true;

cleanup:
    if (s_a) OCRelease(s_a);
    if (s_b) OCRelease(s_b);
    if (s_c) OCRelease(s_c);
    if (s_d) OCRelease(s_d);
    if (s_e) OCRelease(s_e);
    if (s_f) OCRelease(s_f);
    if (marr) OCRelease(marr);
    if (temp_str) OCRelease(temp_str);
    if (empty_marr) OCRelease(empty_marr); // Should be NULL if released mid-function

    if (!success) {
        fprintf(stderr, "Test %s FAILED (due to prior error or assertion failure) in file %s\n", __func__, __FILE__);
    }
    return success;
}

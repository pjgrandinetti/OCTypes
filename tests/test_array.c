#include "test_array.h"
#include "../src/OCString.h" // For OCStringRef, OCStringCreateWithCString, OCStringEqual, OCRelease
#include "../src/OCArray.h"  // For OCArray specific functions like OCArrayCreateMutable, OCArrayAppendValue etc.

// Adapter for OCArraySortValues on OCStringRefs:
OCComparisonResult OCStringCompareAdapter(const void *a, const void *b, void *ctx) {
    (void)ctx; // Context not used in this adapter
    return OCStringCompare((OCStringRef)a, (OCStringRef)b, 0);
}

// Original arrayTest0 implementation
bool arrayTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCMutableArrayRef arr = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    if (!arr) PRINTERROR; // Check for null after creation

    const char *in[]   = {"e","b","a","c","d"};
    const char *want[] = {"a","b","c","d","e"};
    OCStringRef temp[5];

    for(int i=0;i<5;i++){
        temp[i] = OCStringCreateWithCString(in[i]);
        if (!temp[i]) { // Check for null after creation
            OCRelease(arr);
            for (int j = 0; j < i; ++j) OCRelease(temp[j]);
            PRINTERROR;
        }
        OCArrayAppendValue(arr, temp[i]);
    }

    // verify insertion order
    for(int i=0;i<5;i++){
        OCStringRef got = (OCStringRef)OCArrayGetValueAtIndex(arr,i);
        // No need to create a new OCString for comparison if OCStringEqual can take char*
        // Assuming OCStringEqual takes two OCStringRef, so chk is needed.
        OCStringRef chk = OCStringCreateWithCString(in[i]);
        if (!chk) { /* Handle allocation failure for chk */ PRINTERROR; }
        if (!OCStringEqual(got,chk)) { 
            OCRelease(chk); 
            // Release previously created temp strings and array
            for(int k=0;k<5;k++) OCRelease(temp[k]);
            OCRelease(arr);
            PRINTERROR; 
        }
        OCRelease(chk);
    }

    // sort and verify
    OCArraySortValues(arr, OCRangeMake(0,5), OCStringCompareAdapter, NULL);
    for(int i=0;i<5;i++){
        OCStringRef got = (OCStringRef)OCArrayGetValueAtIndex(arr,i);
        OCStringRef chk = OCStringCreateWithCString(want[i]);
        if (!chk) { /* Handle allocation failure for chk */ PRINTERROR; }
        if (!OCStringEqual(got,chk)) { 
            OCRelease(chk); 
            // Release previously created temp strings and array
            for(int k=0;k<5;k++) OCRelease(temp[k]);
            OCRelease(arr);
            PRINTERROR; 
        }
        OCRelease(chk);
    }

    fprintf(stderr, "%s end...without problems\n", __func__);
    for(int i=0;i<5;i++) OCRelease(temp[i]);
    OCRelease(arr);
    return true;
}

bool arrayTest1_creation(void) {
    OCAutoreleasePoolRef pool = OCAutoreleasePoolCreate();
    fprintf(stderr, "%s begin...\n", __func__);

    // Test OCArrayCreate (immutable)
    OCStringRef s1 = OCStringCreateWithCString("hello");
    OCStringRef s2 = OCStringCreateWithCString("world");
    const void *values[] = {s1, s2};
    OCArrayRef arr1 = OCArrayCreate(values, 2, &kOCTypeArrayCallBacks);
    ASSERT_NOT_NULL(arr1, "OCArrayCreate failed");
    ASSERT_EQUAL(OCArrayGetCount(arr1), 2, "OCArrayCreate count mismatch");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(arr1, 0), s1), "OCArrayCreate value mismatch at index 0");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(arr1, 1), s2), "OCArrayCreate value mismatch at index 1");

    // Test OCArrayCreate with NULL values (should be valid for count 0)
    OCArrayRef arr_empty_explicit_null = OCArrayCreate(NULL, 0, &kOCTypeArrayCallBacks);
    ASSERT_NOT_NULL(arr_empty_explicit_null, "OCArrayCreate with NULL values and count 0 failed");
    ASSERT_EQUAL(OCArrayGetCount(arr_empty_explicit_null), 0, "OCArrayCreate with NULL values and count 0: count mismatch");

    // Test OCArrayCreate with non-NULL values pointer but count 0
    const void *empty_values_ptr[] = {NULL}; // Dummy non-NULL pointer
    OCArrayRef arr_empty_non_null_ptr = OCArrayCreate(empty_values_ptr, 0, &kOCTypeArrayCallBacks);
    ASSERT_NOT_NULL(arr_empty_non_null_ptr, "OCArrayCreate with non-NULL values pointer and count 0 failed");
    ASSERT_EQUAL(OCArrayGetCount(arr_empty_non_null_ptr), 0, "OCArrayCreate with non-NULL values pointer and count 0: count mismatch");
    OCRelease(arr_empty_non_null_ptr);

    // Test OCArrayCreateCopy (immutable from immutable)
    OCArrayRef arr1_copy = OCArrayCreateCopy(arr1);
    ASSERT_NOT_NULL(arr1_copy, "OCArrayCreateCopy failed");
    ASSERT_EQUAL(OCArrayGetCount(arr1_copy), 2, "OCArrayCreateCopy count mismatch");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(arr1_copy, 0), s1), "OCArrayCreateCopy value mismatch at index 0");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(arr1_copy, 1), s2), "OCArrayCreateCopy value mismatch at index 1");
    ASSERT_FALSE(arr1 == arr1_copy, "OCArrayCreateCopy should create a new instance");
    OCRelease(arr1_copy);

    // Test OCArrayCreateMutable
    OCMutableArrayRef marr1 = OCArrayCreateMutable(5, &kOCTypeArrayCallBacks);
    ASSERT_NOT_NULL(marr1, "OCArrayCreateMutable failed");
    ASSERT_EQUAL(OCArrayGetCount(marr1), 0, "OCArrayCreateMutable initial count should be 0");
    // Capacity check is internal, but we can check if appends work up to initial capacity without issues.

    // Test OCArrayCreateMutableCopy (mutable from immutable)
    OCMutableArrayRef marr1_copy = OCArrayCreateMutableCopy(arr1);
    ASSERT_NOT_NULL(marr1_copy, "OCArrayCreateMutableCopy from immutable failed");
    ASSERT_EQUAL(OCArrayGetCount(marr1_copy), 2, "OCArrayCreateMutableCopy (from immutable) count mismatch");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr1_copy, 0), s1), "OCArrayCreateMutableCopy (from immutable) value mismatch at index 0");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr1_copy, 1), s2), "OCArrayCreateMutableCopy (from immutable) value mismatch at index 1");
    OCRelease(marr1_copy);

    // Test OCArrayCreateMutableCopy (mutable from mutable)
    OCArrayAppendValue(marr1, s1);
    OCMutableArrayRef marr1_mcopy = OCArrayCreateMutableCopy(marr1);
    ASSERT_NOT_NULL(marr1_mcopy, "OCArrayCreateMutableCopy from mutable failed");
    ASSERT_EQUAL(OCArrayGetCount(marr1_mcopy), 1, "OCArrayCreateMutableCopy (from mutable) count mismatch");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr1_mcopy, 0), s1), "OCArrayCreateMutableCopy (from mutable) value mismatch at index 0");
    OCRelease(marr1_mcopy);

    // Test OCArrayCreate with empty values
    OCArrayRef arr_empty = OCArrayCreate(NULL, 0, &kOCTypeArrayCallBacks);
    ASSERT_NOT_NULL(arr_empty, "OCArrayCreate with 0 values failed");
    ASSERT_EQUAL(OCArrayGetCount(arr_empty), 0, "OCArrayCreate with 0 values count mismatch");
    OCRelease(arr_empty);

    // Test OCArrayCreateCopy with empty array
    OCArrayRef arr_empty_copy = OCArrayCreateCopy(arr_empty_explicit_null); // Using previously created empty array
    ASSERT_NOT_NULL(arr_empty_copy, "OCArrayCreateCopy with empty array failed");
    ASSERT_EQUAL(OCArrayGetCount(arr_empty_copy), 0, "OCArrayCreateCopy with empty array count mismatch");
    OCRelease(arr_empty_copy);

    // Test OCArrayCreateMutable with 0 capacity
    OCMutableArrayRef marr_zero_cap = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    ASSERT_NOT_NULL(marr_zero_cap, "OCArrayCreateMutable with 0 capacity failed");
    ASSERT_EQUAL(OCArrayGetCount(marr_zero_cap), 0, "OCArrayCreateMutable with 0 capacity count mismatch");
    // Append a value to ensure it handles reallocation if capacity was indeed 0
    OCArrayAppendValue(marr_zero_cap, s1);
    ASSERT_EQUAL(OCArrayGetCount(marr_zero_cap), 1, "OCArrayCreateMutable with 0 capacity, count after append mismatch");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr_zero_cap, 0), s1), "OCArrayCreateMutable with 0 capacity, value after append mismatch");
    OCRelease(marr_zero_cap);

    // Test OCArrayCreateMutableCopy with empty array
    OCMutableArrayRef marr_empty_copy = OCArrayCreateMutableCopy(arr_empty_explicit_null);
    ASSERT_NOT_NULL(marr_empty_copy, "OCArrayCreateMutableCopy with empty array failed");
    ASSERT_EQUAL(OCArrayGetCount(marr_empty_copy), 0, "OCArrayCreateMutableCopy with empty array count mismatch");
    OCRelease(marr_empty_copy);

    OCRelease(s1);
    OCRelease(s2);
    OCRelease(arr1);
    OCRelease(marr1);
    OCRelease(arr_empty_explicit_null); // Release moved here

    fprintf(stderr, "%s end...without problems\n", __func__);
    OCAutoreleasePoolRelease(pool);
    return true;
}

bool arrayTest2_access(void) {
    OCAutoreleasePoolRef pool = OCAutoreleasePoolCreate();
    fprintf(stderr, "%s begin...\n", __func__);

    OCStringRef s1 = OCStringCreateWithCString("apple");
    OCStringRef s2 = OCStringCreateWithCString("banana");
    OCStringRef s3 = OCStringCreateWithCString("cherry");
    const void *values[] = {s1, s2, s3};
    OCArrayRef arr = OCArrayCreate(values, 3, &kOCTypeArrayCallBacks);
    ASSERT_NOT_NULL(arr, "Failed to create array for access tests");

    // Test OCArrayGetCount
    ASSERT_EQUAL(OCArrayGetCount(arr), 3, "OCArrayGetCount basic test failed");

    // Test OCArrayGetValueAtIndex
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(arr, 0), s1), "OCArrayGetValueAtIndex(0) failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(arr, 1), s2), "OCArrayGetValueAtIndex(1) failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(arr, 2), s3), "OCArrayGetValueAtIndex(2) failed");

    // Test OCArrayGetValueAtIndex (out of bounds)
    ASSERT_NULL(OCArrayGetValueAtIndex(arr, 3), "OCArrayGetValueAtIndex out of bounds (upper) should return NULL or be handled");
    ASSERT_NULL(OCArrayGetValueAtIndex(arr, (uint64_t)-1), "OCArrayGetValueAtIndex out of bounds (large index) should return NULL or be handled");

    // Test with an empty array
    OCArrayRef empty_arr = OCArrayCreate(NULL, 0, &kOCTypeArrayCallBacks);
    ASSERT_NOT_NULL(empty_arr, "Failed to create empty array for access tests");
    ASSERT_EQUAL(OCArrayGetCount(empty_arr), 0, "OCArrayGetCount for empty array failed");
    ASSERT_NULL(OCArrayGetValueAtIndex(empty_arr, 0), "OCArrayGetValueAtIndex on empty array should return NULL or be handled");

    // Test OCArrayGetFirstIndexOfValue
    OCNumberRef n1 = OCNumberCreateWithSInt32(10);
    OCNumberRef n2 = OCNumberCreateWithSInt32(20);
    OCNumberRef n3 = OCNumberCreateWithSInt32(10); // Same value as n1, different instance
    OCNumberRef n4 = OCNumberCreateWithSInt32(30);
    const void *num_values[] = {n1, n2, n3};
    OCArrayRef num_arr = OCArrayCreate(num_values, 3, &kOCTypeArrayCallBacks);
    ASSERT_NOT_NULL(num_arr, "Failed to create number array for access tests");

    ASSERT_EQUAL(OCArrayGetFirstIndexOfValue(num_arr, n1), 0, "OCArrayGetFirstIndexOfValue (n1) failed");
    ASSERT_EQUAL(OCArrayGetFirstIndexOfValue(num_arr, n2), 1, "OCArrayGetFirstIndexOfValue (n2) failed");
    ASSERT_EQUAL(OCArrayGetFirstIndexOfValue(num_arr, n3), 0, "OCArrayGetFirstIndexOfValue (n3, should find n1 due to OCEqual) failed");
    ASSERT_EQUAL(OCArrayGetFirstIndexOfValue(num_arr, n4), kOCNotFound, "OCArrayGetFirstIndexOfValue (n4, not in array) failed");

    // Test OCArrayGetFirstIndexOfValue on empty array
    ASSERT_EQUAL(OCArrayGetFirstIndexOfValue(empty_arr, n1), kOCNotFound, "OCArrayGetFirstIndexOfValue on empty array failed");

    // Test OCArrayContainsValue
    ASSERT_TRUE(OCArrayContainsValue(num_arr, n1), "OCArrayContainsValue (n1) failed");
    ASSERT_TRUE(OCArrayContainsValue(num_arr, n3), "OCArrayContainsValue (n3, should find n1) failed");
    ASSERT_FALSE(OCArrayContainsValue(num_arr, n4), "OCArrayContainsValue (n4, not in array) failed");
    ASSERT_FALSE(OCArrayContainsValue(empty_arr, n1), "OCArrayContainsValue on empty array failed");

    OCRelease(s1);
    OCRelease(s2);
    OCRelease(s3);
    OCRelease(arr);
    OCRelease(n1);
    OCRelease(n2);
    OCRelease(n3);
    OCRelease(n4);
    OCRelease(num_arr);
    OCRelease(empty_arr); // Release moved here

    fprintf(stderr, "%s end...without problems\n", __func__);
    OCAutoreleasePoolRelease(pool);
    return true;
}

bool arrayTest3_modification(void) {
    OCAutoreleasePoolRef pool = OCAutoreleasePoolCreate();
    fprintf(stderr, "%s begin...\n", __func__);

    OCStringRef s1 = OCStringCreateWithCString("a");
    OCStringRef s2 = OCStringCreateWithCString("b");
    OCStringRef s3 = OCStringCreateWithCString("c");
    OCStringRef s4 = OCStringCreateWithCString("d");
    OCStringRef s_extra = OCStringCreateWithCString("extra");

    OCMutableArrayRef marr = OCArrayCreateMutable(2, &kOCTypeArrayCallBacks);
    ASSERT_NOT_NULL(marr, "Failed to create mutable array for modification tests");

    // Test OCArrayAppendValue
    OCArrayAppendValue(marr, s1); // a
    ASSERT_EQUAL(OCArrayGetCount(marr), 1, "AppendValue count failed (1)");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 0), s1), "AppendValue value failed (1)");

    OCArrayAppendValue(marr, s2); // a, b
    ASSERT_EQUAL(OCArrayGetCount(marr), 2, "AppendValue count failed (2)");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 1), s2), "AppendValue value failed (2)");

    OCArrayAppendValue(marr, s3); // a, b, c (should trigger reallocation if initial capacity was 2)
    ASSERT_EQUAL(OCArrayGetCount(marr), 3, "AppendValue count failed (3)");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 2), s3), "AppendValue value failed (3)");

    // Test OCArrayInsertValueAtIndex
    // marr is [a, b, c]
    OCArrayInsertValueAtIndex(marr, 1, s4); // a, d, b, c
    ASSERT_EQUAL(OCArrayGetCount(marr), 4, "InsertValueAtIndex count failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 0), s1), "InsertValueAtIndex value s1 moved? failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 1), s4), "InsertValueAtIndex value s4 failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 2), s2), "InsertValueAtIndex value s2 shifted failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 3), s3), "InsertValueAtIndex value s3 shifted failed");

    OCArrayInsertValueAtIndex(marr, 0, s_extra); // extra, a, d, b, c
    ASSERT_EQUAL(OCArrayGetCount(marr), 5, "InsertValueAtIndex at 0 count failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 0), s_extra), "InsertValueAtIndex at 0 value failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 1), s1), "InsertValueAtIndex at 0, s1 shifted failed");

    OCArrayInsertValueAtIndex(marr, 5, s_extra); // extra, a, d, b, c, extra (insert at end)
    ASSERT_EQUAL(OCArrayGetCount(marr), 6, "InsertValueAtIndex at end count failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 5), s_extra), "InsertValueAtIndex at end value failed");

    // Test OCArrayRemoveValueAtIndex
    // marr is [extra, a, d, b, c, extra]
    OCArrayRemoveValueAtIndex(marr, 1); // extra, d, b, c, extra (removed 'a')
    ASSERT_EQUAL(OCArrayGetCount(marr), 5, "RemoveValueAtIndex count failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 0), s_extra), "RemoveValueAtIndex value s_extra (0) moved? failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 1), s4), "RemoveValueAtIndex value s4 shifted failed"); // formerly 'd'
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 2), s2), "RemoveValueAtIndex value s2 shifted failed"); // formerly 'b'

    OCArrayRemoveValueAtIndex(marr, 0); // d, b, c, extra (removed first 'extra')
    ASSERT_EQUAL(OCArrayGetCount(marr), 4, "RemoveValueAtIndex at 0 count failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 0), s4), "RemoveValueAtIndex at 0, s4 shifted failed");

    OCArrayRemoveValueAtIndex(marr, 3); // d, b, c (removed last 'extra')
    ASSERT_EQUAL(OCArrayGetCount(marr), 3, "RemoveValueAtIndex at end count failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 2), s3), "RemoveValueAtIndex at end, s3 should be last");

    // Test OCArrayAppendArray
    OCStringRef o1 = OCStringCreateWithCString("other1");
    OCStringRef o2 = OCStringCreateWithCString("other2");
    const void *other_values[] = {o1, o2};
    OCArrayRef other_arr = OCArrayCreate(other_values, 2, &kOCTypeArrayCallBacks);
    ASSERT_NOT_NULL(other_arr, "Failed to create other_arr for AppendArray test");

    // marr is [d, b, c]
    OCArrayAppendArray(marr, other_arr, OCRangeMake(0, 2)); // d, b, c, other1, other2
    ASSERT_EQUAL(OCArrayGetCount(marr), 5, "AppendArray count failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 3), o1), "AppendArray value o1 failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 4), o2), "AppendArray value o2 failed");

    // Append a sub-range
    OCMutableArrayRef marr2 = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    OCArrayAppendArray(marr2, marr, OCRangeMake(1, 3)); // b, c, other1
    ASSERT_EQUAL(OCArrayGetCount(marr2), 3, "AppendArray sub-range count failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr2, 0), s2), "AppendArray sub-range value s2 (b) failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr2, 1), s3), "AppendArray sub-range value s3 (c) failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr2, 2), o1), "AppendArray sub-range value o1 failed");
    OCRelease(marr2);

    // Test removing all values
    OCArrayRemoveValueAtIndex(marr, 0); // b, c, other1, other2
    OCArrayRemoveValueAtIndex(marr, 0); // c, other1, other2
    OCArrayRemoveValueAtIndex(marr, 0); // other1, other2
    OCArrayRemoveValueAtIndex(marr, 0); // other2
    OCArrayRemoveValueAtIndex(marr, 0); // empty
    ASSERT_EQUAL(OCArrayGetCount(marr), 0, "Count after removing all elements should be 0");

    // Test appending to an empty-after-removal array
    OCArrayAppendValue(marr, s_extra);
    ASSERT_EQUAL(OCArrayGetCount(marr), 1, "Count after append to emptied array failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 0), s_extra), "Value after append to emptied array failed");

    OCRelease(s1);
    OCRelease(s2);
    OCRelease(s3);
    OCRelease(s4);
    OCRelease(s_extra);
    OCRelease(o1);
    OCRelease(o2);
    OCRelease(other_arr);
    OCRelease(marr);

    fprintf(stderr, "%s end...without problems\n", __func__);
    OCAutoreleasePoolRelease(pool);
    return true;
}

bool arrayTest4_search_sort(void) {
    OCAutoreleasePoolRef pool = OCAutoreleasePoolCreate();
    fprintf(stderr, "%s begin...\n", __func__);

    OCStringRef s_e = OCStringCreateWithCString("e");
    OCStringRef s_b = OCStringCreateWithCString("b");
    OCStringRef s_a = OCStringCreateWithCString("a");
    OCStringRef s_c = OCStringCreateWithCString("c");
    OCStringRef s_d = OCStringCreateWithCString("d");
    OCStringRef s_f = OCStringCreateWithCString("f"); // Not in initial array

    OCMutableArrayRef marr = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    OCArrayAppendValue(marr, s_e);
    OCArrayAppendValue(marr, s_b);
    OCArrayAppendValue(marr, s_a);
    OCArrayAppendValue(marr, s_c);
    OCArrayAppendValue(marr, s_d);
    // marr is [e, b, a, c, d]

    // Test OCArrayGetFirstIndexOfValue (already partially tested in arrayTest2_access)
    ASSERT_EQUAL(OCArrayGetFirstIndexOfValue(marr, s_a), 2, "GetFirstIndexOfValue (a) before sort failed");
    ASSERT_EQUAL(OCArrayGetFirstIndexOfValue(marr, s_f), kOCNotFound, "GetFirstIndexOfValue (f, not present) failed");

    // Test OCArrayContainsValue (already partially tested in arrayTest2_access)
    ASSERT_TRUE(OCArrayContainsValue(marr, s_c), "ContainsValue (c) before sort failed");
    ASSERT_FALSE(OCArrayContainsValue(marr, s_f), "ContainsValue (f, not present) failed");

    // Test OCArraySortValues
    OCArraySortValues(marr, OCRangeMake(0, OCArrayGetCount(marr)), OCStringCompareAdapter, NULL);
    // marr should now be [a, b, c, d, e]
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 0), s_a), "SortValues: value at index 0 (a) failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 1), s_b), "SortValues: value at index 1 (b) failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 2), s_c), "SortValues: value at index 2 (c) failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 3), s_d), "SortValues: value at index 3 (d) failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr, 4), s_e), "SortValues: value at index 4 (e) failed");

    // Test OCArrayBSearchValues (binary search on sorted array)
    ASSERT_EQUAL(OCArrayBSearchValues(marr, OCRangeMake(0, OCArrayGetCount(marr)), s_a, OCStringCompareAdapter, NULL), 0, "BSearchValues (a) failed");
    ASSERT_EQUAL(OCArrayBSearchValues(marr, OCRangeMake(0, OCArrayGetCount(marr)), s_c, OCStringCompareAdapter, NULL), 2, "BSearchValues (c) failed");
    ASSERT_EQUAL(OCArrayBSearchValues(marr, OCRangeMake(0, OCArrayGetCount(marr)), s_e, OCStringCompareAdapter, NULL), 4, "BSearchValues (e) failed");
    ASSERT_EQUAL(OCArrayBSearchValues(marr, OCRangeMake(0, OCArrayGetCount(marr)), s_f, OCStringCompareAdapter, NULL), kOCNotFound, "BSearchValues (f, not present) failed");

    // Test BSearch on a sub-range
    ASSERT_EQUAL(OCArrayBSearchValues(marr, OCRangeMake(1, 3), s_d, OCStringCompareAdapter, NULL), 3, "BSearchValues (d) in sub-range [b,c,d] failed - expected original index 3");
    ASSERT_EQUAL(OCArrayBSearchValues(marr, OCRangeMake(1, 3), s_a, OCStringCompareAdapter, NULL), kOCNotFound, "BSearchValues (a) in sub-range [b,c,d] (not in range) failed");

    // Test sorting a partial range
    OCMutableArrayRef marr2 = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
    OCArrayAppendValue(marr2, s_e); // 0:e
    OCArrayAppendValue(marr2, s_b); // 1:b
    OCArrayAppendValue(marr2, s_d); // 2:d
    OCArrayAppendValue(marr2, s_a); // 3:a
    OCArrayAppendValue(marr2, s_c); // 4:c
    // marr2 is [e, b, d, a, c]
    OCArraySortValues(marr2, OCRangeMake(1, 3), OCStringCompareAdapter, NULL); // Sort elements at index 1, 2, 3 ([b, d, a])
    // marr2 should be [e, a, b, d, c]
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr2, 0), s_e), "SortValues partial: value at index 0 (e) failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr2, 1), s_a), "SortValues partial: value at index 1 (a) failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr2, 2), s_b), "SortValues partial: value at index 2 (b) failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr2, 3), s_d), "SortValues partial: value at index 3 (d) failed");
    ASSERT_TRUE(OCTypeEqual(OCArrayGetValueAtIndex(marr2, 4), s_c), "SortValues partial: value at index 4 (c) failed");
    OCRelease(marr2);

    OCRelease(s_a);
    OCRelease(s_b);
    OCRelease(s_c);
    OCRelease(s_d);
    OCRelease(s_e);
    OCRelease(s_f);
    OCRelease(marr);

    fprintf(stderr, "%s end...without problems\n", __func__);
    OCAutoreleasePoolRelease(pool);
    return true;
}

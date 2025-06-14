//
//  test_data.c
//  tests
//
//  Created by Philip Grandinetti on 6/17/17.
//  Updated by ChatGPT on 2025-05-11.
//

#include "test_utils.h"
#include "../src/OCData.h"

bool dataTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);

    // Test 1: OCDataCreate and OCDataGetLength
    uint8_t bytes1[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    OCDataRef data1 = OCDataCreate(bytes1, sizeof(bytes1));
    if (!data1) PRINTERROR;
    if (OCDataGetLength(data1) != sizeof(bytes1)) PRINTERROR;
    const uint8_t *ptr1 = OCDataGetBytesPtr(data1);
    if (!ptr1) PRINTERROR;
    for (size_t i = 0; i < sizeof(bytes1); ++i) {
        if (ptr1[i] != bytes1[i]) PRINTERROR;
    }

    // Test 2: OCDataCreateWithBytesNoCopy (heap-allocate the source buffer)
    size_t len2 = 3;
    uint8_t *bytes2 = malloc(len2);
    if (!bytes2) { OCRelease(data1); PRINTERROR; }
    bytes2[0] = 0x0A;
    bytes2[1] = 0x0B;
    bytes2[2] = 0x0C;

    OCDataRef data2 = OCDataCreateWithBytesNoCopy(bytes2, len2);
    if (!data2) { free(bytes2); 
    OCRelease(data1); 
    PRINTERROR; }
    if (OCDataGetLength(data2) != len2) { OCRelease(data2); OCRelease(data1); PRINTERROR; }
    const uint8_t *ptr2 = OCDataGetBytesPtr(data2);
    for (size_t i = 0; i < len2; ++i) {
        if (ptr2[i] != bytes2[i]) { OCRelease(data2); OCRelease(data1); PRINTERROR; }
    }
    // When we release data2, ownership of bytes2 is transferred to the OCData object,
    // so we must not free(bytes2) here.
    OCRelease(data2);

    // Test 3: OCDataCreateCopy
    OCDataRef data3 = OCDataCreateCopy(data1);
    if (!data3) { OCRelease(data1); PRINTERROR; }
    if (OCDataGetLength(data3) != OCDataGetLength(data1)) { OCRelease(data3); OCRelease(data1); PRINTERROR; }
    const uint8_t *ptr3 = OCDataGetBytesPtr(data3);
    if (ptr3 == ptr1) { OCRelease(data3); OCRelease(data1); PRINTERROR; }
    for (size_t i = 0; i < OCDataGetLength(data3); ++i) {
        if (ptr3[i] != ptr1[i]) { OCRelease(data3); OCRelease(data1); PRINTERROR; }
    }
    OCRelease(data3);

    // Clean up
    OCRelease(data1);

    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

// Consolidated dataTest1 from test_data_mutable.c
bool dataTest1(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    OCMutableDataRef mdata = OCDataCreateMutable(5);
    ASSERT_NOT_NULL(mdata, "Test 1.1: OCDataCreateMutable should not return NULL");
    ASSERT_EQUAL(OCDataGetLength(mdata), 0, "Test 1.2: New mutable data length should be 0");
    OCDataSetLength(mdata, 3);
    ASSERT_EQUAL(OCDataGetLength(mdata), 3, "Test 1.3: Length after OCDataSetLength should be correct");
    OCDataIncreaseLength(mdata, 2);
    ASSERT_EQUAL(OCDataGetLength(mdata), 5, "Test 1.4: Length after OCDataIncreaseLength should be correct");
    uint8_t extra[] = {10, 20, 30};
    OCDataAppendBytes(mdata, extra, sizeof(extra));
    ASSERT_EQUAL(OCDataGetLength(mdata), 8, "Test 1.5: Length after OCDataAppendBytes should be correct");
    uint8_t buffer[8] = {0};
    OCDataGetBytes((OCDataRef)mdata, OCRangeMake(0, OCDataGetLength(mdata)), buffer);
    ASSERT_TRUE(buffer[5] == 10 && buffer[6] == 20 && buffer[7] == 30, "Test 1.6: OCDataGetBytes should return appended data");
    uint8_t *mutPtr = OCDataGetMutableBytesPtr(mdata);
    ASSERT_NOT_NULL(mutPtr, "Test 1.7: OCDataGetMutableBytesPtr should not return NULL");
    mutPtr[0] = 55;
    ASSERT_EQUAL(OCDataGetBytesPtr((OCDataRef)mdata)[0], 55, "Test 1.8: OCDataGetMutableBytesPtr should allow writing data");
    OCRelease(mdata);
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool dataTest_deepcopy(void) {
    fprintf(stderr, "%s begin...\n", __func__);

    uint8_t bytes[] = {0xDE, 0xAD, 0xBE, 0xEF};
    OCDataRef orig = OCDataCreate(bytes, sizeof(bytes));
    if (!orig) PRINTERROR;

    // Immutable deep copy
    OCDataRef copy = (OCDataRef)OCTypeDeepCopy(orig);
    ASSERT_NOT_NULL(copy, "OCTypeDeepCopy should not return NULL");
    ASSERT_TRUE(OCTypeEqual(orig, copy), "OCTypeDeepCopy should return equal data");
    ASSERT_TRUE(OCDataGetBytesPtr(orig) != OCDataGetBytesPtr(copy), "OCTypeDeepCopy should return different memory");
    OCRelease(copy);

    // Mutable deep copy
    OCMutableDataRef mcopy = (OCMutableDataRef)OCTypeDeepCopyMutable(orig);
    ASSERT_NOT_NULL(mcopy, "OCTypeDeepCopyMutable should not return NULL");
    ASSERT_TRUE(OCTypeEqual(orig, mcopy), "OCTypeDeepCopyMutable should return equal data");
    ASSERT_TRUE(OCDataGetBytesPtr(orig) != OCDataGetBytesPtr(mcopy), "OCTypeDeepCopyMutable should return different memory");

    // Try modifying
    uint8_t *ptr = OCDataGetMutableBytesPtr(mcopy);
    ptr[0] = 0xAB;
    ASSERT_TRUE(OCDataGetBytesPtr(mcopy)[0] == 0xAB, "Modified byte should be reflected in mutable deep copy");
    ASSERT_TRUE(OCDataGetBytesPtr(orig)[0] != 0xAB, "Original should remain unchanged");

    OCRelease(orig);
    OCRelease(mcopy);

    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}
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
    const uint8_t *ptr1 = OCDataGetBytePtr(data1);
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
    if (!data2) { free(bytes2); OCRelease(data1); PRINTERROR; }
    if (OCDataGetLength(data2) != len2) { OCRelease(data2); OCRelease(data1); PRINTERROR; }
    const uint8_t *ptr2 = OCDataGetBytePtr(data2);
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
    const uint8_t *ptr3 = OCDataGetBytePtr(data3);
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

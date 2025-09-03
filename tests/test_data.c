//
//  test_data.c
//  tests
//
//  Created by Philip Grandinetti on 6/17/17.
//  Updated by ChatGPT on 2025-05-11.
//
#include <string.h>  // for memcmp
#include "../src/OCData.h"
#include "../src/OCString.h"  // For OCStringGetCString
#include "test_utils.h"
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
    if (!bytes2) {
        OCRelease(data1);
        PRINTERROR;
    }
    bytes2[0] = 0x0A;
    bytes2[1] = 0x0B;
    bytes2[2] = 0x0C;
    OCDataRef data2 = OCDataCreateWithBytesNoCopy(bytes2, len2);
    if (!data2) {
        free(bytes2);
        OCRelease(data1);
        PRINTERROR;
    }
    if (OCDataGetLength(data2) != len2) {
        OCRelease(data2);
        OCRelease(data1);
        PRINTERROR;
    }
    const uint8_t *ptr2 = OCDataGetBytesPtr(data2);
    for (size_t i = 0; i < len2; ++i) {
        if (ptr2[i] != bytes2[i]) {
            OCRelease(data2);
            OCRelease(data1);
            PRINTERROR;
        }
    }
    // When we release data2, ownership of bytes2 is transferred to the OCData object,
    // so we must not free(bytes2) here.
    OCRelease(data2);
    // Test 3: OCDataCreateCopy
    OCDataRef data3 = OCDataCreateCopy(data1);
    if (!data3) {
        OCRelease(data1);
        PRINTERROR;
    }
    if (OCDataGetLength(data3) != OCDataGetLength(data1)) {
        OCRelease(data3);
        OCRelease(data1);
        PRINTERROR;
    }
    const uint8_t *ptr3 = OCDataGetBytesPtr(data3);
    if (ptr3 == ptr1) {
        OCRelease(data3);
        OCRelease(data1);
        PRINTERROR;
    }
    for (size_t i = 0; i < OCDataGetLength(data3); ++i) {
        if (ptr3[i] != ptr1[i]) {
            OCRelease(data3);
            OCRelease(data1);
            PRINTERROR;
        }
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
    uint8_t *mutPtr = OCDataGetMutableBytes(mdata);
    ASSERT_NOT_NULL(mutPtr, "Test 1.7: OCDataGetMutableBytes should not return NULL");
    mutPtr[0] = 55;
    ASSERT_EQUAL(OCDataGetBytesPtr((OCDataRef)mdata)[0], 55, "Test 1.8: OCDataGetMutableBytes should allow writing data");
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
    uint8_t *ptr = OCDataGetMutableBytes(mcopy);
    ptr[0] = 0xAB;
    ASSERT_TRUE(OCDataGetBytesPtr(mcopy)[0] == 0xAB, "Modified byte should be reflected in mutable deep copy");
    ASSERT_TRUE(OCDataGetBytesPtr(orig)[0] != 0xAB, "Original should remain unchanged");
    OCRelease(orig);
    OCRelease(mcopy);
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}
bool dataTest_base64_roundtrip(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    // Test 1: Simple case - 16 bytes (2 doubles)
    uint8_t test_data1[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x3F,  // 1.0 as double
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40   // 2.0 as double
    };
    size_t len1 = sizeof(test_data1);
    OCDataRef original1 = OCDataCreate(test_data1, len1);
    ASSERT_NOT_NULL(original1, "Original data should not be NULL");
    ASSERT_EQUAL(OCDataGetLength(original1), len1, "Original data length should match");
    // Encode to base64
    OCStringRef b64_1 = OCDataCreateBase64EncodedString(original1, OCBase64EncodingOptionsNone);
    ASSERT_NOT_NULL(b64_1, "Base64 encoding should not return NULL");
    printf("Original length: %zu\n", len1);
    printf("Base64 string: %s\n", OCStringGetCString(b64_1));
    // Decode back from base64
    OCDataRef decoded1 = OCDataCreateFromBase64EncodedString(b64_1);
    ASSERT_NOT_NULL(decoded1, "Base64 decoding should not return NULL");
    printf("Decoded length: %llu\n", OCDataGetLength(decoded1));
    // Check if lengths match
    ASSERT_EQUAL(OCDataGetLength(original1), OCDataGetLength(decoded1),
                 "Original and decoded lengths should match");
    // Check if data matches byte-by-byte
    const uint8_t *orig_bytes = OCDataGetBytesPtr(original1);
    const uint8_t *decoded_bytes = OCDataGetBytesPtr(decoded1);
    for (size_t i = 0; i < len1; i++) {
        if (orig_bytes[i] != decoded_bytes[i]) {
            printf("Data mismatch at byte %zu: original=0x%02X, decoded=0x%02X\n",
                   i, orig_bytes[i], decoded_bytes[i]);
            PRINTERROR;
        }
    }
    OCRelease(original1);
    OCRelease(b64_1);
    OCRelease(decoded1);
    // Test 2: Various lengths to test padding
    for (size_t test_len = 1; test_len <= 20; test_len++) {
        uint8_t *test_data = malloc(test_len);
        for (size_t i = 0; i < test_len; i++) {
            test_data[i] = (uint8_t)(i & 0xFF);
        }
        OCDataRef original = OCDataCreate(test_data, test_len);
        OCStringRef b64 = OCDataCreateBase64EncodedString(original, OCBase64EncodingOptionsNone);
        OCDataRef decoded = OCDataCreateFromBase64EncodedString(b64);
        if (OCDataGetLength(original) != OCDataGetLength(decoded)) {
            printf("Length mismatch for test_len=%zu: original=%llu, decoded=%llu\n",
                   test_len, OCDataGetLength(original), OCDataGetLength(decoded));
            free(test_data);
            OCRelease(original);
            OCRelease(b64);
            OCRelease(decoded);
            PRINTERROR;
        }
        const uint8_t *orig_ptr = OCDataGetBytesPtr(original);
        const uint8_t *decoded_ptr = OCDataGetBytesPtr(decoded);
        if (memcmp(orig_ptr, decoded_ptr, test_len) != 0) {
            printf("Data mismatch for test_len=%zu\n", test_len);
            free(test_data);
            OCRelease(original);
            OCRelease(b64);
            OCRelease(decoded);
            PRINTERROR;
        }
        free(test_data);
        OCRelease(original);
        OCRelease(b64);
        OCRelease(decoded);
    }
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

bool dataTest_json_encoding(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    
    // Create test data
    uint8_t testBytes[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0x57, 0x6F, 0x72, 0x6C, 0x64, 0xFF, 0xFE};
    size_t dataLength = sizeof(testBytes);
    
    OCMutableDataRef originalData = OCDataCreateMutable(dataLength);
    if (!originalData) PRINTERROR;
    
    OCDataAppendBytes(originalData, testBytes, dataLength);
    
    // Test 1: Default encoding (should be OCJSONEncodingBase64 for OCData)
    OCJSONEncoding defaultEncoding = OCDataCopyEncoding(originalData);
    if (defaultEncoding != OCJSONEncodingBase64) {
        fprintf(stderr, "FAIL: Default encoding should be OCJSONEncodingBase64\n");
        OCRelease(originalData);
        return false;
    }
    
    // Test 2: Serialize with default encoding (base64)
    cJSON *jsonDefault = OCDataCopyAsJSON(originalData, true);
    if (!jsonDefault) {
        fprintf(stderr, "FAIL: Could not serialize OCData with default encoding\n");
        OCRelease(originalData);
        return false;
    }
    
    // Verify JSON structure
    cJSON *type = cJSON_GetObjectItem(jsonDefault, "type");
    cJSON *encoding = cJSON_GetObjectItem(jsonDefault, "encoding");
    cJSON *value = cJSON_GetObjectItem(jsonDefault, "value");
    
    if (!type || !cJSON_IsString(type) || strcmp(cJSON_GetStringValue(type), "OCData") != 0) {
        fprintf(stderr, "FAIL: Invalid type field\n");
        cJSON_Delete(jsonDefault);
        OCRelease(originalData);
        return false;
    }
    
    if (!encoding || !cJSON_IsString(encoding) || strcmp(cJSON_GetStringValue(encoding), "base64") != 0) {
        fprintf(stderr, "FAIL: Invalid encoding field\n");
        cJSON_Delete(jsonDefault);
        OCRelease(originalData);
        return false;
    }
    
    if (!value || !cJSON_IsString(value)) {
        fprintf(stderr, "FAIL: Invalid value field\n");
        cJSON_Delete(jsonDefault);
        OCRelease(originalData);
        return false;
    }
    
    // Test 3: Roundtrip with base64 encoding
    OCStringRef error = NULL;
    OCDataRef deserializedData = OCDataCreateFromJSON(jsonDefault, &error);
    if (!deserializedData) {
        fprintf(stderr, "FAIL: Could not deserialize OCData: %s\n", 
                error ? OCStringGetCString(error) : "unknown error");
        if (error) OCRelease(error);
        cJSON_Delete(jsonDefault);
        OCRelease(originalData);
        return false;
    }
    
    // Verify data equality
    if (!OCTypeEqual(originalData, deserializedData)) {
        fprintf(stderr, "FAIL: Roundtrip data does not match original\n");
        OCRelease(deserializedData);
        cJSON_Delete(jsonDefault);
        OCRelease(originalData);
        return false;
    }
    
    // Verify encoding is preserved
    OCJSONEncoding deserializedEncoding = OCDataCopyEncoding(deserializedData);
    if (deserializedEncoding != OCJSONEncodingBase64) {
        fprintf(stderr, "FAIL: Encoding not preserved in roundtrip\n");
        OCRelease(deserializedData);
        cJSON_Delete(jsonDefault);
        OCRelease(originalData);
        return false;
    }
    
    // Test 4: Test encoding setter (though OCData can only use base64)
    OCDataSetEncoding(originalData, OCJSONEncodingNone);
    OCJSONEncoding newEncoding = OCDataCopyEncoding(originalData);
    if (newEncoding != OCJSONEncodingNone) {
        fprintf(stderr, "FAIL: Could not set encoding to OCJSONEncodingNone\n");
        OCRelease(deserializedData);
        cJSON_Delete(jsonDefault);
        OCRelease(originalData);
        return false;
    }
    
    // Test 5: Serialize with OCJSONEncodingNone (should still use base64 for JSON compatibility)
    cJSON *jsonNone = OCDataCopyAsJSON(originalData, true);
    if (!jsonNone) {
        fprintf(stderr, "FAIL: Could not serialize OCData with OCJSONEncodingNone\n");
        OCRelease(deserializedData);
        cJSON_Delete(jsonDefault);
        OCRelease(originalData);
        return false;
    }
    
    // Even with OCJSONEncodingNone, OCData should still use base64 for JSON
    cJSON *encodingNone = cJSON_GetObjectItem(jsonNone, "encoding");
    if (encodingNone && cJSON_IsString(encodingNone)) {
        // If encoding field exists, it should be "base64" or "none"
        const char *encodingStr = cJSON_GetStringValue(encodingNone);
        if (strcmp(encodingStr, "base64") != 0 && strcmp(encodingStr, "none") != 0) {
            fprintf(stderr, "FAIL: Invalid encoding value: %s\n", encodingStr);
            cJSON_Delete(jsonNone);
            OCRelease(deserializedData);
            cJSON_Delete(jsonDefault);
            OCRelease(originalData);
            return false;
        }
    }
    
    // Cleanup
    cJSON_Delete(jsonNone);
    cJSON_Delete(jsonDefault);
    OCRelease(deserializedData);
    OCRelease(originalData);
    
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

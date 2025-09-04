#include <complex.h>   // for float complex / double complex
#include <inttypes.h>  // Ensure this is included at the top
#include <stdint.h>
#include <string.h>
#include "../src/OCNumber.h"
#include "../src/OCString.h"
#include "../src/OCType.h"
#include "../src/OCData.h"
#include "../src/OCArray.h"
#include "test_utils.h"
#define OCTypeEqual(a, b) OCTypeEqual((const void *)(a), (const void *)(b))
bool numberTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    // ---- Diagnostic: verify OCNumberTypeSize returns correct values for all OCNumberTypes ----
    struct {
        OCNumberType type;
        int expectedSize;
        const char *label;
    } size_tests[] = {
        {kOCNumberUInt8Type, sizeof(uint8_t), "UInt8"},
        {kOCNumberSInt8Type, sizeof(int8_t), "SInt8"},
        {kOCNumberUInt16Type, sizeof(uint16_t), "UInt16"},
        {kOCNumberSInt16Type, sizeof(int16_t), "SInt16"},
        {kOCNumberUInt32Type, sizeof(uint32_t), "UInt32"},
        {kOCNumberSInt32Type, sizeof(int32_t), "SInt32"},
        {kOCNumberUInt64Type, sizeof(uint64_t), "UInt64"},
        {kOCNumberSInt64Type, sizeof(int64_t), "SInt64"},
        {kOCNumberFloat32Type, sizeof(float), "Float32"},
        {kOCNumberFloat64Type, sizeof(double), "Float64"},
        {kOCNumberComplex64Type, sizeof(float complex), "Complex64"},
        {kOCNumberComplex128Type, sizeof(double complex), "Complex128"},
    };
    bool all_ok = true;
    fprintf(stderr, "BEGIN: OCNumberTypeSize() diagnostics\n");
    for (size_t i = 0; i < sizeof(size_tests) / sizeof(size_tests[0]); ++i) {
        int actualSize = OCNumberTypeSize(size_tests[i].type);
        int expectedSize = size_tests[i].expectedSize;
        if (actualSize == 0) {
            fprintf(stderr, "ERROR: OCNumberTypeSize(%s) returned 0 — possibly unhandled type!\n", size_tests[i].label);
            all_ok = false;
            PRINTERROR;
        } else if (actualSize != expectedSize) {
            fprintf(stderr, "ERROR: OCNumberTypeSize(%s) = %d, expected %d\n",
                    size_tests[i].label, actualSize, expectedSize);
            all_ok = false;
            PRINTERROR;
        } else {
            fprintf(stderr, "PASS : OCNumberTypeSize(%s) = %d\n", size_tests[i].label, actualSize);
        }
    }
    if (all_ok) {
        fprintf(stderr, "PASS : All OCNumberTypeSize checks passed\n");
    } else {
        fprintf(stderr, "FAIL : One or more OCNumberTypeSize checks failed\n");
    }
    fprintf(stderr, "END  : OCNumberTypeSize() diagnostics\n");
    // everyone should share the same TypeID
    OCTypeID numTypeID = OCNumberGetTypeID();
    if (numTypeID == kOCNotATypeID) PRINTERROR;
    // --- signed integers ---
    struct {
        int64_t v;
        OCNumberType t;
        const char *s;
    } signed_tests[] = {
        {INT8_MIN, kOCNumberSInt8Type, "-128"},
        {INT16_MIN, kOCNumberSInt16Type, "-32768"},
        {INT32_MIN, kOCNumberSInt32Type, "-2147483648"},
        {INT64_MIN, kOCNumberSInt64Type, "-9223372036854775808"},
    };
    for (size_t i = 0; i < sizeof signed_tests / sizeof *signed_tests; ++i) {
        int64_t v = signed_tests[i].v;
        OCNumberType t = signed_tests[i].t;
        const char *exp_s = signed_tests[i].s;
        OCNumberRef n = NULL;
        switch (t) {
            case kOCNumberSInt8Type:
                n = OCNumberCreateWithSInt8((int8_t)v);
                break;
            case kOCNumberSInt16Type:
                n = OCNumberCreateWithSInt16((int16_t)v);
                break;
            case kOCNumberSInt32Type:
                n = OCNumberCreateWithSInt32((int32_t)v);
                break;
            case kOCNumberSInt64Type:
                n = OCNumberCreateWithSInt64(v);
                break;
            default:
                PRINTERROR;
        }
        if (!n) PRINTERROR;
        // TypeID check
        if (OCGetTypeID(n) != numTypeID) {
            fprintf(stderr,
                    "DEBUG: got TypeID %" PRIu64 ", expected %" PRIu64 "\n",
                    (uint64_t)OCGetTypeID(n),
                    (uint64_t)numTypeID);
            OCRelease(n);
            PRINTERROR;
        }
        // Size check
        int got_sz = OCNumberTypeSize(t);
        int exp_sz = (t == kOCNumberSInt8Type    ? sizeof(int8_t)
                      : t == kOCNumberSInt16Type ? sizeof(int16_t)
                      : t == kOCNumberSInt32Type ? sizeof(int32_t)
                                                 : sizeof(int64_t));
        if (got_sz != exp_sz) {
            fprintf(stderr,
                    "DEBUG: for signed type %d size() == %d, want %d\n",
                    (int)t, got_sz, exp_sz);
            OCRelease(n);
            PRINTERROR;
        }
        // String repr
        OCStringRef str = OCNumberCreateStringValue(n);
        OCStringRef expected = OCStringCreateWithCString(exp_s);
        const char *got_c = OCStringGetCString(str);
        const char *exp_c = OCStringGetCString(expected);
        if (!OCStringEqual(str, expected)) {
            fprintf(stderr,
                    "DEBUG: signed %" PRId64 " → got \"%s\", expected \"%s\"\n",
                    v, got_c, exp_c);
            OCRelease(str);
            OCRelease(expected);
            OCRelease(n);
            PRINTERROR;
        }
        OCRelease(str);
        OCRelease(expected);
        OCRelease(n);
    }
    // --- unsigned integers ---
    struct {
        uint64_t v;
        OCNumberType t;
        const char *s;
    } unsigned_tests[] = {
        {UINT8_MAX, kOCNumberUInt8Type, "255"},
        {UINT16_MAX, kOCNumberUInt16Type, "65535"},
        {4000000000U, kOCNumberUInt32Type, "4000000000"},
        {UINT64_MAX, kOCNumberUInt64Type, "18446744073709551615"},
    };
    for (size_t i = 0; i < sizeof unsigned_tests / sizeof *unsigned_tests; ++i) {
        uint64_t v = unsigned_tests[i].v;
        OCNumberType t = unsigned_tests[i].t;
        const char *exp_s = unsigned_tests[i].s;
        OCNumberRef n = NULL;
        switch (t) {
            case kOCNumberUInt8Type:
                n = OCNumberCreateWithUInt8((uint8_t)v);
                break;
            case kOCNumberUInt16Type:
                n = OCNumberCreateWithUInt16((uint16_t)v);
                break;
            case kOCNumberUInt32Type:
                n = OCNumberCreateWithUInt32((uint32_t)v);
                break;
            case kOCNumberUInt64Type:
                n = OCNumberCreateWithUInt64(v);
                break;
            default:
                PRINTERROR;
        }
        if (!n) PRINTERROR;
        OCStringRef str = OCNumberCreateStringValue(n);
        OCStringRef expected = OCStringCreateWithCString(exp_s);
        const char *got_c = OCStringGetCString(str);
        const char *exp_c = OCStringGetCString(expected);
        if (!OCStringEqual(str, expected)) {
            fprintf(stderr,
                    "DEBUG: unsigned %" PRIu64 " → got \"%s\", expected \"%s\"\n",
                    v, got_c, exp_c);
            OCRelease(str);
            OCRelease(expected);
            OCRelease(n);
            PRINTERROR;
        }
        OCRelease(str);
        OCRelease(expected);
        OCRelease(n);
    }
    // --- floating point numbers ---
    struct {
        double v;
        OCNumberType t;
        const char *s;
    } float_tests[] = {
        {3.14f, kOCNumberFloat32Type, "3.14"},
        {2.71828, kOCNumberFloat64Type, "2.71828"},
        {0.0f, kOCNumberFloat32Type, "0"},
        {-123.456, kOCNumberFloat64Type, "-123.456"}};
    for (size_t i = 0; i < sizeof float_tests / sizeof *float_tests; ++i) {
        double v = float_tests[i].v;
        OCNumberType t = float_tests[i].t;
        const char *exp_s = float_tests[i].s;
        OCNumberRef n = (t == kOCNumberFloat32Type)
                            ? OCNumberCreateWithFloat((float)v)
                            : OCNumberCreateWithDouble(v);
        if (!n) PRINTERROR;
        // Size check
        int got_sz = OCNumberTypeSize(t);
        int exp_sz = (t == kOCNumberFloat32Type ? sizeof(float) : sizeof(double));
        if (got_sz != exp_sz) {
            fprintf(stderr,
                    "DEBUG: float type %d size %d, expect %d\n",
                    (int)t, got_sz, exp_sz);
            OCRelease(n);
            PRINTERROR;
        }
        // Prefix-match the textual repr
        OCStringRef str = OCNumberCreateStringValue(n);
        OCStringRef expected = OCStringCreateWithCString(exp_s);
        const char *got_c = OCStringGetCString(str);
        const char *exp_c = OCStringGetCString(expected);
        if (strncmp(got_c, exp_c, strlen(exp_c)) != 0) {
            fprintf(stderr,
                    "DEBUG: float %.17g → got \"%s\", expected prefix \"%s\"\n",
                    v, got_c, exp_c);
            OCRelease(str);
            OCRelease(expected);
            OCRelease(n);
            PRINTERROR;
        }
        OCRelease(str);
        OCRelease(expected);
        OCRelease(n);
    }
    // --- complex numbers ---
    struct {
        double complex v;
        OCNumberType t;
    } complex_tests[] = {
        {1.0f + 2.5f * I, kOCNumberComplex64Type},
        {-3.5 + 4.75 * I, kOCNumberComplex128Type},
        {0.0f + 0.0f * I, kOCNumberComplex64Type},      // zero case
        {1.23e4f - .0567f * I, kOCNumberComplex64Type}  // precision case
    };
    for (size_t i = 0; i < sizeof complex_tests / sizeof *complex_tests; ++i) {
        double complex v = complex_tests[i].v;
        OCNumberType t = complex_tests[i].t;
        // create
        OCNumberRef n = (t == kOCNumberComplex64Type)
                            ? OCNumberCreateWithFloatComplex((float complex)v)
                            : OCNumberCreateWithDoubleComplex(v);
        if (!n) PRINTERROR;
        OCStringRef str = OCNumberCreateStringValue(n);
        const char *got_c = OCStringGetCString(str);
        // decompose using the correct real/imag functions
        double real, imag;
        if (t == kOCNumberComplex64Type) {
            real = crealf((float complex)v);
            imag = cimagf((float complex)v);
        } else {
            real = creal(v);
            imag = cimag(v);
        }
        if (real == 0.0 && imag == 0.0) {
            // must be exactly "0"
            if (strcmp(got_c, "0") != 0) {
                fprintf(stderr,
                        "DEBUG: complex zero → got \"%s\", expected \"0\"\n",
                        got_c);
                OCRelease(str);
                OCRelease(n);
                PRINTERROR;
            }
        } else {
            // build expected with the same format
            char expected_buf[64];
            if (t == kOCNumberComplex64Type) {
                snprintf(expected_buf, sizeof expected_buf,
                         "%.9g+I*%.9g", real, imag);
            } else {
                snprintf(expected_buf, sizeof expected_buf,
                         "%.17g+I*%.17g", real, imag);
            }
            if (strcmp(got_c, expected_buf) != 0) {
                fprintf(stderr,
                        "DEBUG: complex → got \"%s\", expected \"%s\"\n",
                        got_c, expected_buf);
                OCRelease(str);
                OCRelease(n);
                PRINTERROR;
            }
        }
        OCRelease(str);
        OCRelease(n);
    }
    // --- Deep copy tests ---
    {
        OCNumberRef original = OCNumberCreateWithDouble(123.456);
        OCNumberRef copy = (OCNumberRef)OCTypeDeepCopy(original);
        if (!copy || !OCTypeEqual(original, copy) || original == copy) {
            fprintf(stderr, "DEBUG: Deep copy failed (equality or independence)\n");
            if (copy) OCRelease(copy);
            OCRelease(original);
            PRINTERROR;
        }
        OCRelease(copy);
        OCRelease(original);
    }
    // --- OCNumberTypeSize: test invalid / undefined types ---
    {
        OCNumberType bogus_types[] = {
            (OCNumberType)(999),   // clearly invalid
            (OCNumberType)(-999),  // invalid negative
            (OCNumberType)(0),     // possibly unhandled
        };
        for (size_t i = 0; i < sizeof bogus_types / sizeof *bogus_types; ++i) {
            int sz = OCNumberTypeSize(bogus_types[i]);
            if (sz != 0) {
                fprintf(stderr, "ERROR: OCNumberTypeSize(%d) returned %d, expected 0\n",
                        (int)bogus_types[i], sz);
                PRINTERROR;
            }
        }
    }
    
    // --- OCNumberCreateArrayFromData tests ---
    {
        fprintf(stderr, "BEGIN: OCNumberCreateArrayFromData tests\n");
        
        // Test 1: Create array from uint32 data
        {
            uint32_t testData[] = {100, 200, 300, 400};
            OCDataRef data = OCDataCreate((const uint8_t*)testData, sizeof(testData));
            if (!data) PRINTERROR;
            
            OCStringRef error = NULL;
            OCArrayRef array = OCNumberCreateArrayFromData(data, kOCNumberUInt32Type, &error);
            if (!array) {
                fprintf(stderr, "ERROR: Failed to create array from uint32 data\n");
                if (error) {
                    fprintf(stderr, "Error: %s\n", OCStringGetCString(error));
                }
                OCRelease(data);
                PRINTERROR;
            }
            
            // Verify array count
            OCIndex count = OCArrayGetCount(array);
            if (count != 4) {
                fprintf(stderr, "ERROR: Expected 4 elements, got %ld\n", (long)count);
                OCRelease(array);
                OCRelease(data);
                PRINTERROR;
            }
            
            // Verify array contents
            for (OCIndex i = 0; i < count; i++) {
                OCNumberRef num = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!num) {
                    fprintf(stderr, "ERROR: NULL number at index %ld\n", (long)i);
                    OCRelease(array);
                    OCRelease(data);
                    PRINTERROR;
                }
                
                uint32_t value;
                if (!OCNumberTryGetUInt32(num, &value)) {
                    fprintf(stderr, "ERROR: Failed to get uint32 value at index %ld\n", (long)i);
                    OCRelease(array);
                    OCRelease(data);
                    PRINTERROR;
                }
                
                if (value != testData[i]) {
                    fprintf(stderr, "ERROR: Expected %u, got %u at index %ld\n", 
                            testData[i], value, (long)i);
                    OCRelease(array);
                    OCRelease(data);
                    PRINTERROR;
                }
            }
            
            OCRelease(array);
            OCRelease(data);
            fprintf(stderr, "PASS : uint32 array creation test\n");
        }
        
        // Test 2: Create array from float data
        {
            float testData[] = {1.5f, 2.5f, 3.5f};
            OCDataRef data = OCDataCreate((const uint8_t*)testData, sizeof(testData));
            if (!data) PRINTERROR;
            
            OCStringRef error = NULL;
            OCArrayRef array = OCNumberCreateArrayFromData(data, kOCNumberFloat32Type, &error);
            if (!array) {
                fprintf(stderr, "ERROR: Failed to create array from float data\n");
                OCRelease(data);
                PRINTERROR;
            }
            
            OCIndex count = OCArrayGetCount(array);
            if (count != 3) {
                fprintf(stderr, "ERROR: Expected 3 elements, got %ld\n", (long)count);
                OCRelease(array);
                OCRelease(data);
                PRINTERROR;
            }
            
            for (OCIndex i = 0; i < count; i++) {
                OCNumberRef num = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                float value;
                if (!OCNumberTryGetFloat32(num, &value)) {
                    fprintf(stderr, "ERROR: Failed to get float value at index %ld\n", (long)i);
                    OCRelease(array);
                    OCRelease(data);
                    PRINTERROR;
                }
                
                if (value != testData[i]) {
                    fprintf(stderr, "ERROR: Expected %f, got %f at index %ld\n", 
                            testData[i], value, (long)i);
                    OCRelease(array);
                    OCRelease(data);
                    PRINTERROR;
                }
            }
            
            OCRelease(array);
            OCRelease(data);
            fprintf(stderr, "PASS : float32 array creation test\n");
        }
        
        // Test 3: Error handling - NULL data
        {
            OCStringRef error = NULL;
            OCArrayRef array = OCNumberCreateArrayFromData(NULL, kOCNumberUInt32Type, &error);
            if (array) {
                fprintf(stderr, "ERROR: Expected NULL for NULL data input\n");
                OCRelease(array);
                PRINTERROR;
            }
            if (!error) {
                fprintf(stderr, "ERROR: Expected error message for NULL data\n");
                PRINTERROR;
            }
            fprintf(stderr, "PASS : NULL data error handling test\n");
        }
        
        // Test 4: Error handling - invalid type size
        {
            uint8_t testData[] = {1, 2, 3};  // 3 bytes
            OCDataRef data = OCDataCreate(testData, sizeof(testData));
            if (!data) PRINTERROR;
            
            OCStringRef error = NULL;
            // Try to create uint32 array from 3 bytes (not divisible by 4)
            OCArrayRef array = OCNumberCreateArrayFromData(data, kOCNumberUInt32Type, &error);
            if (array) {
                fprintf(stderr, "ERROR: Expected NULL for incompatible data size\n");
                OCRelease(array);
                OCRelease(data);
                PRINTERROR;
            }
            if (!error) {
                fprintf(stderr, "ERROR: Expected error message for incompatible size\n");
                OCRelease(data);
                PRINTERROR;
            }
            
            OCRelease(data);
            fprintf(stderr, "PASS : Incompatible data size error handling test\n");
        }
        
        // Test 5: Complex numbers
        {
            double complex testData[] = {1.0 + 2.0*I, 3.0 + 4.0*I};
            OCDataRef data = OCDataCreate((const uint8_t*)testData, sizeof(testData));
            if (!data) PRINTERROR;
            
            OCStringRef error = NULL;
            OCArrayRef array = OCNumberCreateArrayFromData(data, kOCNumberComplex128Type, &error);
            if (!array) {
                fprintf(stderr, "ERROR: Failed to create array from complex data\n");
                OCRelease(data);
                PRINTERROR;
            }
            
            OCIndex count = OCArrayGetCount(array);
            if (count != 2) {
                fprintf(stderr, "ERROR: Expected 2 elements, got %ld\n", (long)count);
                OCRelease(array);
                OCRelease(data);
                PRINTERROR;
            }
            
            for (OCIndex i = 0; i < count; i++) {
                OCNumberRef num = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                double complex value;
                if (!OCNumberTryGetComplex128(num, &value)) {
                    fprintf(stderr, "ERROR: Failed to get complex value at index %ld\n", (long)i);
                    OCRelease(array);
                    OCRelease(data);
                    PRINTERROR;
                }
                
                if (creal(value) != creal(testData[i]) || cimag(value) != cimag(testData[i])) {
                    fprintf(stderr, "ERROR: Complex mismatch at index %ld\n", (long)i);
                    OCRelease(array);
                    OCRelease(data);
                    PRINTERROR;
                }
            }
            
            OCRelease(array);
            OCRelease(data);
            fprintf(stderr, "PASS : complex128 array creation test\n");
        }
        
        fprintf(stderr, "END  : OCNumberCreateArrayFromData tests\n");
    }
    
    // --- OCNumberCreateDataFromArray tests ---
    {
        fprintf(stderr, "BEGIN: OCNumberCreateDataFromArray tests\n");
        
        // Test 1: Round-trip test with uint32 data
        {
            uint32_t originalData[] = {100, 200, 300, 400};
            OCDataRef originalOCData = OCDataCreate((const uint8_t*)originalData, sizeof(originalData));
            if (!originalOCData) PRINTERROR;
            
            // Convert to array
            OCStringRef error = NULL;
            OCArrayRef array = OCNumberCreateArrayFromData(originalOCData, kOCNumberUInt32Type, &error);
            if (!array) {
                fprintf(stderr, "ERROR: Failed to create array from data\n");
                OCRelease(originalOCData);
                PRINTERROR;
            }
            
            // Convert back to data
            OCDataRef resultData = OCNumberCreateDataFromArray(array, kOCNumberUInt32Type, &error);
            if (!resultData) {
                fprintf(stderr, "ERROR: Failed to create data from array\n");
                OCRelease(array);
                OCRelease(originalOCData);
                PRINTERROR;
            }
            
            // Verify sizes match
            OCIndex originalLength = OCDataGetLength(originalOCData);
            OCIndex resultLength = OCDataGetLength(resultData);
            if (originalLength != resultLength) {
                fprintf(stderr, "ERROR: Data length mismatch: original %ld, result %ld\n", 
                        (long)originalLength, (long)resultLength);
                OCRelease(resultData);
                OCRelease(array);
                OCRelease(originalOCData);
                PRINTERROR;
            }
            
            // Verify data contents match
            const uint8_t *originalBytes = OCDataGetBytesPtr(originalOCData);
            const uint8_t *resultBytes = OCDataGetBytesPtr(resultData);
            if (memcmp(originalBytes, resultBytes, originalLength) != 0) {
                fprintf(stderr, "ERROR: Data contents do not match\n");
                OCRelease(resultData);
                OCRelease(array);
                OCRelease(originalOCData);
                PRINTERROR;
            }
            
            OCRelease(resultData);
            OCRelease(array);
            OCRelease(originalOCData);
            fprintf(stderr, "PASS : uint32 round-trip test\n");
        }
        
        // Test 2: Direct array to data conversion
        {
            // Create array manually
            OCMutableArrayRef array = OCArrayCreateMutable(3, &kOCTypeArrayCallBacks);
            if (!array) PRINTERROR;
            
            OCNumberRef num1 = OCNumberCreateWithFloat(1.5f);
            OCNumberRef num2 = OCNumberCreateWithFloat(2.5f);
            OCNumberRef num3 = OCNumberCreateWithFloat(3.5f);
            if (!num1 || !num2 || !num3) {
                if (num1) OCRelease(num1);
                if (num2) OCRelease(num2);
                if (num3) OCRelease(num3);
                OCRelease(array);
                PRINTERROR;
            }
            
            OCArrayAppendValue(array, num1);
            OCArrayAppendValue(array, num2);
            OCArrayAppendValue(array, num3);
            
            // Convert to data
            OCStringRef error = NULL;
            OCDataRef data = OCNumberCreateDataFromArray(array, kOCNumberFloat32Type, &error);
            if (!data) {
                fprintf(stderr, "ERROR: Failed to create data from float array\n");
                OCRelease(num3);
                OCRelease(num2);
                OCRelease(num1);
                OCRelease(array);
                PRINTERROR;
            }
            
            // Verify data size
            OCIndex expectedSize = 3 * sizeof(float);
            OCIndex actualSize = OCDataGetLength(data);
            if (actualSize != expectedSize) {
                fprintf(stderr, "ERROR: Expected data size %ld, got %ld\n", 
                        (long)expectedSize, (long)actualSize);
                OCRelease(data);
                OCRelease(num3);
                OCRelease(num2);
                OCRelease(num1);
                OCRelease(array);
                PRINTERROR;
            }
            
            // Verify data contents
            const float *floatData = (const float*)OCDataGetBytesPtr(data);
            float expectedValues[] = {1.5f, 2.5f, 3.5f};
            for (int i = 0; i < 3; i++) {
                if (floatData[i] != expectedValues[i]) {
                    fprintf(stderr, "ERROR: Expected %f, got %f at index %d\n", 
                            expectedValues[i], floatData[i], i);
                    OCRelease(data);
                    OCRelease(num3);
                    OCRelease(num2);
                    OCRelease(num1);
                    OCRelease(array);
                    PRINTERROR;
                }
            }
            
            OCRelease(data);
            OCRelease(num3);
            OCRelease(num2);
            OCRelease(num1);
            OCRelease(array);
            fprintf(stderr, "PASS : float32 direct conversion test\n");
        }
        
        // Test 3: Error handling - NULL array
        {
            OCStringRef error = NULL;
            OCDataRef data = OCNumberCreateDataFromArray(NULL, kOCNumberUInt32Type, &error);
            if (data) {
                fprintf(stderr, "ERROR: Expected NULL for NULL array input\n");
                OCRelease(data);
                PRINTERROR;
            }
            if (!error) {
                fprintf(stderr, "ERROR: Expected error message for NULL array\n");
                PRINTERROR;
            }
            fprintf(stderr, "PASS : NULL array error handling test\n");
        }
        
        // Test 4: Error handling - type mismatch in array
        {
            OCMutableArrayRef array = OCArrayCreateMutable(2, &kOCTypeArrayCallBacks);
            if (!array) PRINTERROR;
            
            OCNumberRef num1 = OCNumberCreateWithUInt32(100);  // uint32
            OCNumberRef num2 = OCNumberCreateWithFloat(2.5f);  // float32 - different type!
            if (!num1 || !num2) {
                if (num1) OCRelease(num1);
                if (num2) OCRelease(num2);
                OCRelease(array);
                PRINTERROR;
            }
            
            OCArrayAppendValue(array, num1);
            OCArrayAppendValue(array, num2);
            
            OCStringRef error = NULL;
            OCDataRef data = OCNumberCreateDataFromArray(array, kOCNumberUInt32Type, &error);
            if (data) {
                fprintf(stderr, "ERROR: Expected NULL for type mismatch\n");
                OCRelease(data);
                OCRelease(num2);
                OCRelease(num1);
                OCRelease(array);
                PRINTERROR;
            }
            if (!error) {
                fprintf(stderr, "ERROR: Expected error message for type mismatch\n");
                OCRelease(num2);
                OCRelease(num1);
                OCRelease(array);
                PRINTERROR;
            }
            
            OCRelease(num2);
            OCRelease(num1);
            OCRelease(array);
            fprintf(stderr, "PASS : Type mismatch error handling test\n");
        }
        
        // Test 5: Empty array test
        {
            OCMutableArrayRef array = OCArrayCreateMutable(0, &kOCTypeArrayCallBacks);
            if (!array) PRINTERROR;
            
            OCStringRef error = NULL;
            OCDataRef data = OCNumberCreateDataFromArray(array, kOCNumberUInt32Type, &error);
            if (!data) {
                fprintf(stderr, "ERROR: Failed to create data from empty array\n");
                OCRelease(array);
                PRINTERROR;
            }
            
            OCIndex dataLength = OCDataGetLength(data);
            if (dataLength != 0) {
                fprintf(stderr, "ERROR: Expected empty data, got length %ld\n", (long)dataLength);
                OCRelease(data);
                OCRelease(array);
                PRINTERROR;
            }
            
            OCRelease(data);
            OCRelease(array);
            fprintf(stderr, "PASS : Empty array test\n");
        }
        
        fprintf(stderr, "END  : OCNumberCreateDataFromArray tests\n");
    }
    
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}

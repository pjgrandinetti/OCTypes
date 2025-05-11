#include "test_utils.h"
#include <stdint.h>
#include <complex.h>       // for float complex / double complex
#include "../src/OCNumber.h"
#include "../src/OCType.h"
#include "../src/OCString.h"

// Declare the unwrapped‐value getter:
extern void OCNumberGetValue(OCNumberRef number, OCNumberType type, void *outValue);

// Map OCEqual → OCTypeEqual so your ASSERTs compile:
#define OCEqual(a,b) OCTypeEqual((const void*)(a),(const void*)(b))

bool numberTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);

    // everyone should share the same TypeID
    OCTypeID numTypeID = OCNumberGetTypeID();
    if (numTypeID == _kOCNotATypeID) PRINTERROR;

    // --- signed integers ---
    struct {
        int64_t      v;
        OCNumberType t;
        const char  *s;
    } signed_tests[] = {
        { INT8_MIN,  kOCNumberSInt8Type,   "-128" },
        { INT16_MIN, kOCNumberSInt16Type,  "-32768" },
        { INT32_MIN, kOCNumberSInt32Type,  "-2147483648" },
        { INT64_MIN, kOCNumberSInt64Type,  "-9223372036854775808" },
    };
    for (size_t i = 0; i < sizeof signed_tests/sizeof *signed_tests; ++i) {
        int64_t v = signed_tests[i].v;
        OCNumberType t = signed_tests[i].t;
        const char *exp_s = signed_tests[i].s;

        // create
        OCNumberRef n = NULL;
        switch (t) {
            case kOCNumberSInt8Type:  n = OCNumberCreateWithSInt8((int8_t)v);  break;
            case kOCNumberSInt16Type: n = OCNumberCreateWithSInt16((int16_t)v); break;
            case kOCNumberSInt32Type: n = OCNumberCreateWithSInt32((int32_t)v); break;
            case kOCNumberSInt64Type: n = OCNumberCreateWithSInt64((int64_t)v); break;
            default: PRINTERROR;
        }
        if (!n) PRINTERROR;

        // TypeID
        if (OCGetTypeID(n) != numTypeID) {
            fprintf(stderr, "DEBUG: got TypeID %llu, expected %llu\n",
                    (unsigned long long)OCGetTypeID(n),
                    (unsigned long long)numTypeID);
            OCRelease(n);
            PRINTERROR;
        }

        // Size
        int got_sz = OCNumberTypeSize(t);
        int exp_sz = (t==kOCNumberSInt8Type  ? sizeof(int8_t)
                      :t==kOCNumberSInt16Type ? sizeof(int16_t)
                      :t==kOCNumberSInt32Type ? sizeof(int32_t)
                                                : sizeof(int64_t));
        if (got_sz != exp_sz) {
            fprintf(stderr, "DEBUG: for signed type %d size() == %d, want %d\n",
                    (int)t, got_sz, exp_sz);
            OCRelease(n);
            PRINTERROR;
        }

        // String repr
        OCStringRef str      = OCNumberCreateStringValue(n);
        OCStringRef expected = OCStringCreateWithCString(exp_s);

        const char *got_c = OCStringGetCString(str);
        const char *exp_c = OCStringGetCString(expected);
        if (!OCStringEqual(str, expected)) {
            fprintf(stderr,
                    "DEBUG: signed %lld → got \"%s\", expected \"%s\"\n",
                    (long long)v, got_c, exp_c);
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
        uint64_t     v;
        OCNumberType t;
        const char  *s;
    } unsigned_tests[] = {
        { UINT8_MAX,   kOCNumberUInt8Type,  "255" },
        { UINT16_MAX,  kOCNumberUInt16Type, "65535" },
        { 4000000000U, kOCNumberUInt32Type, "4000000000" },
        { UINT64_MAX,  kOCNumberUInt64Type, "18446744073709551615" },
    };
    for (size_t i = 0; i < sizeof unsigned_tests/sizeof *unsigned_tests; ++i) {
        uint64_t v = unsigned_tests[i].v;
        OCNumberType t = unsigned_tests[i].t;
        const char *exp_s = unsigned_tests[i].s;

        OCNumberRef n = NULL;
        switch (t) {
            case kOCNumberUInt8Type:  n = OCNumberCreateWithUInt8((uint8_t)v);  break;
            case kOCNumberUInt16Type: n = OCNumberCreateWithUInt16((uint16_t)v); break;
            case kOCNumberUInt32Type: n = OCNumberCreateWithUInt32((uint32_t)v); break;
            case kOCNumberUInt64Type: n = OCNumberCreateWithUInt64((uint64_t)v); break;
            default: PRINTERROR;
        }
        if (!n) PRINTERROR;

        OCStringRef str      = OCNumberCreateStringValue(n);
        OCStringRef expected = OCStringCreateWithCString(exp_s);

        const char *got_c = OCStringGetCString(str);
        const char *exp_c = OCStringGetCString(expected);
        if (!OCStringEqual(str, expected)) {
            fprintf(stderr,
                    "DEBUG: unsigned %llu → got \"%s\", expected \"%s\"\n",
                    (unsigned long long)v, got_c, exp_c);
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
        double        v;
        OCNumberType  t;
        const char   *s;
    } float_tests[] = {
        {  3.14f, kOCNumberFloat32Type, "3.14"    },
        {  2.71828, kOCNumberFloat64Type, "2.71828" },
        {  0.0f,   kOCNumberFloat32Type, "0"       },
        { -123.456, kOCNumberFloat64Type, "-123.456"}
    };
    for (size_t i = 0; i < sizeof float_tests/sizeof *float_tests; ++i) {
        double v = float_tests[i].v;
        OCNumberType t = float_tests[i].t;
        const char *exp_s = float_tests[i].s;

        OCNumberRef n = NULL;
        if (t == kOCNumberFloat32Type)
            n = OCNumberCreateWithFloat((float)v);
        else
            n = OCNumberCreateWithDouble(v);
        if (!n) PRINTERROR;

        // Size
        int got_sz = OCNumberTypeSize(t);
        int exp_sz = (t==kOCNumberFloat32Type ? sizeof(float) : sizeof(double));
        if (got_sz != exp_sz) {
            fprintf(stderr,"DEBUG: float type %d size %d, expect %d\n",
                    (int)t, got_sz, exp_sz);
            OCRelease(n);
            PRINTERROR;
        }

        OCStringRef str      = OCNumberCreateStringValue(n);
        OCStringRef expected = OCStringCreateWithCString(exp_s);

        const char *got_c = OCStringGetCString(str);
        const char *exp_c = OCStringGetCString(expected);
        // compare prefix
        if (strncmp(got_c, exp_c, strlen(exp_c)) != 0) {
            fprintf(stderr,
                    "DEBUG: float %g → got \"%s\", expected prefix \"%s\"\n",
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
        OCNumberType   t;
        const char    *s;
    } complex_tests[] = {
        { 1.0f + 2.5f*I,    kOCNumberFloat32ComplexType, "1+I*2.5"     },
        { -3.5  + 4.75*I,   kOCNumberFloat64ComplexType, "-3.5+I*4.75" },
        { 0.0f + 0.0f*I,    kOCNumberFloat32ComplexType, NULL           }, // allow either "0" or "0+I*0"
        { 1.23e4f - .0567f*I, kOCNumberFloat32ComplexType, "12300+I*-0.0567" }
    };
    for (size_t i = 0; i < sizeof complex_tests/sizeof *complex_tests; ++i) {
        double complex v = complex_tests[i].v;
        OCNumberType t   = complex_tests[i].t;
        const char *exp_s= complex_tests[i].s;

        OCNumberRef n = NULL;
        if (t == kOCNumberFloat32ComplexType)
            n = OCNumberCreateWithFloatComplex((float complex)v);
        else
            n = OCNumberCreateWithDoubleComplex((double complex)v);
        if (!n) PRINTERROR;

        OCStringRef str      = OCNumberCreateStringValue(n);
        const char *got_c    = OCStringGetCString(str);

        if (exp_s) {
            // strict compare
            if (strcmp(got_c, exp_s) != 0) {
                fprintf(stderr,
                        "DEBUG: complex %g%+gi → got \"%s\", expected \"%s\"\n",
                        crealf(v), cimagf(v), got_c, exp_s);
                OCRelease(str);
                OCRelease(n);
                PRINTERROR;
            }
        } else {
            // allow either "0" or "0+I*0"
            if (!(strcmp(got_c, "0")==0 || strcmp(got_c,"0+I*0")==0)) {
                fprintf(stderr,
                        "DEBUG: complex zero → got \"%s\", expected \"0\" or \"0+I*0\"\n",
                        got_c);
                OCRelease(str);
                OCRelease(n);
                PRINTERROR;
            }
        }

        OCRelease(str);
        OCRelease(n);
    }

    // --- Test OCNumberCreate generic constructor ---
    fprintf(stderr, "Testing OCNumberCreate generic constructor...\n");
    int32_t val_s32 = 123;
    OCNumberRef n_s32 = OCNumberCreate(kOCNumberSInt32Type, &val_s32);
    ASSERT_NOT_NULL(n_s32, "OCNumberCreate with SInt32 failed");
    int32_t check_s32;
    OCNumberGetValue(n_s32, kOCNumberSInt32Type, &check_s32);
    ASSERT_EQUAL(check_s32, val_s32, "OCNumberCreate SInt32 value mismatch");
    OCRelease(n_s32);

    double val_f64 = 123.456;
    OCNumberRef n_f64 = OCNumberCreate(kOCNumberFloat64Type, &val_f64);
    ASSERT_NOT_NULL(n_f64, "OCNumberCreate with Float64 failed");
    double check_f64;
    OCNumberGetValue(n_f64, kOCNumberFloat64Type, &check_f64);
    // Using a tolerance for float comparison
    if (fabs(check_f64 - val_f64) > 1e-9) {
        fprintf(stderr, "DEBUG: OCNumberCreate Float64 value mismatch. Got %f, expected %f\n", check_f64, val_f64);
        PRINTERROR;
    }
    OCRelease(n_f64);

    // --- Test OCEqual for Numbers ---
    fprintf(stderr, "Testing OCEqual for Numbers...\n");
    OCNumberRef num_int_10 = OCNumberCreateWithSInt32(10); // Changed from OCNumberCreateWithInt
    OCNumberRef num_int_10_copy = OCNumberCreateWithSInt32(10); // Changed from OCNumberCreateWithInt
    OCNumberRef num_int_20 = OCNumberCreateWithSInt32(20); // Changed from OCNumberCreateWithInt
    OCNumberRef num_float_10 = OCNumberCreateWithFloat(10.0f);
    OCNumberRef num_float_20 = OCNumberCreateWithFloat(20.0f);

    ASSERT_TRUE(OCEqual(num_int_10, num_int_10_copy), "OCEqual: int(10) == int(10) failed");
    ASSERT_FALSE(OCEqual(num_int_10, num_int_20), "OCEqual: int(10) == int(20) should be false");
    
    // Test equality between integer and float representations of the same value
    // This depends on the implementation of OCEqual for OCNumber. 
    // Assuming OCEqual for numbers might convert or compare them appropriately.
    // If OCEqual strictly checks type first, this might fail and is a design consideration.
    ASSERT_TRUE(OCEqual(num_int_10, num_float_10), "OCEqual: int(10) == float(10.0) failed"); 
    ASSERT_FALSE(OCEqual(num_int_10, num_float_20), "OCEqual: int(10) == float(20.0) should be false");

    OCRelease(num_int_10);
    OCRelease(num_int_10_copy);
    OCRelease(num_int_20);
    OCRelease(num_float_10);
    OCRelease(num_float_20);

    // Test OCEqual with different number types but same value
    OCNumberRef num_uint8_max = OCNumberCreateWithUInt8(UINT8_MAX);
    OCNumberRef num_sint16_max_uint8 = OCNumberCreateWithSInt16(UINT8_MAX);
    ASSERT_TRUE(OCEqual(num_uint8_max, num_sint16_max_uint8), "OCEqual: uint8_max == sint16_max_uint8 failed");
    OCRelease(num_uint8_max);
    OCRelease(num_sint16_max_uint8);

    // Test OCEqual with different number types and different values
    OCNumberRef num_sint32_neg = OCNumberCreateWithSInt32(-5);
    OCNumberRef num_uint32_pos = OCNumberCreateWithUInt32(5);
    ASSERT_FALSE(OCEqual(num_sint32_neg, num_uint32_pos), "OCEqual: sint32(-5) == uint32(5) should be false");
    OCRelease(num_sint32_neg);
    OCRelease(num_uint32_pos);

    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}


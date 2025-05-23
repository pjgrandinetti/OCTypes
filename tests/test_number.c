#include "test_utils.h"
#include <stdint.h>
#include <complex.h>       // for float complex / double complex
#include <string.h>
#include <inttypes.h> // Ensure this is included at the top
#include "../src/OCNumber.h"
#include "../src/OCType.h"
#include "../src/OCString.h"

#define OCEqual(a,b) OCTypeEqual((const void*)(a),(const void*)(b))

bool numberTest0(void) {

    fprintf(stderr, "%s begin...\n", __func__);

    // everyone should share the same TypeID
    OCTypeID numTypeID = OCNumberGetTypeID();
    if (numTypeID == _kOCNotATypeID) PRINTERROR;

    // --- signed integers ---
    struct { int64_t v; OCNumberType t; const char *s; } signed_tests[] = {
        { INT8_MIN,  kOCNumberSInt8Type,   "-128" },
        { INT16_MIN, kOCNumberSInt16Type,  "-32768" },
        { INT32_MIN, kOCNumberSInt32Type,  "-2147483648" },
        { INT64_MIN, kOCNumberSInt64Type,  "-9223372036854775808" },
    };
    for (size_t i = 0; i < sizeof signed_tests/sizeof *signed_tests; ++i) {
        int64_t v = signed_tests[i].v;
        OCNumberType t = signed_tests[i].t;
        const char *exp_s = signed_tests[i].s;

        OCNumberRef n = NULL;
        switch (t) {
            case kOCNumberSInt8Type:  n = OCNumberCreateWithSInt8((int8_t)v);  break;
            case kOCNumberSInt16Type: n = OCNumberCreateWithSInt16((int16_t)v); break;
            case kOCNumberSInt32Type: n = OCNumberCreateWithSInt32((int32_t)v); break;
            case kOCNumberSInt64Type: n = OCNumberCreateWithSInt64(v);         break;
            default: PRINTERROR;
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
        int exp_sz = (t==kOCNumberSInt8Type  ? sizeof(int8_t)
                      :t==kOCNumberSInt16Type ? sizeof(int16_t)
                      :t==kOCNumberSInt32Type ? sizeof(int32_t)
                                                : sizeof(int64_t));
        if (got_sz != exp_sz) {
            fprintf(stderr,
                    "DEBUG: for signed type %d size() == %d, want %d\n",
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
    struct { uint64_t v; OCNumberType t; const char *s; } unsigned_tests[] = {
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
            case kOCNumberUInt8Type:  n = OCNumberCreateWithUInt8((uint8_t)v);   break;
            case kOCNumberUInt16Type: n = OCNumberCreateWithUInt16((uint16_t)v); break;
            case kOCNumberUInt32Type: n = OCNumberCreateWithUInt32((uint32_t)v); break;
            case kOCNumberUInt64Type: n = OCNumberCreateWithUInt64(v);          break;
            default: PRINTERROR;
        }
        if (!n) PRINTERROR;

        OCStringRef str      = OCNumberCreateStringValue(n);
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
    struct { double v; OCNumberType t; const char *s; } float_tests[] = {
        {  3.14f,   kOCNumberFloat32Type,  "3.14"    },
        {  2.71828, kOCNumberFloat64Type,  "2.71828" },
        {  0.0f,    kOCNumberFloat32Type,  "0"       },
        { -123.456, kOCNumberFloat64Type,  "-123.456"}
    };
    for (size_t i = 0; i < sizeof float_tests/sizeof *float_tests; ++i) {
        double v = float_tests[i].v;
        OCNumberType t = float_tests[i].t;
        const char *exp_s = float_tests[i].s;

        OCNumberRef n = (t == kOCNumberFloat32Type)
            ? OCNumberCreateWithFloat((float)v)
            : OCNumberCreateWithDouble(v);
        if (!n) PRINTERROR;

        // Size check
        int got_sz = OCNumberTypeSize(t);
        int exp_sz = (t==kOCNumberFloat32Type ? sizeof(float) : sizeof(double));
        if (got_sz != exp_sz) {
            fprintf(stderr,
                    "DEBUG: float type %d size %d, expect %d\n",
                    (int)t, got_sz, exp_sz);
            OCRelease(n);
            PRINTERROR;
        }

        // Prefix-match the textual repr
        OCStringRef str      = OCNumberCreateStringValue(n);
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
    struct { double complex v; OCNumberType t; } complex_tests[] = {
        { 1.0f + 2.5f*I,      kOCNumberFloat32ComplexType },
        { -3.5  + 4.75*I,     kOCNumberFloat64ComplexType },
        { 0.0f + 0.0f*I,      kOCNumberFloat32ComplexType },  // zero case
        { 1.23e4f - .0567f*I, kOCNumberFloat32ComplexType }   // precision case
    };
    for (size_t i = 0; i < sizeof complex_tests/sizeof *complex_tests; ++i) {
        double complex v = complex_tests[i].v;
        OCNumberType t   = complex_tests[i].t;

        // create
        OCNumberRef n = (t == kOCNumberFloat32ComplexType)
            ? OCNumberCreateWithFloatComplex((float complex)v)
            : OCNumberCreateWithDoubleComplex(       v );
        if (!n) PRINTERROR;

        OCStringRef str = OCNumberCreateStringValue(n);
        const char *got_c = OCStringGetCString(str);

        // decompose using the correct real/imag functions
        double real, imag;
        if (t == kOCNumberFloat32ComplexType) {
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
        }
        else {
            // build expected with the same format
            char expected_buf[64];
            if (t == kOCNumberFloat32ComplexType) {
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

    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}
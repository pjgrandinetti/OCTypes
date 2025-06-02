//
//  OCNumber.c
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//  Updated to fix format‐string warnings by wrapping literals in STR()
//

#include <stdio.h>
#include <stdlib.h>   // for malloc(), free()
#include <complex.h>   // for creal/cimag on complex types
#include <inttypes.h> // Provides PRIu64 and related macros
#include "OCLibrary.h"

static OCTypeID kOCNumberID = _kOCNotATypeID;

// OCNumber Opaque Type
struct __OCNumber {
    OCBase _base;

    // OCNumber Type attributes  - order of declaration is essential
    OCNumberType    type;
    __Number        value;
};

static bool __OCNumberEqual(const void * theType1, const void * theType2)
{
    OCNumberRef a = (OCNumberRef)theType1;
    OCNumberRef b = (OCNumberRef)theType2;
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->_base.typeID != b->_base.typeID) return false;

    // Convert both numbers to double or double complex for comparison
    double val_a_real, val_b_real;
    double val_a_imag = 0.0, val_b_imag = 0.0;
    bool a_is_complex = false, b_is_complex = false;

    switch (a->type) {
        case kOCNumberUInt8Type:    val_a_real = (double)a->value.uint8Value; break;
        case kOCNumberSInt8Type:    val_a_real = (double)a->value.int8Value; break;
        case kOCNumberUInt16Type:   val_a_real = (double)a->value.uint16Value; break;
        case kOCNumberSInt16Type:   val_a_real = (double)a->value.int16Value; break;
        case kOCNumberUInt32Type:   val_a_real = (double)a->value.uint32Value; break;
        case kOCNumberSInt32Type:   val_a_real = (double)a->value.int32Value; break;
        case kOCNumberUInt64Type:   val_a_real = (double)a->value.uint64Value; break;
        case kOCNumberSInt64Type:   val_a_real = (double)a->value.int64Value; break;
        case kOCNumberFloat32Type:  val_a_real = (double)a->value.floatValue; break;
        case kOCNumberFloat64Type:  val_a_real = a->value.doubleValue; break;
        case kOCNumberFloat32ComplexType:
            val_a_real = crealf(a->value.floatComplexValue);
            val_a_imag = cimagf(a->value.floatComplexValue);
            a_is_complex = true;
            break;
        case kOCNumberFloat64ComplexType:
            val_a_real = creal(a->value.doubleComplexValue);
            val_a_imag = cimag(a->value.doubleComplexValue);
            a_is_complex = true;
            break;
        default: return false; // Should not happen
    }

    switch (b->type) {
        case kOCNumberUInt8Type:    val_b_real = (double)b->value.uint8Value; break;
        case kOCNumberSInt8Type:    val_b_real = (double)b->value.int8Value; break;
        case kOCNumberUInt16Type:   val_b_real = (double)b->value.uint16Value; break;
        case kOCNumberSInt16Type:   val_b_real = (double)b->value.int16Value; break;
        case kOCNumberUInt32Type:   val_b_real = (double)b->value.uint32Value; break;
        case kOCNumberSInt32Type:   val_b_real = (double)b->value.int32Value; break;
        case kOCNumberUInt64Type:   val_b_real = (double)b->value.uint64Value; break;
        case kOCNumberSInt64Type:   val_b_real = (double)b->value.int64Value; break;
        case kOCNumberFloat32Type:  val_b_real = (double)b->value.floatValue; break;
        case kOCNumberFloat64Type:  val_b_real = b->value.doubleValue; break;
        case kOCNumberFloat32ComplexType:
            val_b_real = crealf(b->value.floatComplexValue);
            val_b_imag = cimagf(b->value.floatComplexValue);
            b_is_complex = true;
            break;
        case kOCNumberFloat64ComplexType:
            val_b_real = creal(b->value.doubleComplexValue);
            val_b_imag = cimag(b->value.doubleComplexValue);
            b_is_complex = true;
            break;
        default: return false; // Should not happen
    }

    if (a_is_complex || b_is_complex) {
        // If either is complex, compare both as complex numbers
        return val_a_real == val_b_real && val_a_imag == val_b_imag;
    } else {
        // Otherwise, compare as real numbers
        return val_a_real == val_b_real;
    }
}

static void __OCNumberFinalize(const void * theType)
{
    free((void *)theType);
    theType = NULL; // Set to NULL to avoid dangling pointer

}

static OCStringRef __OCNumberCopyFormattingDescription(OCTypeRef theType)
{
    OCNumberRef n = (OCNumberRef)theType;
    switch (n->type) {
        case kOCNumberUInt8Type:
            return OCStringCreateWithFormat(STR("%u"), n->value.uint8Value);
        case kOCNumberSInt8Type:
            return OCStringCreateWithFormat(STR("%d"), n->value.int8Value);
        case kOCNumberUInt16Type:
            return OCStringCreateWithFormat(STR("%u"), n->value.uint16Value);
        case kOCNumberSInt16Type:
            return OCStringCreateWithFormat(STR("%d"), n->value.int16Value);
        case kOCNumberUInt32Type:
            return OCStringCreateWithFormat(STR("%u"), n->value.uint32Value);
        case kOCNumberSInt32Type:
            return OCStringCreateWithFormat(STR("%d"), n->value.int32Value);
        case kOCNumberUInt64Type:
            return OCStringCreateWithFormat(STR("%lu"), n->value.uint64Value);
        case kOCNumberSInt64Type:
            return OCStringCreateWithFormat(STR("%ld"), n->value.int64Value);
        case kOCNumberFloat32Type:
            return OCStringCreateWithFormat(STR("%f"), n->value.floatValue);
        case kOCNumberFloat64Type:
            return OCStringCreateWithFormat(STR("%lf"), n->value.doubleValue);
        case kOCNumberFloat32ComplexType:
            return OCStringCreateWithFormat(STR("%f+I•%f"),
                                            crealf(n->value.floatComplexValue),
                                            cimagf(n->value.floatComplexValue));
        case kOCNumberFloat64ComplexType:
            return OCStringCreateWithFormat(STR("%lf+I•%lf"),
                                            creal(n->value.doubleComplexValue),
                                            cimag(n->value.doubleComplexValue));
    }
    return NULL;
}

OCTypeID OCNumberGetTypeID(void)
{
    if (kOCNumberID == _kOCNotATypeID) {
        kOCNumberID = OCRegisterType("OCNumber");
    }
    return kOCNumberID;
}

static struct __OCNumber *OCNumberAllocate(void)
{
    struct __OCNumber *n = malloc(sizeof(*n));
    if (NULL==n) {
        fprintf(stderr, "OCNumberAllocate: Memory allocation failed.\n");
        return NULL;
    }
    n->_base.typeID             = OCNumberGetTypeID();
    n->_base.static_instance    = false; // Not static
    n->_base.finalize           = __OCNumberFinalize;
    n->_base.equal              = __OCNumberEqual;
    n->_base.copyFormattingDesc = __OCNumberCopyFormattingDescription;
    n->_base.retainCount        = 0;
    n->_base.finalized = false; // Not finalized yet

    n->type                    = kOCNumberFloat64Type; // Default type
    return (struct __OCNumber *) OCRetain(n);
}

OCNumberRef OCNumberCreate(const OCNumberType type, void *value)
{
    struct __OCNumber *n = OCNumberAllocate();
    if (!n) return NULL;
    n->type = type;
    switch (type) {
        case kOCNumberUInt8Type:        n->value.uint8Value      = *(uint8_t*)value; break;
        case kOCNumberSInt8Type:        n->value.int8Value       = *(int8_t*)value;  break;
        case kOCNumberUInt16Type:       n->value.uint16Value     = *(uint16_t*)value;break;
        case kOCNumberSInt16Type:       n->value.int16Value      = *(int16_t*)value; break;
        case kOCNumberUInt32Type:       n->value.uint32Value     = *(uint32_t*)value;break;
        case kOCNumberSInt32Type:       n->value.int32Value      = *(int32_t*)value; break;
        case kOCNumberUInt64Type:       n->value.uint64Value     = *(uint64_t*)value;break;
        case kOCNumberSInt64Type:       n->value.int64Value      = *(int64_t*)value; break;
        case kOCNumberFloat32Type:      n->value.floatValue      = *(float*)value;   break;
        case kOCNumberFloat64Type:      n->value.doubleValue     = *(double*)value;  break;
        case kOCNumberFloat32ComplexType:
            n->value.floatComplexValue  = *(float complex*)value; break;
        case kOCNumberFloat64ComplexType:
            n->value.doubleComplexValue = *(double complex*)value;break;
    }
    return n;
}

OCNumberRef OCNumberCreateWithUInt8(uint8_t v)  { return OCNumberCreate(kOCNumberUInt8Type,  &v); }
OCNumberRef OCNumberCreateWithUInt16(uint16_t v) { return OCNumberCreate(kOCNumberUInt16Type, &v); }
OCNumberRef OCNumberCreateWithUInt32(uint32_t v) { return OCNumberCreate(kOCNumberUInt32Type, &v); }
OCNumberRef OCNumberCreateWithUInt64(uint64_t v) { return OCNumberCreate(kOCNumberUInt64Type, &v); }

OCNumberRef OCNumberCreateWithSInt8(int8_t v)    { return OCNumberCreate(kOCNumberSInt8Type,   &v); }
OCNumberRef OCNumberCreateWithSInt16(int16_t v)  { return OCNumberCreate(kOCNumberSInt16Type,  &v); }
OCNumberRef OCNumberCreateWithSInt32(int32_t v)  { return OCNumberCreate(kOCNumberSInt32Type,  &v); }
OCNumberRef OCNumberCreateWithSInt64(int64_t v)  { return OCNumberCreate(kOCNumberSInt64Type,  &v); }

OCNumberRef OCNumberCreateWithFloat(float v)     { return OCNumberCreate(kOCNumberFloat32Type, &v); }
OCNumberRef OCNumberCreateWithDouble(double v)   { return OCNumberCreate(kOCNumberFloat64Type, &v); }

OCNumberRef OCNumberCreateWithFloatComplex(float complex v)
                                                  { return OCNumberCreate(kOCNumberFloat32ComplexType, &v); }
OCNumberRef OCNumberCreateWithDoubleComplex(double complex v)
                                                  { return OCNumberCreate(kOCNumberFloat64ComplexType, &v); }


OCNumberRef OCNumberCreateWithInt(int value) {
    OCNumberType type;

    switch (sizeof(int)) {
        case 1: type = kOCNumberSInt8Type; break;
        case 2: type = kOCNumberSInt16Type; break;
        case 4: type = kOCNumberSInt32Type; break;
        case 8: type = kOCNumberSInt64Type; break;
        default: return NULL;
    }

    return OCNumberCreate(type, &value);
}

OCNumberRef OCNumberCreateWithLong(long value) {
    OCNumberType type;

    switch (sizeof(long)) {
        case 4: type = kOCNumberSInt32Type; break;
        case 8: type = kOCNumberSInt64Type; break;
        default: return NULL;
    }

    return OCNumberCreate(type, &value);
}

int OCNumberTypeSize(OCNumberType type)
{
    switch (type) {
        case kOCNumberUInt8Type:
        case kOCNumberSInt8Type:            return sizeof(int8_t);
        case kOCNumberUInt16Type:
        case kOCNumberSInt16Type:           return sizeof(int16_t);
        case kOCNumberUInt32Type:
        case kOCNumberSInt32Type:           return sizeof(int32_t);
        case kOCNumberUInt64Type:
        case kOCNumberSInt64Type:           return sizeof(int64_t);
        case kOCNumberFloat32Type:          return sizeof(float);
        case kOCNumberFloat64Type:          return sizeof(double);
        case kOCNumberFloat32ComplexType:   return sizeof(float complex);
        case kOCNumberFloat64ComplexType:   return sizeof(double complex);
    }
    return 0;
}

OCStringRef OCNumberCreateStringValue(OCNumberRef n)
{
    if (!n) return NULL;

    switch (n->type) {
        case kOCNumberUInt8Type:
            return OCStringCreateWithFormat(STR("%hhu"), n->value.uint8Value);
        case kOCNumberSInt8Type:
            return OCStringCreateWithFormat(STR("%hhi"), n->value.int8Value);
        case kOCNumberUInt16Type:
            return OCStringCreateWithFormat(STR("%hu"), n->value.uint16Value);
        case kOCNumberSInt16Type:
            return OCStringCreateWithFormat(STR("%hi"), n->value.int16Value);
        case kOCNumberUInt32Type:
            return OCStringCreateWithFormat(STR("%u"),  n->value.uint32Value);
        case kOCNumberSInt32Type:
            return OCStringCreateWithFormat(STR("%d"),  n->value.int32Value);
        case kOCNumberUInt64Type:
            return OCStringCreateWithFormat(STR("%" PRIu64), (uint64_t)n->value.uint64Value);
        case kOCNumberSInt64Type:
            return OCStringCreateWithFormat(STR("%" PRId64), (int64_t)n->value.int64Value);
        case kOCNumberFloat32Type:
            return OCStringCreateWithFormat(STR("%.9g"),  n->value.floatValue);   // Float precision
        case kOCNumberFloat64Type:
            return OCStringCreateWithFormat(STR("%.17g"), n->value.doubleValue);  // Double precision

        case kOCNumberFloat32ComplexType: {
            float r = crealf(n->value.floatComplexValue);
            float i = cimagf(n->value.floatComplexValue);
            if (r == 0.0f && i == 0.0f)
                return OCStringCreateWithCString("0");
            return OCStringCreateWithFormat(STR("%.9g+I*%.9g"), r, i);
        }

        case kOCNumberFloat64ComplexType: {
            double r = creal(n->value.doubleComplexValue);
            double i = cimag(n->value.doubleComplexValue);
            if (r == 0.0 && i == 0.0)
                return OCStringCreateWithCString("0");
            return OCStringCreateWithFormat(STR("%.17g+I*%.17g"), r, i);
        }
    }

    return NULL;
}

void OCNumberGetValue(OCNumberRef number, OCNumberType type, void *valuePtr) {
    if (!number || !valuePtr) {
        // Consider logging an error or asserting for debug builds
        return;
    }

    // Current implementation requires the requested type to match the internal type.
    // A more advanced version might handle type conversions.
    if (number->type != type) {
        // Consider logging an error: type mismatch.
        // For now, the function will simply not copy the value if types don't match.
        // The caller must be aware that valuePtr might not be updated.
        // A robust solution might involve returning a status code.
        return;
    }

    switch (number->type) {
        case kOCNumberUInt8Type:        *(uint8_t*)valuePtr      = number->value.uint8Value; break;
        case kOCNumberSInt8Type:        *(int8_t*)valuePtr       = number->value.int8Value;  break;
        case kOCNumberUInt16Type:       *(uint16_t*)valuePtr     = number->value.uint16Value;break;
        case kOCNumberSInt16Type:       *(int16_t*)valuePtr      = number->value.int16Value; break;
        case kOCNumberUInt32Type:       *(uint32_t*)valuePtr     = number->value.uint32Value;break;
        case kOCNumberSInt32Type:       *(int32_t*)valuePtr      = number->value.int32Value; break;
        case kOCNumberUInt64Type:       *(uint64_t*)valuePtr     = number->value.uint64Value;break;
        case kOCNumberSInt64Type:       *(int64_t*)valuePtr      = number->value.int64Value; break;
        case kOCNumberFloat32Type:      *(float*)valuePtr        = number->value.floatValue;   break;
        case kOCNumberFloat64Type:      *(double*)valuePtr       = number->value.doubleValue;  break;
        case kOCNumberFloat32ComplexType: *(float complex*)valuePtr = number->value.floatComplexValue; break;
        case kOCNumberFloat64ComplexType: *(double complex*)valuePtr= number->value.doubleComplexValue;break;
        // No default needed as we check number->type == type above,
        // and assume number->type is always a valid OCNumberType.
    }
}

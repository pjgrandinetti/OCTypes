// OCNumber.c – Updated to use OCTypeAlloc and leak-safe finalization
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <inttypes.h>
#include "OCLibrary.h"

static OCTypeID kOCNumberID = _kOCNotATypeID;

struct __OCNumber {
    OCBase _base;
    OCNumberType type;
    __Number value;
};

static bool __OCNumberEqual(const void *a_, const void *b_)
{
    OCNumberRef a = (OCNumberRef)a_;
    OCNumberRef b = (OCNumberRef)b_;
    if (a == b) return true;
    if (!a || !b || a->_base.typeID != b->_base.typeID) return false;

    double ar, ai = 0.0, br, bi = 0.0;
    bool ac = false, bc = false;

    switch (a->type) {
        case kOCNumberUInt8Type: ar = a->value.uint8Value; break;
        case kOCNumberSInt8Type: ar = a->value.int8Value; break;
        case kOCNumberUInt16Type: ar = a->value.uint16Value; break;
        case kOCNumberSInt16Type: ar = a->value.int16Value; break;
        case kOCNumberUInt32Type: ar = a->value.uint32Value; break;
        case kOCNumberSInt32Type: ar = a->value.int32Value; break;
        case kOCNumberUInt64Type: ar = a->value.uint64Value; break;
        case kOCNumberSInt64Type: ar = a->value.int64Value; break;
        case kOCNumberFloat32Type: ar = a->value.floatValue; break;
        case kOCNumberFloat64Type: ar = a->value.doubleValue; break;
        case kOCNumberFloat32ComplexType:
            ar = crealf(a->value.floatComplexValue);
            ai = cimagf(a->value.floatComplexValue);
            ac = true;
            break;
        case kOCNumberFloat64ComplexType:
            ar = creal(a->value.doubleComplexValue);
            ai = cimag(a->value.doubleComplexValue);
            ac = true;
            break;
        default: return false;
    }

    switch (b->type) {
        case kOCNumberUInt8Type: br = b->value.uint8Value; break;
        case kOCNumberSInt8Type: br = b->value.int8Value; break;
        case kOCNumberUInt16Type: br = b->value.uint16Value; break;
        case kOCNumberSInt16Type: br = b->value.int16Value; break;
        case kOCNumberUInt32Type: br = b->value.uint32Value; break;
        case kOCNumberSInt32Type: br = b->value.int32Value; break;
        case kOCNumberUInt64Type: br = b->value.uint64Value; break;
        case kOCNumberSInt64Type: br = b->value.int64Value; break;
        case kOCNumberFloat32Type: br = b->value.floatValue; break;
        case kOCNumberFloat64Type: br = b->value.doubleValue; break;
        case kOCNumberFloat32ComplexType:
            br = crealf(b->value.floatComplexValue);
            bi = cimagf(b->value.floatComplexValue);
            bc = true;
            break;
        case kOCNumberFloat64ComplexType:
            br = creal(b->value.doubleComplexValue);
            bi = cimag(b->value.doubleComplexValue);
            bc = true;
            break;
        default: return false;
    }

    if (ac || bc) return (ar == br && ai == bi);
    return ar == br;
}

static void __OCNumberFinalize(const void *theType)
{
    // No need to free — handled by OCRelease
    (void)theType;
}

static OCStringRef __OCNumberCopyFormattingDescription(OCTypeRef theType)
{
    OCNumberRef n = (OCNumberRef)theType;
    switch (n->type) {
        case kOCNumberUInt8Type: return OCStringCreateWithFormat(STR("%u"), n->value.uint8Value);
        case kOCNumberSInt8Type: return OCStringCreateWithFormat(STR("%d"), n->value.int8Value);
        case kOCNumberUInt16Type: return OCStringCreateWithFormat(STR("%u"), n->value.uint16Value);
        case kOCNumberSInt16Type: return OCStringCreateWithFormat(STR("%d"), n->value.int16Value);
        case kOCNumberUInt32Type: return OCStringCreateWithFormat(STR("%u"), n->value.uint32Value);
        case kOCNumberSInt32Type: return OCStringCreateWithFormat(STR("%d"), n->value.int32Value);
        case kOCNumberUInt64Type: return OCStringCreateWithFormat(STR("%lu"), n->value.uint64Value);
        case kOCNumberSInt64Type: return OCStringCreateWithFormat(STR("%ld"), n->value.int64Value);
        case kOCNumberFloat32Type: return OCStringCreateWithFormat(STR("%f"), n->value.floatValue);
        case kOCNumberFloat64Type: return OCStringCreateWithFormat(STR("%lf"), n->value.doubleValue);
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
    if (kOCNumberID == _kOCNotATypeID)
        kOCNumberID = OCRegisterType("OCNumber");
    return kOCNumberID;
}

static struct __OCNumber *OCNumberAllocate(void)
{
    return OCTypeAlloc(
        struct __OCNumber,
        OCNumberGetTypeID(),
        __OCNumberFinalize,
        __OCNumberEqual,
        __OCNumberCopyFormattingDescription);
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

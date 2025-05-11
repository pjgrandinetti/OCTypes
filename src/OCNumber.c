//
//  OCNumber.c
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//  Updated by ChatGPT on 2025-05-11.
//
#include "OCLibrary.h"
#include "OCNumber.h"
#include "OCType.h"
#include <complex.h>
#include <stdlib.h>

static OCTypeID kOCNumberID = _kOCNotATypeID;

// OCNumber Opaque Type
struct __OCNumber {
    OCBase _base;
    OCNumberType type;
    __Number      value;
};

// Convert any OCNumber to a double‐complex for uniform comparison
static double complex _OCNumberToComplex(OCNumberRef n) {
    switch (n->type) {
        case kOCNumberSInt8Type:          return (double)n->value.int8Value;
        case kOCNumberSInt16Type:         return (double)n->value.int16Value;
        case kOCNumberSInt32Type:         return (double)n->value.int32Value;
        case kOCNumberSInt64Type:         return (double)n->value.int64Value;
        case kOCNumberUInt8Type:          return (double)n->value.uint8Value;
        case kOCNumberUInt16Type:         return (double)n->value.uint16Value;
        case kOCNumberUInt32Type:         return (double)n->value.uint32Value;
        case kOCNumberUInt64Type:         return (double)n->value.uint64Value;
        case kOCNumberFloat32Type:        return (double)n->value.floatValue;
        case kOCNumberFloat64Type:        return n->value.doubleValue;
        case kOCNumberFloat32ComplexType: return crealf(n->value.floatComplexValue) + cimagf(n->value.floatComplexValue)*I;
        case kOCNumberFloat64ComplexType: return n->value.doubleComplexValue;
    }
    return 0.0 + 0.0*I;
}

static bool __OCNumberEqual(const void *a, const void *b) {
    OCNumberRef n1 = (OCNumberRef)a;
    OCNumberRef n2 = (OCNumberRef)b;
    if (n1 == n2) return true;
    if (n1->_base.typeID != n2->_base.typeID) return false;
    double complex c1 = _OCNumberToComplex(n1);
    double complex c2 = _OCNumberToComplex(n2);
    return (c1 == c2);
}

static void __OCNumberFinalize(const void *theType) {
    free((void *)theType);
}

static OCStringRef __OCNumberCopyFormattingDescription(OCTypeRef theType) {
    // Create a string repr and autorelease it, so the internal copyDescription path
    // doesn’t leak.
    OCNumberRef n = (OCNumberRef)theType;
    OCStringRef s = OCNumberCreateStringValue(n);
    return OCAutorelease(s);
}

OCTypeID OCNumberGetTypeID(void) {
    if (kOCNumberID == _kOCNotATypeID) {
        kOCNumberID = OCRegisterType("OCNumber");
    }
    return kOCNumberID;
}

static struct __OCNumber *_OCNumberAllocate(void) {
    struct __OCNumber *n = malloc(sizeof *n);
    if (!n) return NULL;
    n->_base.typeID             = OCNumberGetTypeID();
    n->_base.retainCount        = 1;
    n->_base.finalize           = __OCNumberFinalize;
    n->_base.equal              = __OCNumberEqual;
    n->_base.copyFormattingDesc = __OCNumberCopyFormattingDescription;
    return n;
}

OCNumberRef OCNumberCreate(const OCNumberType type, void *value) {
    struct __OCNumber *n = _OCNumberAllocate();
    if (!n) return NULL;
    n->type = type;
    switch (type) {
        case kOCNumberUInt8Type:        n->value.uint8Value        = *(uint8_t*)value;       break;
        case kOCNumberSInt8Type:        n->value.int8Value         = *(int8_t*)value;        break;
        case kOCNumberUInt16Type:       n->value.uint16Value       = *(uint16_t*)value;      break;
        case kOCNumberSInt16Type:       n->value.int16Value        = *(int16_t*)value;       break;
        case kOCNumberUInt32Type:       n->value.uint32Value       = *(uint32_t*)value;      break;
        case kOCNumberSInt32Type:       n->value.int32Value        = *(int32_t*)value;       break;
        case kOCNumberUInt64Type:       n->value.uint64Value       = *(uint64_t*)value;      break;
        case kOCNumberSInt64Type:       n->value.int64Value        = *(int64_t*)value;       break;
        case kOCNumberFloat32Type:      n->value.floatValue        = *(float*)value;         break;
        case kOCNumberFloat64Type:      n->value.doubleValue       = *(double*)value;        break;
        case kOCNumberFloat32ComplexType:
            n->value.floatComplexValue = *(float complex*)value;  break;
        case kOCNumberFloat64ComplexType:
            n->value.doubleComplexValue= *(double complex*)value; break;
    }
    return n;
}

OCNumberRef OCNumberCreateWithUInt8(uint8_t v)                { return OCNumberCreate(kOCNumberUInt8Type, &v); }
OCNumberRef OCNumberCreateWithUInt16(uint16_t v)              { return OCNumberCreate(kOCNumberUInt16Type, &v); }
OCNumberRef OCNumberCreateWithUInt32(uint32_t v)              { return OCNumberCreate(kOCNumberUInt32Type, &v); }
OCNumberRef OCNumberCreateWithUInt64(uint64_t v)              { return OCNumberCreate(kOCNumberUInt64Type, &v); }
OCNumberRef OCNumberCreateWithSInt8(int8_t v)                 { return OCNumberCreate(kOCNumberSInt8Type, &v); }
OCNumberRef OCNumberCreateWithSInt16(int16_t v)               { return OCNumberCreate(kOCNumberSInt16Type, &v); }
OCNumberRef OCNumberCreateWithSInt32(int32_t v)               { return OCNumberCreate(kOCNumberSInt32Type, &v); }
OCNumberRef OCNumberCreateWithSInt64(int64_t v)               { return OCNumberCreate(kOCNumberSInt64Type, &v); }
OCNumberRef OCNumberCreateWithFloat(float v)                  { return OCNumberCreate(kOCNumberFloat32Type, &v); }
OCNumberRef OCNumberCreateWithDouble(double v)                { return OCNumberCreate(kOCNumberFloat64Type, &v); }
OCNumberRef OCNumberCreateWithFloatComplex(float complex v)   { return OCNumberCreate(kOCNumberFloat32ComplexType, &v); }
OCNumberRef OCNumberCreateWithDoubleComplex(double complex v) { return OCNumberCreate(kOCNumberFloat64ComplexType, &v); }

int OCNumberTypeSize(OCNumberType type) {
    switch (type) {
        case kOCNumberUInt8Type:
        case kOCNumberSInt8Type:          return sizeof(int8_t);
        case kOCNumberUInt16Type:
        case kOCNumberSInt16Type:         return sizeof(int16_t);
        case kOCNumberUInt32Type:
        case kOCNumberSInt32Type:         return sizeof(int32_t);
        case kOCNumberUInt64Type:
        case kOCNumberSInt64Type:         return sizeof(int64_t);
        case kOCNumberFloat32Type:        return sizeof(float);
        case kOCNumberFloat64Type:        return sizeof(double);
        case kOCNumberFloat32ComplexType: return sizeof(float complex);
        case kOCNumberFloat64ComplexType: return sizeof(double complex);
    }
    return 0;
}

OCStringRef OCNumberCreateStringValue(OCNumberRef n) {
    if (!n) return NULL;
    switch (n->type) {
        case kOCNumberUInt8Type:        return OCStringCreateWithFormat(STR("%hhu"), n->value.uint8Value);
        case kOCNumberSInt8Type:        return OCStringCreateWithFormat(STR("%hhi"), n->value.int8Value);
        case kOCNumberUInt16Type:       return OCStringCreateWithFormat(STR("%hu"),  n->value.uint16Value);
        case kOCNumberSInt16Type:       return OCStringCreateWithFormat(STR("%hi"),  n->value.int16Value);
        case kOCNumberUInt32Type:       return OCStringCreateWithFormat(STR("%u"),   n->value.uint32Value);
        case kOCNumberSInt32Type:       return OCStringCreateWithFormat(STR("%d"),   n->value.int32Value);
        case kOCNumberUInt64Type:       return OCStringCreateWithFormat(STR("%llu"), (unsigned long long)n->value.uint64Value);
        case kOCNumberSInt64Type:       return OCStringCreateWithFormat(STR("%lld"), (long long)n->value.int64Value);
        case kOCNumberFloat32Type:      return OCStringCreateWithFormat(STR("%g"),   n->value.floatValue);
        case kOCNumberFloat64Type:      return OCStringCreateWithFormat(STR("%lg"),  n->value.doubleValue);
        case kOCNumberFloat32ComplexType: {
            float r = crealf(n->value.floatComplexValue), i = cimagf(n->value.floatComplexValue);
            if (r == 0.0f && i == 0.0f) return OCStringCreateWithCString("0");
            return OCStringCreateWithFormat(STR("%g+I*%g"), r, i);
        }
        case kOCNumberFloat64ComplexType: {
            double r = creal(n->value.doubleComplexValue), i = cimag(n->value.doubleComplexValue);
            if (r == 0.0 && i == 0.0) return OCStringCreateWithCString("0");
            return OCStringCreateWithFormat(STR("%g+I*%g"), r, i);
        }
    }
    return NULL;
}

// Extract a stored value back into a C variable:
void OCNumberGetValue(OCNumberRef n, OCNumberType type, void *outValue) {
    if (!n || !outValue) return;
    switch (type) {
        case kOCNumberSInt8Type:          *(int8_t*)outValue    = n->value.int8Value;      break;
        case kOCNumberSInt16Type:         *(int16_t*)outValue   = n->value.int16Value;     break;
        case kOCNumberSInt32Type:         *(int32_t*)outValue   = n->value.int32Value;     break;
        case kOCNumberSInt64Type:         *(int64_t*)outValue   = n->value.int64Value;     break;
        case kOCNumberUInt8Type:          *(uint8_t*)outValue   = n->value.uint8Value;     break;
        case kOCNumberUInt16Type:         *(uint16_t*)outValue  = n->value.uint16Value;    break;
        case kOCNumberUInt32Type:         *(uint32_t*)outValue  = n->value.uint32Value;    break;
        case kOCNumberUInt64Type:         *(uint64_t*)outValue  = n->value.uint64Value;    break;
        case kOCNumberFloat32Type:        *(float*)outValue     = n->value.floatValue;     break;
        case kOCNumberFloat64Type:        *(double*)outValue    = n->value.doubleValue;    break;
        case kOCNumberFloat32ComplexType: *(float complex*)outValue  = n->value.floatComplexValue;  break;
        case kOCNumberFloat64ComplexType: *(double complex*)outValue = n->value.doubleComplexValue; break;
        default: break;
    }
}

// OCNumber.c – Updated to use OCTypeAlloc and leak-safe finalization
#include <complex.h>
#include <stddef.h> // for NULL
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OCLibrary.h"
static OCTypeID kOCNumberID = kOCNotATypeID;
struct impl_OCNumber {
    OCBase base;
    OCNumberType type;
    __Number value;
};
const char *OCNumberGetTypeName(OCNumberType type) {
    switch (type) {
        case kOCNumberUInt8Type:
            return "uint8";
        case kOCNumberSInt8Type:
            return "sint8";
        case kOCNumberUInt16Type:
            return "uint16";
        case kOCNumberSInt16Type:
            return "sint16";
        case kOCNumberUInt32Type:
            return "uint32";
        case kOCNumberSInt32Type:
            return "sint32";
        case kOCNumberUInt64Type:
            return "uint64";
        case kOCNumberSInt64Type:
            return "sint64";
        case kOCNumberFloat32Type:
            return "float32";
        case kOCNumberFloat64Type:
            return "float64";
        case kOCNumberComplex64Type:
            return "complex64";
        case kOCNumberComplex128Type:
            return "complex128";
        default:
            return NULL;
    }
}
OCNumberType OCNumberTypeFromName(const char *name) {
    if (!name) return (OCNumberType)(-1);
    if (strcmp(name, "uint8") == 0) return kOCNumberUInt8Type;
    if (strcmp(name, "sint8") == 0) return kOCNumberSInt8Type;
    if (strcmp(name, "uint16") == 0) return kOCNumberUInt16Type;
    if (strcmp(name, "sint16") == 0) return kOCNumberSInt16Type;
    if (strcmp(name, "uint32") == 0) return kOCNumberUInt32Type;
    if (strcmp(name, "sint32") == 0) return kOCNumberSInt32Type;
    if (strcmp(name, "uint64") == 0) return kOCNumberUInt64Type;
    if (strcmp(name, "sint64") == 0) return kOCNumberSInt64Type;
    if (strcmp(name, "float32") == 0) return kOCNumberFloat32Type;
    if (strcmp(name, "float64") == 0) return kOCNumberFloat64Type;
    if (strcmp(name, "complex64") == 0) return kOCNumberComplex64Type;
    if (strcmp(name, "complex128") == 0) return kOCNumberComplex128Type;
    return kOCNumberTypeInvalid;  // Unrecognized name
}
OCTypeID OCNumberGetTypeID(void) {
    if (kOCNumberID == kOCNotATypeID)
        kOCNumberID = OCRegisterType("OCNumber");
    return kOCNumberID;
}
static bool impl_OCNumberEqual(const void *a_, const void *b_) {
    OCNumberRef a = (OCNumberRef)a_;
    OCNumberRef b = (OCNumberRef)b_;
    if (a == b) return true;
    if (!a || !b || a->base.typeID != b->base.typeID) return false;
    double ar, ai = 0.0, br, bi = 0.0;
    bool ac = false, bc = false;
    switch (a->type) {
        case kOCNumberUInt8Type:
            ar = a->value.uint8Value;
            break;
        case kOCNumberSInt8Type:
            ar = a->value.int8Value;
            break;
        case kOCNumberUInt16Type:
            ar = a->value.uint16Value;
            break;
        case kOCNumberSInt16Type:
            ar = a->value.int16Value;
            break;
        case kOCNumberUInt32Type:
            ar = a->value.uint32Value;
            break;
        case kOCNumberSInt32Type:
            ar = a->value.int32Value;
            break;
        case kOCNumberUInt64Type:
            ar = a->value.uint64Value;
            break;
        case kOCNumberSInt64Type:
            ar = a->value.int64Value;
            break;
        case kOCNumberFloat32Type:
            ar = a->value.floatValue;
            break;
        case kOCNumberFloat64Type:
            ar = a->value.doubleValue;
            break;
        case kOCNumberComplex64Type:
            ar = crealf(a->value.floatComplexValue);
            ai = cimagf(a->value.floatComplexValue);
            ac = true;
            break;
        case kOCNumberComplex128Type:
            ar = creal(a->value.doubleComplexValue);
            ai = cimag(a->value.doubleComplexValue);
            ac = true;
            break;
        default:
            return false;
    }
    switch (b->type) {
        case kOCNumberUInt8Type:
            br = b->value.uint8Value;
            break;
        case kOCNumberSInt8Type:
            br = b->value.int8Value;
            break;
        case kOCNumberUInt16Type:
            br = b->value.uint16Value;
            break;
        case kOCNumberSInt16Type:
            br = b->value.int16Value;
            break;
        case kOCNumberUInt32Type:
            br = b->value.uint32Value;
            break;
        case kOCNumberSInt32Type:
            br = b->value.int32Value;
            break;
        case kOCNumberUInt64Type:
            br = b->value.uint64Value;
            break;
        case kOCNumberSInt64Type:
            br = b->value.int64Value;
            break;
        case kOCNumberFloat32Type:
            br = b->value.floatValue;
            break;
        case kOCNumberFloat64Type:
            br = b->value.doubleValue;
            break;
        case kOCNumberComplex64Type:
            br = crealf(b->value.floatComplexValue);
            bi = cimagf(b->value.floatComplexValue);
            bc = true;
            break;
        case kOCNumberComplex128Type:
            br = creal(b->value.doubleComplexValue);
            bi = cimag(b->value.doubleComplexValue);
            bc = true;
            break;
        default:
            return false;
    }
    if (ac || bc) return (ar == br && ai == bi);
    return ar == br;
}
static void impl_OCNumberFinalize(const void *theType) {
    (void)theType;
}
#include <inttypes.h>
static OCStringRef impl_OCNumberCopyFormattingDesc(OCTypeRef theType) {
    if (!theType) return NULL;
    OCBase *base = (OCBase *)theType;
    if (base->typeID != OCNumberGetTypeID()) {
        fprintf(stderr, "[OCNumberCopyFormattingDesc] Invalid typeID %u\n", base->typeID);
        return NULL;
    }
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
            return OCStringCreateWithFormat(STR("%" PRIu64), n->value.uint64Value);
        case kOCNumberSInt64Type:
            return OCStringCreateWithFormat(STR("%" PRId64), n->value.int64Value);
        case kOCNumberFloat32Type:
            return OCStringCreateWithFormat(STR("%f"), n->value.floatValue);
        case kOCNumberFloat64Type:
            return OCStringCreateWithFormat(STR("%lf"), n->value.doubleValue);
        case kOCNumberComplex64Type:
            return OCStringCreateWithFormat(STR("%f+I•%f"),
                                            crealf(n->value.floatComplexValue),
                                            cimagf(n->value.floatComplexValue));
        case kOCNumberComplex128Type:
            return OCStringCreateWithFormat(STR("%lf+I•%lf"),
                                            creal(n->value.doubleComplexValue),
                                            cimag(n->value.doubleComplexValue));
    }
    return NULL;
}
/// @internal
/// Serialize an OCNumber (including all integer widths, floats & complexes) into cJSON.
static cJSON *
impl_OCNumberCopyJSON(const void *obj) {
    return OCNumberCreateJSON((OCNumberRef)obj);
}
static void *impl_OCNumberDeepCopy(const void *obj) {
    const OCNumberRef src = (const OCNumberRef)obj;
    if (!src) return NULL;
    return (void *)OCNumberCreate(src->type, (void *)&src->value);
}
static void *impl_OCNumberDeepCopyMutable(const void *obj) {
    // OCNumber is immutable by design, so mutable deep copy is the same.
    return impl_OCNumberDeepCopy(obj);
}
static struct impl_OCNumber *OCNumberAllocate(void) {
    return OCTypeAlloc(
        struct impl_OCNumber,
        OCNumberGetTypeID(),
        impl_OCNumberFinalize,
        impl_OCNumberEqual,
        impl_OCNumberCopyFormattingDesc,
        impl_OCNumberCopyJSON,
        impl_OCNumberDeepCopy,
        impl_OCNumberDeepCopyMutable);
}
OCNumberRef OCNumberCreate(const OCNumberType type, void *value) {
    struct impl_OCNumber *n = OCNumberAllocate();
    if (!n) return NULL;
    n->type = type;
    switch (type) {
        case kOCNumberUInt8Type:
            n->value.uint8Value = *(uint8_t *)value;
            break;
        case kOCNumberSInt8Type:
            n->value.int8Value = *(int8_t *)value;
            break;
        case kOCNumberUInt16Type:
            n->value.uint16Value = *(uint16_t *)value;
            break;
        case kOCNumberSInt16Type:
            n->value.int16Value = *(int16_t *)value;
            break;
        case kOCNumberUInt32Type:
            n->value.uint32Value = *(uint32_t *)value;
            break;
        case kOCNumberSInt32Type:
            n->value.int32Value = *(int32_t *)value;
            break;
        case kOCNumberUInt64Type:
            n->value.uint64Value = *(uint64_t *)value;
            break;
        case kOCNumberSInt64Type:
            n->value.int64Value = *(int64_t *)value;
            break;
        case kOCNumberFloat32Type:
            n->value.floatValue = *(float *)value;
            break;
        case kOCNumberFloat64Type:
            n->value.doubleValue = *(double *)value;
            break;
        case kOCNumberComplex64Type:
            n->value.floatComplexValue = *(float complex *)value;
            break;
        case kOCNumberComplex128Type:
            n->value.doubleComplexValue = *(double complex *)value;
            break;
    }
    return n;
}
OCNumberRef OCNumberCreateWithUInt8(uint8_t v) {
    return OCNumberCreate(kOCNumberUInt8Type, &v);
}
OCNumberRef OCNumberCreateWithUInt16(uint16_t v) {
    return OCNumberCreate(kOCNumberUInt16Type, &v);
}
OCNumberRef OCNumberCreateWithUInt32(uint32_t v) {
    return OCNumberCreate(kOCNumberUInt32Type, &v);
}
OCNumberRef OCNumberCreateWithUInt64(uint64_t v) {
    return OCNumberCreate(kOCNumberUInt64Type, &v);
}
OCNumberRef OCNumberCreateWithSInt8(int8_t v) {
    return OCNumberCreate(kOCNumberSInt8Type, &v);
}
OCNumberRef OCNumberCreateWithSInt16(int16_t v) {
    return OCNumberCreate(kOCNumberSInt16Type, &v);
}
OCNumberRef OCNumberCreateWithSInt32(int32_t v) {
    return OCNumberCreate(kOCNumberSInt32Type, &v);
}
OCNumberRef OCNumberCreateWithSInt64(int64_t v) {
    return OCNumberCreate(kOCNumberSInt64Type, &v);
}
OCNumberRef OCNumberCreateWithFloat(float v) {
    return OCNumberCreate(kOCNumberFloat32Type, &v);
}
OCNumberRef OCNumberCreateWithDouble(double v) {
    return OCNumberCreate(kOCNumberFloat64Type, &v);
}
OCNumberRef OCNumberCreateWithFloatComplex(float complex v) {
    return OCNumberCreate(kOCNumberComplex64Type, &v);
}
OCNumberRef OCNumberCreateWithDoubleComplex(double complex v) {
    return OCNumberCreate(kOCNumberComplex128Type, &v);
}
OCNumberRef OCNumberCreateWithOCIndex(OCIndex index) {
    OCNumberType type;
    switch (sizeof(OCIndex)) {
        case 1:
            type = kOCNumberSInt8Type;
            break;
        case 2:
            type = kOCNumberSInt16Type;
            break;
        case 4:
            type = kOCNumberSInt32Type;
            break;
        case 8:
            type = kOCNumberSInt64Type;
            break;
        default:
            return NULL;
    }
    return OCNumberCreate(type, &index);
}
OCNumberRef OCNumberCreateWithInt(int value) {
    OCNumberType type;
    switch (sizeof(int)) {
        case 1:
            type = kOCNumberSInt8Type;
            break;
        case 2:
            type = kOCNumberSInt16Type;
            break;
        case 4:
            type = kOCNumberSInt32Type;
            break;
        case 8:
            type = kOCNumberSInt64Type;
            break;
        default:
            return NULL;
    }
    return OCNumberCreate(type, &value);
}
OCNumberRef OCNumberCreateWithLong(long value) {
    OCNumberType type;
    switch (sizeof(long)) {
        case 4:
            type = kOCNumberSInt32Type;
            break;
        case 8:
            type = kOCNumberSInt64Type;
            break;
        default:
            return NULL;
    }
    return OCNumberCreate(type, &value);
}
int OCNumberTypeSize(OCNumberType type) {
    switch (type) {
        case kOCNumberUInt8Type:
            return sizeof(uint8_t);
        case kOCNumberSInt8Type:
            return sizeof(int8_t);
        case kOCNumberUInt16Type:
            return sizeof(uint16_t);
        case kOCNumberSInt16Type:
            return sizeof(int16_t);
        case kOCNumberUInt32Type:
            return sizeof(uint32_t);
        case kOCNumberSInt32Type:
            return sizeof(int32_t);
        case kOCNumberUInt64Type:
            return sizeof(uint64_t);
        case kOCNumberSInt64Type:
            return sizeof(int64_t);
        case kOCNumberFloat32Type:
            return sizeof(float);
        case kOCNumberFloat64Type:
            return sizeof(double);
        case kOCNumberComplex64Type:
            return sizeof(float complex);
        case kOCNumberComplex128Type:
            return sizeof(double complex);
    }
    return 0;
}
OCNumberType
OCNumberGetType(OCNumberRef number) {
    if (!number) return (OCNumberType)(-1);
    return number->type;
}
OCStringRef OCNumberCreateStringValue(OCNumberRef n) {
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
            return OCStringCreateWithFormat(STR("%u"), n->value.uint32Value);
        case kOCNumberSInt32Type:
            return OCStringCreateWithFormat(STR("%d"), n->value.int32Value);
        case kOCNumberUInt64Type:
            return OCStringCreateWithFormat(STR("%" PRIu64), (uint64_t)n->value.uint64Value);
        case kOCNumberSInt64Type:
            return OCStringCreateWithFormat(STR("%" PRId64), (int64_t)n->value.int64Value);
        case kOCNumberFloat32Type:
            return OCStringCreateWithFormat(STR("%.9g"), n->value.floatValue);  // Float precision
        case kOCNumberFloat64Type:
            return OCStringCreateWithFormat(STR("%.17g"), n->value.doubleValue);  // Double precision
        case kOCNumberComplex64Type: {
            float r = crealf(n->value.floatComplexValue);
            float i = cimagf(n->value.floatComplexValue);
            if (r == 0.0f && i == 0.0f)
                return OCStringCreateWithCString("0");
            return OCStringCreateWithFormat(STR("%.9g+I*%.9g"), r, i);
        }
        case kOCNumberComplex128Type: {
            double r = creal(n->value.doubleComplexValue);
            double i = cimag(n->value.doubleComplexValue);
            if (r == 0.0 && i == 0.0)
                return OCStringCreateWithCString("0");
            return OCStringCreateWithFormat(STR("%.17g+I*%.17g"), r, i);
        }
    }
    return NULL;
}
OCNumberRef OCNumberCreateWithStringValue(OCNumberType type, const char *stringValue) {
    if (!stringValue) return NULL;
    union __Number value;
    switch (type) {
        case kOCNumberUInt8Type:
            value.uint8Value = (uint8_t)strtoul(stringValue, NULL, 10);
            break;
        case kOCNumberSInt8Type:
            value.int8Value = (int8_t)strtol(stringValue, NULL, 10);
            break;
        case kOCNumberUInt16Type:
            value.uint16Value = (uint16_t)strtoul(stringValue, NULL, 10);
            break;
        case kOCNumberSInt16Type:
            value.int16Value = (int16_t)strtol(stringValue, NULL, 10);
            break;
        case kOCNumberUInt32Type:
            value.uint32Value = (uint32_t)strtoul(stringValue, NULL, 10);
            break;
        case kOCNumberSInt32Type:
            value.int32Value = (int32_t)strtol(stringValue, NULL, 10);
            break;
        case kOCNumberUInt64Type:
            value.uint64Value = (uint64_t)strtoull(stringValue, NULL, 10);
            break;
        case kOCNumberSInt64Type:
            value.int64Value = (int64_t)strtoll(stringValue, NULL, 10);
            break;
        case kOCNumberFloat32Type:
            value.floatValue = strtof(stringValue, NULL);
            break;
        case kOCNumberFloat64Type:
            value.doubleValue = strtod(stringValue, NULL);
            break;
        case kOCNumberComplex64Type: {
            double complex z = OCComplexFromCString(stringValue);
            value.floatComplexValue = (float complex)z;
            break;
        }
        case kOCNumberComplex128Type:
            value.doubleComplexValue = OCComplexFromCString(stringValue);
            break;
        default:
            return NULL;  // Unsupported type
    }
    return OCNumberCreate(type, &value);
}
bool OCNumberGetValue(OCNumberRef number, OCNumberType type, void *valuePtr) {
    if (!number || !valuePtr) {
        // Consider logging an error or asserting for debug builds
        return false;
    }
    // Current implementation requires the requested type to match the internal type.
    // A more advanced version might handle type conversions.
    if (number->type != type) {
        // Consider logging an error: type mismatch.
        // For now, the function will simply not copy the value if types don't match.
        // The caller must be aware that valuePtr might not be updated.
        // A robust solution might involve returning a status code.
        return false;
    }
    switch (number->type) {
        case kOCNumberUInt8Type:
            *(uint8_t *)valuePtr = number->value.uint8Value;
            break;
        case kOCNumberSInt8Type:
            *(int8_t *)valuePtr = number->value.int8Value;
            break;
        case kOCNumberUInt16Type:
            *(uint16_t *)valuePtr = number->value.uint16Value;
            break;
        case kOCNumberSInt16Type:
            *(int16_t *)valuePtr = number->value.int16Value;
            break;
        case kOCNumberUInt32Type:
            *(uint32_t *)valuePtr = number->value.uint32Value;
            break;
        case kOCNumberSInt32Type:
            *(int32_t *)valuePtr = number->value.int32Value;
            break;
        case kOCNumberUInt64Type:
            *(uint64_t *)valuePtr = number->value.uint64Value;
            break;
        case kOCNumberSInt64Type:
            *(int64_t *)valuePtr = number->value.int64Value;
            break;
        case kOCNumberFloat32Type:
            *(float *)valuePtr = number->value.floatValue;
            break;
        case kOCNumberFloat64Type:
            *(double *)valuePtr = number->value.doubleValue;
            break;
        case kOCNumberComplex64Type:
            *(float complex *)valuePtr = number->value.floatComplexValue;
            break;
        case kOCNumberComplex128Type:
            *(double complex *)valuePtr = number->value.doubleComplexValue;
            break;
            // No default needed as we check number->type == type above,
            // and assume number->type is always a valid OCNumberType.
    }
    return true;
}
cJSON *OCNumberCreateJSON(OCNumberRef number) {
    if (!number) return cJSON_CreateNull();

    switch (number->type) {
        case kOCNumberUInt8Type:
            return cJSON_CreateNumber(number->value.uint8Value);
        case kOCNumberSInt8Type:
            return cJSON_CreateNumber(number->value.int8Value);
        case kOCNumberUInt16Type:
            return cJSON_CreateNumber(number->value.uint16Value);
        case kOCNumberSInt16Type:
            return cJSON_CreateNumber(number->value.int16Value);
        case kOCNumberUInt32Type:
            return cJSON_CreateNumber(number->value.uint32Value);
        case kOCNumberSInt32Type:
            return cJSON_CreateNumber(number->value.int32Value);
        case kOCNumberUInt64Type:
            return cJSON_CreateNumber((double)number->value.uint64Value);
        case kOCNumberSInt64Type:
            return cJSON_CreateNumber((double)number->value.int64Value);
        case kOCNumberFloat32Type:
            return cJSON_CreateNumber(number->value.floatValue);
        case kOCNumberFloat64Type:
            return cJSON_CreateNumber(number->value.doubleValue);
        case kOCNumberComplex64Type:
        case kOCNumberComplex128Type: {
            // Complex numbers: serialize as string
            OCStringRef valueStr = OCNumberCreateStringValue(number);
            if (!valueStr) {
                fprintf(stderr, "OCNumberCreateJSON: Failed to convert OCNumber to string.\n");
                return cJSON_CreateNull();
            }
            const char *s = OCStringGetCString(valueStr);
            cJSON *node = cJSON_CreateString(s ? s : "");
            OCRelease(valueStr);
            return node;
        }
        default:
            return cJSON_CreateNull();
    }
}
OCNumberRef OCNumberCreateFromJSON(cJSON *json, OCNumberType type) {
    if (!json || !cJSON_IsString(json)) return NULL;

    const char *valueStr = cJSON_GetStringValue(json);
    if (!valueStr) return NULL;

    return OCNumberCreateWithStringValue(type, valueStr);
}

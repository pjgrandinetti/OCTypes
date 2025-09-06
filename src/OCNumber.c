// OCNumber.c – Updated to use OCTypeAlloc and leak-safe finalization
#include <complex.h>
#include <inttypes.h>
#include <stddef.h>  // for NULL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Performance optimization headers
#ifdef __APPLE__
    #include <Accelerate/Accelerate.h>  // Apple Accelerate framework
    #define HAVE_ACCELERATE 1
#endif

// OpenMP support (widely available)
#ifdef _OPENMP
    #include <omp.h>
    #define HAVE_OPENMP 1
#endif

// SIMD intrinsics for better performance
#if defined(__SSE2__) || defined(_M_X64) || defined(_M_IX86_FP)
    #include <emmintrin.h>  // SSE2
    #define HAVE_SSE2 1
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    #include <arm_neon.h>
    #define HAVE_NEON 1
#endif

#include "OCTypes.h"
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
    if (kOCNumberID == kOCNotATypeID) {
        kOCNumberID = OCRegisterType("OCNumber", (OCTypeRef (*)(cJSON *, OCStringRef *))OCNumberCreateFromJSONTyped);
    }
    return kOCNumberID;
}
static bool impl_OCNumberEqual(const void *a_, const void *b_) {
    OCNumberRef a = (OCNumberRef)a_;
    OCNumberRef b = (OCNumberRef)b_;
    if (a == b) return true;
    if (!a || !b || a->base.typeID != b->base.typeID) return false;

    // Fast path: if types are identical, do exact comparison
    if (a->type == b->type) {
        switch (a->type) {
            case kOCNumberUInt8Type:
                return a->value.uint8Value == b->value.uint8Value;
            case kOCNumberSInt8Type:
                return a->value.int8Value == b->value.int8Value;
            case kOCNumberUInt16Type:
                return a->value.uint16Value == b->value.uint16Value;
            case kOCNumberSInt16Type:
                return a->value.int16Value == b->value.int16Value;
            case kOCNumberUInt32Type:
                return a->value.uint32Value == b->value.uint32Value;
            case kOCNumberSInt32Type:
                return a->value.int32Value == b->value.int32Value;
            case kOCNumberUInt64Type:
                return a->value.uint64Value == b->value.uint64Value;
            case kOCNumberSInt64Type:
                return a->value.int64Value == b->value.int64Value;
            case kOCNumberFloat32Type:
                return a->value.floatValue == b->value.floatValue;
            case kOCNumberFloat64Type:
                return a->value.doubleValue == b->value.doubleValue;
            case kOCNumberComplex64Type:
                return a->value.floatComplexValue == b->value.floatComplexValue;
            case kOCNumberComplex128Type:
                return a->value.doubleComplexValue == b->value.doubleComplexValue;
            default:
                return false;
        }
    }

    // Slow path: different types, convert to double for comparison
    // This allows cross-type equality (e.g., int32(5) == float64(5.0))
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
impl_OCNumberCopyJSON(const void *obj, bool typed, OCStringRef *outError) {
    return OCNumberCopyAsJSON((OCNumberRef)obj, typed, outError);
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

/**
 * @brief Serialize an OCNumber to JSON using unified complex number format.
 *
 * JSON Serialization Format:
 *
 * TYPED MODE:
 * - All types: {"type": "OCNumber", "numeric_type": "<type>", "value": <value>}
 * - Complex:     value = [real, imag]  (array of 2 numbers)
 * - Non-complex: value = <number>      (JSON number)
 * - 64-bit ints: value = "<string>"    (string for precision)
 *
 * UNTYPED MODE:
 * - Complex:     [real, imag]          (array of 2 numbers)
 * - Non-complex: <number>              (JSON number)
 * - 64-bit ints: <number>              (JSON number, may lose precision)
 *
 * Examples:
 * - Complex64 typed:   {"type": "OCNumber", "numeric_type": "complex64", "value": [1.0, 2.0]}
 * - Complex64 untyped: [1.0, 2.0]
 * - Float64 typed:     {"type": "OCNumber", "numeric_type": "float64", "value": 3.14}
 * - Float64 untyped:   3.14
 *
 * @param number The OCNumber to serialize
 * @param typed If true, include type information; if false, serialize value only
 * @return cJSON object representing the serialized number, or NULL on error
 */
cJSON *OCNumberCopyAsJSON(OCNumberRef number, bool typed, OCStringRef *outError) {
    if (outError) *outError = NULL;

    if (!number) {
        if (outError) *outError = STR("OCNumber is NULL");
        return cJSON_CreateNull();
    }

    cJSON *result = NULL;
    char buffer[32];

    // Create the JSON value based on number type and mode
    switch (number->type) {
        case kOCNumberComplex64Type: {
            // Complex numbers: always use [real, imag] array format
            result = cJSON_CreateArray();
            if (!result) {
                if (outError) *outError = STR("Failed to create JSON array for complex64");
                break;
            }

            float real = crealf(number->value.floatComplexValue);
            float imag = cimagf(number->value.floatComplexValue);

            cJSON *realJson = cJSON_CreateNumber(real);
            cJSON *imagJson = cJSON_CreateNumber(imag);

            if (!realJson || !imagJson) {
                if (outError) *outError = STR("Failed to create JSON numbers for complex64 components");
                cJSON_Delete(result);
                cJSON_Delete(realJson);
                cJSON_Delete(imagJson);
                result = NULL;
                break;
            }

            cJSON_AddItemToArray(result, realJson);
            cJSON_AddItemToArray(result, imagJson);
            break;
        }
        case kOCNumberComplex128Type: {
            // Complex numbers: always use [real, imag] array format
            result = cJSON_CreateArray();
            if (!result) {
                if (outError) *outError = STR("Failed to create JSON array for complex128");
                break;
            }

            double real = creal(number->value.doubleComplexValue);
            double imag = cimag(number->value.doubleComplexValue);

            cJSON *realJson = cJSON_CreateNumber(real);
            cJSON *imagJson = cJSON_CreateNumber(imag);

            if (!realJson || !imagJson) {
                if (outError) *outError = STR("Failed to create JSON numbers for complex128 components");
                cJSON_Delete(result);
                cJSON_Delete(realJson);
                cJSON_Delete(imagJson);
                result = NULL;
                break;
            }

            cJSON_AddItemToArray(result, realJson);
            cJSON_AddItemToArray(result, imagJson);
            break;
        }
        case kOCNumberUInt64Type:
            if (typed) {
                // Use string for precision in typed mode
                snprintf(buffer, sizeof(buffer), "%" PRIu64, number->value.uint64Value);
                result = cJSON_CreateString(buffer);
            } else {
                // Use number in untyped mode (may lose precision)
                result = cJSON_CreateNumber((double)number->value.uint64Value);
            }
            break;
        case kOCNumberSInt64Type:
            if (typed) {
                // Use string for precision in typed mode
                snprintf(buffer, sizeof(buffer), "%" PRId64, number->value.int64Value);
                result = cJSON_CreateString(buffer);
            } else {
                // Use number in untyped mode (may lose precision)
                result = cJSON_CreateNumber((double)number->value.int64Value);
            }
            break;
        case kOCNumberUInt8Type:
            result = cJSON_CreateNumber(number->value.uint8Value);
            break;
        case kOCNumberSInt8Type:
            result = cJSON_CreateNumber(number->value.int8Value);
            break;
        case kOCNumberUInt16Type:
            result = cJSON_CreateNumber(number->value.uint16Value);
            break;
        case kOCNumberSInt16Type:
            result = cJSON_CreateNumber(number->value.int16Value);
            break;
        case kOCNumberUInt32Type:
            result = cJSON_CreateNumber(number->value.uint32Value);
            break;
        case kOCNumberSInt32Type:
            result = cJSON_CreateNumber(number->value.int32Value);
            break;
        case kOCNumberFloat32Type:
            result = cJSON_CreateNumber(number->value.floatValue);
            break;
        case kOCNumberFloat64Type:
            result = cJSON_CreateNumber(number->value.doubleValue);
            break;
        default:
            result = cJSON_CreateNull();
            break;
    }

    // Handle error case
    if (!result) {
        if (outError && !*outError) {
            *outError = STR("Failed to convert OCNumber to JSON");
        }
        return cJSON_CreateNull();
    }

    // Wrap in type information if needed
    if (typed) {
        cJSON *entry = cJSON_CreateObject();
        if (!entry) {
            if (outError) *outError = STR("Failed to create JSON object for typed serialization");
            cJSON_Delete(result);
            return cJSON_CreateNull();
        }

        cJSON_AddStringToObject(entry, "type", "OCNumber");

        const char *numericTypeName = OCNumberGetTypeName(number->type);
        cJSON_AddStringToObject(entry, "numeric_type", numericTypeName ? numericTypeName : "unknown");

        cJSON_AddItemToObject(entry, "value", result);
        return entry;
    }

    return result;
}
/**
 * @brief Create OCNumber from untyped JSON value.
 *
 * Handles the unified complex number format:
 * - JSON number → Non-complex OCNumber (type must be specified)
 * - JSON array of 2 numbers → Complex OCNumber (type must specify complex64/128)
 * - JSON string → Any type via string parsing (for backward compatibility)
 *
 * @param json JSON value (number, array of 2, or string)
 * @param type Expected OCNumber type
 * @param outError Error message if operation fails
 * @return OCNumber instance or NULL on error
 */
OCNumberRef OCNumberCreateFromJSON(cJSON *json, OCNumberType type, OCStringRef *outError) {
    if (!json) {
        if (outError) *outError = STR("JSON input is NULL");
        return NULL;
    }

    // Handle complex numbers: expect array of [real, imag]
    if (type == kOCNumberComplex64Type || type == kOCNumberComplex128Type) {
        if (cJSON_IsArray(json)) {
            // New array format: [real, imag]
            if (cJSON_GetArraySize(json) != 2) {
                if (outError) *outError = STR("Complex number array must have exactly 2 elements [real, imag]");
                return NULL;
            }

            cJSON *realJson = cJSON_GetArrayItem(json, 0);
            cJSON *imagJson = cJSON_GetArrayItem(json, 1);

            if (!cJSON_IsNumber(realJson) || !cJSON_IsNumber(imagJson)) {
                if (outError) *outError = STR("Complex number array elements must be numbers");
                return NULL;
            }

            double real = cJSON_GetNumberValue(realJson);
            double imag = cJSON_GetNumberValue(imagJson);

            union __Number value;
            if (type == kOCNumberComplex64Type) {
                value.floatComplexValue = (float)real + (float)imag * I;
            } else {
                value.doubleComplexValue = real + imag * I;
            }

            OCNumberRef result = OCNumberCreate(type, &value);
            if (!result && outError) {
                *outError = STR("Failed to create complex OCNumber from array");
            }
            return result;
        } else if (cJSON_IsString(json)) {
            // Backward compatibility: string format
            const char *valueStr = cJSON_GetStringValue(json);
            if (!valueStr) {
                if (outError) *outError = STR("Failed to get string value from JSON");
                return NULL;
            }

            OCNumberRef result = OCNumberCreateWithStringValue(type, valueStr);
            if (!result && outError) {
                *outError = STR("Failed to create complex OCNumber from string");
            }
            return result;
        } else {
            if (outError) *outError = STR("Complex number JSON must be array [real, imag] or string");
            return NULL;
        }
    }

    // Handle non-complex numbers: expect JSON number or string
    if (cJSON_IsNumber(json)) {
        double numValue = cJSON_GetNumberValue(json);

        union __Number value;
        switch (type) {
            case kOCNumberUInt8Type:
                value.uint8Value = (uint8_t)numValue;
                break;
            case kOCNumberSInt8Type:
                value.int8Value = (int8_t)numValue;
                break;
            case kOCNumberUInt16Type:
                value.uint16Value = (uint16_t)numValue;
                break;
            case kOCNumberSInt16Type:
                value.int16Value = (int16_t)numValue;
                break;
            case kOCNumberUInt32Type:
                value.uint32Value = (uint32_t)numValue;
                break;
            case kOCNumberSInt32Type:
                value.int32Value = (int32_t)numValue;
                break;
            case kOCNumberUInt64Type:
                value.uint64Value = (uint64_t)numValue;
                break;
            case kOCNumberSInt64Type:
                value.int64Value = (int64_t)numValue;
                break;
            case kOCNumberFloat32Type:
                value.floatValue = (float)numValue;
                break;
            case kOCNumberFloat64Type:
                value.doubleValue = numValue;
                break;
            default:
                if (outError) *outError = STR("Unsupported number type for JSON number");
                return NULL;
        }

        OCNumberRef result = OCNumberCreate(type, &value);
        if (!result && outError) {
            *outError = STR("Failed to create OCNumber from JSON number");
        }
        return result;
    } else if (cJSON_IsString(json)) {
        // Backward compatibility and 64-bit precision: string format
        const char *valueStr = cJSON_GetStringValue(json);
        if (!valueStr) {
            if (outError) *outError = STR("Failed to get string value from JSON");
            return NULL;
        }

        OCNumberRef result = OCNumberCreateWithStringValue(type, valueStr);
        if (!result && outError) {
            *outError = STR("Failed to create OCNumber from string value");
        }
        return result;
    } else {
        if (outError) *outError = STR("JSON input must be number, array (for complex), NULL, or string");
        return NULL;
    }
}

/**
 * @brief Create OCNumber from typed JSON object.
 *
 * Expected format: {"type": "OCNumber", "numeric_type": "<type>", "value": <value>}
 * Where <value> depends on the numeric_type:
 * - Complex types: [real, imag] (array of 2 numbers)
 * - 64-bit integers: "<string>" (string for precision)
 * - Other types: <number> (JSON number)
 *
 * Also supports legacy "subtype" field for backward compatibility.
 *
 * @param json JSON object with type information
 * @param outError Error message if operation fails
 * @return OCNumber instance or NULL on error
 */
OCNumberRef OCNumberCreateFromJSONTyped(cJSON *json, OCStringRef *outError) {
    if (!json) {
        if (outError) *outError = STR("JSON input is NULL");
        return NULL;
    }

    if (!cJSON_IsObject(json)) {
        if (outError) *outError = STR("JSON input is not an object");
        return NULL;
    }

    cJSON *type = cJSON_GetObjectItem(json, "type");
    cJSON *numericType = cJSON_GetObjectItem(json, "numeric_type");
    cJSON *value = cJSON_GetObjectItem(json, "value");

    // Support legacy "subtype" field for backward compatibility
    if (!numericType) {
        numericType = cJSON_GetObjectItem(json, "subtype");
    }

    if (!cJSON_IsString(type) || !cJSON_IsString(numericType) || !value) {
        if (outError) *outError = STR("Invalid typed JSON format: missing or invalid type/numeric_type/value fields");
        return NULL;
    }

    const char *typeName = cJSON_GetStringValue(type);
    const char *numericTypeName = cJSON_GetStringValue(numericType);
    if (!typeName || !numericTypeName || strcmp(typeName, "OCNumber") != 0) {
        if (outError) *outError = STR("Invalid type: expected OCNumber");
        return NULL;
    }

    OCNumberType numberType = OCNumberTypeFromName(numericTypeName);
    if (numberType == kOCNumberTypeInvalid) {
        if (outError) *outError = STR("Invalid number numeric_type");
        return NULL;
    }

    // Handle complex numbers (stored as arrays [real, imag])
    if (numberType == kOCNumberComplex64Type || numberType == kOCNumberComplex128Type) {
        if (cJSON_IsArray(value)) {
            // New array format: [real, imag]
            if (cJSON_GetArraySize(value) != 2) {
                if (outError) *outError = STR("Complex number value array must have exactly 2 elements [real, imag]");
                return NULL;
            }

            cJSON *realJson = cJSON_GetArrayItem(value, 0);
            cJSON *imagJson = cJSON_GetArrayItem(value, 1);

            if (!cJSON_IsNumber(realJson) || !cJSON_IsNumber(imagJson)) {
                if (outError) *outError = STR("Complex number array elements must be numbers");
                return NULL;
            }

            double real = cJSON_GetNumberValue(realJson);
            double imag = cJSON_GetNumberValue(imagJson);

            union __Number val;
            if (numberType == kOCNumberComplex64Type) {
                val.floatComplexValue = (float)real + (float)imag * I;
            } else {
                val.doubleComplexValue = real + imag * I;
            }

            OCNumberRef result = OCNumberCreate(numberType, &val);
            if (!result && outError) {
                *outError = STR("Failed to create complex OCNumber");
            }
            return result;
        } else if (cJSON_IsString(value)) {
            // Backward compatibility: string format
            const char *valueStr = cJSON_GetStringValue(value);
            if (!valueStr) {
                if (outError) *outError = STR("Invalid value string: NULL");
                return NULL;
            }
            OCNumberRef result = OCNumberCreateWithStringValue(numberType, valueStr);
            if (!result && outError) {
                *outError = STR("Failed to create complex number from string");
            }
            return result;
        } else {
            if (outError) *outError = STR("Invalid complex number value: expected array [real, imag] or string");
            return NULL;
        }
    }

    // Handle 64-bit integers (stored as strings for precision)
    if (numberType == kOCNumberUInt64Type || numberType == kOCNumberSInt64Type) {
        if (!cJSON_IsString(value)) {
            if (outError) *outError = STR("Invalid 64-bit integer value: expected string");
            return NULL;
        }
        const char *valueStr = cJSON_GetStringValue(value);
        if (!valueStr) {
            if (outError) *outError = STR("Invalid value string: NULL");
            return NULL;
        }

        union __Number val;
        char *endptr;
        if (numberType == kOCNumberUInt64Type) {
            val.uint64Value = strtoull(valueStr, &endptr, 10);
            if (*endptr != '\0') {
                if (outError) *outError = STR("Invalid uint64 number format");
                return NULL;
            }
        } else {
            val.int64Value = strtoll(valueStr, &endptr, 10);
            if (*endptr != '\0') {
                if (outError) *outError = STR("Invalid int64 number format");
                return NULL;
            }
        }
        OCNumberRef result = OCNumberCreate(numberType, &val);
        if (!result && outError) {
            *outError = STR("Failed to create OCNumber");
        }
        return result;
    }

    // Handle regular numeric types
    if (!cJSON_IsNumber(value)) {
        if (outError) *outError = STR("Invalid numeric value: expected number");
        return NULL;
    }
    double numValue = cJSON_GetNumberValue(value);

    union __Number val;
    switch (numberType) {
        case kOCNumberUInt8Type:
            val.uint8Value = (uint8_t)numValue;
            break;
        case kOCNumberSInt8Type:
            val.int8Value = (int8_t)numValue;
            break;
        case kOCNumberUInt16Type:
            val.uint16Value = (uint16_t)numValue;
            break;
        case kOCNumberSInt16Type:
            val.int16Value = (int16_t)numValue;
            break;
        case kOCNumberUInt32Type:
            val.uint32Value = (uint32_t)numValue;
            break;
        case kOCNumberSInt32Type:
            val.int32Value = (int32_t)numValue;
            break;
        case kOCNumberFloat32Type:
            val.floatValue = (float)numValue;
            break;
        case kOCNumberFloat64Type:
            val.doubleValue = numValue;
            break;
        default:
            if (outError) *outError = STR("Unsupported number type");
            return NULL;
    }

    OCNumberRef result = OCNumberCreate(numberType, &val);
    if (!result && outError) {
        *outError = STR("Failed to create OCNumber");
    }
    return result;
}

// ============================================================================
// Try-get Accessor Implementations
// ============================================================================
bool OCNumberTryGetUInt8(OCNumberRef n, uint8_t *out) {
    return OCNumberGetValue(n, kOCNumberUInt8Type, out);
}
bool OCNumberTryGetSInt8(OCNumberRef n, int8_t *out) {
    return OCNumberGetValue(n, kOCNumberSInt8Type, out);
}
bool OCNumberTryGetUInt16(OCNumberRef n, uint16_t *out) {
    return OCNumberGetValue(n, kOCNumberUInt16Type, out);
}
bool OCNumberTryGetSInt16(OCNumberRef n, int16_t *out) {
    return OCNumberGetValue(n, kOCNumberSInt16Type, out);
}
bool OCNumberTryGetUInt32(OCNumberRef n, uint32_t *out) {
    return OCNumberGetValue(n, kOCNumberUInt32Type, out);
}
bool OCNumberTryGetSInt32(OCNumberRef n, int32_t *out) {
    return OCNumberGetValue(n, kOCNumberSInt32Type, out);
}
bool OCNumberTryGetUInt64(OCNumberRef n, uint64_t *out) {
    return OCNumberGetValue(n, kOCNumberUInt64Type, out);
}
bool OCNumberTryGetSInt64(OCNumberRef n, int64_t *out) {
    return OCNumberGetValue(n, kOCNumberSInt64Type, out);
}
bool OCNumberTryGetFloat32(OCNumberRef n, float *out) {
    return OCNumberGetValue(n, kOCNumberFloat32Type, out);
}
bool OCNumberTryGetFloat64(OCNumberRef n, double *out) {
    return OCNumberGetValue(n, kOCNumberFloat64Type, out);
}
bool OCNumberTryGetComplex64(OCNumberRef n, float complex *out) {
    return OCNumberGetValue(n, kOCNumberComplex64Type, out);
}
bool OCNumberTryGetComplex128(OCNumberRef n, double complex *out) {
    return OCNumberGetValue(n, kOCNumberComplex128Type, out);
}
bool OCNumberTryGetFloat(OCNumberRef n, float *out) {
    return OCNumberTryGetFloat32(n, out);
}
bool OCNumberTryGetDouble(OCNumberRef n, double *out) {
    return OCNumberTryGetFloat64(n, out);
}
bool OCNumberTryGetFloatComplex(OCNumberRef n, float complex *out) {
    return OCNumberTryGetComplex64(n, out);
}
bool OCNumberTryGetDoubleComplex(OCNumberRef n, double complex *out) {
    return OCNumberTryGetComplex128(n, out);
}
bool OCNumberTryGetInt(OCNumberRef n, int *out) {
    if (!n || !out) return false;
    OCNumberType type = OCNumberGetType(n);
    switch (type) {
        case kOCNumberSInt8Type: {
            int8_t v;
            if (OCNumberTryGetSInt8(n, &v)) {
                *out = (int)v;
                return true;
            }
            break;
        }
        case kOCNumberSInt16Type: {
            int16_t v;
            if (OCNumberTryGetSInt16(n, &v)) {
                *out = (int)v;
                return true;
            }
            break;
        }
        case kOCNumberSInt32Type: {
            int32_t v;
            if (OCNumberTryGetSInt32(n, &v)) {
                *out = (int)v;
                return true;
            }
            break;
        }
        case kOCNumberSInt64Type: {
            int64_t v;
            if (OCNumberTryGetSInt64(n, &v)) {
                if (v < (int64_t)INT_MIN || v > (int64_t)INT_MAX) return false;  // overflow check
                *out = (int)v;
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}
bool OCNumberTryGetLong(OCNumberRef n, long *out) {
    if (!n || !out) return false;
    OCNumberType type = OCNumberGetType(n);
    switch (type) {
        case kOCNumberSInt8Type: {
            int8_t v;
            if (OCNumberTryGetSInt8(n, &v)) {
                *out = (long)v;
                return true;
            }
            break;
        }
        case kOCNumberSInt16Type: {
            int16_t v;
            if (OCNumberTryGetSInt16(n, &v)) {
                *out = (long)v;
                return true;
            }
            break;
        }
        case kOCNumberSInt32Type: {
            int32_t v;
            if (OCNumberTryGetSInt32(n, &v)) {
                *out = (long)v;
                return true;
            }
            break;
        }
        case kOCNumberSInt64Type: {
            int64_t v;
            if (OCNumberTryGetSInt64(n, &v)) {
                // Optional: validate that v fits into a long if long < int64_t
#if LONG_MAX < INT64_MAX || LONG_MIN > INT64_MIN
                if (v < (int64_t)LONG_MIN || v > (int64_t)LONG_MAX) return false;
#endif
                *out = (long)v;
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}
bool OCNumberTryGetOCIndex(OCNumberRef n, OCIndex *out) {
    if (!n || !out) return false;
    OCNumberType type = OCNumberGetType(n);
    switch (type) {
        case kOCNumberSInt8Type: {
            int8_t v;
            if (OCNumberTryGetSInt8(n, &v)) {
                *out = (OCIndex)v;
                return true;
            }
            break;
        }
        case kOCNumberSInt16Type: {
            int16_t v;
            if (OCNumberTryGetSInt16(n, &v)) {
                *out = (OCIndex)v;
                return true;
            }
            break;
        }
        case kOCNumberSInt32Type: {
            int32_t v;
            if (OCNumberTryGetSInt32(n, &v)) {
                *out = (OCIndex)v;
                return true;
            }
            break;
        }
        case kOCNumberSInt64Type: {
            int64_t v;
            if (OCNumberTryGetSInt64(n, &v)) {
                // Optional: clamp or validate if OCIndex is narrower
                *out = (OCIndex)v;
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

// ============================================================================
// Performance-optimized bulk conversion helpers
// ============================================================================

/**
 * @brief Check if we should use optimized bulk operations
 */
static inline bool shouldUseOptimizedBulk(OCIndex elementCount, OCNumberType type) {
    // Only optimize for larger datasets and supported types
    return elementCount >= 1000 &&
           (type == kOCNumberFloat32Type ||
            type == kOCNumberFloat64Type ||
            type == kOCNumberUInt32Type ||
            type == kOCNumberSInt32Type);
}

/**
 * @brief Memory-optimized array pre-allocation
 */
static inline OCMutableArrayRef createOptimizedArray(const uint8_t *bytes, OCIndex elementCount, OCNumberType type, OCStringRef *outError) {
    OCMutableArrayRef array = OCArrayCreateMutable(elementCount, &kOCTypeArrayCallBacks);
    if (!array) {
        if (outError) *outError = STR("Failed to create optimized array");
        return NULL;
    }

    // Use memory prefetching for better cache performance
    #ifdef HAVE_BUILTIN_PREFETCH
    const OCIndex prefetch_distance = 64;  // Cache line size
    #endif

    switch (type) {
        case kOCNumberFloat32Type: {
            const float *data = (const float *)bytes;

            #ifdef HAVE_ACCELERATE
            // Use Apple Accelerate framework for bulk float processing
            float *temp_buffer = malloc(elementCount * sizeof(float));
            if (temp_buffer) {
                vDSP_mmov(data, temp_buffer, 1, elementCount, 1, 1);
                // Process in chunks for better memory management
                for (OCIndex i = 0; i < elementCount; i++) {
                    OCNumberRef number = OCNumberCreateWithFloat(temp_buffer[i]);
                    if (number) {
                        OCArrayAppendValue(array, number);
                        OCRelease(number);
                    } else {
                        free(temp_buffer);
                        if (outError) *outError = STR("Failed to create OCNumber in optimized path");
                        OCRelease(array);
                        return NULL;
                    }
                }
                free(temp_buffer);
                return array;
            }
            #endif

            #ifdef HAVE_SSE2
            // Use SSE2 for vectorized processing
            const OCIndex simd_count = elementCount & ~3;  // Process in groups of 4
            for (OCIndex i = 0; i < simd_count; i += 4) {
                #ifdef HAVE_BUILTIN_PREFETCH
                if (i + prefetch_distance < elementCount) {
                    __builtin_prefetch(&data[i + prefetch_distance], 0, 1);
                }
                #endif

                // Process 4 floats at once
                for (int j = 0; j < 4; j++) {
                    OCNumberRef number = OCNumberCreateWithFloat(data[i + j]);
                    if (number) {
                        OCArrayAppendValue(array, number);
                        OCRelease(number);
                    } else {
                        if (outError) *outError = STR("Failed to create OCNumber in SIMD path");
                        OCRelease(array);
                        return NULL;
                    }
                }
            }

            // Process remaining elements
            for (OCIndex i = simd_count; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithFloat(data[i]);
                if (number) {
                    OCArrayAppendValue(array, number);
                    OCRelease(number);
                } else {
                    if (outError) *outError = STR("Failed to create OCNumber in SIMD remainder");
                    OCRelease(array);
                    return NULL;
                }
            }
            return array;
            #endif

            // Serial processing with prefetching
            for (OCIndex i = 0; i < elementCount; i++) {
                #ifdef HAVE_BUILTIN_PREFETCH
                if (i + prefetch_distance < elementCount) {
                    __builtin_prefetch(&data[i + prefetch_distance], 0, 1);
                }
                #endif

                OCNumberRef number = OCNumberCreateWithFloat(data[i]);
                if (number) {
                    OCArrayAppendValue(array, number);
                    OCRelease(number);
                } else {
                    if (outError) *outError = STR("Failed to create OCNumber in optimized serial path");
                    OCRelease(array);
                    return NULL;
                }
            }
            return array;
        }

        case kOCNumberFloat64Type: {
            const double *data = (const double *)bytes;

            #ifdef HAVE_ACCELERATE
            // Use Apple Accelerate framework for bulk double processing
            double *temp_buffer = malloc(elementCount * sizeof(double));
            if (temp_buffer) {
                vDSP_mmovD(data, temp_buffer, 1, elementCount, 1, 1);
                for (OCIndex i = 0; i < elementCount; i++) {
                    OCNumberRef number = OCNumberCreateWithDouble(temp_buffer[i]);
                    if (number) {
                        OCArrayAppendValue(array, number);
                        OCRelease(number);
                    } else {
                        free(temp_buffer);
                        if (outError) *outError = STR("Failed to create OCNumber in optimized double path");
                        OCRelease(array);
                        return NULL;
                    }
                }
                free(temp_buffer);
                return array;
            }
            #endif

            // Serial processing with prefetching for doubles
            for (OCIndex i = 0; i < elementCount; i++) {
                #ifdef HAVE_BUILTIN_PREFETCH
                if (i + prefetch_distance < elementCount) {
                    __builtin_prefetch(&data[i + prefetch_distance], 0, 1);
                }
                #endif

                OCNumberRef number = OCNumberCreateWithDouble(data[i]);
                if (number) {
                    OCArrayAppendValue(array, number);
                    OCRelease(number);
                } else {
                    if (outError) *outError = STR("Failed to create OCNumber in optimized double path");
                    OCRelease(array);
                    return NULL;
                }
            }
            return array;
        }

        default:
            // For other types, return NULL to fall back to standard processing
            OCRelease(array);
            return NULL;
    }
}

/**
 * @brief Optimized bulk data extraction from OCNumber array
 * @param array Array of OCNumbers
 * @param elementCount Number of elements
 * @param type Target number type
 * @param buffer Pre-allocated buffer to store data
 * @param outError Error message if operation fails
 * @return true if successful, false if should fall back to standard processing
 */
static inline bool extractOptimizedData(OCArrayRef array, OCIndex elementCount, OCNumberType type, uint8_t *buffer, OCStringRef *outError) {
    // Use memory prefetching for better cache performance
    #ifdef HAVE_BUILTIN_PREFETCH
    const OCIndex prefetch_distance = 16;  // Prefetch ahead
    #endif

    switch (type) {
        case kOCNumberFloat32Type: {
            float *data = (float *)buffer;

            #ifdef HAVE_ACCELERATE
            // Use Apple Accelerate framework for bulk extraction
            float *temp_buffer = malloc(elementCount * sizeof(float));
            if (temp_buffer) {
                // Extract all values first
                for (OCIndex i = 0; i < elementCount; i++) {
                    OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                    if (!number || OCNumberGetType(number) != type) {
                        free(temp_buffer);
                        return false;  // Fall back to standard processing
                    }
                    if (!OCNumberTryGetFloat32(number, &temp_buffer[i])) {
                        free(temp_buffer);
                        return false;
                    }
                }
                // Bulk copy with Accelerate
                vDSP_mmov(temp_buffer, data, 1, elementCount, 1, 1);
                free(temp_buffer);
                return true;
            }
            #endif

            #ifdef HAVE_SSE2
            // Use vectorized processing for extraction
            const OCIndex simd_count = elementCount & ~3;  // Process in groups of 4
            for (OCIndex i = 0; i < simd_count; i += 4) {
                #ifdef HAVE_BUILTIN_PREFETCH
                if (i + prefetch_distance < elementCount) {
                    OCNumberRef prefetch_number = (OCNumberRef)OCArrayGetValueAtIndex(array, i + prefetch_distance);
                    __builtin_prefetch(prefetch_number, 0, 1);
                }
                #endif

                // Process 4 elements at once
                for (int j = 0; j < 4; j++) {
                    OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i + j);
                    if (!number || OCNumberGetType(number) != type) {
                        return false;  // Fall back to standard processing
                    }
                    if (!OCNumberTryGetFloat32(number, &data[i + j])) {
                        return false;
                    }
                }
            }

            // Process remaining elements
            for (OCIndex i = simd_count; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number || OCNumberGetType(number) != type) {
                    return false;
                }
                if (!OCNumberTryGetFloat32(number, &data[i])) {
                    return false;
                }
            }
            return true;
            #endif

            // Serial processing with prefetching
            for (OCIndex i = 0; i < elementCount; i++) {
                #ifdef HAVE_BUILTIN_PREFETCH
                if (i + prefetch_distance < elementCount) {
                    OCNumberRef prefetch_number = (OCNumberRef)OCArrayGetValueAtIndex(array, i + prefetch_distance);
                    __builtin_prefetch(prefetch_number, 0, 1);
                }
                #endif

                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number || OCNumberGetType(number) != type) {
                    return false;
                }
                if (!OCNumberTryGetFloat32(number, &data[i])) {
                    return false;
                }
            }
            return true;
        }

        case kOCNumberFloat64Type: {
            double *data = (double *)buffer;

            #ifdef HAVE_ACCELERATE
            // Use Apple Accelerate framework for bulk double extraction
            double *temp_buffer = malloc(elementCount * sizeof(double));
            if (temp_buffer) {
                for (OCIndex i = 0; i < elementCount; i++) {
                    OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                    if (!number || OCNumberGetType(number) != type) {
                        free(temp_buffer);
                        return false;
                    }
                    if (!OCNumberTryGetFloat64(number, &temp_buffer[i])) {
                        free(temp_buffer);
                        return false;
                    }
                }
                vDSP_mmovD(temp_buffer, data, 1, elementCount, 1, 1);
                free(temp_buffer);
                return true;
            }
            #endif

            // Serial processing with prefetching for doubles
            for (OCIndex i = 0; i < elementCount; i++) {
                #ifdef HAVE_BUILTIN_PREFETCH
                if (i + prefetch_distance < elementCount) {
                    OCNumberRef prefetch_number = (OCNumberRef)OCArrayGetValueAtIndex(array, i + prefetch_distance);
                    __builtin_prefetch(prefetch_number, 0, 1);
                }
                #endif

                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number || OCNumberGetType(number) != type) {
                    return false;
                }
                if (!OCNumberTryGetFloat64(number, &data[i])) {
                    return false;
                }
            }
            return true;
        }

        default:
            // For other types, return false to fall back to standard processing
            return false;
    }
}

OCArrayRef OCNumberCreateArrayFromData(OCDataRef data, OCNumberType type, OCStringRef *outError) {
    if (outError) *outError = NULL;

    if (!data) {
        if (outError) *outError = STR("OCData is NULL");
        return NULL;
    }

    // Get the size of the number type
    int typeSize = OCNumberTypeSize(type);
    if (typeSize == 0) {
        if (outError) *outError = STR("Invalid OCNumberType");
        return NULL;
    }

    // Get data properties
    OCIndex dataLength = OCDataGetLength(data);
    const uint8_t *bytes = OCDataGetBytesPtr(data);

    if (!bytes) {
        if (outError) *outError = STR("Failed to get data bytes");
        return NULL;
    }

    // Check if data length is compatible with the type size
    if (dataLength % typeSize != 0) {
        if (outError) {
            *outError = STR("Data length is not divisible by type size");
        }
        return NULL;
    }

    // Calculate number of elements
    OCIndex elementCount = dataLength / typeSize;

    // Create the array
    OCMutableArrayRef array = OCArrayCreateMutable(elementCount, &kOCTypeArrayCallBacks);
    if (!array) {
        if (outError) *outError = STR("Failed to create array");
        return NULL;
    }

    // Use optimized bulk conversion for large arrays
    if (shouldUseOptimizedBulk(elementCount, type)) {
        OCMutableArrayRef optimizedArray = createOptimizedArray(bytes, elementCount, type, outError);
        if (optimizedArray) {
            OCRelease(array);
            return optimizedArray;
        }
        // Fall back to standard processing if optimization fails
        if (outError && *outError) {
            // Clear optimization error and continue with standard processing
            *outError = NULL;
        }
    }

    // Standard processing with switch outside loop and type-specific bulk processing
    switch (type) {
        case kOCNumberUInt8Type: {
            const uint8_t *data = (const uint8_t *)bytes;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithUInt8(data[i]);
                if (!number) {
                    if (outError) *outError = STR("Failed to create OCNumber from data");
                    OCRelease(array);
                    return NULL;
                }
                OCArrayAppendValue(array, number);
                OCRelease(number);
            }
            break;
        }
        case kOCNumberSInt8Type: {
            const int8_t *data = (const int8_t *)bytes;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithSInt8(data[i]);
                if (!number) {
                    if (outError) *outError = STR("Failed to create OCNumber from data");
                    OCRelease(array);
                    return NULL;
                }
                OCArrayAppendValue(array, number);
                OCRelease(number);
            }
            break;
        }
        case kOCNumberUInt16Type: {
            const uint16_t *data = (const uint16_t *)bytes;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithUInt16(data[i]);
                if (!number) {
                    if (outError) *outError = STR("Failed to create OCNumber from data");
                    OCRelease(array);
                    return NULL;
                }
                OCArrayAppendValue(array, number);
                OCRelease(number);
            }
            break;
        }
        case kOCNumberSInt16Type: {
            const int16_t *data = (const int16_t *)bytes;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithSInt16(data[i]);
                if (!number) {
                    if (outError) *outError = STR("Failed to create OCNumber from data");
                    OCRelease(array);
                    return NULL;
                }
                OCArrayAppendValue(array, number);
                OCRelease(number);
            }
            break;
        }
        case kOCNumberUInt32Type: {
            const uint32_t *data = (const uint32_t *)bytes;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithUInt32(data[i]);
                if (!number) {
                    if (outError) *outError = STR("Failed to create OCNumber from data");
                    OCRelease(array);
                    return NULL;
                }
                OCArrayAppendValue(array, number);
                OCRelease(number);
            }
            break;
        }
        case kOCNumberSInt32Type: {
            const int32_t *data = (const int32_t *)bytes;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithSInt32(data[i]);
                if (!number) {
                    if (outError) *outError = STR("Failed to create OCNumber from data");
                    OCRelease(array);
                    return NULL;
                }
                OCArrayAppendValue(array, number);
                OCRelease(number);
            }
            break;
        }
        case kOCNumberUInt64Type: {
            const uint64_t *data = (const uint64_t *)bytes;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithUInt64(data[i]);
                if (!number) {
                    if (outError) *outError = STR("Failed to create OCNumber from data");
                    OCRelease(array);
                    return NULL;
                }
                OCArrayAppendValue(array, number);
                OCRelease(number);
            }
            break;
        }
        case kOCNumberSInt64Type: {
            const int64_t *data = (const int64_t *)bytes;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithSInt64(data[i]);
                if (!number) {
                    if (outError) *outError = STR("Failed to create OCNumber from data");
                    OCRelease(array);
                    return NULL;
                }
                OCArrayAppendValue(array, number);
                OCRelease(number);
            }
            break;
        }
        case kOCNumberFloat32Type: {
            const float *data = (const float *)bytes;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithFloat(data[i]);
                if (!number) {
                    if (outError) *outError = STR("Failed to create OCNumber from data");
                    OCRelease(array);
                    return NULL;
                }
                OCArrayAppendValue(array, number);
                OCRelease(number);
            }
            break;
        }
        case kOCNumberFloat64Type: {
            const double *data = (const double *)bytes;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithDouble(data[i]);
                if (!number) {
                    if (outError) *outError = STR("Failed to create OCNumber from data");
                    OCRelease(array);
                    return NULL;
                }
                OCArrayAppendValue(array, number);
                OCRelease(number);
            }
            break;
        }
        case kOCNumberComplex64Type: {
            const float complex *data = (const float complex *)bytes;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithFloatComplex(data[i]);
                if (!number) {
                    if (outError) *outError = STR("Failed to create OCNumber from data");
                    OCRelease(array);
                    return NULL;
                }
                OCArrayAppendValue(array, number);
                OCRelease(number);
            }
            break;
        }
        case kOCNumberComplex128Type: {
            const double complex *data = (const double complex *)bytes;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = OCNumberCreateWithDoubleComplex(data[i]);
                if (!number) {
                    if (outError) *outError = STR("Failed to create OCNumber from data");
                    OCRelease(array);
                    return NULL;
                }
                OCArrayAppendValue(array, number);
                OCRelease(number);
            }
            break;
        }
        default:
            if (outError) {
                *outError = STR("Unsupported OCNumberType");
            }
            OCRelease(array);
            return NULL;
    }

    return array;
}

OCDataRef OCNumberCreateDataFromArray(OCArrayRef array, OCNumberType type, OCStringRef *outError) {
    if (outError) {
        *outError = NULL;
    }

    if (!array) {
        if (outError) {
            *outError = STR("OCArray is NULL");
        }
        return NULL;
    }

    // Get the size of the number type
    int typeSize = OCNumberTypeSize(type);
    if (typeSize == 0) {
        if (outError) {
            *outError = STR("Invalid OCNumberType");
        }
        return NULL;
    }

    // Get array count
    OCIndex elementCount = OCArrayGetCount(array);
    if (elementCount == 0) {
        // Return empty data for empty array
        return OCDataCreate(NULL, 0);
    }

    // Calculate total data size
    OCIndex totalSize = elementCount * typeSize;

    // Allocate buffer for the data
    uint8_t *buffer = malloc(totalSize);
    if (!buffer) {
        if (outError) {
            *outError = STR("Failed to allocate memory for data buffer");
        }
        return NULL;
    }

    // Convert each OCNumber to bytes - try optimized bulk processing first
    if (shouldUseOptimizedBulk(elementCount, type)) {
        if (extractOptimizedData(array, elementCount, type, buffer, outError)) {
            // Create OCData from the optimized buffer
            OCDataRef data = OCDataCreate(buffer, totalSize);
            free(buffer);

            if (!data && outError) {
                *outError = STR("Failed to create OCData from optimized buffer");
            }

            return data;
        }
        // Clear any optimization error and continue with standard processing
        if (outError && *outError) {
            *outError = NULL;
        }
    }

    // Standard processing with switch outside loop for optimization
    switch (type) {
        case kOCNumberUInt8Type: {
            uint8_t *data = (uint8_t *)buffer;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number) {
                    if (outError) *outError = STR("NULL OCNumber found in array");
                    free(buffer);
                    return NULL;
                }
                if (OCNumberGetType(number) != type) {
                    if (outError) *outError = STR("OCNumber type mismatch in array");
                    free(buffer);
                    return NULL;
                }
                if (!OCNumberTryGetUInt8(number, &data[i])) {
                    if (outError) *outError = STR("Failed to extract value from OCNumber");
                    free(buffer);
                    return NULL;
                }
            }
            break;
        }
        case kOCNumberSInt8Type: {
            int8_t *data = (int8_t *)buffer;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number) {
                    if (outError) *outError = STR("NULL OCNumber found in array");
                    free(buffer);
                    return NULL;
                }
                if (OCNumberGetType(number) != type) {
                    if (outError) *outError = STR("OCNumber type mismatch in array");
                    free(buffer);
                    return NULL;
                }
                if (!OCNumberTryGetSInt8(number, &data[i])) {
                    if (outError) *outError = STR("Failed to extract value from OCNumber");
                    free(buffer);
                    return NULL;
                }
            }
            break;
        }
        case kOCNumberUInt16Type: {
            uint16_t *data = (uint16_t *)buffer;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number) {
                    if (outError) *outError = STR("NULL OCNumber found in array");
                    free(buffer);
                    return NULL;
                }
                if (OCNumberGetType(number) != type) {
                    if (outError) *outError = STR("OCNumber type mismatch in array");
                    free(buffer);
                    return NULL;
                }
                if (!OCNumberTryGetUInt16(number, &data[i])) {
                    if (outError) *outError = STR("Failed to extract value from OCNumber");
                    free(buffer);
                    return NULL;
                }
            }
            break;
        }
        case kOCNumberSInt16Type: {
            int16_t *data = (int16_t *)buffer;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number) {
                    if (outError) *outError = STR("NULL OCNumber found in array");
                    free(buffer);
                    return NULL;
                }
                if (OCNumberGetType(number) != type) {
                    if (outError) *outError = STR("OCNumber type mismatch in array");
                    free(buffer);
                    return NULL;
                }
                if (!OCNumberTryGetSInt16(number, &data[i])) {
                    if (outError) *outError = STR("Failed to extract value from OCNumber");
                    free(buffer);
                    return NULL;
                }
            }
            break;
        }
        case kOCNumberUInt32Type: {
            uint32_t *data = (uint32_t *)buffer;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number) {
                    if (outError) *outError = STR("NULL OCNumber found in array");
                    free(buffer);
                    return NULL;
                }
                if (OCNumberGetType(number) != type) {
                    if (outError) *outError = STR("OCNumber type mismatch in array");
                    free(buffer);
                    return NULL;
                }
                if (!OCNumberTryGetUInt32(number, &data[i])) {
                    if (outError) *outError = STR("Failed to extract value from OCNumber");
                    free(buffer);
                    return NULL;
                }
            }
            break;
        }
        case kOCNumberSInt32Type: {
            int32_t *data = (int32_t *)buffer;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number) {
                    if (outError) *outError = STR("NULL OCNumber found in array");
                    free(buffer);
                    return NULL;
                }
                if (OCNumberGetType(number) != type) {
                    if (outError) *outError = STR("OCNumber type mismatch in array");
                    free(buffer);
                    return NULL;
                }
                if (!OCNumberTryGetSInt32(number, &data[i])) {
                    if (outError) *outError = STR("Failed to extract value from OCNumber");
                    free(buffer);
                    return NULL;
                }
            }
            break;
        }
        case kOCNumberUInt64Type: {
            uint64_t *data = (uint64_t *)buffer;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number) {
                    if (outError) *outError = STR("NULL OCNumber found in array");
                    free(buffer);
                    return NULL;
                }
                if (OCNumberGetType(number) != type) {
                    if (outError) *outError = STR("OCNumber type mismatch in array");
                    free(buffer);
                    return NULL;
                }
                if (!OCNumberTryGetUInt64(number, &data[i])) {
                    if (outError) *outError = STR("Failed to extract value from OCNumber");
                    free(buffer);
                    return NULL;
                }
            }
            break;
        }
        case kOCNumberSInt64Type: {
            int64_t *data = (int64_t *)buffer;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number) {
                    if (outError) *outError = STR("NULL OCNumber found in array");
                    free(buffer);
                    return NULL;
                }
                if (OCNumberGetType(number) != type) {
                    if (outError) *outError = STR("OCNumber type mismatch in array");
                    free(buffer);
                    return NULL;
                }
                if (!OCNumberTryGetSInt64(number, &data[i])) {
                    if (outError) *outError = STR("Failed to extract value from OCNumber");
                    free(buffer);
                    return NULL;
                }
            }
            break;
        }
        case kOCNumberFloat32Type: {
            float *data = (float *)buffer;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number) {
                    if (outError) *outError = STR("NULL OCNumber found in array");
                    free(buffer);
                    return NULL;
                }
                if (OCNumberGetType(number) != type) {
                    if (outError) *outError = STR("OCNumber type mismatch in array");
                    free(buffer);
                    return NULL;
                }
                if (!OCNumberTryGetFloat32(number, &data[i])) {
                    if (outError) *outError = STR("Failed to extract value from OCNumber");
                    free(buffer);
                    return NULL;
                }
            }
            break;
        }
        case kOCNumberFloat64Type: {
            double *data = (double *)buffer;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number) {
                    if (outError) *outError = STR("NULL OCNumber found in array");
                    free(buffer);
                    return NULL;
                }
                if (OCNumberGetType(number) != type) {
                    if (outError) *outError = STR("OCNumber type mismatch in array");
                    free(buffer);
                    return NULL;
                }
                if (!OCNumberTryGetFloat64(number, &data[i])) {
                    if (outError) *outError = STR("Failed to extract value from OCNumber");
                    free(buffer);
                    return NULL;
                }
            }
            break;
        }
        case kOCNumberComplex64Type: {
            float complex *data = (float complex *)buffer;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number) {
                    if (outError) *outError = STR("NULL OCNumber found in array");
                    free(buffer);
                    return NULL;
                }
                if (OCNumberGetType(number) != type) {
                    if (outError) *outError = STR("OCNumber type mismatch in array");
                    free(buffer);
                    return NULL;
                }
                if (!OCNumberTryGetComplex64(number, &data[i])) {
                    if (outError) *outError = STR("Failed to extract value from OCNumber");
                    free(buffer);
                    return NULL;
                }
            }
            break;
        }
        case kOCNumberComplex128Type: {
            double complex *data = (double complex *)buffer;
            for (OCIndex i = 0; i < elementCount; i++) {
                OCNumberRef number = (OCNumberRef)OCArrayGetValueAtIndex(array, i);
                if (!number) {
                    if (outError) *outError = STR("NULL OCNumber found in array");
                    free(buffer);
                    return NULL;
                }
                if (OCNumberGetType(number) != type) {
                    if (outError) *outError = STR("OCNumber type mismatch in array");
                    free(buffer);
                    return NULL;
                }
                if (!OCNumberTryGetComplex128(number, &data[i])) {
                    if (outError) *outError = STR("Failed to extract value from OCNumber");
                    free(buffer);
                    return NULL;
                }
            }
            break;
        }
        default:
            if (outError) *outError = STR("Unsupported OCNumberType");
            free(buffer);
            return NULL;
    }

    // Create OCData from the buffer
    OCDataRef data = OCDataCreate(buffer, totalSize);
    free(buffer);

    if (!data && outError) {
        *outError = STR("Failed to create OCData from buffer");
    }

    return data;
}

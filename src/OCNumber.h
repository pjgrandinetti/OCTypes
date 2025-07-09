//
//  OCNumber.h
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//
#ifndef OCNumber_h
#define OCNumber_h
#include <complex.h>
#include <limits.h>
#include "OCLibrary.h"
/**
 * @note Ownership follows CoreFoundation conventions:
 *       The caller owns any OCNumberRef returned from functions with "Create"
 *       in the name, and must call OCRelease() when done.
 */
/** @cond INTERNAL */
typedef union __Number {
    int8_t int8Value;                  /**< Signed 8-bit integer value. */
    int16_t int16Value;                /**< Signed 16-bit integer value. */
    int32_t int32Value;                /**< Signed 32-bit integer value. */
    int64_t int64Value;                /**< Signed 64-bit integer value. */
    uint8_t uint8Value;                /**< Unsigned 8-bit integer value. */
    uint16_t uint16Value;              /**< Unsigned 16-bit integer value. */
    uint32_t uint32Value;              /**< Unsigned 32-bit integer value. */
    uint64_t uint64Value;              /**< Unsigned 64-bit integer value. */
    float floatValue;                  /**< 32-bit float value. */
    double doubleValue;                /**< 64-bit float (double) value. */
    float complex floatComplexValue;   /**< Complex float value. */
    double complex doubleComplexValue; /**< Complex double value. */
} __Number;
/** @endcond */
/**
 * @defgroup OCNumber OCNumber
 * @brief Numerical value representation using OCType-compatible objects.
 * @{
 */
/** @cond INTERNAL */
#define SInt8Type 1
#define SInt16Type 2
#define SInt32Type 3
#define SInt64Type 4
#define Float32Type 5
#define Float64Type 6
#define UInt8Type 17
#define UInt16Type 18
#define UInt32Type 19
#define UInt64Type 20
#define Complex64Type 24
#define Complex128Type 26
typedef enum csdmNumericType {
    kCSDMNumberUInt8Type = -1,
    kCSDMNumberSInt8Type = 1,
    kCSDMNumberUInt16Type = -2,
    kCSDMNumberSInt16Type = 2,
    kCSDMNumberUInt32Type = -3,
    kCSDMNumberSInt32Type = 3,
    kCSDMNumberUInt64Type = -4,
    kCSDMNumberSInt64Type = 4,
    kCSDMNumberFloat32Type = 12,
    kCSDMNumberFloat64Type = 13,
    kCSDMNumberComplex64Type = 24,
    kCSDMNumberComplex128Type = 26,
} csdmNumericType;
/** @endcond */
/**
 * @enum OCNumberType
 * @brief Enumerates the specific numeric data types that an OCNumber object can represent.
 * @ingroup OCNumber
 */
typedef enum {
    kOCNumberSInt8Type = SInt8Type,          /**< Signed 8-bit integer. */
    kOCNumberSInt16Type = SInt16Type,        /**< Signed 16-bit integer. */
    kOCNumberSInt32Type = SInt32Type,        /**< Signed 32-bit integer. */
    kOCNumberSInt64Type = SInt64Type,        /**< Signed 64-bit integer. */
    kOCNumberFloat32Type = Float32Type,      /**< 32-bit float. */
    kOCNumberFloat64Type = Float64Type,      /**< 64-bit float (double). */
    kOCNumberUInt8Type = UInt8Type,          /**< Unsigned 8-bit integer. */
    kOCNumberUInt16Type = UInt16Type,        /**< Unsigned 16-bit integer. */
    kOCNumberUInt32Type = UInt32Type,        /**< Unsigned 32-bit integer. */
    kOCNumberUInt64Type = UInt64Type,        /**< Unsigned 64-bit integer. */
    kOCNumberComplex64Type = Complex64Type,  /**< Complex float. */
    kOCNumberComplex128Type = Complex128Type /**< Complex double. */
} OCNumberType;
#if INT_MAX == 127
#define kOCNumberIntType kOCNumberSInt8Type
#elif INT_MAX == 32767
#define kOCNumberIntType kOCNumberSInt16Type
#elif INT_MAX == 2147483647
#define kOCNumberIntType kOCNumberSInt32Type
#elif INT_MAX == 9223372036854775807
#define kOCNumberIntType kOCNumberSInt64Type
#else
#error Unsupported platform: cannot define kOCNumberIntType
#endif
#if LONG_MAX == 2147483647L
#define kOCNumberLongType kOCNumberSInt32Type
#elif LONG_MAX == 9223372036854775807L
#define kOCNumberLongType kOCNumberSInt64Type
#else
#error Unsupported platform: cannot define kOCNumberLongType
#endif
/**
 * @brief Returns the unique OCTypeID for OCNumber.
 * @return The OCTypeID for the OCNumber type.
 * @ingroup OCNumber
 */
OCTypeID OCNumberGetTypeID(void);
/**
 * @brief Create a new OCNumber of given type from raw pointer to value.
 * @param type Numeric type tag.
 * @param value Pointer to matching C value.
 * @return New OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreate(OCNumberType type, void *value);
/**
 * @brief Get the stored type of an OCNumber.
 * @param number An OCNumberRef.
 * @return Stored OCNumberType or -1 if NULL.
 * @ingroup OCNumber
 */
OCNumberType OCNumberGetType(OCNumberRef number);
/**
 * @brief Create string representation of a number.
 * @param number An OCNumberRef.
 * @return OCStringRef (caller must release).
 * @ingroup OCNumber
 */
OCStringRef OCNumberCreateStringValue(OCNumberRef number);
/**
 * @brief Create OCNumber from string and type.
 * @param type Target OCNumberType.
 * @param stringValue Null-terminated C string.
 * @return OCNumberRef or NULL on parse error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithStringValue(OCNumberType type, const char *stringValue);
/**
 * @brief Copy a human-readable description of the number.
 * @param number An OCNumberRef.
 * @return OCStringRef (caller must release).
 * @ingroup OCNumber
 */
OCStringRef OCNumberCopyFormattingDesc(OCNumberRef number);
/**
 * @brief Extract raw value if types match.
 * @param number An OCNumberRef.
 * @param type Expected type.
 * @param valuePtr Pointer to output buffer.
 * @return true on success, false on NULL or type mismatch.
 * @ingroup OCNumber
 */
bool OCNumberGetValue(OCNumberRef number, OCNumberType type, void *valuePtr);
/**
 * @brief Size in bytes of numeric type.
 * @param type OCNumberType.
 * @return Byte size or 0 if invalid.
 * @ingroup OCNumber
 */
int OCNumberTypeSize(OCNumberType type);
/**
 * @brief Serialize to cJSON (value encoded as string).
 * @param number An OCNumberRef.
 * @return cJSON object (caller responsible).
 * @ingroup OCNumber
 */
cJSON *OCNumberCreateJSON(OCNumberRef number);
/**
 * @brief Deserialize from cJSON using specified type.
 * @param json A cJSON string node.
 * @param type Expected OCNumberType.
 * @return New OCNumberRef or NULL.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateFromJSON(cJSON *json, OCNumberType type);
/**
 * @name Convenience Constructors
 * Create OCNumberRefs for native types.
 * @{ */
OCNumberRef OCNumberCreateWithUInt8(uint8_t value);
OCNumberRef OCNumberCreateWithUInt16(uint16_t value);
OCNumberRef OCNumberCreateWithUInt32(uint32_t value);
OCNumberRef OCNumberCreateWithUInt64(uint64_t value);
OCNumberRef OCNumberCreateWithSInt8(int8_t value);
OCNumberRef OCNumberCreateWithSInt16(int16_t value);
OCNumberRef OCNumberCreateWithSInt32(int32_t value);
OCNumberRef OCNumberCreateWithSInt64(int64_t value);
OCNumberRef OCNumberCreateWithInt(int value);
OCNumberRef OCNumberCreateWithLong(long value);
OCNumberRef OCNumberCreateWithOCIndex(OCIndex index);
OCNumberRef OCNumberCreateWithFloat(float value);
OCNumberRef OCNumberCreateWithDouble(double value);
OCNumberRef OCNumberCreateWithFloatComplex(float complex value);
OCNumberRef OCNumberCreateWithDoubleComplex(double complex value);
/** @} */
/** @name Try-get Accessors
 *  Inline helpers to read back C values if type matches.
 * @{ */
/** @brief Try extract uint8_t. */
static inline bool OCNumberTryGetUInt8(OCNumberRef n, uint8_t *out) {
    return OCNumberGetValue(n, kOCNumberUInt8Type, out);
}
/** @brief Try extract int8_t. */
static inline bool OCNumberTryGetSInt8(OCNumberRef n, int8_t *out) {
    return OCNumberGetValue(n, kOCNumberSInt8Type, out);
}
/** @brief Try extract uint16_t. */
static inline bool OCNumberTryGetUInt16(OCNumberRef n, uint16_t *out) {
    return OCNumberGetValue(n, kOCNumberUInt16Type, out);
}
/** @brief Try extract int16_t. */
static inline bool OCNumberTryGetSInt16(OCNumberRef n, int16_t *out) {
    return OCNumberGetValue(n, kOCNumberSInt16Type, out);
}
/** @brief Try extract uint32_t. */
static inline bool OCNumberTryGetUInt32(OCNumberRef n, uint32_t *out) {
    return OCNumberGetValue(n, kOCNumberUInt32Type, out);
}
/** @brief Try extract int32_t. */
static inline bool OCNumberTryGetSInt32(OCNumberRef n, int32_t *out) {
    return OCNumberGetValue(n, kOCNumberSInt32Type, out);
}
/** @brief Try extract uint64_t. */
static inline bool OCNumberTryGetUInt64(OCNumberRef n, uint64_t *out) {
    return OCNumberGetValue(n, kOCNumberUInt64Type, out);
}
/** @brief Try extract int64_t. */
static inline bool OCNumberTryGetSInt64(OCNumberRef n, int64_t *out) {
    return OCNumberGetValue(n, kOCNumberSInt64Type, out);
}
/** @brief Try extract float (32-bit). */
static inline bool OCNumberTryGetFloat32(OCNumberRef n, float *out) {
    return OCNumberGetValue(n, kOCNumberFloat32Type, out);
}
/** @brief Try extract double (64-bit). */
static inline bool OCNumberTryGetFloat64(OCNumberRef n, double *out) {
    return OCNumberGetValue(n, kOCNumberFloat64Type, out);
}
/** @brief Try extract complex float (32-bit). */
static inline bool OCNumberTryGetComplex64(OCNumberRef n, float complex *out) {
    return OCNumberGetValue(n, kOCNumberComplex64Type, out);
}
/** @brief Try extract complex double (64-bit). */
static inline bool OCNumberTryGetComplex128(OCNumberRef n, double complex *out) {
    return OCNumberGetValue(n, kOCNumberComplex128Type, out);
}
/** @brief Alias for TryGetFloat32. */
static inline bool OCNumberTryGetFloat(OCNumberRef n, float *out) {
    return OCNumberTryGetFloat32(n, out);
}
/** @brief Alias for TryGetFloat64. */
static inline bool OCNumberTryGetDouble(OCNumberRef n, double *out) {
    return OCNumberTryGetFloat64(n, out);
}
/** @brief Alias for TryGetComplex64. */
static inline bool OCNumberTryGetFloatComplex(OCNumberRef n, float complex *out) {
    return OCNumberTryGetComplex64(n, out);
}
/** @brief Alias for TryGetComplex128. */
static inline bool OCNumberTryGetDoubleComplex(OCNumberRef n, double complex *out) {
    return OCNumberTryGetComplex128(n, out);
}
/** @brief Try extract int native size. */
static inline bool OCNumberTryGetInt(OCNumberRef n, int *out) {
    if (!n || !out) return false;
    if (sizeof(int) == sizeof(int32_t)) {
        int32_t v;
        if (!OCNumberTryGetSInt32(n, &v)) return false;
        *out = (int)v;
        return true;
    } else if (sizeof(int) == sizeof(int64_t)) {
        int64_t v;
        if (!OCNumberTryGetSInt64(n, &v)) return false;
        *out = (int)v;
        return true;
    }
    return false;
}
/** @brief Try extract long native size. */
static inline bool OCNumberTryGetLong(OCNumberRef n, long *out) {
    if (!n || !out) return false;
    if (sizeof(long) == sizeof(int32_t)) {
        int32_t v;
        if (!OCNumberTryGetSInt32(n, &v)) return false;
        *out = (long)v;
        return true;
    } else if (sizeof(long) == sizeof(int64_t)) {
        int64_t v;
        if (!OCNumberTryGetSInt64(n, &v)) return false;
        *out = (long)v;
        return true;
    }
    return false;
}
/** @brief Try extract OCIndex native size. */
static inline bool OCNumberTryGetOCIndex(OCNumberRef n, OCIndex *out) {
    if (!n || !out) return false;
    if (sizeof(OCIndex) == sizeof(int32_t)) {
        int32_t v;
        if (!OCNumberTryGetSInt32(n, &v)) return false;
        *out = (OCIndex)v;
        return true;
    } else if (sizeof(OCIndex) == sizeof(int64_t)) {
        int64_t v;
        if (!OCNumberTryGetSInt64(n, &v)) return false;
        *out = (OCIndex)v;
        return true;
    }
    return false;
}
/** @} */  // end Try-get Accessors
/** @} */  // end OCNumber
#endif     /* OCNumber_h */

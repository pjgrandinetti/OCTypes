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
#include "OCType.h"
#include "cJSON.h"
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
/**
 * @enum OCNumberType
 * @brief Enumerates the specific numeric data types that an OCNumber object can represent.
 * @ingroup OCNumber
 */
typedef enum {
    kOCNumberSInt8Type = 1, /**< Signed 8-bit integer. */
    kOCNumberSInt16Type,    /**< Signed 16-bit integer. */
    kOCNumberSInt32Type,    /**< Signed 32-bit integer. */
    kOCNumberSInt64Type,    /**< Signed 64-bit integer. */
    kOCNumberFloat32Type,   /**< 32-bit float. */
    kOCNumberFloat64Type,   /**< 64-bit float (double). */
    kOCNumberUInt8Type,     /**< Unsigned 8-bit integer. */
    kOCNumberUInt16Type,    /**< Unsigned 16-bit integer. */
    kOCNumberUInt32Type,    /**< Unsigned 32-bit integer. */
    kOCNumberUInt64Type,    /**< Unsigned 64-bit integer. */
    kOCNumberComplex64Type, /**< Complex float. */
    kOCNumberComplex128Type /**< Complex double. */
} OCNumberType;
#define kOCNumberTypeInvalid 0
/**
 * @brief Returns the canonical string name for a given OCNumberType.
 *
 * @param type The OCNumberType enum value.
 * @return The string name (e.g., "int32", "float64"), or NULL if invalid.
 * @ingroup OCNumber
 */
const char *OCNumberGetTypeName(OCNumberType type);
/**
 * @brief Returns the OCNumberType enum value for a given string name.
 *
 * @param name The string name (e.g., "int32", "float64").
 * @return The corresponding OCNumberType, or -1 if unrecognized.
 * @ingroup OCNumber
 */
OCNumberType OCNumberTypeFromName(const char *name);
/**
 * @brief The OCTypeID for OCNumber.
 * @ingroup OCNumber
 */
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
 * @brief Serialize an OCNumber to JSON.
 *
 * For untyped serialization (typed=false), real numbers are serialized as JSON
 * numbers, and complex numbers as JSON strings. Type information is lost.
 *
 * For typed serialization (typed=true), creates a self-describing JSON object
 * with "type": "OCNumber", "subtype" field indicating the specific OCNumberType,
 * and "value" field containing the numeric data.
 *
 * @param number An OCNumberRef.
 * @param typed Whether to include type information in the serialization.
 * @return cJSON object on success (caller responsible), or cJSON null on failure.
 * @ingroup OCNumber
 */
cJSON *OCNumberCopyAsJSON(OCNumberRef number, bool typed);
/**
 * @brief Deserialize from cJSON using specified type.
 * @param json A cJSON string node.
 * @param type Expected OCNumberType.
 * @return New OCNumberRef or NULL.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateFromJSON(cJSON *json, OCNumberType type);

/**
 * @brief Create an OCNumberRef from a self-describing JSON object.
 *
 * Parses a JSON object that contains type information and a numeric value.
 * The JSON object should have a "type" field set to "OCNumber", a "subtype"
 * field indicating the specific OCNumberType, and a "value" field containing
 * the actual numeric data.
 *
 * @param json A cJSON object with type, subtype, and value fields.
 * @return A newly allocated OCNumberRef, or NULL on failure.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateFromJSONTyped(cJSON *json);
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
 *  Functions to read back C values if type matches.
 * @{ */
/** @brief Try extract uint8_t. */
bool OCNumberTryGetUInt8(OCNumberRef n, uint8_t *out);
/** @brief Try extract int8_t. */
bool OCNumberTryGetSInt8(OCNumberRef n, int8_t *out);
/** @brief Try extract uint16_t. */
bool OCNumberTryGetUInt16(OCNumberRef n, uint16_t *out);
/** @brief Try extract int16_t. */
bool OCNumberTryGetSInt16(OCNumberRef n, int16_t *out);
/** @brief Try extract uint32_t. */
bool OCNumberTryGetUInt32(OCNumberRef n, uint32_t *out);
/** @brief Try extract int32_t. */
bool OCNumberTryGetSInt32(OCNumberRef n, int32_t *out);
/** @brief Try extract uint64_t. */
bool OCNumberTryGetUInt64(OCNumberRef n, uint64_t *out);
/** @brief Try extract int64_t. */
bool OCNumberTryGetSInt64(OCNumberRef n, int64_t *out);
/** @brief Try extract float (32-bit). */
bool OCNumberTryGetFloat32(OCNumberRef n, float *out);
/** @brief Try extract double (64-bit). */
bool OCNumberTryGetFloat64(OCNumberRef n, double *out);
/** @brief Try extract complex float (32-bit). */
bool OCNumberTryGetComplex64(OCNumberRef n, float complex *out);
/** @brief Try extract complex double (64-bit). */
bool OCNumberTryGetComplex128(OCNumberRef n, double complex *out);
/** @brief Alias for TryGetFloat32. */
bool OCNumberTryGetFloat(OCNumberRef n, float *out);
/** @brief Alias for TryGetFloat64. */
bool OCNumberTryGetDouble(OCNumberRef n, double *out);
/** @brief Alias for TryGetComplex64. */
bool OCNumberTryGetFloatComplex(OCNumberRef n, float complex *out);
/** @brief Alias for TryGetComplex128. */
bool OCNumberTryGetDoubleComplex(OCNumberRef n, double complex *out);
/** @brief Try extract int native size. */
bool OCNumberTryGetInt(OCNumberRef n, int *out);
/** @brief Try extract long native size. */
bool OCNumberTryGetLong(OCNumberRef n, long *out);
/** @brief Try extract OCIndex native size. */
bool OCNumberTryGetOCIndex(OCNumberRef n, OCIndex *out);
/** @} */  // end Try-get Accessors
/** @} */  // end OCNumber
#endif     /* OCNumber_h */

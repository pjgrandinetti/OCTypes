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
    int8_t  int8Value;           /**< Signed 8-bit integer value. */
    int16_t int16Value;          /**< Signed 16-bit integer value. */
    int32_t int32Value;          /**< Signed 32-bit integer value. */
    int64_t int64Value;          /**< Signed 64-bit integer value. */
    uint8_t uint8Value;          /**< Unsigned 8-bit integer value. */
    uint16_t uint16Value;        /**< Unsigned 16-bit integer value. */
    uint32_t uint32Value;        /**< Unsigned 32-bit integer value. */
    uint64_t uint64Value;        /**< Unsigned 64-bit integer value. */
    float   floatValue;          /**< 32-bit float value. */
    double  doubleValue;         /**< 64-bit float (double) value. */
    float complex floatComplexValue; /**< Complex float value. */
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
#define Float32ComplexType 24
#define Float64ComplexType 26

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
    kOCNumberSInt8Type = SInt8Type,               /**< Signed 8-bit integer. */
    kOCNumberSInt16Type = SInt16Type,             /**< Signed 16-bit integer. */
    kOCNumberSInt32Type = SInt32Type,             /**< Signed 32-bit integer. */
    kOCNumberSInt64Type = SInt64Type,             /**< Signed 64-bit integer. */
    kOCNumberFloat32Type = Float32Type,           /**< 32-bit float. */
    kOCNumberFloat64Type = Float64Type,           /**< 64-bit float (double). */
    kOCNumberUInt8Type = UInt8Type,               /**< Unsigned 8-bit integer. */
    kOCNumberUInt16Type = UInt16Type,             /**< Unsigned 16-bit integer. */
    kOCNumberUInt32Type = UInt32Type,             /**< Unsigned 32-bit integer. */
    kOCNumberUInt64Type = UInt64Type,             /**< Unsigned 64-bit integer. */
    kOCNumberFloat32ComplexType = Float32ComplexType, /**< Complex float. */
    kOCNumberFloat64ComplexType = Float64ComplexType  /**< Complex double. */
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
 * @brief Creates an OCNumber object from a raw value and specified OCNumberType.
 * @param type The numeric type.
 * @param value Pointer to a variable of the matching C type (e.g., uint8_t*, double*, etc.).
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreate(OCNumberType type, void *value);

/**
 * @brief Get the underlying OCNumberType of an OCNumber.
 * @param number  The OCNumber instance.
 * @return        The type (one of kOCNumber…Type), or undefined if number is NULL.
 * @ingroup      OCNumber
 */
OCNumberType OCNumberGetType(OCNumberRef number);

/**
 * @brief Creates a formatted string representing the OCNumber's value.
 * @param theNumber An OCNumberRef.
 * @return An OCStringRef with the formatted string. Caller must release.
 * @ingroup OCNumber
 */
OCStringRef OCNumberCreateStringValue(OCNumberRef theNumber);

/**
 * @brief Returns a formatting-friendly description of the number.
 * @param theNumber OCNumberRef to describe.
 * @return OCStringRef describing the number (caller must release).
 * @ingroup OCNumber
 */
OCStringRef OCNumberCopyFormattingDesc(OCNumberRef theNumber);

/**
 * @brief Retrieves the raw value from an OCNumber.
 *
 * If the type does not match the OCNumber's actual type, no value is written.
 *
 * @param number The OCNumberRef to query.
 * @param type Expected OCNumberType.
 * @param outValue Destination buffer (must match type).
 * @ingroup OCNumber
 */
bool OCNumberGetValue(OCNumberRef number, OCNumberType type, void *valuePtr);

/**
 * @brief Returns the size in bytes of the C type corresponding to the given OCNumberType.
 * @param type The OCNumberType to query.
 * @return The size in bytes of the specified type, or 0 if invalid.
 * @ingroup OCNumber
 */
int OCNumberTypeSize(OCNumberType type);

/**
 * @name Convenience Constructors
 * @brief Create OCNumberRefs for native numeric types.
 * @ingroup OCNumber
 * @{ 
 */
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

/** @} */ // end of OCNumber group

#endif /* OCNumber_h */


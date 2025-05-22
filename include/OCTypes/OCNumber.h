//
//  OCNumber.h
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//

#ifndef OCNumber_h
#define OCNumber_h

#include <complex.h>      // Needed for complex number types
#include "OCLibrary.h"    // Ensures OCNumberRef and other types are available

/** @cond INTERNAL */
typedef union __Number {
    /** @brief Signed 8-bit integer value. */
    int8_t  int8Value;
    /** @brief Signed 16-bit integer value. */
    int16_t int16Value;
    /** @brief Signed 32-bit integer value. */
    int32_t int32Value;
    /** @brief Signed 64-bit integer value. */
    int64_t int64Value;
    /** @brief Unsigned 8-bit integer value. */
    uint8_t uint8Value;
    /** @brief Unsigned 16-bit integer value. */
    uint16_t uint16Value;
    /** @brief Unsigned 32-bit integer value. */
    uint32_t uint32Value;
    /** @brief Unsigned 64-bit integer value. */
    uint64_t uint64Value;
    /** @brief 32-bit floating-point value. */
    float   floatValue;
    /** @brief 64-bit floating-point value (double). */
    double  doubleValue;
    /** @brief Complex float value (float _Complex). */
    float complex floatComplexValue;
    /** @brief Complex double value (double _Complex). */
    double complex doubleComplexValue;
} __Number;
/** @endcond */

/**
 * @defgroup OCNumber OCNumber
 *  @brief Numerical value representation using OCType-compatible objects.
 *  @{
 */

/** @cond INTERNAL */
/**
 * @name Legacy Numeric Type Codes
 * @brief Internal codes for OCNumberType. Prefer using the OCNumberType enum.
 * @{
 */
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
/** @} */
/** @endcond */

/**
 * @enum OCNumberType
 * @brief Enumerates the specific numeric data types that an OCNumber object can represent.
 * @ingroup OCNumber
 */
typedef enum { // Anonymous enum
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

/**
 * @brief Returns the unique OCTypeID for OCNumber.
 * @return The OCTypeID for the OCNumber type.
 * @ingroup OCNumber
 */
OCTypeID OCNumberGetTypeID(void);

/**
 * @brief Creates an OCNumber object from a raw value and specified OCNumberType.
 * @param type The numeric type.
 * @param value Pointer to the raw value (must match the size of the type).
 * @return An OCNumberRef or NULL on error.
 */
OCNumberRef OCNumberCreate(OCNumberType type, void *value);

/* --- Convenience Constructors --- */

/**
 * @brief Creates an OCNumber from an unsigned 8-bit integer.
 * @param value The uint8_t value to store.
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithUInt8(uint8_t value);

/**
 * @brief Creates an OCNumber from an unsigned 16-bit integer.
 * @param value The uint16_t value to store.
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithUInt16(uint16_t value);

/**
 * @brief Creates an OCNumber from an unsigned 32-bit integer.
 * @param value The uint32_t value to store.
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithUInt32(uint32_t value);

/**
 * @brief Creates an OCNumber from an unsigned 64-bit integer.
 * @param value The uint64_t value to store.
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithUInt64(uint64_t value);

/**
 * @brief Creates an OCNumber from a signed 8-bit integer.
 * @param value The int8_t value to store.
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithSInt8(int8_t value);

/**
 * @brief Creates an OCNumber from a signed 16-bit integer.
 * @param value The int16_t value to store.
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithSInt16(int16_t value);

/**
 * @brief Creates an OCNumber from a signed 32-bit integer.
 * @param value The int32_t value to store.
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithSInt32(int32_t value);

/**
 * @brief Creates an OCNumber from a signed 64-bit integer.
 * @param value The int64_t value to store.
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithSInt64(int64_t value);

/**
 * @brief Creates an OCNumber from a 32-bit floating-point value.
 * @param value The float value to store.
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithFloat(float value);

/**
 * @brief Creates an OCNumber from a 64-bit floating-point value.
 * @param value The double value to store.
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithDouble(double value);

/**
 * @brief Creates an OCNumber from a complex float value.
 * @param value The float complex value to store.
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithFloatComplex(float complex value);

/**
 * @brief Creates an OCNumber from a complex double value.
 * @param value The double complex value to store.
 * @return An OCNumberRef or NULL on error.
 * @ingroup OCNumber
 */
OCNumberRef OCNumberCreateWithDoubleComplex(double complex value);

/**
 * @brief Returns the size in bytes of the C type corresponding to the given OCNumberType.
 * @param type The OCNumberType to query.
 * @return The size in bytes of the specified type, or 0 if invalid.
 * @ingroup OCNumber
 */
int OCNumberTypeSize(OCNumberType type);

/**
 * @brief Creates a formatted string representing the OCNumber's value.
 * @param theNumber An OCNumberRef.
 * @return An OCStringRef with the formatted string. Caller must release.
 */
OCStringRef OCNumberCreateStringValue(OCNumberRef theNumber);

/**
 * @brief Retrieves the raw value from an OCNumber.
 * @param number The OCNumberRef to query.
 * @param type Expected OCNumberType.
 * @param outValue Destination buffer (must match type).
 * @return None. The value is written to outValue.
 * @ingroup OCNumber
 */
void OCNumberGetValue(OCNumberRef number, OCNumberType type, void *outValue);

/** @} */ // end of OCNumber group

#endif /* OCNumber_h */

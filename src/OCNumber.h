//
//  OCNumber.h
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//

#ifndef OCNumber_h
#define OCNumber_h

#include "OCLibrary.h"

/** @defgroup OCNumber OCNumber */
/** @addtogroup OCNumber
 *  @{
 */

/**
 * @brief A reference to an immutable OCNumber object.
 *
 * OCNumberRef is used to represent various numerical values within the OCTypes framework.
 * These objects encapsulate primitive number types and provide a common interface for them.
 */
typedef const struct __OCNumber * OCNumberRef;

/* Legacy or internal numeric type codes. Prefer using OCNumberType. */
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

/**
 * @brief Enumerates the specific numeric data types that an OCNumber object can represent.
 */
typedef enum OCNumberType {
    kOCNumberSInt8Type = SInt8Type,
    kOCNumberSInt16Type = SInt16Type,
    kOCNumberSInt32Type = SInt32Type,
    kOCNumberSInt64Type = SInt64Type,
    kOCNumberFloat32Type = Float32Type,
    kOCNumberFloat64Type = Float64Type,
    kOCNumberUInt8Type = UInt8Type,
    kOCNumberUInt16Type = UInt16Type,
    kOCNumberUInt32Type = UInt32Type,
    kOCNumberUInt64Type = UInt64Type,
    kOCNumberFloat32ComplexType = Float32ComplexType,
    kOCNumberFloat64ComplexType = Float64ComplexType
} OCNumberType;

/**
 * @brief A union to hold the actual numeric value for an OCNumber.
 * @note Used internally.
 */
typedef union __Number {
    int8_t  int8Value;
    int16_t int16Value;
    int32_t int32Value;
    int64_t int64Value;
    uint8_t uint8Value;
    uint16_t uint16Value;
    uint32_t uint32Value;
    uint64_t uint64Value;
    float   floatValue;
    double  doubleValue;
    float complex floatComplexValue;
    double complex doubleComplexValue;
} __Number;

/**
 * @brief Legacy number type enum. Prefer using OCNumberType.
 */
typedef enum numberType {
    kPSNumberFloat32Type = Float32Type,
    kPSNumberFloat64Type = Float64Type,
    kPSNumberFloat32ComplexType = Float32ComplexType,
    kPSNumberFloat64ComplexType = Float64ComplexType
} numberType;

/**
 * @brief Returns the unique OCTypeID for OCNumber.
 */
OCTypeID OCNumberGetTypeID(void);

/**
 * @brief Creates an OCNumber object from a value and type.
 */
OCNumberRef OCNumberCreate(OCNumberType type, void *value);

/* Convenience constructors for unsigned integer types */
OCNumberRef OCNumberCreateWithUInt8(uint8_t value);
OCNumberRef OCNumberCreateWithUInt16(uint16_t value);
OCNumberRef OCNumberCreateWithUInt32(uint32_t value);
OCNumberRef OCNumberCreateWithUInt64(uint64_t value);

/* Convenience constructors for signed integer types */
OCNumberRef OCNumberCreateWithSInt8(int8_t value);
OCNumberRef OCNumberCreateWithSInt16(int16_t value);
OCNumberRef OCNumberCreateWithSInt32(int32_t value);
OCNumberRef OCNumberCreateWithSInt64(int64_t value);

/* Convenience constructors for float and double */
OCNumberRef OCNumberCreateWithFloat(float value);
OCNumberRef OCNumberCreateWithDouble(double value);

/* Convenience constructors for complex float and double */
OCNumberRef OCNumberCreateWithFloatComplex(float complex value);
OCNumberRef OCNumberCreateWithDoubleComplex(double complex value);

/**
 * @brief Returns the size in bytes of an OCNumberType.
 */
int OCNumberTypeSize(OCNumberType type);

/**
 * @brief Returns a string representation of an OCNumber.
 */
OCStringRef OCNumberCreateStringValue(OCNumberRef theNumber);

/**
 * @brief Copies the value from an OCNumber into a provided buffer.
 */
void OCNumberGetValue(OCNumberRef number, OCNumberType type, void *outValue);

/** @} */ // end of OCNumber group

#endif /* OCNumber_h */

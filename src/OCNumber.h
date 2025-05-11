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
 * .. _OCNumber_h:
 *
 * OCNumber.h
 * ==========
 *
 * Defines the OCNumberRef type and related functions for representing various numerical types.
 *
 * OCNumber provides a way to work with different numerical data types (integers, floats, complex numbers)
 * as OCType objects. It allows for creation and inspection of these numerical objects.
 */

/**
 * :type OCNumberRef:
 * :brief: An opaque reference to an immutable OCNumber object.
 *
 * OCNumberRef is used to represent various numerical values within the OCTypes framework.
 * These objects encapsulate primitive number types and provide a common interface for them.
 */
typedef const struct __OCNumber * OCNumberRef;

// These #defines are legacy or internal mapping values for OCNumberType.
// It's generally recommended to use the OCNumberType enum values directly.
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
 * :enum: OCNumberType
 * :brief: Enumerates the specific numerical data types that an OCNumber object can represent.
 *
 * This enumeration is used when creating an OCNumber object to specify its underlying
 * data type and can be queried to determine the type of an existing OCNumber.
 */
typedef enum OCNumberType {
    kOCNumberSInt8Type = SInt8Type,          /**< Signed 8-bit integer. */
    kOCNumberSInt16Type = SInt16Type,        /**< Signed 16-bit integer. */
    kOCNumberSInt32Type = SInt32Type,        /**< Signed 32-bit integer. */
    kOCNumberSInt64Type = SInt64Type,        /**< Signed 64-bit integer. */
    kOCNumberFloat32Type = Float32Type,      /**< 32-bit floating-point number (float). */
    kOCNumberFloat64Type = Float64Type,      /**< 64-bit floating-point number (double). */
    kOCNumberUInt8Type = UInt8Type,          /**< Unsigned 8-bit integer. */
    kOCNumberUInt16Type = UInt16Type,        /**< Unsigned 16-bit integer. */
    kOCNumberUInt32Type = UInt32Type,        /**< Unsigned 32-bit integer. */
    kOCNumberUInt64Type = UInt64Type,        /**< Unsigned 64-bit integer. */
    kOCNumberFloat32ComplexType = Float32ComplexType, /**< 32-bit complex floating-point number (float complex). */
    kOCNumberFloat64ComplexType = Float64ComplexType  /**< 64-bit complex floating-point number (double complex). */
} OCNumberType;

/**
 * :union: __Number
 * :brief: A union to hold the actual numerical value for an OCNumber object.
 * :meta: internal
 *
 * This union is used internally by the OCNumber implementation to store the value
 * corresponding to its OCNumberType.
 * 
 * :ivar int8Value: Value if type is kOCNumberSInt8Type.
 * :ivar int16Value: Value if type is kOCNumberSInt16Type.
 * :ivar int32Value: Value if type is kOCNumberSInt32Type.
 * :ivar int64Value: Value if type is kOCNumberSInt64Type.
 * :ivar uint8Value: Value if type is kOCNumberUInt8Type.
 * :ivar uint16Value: Value if type is kOCNumberUInt16Type.
 * :ivar uint32Value: Value if type is kOCNumberUInt32Type.
 * :ivar uint64Value: Value if type is kOCNumberUInt64Type.
 * :ivar floatValue: Value if type is kOCNumberFloat32Type.
 * :ivar doubleValue: Value if type is kOCNumberFloat64Type.
 * :ivar floatComplexValue: Value if type is kOCNumberFloat32ComplexType.
 * :ivar doubleComplexValue: Value if type is kOCNumberFloat64ComplexType.
 */
typedef union __Number
{
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
 * :enum: numberType
 * :brief: A legacy or alternative enumeration for some number types.
 * :deprecated: This enum appears to be a subset of OCNumberType or a legacy definition.
 *              Prefer using OCNumberType for new code.
 */
typedef enum numberType {
    kPSNumberFloat32Type = Float32Type,
    kPSNumberFloat64Type = Float64Type,
    kPSNumberFloat32ComplexType = Float32ComplexType,
    kPSNumberFloat64ComplexType = Float64ComplexType,
} numberType;

/**
 * :func: OCNumberGetTypeID
 * :brief: Returns the unique type identifier for the OCNumber class.
 * :return: The OCTypeID associated with OCNumber objects.
 */
OCTypeID OCNumberGetTypeID(void);

/**
 * :func: OCNumberCreate
 * :brief: Creates a new OCNumber object with the specified type and value.
 * :param type: The OCNumberType specifying the data type of the number.
 * :param value: A pointer to the actual numerical value. The pointed-to data will be copied.
 *               The size of the data pointed to must match the size of the specified `type`.
 * :return: A new OCNumberRef object, or NULL if creation fails (e.g., invalid type, memory allocation failure).
 *          The caller is responsible for releasing the returned object using OCRelease.
 */
OCNumberRef OCNumberCreate(const OCNumberType type, void *value);

/**
 * .. rubric:: Convenience Constructors for Unsigned Integer Types
 */

/**
 * :func: OCNumberCreateWithUInt8
 * :brief: Creates a new OCNumber object with a uint8_t value.
 * :param value: The uint8_t value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithUInt8(uint8_t value);
/**
 * :func: OCNumberCreateWithUInt16
 * :brief: Creates a new OCNumber object with a uint16_t value.
 * :param value: The uint16_t value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithUInt16(uint16_t value);
/**
 * :func: OCNumberCreateWithUInt32
 * :brief: Creates a new OCNumber object with a uint32_t value.
 * :param value: The uint32_t value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithUInt32(uint32_t value);
/**
 * :func: OCNumberCreateWithUInt64
 * :brief: Creates a new OCNumber object with a uint64_t value.
 * :param value: The uint64_t value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithUInt64(uint64_t value);

/**
 * .. rubric:: Convenience Constructors for Signed Integer Types
 */

/**
 * :func: OCNumberCreateWithSInt8
 * :brief: Creates a new OCNumber object with an int8_t value.
 * :param value: The int8_t value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithSInt8(int8_t value);
/**
 * :func: OCNumberCreateWithSInt16
 * :brief: Creates a new OCNumber object with an int16_t value.
 * :param value: The int16_t value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithSInt16(int16_t value);
/**
 * :func: OCNumberCreateWithSInt32
 * :brief: Creates a new OCNumber object with an int32_t value.
 * :param value: The int32_t value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithSInt32(int32_t value);
/**
 * :func: OCNumberCreateWithSInt64
 * :brief: Creates a new OCNumber object with an int64_t value.
 * :param value: The int64_t value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithSInt64(int64_t value);

/**
 * .. rubric:: Convenience Constructors for Floating-Point Types
 */

/**
 * :func: OCNumberCreateWithFloat
 * :brief: Creates a new OCNumber object with a float value.
 * :param value: The float value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithFloat(float value);
/**
 * :func: OCNumberCreateWithDouble
 * :brief: Creates a new OCNumber object with a double value.
 * :param value: The double value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithDouble(double value);

/**
 * .. rubric:: Convenience Constructors for Complex Floating-Point Types
 */

/**
 * :func: OCNumberCreateWithFloatComplex
 * :brief: Creates a new OCNumber object with a float complex value.
 * :param value: The float complex value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithFloatComplex(float complex value);
/**
 * :func: OCNumberCreateWithDoubleComplex
 * :brief: Creates a new OCNumber object with a double complex value.
 * :param value: The double complex value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithDoubleComplex(double complex value);

/**
 * :func: OCNumberCreateWithLongDouble
 * :brief: Creates a new OCNumber object with a long double value.
 * :param value: The long double value.
 * :return: A new OCNumberRef object, or NULL on failure. Caller owns the returned object.
 */
OCNumberRef OCNumberCreateWithLongDouble(long double value);

/** @} */ // end of OCNumber group

/**
 * :func: OCNumberTypeSize
 * :brief: Returns the size in bytes of the C type corresponding to an OCNumberType.
 * :param type: The OCNumberType to query.
 * :return: The size in bytes of the specified number type. Returns 0 if the type is invalid.
 */
int OCNumberTypeSize(OCNumberType type);

/**
 * :func: OCNumberCreateStringValue
 * :brief: Creates a string representation of the OCNumber's value.
 * :param theNumber: The OCNumberRef object to convert to a string.
 * :return: A new OCStringRef object containing the string representation of the number,
 *          or NULL if theNumber is NULL or on memory allocation failure.
 *          The caller is responsible for releasing the returned OCStringRef using OCRelease.
 */
OCStringRef OCNumberCreateStringValue(OCNumberRef theNumber);

/**
 * :func: OCNumberGetValue
 * :brief: Retrieves the raw value of an OCNumber object.
 * :param number: The OCNumberRef object.
 * :param type: The expected OCNumberType of the value. This function currently requires
 *              this type to match the internal type of the OCNumber object.
 * :param outValue: A pointer to a memory location where the value will be copied.
 *                  The caller must ensure that `outValue` points to a variable of the
 *                  correct type and size corresponding to the `type` parameter.
 * :note: If `number` or `outValue` is NULL, or if `type` does not match the internal
 *        type of `number`, the function does nothing. A more robust implementation
 *        might return a status or handle type conversions.
 */
void OCNumberGetValue(OCNumberRef number, OCNumberType type, void *outValue);

#endif /* OCNumber_h */

//
//  OCNumber.h
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//

#ifndef OCNumber_h
#define OCNumber_h

#include "OCLibrary.h"

typedef const struct __OCNumber * OCNumberRef;

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
    kOCNumberFloat64ComplexType = Float64ComplexType,
} OCNumberType;

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

typedef enum numberType {
    kPSNumberFloat32Type = Float32Type,
    kPSNumberFloat64Type = Float64Type,
    kPSNumberFloat32ComplexType = Float32ComplexType,
    kPSNumberFloat64ComplexType = Float64ComplexType,
} numberType;


OCTypeID OCNumberGetTypeID(void);

OCNumberRef OCNumberCreate(const OCNumberType type, void *value);

OCNumberRef OCNumberCreateWithUInt8(uint8_t value);
OCNumberRef OCNumberCreateWithUInt16(uint16_t value);
OCNumberRef OCNumberCreateWithUInt32(uint32_t value);
OCNumberRef OCNumberCreateWithUInt64(uint64_t value);

OCNumberRef OCNumberCreateWithSInt8(int8_t value);
OCNumberRef OCNumberCreateWithSInt16(int16_t value);
OCNumberRef OCNumberCreateWithSInt32(int32_t value);
OCNumberRef OCNumberCreateWithSInt64(int64_t value);

OCNumberRef OCNumberCreateWithFloat(float value);
OCNumberRef OCNumberCreateWithDouble(double value);

OCNumberRef OCNumberCreateWithFloatComplex(float complex value);
OCNumberRef OCNumberCreateWithDoubleComplex(double complex value);

OCNumberRef OCNumberCreateWithFloatComplex(float complex value);
OCNumberRef OCNumberCreateWithDoubleComplex(double complex value);

int OCNumberTypeSize(OCNumberType type);
OCStringRef OCNumberCreateStringValue(OCNumberRef theNumber);

#endif /* OCNumber_h */

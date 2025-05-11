//
//  OCNumber.c
//  OCTypes
//
//  Created by Philip Grandinetti on 5/15/17.
//

#include "OCLibrary.h"

static OCTypeID kOCNumberID = _kOCNotATypeID;

// OCNumber Opaque Type
struct __OCNumber {
    OCBase _base;

    // OCNumber Type attributes  - order of declaration is essential
    OCNumberType    type;
    __Number        value;
};

static bool __OCNumberEqual(const void * theType1, const void * theType2)
{
    OCNumberRef theNumber1 = (OCNumberRef) theType1;
    OCNumberRef theNumber2 = (OCNumberRef) theType2;
    if(theNumber1->_base.typeID != theNumber2->_base.typeID) return false;
    
    if(NULL == theNumber1 || NULL == theNumber2) return false;
    if(theNumber1 == theNumber2) return true;
    if(theNumber1->type != theNumber2->type) return false;

    switch (theNumber1->type) {
        case kOCNumberUInt8Type: {
            if(theNumber1->value.uint8Value != theNumber2->value.uint8Value) return false;
            break;
        }
        case kOCNumberSInt8Type: {
            if(theNumber1->value.int8Value != theNumber2->value.int8Value) return false;
            break;
        }
        case kOCNumberSInt16Type: {
            if(theNumber1->value.int16Value != theNumber2->value.int16Value) return false;
            break;
        }
        case kOCNumberUInt16Type: {
            if(theNumber1->value.uint16Value != theNumber2->value.uint16Value) return false;
            break;
        }
        case kOCNumberSInt32Type: {
            if(theNumber1->value.int32Value != theNumber2->value.int32Value) return false;
            break;
        }
        case kOCNumberUInt32Type: {
            if(theNumber1->value.uint32Value != theNumber2->value.uint32Value) return false;
            break;
        }
        case kOCNumberUInt64Type: {
            if(theNumber1->value.uint64Value != theNumber2->value.uint64Value) return false;
            break;
        }
        case kOCNumberSInt64Type: {
            if(theNumber1->value.int64Value != theNumber2->value.int64Value) return false;
            break;
        }
        case kOCNumberFloat32Type: {
            if(theNumber1->value.floatValue != theNumber2->value.floatValue) return false;
            break;
        }
        case kOCNumberFloat64Type: {
            if(theNumber1->value.doubleValue != theNumber2->value.doubleValue) return false;
            break;
        }
        case kOCNumberFloat32ComplexType: {
            if(theNumber1->value.floatComplexValue != theNumber2->value.floatComplexValue) return false;
            break;
        }
        case kOCNumberFloat64ComplexType: {
            if(theNumber1->value.floatComplexValue != theNumber2->value.doubleComplexValue) return false;
            break;
        }
    }
    return true;
}

static void __OCNumberFinalize(const void * theType)
{
    if(NULL == theType) return;
    OCNumberRef theNumber = (OCNumberRef) theType;
    free((void *)theNumber);
}

static OCStringRef __OCNumberCopyFormattingDescription(OCTypeRef theType)
{
    OCNumberRef theNumber = (OCNumberRef) theType;
    switch (theNumber->type) {
        case kOCNumberUInt8Type: {
            return  OCStringCreateWithFormat(STR("%u"), theNumber->value.uint8Value);
            break;
        }
        case kOCNumberSInt8Type: {
            return  OCStringCreateWithFormat(STR("%d"), theNumber->value.int8Value);
            break;
        }
        case kOCNumberSInt16Type: {
            return  OCStringCreateWithFormat(STR("%d"), theNumber->value.int16Value);
            break;
        }
        case kOCNumberUInt16Type: {
            return  OCStringCreateWithFormat(STR("%u"), theNumber->value.uint16Value);
            break;
        }
        case kOCNumberSInt32Type: {
            return  OCStringCreateWithFormat(STR("%d"), theNumber->value.int32Value);
            break;
        }
        case kOCNumberUInt32Type: {
            return  OCStringCreateWithFormat(STR("%u"), theNumber->value.uint32Value);
            break;
        }
        case kOCNumberUInt64Type: {
            return  OCStringCreateWithFormat(STR("%lu"), theNumber->value.uint64Value);
            break;
        }
        case kOCNumberSInt64Type: {
            return  OCStringCreateWithFormat(STR("%ld"), theNumber->value.int64Value);
            break;
        }
        case kOCNumberFloat32Type: {
            return  OCStringCreateWithFormat(STR("%f"), theNumber->value.floatValue);
            break;
        }
        case kOCNumberFloat64Type: {
            return  OCStringCreateWithFormat(STR("%lf"), theNumber->value.doubleValue);
            break;
        }
        case kOCNumberFloat32ComplexType: {
            return  OCStringCreateWithFormat(STR("%f+I•%f"), crealf(theNumber->value.floatComplexValue), cimagf(theNumber->value.floatComplexValue));
            break;
        }
        case kOCNumberFloat64ComplexType: {
            return  OCStringCreateWithFormat(STR("%lf+I•%lf"), creal(theNumber->value.doubleComplexValue), cimag(theNumber->value.doubleComplexValue));
            break;
        }
    }
}

OCTypeID OCNumberGetTypeID(void)
{
    if(kOCNumberID == _kOCNotATypeID) kOCNumberID = OCRegisterType("OCNumber");
    return kOCNumberID;
}

static struct __OCNumber *OCNumberAllocate()
{
    struct __OCNumber *theNumber = malloc(sizeof(struct __OCNumber));
    if(NULL == theNumber) return NULL;
    theNumber->_base.typeID = OCNumberGetTypeID();
    theNumber->_base.retainCount = 1;
    theNumber->_base.finalize = __OCNumberFinalize;
    theNumber->_base.equal = __OCNumberEqual;
    theNumber->_base.copyFormattingDesc = __OCNumberCopyFormattingDescription;

    return theNumber;
}

OCNumberRef OCNumberCreate(const OCNumberType type, void *value)
{
    struct __OCNumber *theNumber = OCNumberAllocate();
    if(NULL == theNumber) return NULL;

    theNumber->type = type;
    switch (type) {
        case kOCNumberUInt8Type: {
            uint8_t *ptr = (uint8_t *) value;
            theNumber->value.uint8Value = *ptr;
            break;
        }
        case kOCNumberSInt8Type: {
            int8_t *ptr = (int8_t *) value;
            theNumber->value.int8Value = *ptr;
            break;
        }
        case kOCNumberUInt16Type: {
            uint16_t *ptr = (uint16_t *) value;
            theNumber->value.uint16Value = *ptr;
            break;
        }
        case kOCNumberSInt16Type: {
            int16_t *ptr = (int16_t *) value;
            theNumber->value.int16Value = *ptr;
            break;
        }
        case kOCNumberSInt32Type: {
            int32_t *ptr = (int32_t *) value;
            theNumber->value.int32Value = *ptr;
            break;
        }
        case kOCNumberUInt32Type: {
            uint32_t *ptr = (uint32_t *) value;
            theNumber->value.uint32Value = *ptr;
            break;
        }
        case kOCNumberUInt64Type: {
            uint64_t *ptr = (uint64_t *) value;
            theNumber->value.uint64Value = *ptr;
            break;
        }
        case kOCNumberSInt64Type: {
            int64_t *ptr = (int64_t *) value;
            theNumber->value.int64Value = *ptr;
            break;
        }
        case kOCNumberFloat32Type: {
            float *ptr = (float *) value;
            theNumber->value.floatValue = *ptr;
            break;
        }
        case kOCNumberFloat64Type: {
            double *ptr = (double *) value;
            theNumber->value.doubleValue = *ptr;
            break;
        }
        case kOCNumberFloat32ComplexType: {
            float complex *ptr = (float complex *) value;
            theNumber->value.floatComplexValue = *ptr;
            break;
        }
        case kOCNumberFloat64ComplexType: {
            double complex *ptr = (double complex *) value;
            theNumber->value.doubleComplexValue = *ptr;
            break;
        }
    }
    return theNumber;
}

OCNumberRef OCNumberCreateWithUInt8(uint8_t value){return OCNumberCreate(kOCNumberUInt8Type, &value);}
OCNumberRef OCNumberCreateWithUInt16(uint16_t value){return OCNumberCreate(kOCNumberUInt16Type, &value);}
OCNumberRef OCNumberCreateWithUInt32(uint32_t value){return OCNumberCreate(kOCNumberUInt32Type, &value);}
OCNumberRef OCNumberCreateWithUInt64(uint64_t value){return OCNumberCreate(kOCNumberUInt64Type, &value);}

OCNumberRef OCNumberCreateWithSInt8(int8_t value){return OCNumberCreate(kOCNumberSInt8Type, &value);}
OCNumberRef OCNumberCreateWithSInt16(int16_t value){return OCNumberCreate(kOCNumberSInt16Type, &value);}
OCNumberRef OCNumberCreateWithSInt32(int32_t value){return OCNumberCreate(kOCNumberSInt32Type, &value);}
OCNumberRef OCNumberCreateWithSInt64(int64_t value){return OCNumberCreate(kOCNumberSInt64Type, &value);}

OCNumberRef OCNumberCreateWithFloat(float value){return OCNumberCreate(kOCNumberFloat32Type, &value);}
OCNumberRef OCNumberCreateWithDouble(double value){return OCNumberCreate(kOCNumberFloat64Type, &value);}

OCNumberRef OCNumberCreateWithFloatComplex(float complex value){return OCNumberCreate(kOCNumberFloat32ComplexType, &value);}
OCNumberRef OCNumberCreateWithDoubleComplex(double complex value){return OCNumberCreate(kOCNumberFloat64ComplexType, &value);}

int OCNumberTypeSize(OCNumberType type)
{
    switch (type) {
        case kOCNumberUInt8Type:
        case kOCNumberSInt8Type:
            return sizeof(int8_t);
        case kOCNumberUInt16Type:
        case kOCNumberSInt16Type:
            return sizeof(int16_t);
        case kOCNumberUInt32Type:
        case kOCNumberSInt32Type:
            return sizeof(int32_t);
        case kOCNumberUInt64Type:
        case kOCNumberSInt64Type:
            return sizeof(int64_t);
        case kOCNumberFloat32Type:
            return sizeof(float);
        case kOCNumberFloat64Type:
            return sizeof(double);
        case kOCNumberFloat32ComplexType:
            return sizeof(float complex);
        case kOCNumberFloat64ComplexType:
            return sizeof(double complex);
    }
    return 0;
}

OCStringRef OCNumberCreateStringValue(OCNumberRef theNumber)
{
    if (!theNumber) return NULL;

    OCNumberType type = theNumber->type;
    switch (type) {
        case kOCNumberUInt8Type: {
            return OCStringCreateWithFormat(STR("%hhu"), (unsigned char)theNumber->value.uint8Value);
        }
        case kOCNumberSInt8Type: {
            return OCStringCreateWithFormat(STR("%hhi"), (signed char)theNumber->value.int8Value);
        }
        case kOCNumberUInt16Type: {
            return OCStringCreateWithFormat(STR("%hu"), (unsigned short)theNumber->value.uint16Value);
        }
        case kOCNumberSInt16Type: {
            return OCStringCreateWithFormat(STR("%hi"), (short)theNumber->value.int16Value);
        }
        case kOCNumberUInt32Type: {
            return OCStringCreateWithFormat(STR("%u"), (unsigned int)theNumber->value.uint32Value);
        }
        case kOCNumberSInt32Type: {
            return OCStringCreateWithFormat(STR("%d"), (int)theNumber->value.int32Value);
        }
        case kOCNumberUInt64Type: {
            return OCStringCreateWithFormat(STR("%llu"), (unsigned long long)theNumber->value.uint64Value);
        }
        case kOCNumberSInt64Type: {
            return OCStringCreateWithFormat(STR("%lld"), (long long)theNumber->value.int64Value);
        }
        case kOCNumberFloat32Type: {
            return OCStringCreateWithFormat(STR("%g"), theNumber->value.floatValue);
        }
        case kOCNumberFloat64Type: {
            return OCStringCreateWithFormat(STR("%lg"), theNumber->value.doubleValue);
        }
        case kOCNumberFloat32ComplexType: {
            float r_f = crealf(theNumber->value.floatComplexValue);
            float i_f = cimagf(theNumber->value.floatComplexValue);
            if (r_f == 0.0f && i_f == 0.0f) {
                return OCStringCreateWithCString("0");
            }
            return OCStringCreateWithFormat(STR("%g+I*%g"), r_f, i_f);
        }
        case kOCNumberFloat64ComplexType: {
            double r_d = creal(theNumber->value.doubleComplexValue);
            double i_d = cimag(theNumber->value.doubleComplexValue);
            if (r_d == 0.0 && i_d == 0.0) {
                return OCStringCreateWithCString("0");
            }
            return OCStringCreateWithFormat(STR("%g+I*%g"), r_d, i_d);
        }
    }
    return NULL;
}



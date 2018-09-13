//
//  OCMath.c
//  OCTypes
//
//  Created by philip on 6/7/17.
//  Copyright © 2017 PhySy Ltd. All rights reserved.
//

#include "OCLibrary.h"

double cargument(double complex z)
{
    double temp = cabs(z);
    if(temp == 0.0) return 0.0;
    double cosphase = creal(z)/temp;
    double sinphase = cimag(z)/temp;
    double phase = -acos((double) cosphase);
    if(sinphase < 0) phase *= -1;
    return phase;
}

double complex ccbrt(double complex z)
{
    return cpow(z, 1./3.);
}

double complex cqtrt(double complex z)
{
    return cpow(z, 1./4.);
}

static bool AlmostEqual2sComplementFloat(float A, float B, int maxUlps)
{
    // Make sure maxUlps is non-negative and small enough that the
    // default NAN won't compare as equal to anything.
    assert(maxUlps > 0 && maxUlps < 4 * 1024 * 1024);
    int aInt = *(int*)&A;
    // Make aInt lexicographically ordered as a twos-complement int
    if (aInt < 0)
        aInt = 0x80000000 - aInt;
    // Make bInt lexicographically ordered as a twos-complement int
    int bInt = *(int*)&B;
    if (bInt < 0)
        bInt = 0x80000000 - bInt;
    int intDiff = abs(aInt - bInt);
    if (intDiff <= maxUlps)
        return true;
    return false;
}

static bool AlmostEqual2sComplementDouble(double A, double B, int maxUlps)
{
    // Make sure maxUlps is non-negative and small enough that the
    // default NAN won't compare as equal to anything.
    assert(maxUlps > 0 && maxUlps < 8 * 1024 * 1024);
    int64_t aInt = *(int64_t*)&A;
    // Make aInt lexicographically ordered as a twos-complement int
    if (aInt < 0)
        aInt = 0x8000000000000000 - aInt;
    // Make bInt lexicographically ordered as a twos-complement int
    int64_t bInt = *(int64_t*)&B;
    if (bInt < 0)
        bInt = 0x8000000000000000 - bInt;
    int64_t intDiff = llabs(aInt - bInt);
    if (intDiff <= maxUlps)
        return true;
    return false;
}

OCComparisonResult OCCompareDoubleValues(double value, double otherValue)
{
    //    if(AlmostEqual2sComplementDouble(value, otherValue, 12)) return kOCCompareEqualTo;
    if(AlmostEqual2sComplementDouble(value, otherValue, 14)) return kOCCompareEqualTo;
    if(value>otherValue) return kOCCompareGreaterThan;
    return kOCCompareLessThan;
}

OCComparisonResult OCCompareFloatValues(float value, float otherValue)
{
    if(AlmostEqual2sComplementFloat(value, otherValue, 8)) return kOCCompareEqualTo;
    if(value>otherValue) return kOCCompareGreaterThan;
    return kOCCompareLessThan;
}

OCComparisonResult OCCompareIntegerValues(int64_t value, int64_t otherValue)
{
    OCComparisonResult result = kOCCompareGreaterThan;
    if(value>otherValue) return result;
    if(value<otherValue) result = kOCCompareLessThan;
    if(value==otherValue) result = kOCCompareEqualTo;
    return result;
}

OCComparisonResult OCCompareDoubleValuesLoose(double value, double otherValue)
{
    if(AlmostEqual2sComplementDouble(value, otherValue, 256)) return kOCCompareEqualTo;
    if(value>otherValue) return kOCCompareGreaterThan;
    return kOCCompareLessThan;
}

OCComparisonResult OCCompareFloatValuesLoose(float value, float otherValue)
{
    if(AlmostEqual2sComplementFloat(value, otherValue, 128)) return kOCCompareEqualTo;
    if(value>otherValue) return kOCCompareGreaterThan;
    return kOCCompareLessThan;
}

double OCDoubleFloor(double value)
{
    double ceilValue = ceil(value);
    if(OCCompareDoubleValues(ceilValue,value) == kOCCompareEqualTo) return ceilValue;
    return floor(value);
}

double OCDoubleCeil(double value)
{
    double floorValue = floor(value);
    if(OCCompareDoubleValues(floorValue,value) == kOCCompareEqualTo) return floorValue;
    return ceil(value);
}

double complex complex_sine(double complex angle)
{
    double my_pi = 4*atan(1);
    double real_angle = creal(angle);
    int multiple = rint(real_angle/my_pi);
    double weight = cos(real_angle);
    return csin(angle) - csin(multiple*my_pi)*weight*weight;
}

double complex complex_cosine(double complex angle)
{
    double complex my_pi = 4*atan(1);
    double real_angle = creal(angle);
    int multiple = rint((real_angle-my_pi/2.)/my_pi);
    double weight = sin(real_angle);
    return  ccos(angle) - ccos((2*multiple+1)*my_pi/2.)*weight*weight;
}

double complex complex_tangent(double complex angle)
{
    double complex cosine = complex_cosine(angle);
    double complex sine = complex_sine(angle);
    if(cosine == 0.0) {
        if(signbit(sine)) return -INFINITY;
        else return INFINITY;
    }
    return sine/cosine;
}

double complex raise_to_integer_power(double complex x, long power)
{
    if(power==0) return 1.;
    if(power>0) {
        double complex result = x;
        for(long i=1;i<power;i++) result *= x;
        if(isnan(result)) return nan(NULL);
        return result;
    }
    else if(x!=0.0){
        double complex result = 1./x;
        for(long i=1;i<-power;i++) result *= 1./x;
        if(isnan(result)) return nan(NULL);
        return result;
    }
    return nan(NULL);
}





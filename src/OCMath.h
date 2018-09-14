//
//  OCMath.h
//  OCTypes
//
//  Created by philip on 6/7/17.
//

#ifndef OCMath_h
#define OCMath_h

#include "OCLibrary.h"


double cargument(double complex z);
double complex ccbrt(double complex z);
double complex cqtrt(double complex z);
OCComparisonResult OCCompareFloatValues(float value, float otherValue);
OCComparisonResult OCCompareDoubleValues(double value, double otherValue);
OCComparisonResult OCCompareDoubleValuesLoose(double value, double otherValue);
OCComparisonResult OCCompareFloatValuesLoose(float value, float otherValue);
double OCDoubleFloor(double value);
double OCDoubleCeil(double value);

double complex complex_sine(double complex angle);
double complex complex_cosine(double complex angle);
double complex complex_tangent(double complex angle);

#endif /* OCMath_h */

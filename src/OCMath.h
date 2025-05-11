//
//  OCMath.h
//  OCTypes
//
//  Created by philip on 6/7/17.
//

#ifndef OCMath_h
#define OCMath_h

#include <complex.h>
#include "OCLibrary.h"

/** @defgroup OCMath OCMath */
/** @addtogroup OCMath
 *  @{
 */

/**
 * :func: cargument
 * :brief: Calculates the argument (or phase) of a complex number.
 * :param z: The complex number.
 * :return: The argument of z in radians.
 */
double cargument(double complex z);

/**
 * :func: ccbrt
 * :brief: Calculates the principal complex cube root of a complex number.
 * :param z: The complex number.
 * :return: The principal complex cube root of z.
 */
double complex ccbrt(double complex z);

/**
 * :func: cqtrt
 * :brief: Calculates the principal complex fourth root of a complex number.
 * :param z: The complex number.
 * :return: The principal complex fourth root of z.
 */
double complex cqtrt(double complex z);

/**
 * :func: OCCompareFloatValues
 * :brief: Compares two float values.
 * :param value: The first float value.
 * :param otherValue: The second float value.
 * :return: kOCCompareLessThan if value < otherValue,
 *          kOCCompareEqualTo if value == otherValue,
 *          kOCCompareGreaterThan if value > otherValue.
 */
OCComparisonResult OCCompareFloatValues(float value, float otherValue);

/**
 * :func: OCCompareDoubleValues
 * :brief: Compares two double values.
 * :param value: The first double value.
 * :param otherValue: The second double value.
 * :return: kOCCompareLessThan if value < otherValue,
 *          kOCCompareEqualTo if value == otherValue,
 *          kOCCompareGreaterThan if value > otherValue.
 */
OCComparisonResult OCCompareDoubleValues(double value, double otherValue);

/**
 * :func: OCCompareDoubleValuesLoose
 * :brief: Compares two double values with a tolerance (loose comparison).
 * :param value: The first double value.
 * :param otherValue: The second double value.
 * :return: kOCCompareEqualTo if the absolute difference is within a small epsilon,
 *          otherwise the result of a direct comparison.
 */
OCComparisonResult OCCompareDoubleValuesLoose(double value, double otherValue);

/**
 * :func: OCCompareFloatValuesLoose
 * :brief: Compares two float values with a tolerance (loose comparison).
 * :param value: The first float value.
 * :param otherValue: The second float value.
 * :return: kOCCompareEqualTo if the absolute difference is within a small epsilon,
 *          otherwise the result of a direct comparison.
 */
OCComparisonResult OCCompareFloatValuesLoose(float value, float otherValue);

/**
 * :func: OCDoubleFloor
 * :brief: Computes the floor of a double value.
 * :param value: The double value.
 * :return: The largest integer value not greater than value.
 */
double OCDoubleFloor(double value);

/**
 * :func: OCDoubleCeil
 * :brief: Computes the ceiling of a double value.
 * :param value: The double value.
 * :return: The smallest integer value not less than value.
 */
double OCDoubleCeil(double value);

/**
 * :func: complex_sine
 * :brief: Calculates the complex sine of a complex angle.
 * :param angle: The complex angle in radians.
 * :return: The complex sine of the angle.
 */
double complex complex_sine(double complex angle);

/**
 * :func: complex_cosine
 * :brief: Calculates the complex cosine of a complex angle.
 * :param angle: The complex angle in radians.
 * :return: The complex cosine of the angle.
 */
double complex complex_cosine(double complex angle);

/**
 * :func: complex_tangent
 * :brief: Calculates the complex tangent of a complex angle.
 * :param angle: The complex angle in radians.
 * :return: The complex tangent of the angle.
 */
double complex complex_tangent(double complex angle);

/** @} */ // end of OCMath group

#endif /* OCMath_h */

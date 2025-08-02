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
 * @brief Calculates the argument (or phase) of a complex number.
 * @param z The complex number.
 * @return The argument of z in radians.
 * @ingroup OCMath
 */
double cargument(double complex z);
/**
 * @brief Calculates the principal complex cube root of a complex number.
 * @param z The complex number.
 * @return The principal complex cube root of z.
 * @ingroup OCMath
 */
double complex ccbrt(double complex z);
/**
 * @brief Calculates the principal complex fourth root of a complex number.
 * @param z The complex number.
 * @return The principal complex fourth root of z.
 * @ingroup OCMath
 */
double complex cqtrt(double complex z);
/**
 * @brief Compares two float values.
 * @param value The first float value.
 * @param otherValue The second float value.
 * @return kOCCompareLessThan if value < otherValue,
 *         kOCCompareEqualTo if value == otherValue,
 *         kOCCompareGreaterThan if value > otherValue.
 * @ingroup OCMath
 */
OCComparisonResult OCCompareFloatValues(float value, float otherValue);
/**
 * @brief Compares two double values.
 * @param value The first double value.
 * @param otherValue The second double value.
 * @return kOCCompareLessThan if value < otherValue,
 *         kOCCompareEqualTo if value == otherValue,
 *         kOCCompareGreaterThan if value > otherValue.
 * @ingroup OCMath
 */
OCComparisonResult OCCompareDoubleValues(double value, double otherValue);
/**
 * @brief Compares two double values with a tolerance (loose comparison).
 * @param value The first double value.
 * @param otherValue The second double value.
 * @return kOCCompareEqualTo if the absolute difference is within a small epsilon,
 *         otherwise the result of a direct comparison.
 * @ingroup OCMath
 */
OCComparisonResult OCCompareDoubleValuesLoose(double value, double otherValue);
/**
 * @brief Compares two float values with a tolerance (loose comparison).
 * @param value The first float value.
 * @param otherValue The second float value.
 * @return kOCCompareEqualTo if the absolute difference is within a small epsilon,
 *         otherwise the result of a direct comparison.
 * @ingroup OCMath
 */
OCComparisonResult OCCompareFloatValuesLoose(float value, float otherValue);
/**
 * @brief Computes the floor of a double value.
 * @param value The double value.
 * @return The largest integer value not greater than value.
 * @ingroup OCMath
 */
double OCDoubleFloor(double value);
/**
 * @brief Computes the ceiling of a double value.
 * @param value The double value.
 * @return The smallest integer value not less than value.
 * @ingroup OCMath
 */
double OCDoubleCeil(double value);
/**
 * @brief Calculates the complex sine of a complex angle.
 * @param angle The complex angle in radians.
 * @return The complex sine of the angle.
 * @ingroup OCMath
 */
double complex complex_sine(double complex angle);
/**
 * @brief Calculates the complex cosine of a complex angle.
 * @param angle The complex angle in radians.
 * @return The complex cosine of the angle.
 * @ingroup OCMath
 */
double complex complex_cosine(double complex angle);
/**
 * @brief Calculates the complex tangent of a complex angle.
 * @param angle The complex angle in radians.
 * @return The complex tangent of the angle.
 * @ingroup OCMath
 */
double complex complex_tangent(double complex angle);
/**
 * @brief Raises a complex number to an integer power.
 * @param x The complex base.
 * @param power The integer exponent.
 * @return The complex number x raised to the power of power.
 * @ingroup OCMath
 */
double complex raise_to_integer_power(double complex x, long power);
/** @} */  // end of OCMath group
#endif     /* OCMath_h */

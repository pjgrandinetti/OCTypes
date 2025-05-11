/**
 * @file OCMath.h
 * @brief Extended math utilities for real and complex number operations in OCTypes.
 *
 * This header declares functions for working with complex numbers and performing
 * numeric comparisons with precision tolerance, along with basic floor/ceil operations.
 */

#ifndef OCMath_h
#define OCMath_h

#include <complex.h>
#include "OCLibrary.h"

/**
 * @defgroup OCMath OCMath
 * @brief Mathematical utilities, including complex arithmetic and tolerant comparisons.
 * @{
 */

/**
 * @brief Calculates the argument (phase angle) of a complex number.
 * @param z Complex number.
 * @return Argument of `z` in radians.
 * @ingroup OCMath
 */
double cargument(double complex z);

/**
 * @brief Calculates the principal cube root of a complex number.
 * @param z Complex number.
 * @return Principal complex cube root of `z`.
 * @ingroup OCMath
 */
double complex ccbrt(double complex z);

/**
 * @brief Calculates the principal fourth root of a complex number.
 * @param z Complex number.
 * @return Principal complex fourth root of `z`.
 * @ingroup OCMath
 */
double complex cqtrt(double complex z);

/**
 * @brief Compares two float values using strict equality.
 * @param value First float value.
 * @param otherValue Second float value.
 * @return Comparison result.
 * @ingroup OCMath
 */
OCComparisonResult OCCompareFloatValues(float value, float otherValue);

/**
 * @brief Compares two double values using strict equality.
 * @param value First double value.
 * @param otherValue Second double value.
 * @return Comparison result.
 * @ingroup OCMath
 */
OCComparisonResult OCCompareDoubleValues(double value, double otherValue);

/**
 * @brief Loosely compares two double values using a precision threshold.
 * @param value First double value.
 * @param otherValue Second double value.
 * @return kOCCompareEqualTo if within epsilon; otherwise, result of strict comparison.
 * @ingroup OCMath
 */
OCComparisonResult OCCompareDoubleValuesLoose(double value, double otherValue);

/**
 * @brief Loosely compares two float values using a precision threshold.
 * @param value First float value.
 * @param otherValue Second float value.
 * @return kOCCompareEqualTo if within epsilon; otherwise, result of strict comparison.
 * @ingroup OCMath
 */
OCComparisonResult OCCompareFloatValuesLoose(float value, float otherValue);

/**
 * @brief Returns the largest integer not greater than the given double value.
 * @param value A double-precision value.
 * @return The floor of `value`.
 * @ingroup OCMath
 */
double OCDoubleFloor(double value);

/**
 * @brief Returns the smallest integer not less than the given double value.
 * @param value A double-precision value.
 * @return The ceiling of `value`.
 * @ingroup OCMath
 */
double OCDoubleCeil(double value);

/**
 * @brief Computes the complex sine of a complex number.
 * @param angle A complex angle in radians.
 * @return Complex sine of `angle`.
 * @ingroup OCMath
 */
double complex complex_sine(double complex angle);

/**
 * @brief Computes the complex cosine of a complex number.
 * @param angle A complex angle in radians.
 * @return Complex cosine of `angle`.
 * @ingroup OCMath
 */
double complex complex_cosine(double complex angle);

/**
 * @brief Computes the complex tangent of a complex number.
 * @param angle A complex angle in radians.
 * @return Complex tangent of `angle`.
 * @ingroup OCMath
 */
double complex complex_tangent(double complex angle);

/** @} */ // end of OCMath group

#endif /* OCMath_h */

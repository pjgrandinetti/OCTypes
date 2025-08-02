#ifndef TEST_MATH_H
#define TEST_MATH_H
#include "test_utils.h"
// Test prototype for math tests
bool mathTest0(void);
// New: tests for additional math functions
bool mathTest1(void);
// If raise_to_integer_power isn't in OCMath.h, declare it (though it should be)
// This declaration might be specific to how tests were originally structured.
// It's better if OCMath.h provides all necessary declarations.
extern double complex raise_to_integer_power(double complex x, long power);
#endif /* TEST_MATH_H */

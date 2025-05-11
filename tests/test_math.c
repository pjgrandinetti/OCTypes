#include "test_math.h"
#include "../src/OCMath.h" // For OCCompareDoubleValuesLoose and other math functions if used directly
#include <complex.h>   // For double complex, creal, cimag, I

// Definition for raise_to_integer_power if it's not part of the library
// This is here because it was extern in the original main.c
// Ideally, this function should be part of OCMath.c and declared in OCMath.h
/* extern double complex raise_to_integer_power(double complex x, long power); */
// Assuming raise_to_integer_power is available from linking with OCMath.o or similar

// Original mathTest0 implementation
bool mathTest0(void) {
    fprintf(stderr, "%s begin...\n", __func__);
    double complex z = 2.0 + 3.0*I;
    double complex r0 = raise_to_integer_power(z,0);
    if (OCCompareDoubleValuesLoose(creal(r0),1.0)!=kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValuesLoose(cimag(r0),0.0)!=kOCCompareEqualTo) PRINTERROR;
    
    double complex r2 = raise_to_integer_power(z,2);
    double complex e2 = z*z;
    if (OCCompareDoubleValuesLoose(creal(r2),creal(e2))!=kOCCompareEqualTo) PRINTERROR;
    if (OCCompareDoubleValuesLoose(cimag(r2),cimag(e2))!=kOCCompareEqualTo) PRINTERROR;
    
    fprintf(stderr, "%s end...without problems\n", __func__);
    return true;
}
